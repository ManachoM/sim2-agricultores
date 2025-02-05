#ifndef _PARALLEL_SIMULATION_H_
#define _PARALLEL_SIMULATION_H_

#include "event.h"
#include "monitor.h"
#include "simulation.h"

#include <map>

class Message;
class Producto;
class MercadoMayorista;

class ParallelSimulation : public Simulation
{

public:
  ParallelSimulation();
  ParallelSimulation(
      const double _max_sim_time, const std::string &config_path
  );
  void run();
  ~ParallelSimulation();

private:
  static constexpr int QUEUE_SIZE = 100'000;
  int GVT_CALCULATION_FREQ = 20;
  int pid;
  int nprocs;
  int finished_procs = 0;
  double gvt = 0.0;
  void update_gvt();
  std::map<int, double> credit_per_proc;
  // Guarda qué procesador almacena qué producto
  std::unordered_map<int, std::vector<Producto *>> prods_per_proc;
  // Guarda qué procesador le corresponde al producto según posición
  std::vector<int> proc_per_prod;
  // Heaps para mantener los mensajes de salida y los eventos de entrada
  std::unordered_map<int, std::vector<Message>> out_queue;
  std::vector<Message> incoming_queue;
  std::unordered_map<int, int> last_seller_by_prod;
  void message_handler(Message &msg);
  void read_products();
  void read_terrenos();
  void read_ferias();
  MercadoMayorista *mercado;
  void initialize_agents(MercadoMayorista *_mer);
  void route_event(Event *e);
  Monitor *monitor;
};

#endif // !_PARALLEL_SIMULATION_H_
