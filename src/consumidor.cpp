#include "../includes/consumidor.h"

int Consumidor::current_consumer_id(-1);

Consumidor::Consumidor(FEL *_fel, int _feria)
    : fel(_fel), id_feria(_feria)
{
}

void Consumidor::process_event(Event *e)
{
  json log;
  printf("%s", "procesando evento de consumidor en [CONSUMIDOR]\n");
  switch (e->get_process())
  {
  case EVENTOS_CONSUMIDOR::INIT_COMPRA_FERIANTE:
  {
    this->process_init_compra();
    break;
  }
  case EVENTOS_CONSUMIDOR::PROCESAR_COMPRA_FERIANTE:
  {
    this->process_resp_feriante(e, log);
    break;
  }
  default:
    break;
  }
  this->monitor->write_log(log);
}

void Consumidor::process_init_compra()
{
  printf("%s", "procesando inicio de compra de feriante en [CONSUMIDOR]\n");
  // Determinamos el producto a comprar y la cantidad
  std::vector<int> prods = this->choose_product();

  for (int prod_id : prods)
  { //
    double amount = this->purchase_amount(prod_id);

    // Elegimos el feriante del cual vamos a comprar
    Feriante *fer = this->choose_feriante(prod_id, amount);
    //* TODO: Cambiar con implementación de cola de mensajes
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> d(2);
    double purchase_time = d(gen);
    std::map<std::string, double> content = {{"amount", amount}, {"prod_id", (double)prod_id}, {"buyer_id", (double)this->get_consumer_id()}};
    this->fel->insert_event(
        purchase_time, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::VENTA_CONSUMIDOR, fer->get_id(), Message(content), fer);
  }
}

void Consumidor::process_resp_feriante(const Event *e, json &log)
{
  log["event_type"] = "PROCESAR_COMPRA_FERIANTE";
  printf("%s", "procesando respuesta de feriante en [CONSUMIDOR]\n");
  Message msg = e->get_message();
  if (msg.msg.count("error")) // Si retornamos con error
  {
    msg.msg.erase("error");
    Feriante *new_fer;
    double prod_id = msg.msg.at("prod_id");
    double amount = msg.msg.at("amount");

    // Lanzamos una excepción si no quedan feriantes de los cuales comprar, ver "Zero-Cost Exceptions" en gcc y MSVC
    // try{
    new_fer = this->choose_feriante((int)prod_id, amount);
    //}
    this->fel->insert_event(
        0.0, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::VENTA_CONSUMIDOR, new_fer->get_id(), msg, new_fer);
    return;
  }
  this->finish_purchase();
}

void Consumidor::set_feria(int _feria_id) { this->id_feria = _feria_id; }

int Consumidor::get_feria() const { return this->id_feria; }

int Consumidor::get_consumer_id() const { return this->consumer_id; }
