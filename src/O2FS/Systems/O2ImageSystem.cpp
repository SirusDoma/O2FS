#include <O2FS/Systems/O2ImageSystem.hpp>
#include <O2FS/OTwo.hpp>

#include <string>

namespace O2FS
{

    O2ImageSystem::O2ImageSystem() :
        hOPI(0),
        hOPA(0)
    {
    }

    O2ImageSystem::~O2ImageSystem()
    {
    }

    bool O2ImageSystem::Check(HANDLE hFile) const
    {
        if (hFile == hOPI || hFile == hOPA)
            return true;

        auto fileName = OTwo::GetFileName(hFile);
        if (fileName.find(".opi") != std::string::npos)
            hOPI = hFile;

        if (fileName.find(".opa") != std::string::npos)
            hOPA = hFile;

        return hFile == hOPI || hFile == hOPA;
    }

    BOOL O2ImageSystem::ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
    {
        return OTwo::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    }

    DWORD O2ImageSystem::GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
    {
        return OTwo::GetFileSize(hFile, lpFileSizeHigh);
    }
}