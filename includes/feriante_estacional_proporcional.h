#ifndef _FERIANTE_ESTACIONAL_PROPORCIONAL_
#define _FERIANTE_ESTACIONAL_PROPORCIONAL_

#include "feriante_estatico.h"
#include "glob.h"

#include <random>

class Environment;

class FerianteEstacionalProporcional : public FerianteEstatico
{
private:
  std::vector<int> choose_product() override;
  Agricultor *choose_agricultor(const int prod_id, const double amount) override;
  void finish_purchase() override;
  
  int prod_amount;
  std::vector<int> agricultores_consultados;

  // Persistent random generator for better performance
  std::mt19937 gen;
  std::uniform_real_distribution<double> dist;

public:
  FerianteEstacionalProporcional(
      Environment *_env, FEL *fel, MercadoMayorista *mer, int feria_id,
      int prod_amount
  );
};

#endif // !_FERIANTE_ESTACIONAL_PROPORCIONAL_
