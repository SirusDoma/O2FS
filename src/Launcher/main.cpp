#include <string>

#include <Launcher/Process.hpp>
#include <Launcher/Injector.hpp>

int main(int argc, char* argv[])
{
    const std::string client = "OTwo.exe";
    std::string args = "";
    for (int i = 0; i < argc; i++)
        args += std::string(argv[i]) + " ";

    auto process = O2FS::Launcher::Process::Create(client, args, true);
    auto injector = O2FS::Launcher::Injector(&process);
    auto remote = injector.Inject("O2FS.dll");

    process.Resume();
    return 0;
}