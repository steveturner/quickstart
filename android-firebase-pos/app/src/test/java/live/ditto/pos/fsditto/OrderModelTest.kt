package live.ditto.pos.fsditto

import live.ditto.pos.fsditto.data.model.Order
import live.ditto.pos.fsditto.data.model.OrderLineItem
import live.ditto.pos.fsditto.data.model.OrderStatus
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class OrderModelTest {

    @Test
    fun defaultConstructor_emptyItemsAndOpenStatus() {
        val order = Order()
        assertEquals("", order._id)
        assertTrue(order.items.isEmpty())
        assertEquals(0L, order.total)
        assertEquals("OPEN", order.status)
        assertEquals(0L, order.timestamp)
        assertEquals("", order.terminalId)
        assertFalse(order.deleted)
        assertEquals("local", order.syncSource)
        assertEquals(0L, order.lastSyncedAt)
    }

    @Test
    fun items_isListOfOrderLineItem() {
        val item1 = OrderLineItem(productId = "p1", productName = "Espresso", quantity = 2, unitPrice = 299L)
        val item2 = OrderLineItem(productId = "p2", productName = "Croissant", quantity = 1, unitPrice = 349L)
        val order = Order(items = listOf(item1, item2), total = 947L)

        assertTrue(order.items is List<*>)
        assertEquals(2, order.items.size)
        assertEquals("Espresso", order.items[0].productName)
        assertEquals("Croissant", order.items[1].productName)
        assertEquals(947L, order.total)
    }

    @Test
    fun total_isLongType() {
        val order = Order(total = 1250L)
        assertTrue(order.total is Long)
        assertEquals(1250L, order.total)
    }

    @Test
    fun status_defaultsToOpen() {
        val order = Order()
        assertEquals("OPEN", order.status)
        assertEquals(OrderStatus.OPEN.name, order.status)
    }

    @Test
    fun orderStatus_hasExactlyTwoValues() {
        val values = OrderStatus.values()
        assertEquals(2, values.size)
        assertEquals("OPEN", values[0].name)
        assertEquals("FULFILLED", values[1].name)
    }

    @Test
    fun orderStatus_enumValueOf_works() {
        assertEquals(OrderStatus.OPEN, enumValueOf<OrderStatus>("OPEN"))
        assertEquals(OrderStatus.FULFILLED, enumValueOf<OrderStatus>("FULFILLED"))
    }

    @Test
    fun status_canBeSetToFulfilled() {
        val order = Order(status = OrderStatus.FULFILLED.name)
        assertEquals("FULFILLED", order.status)
    }
}
