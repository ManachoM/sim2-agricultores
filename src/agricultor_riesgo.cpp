#include "../includes/agricultor_riesgo.h"

#include <bsp.h> // Added for bsp_pid()
#include <random>

double calc_riesgo(Producto *prod, Environment *env, Terreno *terr);
AgricultorRiesgo::AgricultorRiesgo(
    FEL *_fel, Terreno *_terr, MercadoMayorista *_mer
)
    : Agricultor(_fel, _terr, _mer)
{
  // Initialize random generator with processor ID for better randomization
  std::random_device rd;
  // Get processor ID and use it to add variability to the seed
  int pid = bsp_pid();
  // Create a unique seed using processor ID, agricultor ID, and random device
  unsigned int seed = rd() + pid * 1000 + get_agricultor_id() * 10;
  gen = std::mt19937(seed);

  std::uniform_real_distribution<> dist(0.0, 1.0);
  double rand = dist(gen);
  this->seguro = (rand < INSURANCE_THRESHOLD);
}

const int AgricultorRiesgo::choose_product()
{ /***
   // Check if month is the same as when we last calculated
   if (this->current_month == this->env->get_month())
   {
     // If month hasn't changed, return cached result
     return this->best_candidate;
   }

   // Month has changed, need to recalculate
   this->current_month = this->env->get_month();
   auto prod_mes = this->env->get_siembra_producto_mes();
   auto lista_prods = prod_mes.find(this->env->get_month());
   Producto *prod_elegido;
   if (this->seguro)
   {
     prod_elegido = (*select_randomly(
         lista_prods->second.begin(), lista_prods->second.end()
     ));
     this->best_candidate = prod_elegido->get_id();
     return prod_elegido->get_id();
   }

   // Tomar tres con riesgo mejor
   auto scores = std::vector<std::pair<int, double>>();

   for (Producto *prod : lista_prods->second)
   {
     scores.push_back(
         {prod->get_id(), calc_riesgo(prod, this->env, this->get_terreno())}
     );
   }
   std::sort(
       scores.begin(), scores.end(),
       [](const auto &a, const auto &b) { return a.second < b.second; }
   );
   // elegir el que tenga precio mayor
   //
   std::vector<std::pair<int, double>> safest_three;
   for (const auto &p : scores)
   {
     safest_three.push_back(p);
     if (safest_three.size() == 3)
       break;
   }

   // Now, select the candidate with the highest score according to
   // evaluateCandidate
   auto bestCandidate = std::max_element(
       safest_three.begin(), safest_three.end(),
       [this](const std::pair<int, double> &a, const std::pair<int, double> &b)
       {
         return env->get_productos().at(a.first)->get_precios_mes().at(
                    env->get_month()
                ) <
                env->get_productos().at(b.first)->get_precios_mes().at(
                    env->get_month()
                );
       }
   );
   if (bestCandidate == safest_three.end())
   {
     this->best_candidate = safest_three.at(0).first;
     return safest_three.at(0).first;
   }

   this->best_candidate = bestCandidate->first;
   return bestCandidate->first;***/
  auto prod_mes = this->env->get_siembra_producto_mes();
  auto lista_prods = prod_mes.find(this->env->get_month());
  Producto *prod_elegido;
  if (this->seguro)
  {
    prod_elegido = (*select_randomly(
        lista_prods->second.begin(), lista_prods->second.end()
    ));
    return prod_elegido->get_id();
  }

  // Tomar tres con riesgo mejor
  auto scores = std::vector<std::pair<int, double>>();

  for (Producto *prod : lista_prods->second)
  {
    scores.push_back(
        {prod->get_id(), calc_riesgo(prod, this->env, this->get_terreno())}
    );
  }
  std::sort(
      scores.begin(), scores.end(),
      [](const auto &a, const auto &b) { return a.second < b.second; }
  );
  // elegir el que tenga precio mayor
  //
  std::vector<std::pair<int, double>> safest_three;
  for (const auto &p : scores)
  {
    safest_three.push_back(p);
    if (safest_three.size() == 3)
      break;
  }

  // Now, select the candidate with the highest score according to
  // evaluateCandidate
  auto bestCandidate = std::max_element(
      safest_three.begin(), safest_three.end(),
      [this](const std::pair<int, double> &a, const std::pair<int, double> &b)
      {
        return env->get_productos().at(a.first)->get_precios_mes().at(
                   env->get_month()
               ) <
               env->get_productos().at(b.first)->get_precios_mes().at(
                   env->get_month()
               );
      }
  );
  if (bestCandidate == safest_three.end())
    return safest_three.at(0).first;

  return bestCandidate->first;
}

