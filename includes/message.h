#ifndef MESSAGE_H
#define MESSAGE_H

#include "glob.h"

#include <array>
#include <cstring>

enum MESSAGE_KEYS
{
  AMOUNT = 0,
  PROD_ID = 1,
  BUYER_ID = 2,
  SELLER_ID = 3,
  AGENT_ID = 4,
  AGENT_TYPE = 5,
  PROCESS = 6,
  ORIGIN_PID = 7,
  DEST_PID = 8,
  ERROR = 9,
  _TOTAL = 10,
};

class Message
{

private:
  double amount = -9999999.9999;
  int prod_id = -1;
  int buyer_id = 0;
  int seller_id = 0;
  int agent_id = 0;
  short agent_type = -1;
  short process = -1;
  short origin = -1;
  short dest = -1;
  bool error = false;

public:
  Message();
  void insert(const MESSAGE_KEYS &key, const double &val);
  const double *data() const;
  double *data();
  double find(const MESSAGE_KEYS &key);
  const double find(const MESSAGE_KEYS &key) const;
};

#endif // !MESSAGE_H
