#ifndef _SIMMULATED_ANNEALING_PARTITIONER_TPP_
#define _SIMMULATED_ANNEALING_PARTITIONER_TPP_

#include "simmulated_annealing_partitioner.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <random>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <iostream>

// Helper function for quick cluster-swap grouping (for Ferias)
template <typename T>
static std::unordered_map<int, std::vector<int>> build_clusters_for_ferias(
    const std::unordered_map<int, T*>& items,
    const std::vector<int>& current_solution,
    int n_procs
)
{
    // Example approach: cluster by each partition's "day overlap"
    // This is a simple, approximate "temporal focal" strategy
    // (You can refine or replace with a more sophisticated approach)
    std::unordered_map<int, std::vector<int>> partition_clusters;
    for (size_t idx = 0; idx < current_solution.size(); ++idx) {
        int part_id = current_solution[idx];
        partition_clusters[part_id].push_back((int)idx);
    }
    return partition_clusters;
}

//
// Constructor
//
template <typename T>
SimmulatedAnnealingPartitioner<T>::SimmulatedAnnealingPartitioner(
    const std::unordered_map<int, T*>& _items,
    std::function<double(const std::unordered_map<int, T*>&, const std::vector<int>&)>
        _objective_function
)
    : items(_items), objective_function(_objective_function)
{
    // Determine if we are working with Productos by template type
    is_productos = std::is_same<T, Producto>::value;
}

//
// Acceptance Probability
//
template <typename T>
double SimmulatedAnnealingPartitioner<T>::acceptance_probability(
    double e1, double e2, double temp
)
{
    // We assume a "maximize score" approach. If e2 > e1, we always accept.
    // Otherwise, accept with probability exp((e2 - e1)/temp).
    if (e2 >= e1) {
        return 1.0;
    }
    return std::exp((e2 - e1) / temp);
}

