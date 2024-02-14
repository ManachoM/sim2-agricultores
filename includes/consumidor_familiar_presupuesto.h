#ifndef _CONSUMIDOR_FAMILIAR_PRESUPUESTO_H_
#define _CONSUMIDOR_FAMILIAR_PRESUPUESTO_H_

#include "consumidor.h"
#include "feriante.h"
#include "glob.h"

class Feriante;

class ConsumidorFamiliarPresupuesto : public Consumidor
{
private:
    int choose_product() override;
    double purchase_amount(const int prod_id) override;
    Feriante *choose_feriante(const int prod_id, const double amount) override;
    void finish_purchase() override;
    double budget;
    int fam_size;
    std::map<int, double> gasto_mes;
    std::vector<int> feriantes_consultados;

public:
    ConsumidorFamiliarPresupuesto(int fam_size, int feria, double budget, FEL *fel);
};

#endif // !_CONSUMIDOR_FAMILIAR_PRESUPUESTO_H_