double calc_riesgo(Producto *prod, Environment *env, Terreno *terr)
{
  /** double afectacion_sequias, afectacion_oc, afectacion_heladas,
       afectacion_plagas;

   afectacion_oc = env->get_nivel_olas_calor();
   afectacion_sequias = env->get_nivel_sequias();
   afectacion_plagas = terr->amenaza_plaga;
   afectacion_heladas = env->get_nivel_heladas();
   double NIVEL_AF_HELADAS = 0.3, NIVEL_AF_SEQUIAS = 0.3, NIVEL_AF_PLAGAS = 0.3,
          NIVEL_AF_OC = 0.1;

   int ptje_oc, ptje_heladas, ptje_sequias, ptje_plagas;

   ptje_oc = (afectacion_oc == 0)
                 ? 0
                 : prod->get_olas_calor().at((int)afectacion_oc - 1);

   ptje_heladas = (afectacion_heladas == 0)
                      ? 0
                      : prod->get_heladas().at((int)afectacion_heladas - 1);

   ptje_plagas = (afectacion_plagas == 0)
                     ? 0
                     : prod->get_plagas().at((int)afectacion_plagas - 1);

   ptje_sequias = (afectacion_sequias == 0)
                      ?: prod->get_sequias().at((int)afectacion_sequias - 1);

   return NIVEL_AF_SEQUIAS * afectacion_sequias * ptje_sequias +
          NIVEL_AF_HELADAS * afectacion_heladas * ptje_heladas +
          NIVEL_AF_OC * afectacion_oc * ptje_oc +
          NIVEL_AF_PLAGAS * afectacion_plagas * ptje_plagas;**/

  // Define weight constants
  constexpr double NIVEL_AF_HELADAS = 0.3;
  constexpr double NIVEL_AF_SEQUIAS = 0.3;
  constexpr double NIVEL_AF_PLAGAS = 0.3;
  constexpr double NIVEL_AF_OC = 0.1;

  // Retrieve environmental and terrain impact levels
  const double afectacion_oc = env->get_nivel_olas_calor();
  const double afectacion_sequias = env->get_nivel_sequias();
  const double afectacion_heladas = env->get_nivel_heladas();
  const double afectacion_plagas = terr->amenaza_plaga;

  // Helper lambda to retrieve the corresponding product score.
  // It returns 0 if the impact is 0, otherwise retrieves the score
  // from the appropriate vector.
  auto getScore = [](double afectacion, const auto &scores) -> int
  {
    return (afectacion == 0) ? 0 : scores[static_cast<size_t>(afectacion) - 1];
  };

  // Calculate scores for each impact factor
  const int ptje_oc = getScore(afectacion_oc, prod->get_olas_calor());
  const int ptje_heladas = getScore(afectacion_heladas, prod->get_heladas());
  const int ptje_plagas = getScore(afectacion_plagas, prod->get_plagas());
  const int ptje_sequias = getScore(afectacion_sequias, prod->get_sequias());

  // Combine the weighted contributions of each factor and return the risk
  return NIVEL_AF_SEQUIAS * afectacion_sequias * ptje_sequias +
         NIVEL_AF_HELADAS * afectacion_heladas * ptje_heladas +
         NIVEL_AF_OC * afectacion_oc * ptje_oc +
         NIVEL_AF_PLAGAS * afectacion_plagas * ptje_plagas;
}
