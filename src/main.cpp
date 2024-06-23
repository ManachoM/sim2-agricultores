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

#include <cstdlib>
#include <string>

int main(int argc, char *argv[]) {

  // For parameter parsing
  int opt;
  SimConfig *sim_config;
  double END_SIM_TIME = 13'800;

  std::string CONFIG_PATH_FILE;

  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
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

  if (argc < 2) {
    fprintf(stderr, "Correct Usage: -c [path to config json file]\n");
    return EXIT_FAILURE;
  }
  sim_config = SimConfig::get_instance(CONFIG_PATH_FILE);
  if (sim_config == nullptr) {
    fprintf(stderr, "Error inicializando SimConfig object.\n");
  }

  auto sim = new Simulation(END_SIM_TIME, CONFIG_PATH_FILE);

  sim->run();

  delete sim;

  exit(EXIT_SUCCESS);
}
