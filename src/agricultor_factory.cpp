#include "../includes/agricultor_factory.h"

#include "../includes/agricultor_ganancia.h"
#include "../includes/agricultor_riesgo.h"
#include "../includes/agricultor_simple.h"

AgricultorFactory::AgricultorFactory(
    FEL *_fel, Environment *_env, Monitor *_monitor, MercadoMayorista *_mer
)
    : fel(_fel), env(_env), monitor(_monitor), mer(_mer) {}

Agricultor *AgricultorFactory::create_agricultor(
    std::string const &agr_type, Terreno *terr
) {
  Agricultor *agr;
  if (agr_type == "simple") {
    agr = new AgricultorSimple(this->fel, terr, this->mer);
  } else if (agr_type == "ganancia") {
    agr = new AgricultorGanancia(this->fel, terr, this->mer);
  } else if (agr_type == "risk") {
    agr = new AgricultorRiesgo(this->fel, terr, this->mer);
  } else
    throw std::logic_error("Tipo de agricultor no implementado...");

  agr->set_environment(this->env);
  agr->set_monitor(this->monitor);

  return agr;
}
