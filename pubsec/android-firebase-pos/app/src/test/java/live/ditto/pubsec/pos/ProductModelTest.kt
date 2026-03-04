package live.ditto.pubsec.pos

import live.ditto.pubsec.pos.data.model.Product
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class ProductModelTest {

    @Test
    fun defaultConstructor_allFieldsHaveDefaults() {
        val product = Product()
        assertEquals("", product._id)
        assertEquals("", product.name)
        assertEquals("", product.category)
        assertEquals(0L, product.price)
        assertEquals("", product.imageUrl)
        assertFalse(product.deleted)
        assertEquals("local", product.syncSource)
        assertEquals(0L, product.lastSyncedAt)
    }

    @Test
    fun idField_isStringType() {
        val product = Product(_id = "abc-123")
        assertTrue(product._id is String)
        assertEquals("abc-123", product._id)
    }

    @Test
    fun price_isLongType() {
        // $4.99 stored as 499 cents
        val product = Product(price = 499L)
        assertTrue(product.price is Long)
        assertEquals(499L, product.price)
    }

    @Test
    fun explicitValues_setCorrectly() {
        val product = Product(
            _id = "prod-001",
            name = "Espresso",
            category = "Beverage",
            price = 299L,
            imageUrl = "https://example.com/espresso.jpg",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 1700000000000L
        )
        assertEquals("prod-001", product._id)
        assertEquals("Espresso", product.name)
        assertEquals("Beverage", product.category)
        assertEquals(299L, product.price)
        assertEquals("https://example.com/espresso.jpg", product.imageUrl)
        assertFalse(product.deleted)
        assertEquals("firestore", product.syncSource)
        assertEquals(1700000000000L, product.lastSyncedAt)
    }

    @Test
    fun copy_roundTrip() {
        val original = Product(_id = "p1", name = "Latte", price = 499L)
        val copy = original.copy(name = "Cappuccino", price = 449L)
        assertEquals("p1", copy._id)
        assertEquals("Cappuccino", copy.name)
        assertEquals(449L, copy.price)
        // original unchanged
        assertEquals("Latte", original.name)
    }
}
