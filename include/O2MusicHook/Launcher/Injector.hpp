#ifndef O2MUSICHOK_INJECTOR_HPP
#define O2MUSICHOK_INJECTOR_HPP

#include <string>
#include <O2MusicHook/Launcher/Process.hpp>

namespace O2MusicHook::Launcher
{
    class Injector
    {
    private:
        Process *process;
        void *remoteAddr;
        HANDLE remoteThread;

        bool Close();

    public:
        Injector(Process *process);
        HANDLE Inject(std::string dll_name);
    };
}

#endif