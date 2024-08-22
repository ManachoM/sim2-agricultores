#include "../includes/message.h"

Message::Message() {}

void Message::insert(const MESSAGE_KEYS &key, const double &val)
{
  switch (key)
  {
  case MESSAGE_KEYS::AMOUNT:
    this->amount = val;
    break;
  case MESSAGE_KEYS::PROD_ID:
    this->prod_id = (int)val;
    break;
  case MESSAGE_KEYS::BUYER_ID:
    this->buyer_id = (int)val;
    break;
  case MESSAGE_KEYS::SELLER_ID:
    this->seller_id = (int)val;
    break;
  case MESSAGE_KEYS::AGENT_ID:
    this->agent_id = (int)val;
    break;
  case MESSAGE_KEYS::AGENT_TYPE:
    this->agent_type = (short)val;
    break;
  case MESSAGE_KEYS::PROCESS:
    this->process = (short)val;
    break;
  case MESSAGE_KEYS::ORIGIN_PID:
    this->origin = (short)val;
    break;
  case MESSAGE_KEYS::DEST_PID:
    this->dest = (short)val;
    break;
  case MESSAGE_KEYS::ERROR:
    this->error = val;
    break;
  }
}

double Message::find(const MESSAGE_KEYS &key)
{
  switch (key)
  {
  case MESSAGE_KEYS::AMOUNT:
    return (double)this->amount;
  case MESSAGE_KEYS::PROD_ID:
    return (double)this->prod_id;
  case MESSAGE_KEYS::BUYER_ID:
    return (double)this->buyer_id;
  case MESSAGE_KEYS::SELLER_ID:
    return (double)this->seller_id;
  case MESSAGE_KEYS::AGENT_ID:
    return (double)this->agent_id;
  case MESSAGE_KEYS::AGENT_TYPE:
    return (double)this->agent_type;
  case MESSAGE_KEYS::PROCESS:
    return (double)this->process;
  case MESSAGE_KEYS::ORIGIN_PID:
    return (double)this->origin;
  case MESSAGE_KEYS::DEST_PID:
    return (double)this->dest;
  case MESSAGE_KEYS::ERROR:
    return -(double)this->error;
  default:
    return -999999.99999;
  }
}

const double Message::find(const MESSAGE_KEYS &key) const
{
  switch (key)
  {
  case MESSAGE_KEYS::AMOUNT:
    return (double)this->amount;
  case MESSAGE_KEYS::PROD_ID:
    return (double)this->prod_id;
  case MESSAGE_KEYS::BUYER_ID:
    return (double)this->buyer_id;
  case MESSAGE_KEYS::SELLER_ID:
    return (double)this->seller_id;
  case MESSAGE_KEYS::AGENT_ID:
    return (double)this->agent_id;
  case MESSAGE_KEYS::AGENT_TYPE:
    return (double)this->agent_type;
  case MESSAGE_KEYS::PROCESS:
    return (double)this->process;
  case MESSAGE_KEYS::ERROR:
    return -(double)this->error;
  default:
    return -999999.99999;
  }
}
