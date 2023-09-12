#pragma once

#include <iostream>
#include <cstdint>
#include <cmath>
#include <string>
#include <scoped_allocator>
#include <boost/interprocess/managed_windows_shared_memory.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/lockfree/spsc_queue.hpp>

namespace ipc_chat
{
    namespace Shared
    {
        namespace bip = boost::interprocess;
        namespace bl = boost::lockfree;

        using bip::create_only;
        using index_t = std::uint32_t;

        using Segment = bip::managed_windows_shared_memory;
        using Manager = Segment::segment_manager;
        using String = bip::basic_string<char, std::char_traits<char>>;

        const char MemoryName[] = "Local\\CppToPythonChat";
        const size_t QueueCapacity = 5000;

        struct Message
        {
            explicit Message(index_t index, const String& text)
                : index(index)
            {
                auto input = text.c_str();
                auto len = std::min((size_t)1000, strlen(input));
                strncpy_s(this->text, input, len);
            }

            index_t index;
            char text[1000];
        };

        using MessageAllocator = bip::allocator<Message, Manager>;
        using Messages = bl::spsc_queue<Message, bl::allocator<MessageAllocator>, bl::fixed_sized<true>, bl::capacity<QueueCapacity>>;

        inline void remove(Segment& memory, const char* queue_name) { memory.destroy<Messages>(queue_name); };
    }

    class Chat
    {
    private:
        Shared::Segment memory_;
        Shared::MessageAllocator message_allocator_;
        Shared::Messages* messages_ptr_;
        Shared::index_t index_ = 0;
        std::string queue_name_;

        inline Chat(const Shared::String& queue_name) :
            memory_(Shared::create_only, Shared::MemoryName, 10ul << 20), // 10 Mib per segment
            message_allocator_(memory_.get_segment_manager()),
            queue_name_(queue_name),
            messages_ptr_(memory_.construct<Shared::Messages>(queue_name_.c_str())(Shared::QueueCapacity, message_allocator_))
        {}
    public:
        static Chat& StartChat();

        void write_message(const Shared::String& message);
        Shared::index_t read_message(Shared::String* message);
        inline virtual ~Chat() noexcept { Shared::remove(memory_, queue_name_.c_str()); }
    };

    extern "C"
    {
        __declspec(dllexport) void write_message(const char* message);
        __declspec(dllexport) int read_message(const char* message);
    };
}