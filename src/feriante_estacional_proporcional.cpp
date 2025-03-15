#include "../includes/feriante_estacional_proporcional.h"

#include "../includes/agricultor.h"
#include "../includes/environment.h"
#include "../includes/mercado_mayorista.h"
#include "../includes/product.h"

#include <bsp.h> // Added for bsp_pid()

FerianteEstacionalProporcional::FerianteEstacionalProporcional(
    Environment *_env, FEL *fel, MercadoMayorista *mer, int feria_id,
    int prod_amount
)
    : FerianteEstatico(_env, fel, mer, feria_id), prod_amount(prod_amount)
{
  // Initialize random generator with processor ID for better randomization
  std::random_device rd;
  // Get processor ID and use it to add variability to the seed
  int pid = bsp_pid();
  // Create a unique seed using processor ID, feriante ID, and random device
  unsigned int seed = rd() + pid * 1000 + get_feriante_id() * 10;
  gen = std::mt19937(seed);
  dist = std::uniform_real_distribution<double>(0.0, 1.0);
}

std::vector<int> FerianteEstacionalProporcional::choose_product()
{
  std::vector<int> prods_seleccionados;

  std::vector<Producto *> prods =
      this->env->get_venta_producto_mes().find(this->env->get_month())->second;

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<double> d(0.0, 1.0);

  for (auto prod : prods)
  {
    // Generate random value
    double random_value = d(gen);
    double prob_consumo = prod->get_probabilidad_consumo();
    
    // FIX: Products with higher probability should be MORE likely to be selected
    // Original inverted logic: if (random_value < prob_consumo) continue;
    // Correct logic: if random_value is less than prob_consumo, SELECT the product
    if (random_value <= prob_consumo) 
    {
      prods_seleccionados.push_back(prod->get_id());
      
      if ((int)prods_seleccionados.size() >= this->prod_amount)
        break;
    }
  }

  return prods_seleccionados;
}

Agricultor *FerianteEstacionalProporcional::choose_agricultor(
    const int prod_id, const double amount
)
{
  // Get list of agricultores for this product
  std::vector<int> agricultores =
      this->mercado->get_agricultor_por_prod(prod_id);

  if (agricultores.empty())
    return nullptr;

  for (auto agr_id : agricultores)
  {
    // Skip if we've already consulted this agricultor
    if (std::find(
            this->agricultores_consultados.begin(),
            this->agricultores_consultados.end(), agr_id
        ) != this->agricultores_consultados.end())
      continue;

    // Add to consulted list to avoid repeated queries
    this->agricultores_consultados.push_back(agr_id);

    return this->env->get_agricultor(agr_id);
  }

  return nullptr;
}

void FerianteEstacionalProporcional::finish_purchase()
{
  // Clear the list of consulted agricultores when done
  this->agricultores_consultados.clear();
}
