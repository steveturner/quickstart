package live.ditto.pos.fsditto.data.seed

import com.google.firebase.firestore.FirebaseFirestore
import kotlinx.coroutines.tasks.await
import live.ditto.pos.fsditto.data.model.Collections

class FirestoreSeeder(private val db: FirebaseFirestore) {

    suspend fun seedIfEmpty() {
        val snapshot = db.collection(Collections.PRODUCTS).limit(1).get().await()
        if (snapshot.isEmpty) {
            val batch = db.batch()
            SeedData.products.forEach { product ->
                val ref = db.collection(Collections.PRODUCTS).document(product._id)
                batch.set(ref, product)
            }
            SeedData.inventoryItems.forEach { item ->
                val ref = db.collection(Collections.INVENTORY).document(item._id)
                batch.set(ref, item)
            }
            batch.commit().await()
        }
    }
}
