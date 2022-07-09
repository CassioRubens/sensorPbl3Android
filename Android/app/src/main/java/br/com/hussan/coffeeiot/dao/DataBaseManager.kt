//package br.com.hussan.coffeeiot.dao
//
//import android.arch.persistence.room.Room
//import br.com.hussan.coffeeiot.ui.coffee.CoffeAplication
//
//object DataBaseManager {
//    private var dbInstace: AuxDataBase
//    init {
//        val appContext = CoffeAplication.getInstance().applicationContext
//        dbInstace = Room.databaseBuilder(
//            appContext,
//            AuxDataBase::class.java,
//            "auxs.sqlite"
//        ).build()
//    }
//    fun getAuxDao(): AuxDao {
//        return dbInstace.auxDao()
//    }
//}