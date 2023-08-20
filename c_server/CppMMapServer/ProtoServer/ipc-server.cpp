#include <windows.h>
#include "ipc-server.h"

bool ipc_server::CreateMemoryMap(ipc_server::SharedMemory* shm)
{
    if ((shm->hFileMap = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        shm->Size,
        shm->MapName)) == NULL)
    {
        return false;
    }

    if ((shm->pData = MapViewOfFile(
        shm->hFileMap,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        shm->Size)) == NULL)
    {
        CloseHandle(shm->hFileMap);
        return false;
    }
    return true;
}

bool ipc_server::FreeMemoryMap(ipc_server::SharedMemory* shm)
{
    if (shm && shm->hFileMap)
    {
        if (shm->pData)
        {
            UnmapViewOfFile(shm->pData);
        }

        if (shm->hFileMap)
        {
            CloseHandle(shm->hFileMap);
        }
        return true;
    }
    return false;
}
