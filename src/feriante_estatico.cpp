#include "../includes/feriante_estatico.h"

#include "../includes/environment.h"
#include "../includes/mercado_mayorista.h"
#include "../includes/product.h"

#include <csignal>
FerianteEstatico::FerianteEstatico(
    Environment *_env, FEL *fel, MercadoMayorista *mer, int feria_id
)
    : Feriante(fel, mer, feria_id) {
  this->set_environment(_env);
  std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(
      4, 12
  ); // Para la cantidad de productos a vender
  int num_productos = dist(gen);
  std::vector<int> prod_ids;

  std::unordered_map<int, Producto *> arr = this->env->get_productos();

  for (auto const &[prod_id, prod_ptr] : this->env->get_productos()) {
    prod_ids.push_back(prod_id);
  }
  shuffle(prod_ids.begin(), prod_ids.end(), gen);
  for (int i = 0; i <= num_productos; ++i) {
    this->prods_ids.push_back(prod_ids[i]);
  }
}

std::vector<int> FerianteEstatico::choose_product() { return this->prods_ids; }

double FerianteEstatico::purchase_amount(const int prod_id) {
  auto prod_name = this->env->get_productos().at(prod_id)->get_nombre();
  auto prod_amount =
      this->env->get_productos().at(prod_id)->get_volumen_feriante();

  // if (prod_name == "Sandia" || prod_name == "Sandía")
  // exit(0);
  return this->env->get_productos().at(prod_id)->get_volumen_feriante();
}

Agricultor *
FerianteEstatico::choose_agricultor(const int prod_id, const double amount) {
  // std::cout << "MERCADO PTR " << this->mercado << "\n";
  std::vector<int> agricultores =
      this->mercado->get_agricultor_por_prod(prod_id);

  if (agricultores.size() <= 0)
    return nullptr;
  for (auto agr : agricultores) {
    // Si ya intentamos comprarle al agricultor, seguimos
    if (count(
            this->agricultores_consultados.begin(),
            this->agricultores_consultados.end(), agr
        ))
      continue;

    this->agricultores_consultados.push_back(agr);
    // std::cout << "AGRICULTOR ELEGIDO: " << agr << std::endl;
    //? Esto podría ser caro eventualmente (?) ==> revisar por arreglo
    // estático
    return this->env->get_agricultor(agr);
  }
  // En volaa levantar excepción?
  return nullptr;
}

void FerianteEstatico::finish_purchase() {
  this->agricultores_consultados.clear();
}
