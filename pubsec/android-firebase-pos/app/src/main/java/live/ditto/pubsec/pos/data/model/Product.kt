package live.ditto.pubsec.pos.data.model

data class Product(
    val _id: String = "",
    val name: String = "",
    val category: String = "",
    val price: Long = 0L,
    val imageUrl: String = "",
    val deleted: Boolean = false,
    val syncSource: String = "local",
    val lastSyncedAt: Long = 0L
)
