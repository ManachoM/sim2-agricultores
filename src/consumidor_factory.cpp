#include "../includes/consumidor_factory.h"
#include "../includes/consumidor.h"
#include "../includes/consumidor_familiar.h"
#include "../includes/consumidor_familiar_presupuesto.h"
#include "../includes/consumidor_simple.h"

ConsumidorFactory::ConsumidorFactory(FEL *_fel, Environment *_env,
                                     Monitor *_monitor)
    : fel(_fel), env(_env), monitor(_monitor) {}

Consumidor *ConsumidorFactory::create_consumidor(std::string const &cons_type,
                                                 const int feria_id) {
  Consumidor *cons;
  if (cons_type == "single") {
    cons = new ConsumidorSimple(feria_id, this->fel);
  } else if (cons_type == "family") {
    int cant_integrantes;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(0.0, 1.0);
    double val = d(gen);
    if (val <= 0.43 && val > 0.16)
      cant_integrantes = 2;
    else if (val <= 0.67 && val > 0.43)
      cant_integrantes = 3;
    else if (val <= 0.86 && val > 0.67)
      cant_integrantes = 4;
    else
      cant_integrantes = 5;

    cons = new ConsumidorFamiliar(cant_integrantes, feria_id, this->fel);
  } else if (cons_type == "family_budget") {
    int cant_integrantes;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d_size(0.0, 1.0);
    double val = d_size(gen);
    if (val <= 0.43 && val > 0.16)
      cant_integrantes = 2;
    else if (val <= 0.67 && val > 0.43)
      cant_integrantes = 3;
    else if (val <= 0.86 && val > 0.67)
      cant_integrantes = 4;
    else
      cant_integrantes = 5;

    std::uniform_int_distribution<> d_budget(25000, 45000);
    double budget = d_budget(gen);

    cons = new ConsumidorFamiliarPresupuesto(cant_integrantes, feria_id, budget,
                                             this->fel);
  } else
    throw std::logic_error("Tipo de consumidor no implementado...");

  cons->set_environment(this->env);
  cons->set_monitor(this->monitor);

  return cons;
}
