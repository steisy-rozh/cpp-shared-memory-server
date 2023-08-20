#include "ipc-server.h"
#include <windows.h>
#include <cstring>
#include <cstdbool>
#include <iostream>

const wchar_t MemoryName[256] = L"Local\\Test";

int main()
{
    ipc_server::SharedMemory shm = { 0 };
    shm.Size = 512;
    swprintf_s(shm.MapName, MemoryName);

    if (ipc_server::CreateMemoryMap(&shm))
    {
        char* ptr = (char*)shm.pData;
        memset(ptr, 0, shm.Size);

        while (ptr && (*ptr == 0))
        {
            Sleep(100);
        }

        int size = (int)*ptr;
        size += sizeof(char);

        int i;
        for (i = 0; i < size; ++i)
        {
            std::cout << ptr[i];
        }

        ipc_server::FreeMemoryMap(&shm);
    }
}
