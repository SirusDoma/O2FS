#ifndef O2MUSICHOK_OTWO_HPP
#define O2MUSICHOK_OTWO_HPP

#include <windows.h>
#include <vector>
#include <string>

#define GAMEAPI __fastcall

namespace O2MusicHook
{
    class OTwo
    {
    private:
        static std::vector<char> LoadChart(std::string fileName);

        static BOOL   WINAPI ReadBuffer(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
        static DWORD  WINAPI GetBufferSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
        static HANDLE WINAPI GetFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);

        static int GAMEAPI LoadMusic(void* ecx, void* edx, char* ojmFileName);
        static int GAMEAPI ParseOJN(void* ecx, void* edx, LPCSTR lpFileName, int MusicID, int Difficulty, int pad1, unsigned int seed);
        static int GAMEAPI ReadFileOJN(void* ecx, void* edx, void** data);

        static size_t CDECL OpenFileOJN(LPCSTR lpFileName, void** data);
        static size_t CDECL OpenFileOJNHeader(LPCSTR lpFileName, void** data);

        static BOOL GAMEAPI LoadCover(void* ecx, void* edx, void* buffer, SIZE_T dwBytes);

    public:
        static void Hook(HMODULE hMod);
        static bool Ready();
    };
}

#endif
