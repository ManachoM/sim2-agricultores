#include "../includes/agent.h"

int Agent::current_id(-1);

Agent::Agent() : id(++current_id)
{
}

void Agent::set_monitor(Monitor *_monitor)
{
    this->monitor = _monitor;
}

void Agent::set_environment(Environment *_env)
{
    this->env = _env;
}

Agent::~Agent()
{
}
