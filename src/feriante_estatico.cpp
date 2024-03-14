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

std::vector<int> FerianteEstatico::choose_product()
{
    return this->prods_ids;
}

double FerianteEstatico::purchase_amount(const int prod_id)
{
    return this->env->get_productos().at(prod_id)->get_volumen_feriante();
}

Agricultor *FerianteEstatico::choose_agricultor(const int prod_id, const double amount)
{
    std::vector<int> agricultores = this->mercado->get_agricultor_por_prod(prod_id);

    for (auto agr : agricultores)
    {
        // Si ya intentamos comprarle al agricultor, seguimos
        if (count(this->agricultores_consultados.begin(), this->agricultores_consultados.end(), agr))
            continue;

        this->agricultores_consultados.push_back(agr);

        //? Esto podría ser caro eventualmente (?) ==> revisar por arreglo estático
        return this->env->get_agricultores().at(agr);
    }
    // En volaa levantar excepción?
    return nullptr;
}

void FerianteEstatico::finish_purchase()
{
    this->agricultores_consultados.clear();
}