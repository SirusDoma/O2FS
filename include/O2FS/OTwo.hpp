#ifndef O2FS_OTWO_HPP
#define O2FS_OTWO_HPP

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <string>

#include <O2FS/FileSystem.hpp>

namespace O2FS
{
    class OTwo
    {
    private:
        static std::vector<FileSystem*> systems;
        static std::unordered_map<HANDLE, std::string> fileCaches;

        // This enables modification for outer level assets (e.g OJN, OJM, OPI, OPA, OJNList.dat)
        static BOOL   WINAPI HookReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
        static DWORD  WINAPI HookGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);

        // This enables file scanning modification (e.g *.ojn -> *.bms Scanning)
        static HANDLE WINAPI HookFindFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
        static BOOL   WINAPI HookFindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);

        // This allow cache to be invalidated
        static BOOL   WINAPI HookCloseHandle(HANDLE hObject);

    public:
        static bool Hooked();
        static void Hook();
        static void Unhook();

        // Mount FileSystem hook to handle collection of files that being processed by game.
        static bool Mount(FileSystem *fs);
        static std::string GetFileName(HANDLE hFile);

        // Real functions without detours
        static BOOL   WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
        static DWORD  WINAPI GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
        static HANDLE WINAPI FindFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
        static BOOL   WINAPI FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
        static BOOL   WINAPI CloseHandle(HANDLE hObject);
    };
}

#endif //O2FS_OTWO_HPP
