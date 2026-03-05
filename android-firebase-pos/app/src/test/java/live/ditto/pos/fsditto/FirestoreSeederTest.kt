package live.ditto.pos.fsditto

import com.google.android.gms.tasks.Task
import com.google.firebase.firestore.CollectionReference
import com.google.firebase.firestore.DocumentReference
import com.google.firebase.firestore.FirebaseFirestore
import com.google.firebase.firestore.Query
import com.google.firebase.firestore.QuerySnapshot
import com.google.firebase.firestore.WriteBatch
import io.mockk.coEvery
import io.mockk.coJustRun
import io.mockk.every
import io.mockk.mockk
import io.mockk.mockkStatic
import io.mockk.verify
import kotlinx.coroutines.tasks.await
import kotlinx.coroutines.test.runTest
import live.ditto.pos.fsditto.data.model.Collections
import live.ditto.pos.fsditto.data.seed.FirestoreSeeder
import live.ditto.pos.fsditto.data.seed.SeedData
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class FirestoreSeederTest {

    private lateinit var mockDb: FirebaseFirestore
    private lateinit var mockProductsCollection: CollectionReference
    private lateinit var mockInventoryCollection: CollectionReference
    private lateinit var mockQuery: Query
    private lateinit var mockGetTask: Task<QuerySnapshot>
    private lateinit var mockSnapshot: QuerySnapshot
    private lateinit var mockBatch: WriteBatch
    private lateinit var mockCommitTask: Task<Void>
    private lateinit var seeder: FirestoreSeeder

    @Before
    fun setUp() {
        mockkStatic("kotlinx.coroutines.tasks.TasksKt")

        mockDb = mockk()
        mockProductsCollection = mockk()
        mockInventoryCollection = mockk()
        mockQuery = mockk()
        mockGetTask = mockk()
        mockSnapshot = mockk()
        mockBatch = mockk(relaxed = true)
        mockCommitTask = mockk()

        // Wire collection references
        every { mockDb.collection(Collections.PRODUCTS) } returns mockProductsCollection
        every { mockDb.collection(Collections.INVENTORY) } returns mockInventoryCollection

        // Wire limit/get chain for empty check
        every { mockProductsCollection.limit(1) } returns mockQuery
        every { mockQuery.get() } returns mockGetTask

        // Wire batch
        every { mockDb.batch() } returns mockBatch
        every { mockBatch.commit() } returns mockCommitTask

        // Wire document references for products and inventory
        every { mockProductsCollection.document(any()) } returns mockk(relaxed = true)
        every { mockInventoryCollection.document(any()) } returns mockk(relaxed = true)

        // Wire await on commit task — coJustRun used because Task<Void>.await() returns Unit in coroutines
        coJustRun { mockCommitTask.await() }

        seeder = FirestoreSeeder(mockDb)
    }

    @Test
    fun `seedIfEmpty writes batch when collection is empty`() = runTest {
        every { mockSnapshot.isEmpty } returns true
        coEvery { mockGetTask.await() } returns mockSnapshot

        seeder.seedIfEmpty()

        val expectedSetCount = SeedData.products.size + SeedData.inventoryItems.size
        verify(exactly = expectedSetCount) { mockBatch.set(any<DocumentReference>(), any()) }
        verify(exactly = 1) { mockBatch.commit() }
    }

    @Test
    fun `seedIfEmpty skips write when collection is not empty`() = runTest {
        every { mockSnapshot.isEmpty } returns false
        coEvery { mockGetTask.await() } returns mockSnapshot

        seeder.seedIfEmpty()

        verify(exactly = 0) { mockBatch.set(any<DocumentReference>(), any()) }
        verify(exactly = 0) { mockBatch.commit() }
    }

    @Test
    fun `seedIfEmpty uses product id as document ID`() = runTest {
        every { mockSnapshot.isEmpty } returns true
        coEvery { mockGetTask.await() } returns mockSnapshot

        val capturedProductDocIds = mutableListOf<String>()
        every { mockProductsCollection.document(capture(capturedProductDocIds)) } returns mockk(relaxed = true)

        seeder.seedIfEmpty()

        val expectedProductIds = SeedData.products.map { it._id }
        assertEquals(expectedProductIds.size, capturedProductDocIds.size)
        expectedProductIds.forEach { expectedId ->
            assertTrue(
                "Expected product _id '$expectedId' to be used as document ID",
                capturedProductDocIds.contains(expectedId)
            )
        }
    }

    @Test
    fun `seedIfEmpty uses item id as inventory document ID`() = runTest {
        every { mockSnapshot.isEmpty } returns true
        coEvery { mockGetTask.await() } returns mockSnapshot

        val capturedInventoryDocIds = mutableListOf<String>()
        every { mockInventoryCollection.document(capture(capturedInventoryDocIds)) } returns mockk(relaxed = true)

        seeder.seedIfEmpty()

        val expectedItemIds = SeedData.inventoryItems.map { it._id }
        assertEquals(expectedItemIds.size, capturedInventoryDocIds.size)
        expectedItemIds.forEach { expectedId ->
            assertTrue(
                "Expected inventory item _id '$expectedId' to be used as document ID",
                capturedInventoryDocIds.contains(expectedId)
            )
        }
    }

    @Test
    fun `seedIfEmpty uses Collections constants for collection names`() = runTest {
        every { mockSnapshot.isEmpty } returns true
        coEvery { mockGetTask.await() } returns mockSnapshot

        seeder.seedIfEmpty()

        verify { mockDb.collection(Collections.PRODUCTS) }
        verify { mockDb.collection(Collections.INVENTORY) }
    }
}
