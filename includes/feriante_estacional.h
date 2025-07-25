#ifndef _FERIANTE_ESTACIONAL_H_
#define _FERIANTE_ESTACIONAL_H_

#include "glob.h"
#include "feriante_estatico.h"

class Environment;

class FerianteEstacional : public FerianteEstatico
{
private:
    std::vector<int> choose_product() override;

public:
    FerianteEstacional(Environment *_env, FEL *fel, MercadoMayorista *mer, int feria_id);
};

#endif // !_FERIANTE_ESTACIONAL_H_
