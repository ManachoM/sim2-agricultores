#include "../includes/monitor.h"

Monitor::Monitor(std::string const &_file_prefix, bool _debug)
    : file_prefix(_file_prefix), debug_flag(_debug)
{
  // Generamos el sim_id
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
  this->sim_id = oss.str();
};

void Monitor::write_duration(double t) {}

void Monitor::write_results() {}

void Monitor::write_params(const std::string &key, const std::string &value) {}

void Monitor::add_event_record(SSEventRecord e) {};

void Monitor::add_time_record(SSTimeRecord e) {};

Monitor::~Monitor() = default;
