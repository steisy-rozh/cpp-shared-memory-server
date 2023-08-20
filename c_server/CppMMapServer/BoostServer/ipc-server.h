#pragma once

#include <vector>

namespace ipc_server
{
    typedef struct
    {
        const char MapName[256];
        size_t Size;
    } SharedMemory;

    template<class TData>
    bool CreateMemoryMap(const SharedMemory& shm);

    template<class TData>
    void ReadFromMemory(const SharedMemory& shm, std::vector<TData>& data);

    template<class TData>
    void WriteToMemory(const SharedMemory& shm, const TData& data);
}
