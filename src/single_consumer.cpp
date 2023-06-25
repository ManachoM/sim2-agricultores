#include "../includes/single_consumer.h"

int SingleConsumer::current_cons_id(-1);

SingleConsumer::SingleConsumer(FEL *_fel, int _fer) : fel(_fel), id_feria(_fer)
{
    // Inicializamos la memoria para cada producto
    for (long unsigned int _ = 0; _ < this->env->get_productos().size(); ++_)
    {
        this->feriantes_consultados.emplace_back();
    }
}
void SingleConsumer::process_event(Event *e)
{
    // Generamos el evento a loggear
    json log;
    log["agent_type"] = "CONSUMIDOR";
    log["agent_id"] = this->get_id();
    log["time"] = e->get_time();
    log["day"] = this->env->get_day_month();
    log["month"] = this->env->get_month();
    log["year"] = this->env->get_year();

    // Para cada tipo de evento
    switch (e->get_process())
    {
    // Buscamos feriantes
    case EVENTOS_CONSUMIDOR::BUSCAR_FERIANTE:
    {
        log["agent_process"] = "BUSCAR_FERIANTE";
        log["target_product"] = e->get_message().msg["TARGET_PROD"];
        log["target_amount"] = e->get_message().msg["TARGET_AMMOUNT"];

        int target_prod = e->get_message().msg["TARGET_PROD"];
        // Extraemos los feriantes
        auto feriantes = this->feria->get_feriantes();
        for (auto feriante = feriantes.begin(); feriante != feriantes.end(); ++feriante)
        {
            auto inv = feriante->second->get_inventario_by_id(target_prod);
            // Si el inventario no es válido, seguimos con nuestra vida
            if (!inv.is_valid_inventory())
                continue;

            std::vector<int> banned_feriantes = this->feriantes_consultados.at(target_prod);
            // Si encontramos al feriante en la lista de visitados, continuamos
            if (std::find(banned_feriantes.begin(), banned_feriantes.end(), feriante->second->get_id()) != banned_feriantes.end())
            {
                continue;
            }
            auto msg = e->get_message();
            msg.msg.insert({"BUYER_ID", this->get_id()});
            this->fel->insert_event(0.0,
                                    AGENT_TYPE::FERIANTE,
                                    EVENTOS_FERIANTE::VENTA_CONSUMIDOR,
                                    feriante->second->get_id(),
                                    msg,
                                    feriante->second);
            break;
        }
        break;
    }
    // El feriante nos respondió
    case EVENTOS_CONSUMIDOR::FIN_COMPRA_FERIANTE:
    {
        log["agent_process"] = "FIN_COMPRA_FERIANTE";
        log["target_product"] = e->get_message().msg["TARGET_PROD"];
        log["target_amount"] = e->get_message().msg["TARGET_AMMOUNT"];
        log["feriante_id"] = e->get_message().msg["FERIANTE_ID"];

        int target_prod = e->get_message().msg["TARGET_PROD"];
        double target_amount = e->get_message().msg["TARGET_AMMOUNT"];

        if(!e->get_message().msg["IS_SUCCESSFUL"])
        {
            // Añadimos el feriante a la memoria y seguimos buscando
            auto banned_fers = this->feriantes_consultados.at(target_prod);
            banned_fers.push_back(e->get_message().msg["FERIANTE_ID"]);
            std::map<std::string, double> msg;
            msg.insert({"TARGET_PROD", target_prod});
            msg.insert({"TARGET_AMMOUNT", target_amount});
            if (banned_fers.size() < this->feria->get_feriantes().size())
                this->fel->insert_event(0.0, AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::BUSCAR_FERIANTE, this->get_id(), Message(msg), this);
            return;
        }
        else
        {
            // Si logramos comprar, eliminamos la lista de feriantes consultados para el producto dado
            this->feriantes_consultados[target_prod] = std::vector<int>();
        }

        break;
    }
    default:
        break;
    }
    this->monitor->writeLog(log);
}

void SingleConsumer::set_feria(int _feria_id) { this->id_feria = _feria_id; }

int SingleConsumer::get_feria() const { return this->id_feria; }

int SingleConsumer::get_consumer_id() const { return this->consumer_id; }

void SingleConsumer::initialize_purchase()
{
    // We generate a time for purchase
    std::uniform_real_distribution<> h(6.0, 14.0);
    std::map<int, Producto *> arr = this->env->get_productos();
    std::vector<Producto *> prods = this->env->get_venta_producto_mes().find(this->env->get_month())->second;
    for (long unsigned int i = 0; i < prods.size(); ++i)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> d(0.0, 1.0);
        Producto const *p = prods.at(i);
        double val = d(gen);

        if (val <= p->get_probabilidad_consumo())
        {
            double generatedTime = h(gen);
            std::map<std::string, double> msg;
            msg.insert({"TARGET_PROD", p->get_id()});
            msg.insert({"TARGET_AMMOUNT", p->get_volumen_consumidor()});
            this->fel->insert_event(generatedTime + this->fel->get_time(), AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::BUSCAR_FERIANTE, this->get_id(), Message(msg), this);
        }
    }
}