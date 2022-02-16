#ifndef O2FS_O2IMAGESYSTEM_HPP
#define O2FS_O2IMAGESYSTEM_HPP

#include <windows.h>
#include <O2FS/FileSystem.hpp>

namespace O2FS
{
    class O2ImageSystem : public FileSystem
    {
    private:
        mutable HANDLE hOPI, hOPA;

    public:
        O2ImageSystem();
        virtual ~O2ImageSystem();

        virtual bool Check(HANDLE hFile) const;

        virtual BOOL  WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
        virtual DWORD WINAPI GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
    };
}

#endif //O2FS_O2IMAGESYSTEM_HPP
