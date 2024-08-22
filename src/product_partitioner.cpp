#include "../includes/product_partitioner.h"

#include "../includes/product.h"

#include <cmath>
#include <stdexcept>

ProductPartioner::ProductPartioner(
    const std::unordered_map<int, Producto *> &_prods
)
    : prods(_prods) {}

double ProductPartioner::objective_function(std::vector<int> &_sol) {
  int num_prods = this->prods.size();

  // Cada llave es un id_prod, y cada valor es un vector de 12 elementos,
  // donde cada posición es un mes del año y el valor es la cantidad de
  // productos en ese procesador que se comercializan en dicho mes
  std::unordered_map<int, std::vector<double>> prods_por_mes_por_proc;
  std::vector<double> centroid(12, 0.0);
  // Poblamos los vectores por procesador
  for (int i = 0; i < num_prods; ++i) {
    // Obtenemos el vector de doce meses del procesador
    std::vector<double> prods_por_mes;
    try {
      prods_por_mes = prods_por_mes_por_proc.at(_sol[i]);
    } catch (std::out_of_range &e) {
      prods_por_mes = std::vector<double>(12, 0.0);
    }
    Producto *p = this->prods.at(i);
    std::vector<int> meses_venta = p->get_meses_venta();

    // Para cada uno de los meses en los que se comercializa
    // el producto, aumentamos el respectivo contador
    for (const auto &mes : meses_venta) {
      prods_por_mes[mes]++;
    }

    // Nos aseguramos de guardar el vector actualizado
    prods_por_mes_por_proc[_sol[i]] = prods_por_mes;

    // Actualizamos el centroide
    for (int i = 0; i < 12; ++i)
      centroid[i] += prods_por_mes[i];
  }

  // Que no se nos olvide dividir
  // (en este caso, el tamaño del mapa más grande representa
  // la cantidad de procesadores)
  for (int i = 0; i < 12; ++i)
    centroid[i] /= (double)prods_por_mes_por_proc.size();
  double result = 0.0;

  // Ahora, recorremos cada una de las entradas por procesador y vamos sumando
  // la diferencia con respecto al centroide
  for (const auto &[prod, prods_por_mes] : prods_por_mes_por_proc) {
    for (int i = 0; i < 12; ++i) {
      result += abs(centroid[i] - prods_por_mes[i]);
    }
  }
  return result;
};

std::vector<int> ProductPartioner::next_neighbour(
    const std::vector<int> &_sol, const int r, const int n_procs
) {
  int num_prods = (int)this->prods.size();

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> prod_chooser(0, num_prods - 1);
  std::uniform_int_distribution<int> proc_picker(0, n_procs - 1);

  std::vector<int> changed_prods(num_prods, 0);
  // Elegimos los productos a ser modificados y les designamos un nuevo
  // procesador

  for (int i = 0; i < r; ++i) {
    changed_prods.at(prod_chooser(gen)) = proc_picker(gen);
  }

  // Generamos el vector de salida y retornamos
  std::vector<int> new_sol = _sol;
  for (int i = 0; i < num_prods; ++i) {
    if (changed_prods[i])
      new_sol[i] = changed_prods[i];
  }

  return new_sol;
}

float ProductPartioner::acceptance_probability(
    double e1, double e2, double temp
) {
  if (e2 < e1)
    return 1.0;

  return exp(-(e2 - e1) / temp);
}

std::unordered_map<int, std::vector<Producto *>>
ProductPartioner::partition_products(
    const int n_procs, const int k_max, const int r
) {
  std::vector<int> best_sol(this->prods.size(), 0);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> rand_gen(0, 1.0);
  int num_prods = this->prods.size();

  // Generamos una solución inicial usando el módulo en función de la cantidad
  // de procesadores

  for (const auto &[id, prod] : this->prods) {
    best_sol[id] = id % n_procs;
  }

  int k = 0;
  do {
    // Actualizamos la temperatura
    double temp;
    temp = 1 - (k + 1) / k_max;

    // Elegimos al nuevo vecino
    std::vector<int> neighbour = this->next_neighbour(best_sol, r, n_procs);

    // Si la probabilidad de aceptación es mayor a un random, actualizamos la
    // mejor solución
    if (this->acceptance_probability(
            this->objective_function(best_sol),
            this->objective_function(neighbour), temp
        ) >= rand_gen(gen))
      best_sol = neighbour;
    ++k;
  } while (k < k_max);

  // Convertimos la representación de la solución a algo útil
  std::unordered_map<int, std::vector<Producto *>> ret_val(num_prods);
  for (int i = 0; i < num_prods; ++i) {
    ret_val[best_sol[i]].push_back(this->prods.at(i));
  }

  return ret_val;
};
