#ifndef HEAP_FEL_H
#define HEAP_FEL_H

#include "event.h"
#include "event_comparator.h"
#include "fel.h"
#include "glob.h"

#include <cstddef>

class HeapFEL : public FEL
{
private:
  std::priority_queue<Event *, std::vector<Event *>, EventComparator> pq;
  double clock = 0.0; /** Current sim time */
public:
  HeapFEL();

  HeapFEL(const HeapFEL &) = default;

  HeapFEL(HeapFEL &&) = default;

  HeapFEL &operator=(const HeapFEL &) = default;

  HeapFEL &operator=(HeapFEL &&) = default;

  void insert_event(
      double _time, int _agent_type, int _proc, int _caller_id, Message _msg,
      Agent *_caller_ptr
  ) override;

  void insert_event(Event *e);

  Event *next_event() override;

  double get_time() override;

  bool is_empty() override;

  std::size_t get_size() override;

  std::unordered_map<int, std::unordered_map<int, int>> agent_event_type_count;

  int num_venta_feriante = 0;

  int num_process_resp_agr = 0;
  ~HeapFEL();
};

#endif // !HEAP_FEL_H
