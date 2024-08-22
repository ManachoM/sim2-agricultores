#ifndef _MESSAGE_SERIALIZER_H_
#define _MESSAGE_SERIALIZER_H_

#include "glob.h"
#include "message.h"

#include <bsp.h>

class MessageSerializer
{
private:
  // Para el tamaño del buffer, así se usa un buffer único
  // de memoria estática
  static constexpr bsp_size_t MAX_MESSAGES = 100'000;
  static constexpr bsp_size_t BUFFER_SIZE = MAX_MESSAGES * sizeof(Message);

  using Buffer = std::array<char, BUFFER_SIZE>;

public:
  static void send(const std::vector<Message> &msg, bsp_pid_t dest);
  static std::vector<Message> receive();
};

#endif // !_MESSAGE_SERIALIZER_H_
