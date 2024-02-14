#ifndef _CONSUMIDOR_FAMILIAR_H_
#define _CONSUMIDOR_FAMILIAR_H_

#include "consumidor_simple.h"
#include "glob.h"

class Feriante;

class ConsumidorFamiliar : public ConsumidorSimple
{
private:
    double purchase_amount(const int prod_id) override;
    int fam_size;

public:
    ConsumidorFamiliar(int fam_size, int _feria, FEL *fel = nullptr);
};

#endif // !_CONSUMIDOR_FAMILIAR_H_