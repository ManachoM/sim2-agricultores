#include "../includes/message_serializer.h"

#include <bsp.h>

void MessageSerializer::send(const std::vector<Message> &msg, int dest)
{
  bsp_size_t num_messages = msg.size();
  if (num_messages > MAX_MESSAGES)
    throw std::overflow_error("Demasiados mensajes papurri\n");

  Buffer buffer;

  // copiamos todo el resto de los mensajes
  std::memcpy(buffer.data(), msg.data(), num_messages * sizeof(Message));

  bsp_size_t total_size = num_messages * sizeof(Message);
  bsp_send(dest, &total_size, buffer.data(), total_size);
}

std::vector<Message> MessageSerializer::receive()
{
  bsp_size_t status;
  bsp_size_t total_size;

  // Primero, parseamos el tamaño del mensaje
  bsp_get_tag(&status, &total_size);

  // Si no se recibieron mensajes,
  // retornamos un vector vacio.
  if (status == -1)
  {
    printf("Error al recibir cola de mensajes\n");
    return std::vector<Message>();
  }

  size_t num_messages = status / sizeof(Message);

  std::vector<Message> ret(num_messages);

  bsp_move(ret.data(), status);

  return ret;
}
