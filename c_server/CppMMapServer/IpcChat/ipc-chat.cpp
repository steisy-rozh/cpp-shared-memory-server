#include "ipc-chat.h"
#include "string.h"

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

	*message = reading_message.text;

	std::cout << "read a message with index" << reading_message.index << std::endl;

	return reading_message.index;
}

void write_message(const char* message)
{
	using namespace ipc_chat;

	Chat& chat = Chat::StartChat();

	chat.write_message(message);
}

int read_message(char* message)
{
	using namespace ipc_chat;

	Chat& chat = Chat::StartChat();

	Shared::String text;

	Shared::index_t index = chat.read_message(&text);

	Shared::copy(text, message);

	return index;
}

void ipc_chat::Shared::copy(String& str, char* message)
{
	const rsize_t max_size = 1000;

	auto input = str.c_str();
	auto len = std::min(max_size, strlen(input));
	strncpy_s(message, len, input, max_size);
}
