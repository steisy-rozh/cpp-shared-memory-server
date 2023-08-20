#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>
#include <stdexcept>
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "ipc-server.h"
#include "ipc-chat.h"

using namespace boost::interprocess;

template<>
bool ipc_server::CreateMemoryMap<ipc_chat::Message>(const ipc_server::SharedMemory& shm)
{
	assert(shm.Size > sizeof(ipc_chat::Message), "memory must be ready to keep at least one message.");

	std::string memory_name(shm.MapName);
	windows_shared_memory shdmem (open_or_create, shm.MapName, read_write, shm.Size);
	shared_memories_map[memory_name] = std::make_shared<windows_shared_memory>(shdmem);

	return shared_memories_map[memory_name]->get_size() > 0;
}

template<>
void ipc_server::ReadFromMemory<ipc_chat::Message>(const SharedMemory& shm, messages& output)
{
	auto got = shared_memories_map.find(shm.MapName);

	if (got == shared_memories_map.end())
	{
		throw std::invalid_argument("unknown shared memory name got.");
	}

	mapped_region chat_read_region(*(got->second), read_only);

	const auto size = chat_read_region.get_size() / sizeof(ipc_chat::Message);

	ipc_chat::Message* messages = static_cast<ipc_chat::Message*>(chat_read_region.get_address());

	auto read = size;

	while (--read)
	{
		auto message = *(messages++);

		std::cout << "read message with index " << message.Index << std::endl;

		output.push_back(message);
	};

	chat_read_region.shrink_by(size * sizeof(ipc_chat::Message));
}

template<>
void ipc_server::WriteToMemory<ipc_chat::Message>(const SharedMemory& shm, const ipc_chat::Message& data)
{
	auto got = shared_memories_map.find(shm.MapName);

	if (got == shared_memories_map.end())
	{
		throw std::invalid_argument("unknown shared memory name got.");
	}

	mapped_region chat_write_region(*(got->second), read_write);


}

typedef std::unordered_map<std::string, std::shared_ptr<windows_shared_memory>> named_memory_map;
typedef std::vector<ipc_chat::Message> messages;

static named_memory_map shared_memories_map;
