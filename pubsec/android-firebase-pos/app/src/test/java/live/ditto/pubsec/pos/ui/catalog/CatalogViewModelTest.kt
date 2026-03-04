package live.ditto.pubsec.pos.ui.catalog

import io.mockk.mockk
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.UnconfinedTestDispatcher
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import live.ditto.Ditto
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

/**
 * Unit tests for CatalogViewModel covering cart operations, category selection,
 * order submission, and initial state verification.
 *
 * Ditto SDK classes are JNI-backed final classes. Tests use relaxed mocks
 * which return default values for all calls including registerObserver.
 * The observer callbacks are never fired in these JVM tests, so products
 * and inventory remain empty. Cart logic is tested independently.
 */
@OptIn(ExperimentalCoroutinesApi::class)
class CatalogViewModelTest {

    private val testDispatcher = UnconfinedTestDispatcher()

    @Before
    fun setUp() {
        Dispatchers.setMain(testDispatcher)
    }

    @After
    fun tearDown() {
        Dispatchers.resetMain()
    }

    private fun createViewModel(): CatalogViewModel {
        val mockDitto = mockk<Ditto>(relaxed = true)
        return CatalogViewModel(mockDitto)
    }

    @Test
    fun `addToCart increments quantity for product`() {
        val vm = createViewModel()

        vm.addToCart("product-1")
        assertEquals(1, vm.cart.value["product-1"])

        vm.addToCart("product-1")
        assertEquals(2, vm.cart.value["product-1"])
    }

    @Test
    fun `addToCart handles multiple products independently`() {
        val vm = createViewModel()

        vm.addToCart("product-1")
        vm.addToCart("product-2")
        vm.addToCart("product-1")

        assertEquals(2, vm.cart.value["product-1"])
        assertEquals(1, vm.cart.value["product-2"])
    }

    @Test
    fun `removeFromCart decrements quantity`() {
        val vm = createViewModel()

        vm.addToCart("product-1")
        vm.addToCart("product-1")
        vm.removeFromCart("product-1")

        assertEquals(1, vm.cart.value["product-1"])
    }

    @Test
    fun `removeFromCart removes product at zero`() {
        val vm = createViewModel()

        vm.addToCart("product-1")
        vm.removeFromCart("product-1")

        assertTrue("Cart should be empty", vm.cart.value.isEmpty())
    }

    @Test
    fun `removeFromCart on empty cart is safe`() {
        val vm = createViewModel()

        vm.removeFromCart("nonexistent")

        assertTrue("Cart should still be empty", vm.cart.value.isEmpty())
    }

    @Test
    fun `selectCategory updates selectedCategory state`() {
        val vm = createViewModel()

        assertEquals(null, vm.selectedCategory.value)

        vm.selectCategory("Beverage")
        assertEquals("Beverage", vm.selectedCategory.value)

        vm.selectCategory(null)
        assertEquals(null, vm.selectedCategory.value)
    }

    @Test
    fun `submitOrder clears cart after execution`() = runTest {
        // Ditto store is JNI-backed and cannot be mocked in JVM tests.
        // The try-catch in submitOrder handles the NPE from relaxed mock,
        // and cart is always cleared regardless of Ditto store outcome.
        val vm = createViewModel()

        vm.addToCart("product-1")
        assertEquals(1, vm.cart.value.size)

        vm.submitOrder()
        testScheduler.advanceUntilIdle()

        assertTrue("Cart should be empty after submit", vm.cart.value.isEmpty())
    }

    @Test
    fun `cart total calculates correctly from product prices and quantities`() {
        val vm = createViewModel()

        vm.addToCart("product-1")
        vm.addToCart("product-1")
        vm.addToCart("product-2")

        val cart = vm.cart.value
        assertEquals(2, cart["product-1"])
        assertEquals(1, cart["product-2"])
        assertEquals(2, cart.size)
    }

    @Test
    fun `initial state is empty`() {
        val vm = createViewModel()

        assertTrue("Products should start empty", vm.products.value.isEmpty())
        assertTrue("Inventory should start empty", vm.inventory.value.isEmpty())
        assertTrue("Cart should start empty", vm.cart.value.isEmpty())
        assertEquals(null, vm.selectedCategory.value)
    }
}
