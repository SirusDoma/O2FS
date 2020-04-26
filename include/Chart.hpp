#ifndef CHART_HEADER_HPP
#define CHART_HEADER_HPP

#include <vector>
#include <map>
#include <string>

// TODO: Lot of missing stuffs, add header properties like title, bpm, artist etc
class Package;
class Chart
{
private:
    std::vector<Package> packages;
    std::map<std::string, int> keysounds;

public:
    Chart();

    void AddPackage(Package pkg);
    void AddKeysound(std::string name);

    std::vector<Package> GetPackages();
    int GetKeysound(std::string name);
};

#endif