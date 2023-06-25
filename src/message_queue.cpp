#include "../includes/message_queue.h"

MessageQueue::MessageQueue(int _num_agentes) : queue_array(std::vector<std::queue<const Message *>>(_num_agentes, std::queue<const Message *>()))
{
}

void MessageQueue::send_message(int dest_agent_id, const Message *_msg)
{
    std::queue<const Message *> q = this->queue_array[dest_agent_id];
    q.push(_msg);
    this->message_cont++;
}

std::vector<const Message *> MessageQueue::get_message_list(int agent_id)
{
    std::vector<const Message *> ret;
    std::queue<const Message *> q = this->queue_array[agent_id];
    while (!q.empty())
    {
        ret.push_back(q.front());
        q.pop();
    }
    return ret;
}

MessageQueue::~MessageQueue()
{
    auto l = (int) this->queue_array.size();
    for(int i = 0; i < l; ++i)
    {
        while(!this->queue_array[i].empty())
        {
            const Message * m = this->queue_array[i].front();
            this->queue_array[i].pop();
            delete m;
        }
    }
}