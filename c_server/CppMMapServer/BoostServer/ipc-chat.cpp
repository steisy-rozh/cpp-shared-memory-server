#include "ipc-chat.h"

void ipc_chat::Producer::write_message(const Shared::String& message)
{
	Shared::Message sending_message{ ++index_, message };
	messages_.push(sending_message);

	std::cout << "wrote a message with index " << sending_message.index << std::endl;
}
