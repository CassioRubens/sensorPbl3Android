package br.com.hussan.coffeeiot.ui.coffee

import android.content.Intent
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

import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.android.synthetic.main.activity_medida.*
import org.eclipse.paho.client.mqttv3.MqttMessage
import org.jetbrains.anko.startActivity

class MainActivity : AppCompatActivity() {
    val mqttClient: MqttClient by lazy {
        MqttClient(this)
    }
    var ip:String = ""
    var porta:String = ""
    var onOff = false
    var contTemp = 0
    var contUmi = 0
    var contLumi = 0
    var contPres = 0
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        ip = intent.getStringExtra("ip").toString() 
        porta = intent.getStringExtra("porta").toString()
        // Toast.makeText(this,"${ip}, ${porta}", Toast.LENGTH_SHORT).show()
        connectAndSubscribe()
        btnTemperatura.setOnClickListener {
            startActivity<MedidaActivity>(
                "ip" to ip,
                "porta" to porta,
                "topic" to "temperatura",
                "tipo" to "Temperatura"
            )
        }
        btnUmidade.setOnClickListener {
            startActivity<MedidaActivity>(
                "ip" to ip,
                "porta" to porta,
                "topic" to "umidade",
                "tipo" to "Umidade"
            )
        }
        btnPressao.setOnClickListener {
            startActivity<MedidaActivity>(
                "ip" to ip,
                "porta" to porta,
                "topic" to "pressao",
                "tipo" to "Pressão atmosférica"
            )
        }
        btnLuminosidade.setOnClickListener {
            startActivity<MedidaActivity>(
                "ip" to ip,
                "porta" to porta,
                "topic" to "luminosidade",
                "tipo" to "Luminosidade"
            )
        }
        btnIntervalo.setOnClickListener {
            startActivity<TempoActivity>(
                "ip" to ip,
                "porta" to porta,
                "topic" to "sensor/tempo",
                "tipo" to "Intervalo"
            )
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
        mqttClient.setCallBack(arrayOf("sensor/pressao", "sensor/umidade", "sensor/temperatura", "sensor/luminosidade"), ::getSensor)
    }
    fun getSensor (topic: String, message: MqttMessage) {
        if (topic == "sensor/pressao") {
            savePressao(topic, message)
        } else if (topic == "sensor/umidade") {
            saveUmidade(topic, message)
        } else if (topic == "sensor/luminosidade") {
            saveLuminosidade(topic, message)
        } else if (topic == "sensor/temperatura") {
            saveTemperatura(topic, message)
        }
    }
    fun saveTemperatura (topic: String, message: MqttMessage) {

        val aux = fromJson<Aux>(message.toString())
        if (aux.status == 1) {

            tvTempValor.text = aux.value.toString()
            tvTempHorario.text = aux.tempo.toString()
            var s = Prefs.getString("temperatura")
            // var mAux:Aux = fromJson(message.toString())
            var temperaturas:ArrayList<Aux>? = null

            if (s.isNullOrEmpty()) {
                temperaturas = arrayListOf(aux)
                Log.d("TTEEE",temperaturas.toJson())
            } else {
                Log.d("TTEEE", "aasdasd")
                temperaturas = fromJson<List<Aux>>(s.toString()) as ArrayList<Aux>

            }
            // temperaturas.add(aux)
            //Prefs.setString("temperatura", temperaturas.toJson())
            Log.d("TTEEE", temperaturas.size.toString())
            if (temperaturas.size < 10) {
                temperaturas.add(contTemp, aux)
                contTemp += 1
                Log.d("TTEEE", "PASOU 1")
            } else {
                temperaturas.removeAt(contTemp)
                temperaturas.add(contTemp,  aux)
                Log.d("TTEEE", "PASOU 2 ${contTemp}")
                if (contTemp >= 10) {
                    contTemp = 0
                }

                contTemp += 1;
            }
            Prefs.setString("temperatura", temperaturas.toJson())
        }


}

