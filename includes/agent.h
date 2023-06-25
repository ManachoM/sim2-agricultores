#ifndef AGENT_H
#define AGENT_H

#include "event.h"
#include "message.h"

// Implicit class declaration, to avoid compilation errors
class Environment;
class Monitor;


class Agent
{
public:

    Agent();

    virtual void process_event(Event * e) = 0;

    int get_id();

    void set_monitor(Monitor *_monitor);

    void set_environment(Environment * _env);

    virtual ~Agent();

protected:
    int id; /** Identificador de la instancia entre todos los agentes */
    Environment * env;
    Monitor * monitor;
private:
    static int current_id;
};

#endif // !AGENT_H

