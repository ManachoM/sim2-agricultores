#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "glob.h"

class SimConfig;

class Simulation
{
public:
  Simulation();
  Simulation(const double _max_sim_time, const std::string &config_path);
  void run(const SimConfig &config);

private: 
  const double max_sim_time;
  const std::string conf_path;

};

#endif // !_SIMULATION_H_
#define _SIMULATION_H_
