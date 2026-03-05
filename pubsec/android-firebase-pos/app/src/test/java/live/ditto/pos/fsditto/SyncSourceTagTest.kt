package live.ditto.pos.fsditto

import live.ditto.pos.fsditto.data.model.InventoryItem
import live.ditto.pos.fsditto.data.model.Order
import live.ditto.pos.fsditto.data.model.Product
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class SyncSourceTagTest {

    @Test
    fun product_syncSourceDefaultsToLocal() {
        val product = Product()
        assertEquals("local", product.syncSource)
    }

    @Test
    fun order_syncSourceDefaultsToLocal() {
        val order = Order()
        assertEquals("local", order.syncSource)
    }

    @Test
    fun inventoryItem_syncSourceDefaultsToLocal() {
        val item = InventoryItem()
        assertEquals("local", item.syncSource)
    }

    @Test
    fun product_lastSyncedAtDefaultsToZero() {
        val product = Product()
        assertEquals(0L, product.lastSyncedAt)
    }

    @Test
    fun order_lastSyncedAtDefaultsToZero() {
        val order = Order()
        assertEquals(0L, order.lastSyncedAt)
    }

    @Test
    fun inventoryItem_lastSyncedAtDefaultsToZero() {
        val item = InventoryItem()
        assertEquals(0L, item.lastSyncedAt)
    }

    @Test
    fun syncSource_isStringType_forAllModels() {
        assertTrue(Product().syncSource is String)
        assertTrue(Order().syncSource is String)
        assertTrue(InventoryItem().syncSource is String)
    }

    @Test
    fun lastSyncedAt_isLongType_forAllModels() {
        assertTrue(Product().lastSyncedAt is Long)
        assertTrue(Order().lastSyncedAt is Long)
        assertTrue(InventoryItem().lastSyncedAt is Long)
    }

    @Test
    fun syncSource_canBeSetToFirestore_viaCopy() {
        val product = Product().copy(syncSource = "firestore")
        assertEquals("firestore", product.syncSource)
    }

    @Test
    fun syncSource_canBeSetToDitto_viaCopy() {
        val order = Order().copy(syncSource = "ditto")
        assertEquals("ditto", order.syncSource)
    }

    @Test
    fun syncSource_canBeSetToFirestore_forInventoryItem_viaCopy() {
        val item = InventoryItem().copy(syncSource = "firestore")
        assertEquals("firestore", item.syncSource)
    }

    @Test
    fun lastSyncedAt_canBeSetToEpochMillis() {
        val epochMillis = 1700000000000L
        val product = Product().copy(lastSyncedAt = epochMillis)
        val order = Order().copy(lastSyncedAt = epochMillis)
        val item = InventoryItem().copy(lastSyncedAt = epochMillis)
        assertEquals(epochMillis, product.lastSyncedAt)
        assertEquals(epochMillis, order.lastSyncedAt)
        assertEquals(epochMillis, item.lastSyncedAt)
    }
}
