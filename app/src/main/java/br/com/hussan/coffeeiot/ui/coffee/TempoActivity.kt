package br.com.hussan.coffeeiot.ui.coffee

import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.util.Log
import android.widget.Toast
import br.com.hussan.coffeeiot.R
import br.com.hussan.coffeeiot.extensions.fromJson
import br.com.hussan.coffeeiot.extensions.toJson
import br.com.hussan.coffeeiot.model.Aux
import br.com.hussan.coffeeiot.mqtt.MqttClient
import kotlinx.android.synthetic.main.activity_medida.tvTipo
import kotlinx.android.synthetic.main.activity_tempo.*
import org.eclipse.paho.client.mqttv3.MqttMessage
import org.jetbrains.anko.startActivity
import kotlin.math.log

class TempoActivity : AppCompatActivity() {
    val mqttClient: MqttClient by lazy {
         MqttClient(this)
    }
    var ip:String = ""
    var porta:String = ""
    var topic:String = ""
    var tipo:String = ""
    var tempo:Int = 1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_tempo)

        ip = intent.getStringExtra("ip").toString()
        porta = intent.getStringExtra("porta").toString()
        topic = intent.getStringExtra("topic").toString()
        tipo = intent.getStringExtra("tipo").toString()
        tvTipo.text = tipo.toString()
        connectAndSubscribe()

        btnSelect.setOnClickListener {
            var aux = Aux()
            aux.status = 0
            aux.value = ""
            mqttClient.publishMessage(topic+"/set", tempo.toString())
        }
        btnAdd.setOnClickListener {
            tempo++
            tvTextMedida.text = tempo.toString()
        }
        btnSub.setOnClickListener {
            if (tempo > 1) {
                tempo--
                tvTextMedida.text = tempo.toString()
            }
        }
    }

    private fun connectAndSubscribe() {
        var link:String = ""
        if (!porta.isNullOrBlank()) {
            link = "tcp://${ip}:${porta}"
        } else {
            link = "tcp://${ip}"
        }
        mqttClient.connect(link)
        mqttClient.setCallBack(arrayOf(topic), ::updateButton)
    }
    private fun updateButton(topic: String, message: MqttMessage) {
        //Log.d("TT", message.toString())
        val aux = fromJson<Aux>(message.toString())
        if (aux.status == 1) {
         finish()
        }
        Toast.makeText(this, message.toString(), Toast.LENGTH_SHORT).show()
    }
}