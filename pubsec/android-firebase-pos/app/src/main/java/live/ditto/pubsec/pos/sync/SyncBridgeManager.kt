package live.ditto.pubsec.pos.sync

import android.util.Log
import com.google.firebase.Firebase
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.ValueEventListener
import com.google.firebase.database.database
import com.google.firebase.firestore.FirebaseFirestore
import com.google.firebase.firestore.ListenerRegistration
import com.google.firebase.firestore.SetOptions
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.tasks.await
import live.ditto.Ditto
import live.ditto.DittoStoreObserver
import live.ditto.pubsec.pos.data.model.Collections

/**
 * Bidirectional sync bridge between Firestore (cloud) and Ditto (local store).
 *
 * Syncs all three collections (products, orders, inventory) in both directions:
 * - Firestore -> Ditto: SnapshotListeners write into Ditto via DQL INSERT ... ON ID CONFLICT DO UPDATE_LOCAL_DIFF
 * - Ditto -> Firestore: registerObserver callbacks write to Firestore via set() with SetOptions.merge()
 *
 * ChangeGuard prevents infinite sync loops:
 * - Firestore side: skips docs with hasPendingWrites=true and syncSource="firestore"
 * - Ditto side: skips docs with syncSource="firestore" (only pushes locally-originated docs)
 *
 * Connection detection via Firebase RTDB .info/connected pauses Firestore writes when disconnected.
 * Failed Firestore writes retry up to 3 times with exponential backoff (1s, 2s, 4s).
 */
