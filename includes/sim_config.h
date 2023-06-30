#ifndef SIM_CONFIG_H
#define SIM_CONFIG_H

#include "glob.h"

class SimConfig
{
private:
    static SimConfig *sim_config_instance;
    static std::mutex mutex;
    std::string config_path;
    json config_parameters;

protected:
    explicit SimConfig(const std::string &config_file_path = "");

    ~SimConfig();

public:
    static SimConfig *get_instance(const std::string &_conf_file);

    std::string get_config_file_path() const;

    json get_config();

    SimConfig(SimConfig &other) = delete;

    void operator=(const SimConfig &) = delete;
};

#endif // !SIM_CONFIG_H