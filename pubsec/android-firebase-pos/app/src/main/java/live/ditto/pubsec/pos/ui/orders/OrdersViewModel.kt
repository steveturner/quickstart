package live.ditto.pubsec.pos.ui.orders

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import live.ditto.Ditto
import live.ditto.pubsec.pos.data.model.Order

class OrdersViewModel(private val ditto: Ditto) : ViewModel() {
    // Stub -- Plan 03 will implement full observer and StateFlow
    private val _orders = MutableStateFlow<List<Order>>(emptyList())
    val orders: StateFlow<List<Order>> = _orders.asStateFlow()
}
