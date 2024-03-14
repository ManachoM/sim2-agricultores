#include "../includes/feriante_estacional_proporcional.h"

FerianteEstacionalProporcional::FerianteEstacionalProporcional(FEL *fel, MercadoMayorista *mer, int feria_id, int prod_amount) : FerianteEstatico(fel, mer, feria_id), prod_amount(prod_amount)
{
}

std::vector<int> FerianteEstacionalProporcional::choose_product()
{
    std::vector<int> prods_seleccionados;

    std::vector<Producto*> prods = this->env->get_venta_producto_mes().find(this->env->get_month())->second;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> d(0.0, 1.0);
    
    for (auto prod : prods)
    {
        if (d(gen) < prod->get_probabilidad_consumo())
            continue;
        prods_seleccionados.push_back(prod->get_id());

        if ((int) prods_seleccionados.size() >= this->prod_amount)
            break;
    }

    return prods_seleccionados;
}