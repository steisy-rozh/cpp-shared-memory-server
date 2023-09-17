#include <iostream>
#include "ipc-chat.h"

void RequestForMessage();

int main()
{
    using namespace ipc_chat;

    while (true)
    {
        RequestForMessage();
    }

    return 0;
}

void RequestForMessage()
{
    std::cout << "-------------- read a message from the chat: -----------------" << std::endl;

    const int max_size = 1024;

    char* message = new char[max_size];
    int index = read_message(&message);

    if (index > 0)
    {
        std::cout << "got from chat: " << message << std::endl;
    }
    else 
    {
        std::cout << "nothing to read" << std::endl;
    }
    delete[] message;
}
