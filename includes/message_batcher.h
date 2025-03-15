#ifndef MESSAGE_BATCHER_H
#define MESSAGE_BATCHER_H

#include "agent.h"
#include "message.h"

#include <unordered_map>
#include <vector>

class MessageBatcher
{
private:
  // Messages organized by agent type and process type
  std::unordered_map<int, std::unordered_map<int, std::vector<Message>>>
      batched_messages;

public:
  // Add a message to the appropriate batch
  void add(const Message &msg)
  {
    int agent_type = static_cast<int>(msg.find(MESSAGE_KEYS::AGENT_TYPE));
    int process = static_cast<int>(msg.find(MESSAGE_KEYS::PROCESS));
    batched_messages[agent_type][process].push_back(msg);
  }

  // Get all messages of a specific type
  std::vector<Message> &get(int agent_type, int process)
  {
    return batched_messages[agent_type][process];
  }

  // Get all batches for a specific agent type
  std::unordered_map<int, std::vector<Message>> &
  get_agent_batches(int agent_type)
  {
    return batched_messages[agent_type];
  }

  // Clear all batches
  void clear()
  {
    for (auto &[_, processes] : batched_messages)
    {
      for (auto &[_, msgs] : processes)
      {
        msgs.clear();
      }
    }
  }

  // Check if there are any messages in the batches
  bool empty() const
  {
    for (const auto &[_, processes] : batched_messages)
    {
      for (const auto &[_, msgs] : processes)
      {
        if (!msgs.empty())
          return false;
      }
    }
    return true;
  }
};

#endif // MESSAGE_BATCHER_H
