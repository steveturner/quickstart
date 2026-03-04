package live.ditto.pubsec.pos.di

import live.ditto.pubsec.pos.data.seed.FirestoreSeeder
import live.ditto.pubsec.pos.sync.SyncBridgeManager
import org.koin.dsl.module

val repositoryModule = module {
    single { FirestoreSeeder(get()) }
    single { SyncBridgeManager(get(), get()) }
}
