#include "../includes/consumidor_familiar.h"
#include "../includes/environment.h"

ConsumidorFamiliar::ConsumidorFamiliar(int fam_size, int feria, FEL *fel)
    : ConsumidorSimple(feria, fel), fam_size(fam_size) {}

double ConsumidorFamiliar::purchase_amount(const int prod_id) {
  this->last_purchase_amount =
      this->env->get_productos().at(prod_id)->get_volumen_consumidor() *
      this->fam_size;
  return this->last_purchase_amount;
}
