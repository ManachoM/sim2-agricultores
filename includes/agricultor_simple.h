#ifndef _AGRICULTOR_SIMPLE_H_
#define _AGRICULTOR_SIMPLE_H_

#include "agricultor.h"
#include "glob.h"

class AgricultorSimple : public Agricultor
{
    private:
    const int choose_product() override;

    public:
    AgricultorSimple(FEL *_fel, Terreno *_ter);
};

#endif // !_AGRICULTOR_SIMPLE_H_