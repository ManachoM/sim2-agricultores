#include "../includes/simulation.h"
#include "../includes/sim_config.h"
#include "../includes/heap_fel.h"
#include "../includes/postgres_aggregated_monitor.h"
#include "../includes/environment.h"


Simulation::Simulation(const double _max_sim_time, const std::string &_config_path) : max_sim_time(_max_sim_time), conf_path(_config_path)
{}

void Simulation::run(const SimConfig &sim_config)
{
  auto *fel = new HeapFEL();
  auto *monitor = new PostgresAggregatedMonitor();
  auto *env = new Environment(fel, monitor);

  Event *current_event;
  Agent *caller;

  std::map<std::string, int> agent_type_count = {{"CONSUMIDOR", 0}, {"FERIANTE", 0}, {"AGRICULTOR", 0}, {"AMBIENTE", 0}};
  std::string agent_type;
  auto start_time = std::chrono::high_resolution_clock::now();

  while (fel->get_time() <= this->max_sim_time && !fel->is_empty())
  {
    current_event = fel->next_event();
    agent_type = agent_type_to_agent.at(current_event->get_type());
    ++agent_type_count[agent_type];

    if ((current_event->event_id % 1'000'000) == 0)
         std::cout << "SIM TIME: " << current_event->get_time() << "\n EVENT ID: " << current_event->event_id << "\n";
    
    caller = current_event->get_caller_ptr();
    if (caller != nullptr)
    {
      caller->process_event(current_event);
      delete current_event;
      continue;
    }

    switch (current_event->get_type()) 
    {
    case AGENT_TYPE::AMBIENTE:
        {
          env->process_event(current_event);
          break;
        }
    case AGENT_TYPE::AGRICULTOR:
        {
          caller = env->get_agricultor(current_event->get_caller_id());
          break;
        }
      case AGENT_TYPE::FERIANTE:
        {
          caller = env->get_feriante(current_event->get_caller_id());
          break;
        }
      case AGENT_TYPE::CONSUMIDOR:
        {
          caller = env->get_consumidor(current_event->get_caller_id());
          break;
        }
      default:
      break;
    }

    caller->process_event(current_event);
    delete current_event;
  }
  
  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  monitor->write_duration((double)duration.count());
  monitor->write_results();
  // Memory deletion
  std::cout << "SIM DURATION: " << duration.count() << "[ms]\n";
  delete env;
  
  for (auto const &[tipo, cantidad] : agent_type_count)
  {
    std::cout << "AGENTE " << tipo << " \t CANTIDAD DE EVENTOS PROCESADOS: " << cantidad << "\n";
    monitor->write_params(tipo, std::to_string(cantidad));
  }
  delete monitor;

  printf("ÚLTIMO EVENTO EN COLA %d\n", fel->next_event()->event_id);
  printf("[FIN SIMLUACION]\n");
}

