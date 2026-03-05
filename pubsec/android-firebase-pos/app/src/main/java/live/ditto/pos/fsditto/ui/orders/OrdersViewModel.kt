package live.ditto.pos.fsditto.ui.orders

import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.google.firebase.firestore.FirebaseFirestore
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.tasks.await
import live.ditto.Ditto
import live.ditto.DittoStoreObserver
import live.ditto.pos.fsditto.data.model.Collections
import live.ditto.pos.fsditto.data.model.Order
import live.ditto.pos.fsditto.data.model.OrderLineItem
import live.ditto.pos.fsditto.ui.StatusViewModel

class OrdersViewModel(
    private val ditto: Ditto,
    private val firestore: FirebaseFirestore,
    private val statusViewModel: StatusViewModel
) : ViewModel() {

    companion object {
        private const val TAG = "OrdersVM"
    }

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
        }
    }

    fun deleteOrder(orderId: String) {
        viewModelScope.launch {
            val wroteToFirestore = try {
                firestore.collection(Collections.ORDERS)
                    .document(orderId)
                    .update(Collections.Fields.DELETED, true)
                    .await()
                Log.i(TAG, "Firestore deleteOrder OK: $orderId")
                statusViewModel.addLogEntry("Firestore", "write", "deleteOrder(#${orderId.take(8)})")
                true
            } catch (e: Throwable) {
                Log.w(TAG, "Firestore deleteOrder FAILED: ${e.message}")
                statusViewModel.addLogEntry("Firestore", "error", "deleteOrder(#${orderId.take(8)}): ${e.message}")
                false
            }
            if (!wroteToFirestore) {
                try {
                    ditto.store.execute(
                        "UPDATE ${Collections.ORDERS} SET ${Collections.Fields.DELETED} = true WHERE ${Collections.Fields.ID} = :id",
                        mapOf("id" to orderId)
                    )
                    Log.i(TAG, "Ditto fallback deleteOrder OK: $orderId")
                    statusViewModel.addLogEntry("Ditto", "fallback", "deleteOrder(#${orderId.take(8)})")
                } catch (e: Throwable) {
                    Log.e(TAG, "Ditto fallback deleteOrder FAILED: ${e.message}")
                    statusViewModel.addLogEntry("Ditto", "error", "deleteOrder(#${orderId.take(8)}): ${e.message}")
                }
            }
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
