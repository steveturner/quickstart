package live.ditto.pubsec.pos.di

import com.google.firebase.Firebase
import com.google.firebase.firestore.FirebaseFirestore
import com.google.firebase.firestore.firestoreSettings
import com.google.firebase.firestore.firestore
import com.google.firebase.firestore.memoryCacheSettings
import live.ditto.Ditto
import live.ditto.DittoIdentity
import live.ditto.android.DefaultAndroidDittoDependencies
import live.ditto.pubsec.pos.BuildConfig
import org.koin.android.ext.koin.androidContext
import org.koin.dsl.module

val appModule = module {

    single {
        val androidDependencies = DefaultAndroidDittoDependencies(androidContext())
        val ditto = Ditto(
            androidDependencies,
            DittoIdentity.OnlinePlayground(
                dependencies = androidDependencies,
                appId = BuildConfig.DITTO_APP_ID,
                token = BuildConfig.DITTO_PLAYGROUND_TOKEN,
                customAuthUrl = BuildConfig.DITTO_AUTH_URL,
                enableDittoCloudSync = false
            )
        )
        ditto.updateTransportConfig { config ->
            config.connect.websocketUrls.add(BuildConfig.DITTO_WEBSOCKET_URL)
        }
        // Required for DQL — must be called before any DQL queries
        ditto.disableSyncWithV3()
        // ditto.startSync() is NOT called here — deferred to Phase 3 when subscriptions exist
        ditto
    }

    single {
        // Firebase.firestore uses ktx accessor; KTX is bundled in firebase-firestore since BoM 34.0.0
        val db = Firebase.firestore
        db.firestoreSettings = firestoreSettings {
            // Ditto is the offline store — disable Firestore's built-in local cache
            // Using memoryCacheSettings (current API) instead of deprecated isPersistenceEnabled = false
            setLocalCacheSettings(memoryCacheSettings {})
        }
        db
    }
}
