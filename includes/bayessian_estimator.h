#ifndef _BAYESSIAN_ESTIMATOR_H_
#define _BAYESSIAN_ESTIMATOR_H_

#include <vector>

class BayessianEstimator
{

private:
  int total_success;
  int total_failures;

  int max_product_id;
  int max_proc_id;

  std::vector<int> product_success;
  std::vector<int> product_failures;
  std::vector<int> proc_success;
  std::vector<int> proc_failures;

public:
  BayessianEstimator(int max_prod, int max_proc);
  void update(bool success, int product_id, int proc_id);
  double predict(int product_id, int proc_id) const;
};

#endif // !_BAYESSIAN_ESTIMATOR_H_
