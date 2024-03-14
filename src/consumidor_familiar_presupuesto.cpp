#include "../includes/consumidor_familiar_presupuesto.h"

ConsumidorFamiliarPresupuesto::ConsumidorFamiliarPresupuesto(int fam_size, int feria, double budget, FEL *fel = nullptr) : Consumidor(fel, feria),budget(budget), fam_size(fam_size)
{
}

std::vector<int> ConsumidorFamiliarPresupuesto::choose_product()
{
    int current_month = this->env->get_month();
    std::vector<Producto *> prods = this->env->get_venta_producto_mes().find(current_month)->second;
    // Validamos el presupuesto para este mes
    double presupuesto_mes;
    auto gasto_iter = this->gasto_mes.find(current_month);
    if (gasto_iter == this->gasto_mes.end())
    {
        this->gasto_mes.insert({current_month, 0.0});
        presupuesto_mes = this->budget;
    }
    else
        presupuesto_mes = this->budget - gasto_iter->second;

    std::vector<int> prods_to_buy;
    for (auto prod : prods)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> d(0.0, 1.0);

        // Si no cumple con la prob de consumo, o el prespuesto no le alcanza
        if (d(gen) > prod->get_probabilidad_consumo() ||
            presupuesto_mes < prod->get_precio_feria() * this->fam_size)
            continue;

        prods_to_buy.push_back(prod->get_id());
    }

    // Caso borde, no se pudo elegir ningún producto - excepción?
    return prods_to_buy;
}

double ConsumidorFamiliarPresupuesto::purchase_amount(const int prod_id)
{
    return this->fam_size * this->env->get_productos().at(prod_id)->get_volumen_consumidor();
}

Feriante *ConsumidorFamiliarPresupuesto::choose_feriante(const int prod_id, const double amount)
{
    std::map<int, Feriante *> feriantes = this->env->get_ferias().at(this->get_feria())->get_feriantes();
    for (auto feriante : feriantes)
    {
        // Si el feriante existe en nuestra lista de feriantes visitados, significa que ya intentamos comprarle
        if (count(this->feriantes_consultados.begin(), this->feriantes_consultados.end(), feriante.first))
            continue;

        this->feriantes_consultados.push_back(feriante.first);
        return feriante.second;
    }
    // Caso límite, levantar excepción (?) - TODO
    return nullptr;
}

void ConsumidorFamiliarPresupuesto::finish_purchase()
{
    this->feriantes_consultados.clear();
}
