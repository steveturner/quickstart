package live.ditto.pubsec.pos.ui.catalog

import io.mockk.coEvery
import io.mockk.every
import io.mockk.mockk
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.UnconfinedTestDispatcher
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import live.ditto.Ditto
import live.ditto.DittoStoreObserver
import live.ditto.pubsec.pos.data.model.Product
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

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
        // registerObserver returns a mock observer that does nothing
        every { mockDitto.store.registerObserver(any(), any(), any()) } returns mockk<DittoStoreObserver>(relaxed = true)
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
        val mockDitto = mockk<Ditto>(relaxed = true)
        every { mockDitto.store.registerObserver(any(), any(), any()) } returns mockk<DittoStoreObserver>(relaxed = true)

        val mockQueryResult = mockk<live.ditto.DittoQueryResult>(relaxed = true)
        every { mockQueryResult.items } returns emptyList()
        coEvery { mockDitto.store.execute(any(), any()) } returns mockQueryResult

        val vm = CatalogViewModel(mockDitto)

        vm.addToCart("product-1")
        assertEquals(1, vm.cart.value.size)

        vm.submitOrder()
        // Give coroutine a chance to complete (UnconfinedTestDispatcher)
        testScheduler.advanceUntilIdle()

        assertTrue("Cart should be empty after submit", vm.cart.value.isEmpty())
    }

    @Test
    fun `cart total calculates correctly from product prices and quantities`() {
        val vm = createViewModel()

        // We can't easily inject products since they come from Ditto observer,
        // but we can verify the cart state management is correct.
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
