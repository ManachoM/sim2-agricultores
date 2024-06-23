#include "../includes/agricultor.h"

#include "../includes/feriante.h"
#include "../includes/mercado_mayorista.h"
#include "../includes/monitor.h"

int Agricultor::curr_agricultor_id(-1);

Agricultor::Agricultor(FEL *_fel, Terreno *_terr, MercadoMayorista *_mer)
    : Agent(), agricultor_id(++curr_agricultor_id), fel(_fel), terreno(_terr),
      mercado(_mer) {
  this->fel->insert_event(
      0.0, AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::CULTIVO_TERRENO,
      this->get_id(), Message(), this
  );
}

void Agricultor::process_event(Event *e) {
  // printf("Procesando agricultor...");
  json log;
  log["agent_type"] = "AGRICULTOR";
  log["time"] = e->get_time();
  log["agent_id"] = this->get_id();

  switch (e->get_process()) {
  case EVENTOS_AGRICULTOR::CULTIVO_TERRENO: {
    this->process_cultivo_event(e, log);
    break;
  }
  case EVENTOS_AGRICULTOR::COSECHA: {
    this->process_cosecha_event(e, log);
    break;
  }
  case EVENTOS_AGRICULTOR::VENTA_FERIANTE: {
    log["agent_process"] = "VENTA_FERIANTE";
    this->process_venta_feriante_event(e);
    break;
  }
  case EVENTOS_AGRICULTOR::INVENTARIO_VENCIDO: {
    log["agent_process"] = "INVENTARIO_VENCIDO";
    this->process_inventario_vencido_event(e, log);
    break;
  }
  default: {
    printf("Procesando agricultor...");

    break;
  }
  }

  this->monitor->write_log(log);
}

void Agricultor::process_cultivo_event(const Event *e, json &log) {
  log["agent_process"] = "CULTIVO_TERRENO";

  int prod_id = this->choose_product();

  log["producto_elegido"] = prod_id;

  Producto const *prod_elegido = this->env->get_productos().at(prod_id);
  this->terreno->set_producto_plantado(prod_id);

  // Insertamos la cosecha
  this->fel->insert_event(
      prod_elegido->get_dias_cosecha() * 24.0, AGENT_TYPE::AGRICULTOR,
      EVENTOS_AGRICULTOR::COSECHA, this->get_id(), Message(), this
  );
}

void Agricultor::process_cosecha_event(const Event *e, json &log) {
  log["agent_process"] = "COSECHA";

  Producto *prod = this->env->get_productos()[this->terreno->get_producto()];
  double cantidad_cosechada =
      this->terreno->get_area() * prod->get_rendimiento();
  log["producto_cosechado"] = prod->get_id();
  log["cantidad_cosechada"] = cantidad_cosechada;
  // printf("cantidad_cosechada: %lf\t prod_id: %d\n", cantidad_cosechada,
  // prod->get_id());
  this->inventario.insert(
      {prod->get_id(),
       Inventario(
           e->get_time(),
           e->get_time() +
               (14 * 24), // Damos 14 días de vida útil al inventario
           prod->get_id(), cantidad_cosechada
       )}
  );

  this->mercado->update_index(this->get_agricultor_id(), prod->get_id(), true);

  this->fel->insert_event(
      24.0, AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::CULTIVO_TERRENO,
      this->get_id(), Message(), this
  );
}

void Agricultor::process_venta_feriante_event(const Event *e) {
  // printf("RESPONDIENDO A FERIANTE\n");
  Message msg = e->get_message();
  // for (auto const &[key, val] : msg.msg)
  // {
  //     std::cout << "key; " << key << " val: " << val << "\n";
  // }
  int prod_id = (int)msg.msg.at("prod_id");
  double amount = msg.msg.at("amount");
  int buyer_id = (int)msg.msg.at("buyer_id");

  Inventario inv;
  try {
    inv = this->inventario.at(prod_id);
  } catch (const std::out_of_range &e) {
    msg.msg.insert({"error", -1});
    this->fel->insert_event(
        0.0, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA,
        buyer_id, msg, nullptr
    );
    std::cerr << e.what() << " - Prod_Id: " << prod_id
              << " - agro_id: " << this->agricultor_id << '\n';
    this->mercado->update_index(this->agricultor_id, prod_id, false);
    return;
  }

  double quantity = inv.get_quantity();
  if (!inv.is_valid_inventory() || quantity < amount) {
    // printf("aki se vendió y q pasa, cantidad disponible: %f    cantidad
    // pedida: %f   diferencia: %f\n", quantity, amount, quantity - amount);
    msg.msg.insert({"error", -1});
    this->fel->insert_event(
        0.0, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA,
        buyer_id, msg, nullptr
    );
    return;
  }

  inv.set_quantity(quantity - amount);
  this->inventario.at(prod_id) = inv;

  if (inv.get_quantity() < amount) {
    this->mercado->update_index(this->agricultor_id, prod_id, false);
  }

  msg.msg.insert({"seller_id", this->get_agricultor_id()});
  this->fel->insert_event(
      0.0, AGENT_TYPE::FERIANTE, EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA,
      buyer_id, msg, nullptr
  );
  return;
}

void Agricultor::process_inventario_vencido_event(const Event *e, json &log) {
  log["EVENT_TYPE"] = "INVENTARIO_VENCIDO";

  Message msg = e->get_message();
  int prod_id = (int)msg.msg.at("prod_id");
  double amount = this->inventario.at(prod_id).get_quantity();

  this->inventario[prod_id] = Inventario();
  this->mercado->update_index(this->get_agricultor_id(), prod_id, false);

  log["prod_id"] = prod_id;
  log["amount"] = amount;
}

std::map<int, Inventario> Agricultor::get_inventory() const {
  return this->inventario;
}

Inventario Agricultor::get_inventory_by_id(const int prod_id) {
  auto aux = this->inventario.find(prod_id);

  if (aux == this->inventario.end())
    return Inventario(0.0, 0.0, prod_id, 0.0);
  return aux->second;
}

void Agricultor::set_inventory_by_id(const int prod_id, const Inventario &inv) {
  this->inventario.at(prod_id) = inv;
}

int Agricultor::get_agricultor_id() { return this->agricultor_id; }

Terreno *Agricultor::get_terreno() { return this->terreno; }
