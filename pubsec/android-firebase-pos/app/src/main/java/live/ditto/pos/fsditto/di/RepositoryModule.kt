package live.ditto.pos.fsditto.di

import live.ditto.pos.fsditto.data.seed.DittoSeeder
import live.ditto.pos.fsditto.data.seed.FirestoreSeeder
import live.ditto.pos.fsditto.sync.SyncBridgeManager
import org.koin.dsl.module

val repositoryModule = module {
    single { FirestoreSeeder(get()) }
    single { DittoSeeder(get()) }
    single { SyncBridgeManager(get(), get()) }
}
