#pragma once

#include <iostream>
#include <cstdint>
#include <cmath>
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

        using index_t = std::uint32_t;

        using Segment = bip::managed_windows_shared_memory;
        using Manager = Segment::segment_manager;
        using String = bip::basic_string<char, std::char_traits<char>>;

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
        using Messages = boost::lockfree::spsc_queue<Message, boost::lockfree::allocator<MessageAllocator>>;

        using bip::create_only;

        inline void remove(Segment& memory) { memory.destroy<Messages>("Messages"); };
    }

    class Producer
    {
    private:
        Shared::Segment memory_;
        Shared::MessageAllocator message_allocator_;
        Shared::Messages messages_;
        Shared::index_t index_ = 0;
    public:
        inline Producer(size_t max_message) :
            memory_(Shared::create_only, "Local\\CppToPythonChat", 10ul << 20), // 10 Mib per segment
            message_allocator_(memory_.get_segment_manager()),
            messages_(max_message, message_allocator_) 
        {
        }
        void write_message(const Shared::String& message);
        inline virtual ~Producer() noexcept { Shared::remove(memory_); }
    };
}