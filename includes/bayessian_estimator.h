#ifndef _BAYESSIAN_ESTIMATOR_H_
#define _BAYESSIAN_ESTIMATOR_H_

#include <vector>

/**
 * Bayesian estimator for predicting the success of purchase attempts.
 * 
 * This class implements a Bayesian estimator to predict whether a purchase
 * attempt (INIT_COMPRA_MAYORISTA) will be successful without waiting for
 * synchronization. The estimator tracks success/failure statistics for:
 * 1. Overall transactions
 * 2. Product-specific transactions
 * 3. Processor-specific transactions
 * 4. Joint product-processor combinations
 * 
 * It updates its knowledge based on actual outcomes and can make predictions
 * for future transactions.
 */
class BayessianEstimator
{
private:
  // Overall success/failure counters
  double total_success;
  double total_failures;

  // Maximum IDs for bounds checking
  int max_product_id;
  int max_proc_id;

  // Number of updates since last decay
  int update_count;
  
  // Decay parameters
  static constexpr double DECAY_FACTOR = 0.95;
  static constexpr int DECAY_FREQUENCY = 1000;

  // Product-specific statistics
  std::vector<double> product_success;
  std::vector<double> product_failures;
  
  // Processor-specific statistics
  std::vector<double> proc_success;
  std::vector<double> proc_failures;
  
  // Joint product-processor statistics for more accurate predictions
  std::vector<std::vector<double>> prod_proc_success;
  std::vector<std::vector<double>> prod_proc_failures;
  
  /**
   * Applies exponential decay to all statistics to give more weight to recent events.
   */
  void decay_statistics();

public:
  /**
   * Constructor initializes the estimator with prior probabilities.
   * 
   * @param max_prod Maximum product ID expected in the system
   * @param max_proc Maximum processor ID expected in the system
   */
  BayessianEstimator(int max_prod, int max_proc);
  
  /**
   * Updates the estimator based on actual outcome of a purchase attempt.
   * 
   * @param success Whether the purchase attempt was successful
   * @param product_id ID of the product involved
   * @param proc_id ID of the processor that handled the request
   */
  void update(bool success, int product_id, int proc_id);
  
  /**
   * Predicts the probability of success for a purchase attempt.
   * 
   * @param product_id ID of the product to purchase
   * @param proc_id ID of the processor that will handle the request
   * @return Probability of success (0.0-1.0)
   */
  double predict(int product_id, int proc_id) const;
  
  /**
   * Forces an immediate decay of all statistics.
   */
  void force_decay();
  
  /**
   * Adjusts the decay rate for the estimator.
   * 
   * @param decay_factor New decay factor (0-1), lower values decay faster
   */
  void set_decay_factor(double decay_factor);
};

#endif // !_BAYESSIAN_ESTIMATOR_H_