class SyncBridgeManager(
    private val ditto: Ditto,
    private val firestore: FirebaseFirestore,
    private val dispatcher: CoroutineDispatcher = Dispatchers.IO
) {
    companion object {
        private const val TAG = "SyncBridge"
        private const val MAX_RETRIES = 3
        private const val INITIAL_RETRY_DELAY_MS = 1000L // 1s, 2s, 4s
    }

    private val bridgeScope = CoroutineScope(SupervisorJob() + dispatcher)

    private val _isFirebaseConnected = MutableStateFlow(false)
    val isFirebaseConnected: StateFlow<Boolean> = _isFirebaseConnected.asStateFlow()

    private val listeners = mutableListOf<ListenerRegistration>()
    private val observers = mutableListOf<DittoStoreObserver>()
    private val subscriptions = mutableListOf<Any>()

    /**
     * Retries [block] up to [MAX_RETRIES] times with exponential backoff.
     * Delays: 1s after 1st failure, 2s after 2nd, 4s after 3rd, then gives up.
     * Returns true if succeeded, false if all retries exhausted.
     */
    internal suspend fun retryWithBackoff(
        description: String,
        block: suspend () -> Unit
    ): Boolean {
        var delayMs = INITIAL_RETRY_DELAY_MS
        for (attempt in 1..MAX_RETRIES) {
            try {
                block()
                return true
            } catch (e: Exception) {
                Log.w(TAG, "$description failed (attempt $attempt/$MAX_RETRIES): ${e.message}")
                if (attempt < MAX_RETRIES) {
                    delay(delayMs)
                    delayMs *= 2
                }
            }
        }
        Log.e(TAG, "$description failed after $MAX_RETRIES retries -- dropping write")
        return false
    }

    /**
     * Starts the sync bridge:
     * 1. Registers Ditto subscriptions for all 3 collections
     * 2. Starts Ditto sync
     * 3. Sets up Firestore -> Ditto listeners
     * 4. Sets up Ditto -> Firestore observers
     * 5. Sets up Firebase connection detection
     */
    fun start() {
        setupSubscriptions()
        ditto.startSync()
        setupFirestoreToDittoListeners()
        setupDittoToFirestoreObservers()
        setupConnectionDetection()
    }

    /**
     * Stops the sync bridge, removing all listeners and observers.
     */
    fun stop() {
        listeners.forEach { it.remove() }
        listeners.clear()
        observers.forEach { it.close() }
        observers.clear()
        bridgeScope.cancel()
    }

    private fun setupSubscriptions() {
        val collections = listOf(Collections.PRODUCTS, Collections.ORDERS, Collections.INVENTORY)
        for (collection in collections) {
            val sub = ditto.sync.registerSubscription("SELECT * FROM $collection")
            subscriptions += sub
        }
    }

    private fun setupFirestoreToDittoListeners() {
        addFirestoreToDittoListener(Collections.PRODUCTS)
        addFirestoreToDittoListener(Collections.ORDERS)
        addFirestoreToDittoListener(Collections.INVENTORY)
    }

    private fun addFirestoreToDittoListener(collectionName: String) {
        val listener = firestore.collection(collectionName)
            .addSnapshotListener { snapshots, error ->
                if (error != null) {
                    Log.e(TAG, "Firestore listener error for $collectionName: ${error.message}")
                    return@addSnapshotListener
                }
                if (snapshots == null) return@addSnapshotListener

                for (change in snapshots.documentChanges) {
                    val doc = change.document
                    // ChangeGuard 1: Skip docs with pending local writes
                    // (our Ditto->Firestore bridge wrote them)
                    if (doc.metadata.hasPendingWrites()) continue
                    // ChangeGuard 2: Skip docs already tagged as firestore-sourced
                    val syncSource = doc.getString(Collections.Fields.SYNC_SOURCE) ?: ""
                    if (syncSource == "firestore") continue

                    val data = doc.data.toMutableMap()
                    data[Collections.Fields.ID] = doc.id
                    data[Collections.Fields.SYNC_SOURCE] = "firestore"
                    data[Collections.Fields.LAST_SYNCED_AT] = System.currentTimeMillis()

                    // For inventory: strip quantity to avoid overwriting CRDT counter
                    if (collectionName == Collections.INVENTORY) {
                        data.remove(Collections.Fields.QUANTITY)
                    }

                    bridgeScope.launch {
                        try {
                            ditto.store.execute(
                                "INSERT INTO $collectionName DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE_LOCAL_DIFF",
                                mapOf("doc" to data)
                            )
                        } catch (e: Exception) {
                            Log.e(TAG, "Firestore->Ditto write failed for $collectionName: ${e.message}")
                        }
                    }
                }
            }
        listeners += listener
    }

    private fun setupDittoToFirestoreObservers() {
        addDittoToFirestoreObserver(Collections.PRODUCTS)
        addDittoToFirestoreObserver(Collections.ORDERS)
        addDittoToFirestoreObserver(Collections.INVENTORY)
    }

    private fun addDittoToFirestoreObserver(collectionName: String) {
        val observer = ditto.store.registerObserver(
            "SELECT * FROM $collectionName"
        ) { queryResult ->
            val items = queryResult.use { it.items.map { item -> item.value.toMutableMap() } }
            bridgeScope.launch {
                // SYNC-04: pause when Firebase is disconnected
                if (!_isFirebaseConnected.value) return@launch

                for (doc in items) {
                    val syncSource = doc[Collections.Fields.SYNC_SOURCE] as? String ?: ""
                    // Only push docs that originated locally (syncSource != "firestore")
                    if (syncSource == "firestore") continue

                    val id = doc[Collections.Fields.ID] as? String ?: continue
                    doc[Collections.Fields.SYNC_SOURCE] = "ditto"
                    doc[Collections.Fields.LAST_SYNCED_AT] = System.currentTimeMillis()
                    // Remove _id from payload -- Firestore uses doc path not _id field
                    val firestoreData = doc.toMutableMap()
                    firestoreData.remove(Collections.Fields.ID)

                    retryWithBackoff("Ditto->Firestore $collectionName/$id") {
                        firestore.collection(collectionName)
                            .document(id)
                            .set(firestoreData, SetOptions.merge())
                            .await()
                    }
                }
            }
        }
        observers += observer
    }

    private fun setupConnectionDetection() {
        val database = Firebase.database
        database.getReference("keepAlive").keepSynced(true)
        val connectedRef = database.getReference(".info/connected")
        connectedRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val connected = snapshot.getValue(Boolean::class.java) ?: false
                _isFirebaseConnected.value = connected
                if (connected) {
                    Log.i(TAG, "Firebase connected -- bridge active")
                } else {
                    Log.w(TAG, "Firebase disconnected -- bridge paused for Firestore writes")
                }
            }

            override fun onCancelled(error: DatabaseError) {
                Log.e(TAG, "RTDB listener cancelled: ${error.message}")
            }
        })
    }
}
