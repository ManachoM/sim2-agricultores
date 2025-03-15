#ifndef BINARY_MESSAGE_H
#define BINARY_MESSAGE_H

#include "message.h"

#include <cstdint>

// Compact binary representation of Message
struct BinaryMessage
{
  double amount;
  int32_t prod_id;
  int32_t buyer_id;
  int32_t seller_id;
  int32_t agent_id;
  int16_t agent_type;
  int16_t process;
  int16_t origin;
  int16_t dest;
  uint8_t error;

  // Convert Message to BinaryMessage
  static BinaryMessage fromMessage(const Message &msg)
  {
    BinaryMessage bin;
    bin.amount = msg.find(MESSAGE_KEYS::AMOUNT);
    bin.prod_id = static_cast<int32_t>(msg.find(MESSAGE_KEYS::PROD_ID));
    bin.buyer_id = static_cast<int32_t>(msg.find(MESSAGE_KEYS::BUYER_ID));
    bin.seller_id = static_cast<int32_t>(msg.find(MESSAGE_KEYS::SELLER_ID));
    bin.agent_id = static_cast<int32_t>(msg.find(MESSAGE_KEYS::AGENT_ID));
    bin.agent_type = static_cast<int16_t>(msg.find(MESSAGE_KEYS::AGENT_TYPE));
    bin.process = static_cast<int16_t>(msg.find(MESSAGE_KEYS::PROCESS));
    bin.origin = static_cast<int16_t>(msg.find(MESSAGE_KEYS::ORIGIN_PID));
    bin.dest = static_cast<int16_t>(msg.find(MESSAGE_KEYS::DEST_PID));
    bin.error = static_cast<uint8_t>(msg.find(MESSAGE_KEYS::ERROR) == -1);
    return bin;
  }

  // Convert BinaryMessage to Message
  Message toMessage() const
  {
    Message msg;
    msg.insert(MESSAGE_KEYS::AMOUNT, amount);
    msg.insert(MESSAGE_KEYS::PROD_ID, static_cast<double>(prod_id));
    msg.insert(MESSAGE_KEYS::BUYER_ID, static_cast<double>(buyer_id));
    msg.insert(MESSAGE_KEYS::SELLER_ID, static_cast<double>(seller_id));
    msg.insert(MESSAGE_KEYS::AGENT_ID, static_cast<double>(agent_id));
    msg.insert(MESSAGE_KEYS::AGENT_TYPE, static_cast<double>(agent_type));
    msg.insert(MESSAGE_KEYS::PROCESS, static_cast<double>(process));
    msg.insert(MESSAGE_KEYS::ORIGIN_PID, static_cast<double>(origin));
    msg.insert(MESSAGE_KEYS::DEST_PID, static_cast<double>(dest));
    msg.insert(MESSAGE_KEYS::ERROR, error ? -1.0 : 0.0);
    return msg;
  }
};

#endif // BINARY_MESSAGE_H
