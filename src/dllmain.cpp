#include <O2MusicHook/OTwo.hpp>

using namespace O2MusicHook;

BOOL APIENTRY DllMain(HMODULE hMod, DWORD cause, LPVOID lpReserved)
{
    if (!OTwo::Ready())
        return TRUE;

    switch (cause)
    {
        case DLL_PROCESS_ATTACH:
            OTwo::Hook(hMod);
            break;
        case DLL_PROCESS_DETACH:
            break;
        default:
            break;
    }

    return TRUE;
}


