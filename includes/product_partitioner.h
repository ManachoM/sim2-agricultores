#ifndef _PRODUCT_PARTITIONER_H_
#define _PRODUCT_PARTITIONER_H_

#include "glob.h"
class Producto;

class ProductPartioner
{
private:
  std::map<int, Producto> prods;
  double objective_function(std::vector<int> &_sol);
  std::vector<int> next_neighbour(const std::vector<int> & _sol, const int r, const int n_procs);
  float acceptance_probability(double e1, double e2, double temp);
public:
  ProductPartioner(const std::map<int, Producto> &_prods);
  std::unordered_map<int, std::vector<Producto>> partition_products(const int n_procs, const int k_max = 1'000, const int r = 1);
};

#endif // !_PRODUCT_PARTITIONER_H_
