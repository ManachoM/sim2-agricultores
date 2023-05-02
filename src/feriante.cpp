#include "../includes/feriante.h"

Feriante::Feriante(FEL *_fel, Environment *env, MercadoMayorista *_mer)
    : event_list(_fel), env(env), mercado(_mer)
{
}

void Feriante::process_event(Event *e)
{
    printf("Procesando evento feriante\n");
}

std::map<int, Inventario> Feriante::get_inventario() const { return this->inventario; }

Inventario Feriante::get_inventario_by_id(int _id_producto)
{
    auto aux = this->inventario.find(_id_producto);
    if (aux == this->inventario.end())
    {
        return Inventario(0.0, 0.0, _id_producto, 0.0);
    }
    return aux->second;
}

void Feriante::add_feria(Feria *_feria)
{
    auto search_result = this->ferias.find(_feria->get_id());
    if(search_result == this->ferias.end())
    {
        this->ferias.insert({_feria->get_id(), _feria});
        return;
    }
    search_result->second = _feria;
    return;
}

void Feriante::comprar_productos()
{
    // TODO
}

Feriante::~Feriante() = default;