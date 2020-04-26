#ifndef O2MUSICHOK_LOG_HPP
#define O2MUSICHOK_LOG_HPP

#include <fstream>

namespace O2MusicHook
{
    class Log
    {
    public:

        static void Write(const char* format, ...) 
        {
            static bool init = true;
            FILE* file;
            auto err = fopen_s(&file, "bms.log", init ? "w" : "a");
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

#endif