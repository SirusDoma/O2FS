#ifndef O2FS_O2MUSICWRITER_HPP
#define O2FS_O2MUSICWRITER_HPP

#include <vector>

#include <O2FS/Charting/Chart.hpp>

namespace O2FS
{
    class O2MusicWriter
    {
    public:
        static std::vector<char> Serialize(const Chart& chart);
    };
}

#endif //O2FS_O2MUSICWRITER_HPP
