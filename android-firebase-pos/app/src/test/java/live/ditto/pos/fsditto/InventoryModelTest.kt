package live.ditto.pos.fsditto

import live.ditto.pos.fsditto.data.model.InventoryItem
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class InventoryModelTest {

    @Test
    fun defaultConstructor_quantityZeroAndDefaults() {
        val item = InventoryItem()
        assertEquals("", item._id)
        assertEquals("", item.productId)
        assertEquals(0, item.quantity)
        assertEquals("", item.lastModifiedBy)
        assertFalse(item.deleted)
        assertEquals("local", item.syncSource)
        assertEquals(0L, item.lastSyncedAt)
    }

    @Test
    fun quantity_isIntType() {
        val item = InventoryItem(quantity = 42)
        assertTrue(item.quantity is Int)
        assertEquals(42, item.quantity)
    }

    @Test
    fun productId_referencesProductIdShape() {
        val productId = "prod-abc-123"
        val item = InventoryItem(productId = productId)
        assertTrue(item.productId is String)
        assertEquals(productId, item.productId)
    }

    @Test
    fun lastModifiedBy_isString() {
        val item = InventoryItem(lastModifiedBy = "terminal-01")
        assertTrue(item.lastModifiedBy is String)
        assertEquals("terminal-01", item.lastModifiedBy)
    }

    @Test
    fun explicitValues_setCorrectly() {
        val item = InventoryItem(
            _id = "inv-001",
            productId = "prod-001",
            quantity = 10,
            lastModifiedBy = "terminal-02",
            deleted = false,
            syncSource = "ditto",
            lastSyncedAt = 1700000000000L
        )
        assertEquals("inv-001", item._id)
        assertEquals("prod-001", item.productId)
        assertEquals(10, item.quantity)
        assertEquals("terminal-02", item.lastModifiedBy)
        assertFalse(item.deleted)
        assertEquals("ditto", item.syncSource)
        assertEquals(1700000000000L, item.lastSyncedAt)
    }
}
