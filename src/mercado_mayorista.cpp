#include "../includes/mercado_mayorista.h"

MercadoMayorista::MercadoMayorista(Environment *_env) : env(_env)
{
    auto agrs = this->env->get_agricultores();
    if (agrs.size() <= 0)
        return;
    this->reset_index();
};

void MercadoMayorista::update_index(int agr_id, int prod_id, bool inv)
{
    if (agr_id < 0 || prod_id < 0)
        return;
    this->prod_mat[agr_id][prod_id] = inv;
}

void MercadoMayorista::reset_index()
{
    auto agros = this->env->get_agricultores();

    auto prods = this->env->get_productos();

    // Generamos una nueva matriz de índices
    this->prod_mat = BooleanMatrix((int)agros.size(), (int)prods.size());

    // Para cada agricultor, obtenemos su lista de productos
    for (auto agr : agros)
    {
        std::vector<Inventario> inv = agr.second->get_inventory();
        for (auto const& el : inv)
        {
            this->prod_mat[agr.first][el.get_product_id()] = true;
        }
    }
}