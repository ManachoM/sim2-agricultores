#include "../includes/bayessian_estimator.h"

BayessianEstimator::BayessianEstimator(int max_prod, int max_proc)
    : total_success(0), total_failures(0), max_product_id(max_prod),
      max_proc_id(max_proc), product_success(max_prod + 1, 0),
      product_failures(max_prod + 1, 0), proc_success(max_proc + 1, 0),
      proc_failures(max_proc + 1, 0)
{
}

void BayessianEstimator::update(bool success, int product_id, int proc_id)
{
  if (success)
  {
    ++this->total_success;
    //++provider_success[provider_id];
    ++product_success[product_id];
    ++proc_success[product_id];
  }
  else
  {
    ++total_failures;
    //++provider_failures[provider_id];
    ++product_failures[product_id];
    ++proc_failures[proc_id];
  }
}

double BayessianEstimator::predict(int product_id, int proc_id) const
{
  double alpha = 1.0;

  double prior_successes =
      (total_success + alpha) / (total_success + total_failures + 2.0 * alpha);
  double prior_failures = 1.0 - prior_successes;

  /***
  double prov_suc =
      (provider_success[provider_id] + alpha) / (total_success + 2.0 * alpha);
  double prov_fail =
      (provider_failures[provider_id] + alpha) / (total_failures + 2.0 * alpha);
  ***/
  double prod_suc =
      (product_success[product_id] + alpha) / (total_success + 2.0 * alpha);
  double prod_fail =
      (product_failures[product_id] + alpha) / (total_failures + 2.0 * alpha);

  double proc_suc =
      (proc_success[proc_id] + alpha) / (total_success + 2.0 * alpha);
  double proc_fail =
      (proc_failures[proc_id] + alpha) / (total_failures + 2.0 * alpha);

  // Aquí aplicamos la hipótesis ingenua
  double prob_sucess = prior_successes * prod_suc * proc_suc;
  double prob_faile = prior_failures * prod_fail * proc_fail;
  double denom = prob_sucess + prob_faile;

  if (denom <= 1e-15)
    return prior_successes;

  return prior_successes / denom;
}
