package live.ditto.pos.fsditto.di

import live.ditto.pos.fsditto.ui.StatusViewModel
import live.ditto.pos.fsditto.ui.catalog.CatalogViewModel
import live.ditto.pos.fsditto.ui.orders.OrdersViewModel
import org.koin.core.module.dsl.viewModel
import org.koin.dsl.module

val viewModelModule = module {
    single { StatusViewModel(get(), get(), get()) }
    viewModel { CatalogViewModel(get(), get(), get()) }
    viewModel { OrdersViewModel(get(), get(), get()) }
}
