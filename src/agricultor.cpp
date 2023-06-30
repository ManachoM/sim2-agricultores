#include "../includes/agricultor.h"

Agricultor::Agricultor(FEL *_fel, Terreno *_terr, bool _seg) : fel(_fel), terreno(_terr), tiene_seguro(_seg)
{
}

void Agricultor::process_event(Event *e)
{
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
        log["agent_process"] = "CULTIVO_TERRENO";

        // Consultamos el índice
        auto lista_prods = this->env->get_siembra_producto_mes().find(this->env->get_month());

        if (lista_prods == this->env->get_siembra_producto_mes().end())
        {
            printf("ERROR EN SIEMBRA_PRODUCTO_MES\n");
            exit(EXIT_FAILURE);
        }

        // Elegimos el prod
        Producto *prod_elegido = (*select_randomly(lista_prods->second.begin(), lista_prods->second.end()));
        log["producto_elegido"] = prod_elegido->get_id();
        this->terreno->set_producto_plantado(prod_elegido->get_id());

        // Insertamos la cosecha
        this->fel->insert_event(
            prod_elegido->get_dias_cosecha()*24.0,
            AGENT_TYPE::AGRICULTOR,
            EVENTOS_AGRICULTOR::COSECHA,
            this->get_id(),
            Message(),
            this
        );
        break;
    }

    case EVENTOS_AGRICULTOR::COSECHA:
    {
        log["agent_process"] = "COSECHA";
        log["producto_cosechado"] = this->terreno->get_producto();
        log["cantidad_cosechada"] = this->terreno->get_area() * this->env->get_productos()[this->terreno->get_producto()]->get_rendimiento();
        this->inventario.emplace_back(
            e->get_time(),
            e->get_time() + (14*24),
            this->terreno->get_producto(),
            this->terreno->get_area() * this->env->get_productos()[this->terreno->get_producto()]->get_rendimiento()
        );

        this->fel->insert_event(
            24.0,
            AGENT_TYPE::AGRICULTOR,
            EVENTOS_AGRICULTOR::CULTIVO_TERRENO,
            this->get_id(),
            Message(),
            this
        );
        break;

    }
    }

    this->monitor->writeLog(log);
}