#include <iostream>
#include "ipc-chat.h"

void RequestForMessage(ipc_chat::Producer& chat);

int main()
{
    using namespace ipc_chat;

    Producer chat(10);

    while (true)
    {
        RequestForMessage(chat);
    }

    return 0;
}

void RequestForMessage(ipc_chat::Producer& chat)
{
    std::cout << "type a message to the chat: " << std::endl;

    std::string message;

    std::getline(std::cin, message);

    ipc_chat::Shared::String sending_message(message.c_str());

    std::cout << "sending " << sending_message << " to the chat..." << std::endl;

    chat.write_message(sending_message);

    std::cout << "message sent." << std::endl;
}
