package live.ditto.pubsec.pos

import live.ditto.pubsec.pos.data.seed.SeedData
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class SeedDataTest {

    private val products = SeedData.products
    private val inventoryItems = SeedData.inventoryItems

    @Test
    fun `products has expected count`() {
        assertTrue(
            "Expected 8-10 products, got ${products.size}",
            products.size in 8..10
        )
    }

    @Test
    fun `all products have non-empty id`() {
        products.forEach { product ->
            assertNotEquals("Product '${product.name}' has empty _id", "", product._id)
        }
    }

    @Test
    fun `all product ids are unique`() {
        val ids = products.map { it._id }
        assertEquals(
            "Duplicate product _id values found",
            products.size,
            ids.distinct().size
        )
    }

    @Test
    fun `all prices are positive`() {
        products.forEach { product ->
            assertTrue("Product '${product.name}' has non-positive price: ${product.price}", product.price > 0)
        }
    }

    @Test
    fun `all products have syncSource firestore`() {
        products.forEach { product ->
            assertEquals("Product '${product.name}' syncSource should be firestore", "firestore", product.syncSource)
        }
    }

    @Test
    fun `all products have deleted false`() {
        products.forEach { product ->
            assertFalse("Product '${product.name}' should have deleted=false", product.deleted)
        }
    }

    @Test
    fun `categories are valid`() {
        val validCategories = setOf("Beverage", "Food", "Merchandise")
        products.forEach { product ->
            assertTrue(
                "Product '${product.name}' has invalid category '${product.category}'",
                product.category in validCategories
            )
        }
    }

    @Test
    fun `inventoryItems count matches products`() {
        assertEquals(
            "inventoryItems count should match products count",
            products.size,
            inventoryItems.size
        )
    }

    @Test
    fun `each inventoryItem references a valid product`() {
        val productIds = products.map { it._id }.toSet()
        inventoryItems.forEach { item ->
            assertTrue(
                "InventoryItem ${item._id} has productId '${item.productId}' not in products",
                item.productId in productIds
            )
        }
    }

    @Test
    fun `all inventory ids are unique`() {
        val ids = inventoryItems.map { it._id }
        assertEquals(
            "Duplicate inventory _id values found",
            inventoryItems.size,
            ids.distinct().size
        )
    }

    @Test
    fun `all inventoryItems have syncSource firestore`() {
        inventoryItems.forEach { item ->
            assertEquals("InventoryItem ${item._id} syncSource should be firestore", "firestore", item.syncSource)
        }
    }

    @Test
    fun `all inventoryItems have positive quantity`() {
        inventoryItems.forEach { item ->
            assertTrue("InventoryItem ${item._id} has non-positive quantity: ${item.quantity}", item.quantity > 0)
        }
    }
}
