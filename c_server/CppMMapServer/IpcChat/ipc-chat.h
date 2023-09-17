#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <boost/interprocess/managed_windows_shared_memory.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/container/scoped_allocator.hpp>

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

            using index_t = std::uint32_t;
            using AllocatePermissions = bip::permissions;
            using Segment = bip::managed_windows_shared_memory;
            using Manager = Segment::segment_manager;
            using IOMutex = bip::interprocess_mutex;
            using IOWaitCondition = bip::interprocess_condition;
            using Guard = bip::scoped_lock<IOMutex>;

            const char *MemoryName = "CppToPythonChat.bin";
            const char* QueueName = "Chat";
            const size_t QueueCapacity = 5000;
            const unsigned long MemorySizePerSegment = 10ul << 20; // 10 Mib per segment

            template <class TElement>
            using ElementAllocator = boost::container::scoped_allocator_adaptor<bip::allocator<TElement, Manager>>;

            using CharAllocator = bip::allocator<char, Manager>;

            template <class TElement>
            using ElementQueue = bl::spsc_queue<TElement, bl::allocator<ElementAllocator<TElement>>, bl::fixed_sized<true>, bl::capacity<QueueCapacity>>;;

            template <class TElement>
            inline void remove_queue(Segment& memory) 
            {
                std::cout << "try to destroy shared queue..." << std::endl;
                memory.destroy<ElementQueue<TElement>>(QueueName);
                std::cout << "destroyed shared queue " << QueueName << std::endl;
            };

            inline AllocatePermissions GetPermissions()
            {
                AllocatePermissions unrestricted;
                unrestricted.set_unrestricted();
                return unrestricted;
            }
        }

        namespace Sync
        {
            template <class TElement>
            class SyncQueue
            {
            private:
                Shared::ElementQueue<TElement> message_queue_;
                mutable Shared::IOMutex mutex_;
                mutable Shared::IOWaitCondition wait_condition_;
            public:
                SyncQueue(Shared::ElementAllocator<TElement> allocator) :
                    message_queue_(Shared::QueueCapacity, allocator) 
                {
                    std::cout << "queue created" << std::endl;
                };

                virtual ~SyncQueue() noexcept;

                void push(TElement element);
                TElement& pop();
                bool empty() const;
            };

            template <class TElement>
            inline void empty_destroy(SyncQueue<TElement>* queue_ptr)
            {
                std::cout << "do not destroy the queue by reader" << std::endl;
            }
        }

        struct SharedMessage
        {
            using string = Shared::bip::basic_string<char, std::char_traits<char>, Shared::CharAllocator>;

            Shared::index_t index;
            string text;

            SharedMessage(Shared::index_t index, string::allocator_type a)
                : index(index), text(a)
            {
                std::cout << "index #" << index << " with message " << text << std::endl;
            }

            explicit SharedMessage(Shared::index_t index, string text)
                : index(index), text(text)
            {
                std::cout << "index #" << index << " with message " << text << std::endl;
            }
        };

        struct Message
        {
            public:
                using string = std::string;

                Shared::index_t index;
                string text;

                Message(Shared::index_t index, string text)
                    : index(index), text(text)
                {
                    std::cout << "index #" << index << " with message " << text << std::endl;
                }

                Message(const Message& msg) :
                    index(msg.index), text(msg.text)
                {
                    std::cout << "copy index #" << index << " with message " << text << std::endl;
                }
        };

        using MessageQueue = Sync::SyncQueue<SharedMessage>;
        using MessageAllocator = Shared::ElementAllocator<SharedMessage>;

        class ChatWriter
        {
        private:
            Shared::Segment memory_;
            Shared::CharAllocator charAllocator_;
            MessageAllocator messageAllocator_;
            std::unique_ptr<MessageQueue> messages_ptr_;

            inline ChatWriter(const char* queue_name) :
                memory_(Shared::create_only, Shared::MemoryName, Shared::MemorySizePerSegment, nullptr, Shared::GetPermissions()),
                charAllocator_(memory_.get_segment_manager()),
                messageAllocator_(memory_.get_segment_manager()),
                messages_ptr_(memory_.construct<MessageQueue>(queue_name)(messageAllocator_)) 
            {
            };
        public:
            inline static ChatWriter& Instance() { static ChatWriter writer(Shared::QueueName); return writer; };
            virtual ~ChatWriter() noexcept;
            void wait_to_request();
            Shared::index_t send_message(const Message& message);
        };

        class ChatReader
        {
        private:
            Shared::Segment memory_;
            std::unique_ptr<MessageQueue, void(*)(MessageQueue*)> messages_ptr_;
        public:
            inline ChatReader(const char *queue_name) :
                memory_(Shared::open_only, Shared::MemoryName),
                messages_ptr_(memory_.find<MessageQueue>(queue_name).first, Sync::empty_destroy) 
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
    __declspec(dllexport) int __cdecl read_message(char** message);
};

