#ifndef _SIMMULATED_ANNEALING_PARTITIONER_H_
#define _SIMMULATED_ANNEALING_PARTITIONER_H_
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>
#include <unordered_map>
#include <vector>

// Forward declarations
class Producto;
class Feria;

/**
 * @brief A generic Simulated Annealing-based partitioner for either Productos
 * or Ferias.
 *
 * @tparam T The type of item being partitioned (Producto or Feria).
 */
template <typename T> class SimmulatedAnnealingPartitioner
{
private:
  // All items, indexed by an integer ID
  std::unordered_map<int, T *> items;

  // Objective function pointer:
  //   - Takes the items map plus a solution (vector<int> with partition
  //   assignments)
  //   - Returns a double score (higher is better)
  std::function<
      double(const std::unordered_map<int, T *> &, const std::vector<int> &)>
      objective_function;

  // Flag to determine if items are productos (true) or ferias (false)
  bool is_productos;

public:
  /**
   * @brief Constructor for the partitioner.
   *
   * @param _items A map of (item_id -> item_ptr).
   * @param _objective_function A function to evaluate the quality of a
   * solution.
   */
  SimmulatedAnnealingPartitioner(
      const std::unordered_map<int, T *> &_items,
      std::function<
          double(const std::unordered_map<int, T *> &, const std::vector<int> &)>
          _objective_function
  );

  /**
   * @brief Computes the acceptance probability for simulated annealing.
   *
   * @param e1 Current energy (lower is better if we treat e as cost, or vice
   * versa).
   * @param e2 Neighbor energy.
   * @param temp Current temperature.
   * @return Probability of accepting the neighbor.
   */
  double acceptance_probability(double e1, double e2, double temp);

  /**
   * @brief Generates a neighbor solution from the current solution.
   *
   * For Productos, uses a single-item reassignment (generic).
   * For Ferias, uses a cluster-swap approach focusing on temporal overlap
   * ("TemporalFocal").
   *
   * @param _sol The current solution (vector<int> of partition assignments).
   * @param r Number of times to apply the operator.
   * @param n_procs Number of partitions.
   * @param iter_max Not used here, but can be used for more complex
   * neighborhoods.
   * @return A new solution (neighbor).
   */
  std::vector<int> next_neighbour(
      const std::vector<int> &_sol, const int r = 1, const int n_procs = 1,
      const int iter_max = 10
  );

  /**
   * @brief Main method that partitions items using Simulated Annealing.
   *
   * Automatically selects the best-known parameters for either Productos or
   * Ferias:
   *  - Productos: cooling_rate=0.99, r=2
   *  - Ferias: cooling_rate=0.98, r=1
   *
   * @param n_procs Number of partitions.
   * @param k_max Not used here; can be used for advanced operators.
   * @param r Default neighbor-operator parameter.
   * @param init_temp Initial temperature.
   * @param temp_threshold Temperature threshold below which we stop.
   * @param cooling_rate Geometric cooling factor (0 < cooling_rate < 1).
   * @param debug If true, prints debug info.
   * @return A map of (partition_id -> vector of item pointers).
   */
  std::unordered_map<int, std::vector<T *>> partition_items(
      const int n_procs, const int k_max = 0, const int r = 1,
      const double init_temp = 1000.0, const double temp_threshold = 1e-5,
      const double cooling_rate = 0.95, bool debug = false
  );
};

// Include the implementation
#include "simmulated_annealing_partitioner.tpp"
#endif // !_SIMMULATED_ANNEALING_PARTITIONER_H_
