package live.ditto.pubsec.pos.data.model

object Collections {
    const val PRODUCTS = "products"
    const val ORDERS = "orders"
    const val INVENTORY = "inventory"

    object Fields {
        const val ID = "_id"
        const val DELETED = "deleted"
        const val SYNC_SOURCE = "syncSource"
        const val LAST_SYNCED_AT = "lastSyncedAt"
        const val NAME = "name"
        const val CATEGORY = "category"
        const val PRICE = "price"
        const val IMAGE_URL = "imageUrl"
        const val ITEMS = "items"
        const val TOTAL = "total"
        const val STATUS = "status"
        const val TIMESTAMP = "timestamp"
        const val TERMINAL_ID = "terminalId"
        const val PRODUCT_ID = "productId"
        const val PRODUCT_NAME = "productName"
        const val QUANTITY = "quantity"
        const val UNIT_PRICE = "unitPrice"
        const val LAST_MODIFIED_BY = "lastModifiedBy"
    }
}
