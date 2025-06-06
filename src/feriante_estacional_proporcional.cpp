#include "../includes/feriante_estacional_proporcional.h"

#include "../includes/environment.h"
#include "../includes/product.h"

#include <random>

FerianteEstacionalProporcional::FerianteEstacionalProporcional(
    Environment *_env, FEL *fel, MercadoMayorista *mer, int feria_id,
    int prod_amount
)
    : FerianteEstatico(_env, fel, mer, feria_id), prod_amount(prod_amount) {
  std::random_device rd;
  unsigned int seed = rd() + get_feriante_id() * 10;
  gen = std::mt19937(seed);
  dist = std::uniform_real_distribution<double>(0.0, 1.0);
}

std::vector<int> FerianteEstacionalProporcional::choose_product() {
  std::vector<int> prods_seleccionados;

  std::vector<Producto *> prods =
      this->env->get_venta_producto_mes().find(this->env->get_month())->second;

  for (auto prod : prods) {
    if (dist(gen) < prod->get_probabilidad_consumo())
      continue;
    prods_seleccionados.push_back(prod->get_id());

    if ((int)prods_seleccionados.size() >= this->prod_amount)
      break;
  }

  return prods_seleccionados;
}
