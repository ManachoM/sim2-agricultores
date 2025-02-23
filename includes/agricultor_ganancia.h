#ifndef _AGRICULTOR_GANANCIA_H_
#define _AGRICULTOR_GANANCIA_H_

#include "agricultor.h"
#include "glob.h"

class AgricultorGanancia : public Agricultor {
private:
  const int choose_product() override;

public:
  AgricultorGanancia(FEL *_fel, Terreno *_terr, MercadoMayorista *_mer);
};

#endif // !_AGRICULTOR_GANANCIA_H_
