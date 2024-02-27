#ifndef _FERIANTE_ESTATICO_H_
#define _FERIANTE_ESTATICO_H_

#include "feriante.h"

class FerianteEstatico : public Feriante
{
private: 
    std::vector<int> choose_product() override;
    double purchase_amount(const int prod_id) override;
    Agricultor *choose_agricultor(const int prod_id, const double amount) override;
    void finish_purchase();
    std::vector<int> agricultores_consultados;
    std::vector<int> prods_ids;

public:
    FerianteEstatico(FEL *fel, MercadoMayorista *mer, int feria_id);

};

#endif // !_FERIANTE_ESTATICO_H_