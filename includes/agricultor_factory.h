#ifndef _AGRICULTOR_FACTORY_
#define _AGRICULTOR_FACTORY_

#include "glob.h"

class Agricultor;
class FEL;
class Terreno;
class MercadoMayorista;
class Environment;
class Monitor;

class AgricultorFactory
{
    private:
    FEL *fel;
    Environment *env;
    Monitor *monitor;
    MercadoMayorista *mer;

    public:
    AgricultorFactory(FEL *_fel, Environment *_env, Monitor *_monitor, MercadoMayorista *_mer);

    Agricultor *create_agricultor(std::string const &agr_type, Terreno *terr);
};

#endif // !_AGRICULTOR_FACTORY_