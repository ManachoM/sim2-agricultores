#ifndef FEL_H
#define FEL_H

#include "event.h"
#include "glob.h"
#include "message.h"
#include "object_pool.h"

#include <cstddef>

class FEL
{
public:
  virtual void insert_event(
      double _time, int _agent_type, int _process, int _caller_id, Message _msg,
      Agent *_caller_ptr = nullptr
  ) = 0;

  virtual Event *next_event() = 0;

  virtual double get_time() = 0;

  virtual bool is_empty() = 0;

  virtual std::size_t get_size() = 0;

  int num_venta_feriante = 0;

  int num_process_resp_agr = 0;

  ObjectPool<Event> event_pool;
};

#endif // !FEL_H
