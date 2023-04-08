#include "../includes/sim_config.h"

// Static member initialization
SimConfig *SimConfig::sim_config_instance{nullptr};
std::mutex SimConfig::mutex;

SimConfig::SimConfig(const std::string &_conf_path) : config_path(_conf_path)
{
}

SimConfig::~SimConfig() = default;

SimConfig * SimConfig::get_instance(const std::string &_conf_file)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (sim_config_instance == nullptr)
    {
        sim_config_instance = new SimConfig(_conf_file);
    }

    return sim_config_instance;
}

std::string SimConfig::get_config_file_path() const 
{
    return this->config_path;
}