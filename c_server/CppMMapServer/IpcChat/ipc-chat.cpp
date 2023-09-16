#include "ipc-chat.h"
#include "string.h"
#include <memory>
#include <boost/interprocess/sync/scoped_lock.hpp>

ipc_chat::Chat& ipc_chat::Chat::StartChatAsWriter()
{
	static ipc_chat::Chat chat("CppToPythonChat", Shared::memory);
	return chat;
}

ipc_chat::Chat& ipc_chat::Chat::StartChatAsReader()
{
	static ipc_chat::Chat chat("CppToPythonChat");
	return chat;
}

void ipc_chat::Chat::write_message(const Shared::String& message)
{
	Shared::Message sending_message{ ++index_, message };

	{
		boost::interprocess::scoped_lock lock(mutex_);
		messages_ptr_->push(sending_message);
	}

	std::cout << "wrote a message with index " << sending_message.index << std::endl;
}

ipc_chat::Shared::index_t ipc_chat::Chat::read_message(std::unique_ptr<Shared::String>& message)
{
	if (messages_ptr_->empty())
	{
		std::cout << "no messages in chat" << std::endl;
		return 0;
	}

	auto reading_message = messages_ptr_->front();

	*message = reading_message.text;

	std::cout << "read a message with index" << reading_message.index << std::endl;

	{
		boost::interprocess::scoped_lock lock(mutex_);
		messages_ptr_->pop();
		std::cout << "deleted message from queue" << std::endl;
	}

	return reading_message.index;
}

extern "C" void __cdecl write_message(const char* message)
{
	using namespace ipc_chat;

	Chat& chat = Chat::StartChatAsWriter();

	chat.write_message(message);
}

extern "C" int __cdecl read_message(const char** message)
{
	using namespace ipc_chat;

	std::cout << "creating a message queue..." << std::endl;

	try 
	{
		Chat& chat = Chat::StartChatAsReader();

		auto text = std::make_unique<Shared::String>();

		std::cout << "reading a message from the queue..." << std::endl;

		auto index = chat.read_message(text);

		*message = text->c_str();

		return index;
	}
	catch (Shared::bip::interprocess_exception ex)
	{
		std::cerr << ex.what() << ex.get_error_code() << std::endl;
	}
}

extern "C" void __cdecl free_message(const char** message)
{
	delete message;
	std::cout << "message complete" << std::endl;
}
