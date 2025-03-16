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
double WINDOW_SIZE = 6.0;

void spmd_part();
int main(int argc, char *argv[])
{
  // For parameter parsing
  int opt;
  SimConfig *sim_config;

  while ((opt = getopt(argc, argv, "c:w:")) != -1)
  {
    switch (opt)
    {
    case 'c':
      CONFIG_PATH_FILE = optarg;
      break;
    case 'w':
      WINDOW_SIZE = std::stod(optarg);
      printf("Window size set to: %lf\n", WINDOW_SIZE);
      break;
    default:
      printf("Default case optarg: %s\n", optarg);
      fprintf(stderr, "Correct Usage: -c [path to config json file] -w [window size]\n");
      // return EXIT_FAILURE;
      break;
    }
  }

  if (CONFIG_PATH_FILE.empty())
  {
    fprintf(stderr, "Correct Usage: -c [path to config json file] -w [window size]\n");
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

  // Inicializamos el ambiente BSP
  bsp_begin(bsp_nprocs());

  auto sim = ParallelSimulation(END_SIM_TIME, CONFIG_PATH_FILE);
  sim.run();

  bsp_end();
}
