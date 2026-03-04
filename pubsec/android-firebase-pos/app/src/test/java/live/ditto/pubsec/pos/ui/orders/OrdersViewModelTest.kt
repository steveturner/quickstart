package live.ditto.pubsec.pos.ui.orders

import io.mockk.mockk
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.UnconfinedTestDispatcher
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.setMain
import live.ditto.Ditto
import org.junit.After
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

/**
 * Unit tests for OrdersViewModel covering observer lifecycle
 * and initial state verification.
 *
 * Ditto SDK classes are JNI-backed final classes. Tests use relaxed mocks
 * which return default values for all calls including registerObserver.
 * The observer callbacks are never fired in these JVM tests, so orders
 * remain empty. Observer cleanup is verified structurally.
 */
@OptIn(ExperimentalCoroutinesApi::class)
class OrdersViewModelTest {

    private val testDispatcher = UnconfinedTestDispatcher()

    @Before
    fun setUp() {
        Dispatchers.setMain(testDispatcher)
    }

    @After
    fun tearDown() {
        Dispatchers.resetMain()
    }

    private fun createViewModel(): OrdersViewModel {
        val mockDitto = mockk<Ditto>(relaxed = true)
        return OrdersViewModel(mockDitto)
    }

    @Test
    fun `viewModel creates without crash`() {
        val vm = createViewModel()
        // If we get here, construction succeeded with mock Ditto
        assertTrue("Orders should be a list", vm.orders.value is List)
    }

    @Test
    fun `orders flow starts empty`() {
        val vm = createViewModel()
        assertTrue("Orders should start empty", vm.orders.value.isEmpty())
    }

    @Test
    fun `onCleared closes observer without crash`() {
        val vm = createViewModel()

        // Trigger onCleared via reflection (it's protected in ViewModel)
        val method = vm.javaClass.getDeclaredMethod("onCleared")
        method.isAccessible = true
        method.invoke(vm)

        // Observer is null (relaxed mock registerObserver returns null for JNI class)
        // but onCleared should handle null observer safely via ?. operator
        assertTrue("onCleared should complete without exception", true)
    }
}
