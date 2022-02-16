#ifndef O2FS_BEMUSICREADER_HPP
#define O2FS_BEMUSICREADER_HPP

#include <vector>

#include <O2FS/Charting/Chart.hpp>

namespace O2FS
{
    class BeMusicReader
    {
    public:
        static Chart Deserialize(const std::string& fileName);
    };
}

#endif //O2FS_BEMUSICREADER_HPP
