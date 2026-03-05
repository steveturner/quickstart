package live.ditto.pos.fsditto.data.seed

import live.ditto.Ditto
import live.ditto.pos.fsditto.data.model.Collections

class DittoSeeder(private val ditto: Ditto) {

    suspend fun seedIfEmpty() {
        val result = ditto.store.execute(
            "SELECT * FROM ${Collections.PRODUCTS} LIMIT 1"
        )
        if (result.items.isNotEmpty()) return

        for (product in SeedData.products) {
            val doc = mapOf(
                Collections.Fields.ID to product._id,
                Collections.Fields.NAME to product.name,
                Collections.Fields.CATEGORY to product.category,
                Collections.Fields.PRICE to product.price,
                Collections.Fields.IMAGE_URL to product.imageUrl,
                Collections.Fields.DELETED to product.deleted,
                Collections.Fields.SYNC_SOURCE to product.syncSource,
                Collections.Fields.LAST_SYNCED_AT to product.lastSyncedAt
            )
            ditto.store.execute(
                "INSERT INTO ${Collections.PRODUCTS} DOCUMENTS (:doc) ON ID CONFLICT DO NOTHING",
                mapOf("doc" to doc)
            )
        }

        for (inv in SeedData.inventoryItems) {
            val doc = mapOf(
                Collections.Fields.ID to inv._id,
                Collections.Fields.PRODUCT_ID to inv.productId,
                Collections.Fields.QUANTITY to inv.quantity,
                Collections.Fields.LAST_MODIFIED_BY to inv.lastModifiedBy,
                Collections.Fields.DELETED to inv.deleted,
                Collections.Fields.SYNC_SOURCE to inv.syncSource,
                Collections.Fields.LAST_SYNCED_AT to inv.lastSyncedAt
            )
            ditto.store.execute(
                "INSERT INTO ${Collections.INVENTORY} DOCUMENTS (:doc) ON ID CONFLICT DO NOTHING",
                mapOf("doc" to doc)
            )
        }
    }
}
