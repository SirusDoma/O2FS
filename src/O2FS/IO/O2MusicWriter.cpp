#include <O2FS/IO/O2MusicWriter.hpp>

#include <sstream>

#include <boolinq.h>

namespace O2FS
{
    std::vector<char> O2MusicWriter::Serialize(const Chart &chart)
    {
        auto stream = std::stringstream(std::ios::in | std::ios::out | std::ios::binary);

        char sign[4]    = { 'o', 'j', 'n', '\0' };
        float encVer    = 2.90f;
        short oldEncVer = 29;

        int id    = chart.id;
        float bpm = chart.bpm;
        int genre = (int)chart.genre;

        // Write metadata
        // More Info: https://open2jam.wordpress.com/the-ojn-documentation/
        stream.write(reinterpret_cast<char*>(&id), sizeof(id));
        stream.write(&sign[0], sizeof(sign));
        stream.write(reinterpret_cast<char*>(&encVer), sizeof(encVer));
        stream.write(reinterpret_cast<char*>(&genre), sizeof(genre));
        stream.write(reinterpret_cast<char*>(&bpm), sizeof(bpm));

        for (int l = 0; l < 4; l++)
        {
            auto it = chart.levels.find((Difficulty)l);
            short level = it != chart.levels.end() ? it->second : 0;

            stream.write(reinterpret_cast<char*>(&level), sizeof(level));
        }

        for (int l = 0; l < 3; l++)
        {
            auto it = chart.blocks.find((Difficulty)l);
            int eventCount = it == chart.blocks.end() ? 0 : boolinq::from(it->second).sum([](const EventBlock& block) {
                return block.events.size();
            });

            stream.write(reinterpret_cast<char*>(&eventCount), sizeof(eventCount));
        }

        for (int l = 0; l < 3; l++)
        {
            auto it = chart.blocks.find((Difficulty)l);
            int noteCount = it == chart.blocks.end() ? 0 : boolinq::from(it->second).sum([](const EventBlock& block) {
                return boolinq::from(block.events).count([](const Event& ev){
                    return ev.channel != ChannelType::Measure &&
                           ev.channel != ChannelType::BPM &&
                           ev.channel != ChannelType::SampleNote;
                });
            });

            stream.write(reinterpret_cast<char*>(&noteCount), sizeof(noteCount));
        }

        for (int l = 0; l < 3; l++)
        {
            auto it = chart.blocks.find((Difficulty)l);
            int measureCount = it == chart.blocks.end() ? 0 : it->second[it->second.size() - 1].measure;

            stream.write(reinterpret_cast<char*>(&measureCount), sizeof(measureCount));
        }

        for (int l = 0; l < 3; l++)
        {
            auto it = chart.blocks.find((Difficulty)l);
            int blockCount = it == chart.blocks.end() ? 0 : it->second.size();

            stream.write(reinterpret_cast<char*>(&blockCount), sizeof(blockCount));
        }

        short oldId = chart.id;
        char oldGenre[20] = {};
        int oldFileVersion = 0;
        stream.write(reinterpret_cast<char*>(&oldEncVer), sizeof(oldEncVer));
        stream.write(reinterpret_cast<char*>(&oldId), sizeof(oldId));
        stream.write(reinterpret_cast<char*>(&oldGenre[0]), sizeof(oldGenre));

        int thumbnailSize = chart.thumbnailData.size();
        stream.write(reinterpret_cast<char*>(&thumbnailSize), sizeof(thumbnailSize));
        stream.write(reinterpret_cast<char*>(&oldFileVersion), sizeof(oldFileVersion));

        char title[64];
        memset(&title[0], 0, sizeof(title));
        chart.title.copy(title, chart.title.size());
        stream.write(reinterpret_cast<char*>(&title[0]), sizeof(title));

        char artist[32];
        memset(&artist[0], 0, sizeof(artist));
        chart.artist.copy(artist, sizeof(artist));
        stream.write(reinterpret_cast<char*>(&artist[0]), sizeof(artist));

        char noteDesigner[32];
        memset(&noteDesigner[0], 0, sizeof(noteDesigner));
        chart.noteDesigner.copy(noteDesigner, sizeof(noteDesigner));
        stream.write(reinterpret_cast<char*>(&noteDesigner[0]), sizeof(noteDesigner));

        auto ojmFileName = "o2ma" + std::to_string(chart.id) + ".ojm";
        char ojm[32];
        memset(&ojm[0], 0, sizeof(ojm));
        ojmFileName.copy(ojm, sizeof(ojm));
        stream.write(reinterpret_cast<char*>(&ojm[0]), sizeof(ojm));

        int coverSize = chart.coverData.size();
        stream.write(reinterpret_cast<char*>(&coverSize), sizeof(coverSize));

        for (int l = 0; l < 3; l++)
        {
            auto it = chart.durations.find((Difficulty)l);
            int duration = it == chart.durations.end() ? 0 : it->second;

            stream.write(reinterpret_cast<char*>(&duration), sizeof(duration));
        }

        for (int l = 0; l < 3; l++)
        {
            // Note offset need to be revisited later after writing note data
            int startOffset = 300;
            stream.write(reinterpret_cast<char*>(&startOffset), sizeof(startOffset));
        }

        // Image offset need to be revisited later after writing note data
        int imageOffset = 0;
        stream.write(reinterpret_cast<char*>(&imageOffset), sizeof(imageOffset));


        // Writing Note Data
        // More Info: https://open2jam.wordpress.com/2010/10/05/the-notes-section/

        int offsets[3] = { 0, 0, 0};
        for (int l = 0; l < 3; l++)
        {
            auto it = chart.blocks.find((Difficulty)l);
            if (it == chart.blocks.end())
                continue;

            auto blocks = it->second;
            offsets[l]  = stream.tellp();

            for (auto block : blocks)
            {
                auto events = block.events;
                if (events.empty())
                    continue;

                int measure   = block.measure;
                short channel = (short)block.channel;
                short tempo   = block.tempo;

                // Write block header
                stream.write(reinterpret_cast<char*>(&measure), sizeof(measure));
                stream.write(reinterpret_cast<char*>(&channel), sizeof(channel));
                stream.write(reinterpret_cast<char*>(&tempo), sizeof(tempo));

                int offset = 0;
                for (auto ev : events)
                {
                    int gap = ev.cell - offset;
                    for (int i = 0; i < gap; i++)
                    {
                        int dummy = 0;
                        stream.write(reinterpret_cast<char*>(&dummy), sizeof(dummy));
                    }

                    if (ev.channel == ChannelType::BPM || ev.channel == ChannelType::Measure)
                    {
                        float value = ev.value;
                        stream.write(reinterpret_cast<char*>(&value), sizeof(value));
                    }
                    else
                    {
                        short sampleId = ev.id;
                        char flags     = 0;
                        char type      = (char)ev.type;

                        stream.write(reinterpret_cast<char*>(&sampleId), sizeof(sampleId));
                        stream.write(reinterpret_cast<char*>(&flags), sizeof(flags));
                        stream.write(reinterpret_cast<char*>(&type), sizeof(type));
                    }

                    offset = ev.cell + 1;
                }

                // Fill the remaining cells
                for (; offset < tempo; offset++)
                {
                    int dummy = 0;
                    stream.write(reinterpret_cast<char*>(&dummy), sizeof(dummy));
                }
            }
        }

        // Cover and Thumbnail
        imageOffset = stream.tellp();
        auto coverData = chart.coverData;
        stream.write(coverData.data(), coverData.size());

        auto thumbnailData = chart.thumbnailData;
        stream.write(thumbnailData.data(), thumbnailData.size());

        // Revisit chart and image offset
        stream.seekp(284, std::ios::beg);
        stream.write(reinterpret_cast<char*>(&offsets[0]), sizeof(offsets));
        stream.write(reinterpret_cast<char*>(&imageOffset), sizeof(imageOffset));


        // Dump data into vector
        stream.seekp(0, std::ios::end);
        auto output = std::vector<char>(stream.tellp());

        stream.seekp(0, std::ios::beg);
        stream.read(output.data(), output.size());
        stream.clear();

        return output;
    }
}