#include "ipc-chat.h"

template<class TElement>
ipc_chat::Sync::SyncQueue<TElement>::~SyncQueue() noexcept
{
	wait_condition_.notify_all();
}

template<class TElement>
inline void ipc_chat::Sync::SyncQueue<TElement>::push(TElement element)
{
	Shared::Guard lock(mutex_);

	queue_ring_.push(element);
	
	wait_condition_.notify_one();
}

template<class TElement>
void ipc_chat::Sync::SyncQueue<TElement>::pop(TElement& element)
{
	Shared::Guard lock(mutex_);

	while (queue_ring_.read_available() == 0)
	{
		wait_condition_.wait(lock);
	}

	element = queue_ring_.front();
	queue_ring_.pop();
}

template<class TElement>
bool ipc_chat::Sync::SyncQueue<TElement>::empty() const
{
	Shared::Guard lock(mutex_);

	return queue_ring_.read_available() == 0;
}

ipc_chat::ChatWriter::~ChatWriter() noexcept
{
	Shared::remove<Message>(memory_);
	std::cout << "remove chared chat" << std::endl;
}

void ipc_chat::ChatWriter::wait_to_request()
{
	while (!messages_ptr_->empty())
		continue;
}

void ipc_chat::ChatWriter::send_message(Message message)
{
	messages_ptr_->push(message);
}

ipc_chat::ChatReader::~ChatReader() noexcept
{
}

ipc_chat::Message ipc_chat::ChatReader::read_message()
{
	Message message(0, "empty");
	messages_ptr_->pop(message);

	return message;
}

void __cdecl write_message(const char* message)
{
	using namespace ipc_chat;

	static Message::index_t index = 0;
	static std::string queue_name("Chat");
	static ChatWriter writer(queue_name);

	try 
	{		
		writer.wait_to_request();

		Message sending_message(++index, message);
		writer.send_message(sending_message);
	}
	catch (Shared::bip::interprocess_exception exception)
	{
		std::cerr << exception.what() << std::endl;
	}

	std::cout << "message #" << index << " sent" << std::endl;
}

int __cdecl read_message(const char** message)
{
	using namespace ipc_chat;

	static std::string queue_name("Chat");

	try 
	{
		ChatReader reader(queue_name);

		auto read_message = reader.read_message();

		*message = read_message.text.c_str();

		std::cout << "message #" << read_message.index << " read" << std::endl;

		return read_message.index;
	}
	catch (Shared::bip::interprocess_exception exception)
	{
		std::cerr << exception.what() << std::endl;
	}
}
