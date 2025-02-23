#include "../includes/agricultor_ganancia.h"

AgricultorGanancia::AgricultorGanancia(
    FEL *_fel, Terreno *_terr, MercadoMayorista *_mer
)
    : Agricultor(_fel, _terr, _mer) {}

const int AgricultorGanancia::choose_product() {
  auto prods_mes = this->env->get_siembra_producto_mes();

  auto lista_prods = prods_mes.find(this->env->get_month());

  if (lista_prods == this->env->get_siembra_producto_mes().end()) {
    printf("ERROR EN SIEMBRA_PRODUCTO_MES\n");
    exit(EXIT_FAILURE);
  }
  int month = this->env->get_month();
  double terr_size = this->get_terreno()->get_area();
  // Recorremos la lista buscando la mejor opción económicamente
  Producto *chosen = lista_prods->second.at(0);
  double best_ganancia = (chosen->get_rendimiento() * terr_size *
                          chosen->get_precios_mes().at(month)) -
                         terr_size - chosen->get_costo_ha();
  for (auto prod : lista_prods->second) {

    double precio_prod = prod->get_precios_mes().at(month);
    double rendimiento = prod->get_rendimiento();
    double costo = prod->get_costo_ha();
    double ganancia = rendimiento * terr_size * precio_prod - terr_size * costo;
    if (ganancia > best_ganancia) {
      best_ganancia = ganancia;
      chosen = prod;
    }
  }

  return chosen->get_id();
}
