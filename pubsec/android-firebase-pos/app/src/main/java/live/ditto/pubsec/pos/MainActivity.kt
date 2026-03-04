package live.ditto.pubsec.pos

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import live.ditto.pubsec.pos.ui.StatusScreen
import live.ditto.pubsec.pos.ui.theme.PosTheme
import live.ditto.transports.DittoSyncPermissions

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            PosTheme {
                StatusScreen()
            }
        }

        requestMissingPermissions()
    }

    // Request Bluetooth and WiFi permissions required for Ditto P2P mesh sync.
    // DittoSyncPermissions handles the full list across API levels (including
    // Bluetooth changes at API 31 and NEARBY_WIFI_DEVICES at API 33).
    // Source: android-kotlin/QuickStartTasks/app/src/main/java/.../MainActivity.kt
    private fun requestMissingPermissions() {
        val missingPermissions = DittoSyncPermissions(this).missingPermissions()
        if (missingPermissions.isNotEmpty()) {
            this.requestPermissions(missingPermissions, 0)
        }
    }
}
