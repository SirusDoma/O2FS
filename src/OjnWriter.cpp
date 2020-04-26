#include <OjnWriter.hpp>
#include <Chart.hpp>
#include <Event.hpp>

#include <sstream>
#include <fstream>

std::vector<char> OjnWriter::Write(Chart chart, std::vector<char> headerData, std::vector<char> coverData)
{
    // TODO: Support for multiple difficulty
    std::vector<char> output;
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    stream.write(headerData.data(), headerData.size());

    for (int diff = 0; diff < 3; diff++)
    {
        auto packages = chart.GetPackages();
        int startOffset = stream.tellp();
        int pkgCount = packages.size();
        int noteCount = 0;
        int playableNoteCount = 0;

        bool hold[10] = { false, false, false, false, false, false, false, false, false, false };
        for (auto pkg : packages)
        {
            short count = pkg.events.size();
            stream.write(reinterpret_cast<char*>(&pkg.measure), sizeof(pkg.measure));
            stream.write(reinterpret_cast<char*>(&pkg.channel), sizeof(pkg.channel));
            stream.write(reinterpret_cast<char*>(&count), sizeof(count));

            for (auto ev : pkg.events)
            {
                // TODO: Verify the flags
                char flag = 0;
                if (ev.sampleID > 0)
                {
                    if (pkg.channel < 9)
                    {
                        playableNoteCount++;
                        if (ev.isLongNote)
                        {
                            // 2 = start ln | 3 = end ln
                            flag = hold[pkg.channel] ? 3 : 2;
                            hold[pkg.channel] = !hold[pkg.channel];
                        }
                        else
                            flag = 0;
                    }
                    else
                        flag = 4;

                    noteCount++;
                }

                int aux = 0;
                stream.write(reinterpret_cast<char*>(&ev.sampleID), sizeof(ev.sampleID));
                stream.write(reinterpret_cast<char*>(&aux), 1); // vol and pan, ignore this
                stream.write(reinterpret_cast<char*>(&flag), sizeof(flag));
            }
        }

        // TODO: Ugh, should have map the offsets instead, also missing measure count
        if (diff == 0)
        {
            stream.seekp(28, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&noteCount), sizeof(noteCount));

            stream.seekp(40, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&playableNoteCount), sizeof(playableNoteCount));

            stream.seekp(64, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&pkgCount), sizeof(pkgCount));

            stream.seekp(284, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&startOffset), sizeof(startOffset));
        }
        else if (diff == 1)
        {
            stream.seekp(32, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&noteCount), sizeof(noteCount));

            stream.seekp(44, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&playableNoteCount), sizeof(playableNoteCount));

            stream.seekp(68, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&pkgCount), sizeof(pkgCount));

            stream.seekp(288, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&startOffset), sizeof(startOffset));
        }
        else if (diff == 2)
        {
            stream.seekp(36, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&noteCount), sizeof(noteCount));

            stream.seekp(48, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&playableNoteCount), sizeof(playableNoteCount));

            stream.seekp(72, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&pkgCount), sizeof(pkgCount));

            stream.seekp(292, std::ios::beg);
            stream.write(reinterpret_cast<char*>(&startOffset), sizeof(startOffset));
        }

        stream.seekp(0, std::ios::end);
    }

    // Determine image start offset
    int imgOff = stream.tellp();
    stream.seekp(296, std::ios::beg);
    stream.write(reinterpret_cast<char*>(&imgOff), sizeof(imgOff));

    // Write image data
    stream.seekp(0, std::ios::end);
    stream.write(coverData.data(), coverData.size());
    output = std::vector<char>(stream.tellp());

    // Dump them into vector
    stream.seekp(0, std::ios::beg);
    stream.read(output.data(), output.size());
    stream.clear();

    return output;
}
