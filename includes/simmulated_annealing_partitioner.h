#ifndef _SIMMULATED_ANNEALING_PARTITIONER_H_
#define _SIMMULATED_ANNEALING_PARTITIONER_H_

#include "glob.h"

#include <functional>
#include <unordered_map>

template <typename T> class SimmulatedAnnealingPartitioner
{
private:
  std::unordered_map<int, T *> items;
  std::function<
      double(const std::unordered_map<int, T *> &, const std::vector<int> &)>
      objective_function;
  double acceptance_probability(double e1, double e2, double temp);

  std::vector<int> next_neighbour(
      const std::vector<int> &_sol, const int r, const int n_procs,
      const int iter_max = 100
  );

public:
  SimmulatedAnnealingPartitioner(
      const std::unordered_map<int, T *> &_items,
      std::function<
          double(const std::unordered_map<int, T *> &, const std::vector<int> &)>
          _objective_function
  );

  std::unordered_map<int, std::vector<T *>>
  partition_items(const int n_procs, const int k_max = 10'000, const int r = 1);
};

#include "simmulated_annealing_partitioner.tpp"

#endif // !_SIMMULATED_ANNEALING_PARTITIONER_H_
