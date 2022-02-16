#ifndef O2FS_FILESYSTEM_HPP
#define O2FS_FILESYSTEM_HPP

#include <windows.h>

namespace O2FS
{
    // An abstract class represents FileSystem hook for specific file type.
    class FileSystem
    {
    public:
        FileSystem() = default;
        virtual ~FileSystem() = default;

        // Check whether this FileSystem able to process given file.
        virtual bool Check(HANDLE hFile) const = 0;

        virtual BOOL   WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = 0;
        virtual DWORD  WINAPI GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) = 0;

        //virtual HANDLE WINAPI FindFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) = 0;
        //virtual HANDLE WINAPI FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) = 0;

        virtual BOOL  WINAPI CloseHandle(HANDLE hObject) { return TRUE; }
    };
}

#endif //O2FS_FILESYSTEM_HPP
