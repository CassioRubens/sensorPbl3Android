package br.com.hussan.coffeeiot.ui.coffee

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import br.com.hussan.coffeeiot.R
import br.com.hussan.coffeeiot.model.Aux

class HistoricoAdapter (
    val lista: List<Aux>):
    RecyclerView.Adapter<HistoricoAdapter.HistoricoViewHolder>() {

        class HistoricoViewHolder(view: View): RecyclerView.ViewHolder(view) {
           // var tipo: TextView
            var tempo: TextView
            var medicao: TextView
            init {
                //tipo = view.findViewById<TextView>(R.id.tvTipo)
                tempo = view.findViewById<TextView>(R.id.tvTempo)
                medicao = view.findViewById<TextView>(R.id.tvMedicao)
            }
        }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): HistoricoViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.adapter_historico, parent, false)
        val holder = HistoricoViewHolder(view)
        return holder
    }

    override fun onBindViewHolder(holder: HistoricoViewHolder, position: Int) {
        val context = holder.itemView.context
        val aux = lista[position]
        //holder.tipo.text = aux.tipo
        holder.tempo.text = aux.tempo
        holder.medicao.text = aux.value
    }

    override fun getItemCount() = this.lista.size

}
