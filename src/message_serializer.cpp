#include "../includes/message_serializer.h"

#include <bsp.h>

void MessageSerializer::send(std::vector<Message> &&msg, int dest)
{
  bsp_size_t num_messages = msg.size();
  /**if (num_messages > MAX_MESSAGES)
    throw std::overflow_error("Demasiados mensajes papurri\n");

  Buffer buffer;

  // copiamos todo el resto de los mensajes
  std::memcpy(buffer.data(), msg.data(), num_messages * sizeof(Message));

  bsp_size_t total_size = num_messages * sizeof(Message);
  bsp_send(dest, &total_size, buffer.data(), total_size);**/
  std::vector<BinaryMessage> bin_mgs = to_binary(msg);
  send_binary(bin_mgs, dest);

  // Clear the vector without deallocating
  msg.clear();
}

void MessageSerializer::send_binary(
    const std::vector<BinaryMessage> &bin_msgs, bsp_pid_t dest
)
{
  bsp_size_t num_messages = bin_msgs.size();
  if (num_messages > MAX_MESSAGES)
    throw std::overflow_error("Too many messages");

  bsp_size_t total_size = num_messages * sizeof(BinaryMessage);

  // Send directly using the binary messages data
  bsp_send(dest, &total_size, bin_msgs.data(), total_size);
}

std::vector<Message> MessageSerializer::receive()
{
  std::vector<BinaryMessage> bin_msgs = receive_binary();
  return from_binary(bin_msgs);
}

std::vector<BinaryMessage> MessageSerializer::receive_binary()
{
  bsp_size_t status;
  bsp_size_t total_size;

  // Get tag with payload size
  bsp_get_tag(&status, &total_size);

  // If no messages received, return empty vector
  if (status == -1)
  {
    printf("Error receiving message queue\n");
    return std::vector<BinaryMessage>();
  }

  size_t num_messages = status / sizeof(BinaryMessage);
  std::vector<BinaryMessage> ret(num_messages);

  // Move payload directly into vector
  bsp_move(ret.data(), status);

  return ret;
}

std::vector<BinaryMessage>
MessageSerializer::to_binary(const std::vector<Message> &msgs)
{
  std::vector<BinaryMessage> bin_msgs;
  bin_msgs.reserve(msgs.size());

  for (const auto &msg : msgs)
  {
    bin_msgs.push_back(BinaryMessage::fromMessage(msg));
  }

  return bin_msgs;
}

std::vector<Message>
MessageSerializer::from_binary(const std::vector<BinaryMessage> &bin_msgs)
{
  std::vector<Message> msgs;
  msgs.reserve(bin_msgs.size());

  for (const auto &bin : bin_msgs)
  {
    msgs.push_back(bin.toMessage());
  }

  return msgs;
}
