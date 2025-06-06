#include "../includes/consumidor.h"

#include "../includes/environment.h"
#include "../includes/feria.h"
#include "../includes/monitor.h"

int Consumidor::current_consumer_id(-1);

Consumidor::Consumidor(FEL *_fel, int _feria)
    : Agent(), fel(_fel), id_feria(_feria) {}

void Consumidor::process_event(Event *e) {
  json log;
  log["agent_type"] = "CONSUMIDOR";
  log["time"] = e->get_time();
  log["agent_id"] = this->get_id();

  // printf("%s", "procesando evento de consumidor en [CONSUMIDOR]\n");
  switch (e->get_process()) {
  case EVENTOS_CONSUMIDOR::INIT_COMPRA_FERIANTE: {
    log["agent_process"] = "INIT_COMPRA";
    this->process_init_compra();
    break;
  }
  case EVENTOS_CONSUMIDOR::PROCESAR_COMPRA_FERIANTE: {
    this->process_resp_feriante(e, log);
    break;
  }
  case EVENTOS_CONSUMIDOR::COMPRA_FERIANTE: {
    log["agent_process"] = "COMPRA_FERIANTE";
    this->process_compra_feriante(e, log);
    break;
  }
  default:
    break;
  }
  this->monitor->write_log(log);
}

void Consumidor::process_init_compra() {
  // printf("%s", "procesando inicio de compra de feriante en [CONSUMIDOR]\n");
  // Determinamos el producto a comprar y la cantidad
  std::vector<int> prods = this->choose_product();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::exponential_distribution<> d(2);
  for (int prod_id : prods) { //
    double amount = this->purchase_amount(prod_id);

    // Elegimos el feriante del cual vamos a comprar
    Feriante *fer = this->choose_feriante(prod_id, amount);
    if (!fer)
      continue;
    //* TODO: Cambiar con implementación de cola de mensajes

    double purchase_time = d(gen);
    std::map<std::string, double> content = {
        {"amount", amount},
        {"prod_id", (double)prod_id},
        {"buyer_id", (double)this->get_consumer_id()}
    };
    this->fel->insert_event(
        purchase_time, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::VENTA_CONSUMIDOR,
        fer->get_feriante_id(), Message(content), fer
    );
  }
}

void Consumidor::process_resp_feriante(const Event *e, json &log) {
  log["agent_process"] = "PROCESAR_COMPRA_FERIANTE";
  // printf("%s", "procesando respuesta de feriante en [CONSUMIDOR]\n");
  Message msg = e->get_message();

  double prod_id = msg.msg.at("prod_id");
  double amount = msg.msg.at("amount");
  log["target_product"] = prod_id;
  log["target_amount"] = amount;
  if (msg.msg.count("error")) // Si retornamos con error
  {
    msg.msg.erase("error");
    Feriante *new_fer;

    // Lanzamos una excepción si no quedan feriantes de los cuales comprar, ver
    // "Zero-Cost Exceptions" en gcc y MSVC try{
    new_fer = this->choose_feriante((int)prod_id, amount);
    //}
    if (!new_fer) {
      this->finish_purchase();
      log["feriante_id"] = -1;
      return;
    }
    this->fel->insert_event(
        0.0, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::VENTA_CONSUMIDOR,
        new_fer->get_feriante_id(), msg, new_fer
    );

    log["feriante_id"] = -1;
    return;
  }
  log["feriante_id"] = msg.msg.at("seller_id");
  this->finish_purchase();
}

void Consumidor::process_compra_feriante(const Event *e, json &log) {
  // Productos a comprar
  std::vector<int> prods_ids = this->choose_product();
  // Feria para referencia
  Feria *feria = this->env->get_feria(this->id_feria);
  // Para logs
  std::vector<json> compras;
  for (const int prod_id : prods_ids) {
    // Obtenemos la cantidad a comprar
    double amount = this->purchase_amount(prod_id);

    // Iteramos sobre todos los feriantes hasta comprar o quedarnos
    // sin feriantes.
    auto fers = feria->get_feriantes();
    for (const auto [id, fer] : fers) {
      // Obtenemos el inventario del feriante, y verificamos
      Inventario inv = fer->get_inventario_by_id(prod_id);
      double q = inv.get_quantity();
      if (!inv.is_valid_inventory() || q < amount)
        continue;

      // Actualizamos el inventario
      inv.set_quantity(q - amount);
      fer->set_inventario_by_id(prod_id, inv);

      // Si es apropiado, actualizamos el índice
      if (inv.get_quantity() < amount) {
        feria->update_index(fer->get_id(), prod_id, false);
      }

      // Actualizamos el log
      json compra;
      compra["feriante_id"] = fer->get_feriante_id();
      compra["target_product"] = prod_id;
      compra["target_amount"] = amount;
      compras.push_back(compra);
      // Si logramos comprar, significa que podemos pasar al
      // siguiente producto
      break;
    }
  }

  log["compras"] = compras;
}

void Consumidor::set_feria(int _feria_id) { this->id_feria = _feria_id; }

int Consumidor::get_feria() const { return this->id_feria; }

int Consumidor::get_consumer_id() const { return this->consumer_id; }
