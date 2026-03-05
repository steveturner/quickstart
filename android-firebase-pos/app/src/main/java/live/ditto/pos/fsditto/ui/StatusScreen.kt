package live.ditto.pos.fsditto.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import org.koin.compose.koinInject
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

@OptIn(ExperimentalMaterial3Api::class, ExperimentalLayoutApi::class)
@Composable
fun StatusScreen(vm: StatusViewModel = koinInject()) {
    val peers by vm.peers.collectAsStateWithLifecycle()
    val isFirebaseConnected by vm.isFirebaseConnected.collectAsStateWithLifecycle()
    val firestoreEnabled by vm.firestoreEnabled.collectAsStateWithLifecycle()
    val syncLog by vm.syncLog.collectAsStateWithLifecycle()
    val dittoDocCounts by vm.dittoDocCounts.collectAsStateWithLifecycle()

    val localPeer = peers.firstOrNull { it.isLocal }
    val remotePeers = peers.filter { !it.isLocal }

    Scaffold(
        topBar = {
            TopAppBar(title = { Text("Ditto POS Bridge") })
        }
    ) { paddingValues ->
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            // Firestore toggle card
            item {
                Card(
                    modifier = Modifier.fillMaxWidth(),
                    colors = CardDefaults.cardColors(
                        containerColor = if (firestoreEnabled)
                            MaterialTheme.colorScheme.primaryContainer
                        else
                            MaterialTheme.colorScheme.errorContainer
                    )
                ) {
                    Column(modifier = Modifier.padding(16.dp)) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Column {
                                Text("Firestore Network", style = MaterialTheme.typography.titleMedium)
                                Text(
                                    if (firestoreEnabled) "Enabled -- primary store"
                                    else "Disabled -- Ditto fallback active",
                                    style = MaterialTheme.typography.bodySmall
                                )
                            }
                            Switch(
                                checked = firestoreEnabled,
                                onCheckedChange = { vm.toggleFirestoreNetwork() }
                            )
                        }
                        Spacer(Modifier.height(8.dp))
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(12.dp)
                        ) {
                            StatusChip(
                                label = "RTDB",
                                connected = isFirebaseConnected
                            )
                            StatusChip(
                                label = "Network",
                                connected = firestoreEnabled
                            )
                        }
                    }
                }
            }

            // Ditto store counts
            item {
                Card(modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(16.dp)) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text("Ditto Local Store", style = MaterialTheme.typography.titleMedium)
                            OutlinedButton(onClick = { vm.refreshDittoCounts() }) {
                                Text("Refresh")
                            }
                        }
                        Spacer(Modifier.height(8.dp))
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceEvenly
                        ) {
                            CountChip("Products", dittoDocCounts["products"] ?: 0)
                            CountChip("Orders", dittoDocCounts["orders"] ?: 0)
                            CountChip("Inventory", dittoDocCounts["inventory"] ?: 0)
                        }
                    }
                }
            }

            // Local device + peers
            item {
                Card(
                    modifier = Modifier.fillMaxWidth(),
                    colors = CardDefaults.cardColors(
                        containerColor = MaterialTheme.colorScheme.surfaceVariant
                    )
                ) {
                    Column(modifier = Modifier.padding(16.dp)) {
                        Text("This Device", style = MaterialTheme.typography.titleMedium)
                        Spacer(Modifier.height(4.dp))
                        Text(
                            localPeer?.deviceName ?: "Initializing...",
                            style = MaterialTheme.typography.bodyMedium
                        )
                    }
                }
            }

            item {
                Text(
                    "Remote Peers (${remotePeers.size})",
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier.padding(top = 4.dp)
                )
            }

            if (remotePeers.isEmpty()) {
                item {
                    Text(
                        "No peers connected",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            } else {
                items(remotePeers, key = { it.deviceName }) { peer ->
                    Card(
                        modifier = Modifier.fillMaxWidth(),
                        elevation = CardDefaults.cardElevation(defaultElevation = 1.dp)
                    ) {
                        Column(modifier = Modifier.padding(16.dp)) {
                            Text(peer.deviceName, style = MaterialTheme.typography.titleSmall)
                            Spacer(Modifier.height(4.dp))
                            FlowRow(horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                                peer.connectionTypes.forEach { type ->
                                    Surface(
                                        shape = MaterialTheme.shapes.small,
                                        color = MaterialTheme.colorScheme.secondaryContainer
                                    ) {
                                        Text(
                                            type,
                                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 2.dp),
                                            style = MaterialTheme.typography.labelSmall
                                        )
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Sync log
            item {
                HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))
                Text("Sync Log", style = MaterialTheme.typography.titleMedium)
            }

            if (syncLog.isEmpty()) {
                item {
                    Text(
                        "No sync events yet -- try adding a product or submitting an order",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            } else {
                items(syncLog, key = { it.timestamp }) { entry ->
                    val time = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
                        .format(Date(entry.timestamp))
                    val color = when (entry.operation) {
                        "error" -> MaterialTheme.colorScheme.error
                        "fallback" -> MaterialTheme.colorScheme.tertiary
                        else -> MaterialTheme.colorScheme.onSurface
                    }
                    Text(
                        "$time [${entry.target}] ${entry.operation}: ${entry.detail}",
                        style = MaterialTheme.typography.bodySmall,
                        color = color
                    )
                }
            }
        }
    }
}

@Composable
private fun StatusChip(label: String, connected: Boolean) {
    Surface(
        shape = MaterialTheme.shapes.small,
        color = if (connected)
            MaterialTheme.colorScheme.tertiaryContainer
        else
            MaterialTheme.colorScheme.errorContainer
    ) {
        Text(
            "$label: ${if (connected) "ON" else "OFF"}",
            modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp),
            style = MaterialTheme.typography.labelSmall,
            color = if (connected)
                MaterialTheme.colorScheme.onTertiaryContainer
            else
                MaterialTheme.colorScheme.onErrorContainer
        )
    }
}

@Composable
private fun CountChip(label: String, count: Int) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(
            count.toString(),
            style = MaterialTheme.typography.titleLarge,
            color = MaterialTheme.colorScheme.primary
        )
        Text(label, style = MaterialTheme.typography.labelSmall)
    }
}
