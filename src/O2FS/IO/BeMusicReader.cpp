#include <O2FS/IO/BeMusicReader.hpp>

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>

#include <boolinq.h>

namespace O2FS
{
    static std::vector<std::string> SplitString(const std::string &s, char delim)
    {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, delim))
            result.push_back(item);

        return result;
    }

    static int ParseId(const std::string &fileName)
    {
        auto path = std::filesystem::path(fileName);
        auto fn = path.filename().string();

        std::stringstream ss(fn.substr(4, 4)); // skip "o2ma" part
        std::string id;

        std::getline(ss, id, '.'); // stop at ".ojn"
        return std::stoi(id);
    }

    static Genre ParseGenre(std::string genre)
    {
        if (genre.find("ballad") != std::string::npos)
            return Genre::Ballad;
        else if (genre.find("rock") != std::string::npos)
            return Genre::Rock;
        else if (genre.find("dance") != std::string::npos)
            return Genre::Dance;
        else if (genre.find("techno") != std::string::npos)
            return Genre::Techno;
        else if (genre.find("hipHop") != std::string::npos)
            return Genre::HipHop;
        else if (genre.find("soul") != std::string::npos)
            return Genre::Soul;
        else if (genre.find("jazz") != std::string::npos)
            return Genre::Jazz;
        else if (genre.find("funk") != std::string::npos)
            return Genre::Funk;
        else if (genre.find("classical") != std::string::npos)
            return Genre::Classical;
        else if (genre.find("traditional") != std::string::npos)
            return Genre::Traditional;

        return Genre::Etc;
    };


    Chart BeMusicReader::Deserialize(const std::string& fileName)
    {
        auto chart  = Chart();
        auto stream = std::ifstream(fileName);
        auto line   = std::string();

        chart.id = ParseId(fileName);

        bool holdStates[10];
        memset(holdStates, false, sizeof(holdStates));

        while(std::getline(stream, line))
        {
            // Skip empty or comment line
            if (line.empty() || line[0] == '*')
                continue;

            // Parse chart metadata and keysounds
            auto params = SplitString(line, ' ');
            if (params.size() >= 2)
            {
                auto property = params[0];
                std::string value;
                for (int i = 1; i < params.size(); i++)
                    value += params[i] + " ";

                value = value.substr(0, value.size() - 1);
                if (property == "#TITLE")
                    chart.title = value;

                if (property == "#ARTIST")
                    chart.artist = value;

                if (property == "#PATTERN")
                    chart.noteDesigner = value;

                if (property == "#GENRE")
                    chart.genre = ParseGenre(value);

                if (property == "#BPM")
                    chart.bpm = std::stod(value);

                if (property == "#DURATION")
                {
                    chart.durations = std::unordered_map<Difficulty, int>();
                    for (int l = 0; l < 3; l++)
                        chart.durations[(Difficulty)l] = std::stoi(value);
                }

                if (property == "#PLAYLEVEL")
                {
                    chart.levels = std::unordered_map<Difficulty, int>();
                    for (int l = 0; l < 3; l++)
                        chart.levels[(Difficulty)l] = std::stoi(value);
                }

                if (property == "#STAGEFILE")
                {
                    // Resolve path
                    auto path = std::filesystem::path(fileName);
                    auto coverFileName = path.parent_path().string() + "/" + value;
                    if (fileName.size() > 2 && fileName[0] == '\\' && fileName[1] == '\\')
                        coverFileName = coverFileName.substr(4, coverFileName.size() - 4);

                    // Open stream and initialize data
                    auto coverStream = std::ifstream(coverFileName, std::ios::binary | std::ios::ate);
                    auto coverSize = coverStream.tellg();
                    auto coverData = std::vector<char>(coverSize);

                    // Load cover data
                    coverStream.seekg(0, std::ios::beg);
                    coverStream.read(coverData.data(), coverSize);
                    coverStream.close();

                    // Attach cover data into chart
                    chart.coverData = coverData;
                }

                if (property == "#THUMBFILE")
                {
                    // Resolve path
                    auto path = std::filesystem::path(fileName);
                    auto thumbFileName = path.parent_path().string() + "/" + value;
                    if (fileName.size() > 2 && fileName[0] == '\\' && fileName[1] == '\\')
                        thumbFileName = thumbFileName.substr(4, thumbFileName.size() - 4);

                    // Open stream and initialize data
                    auto thumbnailStream = std::ifstream(thumbFileName, std::ios::binary | std::ios::ate);
                    auto thumbnailSize = thumbnailStream.tellg();
                    auto thumbnailData = std::vector<char>(thumbnailSize);

                    // Load thumbnail data
                    thumbnailStream.seekg(0, std::ios::beg);
                    thumbnailStream.read(thumbnailData.data(), thumbnailSize);
                    thumbnailStream.close();

                    // Attach thumbnail data into chart
                    chart.thumbnailData = thumbnailData;
                }

                // Keysound / Sample note handling
                if (property.substr(0, 4) == "#WAV")
                {
                    auto sampleId = property.substr(4, 2);
                    chart.samples[sampleId] = chart.samples.size() + 1;
                }
            }

            // Parse chart note data
            params = SplitString(line, ':');
            if (params.size() == 2)
            {
                // Format:
                // MMMCC: IDIDIDIDIDID
                //
                // M:  Measure
                // C:  Channel
                // ID: Sample ID (0 mean empty space)

                auto time = params[0];
                auto note = params[1];

                int measure = std::stoi(time.substr(1, 3));
                int rawchan = std::stoi(time.substr(4, 2));
                int tempo   = note.size() / 2;

                ChannelType channel = ChannelType::SampleNote;
                NoteType type       = NoteType::Auto;

                const int bmsChanIndex = 10;
                const int bmsLnChanIndex = 50;

                int channels[7] = { 1, 2, 3, 4, 5, 8, 9 };
                bool ln = false;
                for (int c = 0; c < sizeof(channels); c++)
                {
                    int ch = channels[c];
                    if (rawchan == ch + bmsChanIndex || rawchan == ch + bmsLnChanIndex)
                    {
                        channel = (ChannelType)(c + 2);
                        type    = NoteType::Normal;
                        ln      = rawchan == ch + bmsLnChanIndex;

                        break;
                    }
                }

                auto block = EventBlock
                {
                    measure,
                    channel,
                    tempo
                };

                for (int i = 0; i < note.size(); i += 2)
                {
                    auto sampleId = note.substr(i, 2);
                    short id = sampleId == "00" ? 0 : chart.samples[sampleId];
                    if (id == 0)
                        continue;

                    if (ln)
                    {
                        holdStates[(int)channel] = !holdStates[(int)channel];
                        type = holdStates[(int)channel] ? NoteType::HoldStart : NoteType::HoldEnd;
                    }

                    auto ev    = Event();
                    ev.id      = id;
                    ev.type    = type;
                    ev.channel = channel;
                    ev.tempo   = note.size() / 2;
                    ev.measure = measure;
                    ev.beat    = (int)((i / 2) * (192.f / ev.tempo));
                    ev.cell    = i / 2;

                    block.events.push_back(ev);
                }

                if (!block.events.empty())
                {
                    for (int l = 0; l < 3; l++)
                        chart.blocks[(Difficulty)l].push_back(block);
                }
            }
        }

        return chart;
    }
}