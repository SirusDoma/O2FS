#include <BmsParser.hpp>
#include <Chart.hpp>
#include <Event.hpp>

#include <sstream>

Chart BmsParser::Parse(std::string fileName)
{
    // TODO: Very incomplete parser, ignore lot essential stuffs to o2jam header like bpm and such
    // Chart class will need to provide storage for those essential properties

    auto chart = Chart();
    std::ifstream stream(fileName);

    for (std::string line; std::getline(stream, line); )
    {
        if (line.size() == 0)
            continue;

        // Parse property
        auto params = SplitString(line, ' ');
        if (params.size() == 2)
        {
            std::string prop = params[0];
            std::string value = params[1];

            // Keysound data
            if (prop.substr(0, 4) == "#WAV")
                chart.AddKeysound(prop.substr(4, 2));
            /*else if (params.size() >= 2)
                std::printf("BMS: %s: %s", prop, value);*/
        }

        // Note data, mapping probably stupid
        params = SplitString(line, ':');
        auto package = Package();
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

            package.measure = std::stoi(time.substr(1, 3));
            package.events  = std::vector<Event>();

            auto ch = time.substr(4, 2);
            bool ln = false;
            if (ch == "11" || ch == "51")
            {
                package.channel = 2; // S
                ln = ch == "51";
            }
            else if (ch == "12" || ch == "52")
            {
                package.channel = 3; // D
                ln = ch == "52";
            }
            else if (ch == "13" || ch == "53")
            {
                package.channel = 4; // F
                ln = ch == "53";
            }
            else if (ch == "14" || ch == "54")
            {
                package.channel = 5; // Space
                ln = ch == "54";
            }
            else if (ch == "15" || ch == "55")
            {
                package.channel = 6; // J
                ln = ch == "55";
            }
            else if (ch == "18" || ch == "58")
            {
                package.channel = 7; // K
                ln = ch == "58";
            }
            else if (ch == "19" || ch == "59")
            {
                package.channel = 8; // L
                ln = ch == "59";
            }
            else
            {
                package.channel = 9;
                ln = false;
            }

            int pts = 0;
            for (int i = 0; i < note.size(); i += 2)
            {
                auto sample = note.substr(i, 2);
                int id = sample == "00" ? 0 : chart.GetKeysound(sample);

                auto ev = Event();
                ev.sampleID   = id;
                ev.isLongNote = ln;
                ev.offset     = i / 2;

                pts += id;
                package.events.push_back(ev);
            }

            if (pts > 0)
                chart.AddPackage(package);
        }
    }

    stream.close();
    return chart;
}

// Yea, i need this function to split string in C++, pain in the ass
std::vector<std::string> BmsParser::SplitString(const std::string& s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}