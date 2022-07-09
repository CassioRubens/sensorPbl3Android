package com.massivecode.mysensor

import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import br.com.hussan.coffeeiot.R
import com.massivecode.mysensor.extensions.MqttClient

class SensorActivity : AppCompatActivity() {
    val mqttClient: MqttClient by lazy {
        MqttClient(this)
    }
    var ip:String = ""
    var porta:String = ""
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_sensor)
    }
}