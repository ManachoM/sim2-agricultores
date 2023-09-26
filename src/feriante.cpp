#include "../includes/feriante.h"

int Feriante::current_feriante_id(-1);
std::string Feriante::feriante_mode("static");
bool Feriante::mode_set(false);

Feriante::Feriante(FEL *_fel, Environment *env, MercadoMayorista *_mer)
    : feriante_id(++current_feriante_id), event_list(_fel), env(env), mercado(_mer)
{
    if (Feriante::feriante_mode == "static")
    {
        auto prods = this->env->get_productos();
        // Conocemos los valores de los ids de los productos, así que
        // simplemente elegimos una secuencia aleatoria desde ellos

        std::vector<int> prd_ids(prods.size());
        std::iota(std::begin(prd_ids), std::end(prd_ids), std::begin(prods)->first);

    
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(prd_ids.begin(), prd_ids.end(), rng);

        json config = SimConfig::get_instance("")->get_config();
        int prod_amount = config["feriantes_prods_amount"].get<int>();
        for(int el = 0; el < prod_amount; ++el)
        {
            this->productos.push_back(prd_ids[el]);
        }
    }
}

void Feriante::set_feriante_mode(std::string &mode)
{
    if (Feriante::mode_set)
        return;
    Feriante::mode_set = true;
    Feriante::feriante_mode = mode;
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
    if (search_result == this->ferias.end())
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