#include "ipc-chat.h"

template<class TElement>
ipc_chat::Sync::SyncQueue<TElement>::~SyncQueue() noexcept
{
	wait_condition_.notify_all();
	message_queue_.reset();
	std::cout << "destroy queue" << std::endl;
}

template<class TElement>
inline void ipc_chat::Sync::SyncQueue<TElement>::push(TElement element)
{
	Shared::Guard lock(mutex_);

	message_queue_.push(element);
	
	wait_condition_.notify_one();
}

template<class TElement>
TElement& ipc_chat::Sync::SyncQueue<TElement>::pop()
{
	Shared::Guard lock(mutex_);

	while (message_queue_.read_available() == 0)
	{
		wait_condition_.wait(lock);
	}

	auto& element = message_queue_.front();
	message_queue_.pop();

	return element;
}

template<class TElement>
bool ipc_chat::Sync::SyncQueue<TElement>::empty() const
{
	Shared::Guard lock(mutex_);

	return message_queue_.read_available() == 0;
}

ipc_chat::ChatWriter::~ChatWriter() noexcept
{
	Shared::remove_queue<SharedMessage>(memory_);
	std::cout << "remove shared chat" << std::endl;
}

void ipc_chat::ChatWriter::wait_to_request()
{
	while (!messages_ptr_->empty())
		continue;
}

ipc_chat::Shared::index_t ipc_chat::ChatWriter::send_message(const Message& message)
{
	static Shared::index_t index = 0;

	SharedMessage::string* data_ptr = memory_.construct<SharedMessage::string>("ChatMessage")(charAllocator_);

	data_ptr->assign(message.text.c_str());

	SharedMessage shared_message(++index, *data_ptr);
	messages_ptr_->push(shared_message);

	return index;
}

ipc_chat::ChatReader::~ChatReader() noexcept
{
}

ipc_chat::Message ipc_chat::ChatReader::read_message()
{
	const auto& message = messages_ptr_->pop();

	SharedMessage::string text = message.text;
	
	std::string str(text.begin(), text.end());
	memory_.destroy<SharedMessage::string>("ChatMessage");

	Message read_message(message.index, str);

	return read_message;
}

void __cdecl write_message(const char* message)
{
	using namespace ipc_chat;
	static Shared::index_t index = 0;

	auto& writer = ChatWriter::Instance();

	try 
	{		
		Message sending_message(++index, message);
		auto sending_index = writer.send_message(sending_message);

		std::cout << "message #" << sending_index << " sent" << std::endl;
	}
	catch (Shared::bip::interprocess_exception exception)
	{
		std::cerr << exception.what() << std::endl;
	}

	writer.wait_to_request();
}

int __cdecl read_message(char** message)
{
	using namespace ipc_chat;

	try 
	{
		ChatReader reader(Shared::QueueName);

		Message read_message = reader.read_message();

		strcpy(*message, read_message.text.c_str());

		std::cout << "message #" << read_message.index << " read: " << read_message.text <<std::endl;

		return read_message.index;
	}
	catch (Shared::bip::interprocess_exception exception)
	{
		std::cerr << exception.what() << std::endl;
	}
}
