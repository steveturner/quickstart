package live.ditto.pubsec.pos.ui.orders

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
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
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import live.ditto.pubsec.pos.data.model.Order
import live.ditto.pubsec.pos.data.model.OrderStatus
import org.koin.androidx.compose.koinViewModel

@Composable
fun OrdersScreen(vm: OrdersViewModel = koinViewModel()) {
    val orders by vm.orders.collectAsStateWithLifecycle()

    if (orders.isEmpty()) {
        Column(
            modifier = Modifier.fillMaxSize().padding(16.dp),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text("No orders yet", style = MaterialTheme.typography.titleMedium)
            Spacer(Modifier.height(8.dp))
            Text("Create an order from the Catalog tab", style = MaterialTheme.typography.bodyMedium)
        }
    } else {
        LazyColumn(
            contentPadding = PaddingValues(8.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
            modifier = Modifier.fillMaxSize()
        ) {
            items(orders, key = { it._id }) { order ->
                OrderCard(order)
            }
        }
    }
}

@Composable
private fun OrderCard(order: Order) {
    Card(
        modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 1.dp)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "Order #${order._id.take(8)}",
                    style = MaterialTheme.typography.titleSmall
                )
                Surface(
                    shape = MaterialTheme.shapes.small,
                    color = when (order.status) {
                        OrderStatus.FULFILLED.name -> MaterialTheme.colorScheme.tertiaryContainer
                        else -> MaterialTheme.colorScheme.secondaryContainer
                    }
                ) {
                    Text(
                        order.status,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 2.dp),
                        style = MaterialTheme.typography.labelSmall,
                        color = when (order.status) {
                            OrderStatus.FULFILLED.name -> MaterialTheme.colorScheme.onTertiaryContainer
                            else -> MaterialTheme.colorScheme.onSecondaryContainer
                        }
                    )
                }
            }

            Spacer(Modifier.height(8.dp))

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                val itemCount = order.items.sumOf { it.quantity }
                Text(
                    "$itemCount item${if (itemCount != 1) "s" else ""}",
                    style = MaterialTheme.typography.bodyMedium
                )
                Text(
                    "$${String.format("%.2f", order.total / 100.0)}",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.primary
                )
            }

            Spacer(Modifier.height(4.dp))

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    if (order.terminalId.isNotEmpty()) "Terminal: ${order.terminalId}" else "",
                    style = MaterialTheme.typography.labelSmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Text(
                    formatTimestamp(order.timestamp),
                    style = MaterialTheme.typography.labelSmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}

private fun formatTimestamp(millis: Long): String {
    if (millis == 0L) return ""
    val sdf = java.text.SimpleDateFormat("MMM d, h:mm a", java.util.Locale.getDefault())
    return sdf.format(java.util.Date(millis))
}
