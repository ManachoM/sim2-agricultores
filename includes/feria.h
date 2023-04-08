
#ifndef FERIA_H

#include "glob.h"
#include "fel.h"
#include "environment.h"

class Feriante;
class Environment;

class Feria
{

    private:
        int feriante_id;
        static int _current_id;
        std::map<int, Feriante *> feriantes;
        // TODO: Implementar con objeto de configuración
        FEL *fel;
        std::vector<int> dia_funcionamiento;
        Environment *env;

    public:
        Feria(std::map<int, Feriante *> _feriantes, std::vector<int> _dias);

};


#endif // !FERIA_H

