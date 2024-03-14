#ifndef _FERIANTE_ESTACIONAL_PROPORCIONAL_
#define _FERIANTE_ESTACIONAL_PROPORCIONAL_

#include "glob.h"
#include "feriante_estatico.h"

class FerianteEstacionalProporcional : public FerianteEstatico
{
    private:
        std::vector<int> choose_product() override;
        int prod_amount;

    public:
        FerianteEstacionalProporcional(FEL *fel, MercadoMayorista *mer, int feria_id, int prod_amount); 

};

#endif // !_FERIANTE_ESTACIONAL_PROPORCIONAL_