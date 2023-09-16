#pragma once

#include <iostream>
#include <cstdint>
#include <cmath>
#include <string>
#include <memory>
#include <scoped_allocator>
#include <boost/interprocess/managed_windows_shared_memory.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

namespace 
{
    namespace ipc_chat
    {
        namespace Shared
        {
            namespace bip = boost::interprocess;
            namespace bl = boost::lockfree;

            using bip::create_only;
            using bip::open_only;
            using bip::open_or_create;
            using mode = bip::mode_t;
            using index_t = std::uint32_t;

            using Segment = bip::managed_windows_shared_memory;
            using Manager = Segment::segment_manager;
            using MessageMutex = bip::named_mutex;
            using String = bip::basic_string<char, std::char_traits<char>>;

            const char MemoryName[] = "Local\\CppToPythonChat";
            const size_t QueueCapacity = 5000;
            const unsigned long memory = 10ul << 20; // 10 Mib per segment

            struct Message
            {
                explicit Message(index_t index, const String& text)
                    : index(index), text(text)
                {
                }

                index_t index;
                String text;
            };

            using MessageAllocator = bip::allocator<Message, Manager>;
            using Messages = bl::spsc_queue<Message, bl::allocator<MessageAllocator>, bl::fixed_sized<true>, bl::capacity<QueueCapacity>>;

            inline void remove(Segment& memory, const char* queue_name) { memory.destroy<Messages>(queue_name); std::cout << "remove the chat" << std::endl; };
        }

        class Chat
        {
        private:
            Shared::Segment memory_;
            Shared::MessageAllocator message_allocator_;
            Shared::Messages* messages_ptr_;
            Shared::index_t index_ = 0;
            std::string queue_name_;
            Shared::MessageMutex mutex_;

            inline Chat(const Shared::String& queue_name, unsigned long memory) :
                memory_(Shared::create_only, Shared::MemoryName, memory), 
                message_allocator_(memory_.get_segment_manager()),
                queue_name_(queue_name),
                messages_ptr_(memory_.construct<Shared::Messages>(queue_name_.c_str())(Shared::QueueCapacity, message_allocator_)),
                mutex_(Shared::open_or_create, "message_exchange_named_mutex")
            {
                std::cout << "welcome to the chat as writer" << std::endl;
            }

            inline Chat(const Shared::String& queue_name) :
                memory_(Shared::open_only, Shared::MemoryName),
                message_allocator_(memory_.get_segment_manager()),
                queue_name_(queue_name),
                messages_ptr_(memory_.find<Shared::Messages>(queue_name.c_str()).first),
                mutex_(Shared::open_or_create, "message_exchange_named_mutex")
            {
                std::cout << "welcome to the chat as reader" << std::endl;
            }
        public:
            static Chat& StartChatAsWriter();
            static Chat& StartChatAsReader();

            void write_message(const Shared::String& message);
            Shared::index_t read_message(std::unique_ptr<Shared::String>& message);
            inline virtual ~Chat() noexcept { Shared::remove(memory_, queue_name_.c_str()); }
        };
    }
}

extern "C"
{
    __declspec(dllexport) void __cdecl write_message(const char* message);
    __declspec(dllexport) int __cdecl read_message(const char** message);
    __declspec(dllexport) void __cdecl free_message(const char** message);
};

