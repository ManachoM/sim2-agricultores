#ifndef _AGRICULTOR_RIESGO_H_
#define _AGRICULTOR_RIESGO_H_

#include "agricultor.h"
#include "mercado_mayorista.h"
#include <random>
#include <vector>
#include <utility>

#define INSURANCE_THRESHOLD 0.06

class AgricultorRiesgo : public Agricultor
{
private:
  const int choose_product() override;
  bool seguro = false;
  int current_month = 0;
  int best_candidate = 0;
  
  // Cache calculated risks to avoid recalculations
  std::vector<std::pair<int, double>> cached_scores;

  // Persistent random generator
  std::mt19937 gen;

public:
  AgricultorRiesgo(FEL *_fel, Terreno *_terr, MercadoMayorista *_mer);
};

#endif // !_AGRICULTOR_RIESGO_H_
