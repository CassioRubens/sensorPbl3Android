package br.com.hussan.coffeeiot.ui.coffee

import android.content.Intent

import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import br.com.hussan.coffeeiot.R
import kotlinx.android.synthetic.main.activity_config.*
import kotlinx.android.synthetic.main.activity_config.btnSave
import kotlinx.android.synthetic.main.activity_config2.*


class Config2Activity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_config2)

        btnSave.setOnClickListener {
            val ip = ip.text.toString()
            val port = porta.text.toString()
            if (ip != null && port != null) {
                val intent = Intent(this, MainActivity::class.java)
                val params = Bundle()
                params.putString("ip", ip)
                params.putString("porta", port)
                intent.putExtras(params)
                startActivity(intent)
            }
        }
    }
}