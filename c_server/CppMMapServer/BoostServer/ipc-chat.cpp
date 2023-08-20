#include "ipc-chat.h"

void ipc_chat::Producer::write_message(const Shared::String& message)
{
	Shared::StringMessage sending_message{ ++index_, message };
	messages_.push_back(sending_message);

	std::cout << "wrote a message with index " << sending_message.index << std::endl;
}
