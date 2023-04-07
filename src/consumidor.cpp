#include "../includes/consumidor.h"

int Consumidor::current_consumer_id(-1);

Consumidor::Consumidor(FEL *_fel, int _feria, int _cant_integrantes)
    : consumer_id(++current_consumer_id) {
  this->fel = _fel;
  this->id_feria = _feria;
  this->cant_integrantes = _cant_integrantes;
}

void Consumidor::process_event(Event *e) {
  printf("%s", "procesando evento de consumidor \n");
}

void Consumidor::initialize_purchase() {}

void Consumidor::set_feria(int _feria_id) { this->id_feria = _feria_id; }

int Consumidor::get_feria() { return this->id_feria; }

int Consumidor::get_consumer_id() { return this->consumer_id; }
