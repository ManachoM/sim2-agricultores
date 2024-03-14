#ifndef _FERIANTE_FACTORY_H_
#define _FERIANTE_FACTORY_H_

#include "glob.h"

class Feriante;
class FEL;
class Environment;
class MercadoMayorista;
class Monitor;

class FerianteFactory
{
private:
    FEL *fel;
    Environment *env;
    Monitor *monitor;
    MercadoMayorista *mer;

public:
    FerianteFactory(FEL *_fel, Environment *_env, Monitor *_monitor, MercadoMayorista *_mer);

    Feriante *create_feriante(std::string const &feriante_type, const int feria_id, const int prod_amount = 5);
};

#endif // !_FERIANTE_FACTORY_H_