#include "simmulated_annealing_partitioner.h"

#include <cmath>
#include <cstddef>

template <typename T>
SimmulatedAnnealingPartitioner<T>::SimmulatedAnnealingPartitioner(
    const std::unordered_map<int, T *> &_items,
    std::function<
        double(const std::unordered_map<int, T *> &, const std::vector<int> &)>
        _objective_function
)
    : items(_items), objective_function(_objective_function)
{
}

template <typename T>
double SimmulatedAnnealingPartitioner<T>::acceptance_probability(
    double e1, double e2, double temp
)
{
  if (e2 < e1)
    return 1.0;
  return exp(-(e2 - e1) / temp);
}

template <typename T>
std::vector<int> SimmulatedAnnealingPartitioner<T>::next_neighbour(
    const std::vector<int> &_sol, const int r, const int n_procs,
    const int iter_max
)
{
  size_t num_items = this->items.size();

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> item_chooser(0, num_items - 1);
  std::uniform_int_distribution<int> proc_picker(0, n_procs - 1);

  std::vector<int> new_sol = _sol;
  int idx = item_chooser(gen);
  int new_proc = proc_picker(gen);
  new_sol[idx] = new_proc;

  // for (int iter = 0; iter < iter_max; ++iter)
  // {
  //   std::vector<int> changed_items(num_items, 0);
  //   for (int i = 0; i < r; ++i)
  //     changed_items.at(item_chooser(gen)) = proc_picker(gen);
  //   for (size_t i = 0; i < num_items; ++i)
  //   {
  //     if (changed_items[i])
  //       new_sol[i] = changed_items[i];
  //   }
  //   if (this->objective_function(this->items, _sol) >
  //       this->objective_function(this->items, new_sol))
  //     break;
  // }

  return new_sol;
}

template <typename T>
std::unordered_map<int, std::vector<T *>>
SimmulatedAnnealingPartitioner<T>::partition_items(
    const int n_procs, const int k_max, const int r, const double init_temp,
    const double temp_threshold, const double cooling_rate, bool debug
)
{
  std::vector<int> best_sol(this->items.size(), 0);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> rand_gen(0.0, 1.0);

  std::uniform_int_distribution<int> rand_partition(0, n_procs - 1);
  std::uniform_real_distribution<double> rand_prob(0.0, 1.0);
  size_t num_items = this->items.size();

  // for (const auto &[id, item] : this->items)
  //   best_sol[id] = id % n_procs;
  // Initialize with random assignments like in the Python version
  for (size_t i = 0; i < num_items; ++i)
  {
    best_sol[i] = rand_partition(gen); // Random partition assignment
  }

  std::vector<int> sol(best_sol);
  double best_score = this->objective_function(this->items, best_sol);
  double sol_score = best_score;

  double temp = init_temp;

  while (temp > temp_threshold)
  {
    // Find next neighbour (equivalent to find_neighbour function in Python)
    std::vector<int> neighbour = this->next_neighbour(sol, r, n_procs);
    double new_score = this->objective_function(this->items, neighbour);
    double delta = new_score - sol_score;

    // Acceptance probability
    if (delta > 0 || rand_prob(gen) < std::exp(delta / temp))
    {
      sol = neighbour; // Accept the new solution
      sol_score = new_score;

      // Update best solution if necessary
      if (sol_score > best_score)
      {
        best_sol = sol;
        best_score = sol_score;
      }
    }

    // Cool down the temperature
    temp *= cooling_rate;

    // Debugging prints
    if (debug)
    {
      std::cout << "Best score: " << best_score
                << ", Current score: " << sol_score << ", Temperature: " << temp
                << std::endl;
    }
  }
  // int k = 0;
  // do
  // {
  //   double temp = 1 - (k + 1) / k_max;
  //   std::vector<int> neighbour = this->next_neighbour(best_sol, r, n_procs);
  //   if (this->acceptance_probability(
  //           this->objective_function(this->items, best_sol),
  //           this->objective_function(this->items, neighbour), temp
  //       ))
  //     best_sol = neighbour;
  //   ++k;
  //   if (k % 100 == 0)
  //     printf(
  //         "Iteración: %d\t Puntaje mejor solución: %lf\n", k,
  //         this->objective_function(this->items, best_sol)
  //     );

  // } while (k < k_max);

  std::unordered_map<int, std::vector<T *>> ret_val(n_procs);
  for (size_t i = 0; i < num_items; ++i)
    ret_val[best_sol[i]].push_back(this->items.at(i));
  return ret_val;
}
