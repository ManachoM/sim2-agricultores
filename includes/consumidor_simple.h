#ifndef _CONSUMIDOR_SIMPLE_H_
#define _CONSUMIDOR_SIMPLE_H_

#include "consumidor.h"
#include "glob.h"

class Feriante;

class ConsumidorSimple : public Consumidor
{
private:
    int choose_product() override;
    double purchase_amount(const int prod_id) override;
    Feriante *choose_feriante(const int prod_id, const double amount) override;
    void finish_purchase() override;
    std::vector<int> feriantes_consultados; // TODO revisar implementaciones alternativas - `(unordered_)set`??

public:
    double last_purchase_amount;
    ConsumidorSimple(int _feria, FEL *fel = nullptr);
};

#endif // !_CONSUMIDOR_SIMPLE_H_