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
    std::cout << "type a message to the chat: " << std::endl;

    std::string message;

    std::getline(std::cin, message);

    ipc_chat::Shared::String sending_message(message.c_str());

    std::cout << "sending " << sending_message << " to the chat..." << std::endl;

    ipc_chat::write_message(sending_message.c_str());

    std::cout << "message sent." << std::endl;
}
