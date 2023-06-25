/**
 * @file message_queue.h
 * @author Manuel Ignacio Manríquez (@ManachoM)
 * @brief First iteration of a message queue object
 * @version 0.1
 * @date 2023-06-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

#include "glob.h"
#include "message.h"

class MessageQueue
{
private:
    std::vector<std::queue<const Message *>> queue_array; /* Guardamos una cola por cada agente*/
    int message_cont = 0;                                 /** Para llevar registro de la cantidad de mensajes que pasan por el sistema*/
public:
    MessageQueue(int _num_agentes);

    std::vector<const Message *> get_message_list(int agent_id);

    void send_message(int dest_agent_id, const Message *_msg);

    ~MessageQueue();
};

#endif // !MESSAGE_QUEUE