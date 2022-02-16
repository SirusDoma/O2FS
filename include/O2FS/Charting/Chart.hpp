#ifndef O2FS_CHART_HPP
#define O2FS_CHART_HPP

#include <unordered_map>
#include <string>
#include <vector>

#include <O2FS/Charting/Difficulty.hpp>
#include <O2FS/Charting/Genre.hpp>
#include <O2FS/Charting/Event.hpp>

namespace O2FS
{
    struct EventBlock
    {
        int measure;
        ChannelType channel;
        int tempo;
        std::vector<Event> events;
    };

    struct Chart
    {
        int id;
        std::string title, artist, noteDesigner;
        float bpm;
        Genre genre;
        std::vector<char> coverData, thumbnailData;
        std::unordered_map<Difficulty, int> levels;
        std::unordered_map<Difficulty, int> durations;
        std::unordered_map<std::string, int> samples;
        std::unordered_map<Difficulty, std::vector<EventBlock>> blocks;
    };
}

#endif //O2FS_CHART_HPP
