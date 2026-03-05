package live.ditto.pos.fsditto.data.model

data class Order(
    val _id: String = "",
    val items: List<OrderLineItem> = emptyList(),
    val total: Long = 0L,
    val status: String = "OPEN",
    val timestamp: Long = 0L,
    val terminalId: String = "",
    val deleted: Boolean = false,
    val syncSource: String = "local",
    val lastSyncedAt: Long = 0L
)
