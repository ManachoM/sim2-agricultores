#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "event_handler.h"
#include "feria.h"
#include "glob.h"
#include "heap_fel.h"
#include "postgres_aggregated_monitor.h"

class SimConfig;
class MercadoMayorista;
class Producto;
class Terreno;
class FEL;
class Environment;

class Simulation {
public:
  Simulation();
  Simulation(const double _max_sim_time, const std::string &config_path);
  void run();
  ~Simulation();

private:
  const double max_sim_time;
  const std::string conf_path;
  FEL *fel = new HeapFEL();
  Environment *env;
  Monitor *monitor = new PostgresAggregatedMonitor();
  EventHandlerSystem event_handlers;

  std::vector<Consumidor *> consumidores_arr;
  std::vector<Feriante *> feriante_arr;
  std::vector<Agricultor *> agricultor_arr;
  std::vector<Feria *> feria_arr;
  std::unordered_map<int, Agricultor *> agricultores;
  std::unordered_map<int, Agricultor *> agricultor_por_id_relativo;
  std::unordered_map<int, Feria *> ferias;
  std::unordered_map<int, Consumidor *> consumidores;
  std::unordered_map<int, Feriante *> feriantes;
  std::unordered_map<int, Producto *> productos;
  std::unordered_map<int, Terreno *> terrenos;

  MercadoMayorista *mercado;

  void read_products();
  void read_ferias();
  void read_terrenos();
  void initialize_agents(MercadoMayorista *mercado);
  void initialize_event_handlers();
};

#endif // !_SIMULATION_H_
