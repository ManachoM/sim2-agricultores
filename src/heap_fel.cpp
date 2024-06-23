#include "../includes/heap_fel.h"

HeapFEL::HeapFEL() {}

void HeapFEL::insert_event(
    double _time, int _agent_type, int _proc, int _caller_id, Message _msg,
    Agent *_caller_ptr
) {
  Event *e = new Event(
      _time + this->clock, _agent_type, _proc, _caller_id, _msg, _caller_ptr
  );
  this->pq.push(e);
}

Event *HeapFEL::next_event() {
  Event *e = this->pq.top();

  // TODO: Add exceptions in case of time discrepancies
  //

  this->clock = e->get_time();
  pq.pop();
  return e;
}

double HeapFEL::get_time() { return this->clock; }

bool HeapFEL::is_empty() { return this->pq.empty(); }

HeapFEL::~HeapFEL(){};
