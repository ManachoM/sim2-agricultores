/**
 * @file main.cpp
 * @author @ManachoM
 * @brief Entry point for the simulation app
 * @version 0.1
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "../includes/main.h"

void test_json(json &obj)
{
  obj["test"] = 1;
};

int main(int argc, char *argv[])
{

  // json obj;
  // obj["hola"] = "mundo";
  // std::cout << obj.dump() << '\n';
  // test_json(obj);

  // std::cout << obj.dump() << '\n';
  // exit(0);
  // For parameter parsing
  int opt;
  SimConfig *sim_config;
  double END_SIM_TIME = 15'000;

  std::string CONFIG_PATH_FILE;

  while ((opt = getopt(argc, argv, "c:")) != -1)
  {
    switch (opt)
    {
    case 'c':
      CONFIG_PATH_FILE = optarg;
      break;

    default:
      printf("Default case optarg: %s\n", optarg);
      fprintf(stderr, "Correct Usage: -c [path to config json file]\n");
      return EXIT_FAILURE;
      break;
    }
  }

  if (argc < 2)
  {
    fprintf(stderr, "Correct Usage: -c [path to config json file]\n");
    return EXIT_FAILURE;
  }
  sim_config = SimConfig::get_instance(CONFIG_PATH_FILE);

  auto *fel = new HeapFEL();

  auto *mon = new PostgresAggregatedMonitor();

  auto *env = new Environment(fel, mon);

  Event *current_event;
  std::map<std::string, int> agent_type_count = {{"CONSUMIDOR", 0}, {"FERIANTE", 0}, {"AGRICULTOR", 0}, {"AMBIENTE", 0}};
  std::string agent_type;
  auto start_time = std::chrono::high_resolution_clock::now();
  while (fel->get_time() <= END_SIM_TIME && !fel->is_empty())
  {
    current_event = fel->next_event();
    agent_type = agent_type_to_agent.at(current_event->get_type());
    agent_type_count[agent_type]++;
    assert(current_event->get_time() >= fel->get_time());

    if ((current_event->event_id % 10000) == 0)
      std::cout << "SIM TIME: " << current_event->get_time() << "\n EVENT ID: " << current_event->event_id << "\n";

    if (current_event->get_caller_ptr() != nullptr)
    {
      Agent *caller = current_event->get_caller_ptr();
      // printf("Caller id: %d\n", caller->get_id());
      caller->process_event(current_event);
      delete current_event;
      continue;
    }

    assert(current_event != nullptr);
    switch (current_event->get_type())
    {
    case AGENT_TYPE::AMBIENTE:
    {
      env->process_event(current_event);
      break;
    }
    case AGENT_TYPE::AGRICULTOR:
    {
      Agent *agro = env->get_agricultor(current_event->get_caller_id());
      agro->process_event(current_event);
      break;
    }

    case AGENT_TYPE::FERIANTE:
    {
      Agent *fer = env->get_feriante(current_event->get_caller_id());
      fer->process_event(current_event);
      break;
    }

    case AGENT_TYPE::CONSUMIDOR:
    {

      Agent *cons = env->get_consumidor(current_event->get_caller_id());
      cons->process_event(current_event);
      break;
    }
    default:
      break;
    }

    delete current_event;
  }
  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  mon->write_duration((double)duration.count());
  mon->write_results();
  // Memory deletion
  std::cout << "SIM DURATION: " << duration.count() << "[ms]\n";
  delete env;
  delete mon;
  for (auto const &[tipo, cantidad] : agent_type_count)
  {
    std::cout << "AGENTE " << tipo << " \t CANTIDAD DE EVENTOS PROCESADOS: " << cantidad << "\n";
  }
  printf("ÚLTIMO EVENTO EN COLA %d\n", fel->next_event()->event_id);
  printf("[FIN SIMLUACION]\n");
  exit(EXIT_SUCCESS);
}
