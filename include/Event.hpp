#ifndef CHART_EVENT_HPP
#define CHART_EVENT_HPP

#include <vector>

struct Event
{
    short sampleID;
    bool isLongNote;
    int offset;
};

struct Package
{
    int measure;
    short channel;
    std::vector<Event> events;
};

#endif