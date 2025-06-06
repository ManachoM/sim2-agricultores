#include "../includes/feriante.h"

#include "../includes/agricultor.h"
#include "../includes/environment.h"
#include "../includes/feria.h"
#include "../includes/mercado_mayorista.h"
#include "../includes/monitor.h"

int Feriante::current_feriante_id(-1);

Feriante::Feriante(FEL *_fel, MercadoMayorista *_mer, int _feria_id)
    : Agent(), feriante_id(++current_feriante_id), fel(_fel),
      feria_id(_feria_id), mercado(_mer) {}

void Feriante::process_event(Event *e) {
  auto log = json();
  log["agent_type"] = "FERIANTE";
  log["agent_id"] = this->get_id();
  log["time"] = e->get_time();
  switch (e->get_process()) {
  case EVENTOS_FERIANTE::INIT_COMPRA_MAYORISTA: {
    // printf("Procesando evento feriante INIT_COMPRA\n");
    log["agent_process"] = "INIT_COMPRA";
    this->process_init_compra();
    break;
  }
  case EVENTOS_FERIANTE::PROCESS_COMPRA_MAYORISTA: {
    // printf("Procesando evento feriante PROCESAR_COMPRA_AGRICULTOR\n");
    log["agent_process"] = "PROCESAR_COMPRA_AGRICULTOR";
    this->process_resp_agricultor(e, log);
    break;
  }
  case EVENTOS_FERIANTE::VENTA_CONSUMIDOR: {
    // printf("Procesando evento feriante RESPUESTA_CONSUMIDOR\n");
    log["agent_process"] = "RESPUESTA_CONSUMIDOR";
    this->process_venta_feriante(e);
    break;
  }
  case EVENTOS_FERIANTE::COMPRA_MAYORISTA: {
    log["agent_process"] = "COMPRA_MAYORISTA";
    this->process_compra_mayorista(e, log);
  }
  default:
    break;
  }
  this->monitor->write_log(log);
}

void Feriante::process_init_compra() {
  std::vector<int> prod_ids = this->choose_product();
  for (int prod_id : prod_ids) {
    double amount = this->purchase_amount(prod_id);
    // Esto va dentro de un try catch
    Agricultor *agr = this->choose_agricultor(prod_id, amount);

    if (!agr) {
      continue;
    }
    std::map<std::string, double> content = {
        {"amount", amount},
        {"prod_id", (double)prod_id},
        {"buyer_id", (double)this->get_feriante_id()}
    };

    this->fel->insert_event(
        1.0, AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::VENTA_FERIANTE,
        agr->get_agricultor_id(), Message(content), agr
    );
    // printf("insertado evento oke\n");
  }
}

void Feriante::process_resp_agricultor(const Event *e, json &log) {
  Message msg = e->get_message();
  double prod_id = msg.msg.at("prod_id");
  double amount = msg.msg.at("amount");
  log["target_product"] = prod_id;
  log["target_amount"] = amount;

  if (msg.msg.count("error")) {
    log["id_agricultor"] = -1;
    msg.msg.erase("error");
    Agricultor *new_agr;

    new_agr = this->choose_agricultor((int)prod_id, amount);

    if (!new_agr) {
      this->finish_purchase();
      return;
    }
    this->fel->insert_event(
        0.0, AGENT_TYPE::AGRICULTOR, EVENTOS_AGRICULTOR::VENTA_FERIANTE,
        new_agr->get_agricultor_id(), msg, new_agr
    );

    return;
  }
  log["id_agricultor"] = msg.msg.at("seller_id");
  // Modificamos el inventario acordemente

  auto inv = this->inventario.find((int)prod_id);
  if (inv == this->inventario.end()) {
    this->inventario.insert(
        {prod_id, Inventario(e->get_time(), 0.0, prod_id, amount)}
    ); // FIXME: Vencimiento de inventario de feriantes
  } else {
    double current_quantity = inv->second.get_quantity();
    inv->second.set_quantity(current_quantity + amount);
  }

  this->env->get_feria(this->feria_id)
      ->update_index(this->get_id(), prod_id, true);
  this->finish_purchase();
}

