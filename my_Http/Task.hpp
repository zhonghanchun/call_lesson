#pragma once 

#include <iostream>
#include "Protocol.hpp"

class Task
{
public:
    Task(){}
    Task(int _sock)
        :sock(_sock)
    {}
    void Process_on()
    {
        handler(sock);
    }
private:
    int sock;
    CallBack handler;
};
