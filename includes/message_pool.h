#ifndef MESSAGE_POOL_H
#define MESSAGE_POOL_H

#include "message.h"
#include "object_pool.h"

class MessagePool
{
private:
  static constexpr size_t CHUNK_SIZE = 10000;
  static ObjectPool<Message, CHUNK_SIZE> pool;

public:
  // Get a message from the pool
  static Message *get() { return pool.alloc(); }

  // Return a message to the pool
  static void release(Message *msg)
  {
    if (msg)
      pool.release(msg);
  }

  // Create and initialize a message with data
  template <typename... Args> static Message *create(Args &&...args)
  {
    Message *msg = pool.alloc();
    // Initialize with provided arguments
    msg->initialize(std::forward<Args>(args)...);
    return msg;
  }
};

#endif // MESSAGE_POOL_H
