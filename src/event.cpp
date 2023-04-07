#include "../includes/event.h"

Event::Event()
{
    this->time = 0.0;
    this->agent_type = -1;
    this->process = -1;
    this->caller_id = -1;
    this->msg = Message();
}

Event::Event(double _time, int _agent_type, int _process, int _caller_id, Message _msg)
{
    this->time = _time;
    this->agent_type = _agent_type;
    this->caller_id = _caller_id;
    this->msg = _msg;
}

double Event::get_time()
{
    return this->time;
}

int Event::get_type()
{
    return this->agent_type;
}

int Event::get_process()
{
    return this->process;
}

int Event::get_caller_id()
{
    return this->caller_id;
}

Event * Event::get_next_event()
{
    return this->next;
}

Message Event::get_message()
{
    return this->msg;
}

Agent * Event::get_caller_ptr()
{
    return this->caller_ptr;
}

