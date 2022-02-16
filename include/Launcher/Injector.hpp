#ifndef O2FS_INJECTOR_HPP
#define O2FS_INJECTOR_HPP

#include <string>
#include <Launcher/Process.hpp>

namespace O2FS::Launcher
{
    class Injector
    {
    private:
        Process *process;
        void *remoteAddr;
        HANDLE remoteThread;

    public:
        Injector(Process *process);
        HANDLE Inject(std::string dll);
    };
}

#endif //O2FS_INJECTOR_HPP
