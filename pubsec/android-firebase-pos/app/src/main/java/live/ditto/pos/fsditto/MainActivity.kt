package live.ditto.pos.fsditto

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.automirrored.filled.List
import androidx.compose.material.icons.filled.ShoppingCart
import androidx.compose.material3.Icon
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import live.ditto.pos.fsditto.ui.StatusScreen
import live.ditto.pos.fsditto.ui.catalog.CatalogScreen
import live.ditto.pos.fsditto.ui.orders.OrdersScreen
import live.ditto.pos.fsditto.ui.theme.PosTheme
import live.ditto.transports.DittoSyncPermissions

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            PosTheme {
                val navController = rememberNavController()
                val currentBackStack by navController.currentBackStackEntryAsState()
                val currentRoute = currentBackStack?.destination?.route

                Scaffold(
                    bottomBar = {
                        NavigationBar {
                            NavigationBarItem(
                                selected = currentRoute == "catalog",
                                onClick = {
                                    navController.navigate("catalog") {
                                        launchSingleTop = true
                                        popUpTo("catalog") { inclusive = true }
                                    }
                                },
                                icon = { Icon(Icons.Default.ShoppingCart, contentDescription = "Catalog") },
                                label = { Text("Catalog") }
                            )
                            NavigationBarItem(
                                selected = currentRoute == "orders",
                                onClick = {
                                    navController.navigate("orders") { launchSingleTop = true }
                                },
                                icon = { Icon(Icons.AutoMirrored.Filled.List, contentDescription = "Orders") },
                                label = { Text("Orders") }
                            )
                            NavigationBarItem(
                                selected = currentRoute == "status",
                                onClick = {
                                    navController.navigate("status") { launchSingleTop = true }
                                },
                                icon = { Icon(Icons.Default.Info, contentDescription = "Status") },
                                label = { Text("Status") }
                            )
                        }
                    }
                ) { paddingValues ->
                    NavHost(
                        navController = navController,
                        startDestination = "catalog",
                        modifier = Modifier.padding(paddingValues)
                    ) {
                        composable("catalog") { CatalogScreen() }
                        composable("orders") { OrdersScreen() }
                        composable("status") { StatusScreen() }
                    }
                }
            }
        }

        requestMissingPermissions()
    }

    // Request Bluetooth and WiFi permissions required for Ditto P2P mesh sync.
    private fun requestMissingPermissions() {
        val missingPermissions = DittoSyncPermissions(this).missingPermissions()
        if (missingPermissions.isNotEmpty()) {
            this.requestPermissions(missingPermissions, 0)
        }
    }
}
