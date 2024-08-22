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

extern "C"
{
#include <bsp.h>
}

int _argc;
char **_argv;

std::string CONFIG_PATH_FILE;
double END_SIM_TIME = 13'800;

void spmd_part();
int main(int argc, char *argv[])
{

  // For parameter parsing
  int opt;
  SimConfig *sim_config;

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
      // return EXIT_FAILURE;
      break;
    }
  }

  if (argc < 2)
  {
    fprintf(stderr, "Correct Usage: -c [path to config json file]\n");
    CONFIG_PATH_FILE = "./sim_config.json";
    printf("CONFIG_PATH_FILE: %s\n", CONFIG_PATH_FILE.c_str());
    // return EXIT_FAILURE;
  }
  sim_config = SimConfig::get_instance(CONFIG_PATH_FILE);
  if (sim_config == nullptr)
  {
    fprintf(stderr, "Error inicializando SimConfig object.\n");
  }

  // Copiamos los parámetros de ejecución
  _argc = argc;
  _argv = argv;

  // BSP Init
  bsp_init(spmd_part, _argc, _argv);
  spmd_part();

  exit(EXIT_SUCCESS);
}

void spmd_part()
{
  printf("Aqki vamo\n");
  auto sim = ParallelSimulation(END_SIM_TIME, CONFIG_PATH_FILE);
  sim.run();
}
