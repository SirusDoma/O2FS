#include <windows.h>

#include <O2FS/OTwo.hpp>
#include <O2FS/Systems/O2MusicSystem.hpp>
#include <O2FS/Systems/O2ImageSystem.hpp>

using namespace O2FS;

O2ImageSystem o2ifs;
O2MusicSystem o2mfs;

// This function should always return TRUE otherwise the library won't be loaded
// More Info: https://docs.microsoft.com/en-us/windows/win32/dlls/dllmain
BOOL APIENTRY DllMain(HMODULE hMod, DWORD cause, LPVOID lpReserved)
{
    if (OTwo::Hooked())
        return TRUE;

    switch (cause)
    {
        case DLL_PROCESS_ATTACH:
            // Mount supported FileSystem hooks
            OTwo::Mount(&o2ifs);
            OTwo::Mount(&o2mfs);

            // Attach hook into OTwo.exe
            OTwo::Hook();
            break;
        case DLL_PROCESS_DETACH:
            // Detach hook from OTwo.exe
            OTwo::Unhook();
            break;
        default:
            break;
    }

    return TRUE;
}
