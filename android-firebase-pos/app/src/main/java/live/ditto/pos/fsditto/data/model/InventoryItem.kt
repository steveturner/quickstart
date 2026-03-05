package live.ditto.pos.fsditto.data.model

data class InventoryItem(
    val _id: String = "",
    val productId: String = "",
    val quantity: Int = 0,
    val lastModifiedBy: String = "",
    val deleted: Boolean = false,
    val syncSource: String = "local",
    val lastSyncedAt: Long = 0L
)
