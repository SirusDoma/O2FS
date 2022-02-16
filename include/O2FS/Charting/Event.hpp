#ifndef O2FS_EVENT_HPP
#define O2FS_EVENT_HPP

#include <O2FS/Charting/Channel.hpp>

namespace O2FS
{
    enum class NoteType : char
    {
        Normal    = 0,
        HoldStart = 2,
        HoldEnd   = 3,
        Auto      = 4
    };

    struct Event
    {
        short id;
        float pan;
        float volume;
        float value;
        int tempo;
        int measure;
        int beat;
        int cell;
        NoteType type;
        ChannelType channel;
    };
}

#endif //O2FS_EVENT_HPP