void Feriante::process_venta_feriante(const Event *e) {
  Message msg = e->get_message();
  // printf("akii 1\n");
  double prod_id = msg.msg.at("prod_id");
  double amount = msg.msg.at("amount");
  double buyer_id = msg.msg.at("buyer_id");
  Inventario inv = this->get_inventario_by_id((int)prod_id);
  double quantity = inv.get_quantity();
  if (!inv.is_valid_inventory() || quantity < amount) {
    msg.msg.insert({"error", -1});
    this->fel->insert_event(
        0.0, AGENT_TYPE::CONSUMIDOR,
        EVENTOS_CONSUMIDOR::PROCESAR_COMPRA_FERIANTE, (int)buyer_id, msg,
        nullptr
    );
    return;
  }
  inv.set_quantity(quantity - amount);
  this->inventario.at((int)prod_id) = inv;
  if (inv.get_quantity() < amount) {
    this->env->get_feria(this->feria_id)
        ->update_index(this->get_id(), prod_id, false);
  }
  msg.msg.insert({"seller_id", this->get_feriante_id()});
  this->fel->insert_event(
      0.0, AGENT_TYPE::CONSUMIDOR, EVENTOS_CONSUMIDOR::PROCESAR_COMPRA_FERIANTE,
      (int)buyer_id, msg, nullptr
  );
  return;
}

void Feriante::process_compra_mayorista(const Event *e, json &log) {
  // Para loggearse la bida
  std::vector<json> compras;
  // IDS de productos a consumir
  std::vector<int> prods_ids = this->choose_product();
  for (const int &prod_id : prods_ids) {
    // Cantidad del producto a comprar
    double amount = this->purchase_amount(prod_id);
    // Lista de ids de agricultores potenciales vendedores
    std::vector<int> agros_ids =
        this->mercado->get_agricultor_por_prod(prod_id);

    for (const int &agro_id : agros_ids) {
      // Nos traemos al agricultor en cuestión y su inventario
      Agricultor *agro = this->env->get_agricultor(agro_id);

      Inventario inv = agro->get_inventory_by_id(prod_id);
      double q = inv.get_quantity();
      // Si no tiene inventario o no es válido
      if (!inv.is_valid_inventory() || q < amount) {

        this->mercado->update_index(agro->get_agricultor_id(), prod_id, false);
        continue;
      }

      this->inventario.insert(
          {prod_id, Inventario(e->get_time(), 0.0, prod_id, amount)}
      ); // FIXME: Vencimiento de inventario de feriantes

      // Actualizamos el inventario
      inv.set_quantity(q - amount);
      agro->set_inventory_by_id(prod_id, inv);
      // Si es apropiado, actualizamos el índice
      if (inv.get_quantity() < amount) {
        this->mercado->update_index(agro->get_agricultor_id(), prod_id, false);
      }

      // too lo del log
      json compra;
      compra["id_agricultor"] = agro->get_agricultor_id();
      compra["target_amount"] = amount;
      compra["target_product"] = prod_id;
      compras.push_back(compra);
      // Si llegamos a este punto, significa que ya compramos
      // lo que queríamos de este producto y podemos seguir
      break;
    }
  }
  log["compras"] = compras;
}

int Feriante::get_feriante_id() { return this->feriante_id; }

std::map<int, Inventario> Feriante::get_inventario() const {
  return this->inventario;
}

Inventario Feriante::get_inventario_by_id(int _id_producto) {
  auto aux = this->inventario.find(_id_producto);
  if (aux == this->inventario.end()) {
    return Inventario(0.0, 0.0, _id_producto, 0.0);
  }
  return aux->second;
}

void Feriante::set_inventario_by_id(int prod_id, const Inventario &inv) {
  this->inventario.at(prod_id) = inv;
}

void Feriante::set_feria(const int feria_id) { this->feria_id = feria_id; }

Feriante::~Feriante() = default;
