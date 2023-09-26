#include "../includes/consumidor.h"

int Consumidor::current_consumer_id(-1);
std::string Consumidor::consumer_mode("single");
bool Consumidor::mode_set(false);

Consumidor::Consumidor(FEL *_fel, int _feria, int _cant_integrantes)
    : consumer_id(++current_consumer_id), fel(_fel), id_feria(_feria)
{

  if (Consumidor::consumer_mode == "single")
    this->cant_integrantes = 1;
  else
    this->cant_integrantes = _cant_integrantes;
}

void Consumidor::process_event(Event *e)
{
  printf("%s", "procesando evento de consumidor \n");
}

void Consumidor::initialize_purchase() {}

void Consumidor::set_feria(int _feria_id) { this->id_feria = _feria_id; }

int Consumidor::get_feria() { return this->id_feria; }

int Consumidor::get_consumer_id() { return this->consumer_id; }

void Consumidor::set_consumer_mode(std::string &mode)
{
  if (Consumidor::mode_set)
    return;
  Consumidor::consumer_mode = mode;
  Consumidor::mode_set = true;
}