package br.com.hussan.coffeeiot.ui.coffee

import android.content.ContentValues
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.support.v7.widget.StaggeredGridLayoutManager
import android.util.Log
import android.widget.Toast
import br.com.hussan.coffeeiot.R
import br.com.hussan.coffeeiot.extensions.Prefs
import br.com.hussan.coffeeiot.extensions.fromJson
import br.com.hussan.coffeeiot.extensions.toJson
import br.com.hussan.coffeeiot.model.Aux
import br.com.hussan.coffeeiot.mqtt.MqttClient
import kotlinx.android.synthetic.main.activity_historico.*
import kotlinx.android.synthetic.main.activity_medida.*
import kotlinx.android.synthetic.main.activity_medida.recycleView
import kotlinx.android.synthetic.main.activity_medida.tvTipo
import org.eclipse.paho.client.mqttv3.MqttMessage

import kotlin.math.log

class MedidaActivity : AppCompatActivity() {
    val mqttClient: MqttClient by lazy {
         MqttClient(this)
    }
    var ip:String = ""
    var porta:String = ""
    var topic:String = ""
    var tipo:String = ""
    var cont = 0;

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_medida)
        topic = intent.getStringExtra("topic").toString()
        tipo = intent.getStringExtra("tipo").toString()
        tvTipo.text = tipo.toString()
        updateButton()
    }


    private fun updateButton() {
            var s = Prefs.getString(topic)
                // var mAux:Aux = fromJson(message.toString())
            var temperaturas:ArrayList<Aux> = fromJson<List<Aux>>(s.toString()) as ArrayList<Aux>

            val recyclerView = recycleView
            recyclerView.adapter = HistoricoAdapter(temperaturas)
            val layoutManager = StaggeredGridLayoutManager(1, StaggeredGridLayoutManager.VERTICAL)
            recyclerView.layoutManager = layoutManager

        }
    }

