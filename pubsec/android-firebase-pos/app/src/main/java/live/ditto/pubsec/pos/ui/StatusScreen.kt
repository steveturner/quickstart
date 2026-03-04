package live.ditto.pubsec.pos.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.google.firebase.firestore.FirebaseFirestore
import live.ditto.Ditto
import org.koin.compose.koinInject

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun StatusScreen(
    ditto: Ditto = koinInject(),
    firestore: FirebaseFirestore = koinInject()
) {
    Scaffold(
        topBar = {
            TopAppBar(title = { Text("Ditto POS Bridge") })
        }
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues)
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Text(
                text = "Ditto SDK",
                style = MaterialTheme.typography.titleMedium
            )
            Text(
                text = "Status: initialized",
                style = MaterialTheme.typography.bodyMedium
            )
            Text(
                text = "Device: ${ditto.presence.graph.localPeer.deviceName}",
                style = MaterialTheme.typography.bodySmall
            )

            Spacer(modifier = Modifier.height(16.dp))

            Text(
                text = "Firebase Firestore",
                style = MaterialTheme.typography.titleMedium
            )
            Text(
                text = "Status: connected (offline cache disabled)",
                style = MaterialTheme.typography.bodyMedium
            )
            Text(
                text = "App: ${firestore.app.name}",
                style = MaterialTheme.typography.bodySmall
            )
        }
    }
}