    fun saveUmidade (topic: String, message: MqttMessage) {

        val aux = fromJson<Aux>(message.toString())
        if (aux.status == 1) {

            tvUmidValor.text = aux.value.toString()
            tvUmidHorario.text = aux.tempo.toString()
            var s = Prefs.getString("umidade")
            // var mAux:Aux = fromJson(message.toString())
            var temperaturas:ArrayList<Aux>? = null

            if (s.isNullOrEmpty()) {
                temperaturas = arrayListOf(aux)
                Log.d("TTEEE",temperaturas.toJson())
            } else {
                Log.d("TTEEE", "aasdasd")
                temperaturas = fromJson<List<Aux>>(s.toString()) as ArrayList<Aux>

            }
            // temperaturas.add(aux)
            //Prefs.setString("temperatura", temperaturas.toJson())
            Log.d("TTEEE", temperaturas.size.toString())
            if (temperaturas.size < 10) {
                temperaturas.add(contUmi, aux)
                contUmi += 1
                Log.d("TTEEE", "PASOU 1")
            } else {
                temperaturas.removeAt(contUmi)
                temperaturas.add(contUmi,  aux)
                Log.d("TTEEE", "PASOU 2 ${contUmi}")
                if (contUmi >= 10) {
                    contUmi = 0
                }

                contUmi += 1;
            }
            Prefs.setString("umidade", temperaturas.toJson())
        }


    }

    fun saveLuminosidade (topic: String, message: MqttMessage) {
        val aux = fromJson<Aux>(message.toString())
        if (aux.status == 1) {

            tvLumdValor.text = aux.value.toString()
            tvLumdHorario.text = aux.tempo.toString()
            var s = Prefs.getString("luminosidade")
            // var mAux:Aux = fromJson(message.toString())
            var temperaturas:ArrayList<Aux>? = null

            if (s.isNullOrEmpty()) {
                temperaturas = arrayListOf(aux)
                Log.d("TTEEE",temperaturas.toJson())
            } else {
                Log.d("TTEEE", "aasdasd")
                temperaturas = fromJson<List<Aux>>(s.toString()) as ArrayList<Aux>

            }
            // temperaturas.add(aux)
            //Prefs.setString("temperatura", temperaturas.toJson())
            Log.d("TTEEE", temperaturas.size.toString())
            if (temperaturas.size < 10) {
                temperaturas.add(contLumi, aux)
                contLumi += 1
                Log.d("TTEEE", "PASOU 1")
            } else {
                temperaturas.removeAt(contLumi)
                temperaturas.add(contLumi,  aux)
                Log.d("TTEEE", "PASOU 2 ${contLumi}")
                if (contLumi >= 10) {
                    contLumi = 0
                }

                contLumi += 1;
            }
            Prefs.setString("luminosidade", temperaturas.toJson())
        }
    }

    fun savePressao (topic: String, message: MqttMessage) {
    val aux = fromJson<Aux>(message.toString())
    if (aux.status == 1) {

        tvPresValor.text = aux.value.toString()
        tvPresHorario.text = aux.tempo.toString()
        var s = Prefs.getString("pressao")
        // var mAux:Aux = fromJson(message.toString())
        var temperaturas:ArrayList<Aux>? = null

        if (s.isNullOrEmpty()) {
            temperaturas = arrayListOf(aux)
            Log.d("TTEEE",temperaturas.toJson())
        } else {
            Log.d("TTEEE", "aasdasd")
            temperaturas = fromJson<List<Aux>>(s.toString()) as ArrayList<Aux>

        }
        // temperaturas.add(aux)
        //Prefs.setString("temperatura", temperaturas.toJson())
        Log.d("TTEEE", temperaturas.size.toString())
        if (temperaturas.size < 10) {
            temperaturas.add(contPres, aux)
            contPres += 1
            Log.d("TTEEE", "PASOU 1")
        } else {
            temperaturas.removeAt(contPres)
            temperaturas.add(contPres,  aux)
            Log.d("TTEEE", "PASOU 2 ${contPres}")
            if (contPres >= 10) {
                contPres = 0
            }

            contPres += 1;
        }
        Prefs.setString("pressao", temperaturas.toJson())
    }
    }
}