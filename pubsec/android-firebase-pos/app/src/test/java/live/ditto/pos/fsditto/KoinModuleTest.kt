package live.ditto.pos.fsditto

import android.content.Context
import com.google.firebase.firestore.FirebaseFirestore
import io.mockk.mockk
import live.ditto.Ditto
import live.ditto.pos.fsditto.di.repositoryModule
import live.ditto.pos.fsditto.di.viewModelModule
import org.junit.Test
import org.koin.android.ext.koin.androidContext
import org.koin.core.context.startKoin
import org.koin.core.context.stopKoin
import org.koin.dsl.module
import org.koin.test.KoinTest
import org.koin.test.check.checkModules

class KoinModuleTest : KoinTest {

    @Test
    fun verifyKoinApp() {
        // Verify the Koin DI graph assembles without errors using a mock Android context.
        // appModule is replaced with a test stub providing mock Ditto and FirebaseFirestore
        // instances — both require Android native runtime unavailable in JVM unit tests.
        // repositoryModule and viewModelModule (currently empty stubs) are also verified.
        val testAppModule = module {
            single { mockk<Ditto>(relaxed = true) }
            single { mockk<FirebaseFirestore>(relaxed = true) }
        }

        val koinApp = startKoin {
            androidContext(mockk<Context>(relaxed = true))
            modules(testAppModule, repositoryModule, viewModelModule)
        }
        koinApp.checkModules()
        stopKoin()
    }
}
