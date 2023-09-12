#include "ipc-chat.h"

ipc_chat::Chat& ipc_chat::Chat::StartChat()
{
	static ipc_chat::Chat chat("CppToPythonChat");
	return chat;
}

void ipc_chat::Chat::write_message(const Shared::String& message)
{
	Shared::Message sending_message{ ++index_, message };
	messages_ptr_->push(sending_message);

	std::cout << "wrote a message with index " << sending_message.index << std::endl;
}

ipc_chat::Shared::index_t ipc_chat::Chat::read_message(Shared::String* message)
{
	if (messages_ptr_->empty())
	{
		std::cout << "no messages in chat" << std::endl;
		return 0;
	}

	auto reading_message = messages_ptr_->front();

	std::cout << "read a message with index" << reading_message.index << std::endl;

	return reading_message.index;
}

void ipc_chat::write_message(const char* message)
{
	Chat& chat = Chat::StartChat();

	chat.write_message(message);
}

int ipc_chat::read_message(const char* message)
{
	Chat& chat = Chat::StartChat();

	Shared::String read_message;

	Shared::index_t index = chat.read_message(&read_message);

	message = read_message.c_str();

	return index;
}
