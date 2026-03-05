package live.ditto.pos.fsditto.ui.catalog

import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.google.firebase.firestore.FirebaseFirestore
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.tasks.await
import live.ditto.Ditto
import live.ditto.DittoStoreObserver
import live.ditto.pos.fsditto.data.model.Collections
import live.ditto.pos.fsditto.data.model.OrderStatus
import live.ditto.pos.fsditto.data.model.Product
import live.ditto.pos.fsditto.ui.StatusViewModel

class CatalogViewModel(
    private val ditto: Ditto,
    private val firestore: FirebaseFirestore,
    private val statusViewModel: StatusViewModel
) : ViewModel() {

    companion object {
        private const val TAG = "CatalogVM"
    }

    private val _products = MutableStateFlow<List<Product>>(emptyList())
    val products: StateFlow<List<Product>> = _products.asStateFlow()

    private val _inventory = MutableStateFlow<Map<String, Int>>(emptyMap())
    val inventory: StateFlow<Map<String, Int>> = _inventory.asStateFlow()

    private val _cart = MutableStateFlow<Map<String, Int>>(emptyMap())
    val cart: StateFlow<Map<String, Int>> = _cart.asStateFlow()

    private val _selectedCategory = MutableStateFlow<String?>(null)
    val selectedCategory: StateFlow<String?> = _selectedCategory.asStateFlow()

    private val _showAddDialog = MutableStateFlow(false)
    val showAddDialog: StateFlow<Boolean> = _showAddDialog.asStateFlow()

    private val _orderPlaced = MutableSharedFlow<String>()
    val orderPlaced: SharedFlow<String> = _orderPlaced.asSharedFlow()

    private var productObserver: DittoStoreObserver? = null
    private var inventoryObserver: DittoStoreObserver? = null

    init {
        setupObservers()
    }

    private fun setupObservers() {
        try {
            productObserver = ditto.store.registerObserver(
                "SELECT * FROM ${Collections.PRODUCTS} WHERE ${Collections.Fields.DELETED} = false"
            ) { queryResult ->
                val items = queryResult.use { result ->
                    result.items.map { item ->
                        val v = item.value
                        Product(
                            _id = v[Collections.Fields.ID] as? String ?: "",
                            name = v[Collections.Fields.NAME] as? String ?: "",
                            category = v[Collections.Fields.CATEGORY] as? String ?: "",
                            price = (v[Collections.Fields.PRICE] as? Number)?.toLong() ?: 0L,
                            imageUrl = v[Collections.Fields.IMAGE_URL] as? String ?: "",
                            deleted = v[Collections.Fields.DELETED] as? Boolean ?: false,
                            syncSource = v[Collections.Fields.SYNC_SOURCE] as? String ?: "local",
                            lastSyncedAt = (v[Collections.Fields.LAST_SYNCED_AT] as? Number)?.toLong() ?: 0L
                        )
                    }
                }
                _products.value = items
            }

            inventoryObserver = ditto.store.registerObserver(
                "SELECT * FROM ${Collections.INVENTORY} WHERE ${Collections.Fields.DELETED} = false"
            ) { queryResult ->
                val items = queryResult.use { result ->
                    result.items.associate { item ->
                        val v = item.value
                        val productId = v[Collections.Fields.PRODUCT_ID] as? String ?: ""
                        val quantity = (v[Collections.Fields.QUANTITY] as? Number)?.toInt() ?: 0
                        productId to quantity
                    }
                }
                _inventory.value = items
            }
        } catch (_: Exception) {
            // Ditto store methods are JNI-backed and throw in JVM unit tests.
        }
    }

    fun setShowAddDialog(show: Boolean) {
        _showAddDialog.value = show
    }

    fun selectCategory(category: String?) {
        _selectedCategory.value = category
    }

    fun addProduct(name: String, price: Long, category: String) {
        viewModelScope.launch {
            val productId = java.util.UUID.randomUUID().toString()
            val inventoryId = java.util.UUID.randomUUID().toString()
            val productData = mapOf(
                Collections.Fields.NAME to name,
                Collections.Fields.CATEGORY to category,
                Collections.Fields.PRICE to price,
                Collections.Fields.IMAGE_URL to "",
                Collections.Fields.DELETED to false,
                Collections.Fields.SYNC_SOURCE to "local",
                Collections.Fields.LAST_SYNCED_AT to 0L
            )
            val invData = mapOf(
                Collections.Fields.PRODUCT_ID to productId,
                Collections.Fields.QUANTITY to 0,
                Collections.Fields.LAST_MODIFIED_BY to "",
                Collections.Fields.DELETED to false,
                Collections.Fields.SYNC_SOURCE to "local",
                Collections.Fields.LAST_SYNCED_AT to 0L
            )

            val wroteToFirestore = tryFirestoreWrite("addProduct($name)") {
                firestore.collection(Collections.PRODUCTS)
                    .document(productId).set(productData).await()
                firestore.collection(Collections.INVENTORY)
                    .document(inventoryId).set(invData).await()
            }
            if (!wroteToFirestore) {
                tryDittoFallback("addProduct($name)") {
                    ditto.store.execute(
                        "INSERT INTO ${Collections.PRODUCTS} DOCUMENTS (:doc) ON ID CONFLICT DO NOTHING",
                        mapOf("doc" to productData + (Collections.Fields.ID to productId))
                    )
                    ditto.store.execute(
                        "INSERT INTO ${Collections.INVENTORY} DOCUMENTS (:doc) ON ID CONFLICT DO NOTHING",
                        mapOf("doc" to invData + (Collections.Fields.ID to inventoryId))
                    )
                }
            }
            _showAddDialog.value = false
        }
    }

    fun deleteProduct(productId: String) {
        viewModelScope.launch {
            val wroteToFirestore = tryFirestoreWrite("deleteProduct($productId)") {
                firestore.collection(Collections.PRODUCTS)
                    .document(productId)
                    .update(Collections.Fields.DELETED, true)
                    .await()
                val invSnapshot = firestore.collection(Collections.INVENTORY)
                    .whereEqualTo(Collections.Fields.PRODUCT_ID, productId)
                    .get().await()
                for (doc in invSnapshot.documents) {
                    doc.reference.update(Collections.Fields.DELETED, true).await()
                }
            }
            if (!wroteToFirestore) {
                tryDittoFallback("deleteProduct($productId)") {
                    ditto.store.execute(
                        "UPDATE ${Collections.PRODUCTS} SET ${Collections.Fields.DELETED} = true WHERE ${Collections.Fields.ID} = :id",
                        mapOf("id" to productId)
                    )
                    ditto.store.execute(
                        "UPDATE ${Collections.INVENTORY} SET ${Collections.Fields.DELETED} = true WHERE ${Collections.Fields.PRODUCT_ID} = :pid",
                        mapOf("pid" to productId)
                    )
                }
            }
        }
    }

    fun addToCart(productId: String) {
        _cart.update { current ->
            current.toMutableMap().apply {
                this[productId] = (this[productId] ?: 0) + 1
            }
        }
    }

    fun removeFromCart(productId: String) {
        _cart.update { current ->
            current.toMutableMap().apply {
                val qty = (this[productId] ?: 0) - 1
                if (qty <= 0) remove(productId) else this[productId] = qty
            }
        }
    }

    fun submitOrder() {
        val currentCart = _cart.value
        val currentProducts = _products.value
        if (currentCart.isEmpty()) return

        viewModelScope.launch {
            val orderId = java.util.UUID.randomUUID().toString()
            val lineItems = currentCart.mapNotNull { (productId, qty) ->
                val product = currentProducts.find { it._id == productId }
                    ?: return@mapNotNull null
                mapOf(
                    Collections.Fields.PRODUCT_ID to productId,
                    Collections.Fields.PRODUCT_NAME to product.name,
                    Collections.Fields.QUANTITY to qty,
                    Collections.Fields.UNIT_PRICE to product.price
                )
            }
            val total = currentCart.entries.sumOf { (productId, qty) ->
                val product = currentProducts.find { it._id == productId }
                (product?.price ?: 0L) * qty
            }
            val orderData = mapOf(
                Collections.Fields.ITEMS to lineItems,
                Collections.Fields.TOTAL to total,
                Collections.Fields.STATUS to OrderStatus.OPEN.name,
                Collections.Fields.TIMESTAMP to System.currentTimeMillis(),
                Collections.Fields.TERMINAL_ID to getTerminalId(),
                Collections.Fields.DELETED to false,
                Collections.Fields.SYNC_SOURCE to "local",
                Collections.Fields.LAST_SYNCED_AT to 0L
            )

            val wroteToFirestore = tryFirestoreWrite("submitOrder(#${orderId.take(8)})") {
                firestore.collection(Collections.ORDERS)
                    .document(orderId).set(orderData).await()
                // Decrement inventory
                for ((productId, qty) in currentCart) {
                    val invSnapshot = firestore.collection(Collections.INVENTORY)
                        .whereEqualTo(Collections.Fields.PRODUCT_ID, productId)
                        .get().await()
                    val invDoc = invSnapshot.documents.firstOrNull() ?: continue
                    val currentQty = (invDoc.getLong(Collections.Fields.QUANTITY) ?: 0L).toInt()
                    invDoc.reference.update(
                        Collections.Fields.QUANTITY, (currentQty - qty).coerceAtLeast(0)
                    ).await()
                }
            }
            if (!wroteToFirestore) {
                tryDittoFallback("submitOrder(#${orderId.take(8)})") {
                    ditto.store.execute(
                        "INSERT INTO ${Collections.ORDERS} DOCUMENTS (:doc) ON ID CONFLICT DO NOTHING",
                        mapOf("doc" to orderData + (Collections.Fields.ID to orderId))
                    )
                }
            }

            _cart.value = emptyMap()
            _orderPlaced.emit(orderId.take(8))
            statusViewModel.refreshDittoCounts()
        }
    }

    private suspend fun tryFirestoreWrite(op: String, block: suspend () -> Unit): Boolean {
        return try {
            block()
            Log.i(TAG, "Firestore write OK: $op")
            statusViewModel.addLogEntry("Firestore", "write", op)
            true
        } catch (e: Throwable) {
            Log.w(TAG, "Firestore write FAILED: $op -- ${e.message}")
            statusViewModel.addLogEntry("Firestore", "error", "$op: ${e.message}")
            false
        }
    }

    private suspend fun tryDittoFallback(op: String, block: suspend () -> Unit) {
        try {
            block()
            Log.i(TAG, "Ditto fallback OK: $op")
            statusViewModel.addLogEntry("Ditto", "fallback", op)
        } catch (e: Throwable) {
            Log.e(TAG, "Ditto fallback FAILED: $op -- ${e.message}")
            statusViewModel.addLogEntry("Ditto", "error", "$op: ${e.message}")
        }
    }

    private fun getTerminalId(): String = try {
        ditto.presence.graph.localPeer.deviceName
    } catch (_: Exception) {
        "unknown"
    }

    override fun onCleared() {
        productObserver?.close()
        inventoryObserver?.close()
        super.onCleared()
    }
}
