#ifndef FEL_H
#define FEL_H

#include "glob.h"
#include "message.h"
#include "event.h"

class FEL
{
public: 
    virtual void insert_event(double _time, int _agent_type, int  _process, int _caller_id, Message *_msg, Agent *_caller_ptr = nullptr) = 0;

    virtual Event * next_event() = 0;

    virtual double get_time() = 0;

    virtual bool is_empty() = 0;
};

#endif // !FEL_H

