package live.ditto.pubsec.pos

import android.app.Application
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import live.ditto.Ditto
import live.ditto.pubsec.pos.sync.SyncBridgeManager
import live.ditto.pubsec.pos.di.appModule
import live.ditto.pubsec.pos.di.repositoryModule
import live.ditto.pubsec.pos.di.viewModelModule
import org.koin.android.ext.android.get
import org.koin.android.ext.koin.androidContext
import org.koin.android.ext.koin.androidLogger
import org.koin.core.context.startKoin

class PosApplication : Application() {

    // IO scope for off-main-thread Ditto initialization
    // SupervisorJob: child coroutine failure does not cancel the scope
    private val ioScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    override fun onCreate() {
        super.onCreate()

        // 1. Start Koin first — get<T>() cannot be called before startKoin completes.
        // Koin registers the DI container that backs every get<>() call below.
        startKoin {
            androidLogger()
            androidContext(this@PosApplication)
            modules(appModule, repositoryModule, viewModelModule)
        }

        // 2. Eagerly resolve Ditto on IO dispatcher, then start sync bridge.
        // Koin single{} blocks are lazy by default — first get<Ditto>() triggers the constructor.
        // Running this on IO ensures Ditto's initialization (which touches disk/network) never
        // blocks the main thread, avoiding StrictMode violations and potential ANR.
        //
        // 3. Start sync bridge after Ditto and Firestore are ready.
        // Bridge registers subscriptions, starts sync, then sets up bidirectional listeners.
        ioScope.launch {
            get<Ditto>()  // ensure Ditto is initialized first
            get<SyncBridgeManager>().start()
        }
    }
}
