package live.ditto.pos.fsditto.data.model

data class OrderLineItem(
    val productId: String = "",
    val productName: String = "",
    val quantity: Int = 0,
    val unitPrice: Long = 0L
)
