package live.ditto.pubsec.pos

import live.ditto.pubsec.pos.data.model.InventoryItem
import live.ditto.pubsec.pos.data.model.Order
import live.ditto.pubsec.pos.data.model.Product
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class SoftDeleteConventionTest {

    @Test
    fun product_deletedDefaultsFalse() {
        val product = Product()
        assertFalse(product.deleted)
    }

    @Test
    fun order_deletedDefaultsFalse() {
        val order = Order()
        assertFalse(order.deleted)
    }

    @Test
    fun inventoryItem_deletedDefaultsFalse() {
        val item = InventoryItem()
        assertFalse(item.deleted)
    }

    @Test
    fun product_deletedProperty_isBooleanType() {
        val product = Product()
        assertTrue(product.deleted is Boolean)
    }

    @Test
    fun order_deletedProperty_isBooleanType() {
        val order = Order()
        assertTrue(order.deleted is Boolean)
    }

    @Test
    fun inventoryItem_deletedProperty_isBooleanType() {
        val item = InventoryItem()
        assertTrue(item.deleted is Boolean)
    }

    @Test
    fun softDelete_viaDataClassCopy() {
        val product = Product(_id = "p1", name = "Espresso")
        val deleted = product.copy(deleted = true)
        assertTrue(deleted.deleted)
        assertFalse(product.deleted)
        assertEquals("p1", deleted._id)
    }

    @Test
    fun softDelete_order_viaDataClassCopy() {
        val order = Order(_id = "o1")
        val deleted = order.copy(deleted = true)
        assertTrue(deleted.deleted)
        assertFalse(order.deleted)
    }

    @Test
    fun softDelete_inventoryItem_viaDataClassCopy() {
        val item = InventoryItem(_id = "i1")
        val deleted = item.copy(deleted = true)
        assertTrue(deleted.deleted)
        assertFalse(item.deleted)
    }
}
