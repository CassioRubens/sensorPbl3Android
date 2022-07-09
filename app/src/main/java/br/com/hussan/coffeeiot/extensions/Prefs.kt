package br.com.hussan.coffeeiot.extensions

import android.content.SharedPreferences
import br.com.hussan.coffeeiot.ui.coffee.CoffeAplication

object Prefs {
    val PREF_ID = "aux"
    private fun prefs(): SharedPreferences {
        val context = CoffeAplication.getInstance().applicationContext
        return context.getSharedPreferences(PREF_ID, 0)
    }

    fun setString (flag: String, valor: String) {
        val pref = prefs()
        val editor = pref.edit()
        editor.putString(flag, valor)
        editor.apply()
    }

    fun getString(flag: String): String {
        val pref = prefs()
        val s = pref.getString(flag, "")
        return s
    }
}