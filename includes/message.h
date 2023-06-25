#ifndef MESSAGE_H
#define MESSAGE_H

#include "glob.h"

class Message
{
public:
    Message(std::map<std::string, double> const& _msg = std::map<std::string, double> ());

    std::map<std::string, double> msg;
};

#endif // !MESSAGE_H