//
// Next Neighbour (Key difference between Productos & Ferias)
//
template <typename T>
std::vector<int> SimmulatedAnnealingPartitioner<T>::next_neighbour(
    const std::vector<int>& _sol,
    const int r,
    const int n_procs,
    const int /*iter_max*/
)
{
    size_t num_items = this->items.size();
    if (num_items == 0) {
        return _sol;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> item_chooser(0, (int)num_items - 1);
    std::uniform_int_distribution<int> proc_picker(0, n_procs - 1);

    // Make a copy of the solution to modify
    std::vector<int> new_sol = _sol;

    if (is_productos)
    {
        // BEST CONFIG FOR PRODUCTOS (BalanceTemporal, generic operator)
        // We'll simply do 'r' random single-item reassignments.
        for (int i = 0; i < r; ++i)
        {
            int idx = item_chooser(gen);
            int new_proc = proc_picker(gen);
            new_sol[idx] = new_proc;
        }
    }
    else
    {
        // BEST CONFIG FOR FERIAS (Tiered + TemporalFocal + ClusterSwap)
        // We'll do a simplified cluster swap focusing on daily overlap.

        // 1) Build clusters (by partition) from the current solution
        auto partition_clusters = build_clusters_for_ferias(this->items, _sol, n_procs);

        // 2) Pick two distinct partitions to swap from
        if (n_procs < 2) {
            return new_sol; // no swap possible if only one partition
        }

        std::vector<int> partition_ids;
        partition_ids.reserve(partition_clusters.size());
        for (auto& kv : partition_clusters) {
            if (!kv.second.empty()) {
                partition_ids.push_back(kv.first);
            }
        }
        if (partition_ids.size() < 2) {
            return new_sol; // no swap possible if fewer than 2 non-empty partitions
        }

        // We apply the cluster-swap 'r' times
        for (int swap_count = 0; swap_count < r; ++swap_count) {
            // Choose two partitions at random
            std::uniform_int_distribution<size_t> dist_part(0, partition_ids.size() - 1);
            size_t p1_idx = dist_part(gen);
            size_t p2_idx = dist_part(gen);
            while (p2_idx == p1_idx) {
                p2_idx = dist_part(gen);
            }

            int part1 = partition_ids[p1_idx];
            int part2 = partition_ids[p2_idx];

            // If either partition has no items, skip
            if (partition_clusters[part1].empty() || partition_clusters[part2].empty()) {
                continue;
            }

            // Pick a random cluster subset from each partition
            // (For simplicity, we just pick half of each partition's items)
            auto& cluster1 = partition_clusters[part1];
            auto& cluster2 = partition_clusters[part2];

            if (cluster1.empty() || cluster2.empty()) {
                continue;
            }

            // Shuffle cluster items to pick a random subset
            std::shuffle(cluster1.begin(), cluster1.end(), gen);
            std::shuffle(cluster2.begin(), cluster2.end(), gen);

            size_t half_size_1 = cluster1.size() / 2;
            size_t half_size_2 = cluster2.size() / 2;

            // Swap these subsets
            for (size_t i = 0; i < half_size_1; ++i) {
                int idx_item = cluster1[i];
                new_sol[idx_item] = part2;
            }
            for (size_t i = 0; i < half_size_2; ++i) {
                int idx_item = cluster2[i];
                new_sol[idx_item] = part1;
            }
        }
    }

    return new_sol;
}

//
// The Main SA Loop
//
template <typename T>
std::unordered_map<int, std::vector<T*>>
SimmulatedAnnealingPartitioner<T>::partition_items(
    const int n_procs,
    const int /*k_max*/,
    const int r,
    const double init_temp,
    const double temp_threshold,
    const double cooling_rate,
    bool debug
)
{
    // Select best-known SA parameters depending on whether we have productos or ferias
    double effective_init_temp     = init_temp;
    double effective_cooling_rate  = cooling_rate;
    int    effective_r             = r;

    // According to your “best config” details:
    //   - Productos -> "BalanceTemporal" + generic operator + cooling_rate=0.99 + r=2
    //   - Ferias -> "Tiered" + "TemporalFocal"/"ClusterSwap" + cooling_rate=0.98 + r=1
    if (is_productos) {
        effective_cooling_rate = 0.99;
        effective_r = 2;  // use a bigger 'r' for more single-item moves
    } else {
        effective_cooling_rate = 0.98;
        effective_r = 1;   // cluster-swap is heavier, so keep r=1
    }

    // Initialize random engine
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rand_partition(0, n_procs - 1);
    std::uniform_real_distribution<double> rand_prob(0.0, 1.0);

    // Create an initial random solution
    size_t num_items = this->items.size();
    std::vector<int> best_sol(num_items, 0);
    for (size_t i = 0; i < num_items; ++i) {
        best_sol[i] = rand_partition(gen);
    }

    // Copy as our current solution
    std::vector<int> sol = best_sol;

    // Evaluate
    double best_score = this->objective_function(this->items, best_sol);
    double sol_score  = best_score;

    // Simulated Annealing parameters
    double temp = effective_init_temp;
    int non_improving_iterations = 0;
    const int restart_threshold = 500; // or tune as needed

    // For simple adaptive cooling
    int accepted_moves = 0;
    int total_moves    = 0;
    const int adapt_interval = 100;

    // Main loop
    while (temp > temp_threshold)
    {
        // 1) Generate neighbor
        std::vector<int> neighbour = this->next_neighbour(sol, effective_r, n_procs);

        // 2) Evaluate neighbor
        double new_score = this->objective_function(this->items, neighbour);
        double ap = acceptance_probability(sol_score, new_score, temp);

        bool accept = (rand_prob(gen) < ap);
        if (accept) {
            sol       = neighbour;
            sol_score = new_score;
            accepted_moves++;

            // Update best if improved
            if (sol_score > best_score) {
                best_sol    = sol;
                best_score  = sol_score;
                non_improving_iterations = 0;
            } else {
                non_improving_iterations++;
            }
        } else {
            non_improving_iterations++;
        }

        total_moves++;

        // 3) Adaptive cooling for ferias only
        if (!is_productos && (total_moves % adapt_interval == 0)) {
            double acceptance_ratio = double(accepted_moves) / double(adapt_interval);

            // If acceptance ratio is too high or too low, adjust
            if (acceptance_ratio > 0.8) {
                // Cool faster
                effective_cooling_rate *= 0.9;
            } else if (acceptance_ratio < 0.1) {
                // Cool slower
                effective_cooling_rate *= 1.1;
                if (effective_cooling_rate > 0.999) {
                    effective_cooling_rate = 0.999;
                }
            }
            accepted_moves = 0;
        }

        // 4) Update temperature
        temp *= effective_cooling_rate;

        // 5) Simple “restart” if no improvement for a while (Ferias only)
        if (!is_productos && non_improving_iterations >= restart_threshold) {
            if (debug) {
                std::cout << "[DEBUG] Restart triggered. No improvements for "
                          << non_improving_iterations << " iterations.\n";
            }
            // Perturb the best solution
            std::vector<int> new_assignments(best_sol);
            for (size_t i = 0; i < num_items; ++i) {
                if (rand_prob(gen) < 0.3) {
                    new_assignments[i] = rand_partition(gen);
                }
            }
            sol = new_assignments;
            sol_score = this->objective_function(this->items, sol);
            temp = effective_init_temp * 0.5; // reduce initial temp
            non_improving_iterations = 0;
            accepted_moves = 0;
            total_moves = 0;
        }

        if (debug && (total_moves % 500 == 0)) {
            std::cout << "[DEBUG] Iteration: " << total_moves
                      << ", Temp: " << temp
                      << ", Current Score: " << sol_score
                      << ", Best Score: " << best_score << "\n";
        }
    }

    // Build the final partition map
    std::unordered_map<int, std::vector<T*>> partition_map;
    for (int i = 0; i < n_procs; ++i) {
        partition_map[i] = {};
    }

    for (size_t i = 0; i < best_sol.size(); ++i) {
        partition_map[best_sol[i]].push_back(items.at((int)i));
    }

    if (debug) {
        std::cout << "[DEBUG] Final best score: " << best_score << std::endl;
    }
    return partition_map;
}

#endif // !_SIMMULATED_ANNEALING_PARTITIONER_TPP_
