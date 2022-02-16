#ifndef O2FS_CHANNEL_HPP
#define O2FS_CHANNEL_HPP

namespace O2FS
{
    enum class ChannelType : short
    {
        None    = -1,
        Measure = 0,
        BPM     = 1,
        Note1   = 2,
        Note2   = 3,
        Note3   = 4,
        Note4   = 5,
        Note5   = 6,
        Note6   = 7,
        Note7   = 8,
        SampleNote
    };
}

#endif //O2FS_CHANNEL_HPP
