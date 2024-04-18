#include "../includes/mercado_mayorista.h"

MercadoMayorista::MercadoMayorista(Environment *_env) : env(_env){
                                                            // auto agrs = this->env->get_agricultores();
                                                            // if ((int) agrs.size() <= 0)
                                                            //     return;
                                                            // this->reset_index();
                                                        };

void MercadoMayorista::update_index(int agr_id, int prod_id, bool inv)
{
    if (agr_id < 0 || prod_id < 0)
        return;
    // this->prod_mat[agr_id][prod_id] = inv;
    // std::cout << "MATRIX PTR: " << &this->prod_mat << " - DIMS: " << this->prod_mat.num_rows << " x " << this->prod_mat.num_cols << std::endl;
    this->prod_mat.set(agr_id, prod_id, inv);
}

void MercadoMayorista::reset_index()
{
    auto agros = this->env->get_agricultores();

    auto prods = this->env->get_productos();
    // std::cout << "agros " << agros.size() << "  prods: " << prods.size() << std::endl;
    // Generamos una nueva matriz de índices
    auto new_mat =BooleanMatrix((int)agros.size(), (int)prods.size());
    // this->prod_mat = BooleanMatrix((int)agros.size(), (int)prods.size());

    // Para cada agricultor, obtenemos su lista de productos
    for (auto agr : agros)
    {
        std::map<int, Inventario> inv = agr.second->get_inventory();
        for (auto const &el : inv)
        {
            // this->prod_mat[agr.first][el.second.get_product_id()] = true;
            new_mat.set(agr.first, el.second.get_product_id(), true);
        }
    }
    this->prod_mat = new_mat;
    // std::cout << "Dimensiones nueva matriz" << this->prod_mat.num_rows << " x " << this->prod_mat.num_cols << "\n" << std::endl;
}

std::vector<int> MercadoMayorista::get_agricultor_por_prod(int prod_id)
{
    std::vector<int> agricultores(0);
    for (int i = 0; i < this->prod_mat.num_rows; ++i)
    {
        // printf("i: %d, prod_id %d\t", i, prod_id);
        bool entry = false;
        try
        {
        //     if (!this->prod_mat[i][prod_id])
        //         std::cout << "i: " << i << std::endl;
            // entry = this->prod_mat[i][prod_id];
            entry = this->prod_mat.get(i, prod_id);
        }
        catch (const std::out_of_range &e)
        {
            // printf("error: i: %d, prod_id %d\t", i, prod_id);
            std::cerr << e.what() << '\n';
        }
        if (!entry)
            continue;
        agricultores.push_back(i);
    }

    return agricultores;
}