package live.ditto.pubsec.pos.sync

import io.mockk.mockk
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.UnconfinedTestDispatcher
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

/**
 * Unit tests for SyncBridgeManager.
 * Ditto and Firebase SDK classes are JNI-backed and final, so we test
 * the bridge's internal behavior (retry logic, connection gating) using
 * the injectable dispatcher and internal helper methods.
 */
@OptIn(ExperimentalCoroutinesApi::class)
class SyncBridgeManagerTest {

    @Test
    fun `retryWithBackoff succeeds on first attempt`() = runTest {
        val bridge = SyncBridgeManager(
            ditto = mockk(relaxed = true),
            firestore = mockk(relaxed = true),
            dispatcher = UnconfinedTestDispatcher(testScheduler)
        )

        var callCount = 0
        val result = bridge.retryWithBackoff("test-op") {
            callCount++
        }

        assertTrue("retryWithBackoff should return true on success", result)
        assertTrue("Block should have been called exactly once", callCount == 1)
    }

    @Test
    fun `retryWithBackoff retries on failure and succeeds on third attempt`() = runTest {
        val bridge = SyncBridgeManager(
            ditto = mockk(relaxed = true),
            firestore = mockk(relaxed = true),
            dispatcher = UnconfinedTestDispatcher(testScheduler)
        )

        var callCount = 0
        val result = bridge.retryWithBackoff("test-op") {
            callCount++
            if (callCount < 3) throw RuntimeException("transient failure #$callCount")
        }

        assertTrue("retryWithBackoff should return true after eventual success", result)
        assertTrue("Block should have been called 3 times", callCount == 3)
    }

    @Test
    fun `retryWithBackoff drops write after max retries exhausted`() = runTest {
        val bridge = SyncBridgeManager(
            ditto = mockk(relaxed = true),
            firestore = mockk(relaxed = true),
            dispatcher = UnconfinedTestDispatcher(testScheduler)
        )

        var callCount = 0
        val result = bridge.retryWithBackoff("test-op") {
            callCount++
            throw RuntimeException("permanent failure")
        }

        assertFalse("retryWithBackoff should return false after max retries", result)
        assertTrue("Block should have been called exactly 3 times (MAX_RETRIES)", callCount == 3)
    }

    @Test
    fun `isFirebaseConnected defaults to false`() {
        val bridge = SyncBridgeManager(
            ditto = mockk(relaxed = true),
            firestore = mockk(relaxed = true),
            dispatcher = UnconfinedTestDispatcher()
        )

        assertFalse(
            "isFirebaseConnected should default to false (disconnected)",
            bridge.isFirebaseConnected.value
        )
    }
}
