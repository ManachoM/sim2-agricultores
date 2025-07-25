#include "../includes/event.h"

int Event::current_event_id(-1);

Event::Event() : event_id(++current_event_id)
{
    this->time = 0.0;
    this->agent_type = -1;
    this->process = -1;
    this->caller_id = -1;
    this->msg = Message();
}

Event::Event(double _time, int _agent_type, int _process, int _caller_id, Message _msg, Agent *_caller_ptr) : event_id(++current_event_id)
{
    this->time = _time;
    this->agent_type = _agent_type;
    this->process = _process;
    this->caller_id = _caller_id;
    this->msg = _msg;
    this->caller_ptr = _caller_ptr;
}

double Event::get_time() const
{
    return this->time;
}

int Event::get_type() const
{
    return this->agent_type;
}

int Event::get_process() const
{
    return this->process;
}

int Event::get_caller_id() const
{
    return this->caller_id;
}

Event * Event::get_next_event() const
{
    return this->next;
}

Message Event::get_message() const
{
    return this->msg;
}

Agent * Event::get_caller_ptr() const
{
    return this->caller_ptr;
}

void Event::set_time(double _time)
{
    this->time = _time;
}

void Event::set_message(Message _msg)
{
    this->msg = _msg;
}

void Event::set_next_event(Event *_next)
{
    this->next = _next;
}

Event::~Event() = default;