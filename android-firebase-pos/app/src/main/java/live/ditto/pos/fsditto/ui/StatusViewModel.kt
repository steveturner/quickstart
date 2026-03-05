package live.ditto.pos.fsditto.ui

import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.google.firebase.firestore.FirebaseFirestore
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.tasks.await
import live.ditto.Ditto
import live.ditto.DittoPresenceObserver
import live.ditto.pos.fsditto.sync.SyncBridgeManager

data class PeerInfo(
    val deviceName: String,
    val connectionTypes: List<String>,
    val isLocal: Boolean = false
)

data class SyncLogEntry(
    val timestamp: Long,
    val target: String,   // "Firestore" or "Ditto"
    val operation: String, // "write", "fallback", "error"
    val detail: String
)

class StatusViewModel(
    private val ditto: Ditto,
    private val firestore: FirebaseFirestore,
    private val syncBridgeManager: SyncBridgeManager
) : ViewModel() {

    companion object {
        private const val TAG = "StatusVM"
        private const val MAX_LOG_ENTRIES = 50
    }

    private val _peers = MutableStateFlow<List<PeerInfo>>(emptyList())
    val peers: StateFlow<List<PeerInfo>> = _peers.asStateFlow()

    val isFirebaseConnected: StateFlow<Boolean> = syncBridgeManager.isFirebaseConnected

    private val _firestoreEnabled = MutableStateFlow(true)
    val firestoreEnabled: StateFlow<Boolean> = _firestoreEnabled.asStateFlow()

    private val _syncLog = MutableStateFlow<List<SyncLogEntry>>(emptyList())
    val syncLog: StateFlow<List<SyncLogEntry>> = _syncLog.asStateFlow()

    private val _dittoDocCounts = MutableStateFlow<Map<String, Int>>(emptyMap())
    val dittoDocCounts: StateFlow<Map<String, Int>> = _dittoDocCounts.asStateFlow()

    private var presenceObserver: DittoPresenceObserver? = null

    init {
        try {
            presenceObserver = ditto.presence.observe { graph ->
                val local = PeerInfo(
                    deviceName = graph.localPeer.deviceName,
                    connectionTypes = emptyList(),
                    isLocal = true
                )
                val remote = graph.remotePeers.map { peer ->
                    PeerInfo(
                        deviceName = peer.deviceName,
                        connectionTypes = peer.connections.map { it.connectionType.toString() }.distinct()
                    )
                }
                _peers.value = listOf(local) + remote
            }
        } catch (_: Exception) {
            // Ditto presence methods are JNI-backed and throw in JVM unit tests.
        }
        refreshDittoCounts()
    }

    fun toggleFirestoreNetwork() {
        viewModelScope.launch {
            try {
                if (_firestoreEnabled.value) {
                    firestore.disableNetwork().await()
                    _firestoreEnabled.value = false
                    addLogEntry("Firestore", "toggle", "Network DISABLED -- Ditto is now sole store")
                    Log.i(TAG, "Firestore network disabled")
                } else {
                    firestore.enableNetwork().await()
                    _firestoreEnabled.value = true
                    addLogEntry("Firestore", "toggle", "Network ENABLED -- Firestore is primary")
                    Log.i(TAG, "Firestore network enabled")
                }
            } catch (e: Exception) {
                addLogEntry("Firestore", "error", "Toggle failed: ${e.message}")
            }
        }
    }

    fun addLogEntry(target: String, operation: String, detail: String) {
        _syncLog.value = (listOf(
            SyncLogEntry(System.currentTimeMillis(), target, operation, detail)
        ) + _syncLog.value).take(MAX_LOG_ENTRIES)
    }

    fun refreshDittoCounts() {
        viewModelScope.launch {
            try {
                val counts = mutableMapOf<String, Int>()
                for (collection in listOf("products", "orders", "inventory")) {
                    val result = ditto.store.execute(
                        "SELECT * FROM $collection WHERE deleted = false"
                    )
                    counts[collection] = result.items.size
                }
                _dittoDocCounts.value = counts
            } catch (_: Exception) {
                // JNI-backed, may fail in tests
            }
        }
    }

    override fun onCleared() {
        presenceObserver?.close()
        super.onCleared()
    }
}
