#include <O2MusicHook/OTwo.hpp>

#include <iostream>
#include <Psapi.h>
#include <fileapi.h>
#include <detours.h>

#include <Chart.hpp>
#include <Event.hpp>
#include <BmsParser.hpp>
#include <OjnWriter.hpp>

#include <O2MusicHook/Log.hpp>

BOOL(WINAPI* TrueReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = ReadFile;
DWORD(WINAPI* TrueGetFileSize)(HANDLE hFile, LPDWORD lpFileSizeHigh) = GetFileSize;
HANDLE(WINAPI* TrueFindFirstFile)(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) = FindFirstFileA;

int(GAMEAPI* TrueLoadMusic)(void* ecx, void* edx, char* ojmFileName) = (int(GAMEAPI*)(void*, void*, char*))0x00536560;
int(GAMEAPI* TrueParseOJN)(
    void* ecx,
    void* edx,
    LPCSTR lpFileName, 
    int MusicID, 
    int Difficulty, 
    int pad1, 
    unsigned int pad2
) = (int(GAMEAPI*)(void*, void*, LPCSTR, int, int, int, unsigned int))0x00537D80;

size_t(CDECL* TrueOpenFileOJN)(LPCSTR lpFileName, void** data)       = (size_t(CDECL*)(LPCSTR, void**))0x00411400;
size_t(CDECL* TrueOpenFileOJNHeader)(LPCSTR lpFileName, void** data) = (size_t(CDECL*)(LPCSTR lpFileName, void**))0x004114E0;

int(GAMEAPI* TrueReadFileOJN)(void* ecx, void* edx, void** data) = (int(GAMEAPI*)(void*, void*, void**))0x00410CF0;

int(GAMEAPI* TrueLoadCover)(void* ecx, void* edx, void* buffer, SIZE_T dwBytes) = (int(GAMEAPI*)(void*, void*, void*, SIZE_T))0x0041D480;

namespace O2MusicHook
{
    static Chart chart;
    static std::vector<char> chartData;

    void OTwo::Hook(HMODULE hMod)
    {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueReadFile, ReadBuffer);
        DetourAttach(&(PVOID&)TrueGetFileSize, GetBufferSize);
        //DetourAttach(&(PVOID&)TrueFindFirstFile, GetFirstFile);

        // Game function hook - Only for debugging purposes
        DetourAttach(&(PVOID&)TrueLoadMusic, LoadMusic);
        DetourAttach(&(PVOID&)TrueParseOJN, ParseOJN);
        DetourAttach(&(PVOID&)TrueOpenFileOJN, OpenFileOJN);
        DetourAttach(&(PVOID&)TrueReadFileOJN, ReadFileOJN);
        DetourAttach(&(PVOID&)TrueOpenFileOJNHeader, OpenFileOJNHeader);
        DetourAttach(&(PVOID&)TrueLoadCover, LoadCover);
        DetourTransactionCommit();
    }

    bool OTwo::Ready()
    {
        return !DetourIsHelperProcess();
    }

    BOOL WINAPI OTwo::ReadBuffer(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
    {
        /** API Hook
         * This function will allow you to manipulate outer level asset (opi, opa, ojn, ojm, ojnlist)
         * that means lot of control! you can load anything and shove it into the memory buffer, just make sure the game recognize it properly
         *
         * Inner level asset (ojs, ojt, audio samples inside ojm) will require you to hook game function hook instead of this WinAPI hook
         */
        static bool hooking = false;
        static HANDLE hOpi, hOpa;
        if (hFile == hOpa || hFile == hOpi || hooking) // hooking flag to deactivate hooking when actual impl need to be called directly
            return TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

        // Target buffer is the buffer that the game will process after this function finishes
        // Offset represent current file stream offset, the data you copy into buffer need to be match with the offset and nNumberOfBytesToRead
        char* targetBuffer = (char*)lpBuffer;
        int offset = 0;

        std::string fileName = "";
        if (hFile != hOpa && hFile != hOpi)
        {
            LARGE_INTEGER pos = { 0 };
            if (SetFilePointerEx(hFile, LARGE_INTEGER{ 0 }, &pos, FILE_CURRENT))
                offset = pos.QuadPart;

            char path[MAX_PATH];
            if (GetFinalPathNameByHandleA(hFile, path, MAX_PATH, FILE_NAME_OPENED) > 0)
                fileName = std::string(path);
        }

        BOOL result = TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
        if (fileName.find(".exe") != std::string::npos)
            return result;

        if (fileName.find(".dll") != std::string::npos)
            return result;

        // TODO: Hook OJNList to provide valid ojn header based on bms file
        // Alternatively, sync OJNList file beforehand to align with BMS output instead of altering the data from memory (require client restart)
        if (fileName.find(".dat") != std::string::npos)
            return result;

        // TODO: List of music files that require hooking, Use OJNList to mark them or use FindFirstFile hook to determine music file directly
        if (fileName.find("o2ma100.ojn") != std::string::npos) // if (fileName.find(".ojn") != std::string::npos)
        {
            // This flag help is used to by pass hook, without this, all function regarding I/O files will be rerouted to this function again
            // And it may cause stackoverflow due to infinite loop function call
            hooking = true;

            auto data = LoadChart(fileName);
            memcpy(&targetBuffer[0], &data[offset], nNumberOfBytesToRead);
            *lpNumberOfBytesRead = nNumberOfBytesToRead;

            hooking = false;
            return result;
        }

        // TODO: OJM need to be considered as well, whether stick with OJM file or use bunch of raw sample files
        // You need hooking into the game function if you go with raw sample files route 
        // OJM file also need to be rebuilded wwith those samples, which sounds a little bit nosense if you consider the load time and audio format conversion lol
        if (fileName.find(".ojm") != std::string::npos)
            return result;

        if (fileName.find(".bin") != std::string::npos)
            return result;

        // Cache file handle, these files are quite large
        if (fileName.find(".opi") != std::string::npos)
        {
            hOpi = hFile;
            return result;
        }

        if (fileName.find(".opa") != std::string::npos)
        {
            hOpa = hFile;
            return result;
        }

        return result;
    }

    DWORD WINAPI OTwo::GetBufferSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
    {
        std::string fileName;
        char path[MAX_PATH];
        if (GetFinalPathNameByHandleA(hFile, path, MAX_PATH, FILE_NAME_OPENED) > 0)
            fileName = std::string(path);

        DWORD size = 0;
        if (fileName.find("o2ma100.ojn") != std::string::npos)
        {
            auto data = LoadChart(fileName);
            size = data.size();
        }

        DWORD result = TrueGetFileSize(hFile, lpFileSizeHigh);
        return size == 0 ? result : size;
    }

    HANDLE WINAPI OTwo::GetFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
    {
        
       /** File Detection 
        * TODO: Add hook for FindFirstFileA and FindNextFileA
        * Can be implemented so the extension of filename became ".bms" instead of ".ojn"
        * 
        * The main idea is to let the FindFirstFileA intact without hook so the file searching loop is executed, then hook the FindNextFileA
        * In this hook, call the original FindFirstFileA to retrieve handle for "*.bms" files, then stub all preceeding FindNextFileA to this handle
        * Once all .bms files enumerated, revert the hook and let the program call original FindNextFileA with the original handle
        *
        * To replace filename, use following code:
        *
          std::string fileName = std::string(lpFileName);
          if (fileName.find("o2ma100.ojn") != std::string::npos)
          {
              fileName = fileName.replace(fileName.find(".ojn"), sizeof(".ojn") - 1, ".bms");
              lpFileName = fileName.c_str();
          }
        * 
        * More Info:
        * 
        */

        HANDLE result = TrueFindFirstFile(lpFileName, lpFindFileData);    
        return result;
    }

    std::vector<char> OTwo::LoadChart(std::string fileName)
    {
        // TODO: Load for another difficulty
        chart = BmsParser::Parse(fileName);

        // TODO: Parse STAGEFILE in bms and repalce fileName with STAGEFILE
        std::ifstream file("D:\\Experimental\\bms\\cover.jpg", std::ios::binary | std::ios::ate);
        auto size = file.tellg();
        auto coverData = std::vector<char>(size);
        file.seekg(0, std::ios::beg);
        file.read(coverData.data(), size);
        file.close();

        // TODO: Read about OJNList hook description
        // This header should be generated from BMS and synced with OJNList rather than using static template file that only work for single music
        file = std::ifstream("D:\\Experimental\\bms\\header.ojn", std::ios::binary);
        auto headerData = std::vector<char>(300);
        file.read(headerData.data(), size);
        file.close();

        // value like note count, package count and other related gameplay elements are being replaced in this writer
        return OjnWriter::Write(chart, headerData, coverData);
    }

    // Load Music Files (Next: ParseOJN)
    int GAMEAPI OTwo::LoadMusic(void* ecx, void* edx, char* ojmFileName)
    {
        int hr = TrueLoadMusic(ecx, edx, ojmFileName);
        return hr;
    }

    // Parse OJN File (Next: OpenFileOJN)
    int GAMEAPI OTwo::ParseOJN(void* ecx, void* edx, LPCSTR lpFileName, int MusicID, int Difficulty, int modifier, unsigned int seed)
    {
        int hr = TrueParseOJN(ecx, edx, lpFileName, MusicID, Difficulty, modifier, seed);
        return hr;
    }
    
    // Open OJN File to process, contains file checking and reading and decrypting (Next: ReadFileOJN)
    size_t CDECL OTwo::OpenFileOJN(LPCSTR lpFileName, void** data)
    {
        auto fileSize = TrueOpenFileOJN(lpFileName, data);
        return fileSize;
    }
    
    // Read OJN File, will also decrypt the data if it's encrypted
    int GAMEAPI OTwo::ReadFileOJN(void* ecx, void* edx, void** data)
    {
        int result = TrueReadFileOJN(ecx, edx, data);
        return result;
    }
    
    size_t CDECL OTwo::OpenFileOJNHeader(LPCSTR lpFileName, void** data)
    {
        int result = TrueOpenFileOJNHeader(lpFileName, data);
        return result;
    }
    
    BOOL GAMEAPI OTwo::LoadCover(void* ecx, void* edx, void* buffer, SIZE_T dwBytes)
    {
        BOOL result = TrueLoadCover(ecx, edx, buffer, dwBytes);
        return result;
    }
}
