#ifndef O2FS_LOGGER_HPP
#define O2FS_LOGGER_HPP

#include <fstream>

namespace O2FS
{
    class Logger
    {
    public:
        static void Write(const char* format, ...)
        {
            static bool init = true;
            auto file = fopen("o2fs.log", init ? "w" : "a");
            {
                va_list argptr;
                va_start(argptr, format);
                vfprintf(file, format, argptr);
                va_end(argptr);
            }
            fclose(file);
            init = false;
        }
    };
}

#endif //O2FS_LOGGER_HPP
