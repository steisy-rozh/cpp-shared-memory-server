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
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/string.hpp>

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

            using Segment = bip::managed_windows_shared_memory;
            using Manager = Segment::segment_manager;
            using IOMutex = bip::interprocess_mutex;
            using IOWaitCondition = bip::interprocess_condition;
            using Guard = bip::scoped_lock<IOMutex>;

            const char *MemoryName = "CppToPythonChat.bin";
            const size_t QueueCapacity = 5000;
            const unsigned long MemorySizePerSegment = 10ul << 20; // 10 Mib per segment

            template <class TElement>
            using ElementAllocator = bip::allocator<TElement, Manager>;

            template <class TElement>
            using ElementQueue = bl::spsc_queue<TElement, bl::allocator<ElementAllocator<TElement>>, bl::fixed_sized<true>, bl::capacity<QueueCapacity>>;

            template <class TElement>
            inline void remove(Segment& memory) { memory.destroy<ElementQueue<TElement>>(MemoryName); };
        }

        namespace Sync
        {
            template <class TElement>
            class SyncQueue
            {
            private:
                Shared::ElementQueue<TElement> queue_ring_;
                mutable Shared::IOMutex mutex_;
                mutable Shared::IOWaitCondition wait_condition_;
            public:
                SyncQueue(Shared::ElementAllocator<TElement> allocator) :
                    queue_ring_(Shared::QueueCapacity, allocator) {};

                virtual ~SyncQueue() noexcept;

                void push(TElement element);
                void pop(TElement& element);
                bool empty() const;
            };
        }

        struct Message
        {
            using index_t = std::uint32_t;
            using string = Shared::bip::string;

            index_t index;
            string text;

            Message(index_t index, string text)
                : index(index), text(text) 
            {
                std::cout << "index #" << index << " with message " << text << std::endl;
            };
        };

        using MessageQueue = Sync::SyncQueue<Message>;
        using MessageAllocator = Shared::ElementAllocator<Message>;

        class ChatWriter
        {
        private:
            Shared::Segment memory_;
            MessageAllocator allocator_;
            MessageQueue* messages_ptr_;
        public:
            inline ChatWriter(std::string& queue_name) :
                memory_(Shared::create_only, Shared::MemoryName, Shared::MemorySizePerSegment),
                allocator_(memory_.get_segment_manager()),
                messages_ptr_(memory_.construct<MessageQueue>(queue_name.c_str())(allocator_)) {};
            virtual ~ChatWriter() noexcept;
            void wait_to_request();
            void send_message(Message message);
        };

        class ChatReader
        {
        private:
            Shared::Segment memory_;
            MessageQueue* messages_ptr_;
        public:
            inline ChatReader(std::string& queue_name) :
                memory_(Shared::open_only, Shared::MemoryName),
                messages_ptr_(memory_.find<MessageQueue>(queue_name.c_str()).first) 
            {
                std::cout << "a reader created" << std::endl;
            };
            virtual ~ChatReader() noexcept;
            Message read_message();
        };
    }
}

extern "C"
{
    __declspec(dllexport) void __cdecl write_message(const char* message);
    __declspec(dllexport) int __cdecl read_message(const char** message);
};

