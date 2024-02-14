#ifndef _CONSUMIDOR_FACTORY_H_
#define _CONSUMIDOR_FACTORY_H_

#include "glob.h"
class Consumidor;
class FEL;
class Environment;
class Monitor;



class ConsumidorFactory
{
private:
    FEL *fel;
    Environment *env;
    Monitor *monitor;

public:
    ConsumidorFactory(FEL *_fel, Environment *_env, Monitor *_monitor);

    Consumidor *create_consumidor(std::string const &cons_type, const int feria_id);
};

#endif // !_CONSUMIDOR_FACTORY_H_