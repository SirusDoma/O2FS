#include <O2FS/OTwo.hpp>
#include <detours.h>

#include <Logger.hpp>

// WinAPI Hook Tables
BOOL(WINAPI* TrueReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = ReadFile;
DWORD(WINAPI* TrueGetFileSize)(HANDLE hFile, LPDWORD lpFileSizeHigh) = GetFileSize;
HANDLE(WINAPI* TrueFindFirstFile)(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) = FindFirstFileA;
BOOL(WINAPI *TrueFindNextFile)(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) = FindNextFileA;
BOOL(WINAPI *TrueCloseHandle)(HANDLE hObject) = CloseHandle;

// GameAPI Hook Tables - for debugging purpose only (may varies on client version)
/**
    #define GAMEAPI __fastcall
    int(GAMEAPI* TrueLoadMusic)(void* ecx, void* edx, char* ojmFileName) = (int(GAMEAPI*)(void*, void*, char*))0x00536560;
    size_t(CDECL* TrueOpenFileOJN)(LPCSTR lpFileName, void** data)       = (size_t(CDECL*)(LPCSTR, void**))0x00411400;
    size_t(CDECL* TrueOpenFileOJNHeader)(LPCSTR lpFileName, void** data) = (size_t(CDECL*)(LPCSTR lpFileName, void**))0x004114E0;
    int(GAMEAPI* TrueReadFileOJN)(void* ecx, void* edx, void** data) = (int(GAMEAPI*)(void*, void*, void**))0x00410CF0;
    int(GAMEAPI* TrueLoadCover)(void* ecx, void* edx, void* buffer, SIZE_T dwBytes) = (int(GAMEAPI*)(void*, void*, void*, SIZE_T))0x0041D480;
    int(GAMEAPI* TrueParseOJN)(void* ecx, void* edx, LPCSTR lpFileName, int musicID, int difficulty, int pad1, unsigned int pad2) = (int(GAMEAPI*)(void*, void*, LPCSTR, int, int, int, unsigned int))0x00537D80;
*/

namespace O2FS
{
    std::vector<FileSystem*> OTwo::systems;
    std::unordered_map<HANDLE, std::string> OTwo::fileCaches;

    bool OTwo::Hooked()
    {
        return DetourIsHelperProcess();
    }

    void OTwo::Hook()
    {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueReadFile, HookReadFile);
        DetourAttach(&(PVOID&)TrueGetFileSize, HookGetFileSize);
        DetourAttach(&(PVOID&)TrueFindFirstFile, HookFindFirstFile);
        DetourAttach(&(PVOID&)TrueFindNextFile, HookFindNextFile);
        DetourAttach(&(PVOID&)TrueCloseHandle, HookCloseHandle);
        DetourTransactionCommit();
    }

    void OTwo::Unhook()
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueReadFile, HookReadFile);
        DetourDetach(&(PVOID&)TrueGetFileSize, HookGetFileSize);
        DetourDetach(&(PVOID&)TrueFindFirstFile, HookFindFirstFile);
        DetourDetach(&(PVOID&)TrueFindNextFile, HookFindNextFile);
        DetourAttach(&(PVOID&)TrueCloseHandle, HookCloseHandle);
        DetourTransactionCommit();
    }

    bool OTwo::Mount(FileSystem *fs)
    {
        if (fs)
        {
            // Avoid registering duplicate file system hook
            for (auto f : systems)
            {
                if (f == fs)
                    return false;
            }

            systems.push_back(fs);
            return true;
        }

        return false;
    }

    BOOL OTwo::HookReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
    {
        /** ReadFile
         * This function will allow the hook to manipulate outer level asset (e.g OJN, OJM, OPI, OPA, OJNList.dat).
         * it means lot of control! we can load anything and shove it into the memory buffer, just make sure the game recognize it properly.
         *
         * Inner level asset (OJS, OJI, OJT and audio samples of OJM) will require you to hook game function.
         * It may possible to use WinAPI Hook but it require deep understanding on how the game use WinAPI to deal with inner level assets.
         */

        for (auto fs : systems)
        {
            if (fs->Check(hFile))
                return fs->ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
        }

        // No file system recognize it; pass it into real ReadFile.
        return TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    }

    DWORD OTwo::HookGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
    {
        /** GetFileSize
         * Hooking this function is needed whenever file is intercepted via HookReadFile.
         * This function needs to return intercepted buffer size instead of actual file size.
         * e.g When conversing BMS to OJN, this function shall return OJN size instead of BMS size.
         *
         * Otherwise the game may crash or fail to load chart because file size inconsistency.
         */

        for (auto fs : systems)
        {
            if (fs->Check(hFile))
                return fs->GetFileSize(hFile, lpFileSizeHigh);
        }

        // No file system recognize it; pass it into real GetFileSize.
        return TrueGetFileSize(hFile, lpFileSizeHigh);
    }

    HANDLE OTwo::HookFindFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
    {
        /** FindFirstFile
        * TODO: Add hook for FindFirstFileA and FindNextFileA
        * Can be implemented so the scanning aware of ".bms" files instead of only ".ojn"
        *
        * The main idea is to tamper the pattern of lpFileName argument to accept *.bms as well.
        * This may require more experiments and trial-error to find out whether the game accept the file or not.
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
        * - https://docs.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
        * - https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
        * - https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findnextfilea
        */

        return TrueFindFirstFile(lpFileName, lpFindFileData);
    }

    BOOL OTwo::HookFindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
    {
        /** FindNextFile
         * TODO: Add hook for FindFirstFileA and FindNextFileA
         * Read HookFindFirstFile() description for further details
         */

        return TrueFindNextFile(hFindFile, lpFindFileData);
    }

    BOOL OTwo::HookCloseHandle(HANDLE hObject)
    {
        /** CloseHandle
         * Some of the FileSystem operations can be cached based on given file handle (HANDLE hFile) parameter.
         * However, HANDLE is disposable and once disposed, the same pointer can be reused which invalidate cache.
         *
         * Hook this function whenever a FileSystem need to cache HANDLE, use this to invalidate cache.
         */

        if (auto it = fileCaches.find(hObject); it != fileCaches.end())
            fileCaches.erase(it);

        return TrueCloseHandle(hObject);
    }

    std::string OTwo::GetFileName(HANDLE hFile)
    {
        if (auto it = fileCaches.find(hFile); it != fileCaches.end())
            return it->second;

        char path[MAX_PATH];
        if (GetFinalPathNameByHandleA(hFile, path, MAX_PATH, FILE_NAME_OPENED) > 0)
        {
            auto fileName = std::string(path);
            fileCaches[hFile] = fileName;

            return fileName;
        }

        Logger::Write("O2FS: Failed to retrieve filename from hFile.\n");
        return std::string();
    }

    BOOL OTwo::ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
    {
        return TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    }

    DWORD OTwo::GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
    {
        return TrueGetFileSize(hFile, lpFileSizeHigh);
    }

    HANDLE OTwo::FindFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
    {
        return TrueFindFirstFile(lpFileName, lpFindFileData);
    }

    BOOL OTwo::FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
    {
        return TrueFindNextFile(hFindFile, lpFindFileData);
    }

    BOOL OTwo::CloseHandle(HANDLE hObject)
    {
        return TrueCloseHandle(hObject);
    }
}
