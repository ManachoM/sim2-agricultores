#include "../includes/agricultor.h"

int Agricultor::curr_agricultor_id(-1);

Agricultor::Agricultor(FEL *_fel, Terreno *_terr) : agricultor_id(++curr_agricultor_id), fel(_fel), terreno(_terr)
{
    this->fel->insert_event(
        0.0,
        AGENT_TYPE::AGRICULTOR,
        EVENTOS_AGRICULTOR::CULTIVO_TERRENO,
        this->get_id(),
        Message(),
        this);
}

void Agricultor::process_event(Event *e)
{
    printf("Procesando agricultor...");
    json log;
    log["agent_type"] = "AGRICULTOR";
    log["time"] = e->get_time();
    log["day"] = this->env->get_day_month();
    log["month"] = this->env->get_month();
    log["year"] = this->env->get_year();
    log["agent_id"] = this->get_id();

    switch (e->get_process())
    {
    case EVENTOS_AGRICULTOR::CULTIVO_TERRENO:
    {
        this->process_cultivo_event(e, log);
        break;
    }
    case EVENTOS_AGRICULTOR::COSECHA:
    {
        this->process_cosecha_event(e, log);
        break;
    }
    case EVENTOS_AGRICULTOR::VENTA_FERIANTE:
    {
        this->process_venta_feriante_event(e);
        break;
    }
    case EVENTOS_AGRICULTOR::INVENTARIO_VENCIDO:
    {
        this->process_inventario_vencido_event(e, log);
        break;
    }
    default:
    {
        printf("Procesando agricultor...");

        break;
    }
    }

    this->monitor->write_log(log);
}

void Agricultor::process_cultivo_event(const Event *e, json log)
{
    log["agent_process"] = "CULTIVO_TERRENO";

    int prod_id = this->choose_product();

    log["producto_elegido"] = prod_id;

    Producto const *prod_elegido = this->env->get_productos().at(prod_id);
    this->terreno->set_producto_plantado(prod_id);

    // Insertamos la cosecha
    this->fel->insert_event(
        prod_elegido->get_dias_cosecha() * 24.0,
        AGENT_TYPE::AGRICULTOR,
        EVENTOS_AGRICULTOR::COSECHA,
        this->get_id(),
        Message(),
        this);
}

void Agricultor::process_cosecha_event(const Event *e, json log)
{
    std::cout << "Procesando cosecha...";
    log["agent_process"] = "COSECHA";

    Producto const *prod = this->env->get_productos()[this->terreno->get_producto()];
    double cantidad_cosechada = this->terreno->get_area() * prod->get_rendimiento();
    log["producto_cosechado"] = prod->get_id();
    log["cantidad_cosechada"] = cantidad_cosechada;
    this->inventario.insert({prod->get_id(), Inventario(
                                                 e->get_time(),
                                                 e->get_time() + (14 * 24), // Damos 14 días de vida útil al inventario
                                                 prod->get_id(),
                                                 cantidad_cosechada)});

    this->fel->insert_event(
        24.0,
        AGENT_TYPE::AGRICULTOR,
        EVENTOS_AGRICULTOR::CULTIVO_TERRENO,
        this->get_id(),
        Message(),
        this);

    std::map<std::string, double> content = {{"prod_id", prod->get_id()}};

    this->fel->insert_event(
        14 * 24,
        AGENT_TYPE::AGRICULTOR,
        EVENTOS_AGRICULTOR::VENTA_FERIANTE,
        this->get_id(),
        Message(content),
        this);
}

void Agricultor::process_venta_feriante_event(const Event *e)
{
    Message msg = e->get_message();

    int prod_id = (int)msg.msg.at("prod_id");
    double amount = msg.msg.at("amount");
    Inventario inv = this->inventario.at(prod_id);
    double quantity = inv.get_quantity();
    int buyer_id = (int)msg.msg.at("buyer_id");
    if (!inv.is_valid_inventory() || quantity < amount)
    {
        msg.msg.insert({"error", -1});
        this->fel->insert_event(
            0.0,
            AGENT_TYPE::FERIANTE,
            EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA,
            buyer_id,
            msg,
            nullptr);
        return;
    }

    inv.set_quantity(quantity - amount);
    this->inventario.at(prod_id) = inv;
    msg.msg.insert({"seller_id", this->get_agricultor_id()});
    this->fel->insert_event(
        0.0,
        AGENT_TYPE::FERIANTE,
        EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA,
        buyer_id,
        msg,
        nullptr);
    return;
}

void Agricultor::process_inventario_vencido_event(const Event *e, json log)
{
    log["EVENT_TYPE"] = "INVENTARIO_VENCIDO";

    Message msg = e->get_message();
    int prod_id = (int)msg.msg.at("prod_id");
    double amount = this->inventario.at(prod_id).get_quantity();

    this->inventario[prod_id] = Inventario();
    this->mercado->update_index(this->get_agricultor_id(), prod_id, false);

    log["prod_id"] = prod_id;
    log["amount"] = amount;
}

std::map<int, Inventario> Agricultor::get_inventory() const
{
    return this->inventario;
}

int Agricultor::get_agricultor_id() const
{
    return this->agricultor_id;
}