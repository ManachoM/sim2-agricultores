#include "../includes/agricultor_simple.h"

AgricultorSimple::AgricultorSimple(
    FEL *_fel, Terreno *_ter, MercadoMayorista *_mer
)
    : Agricultor(_fel, _ter, _mer)
{
}

const int AgricultorSimple::choose_product()
{
  // Consultamos el índice
  auto prods_mes = this->env->get_siembra_producto_mes();

  auto lista_prods = prods_mes.find(this->env->get_month());

  if (lista_prods == this->env->get_siembra_producto_mes().end())
  {
    printf("ERROR EN SIEMBRA_PRODUCTO_MES\n");
    exit(EXIT_FAILURE);
  }

  // Elegimos el prod
  Producto *prod_elegido =
      (*select_randomly(lista_prods->second.begin(), lista_prods->second.end())
      );

  return prod_elegido->get_id();
}
