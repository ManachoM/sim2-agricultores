#include "../includes/consumidor_simple.h"
#include "../includes/environment.h"
#include "../includes/feria.h"

ConsumidorSimple::ConsumidorSimple(int _feria, FEL *fel)
    : Consumidor(fel, _feria) {}

std::vector<int> ConsumidorSimple::choose_product() {
  std::vector<Producto *> prods =
      this->env->get_venta_producto_mes().find(this->env->get_month())->second;

  std::vector<int> prods_to_buy;

  for (auto prod : prods) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> d(0.0, 1.0);

    if (d(gen) > prod->get_probabilidad_consumo())
      continue;

    prods_to_buy.push_back(prod->get_id());
  }

  // Caso borde - lanzar excepción? - nunca lo ví en implementación anterior
  return prods_to_buy;
}

double ConsumidorSimple::purchase_amount(const int prod_id) {
  this->last_purchase_amount =
      this->env->get_productos().at(prod_id)->get_volumen_consumidor();
  return this->last_purchase_amount;
}

Feriante *ConsumidorSimple::choose_feriante(const int prod_id,
                                            const double amount) {
  // Feria *feria = this->env->get_ferias().at(this->get_feria());
  Feria *feria = this->env->get_feria(this->get_feria());
  std::vector<int> feriantes = feria->get_feriantes_by_id(prod_id);
  for (auto feriante : feriantes) {
    // Si el feriante existe en nuestra lista de feriantes visitados, significa
    // que ya intentamos comprarle
    if (count(this->feriantes_consultados.begin(),
              this->feriantes_consultados.end(), feriante))
      continue;

    this->feriantes_consultados.push_back(feriante);
    return feria->get_feriantes().at(feriante);
  }
  // Caso límite, levantar excepción (?) - TODO
  return nullptr;
}

void ConsumidorSimple::finish_purchase() {
  this->feriantes_consultados.clear();
}
