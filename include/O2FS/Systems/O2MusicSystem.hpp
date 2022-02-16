#ifndef O2FS_O2MUSICSYSTEM_HPP
#define O2FS_O2MUSICSYSTEM_HPP

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <O2FS/FileSystem.hpp>


namespace O2FS
{
    class O2MusicSystem : public FileSystem
    {
    private:
        std::unordered_set<std::string> handles;
        std::unordered_map<HANDLE, std::vector<char>> cache;

        bool IsOJN(std::string fileName);
        std::vector<char> LoadChart(HANDLE hFile);

    public:
        O2MusicSystem();
        virtual ~O2MusicSystem();

        virtual bool Check(HANDLE hFile) const;

        virtual BOOL  WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
        virtual DWORD WINAPI GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
        virtual BOOL  WINAPI CloseHandle(HANDLE hObject);
    };
}

#endif //O2FS_O2MUSICSYSTEM_HPP
