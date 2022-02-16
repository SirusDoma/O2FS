#include <O2FS/Systems/O2MusicSystem.hpp>

#include <O2FS/OTwo.hpp>
#include <O2FS/IO/BeMusicReader.hpp>
#include <O2FS/IO/O2MusicWriter.hpp>
#include <Logger.hpp>

namespace O2FS
{
    const std::string OJN_SIGNATURE = "ojn\0";

    O2MusicSystem::O2MusicSystem() :
        handles(),
        cache()
    {
    }

    O2MusicSystem::~O2MusicSystem()
    {
        handles.clear();
        cache.clear();
    }

    bool O2MusicSystem::Check(HANDLE hFile) const
    {
        /** Check()
         * This system should recognize OJN file which hold music metadata and notechart.
         * Additionally, it contains music cover and thumbnail as well.
         *
         * TODO: Add OJNList.dat and OJM exstension recognition.
         */

        // Prevents handling file with locked HANDLE
        // This will prevent infinite recursive calls when calling ReadFile inside the hook.
        auto fileName = OTwo::GetFileName(hFile);
        if (handles.find(fileName) != handles.end())
            return false;

        // Using OTwo::GetFileName will provide us caching to avoid slow and repetitive query filename from handle
        return fileName.find(".ojn") != std::string::npos;
    }

    BOOL O2MusicSystem::ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
    {
        /** ReadFile()
         * This function is guaranteed to be called with file type within system interest, which is OJN file.
         * The algorithm should work as follows:
         * - Check whether hFile is valid OJN file or not
         * - Pass it through to original ReadFile if its a valid OJN file
         * - Otherwise, use BMS Reader to parse it into chart
         * - Serialize it into OJN and cache it if needed to avoid redundant conversion
         * - Fill the lpBuffer and lpNumberOfBytesRead with serialized OJN from BMS
         *   Make sure to use corresponding offset that has been set by the game via SetFilePointerEx
         *   (e.g The game may use ReadFile for partial loading: displaying music cover image)
         *
         * TODO:
         *  - Add OJNList.dat processing support.
         *  - Add OJM Processing support.
         *  - Add extension support for other music formats than BMS.
         */

        // Check whether handle is locked or not
        // This will prevent infinite recursive calls when calling ReadFile inside the hook.
        auto fileName = OTwo::GetFileName(hFile);
        if (handles.find(fileName) != handles.end())
            return OTwo::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

        // Lock the file, any ReadFile API will passthrough into real function
        handles.insert(fileName);

        // Prepare the buffer and pinpoint the offset
        char* buffer = (char*)lpBuffer;
        int offset   = 0;

        // Retrieve reading offset of where the game tries to read
        auto position = LARGE_INTEGER { 0 };
        if (SetFilePointerEx(hFile, LARGE_INTEGER{ 0 }, &position, FILE_CURRENT))
            offset = position.QuadPart;

        Logger::Write("O2FS: [ReadFile] Processing %s (@%d - %d bytes).\n",fileName.c_str(), offset, nNumberOfBytesToRead);

        // File is a valid OJN file, skip processing and let it passthrough original API call.
        if (IsOJN(fileName))
        {
            // Release the HANDLE lock so the HANDLE can be hooked again after this call.
            handles.erase(fileName);
            return OTwo::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
        }

        // File is not OJN, assuming its a BMS file.
        // Convert the BMS into OJN file.
        auto data = LoadChart(hFile);

        // Calculate the number of bytes to read to avoid out of range error
        int count = nNumberOfBytesToRead;
        if (offset + count > data.size())
            count = data.size() - offset;

        // Fill the buffer with converted OJN data at requested offset
        memcpy(&buffer[0], &data[offset], count);
        *lpNumberOfBytesRead = count;

        // Release the HANDLE lock so the HANDLE can be hooked again after this call.
        handles.erase(fileName);
        return TRUE;
    }

    DWORD O2MusicSystem::GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
    {
        /** GetFileSize()
         * This function will be first called when the game load the OJN file to determine the size of OJN file.
         * Therefore, this function need to return OJN file size instead of BMS file size.
         *
         * There's no need to maintain HANDLE locking in this function
         * (GetFileSize is unlikely called when deserialize/serialize chart data)
         */

        // File is a valid OJN file, skip processing and let it passthrough original API call.
        auto fileName = OTwo::GetFileName(hFile);
        if (IsOJN(fileName))
            return OTwo::GetFileSize(hFile, lpFileSizeHigh);

        // Parse and Convert the chart and find out the size of the raw data
        auto data = LoadChart(hFile);
        DWORD result = data.size();

        Logger::Write("O2FS: [GetFileSize] Processing %s. (%d)\n", fileName.c_str(), result);
        return result;
    }

    bool O2MusicSystem::IsOJN(std::string fileName)
    {
        char sign[4];
        auto stream  = std::ifstream(fileName, std::ios::binary);\
        int position = stream.tellg();

        stream.seekg(4, std::ios::beg);
        stream.read(&sign[0], 4);
        stream.seekg(position, std::ios::beg);
        stream.close();

        return sign == OJN_SIGNATURE;
    }

    std::vector<char> O2MusicSystem::LoadChart(HANDLE hFile)
    {
        // Check for the data inside the cache, avoid repetitive conversion
        auto data = std::vector<char>();
        if (auto it = cache.find(hFile); it == cache.end())
        {
            // Cache not found, Convert the chart from BMS to OJN
            auto fileName = OTwo::GetFileName(hFile);
            auto chart    = BeMusicReader::Deserialize(fileName);

            // Re-check the cache, because BeMusicReader::Deserialize use ReadFile API call which also call LoadChart
            // Otherwise, BeMusicReader::Deserialize may receive OJN instead of BMS file as the input
            if (it = cache.find(hFile); it == cache.end())
            {
                // Genuinely first attempt to load chart, cache the OJN data
                data = O2MusicWriter::Serialize(chart);
                cache[hFile] = data;
            }
            else
                data = it->second;
        }
        else
        {
            // Cache found, load OJN data from cache
            data = it->second;
        }

        return data;
    }

    BOOL O2MusicSystem::CloseHandle(HANDLE hObject)
    {
        // Release lock handles and invalidate chart data from caches
        handles.erase(OTwo::GetFileName(hObject));
        cache.erase(hObject);

        return FileSystem::CloseHandle(hObject);
    }
}