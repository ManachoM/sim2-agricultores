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

int main(int argc, char *argv[])
{
  // For parameter parsing
  int opt;
  SimConfig *sim_config;
  double END_SIM_TIME = 10'000;

  std::string CONFIG_PATH_FILE;

  while ((opt = getopt(argc, argv, "c:")) != -1)
  {
    switch (opt)
    {
    case 'c':
      CONFIG_PATH_FILE = optarg;
      sim_config = SimConfig::get_instance(CONFIG_PATH_FILE);
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

  auto *fel = new HeapFEL();

  auto *env = new Environment(fel);

  Event *current_event;

  while (fel->get_time() <= END_SIM_TIME && !fel->is_empty())
  {
    current_event = fel->next_event();
    assert(current_event->get_time() >= fel->get_time());


    if(current_event->get_caller_ptr() != nullptr)
    {
      Agent *caller = current_event->get_caller_ptr();
      caller->process_event(current_event);
      delete current_event;
      continue;
    } 

    assert(current_event != nullptr);
    switch ((current_event->get_type()))
    {
    case AGENT_TYPE::AMBIENTE:
      env->process_event(current_event);
      break;
    
    default:
      break;
    }

    delete current_event;
  }

  // Memory deletion
  delete env;
  delete fel;
  printf("[FIN SIMLUACION]\n");
  exit(EXIT_SUCCESS);
}
