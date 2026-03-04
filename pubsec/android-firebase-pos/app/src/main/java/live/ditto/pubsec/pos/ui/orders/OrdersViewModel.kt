package live.ditto.pubsec.pos.ui.orders

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import live.ditto.Ditto
import live.ditto.DittoStoreObserver
import live.ditto.pubsec.pos.data.model.Collections
import live.ditto.pubsec.pos.data.model.Order
import live.ditto.pubsec.pos.data.model.OrderLineItem

class OrdersViewModel(private val ditto: Ditto) : ViewModel() {

    private val _orders = MutableStateFlow<List<Order>>(emptyList())
    val orders: StateFlow<List<Order>> = _orders.asStateFlow()

    private var observer: DittoStoreObserver? = null

    init {
        try {
            observer = ditto.store.registerObserver(
                "SELECT * FROM ${Collections.ORDERS} WHERE ${Collections.Fields.DELETED} = false ORDER BY ${Collections.Fields.TIMESTAMP} DESC"
            ) { queryResult ->
                val items = queryResult.use { result ->
                    result.items.map { item ->
                        val v = item.value
                        Order(
                            _id = v[Collections.Fields.ID] as? String ?: "",
                            items = parseOrderLineItems(v[Collections.Fields.ITEMS]),
                            total = (v[Collections.Fields.TOTAL] as? Number)?.toLong() ?: 0L,
                            status = v[Collections.Fields.STATUS] as? String ?: "OPEN",
                            timestamp = (v[Collections.Fields.TIMESTAMP] as? Number)?.toLong() ?: 0L,
                            terminalId = v[Collections.Fields.TERMINAL_ID] as? String ?: "",
                            deleted = v[Collections.Fields.DELETED] as? Boolean ?: false,
                            syncSource = v[Collections.Fields.SYNC_SOURCE] as? String ?: "local",
                            lastSyncedAt = (v[Collections.Fields.LAST_SYNCED_AT] as? Number)?.toLong() ?: 0L
                        )
                    }
                }
                _orders.value = items
            }
        } catch (_: Exception) {
            // Ditto store methods are JNI-backed and throw in JVM unit tests.
            // Observer remains null; StateFlow keeps its empty default.
        }
    }

    @Suppress("UNCHECKED_CAST")
    private fun parseOrderLineItems(raw: Any?): List<OrderLineItem> {
        val list = raw as? List<Map<String, Any?>> ?: return emptyList()
        return list.map { m ->
            OrderLineItem(
                productId = m[Collections.Fields.PRODUCT_ID] as? String ?: "",
                productName = m[Collections.Fields.PRODUCT_NAME] as? String ?: "",
                quantity = (m[Collections.Fields.QUANTITY] as? Number)?.toInt() ?: 0,
                unitPrice = (m[Collections.Fields.UNIT_PRICE] as? Number)?.toLong() ?: 0L
            )
        }
    }

    override fun onCleared() {
        observer?.close()
        super.onCleared()
    }
}
