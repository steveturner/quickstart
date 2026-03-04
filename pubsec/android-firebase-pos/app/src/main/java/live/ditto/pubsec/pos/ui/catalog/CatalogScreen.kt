package live.ditto.pubsec.pos.ui.catalog

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.ScrollableTabRow
import androidx.compose.material3.Surface
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import live.ditto.pubsec.pos.data.model.Product
import org.koin.androidx.compose.koinViewModel

@Composable
fun CatalogScreen(vm: CatalogViewModel = koinViewModel()) {
    val products by vm.products.collectAsStateWithLifecycle()
    val inventory by vm.inventory.collectAsStateWithLifecycle()
    val cart by vm.cart.collectAsStateWithLifecycle()
    val selectedCategory by vm.selectedCategory.collectAsStateWithLifecycle()
    val snackbarHostState = remember { SnackbarHostState() }

    LaunchedEffect(Unit) {
        vm.orderPlaced.collect { shortId ->
            snackbarHostState.showSnackbar("Order #$shortId placed")
        }
    }

    val categories = listOf(null, "Beverage", "Food", "Merchandise")
    val categoryLabels = listOf("All", "Beverages", "Food", "Merchandise")
    val filteredProducts = if (selectedCategory == null) products
    else products.filter { it.category == selectedCategory }

    val cartItemCount = cart.values.sum()
    val cartTotal = cart.entries.sumOf { (productId, qty) ->
        val product = products.find { it._id == productId }
        (product?.price ?: 0L) * qty
    }

    Scaffold(snackbarHost = { SnackbarHost(snackbarHostState) }) { innerPadding ->
        Column(modifier = Modifier.fillMaxSize().padding(innerPadding)) {
            ScrollableTabRow(
                selectedTabIndex = categories.indexOf(selectedCategory).coerceAtLeast(0),
                edgePadding = 16.dp
            ) {
                categories.forEachIndexed { index, cat ->
                    Tab(
                        selected = selectedCategory == cat,
                        onClick = { vm.selectCategory(cat) },
                        text = { Text(categoryLabels[index]) }
                    )
                }
            }

            LazyVerticalGrid(
                columns = GridCells.Fixed(2),
                contentPadding = PaddingValues(8.dp),
                modifier = Modifier.weight(1f)
            ) {
                items(filteredProducts, key = { it._id }) { product ->
                    ProductCard(
                        product = product,
                        stockCount = inventory[product._id] ?: 0,
                        cartQty = cart[product._id] ?: 0,
                        onTap = { vm.addToCart(product._id) }
                    )
                }
            }

            AnimatedVisibility(visible = cartItemCount > 0) {
                Surface(
                    tonalElevation = 3.dp,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Row(
                        modifier = Modifier.padding(horizontal = 16.dp, vertical = 12.dp),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            "$cartItemCount item${if (cartItemCount != 1) "s" else ""} — $${String.format("%.2f", cartTotal / 100.0)}",
                            style = MaterialTheme.typography.titleMedium
                        )
                        Button(onClick = { vm.submitOrder() }) {
                            Text("Submit Order")
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun ProductCard(
    product: Product,
    stockCount: Int,
    cartQty: Int,
    onTap: () -> Unit
) {
    Card(
        modifier = Modifier
            .padding(4.dp)
            .fillMaxWidth()
            .clickable(onClick = onTap),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(product.name, style = MaterialTheme.typography.titleSmall, maxLines = 1)
            Spacer(Modifier.height(4.dp))
            Text(
                "$${String.format("%.2f", product.price / 100.0)}",
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.primary
            )
            Spacer(Modifier.height(4.dp))
            Row(
                horizontalArrangement = Arrangement.SpaceBetween,
                modifier = Modifier.fillMaxWidth()
            ) {
                Surface(
                    shape = MaterialTheme.shapes.small,
                    color = MaterialTheme.colorScheme.secondaryContainer,
                ) {
                    Text(
                        product.category,
                        modifier = Modifier.padding(horizontal = 6.dp, vertical = 2.dp),
                        style = MaterialTheme.typography.labelSmall
                    )
                }
                Text(
                    "$stockCount in stock",
                    style = MaterialTheme.typography.labelSmall,
                    color = if (stockCount > 0) MaterialTheme.colorScheme.onSurfaceVariant
                    else MaterialTheme.colorScheme.error
                )
            }
            if (cartQty > 0) {
                Spacer(Modifier.height(4.dp))
                Surface(
                    shape = MaterialTheme.shapes.small,
                    color = MaterialTheme.colorScheme.primaryContainer
                ) {
                    Text(
                        "In cart: $cartQty",
                        modifier = Modifier.padding(horizontal = 6.dp, vertical = 2.dp),
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.onPrimaryContainer
                    )
                }
            }
        }
    }
}
