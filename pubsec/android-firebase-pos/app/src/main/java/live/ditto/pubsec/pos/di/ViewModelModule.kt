package live.ditto.pubsec.pos.di

import live.ditto.pubsec.pos.ui.catalog.CatalogViewModel
import live.ditto.pubsec.pos.ui.orders.OrdersViewModel
import org.koin.core.module.dsl.viewModel
import org.koin.dsl.module

val viewModelModule = module {
    viewModel { CatalogViewModel(get()) }
    viewModel { OrdersViewModel(get()) }
}
