#ifndef EVENT_COMPARATOR_H
#define EVENT_COMPARATOR_H

#include "event.h"

class EventComparator {

public:
  inline int operator()(Event const *e1, Event const *e2) const{
    return e1->get_time() > e2->get_time();
  };
};

#endif // !EVENT_COMPARATOR_H 
