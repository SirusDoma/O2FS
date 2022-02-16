#include <Launcher/Injector.hpp>
#include <Logger.hpp>

namespace O2FS::Launcher
{
    Injector::Injector(O2FS::Launcher::Process *process) :
            process(process),
            remoteAddr(0),
            remoteThread(0)
    {
    }

    bool dispose(O2FS::Launcher::Process *process, HANDLE remoteThread, void* remoteAddr)
    {
        // Dispose remote thread
        if (remoteThread != NULL)
            CloseHandle(remoteThread);

        // Free allocated memory for DLL
        if (process && remoteAddr)
        {
            bool success = VirtualFreeEx(process->GetProcessHandle(), remoteAddr, 0, MEM_RELEASE);
            if (!success)
            {
                Logger::Write("O2FS: Failed to clear allocated dll memory (%d).\n", GetLastError());
                return false;
            }
        }

        return true;
    }

    HANDLE Injector::Inject(std::string dll)
    {
        // Lookup DLL to inject
        if (!SearchPath(NULL, dll.c_str(), NULL, 0, NULL, NULL))
        {
            Logger::Write("O2FS: DLL to inject is not found (%s).\n", dll.c_str());
            return NULL;
        }

        // Allocate memory to inject the DLL
        remoteAddr = VirtualAllocEx(
                process->GetProcessHandle(),
                NULL,
                dll.length() + 1,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE
        );

        // Check allocation operation
        if (!remoteAddr)
        {
            Logger::Write("O2FS: Failed to allocate DLL memory (%d).\n", GetLastError());
            return NULL;
        }

        // Write dll name into allocated memory
        bool success = WriteProcessMemory(
                process->GetProcessHandle(),
                remoteAddr,
                dll.c_str(),
                dll.length() + 1,
                NULL
        );

        // Check write DLL operation
        if (!success)
        {
            Logger::Write("O2FS: Failed to write DLL into memory (%d).\n", GetLastError());
            dispose(process, remoteThread, remoteAddr);

            return NULL;
        }

        // Create remote thread to load dll that written into memory
        remoteThread = CreateRemoteThread(
                process->GetProcessHandle(),
                NULL,
                0,
                (LPTHREAD_START_ROUTINE)LoadLibraryA,
                remoteAddr,
                0,
                NULL
        );

        // Validate remote thread
        if (!remoteThread)
        {

            Logger::Write("O2FS: Failed to CreateRemoteThread (%d).\n", GetLastError());
            dispose(process, remoteThread, remoteAddr);

            return remoteThread;
        }

        if (WaitForSingleObject(remoteThread, INFINITE) != WAIT_OBJECT_0)
            Logger::Write("O2FS: Failed to wait object (%d).\n", GetLastError());

        DWORD result;
        if (!GetExitCodeThread(remoteThread, &result))
            Logger::Write("O2FS: Failed to wait exit thread (%d).\n", GetLastError());

        if (result == 0)
            Logger::Write("O2FS: Failed to load inside target process (%d).\n", GetLastError());

        // return created remote thread
        return remoteThread;
    }
}