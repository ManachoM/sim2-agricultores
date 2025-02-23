#ifndef _AGRICULTOR_RIESGO_H_
#define _AGRICULTOR_RIESGO_H_

#include "agricultor.h"
#include "mercado_mayorista.h"

#define INSURANCE_THRESHOLD 0.06

class AgricultorRiesgo : public Agricultor {
private:
  const int choose_product() override;
  bool seguro = false;

public:
  AgricultorRiesgo(FEL *_fel, Terreno *_terr, MercadoMayorista *_mer);
};

#endif // !_AGRICULTOR_RIESGO_H_
