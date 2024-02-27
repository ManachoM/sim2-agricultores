#include "../includes/feriante_estatico.h"

FerianteEstatico::FerianteEstatico(FEL *fel, MercadoMayorista *mer, int feria_id) : Feriante(fel, mer, feria_id)
{
    std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(4, 12); // Para la cantidad de productos a vender
    
    std::vector<int> prod_ids;
    std::map<int, Producto *> arr = this->env->get_productos();
    for (auto it = arr.begin(); it != arr.end(); ++it)
    {
        prod_ids.push_back(it->first);
    }
    shuffle(prod_ids.begin(), prod_ids.end(), gen);
    int num_productos = dist(gen);
    for (int i = 0; i <= num_productos; ++i)
    {
        this->prods_ids.push_back(prod_ids[i]);
    }
}


int FerianteEstatico::choose_product()
{
    
}