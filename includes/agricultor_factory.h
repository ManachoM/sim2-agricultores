#ifndef _AGRICULTOR_FACTORY_
#define _AGRICULTOR_FACTORY_

#include "glob.h"

class Agricultor;
class FEL;
class Terreno;
class Environment;
class Monitor;

class AgricultorFactory
{
    private:
    FEL *fel;
    Environment *env;
    Monitor *monitor;

    public:
    AgricultorFactory(FEL *_fel, Environment *_env, Monitor *_monitor);

    Agricultor *create_agricultor(std::string const &agr_type, Terreno *terr);
};

#endif // !_AGRICULTOR_FACTORY_