#include "../includes/feriante_estacional.h"

#include "../includes/environment.h"
#include "../includes/product.h"

FerianteEstacional::FerianteEstacional(
    Environment *_env, FEL *fel, MercadoMayorista *mer, int feria_id
)
    : FerianteEstatico(_env, fel, mer, feria_id)
{
}

std::vector<int> FerianteEstacional::choose_product()
{
  std::vector<int> prod_seleccionados;
  std::vector<Producto *> prods =
      this->env->get_venta_producto_mes().find(this->env->get_month())->second;

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<double> threshold_dist(0.0, 1.0);
  std::uniform_real_distribution<double> random_num_dist(0.0, 1.0);

  double threashold = threshold_dist(gen);

  for (auto prod : prods)
  {
    double rand_num = random_num_dist(gen);

    if (rand_num < threashold)
      continue;

    prod_seleccionados.push_back(prod->get_id());
  }

  return prod_seleccionados;
}
