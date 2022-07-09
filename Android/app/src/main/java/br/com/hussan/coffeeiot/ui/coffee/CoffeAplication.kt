package br.com.hussan.coffeeiot.ui.coffee

import android.app.Application
import android.util.Log


class CoffeAplication: Application() {
    private val TAG = "CoffeAplication"
    override fun onCreate() {
        super.onCreate()
        appInstance = this
    }
    companion object {
        private var appInstance: CoffeAplication? = null
        fun getInstance(): CoffeAplication {
            if (appInstance == null) {
                throw IllegalStateException("Configure a classe de application")
            }
            return appInstance!!
        }
    }

    override fun onTerminate() {
        super.onTerminate()
        Log.d(TAG, "CoffeAplication.onTerminate()")
    }
}