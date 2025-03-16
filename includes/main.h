#ifndef MAIN_H
#define MAIN_H

#include "agricultor.h"
#include "consumidor.h"
#include "environment.h"
#include "event.h"
#include "glob.h"
#include "heap_fel.h"
#include "message.h"
#include "parallel_simulation.h"
#include "postgres_aggregated_monitor.h"
#include "sim_config.h"

// Global variables
extern std::string CONFIG_PATH_FILE;
extern double END_SIM_TIME;
extern double WINDOW_SIZE;

#endif // !MAIN_H
