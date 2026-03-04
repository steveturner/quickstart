package live.ditto.pubsec.pos.data.seed

import live.ditto.pubsec.pos.data.model.InventoryItem
import live.ditto.pubsec.pos.data.model.Product

object SeedData {

    val products: List<Product> = listOf(
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440001",
            name = "Espresso",
            category = "Beverage",
            price = 299L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440002",
            name = "Latte",
            category = "Beverage",
            price = 449L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440003",
            name = "Cappuccino",
            category = "Beverage",
            price = 399L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440004",
            name = "Cold Brew",
            category = "Beverage",
            price = 479L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440005",
            name = "Croissant",
            category = "Food",
            price = 349L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440006",
            name = "Turkey Sandwich",
            category = "Food",
            price = 799L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440007",
            name = "Blueberry Muffin",
            category = "Food",
            price = 299L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440008",
            name = "Coffee Mug",
            category = "Merchandise",
            price = 1299L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        Product(
            _id = "550e8400-e29b-41d4-a716-446655440009",
            name = "Tote Bag",
            category = "Merchandise",
            price = 1599L,
            imageUrl = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        )
    )

    val inventoryItems: List<InventoryItem> = listOf(
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440001",
            productId = "550e8400-e29b-41d4-a716-446655440001",
            quantity = 50,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440002",
            productId = "550e8400-e29b-41d4-a716-446655440002",
            quantity = 50,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440003",
            productId = "550e8400-e29b-41d4-a716-446655440003",
            quantity = 50,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440004",
            productId = "550e8400-e29b-41d4-a716-446655440004",
            quantity = 50,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440005",
            productId = "550e8400-e29b-41d4-a716-446655440005",
            quantity = 30,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440006",
            productId = "550e8400-e29b-41d4-a716-446655440006",
            quantity = 20,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440007",
            productId = "550e8400-e29b-41d4-a716-446655440007",
            quantity = 25,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440008",
            productId = "550e8400-e29b-41d4-a716-446655440008",
            quantity = 15,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        ),
        InventoryItem(
            _id = "aa0e8400-e29b-41d4-a716-446655440009",
            productId = "550e8400-e29b-41d4-a716-446655440009",
            quantity = 10,
            lastModifiedBy = "",
            deleted = false,
            syncSource = "firestore",
            lastSyncedAt = 0L
        )
    )
}
