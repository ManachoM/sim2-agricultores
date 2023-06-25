#include "../includes/environment.h"

Environment::Environment(FEL *_fel): fel(_fel) {  }

void Environment::set_feriantes(std::map<int, Feriante *> _feriantes)
{
  this->feriantes = _feriantes;
}

void Environment::set_consumidores(std::map<int, Consumidor *> _cons)
{
  this->consumidores = _cons;
}

void Environment::set_ferias(std::map<int, Feria *> _ferias)
{
  this->ferias = _ferias;
}

void Environment::process_event(Event *e)
{
  printf("Procesando evento del ambiente...");
}

short Environment::get_day_week()
{
  return (short)(this->fel->get_time() / 24) % 24;
}

short Environment::get_day_month()
{
  return (short)(this->fel->get_time() / 24.0) % 30;
}

short Environment::get_month()
{
  return (short)(this->fel->get_time() / 720) % 12;
}

short Environment::get_year()
{
  return ((short)this->fel->get_time() / 8640);
}

std::map<int, Feria *> Environment::get_ferias() { return this->ferias; }

std::map<int, Feriante *> Environment::get_feriantes() { return this->feriantes; }

std::map<int, Agricultor *> Environment::get_agricultores() { return this->agricultores; }

std::map<int, Producto *> Environment::get_productos() { return this->productos; }

std::map<int, std::vector<Producto *>> Environment::get_venta_producto_mes() { return this->venta_producto_mes; }

int Environment::get_nivel_heladas() { return -1; }

int Environment::get_nivel_sequias() { return this->sequias_nivel.at(this->get_month()); }

int Environment::get_nivel_olas_calor() { return this->oc_nivel.at(this->get_month()); }

void Environment::initialize_system()
{
  printf("Inicializando sistema...\n");
  this->message_queue = new MessageQueue(
    (int) this->agricultores.size() + this->consumidores.size() + this->consumidores.size()
  );
}

Environment::~Environment() = default;