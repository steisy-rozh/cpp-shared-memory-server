#pragma once

namespace ipc_server
{
    typedef struct
    {
        void* hFileMap;
        void* pData;
        wchar_t MapName[256];
        size_t Size;
    } SharedMemory;

    bool CreateMemoryMap(SharedMemory* shm);
    bool FreeMemoryMap(SharedMemory* shm);
}