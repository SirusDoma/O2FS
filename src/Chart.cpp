#include <Chart.hpp>
#include <Event.hpp>

Chart::Chart() :
    packages(),
    keysounds()
{
}

void Chart::AddPackage(Package pkg)
{
    packages.push_back(pkg);
}

void Chart::AddKeysound(std::string name)
{
    keysounds[name] = keysounds.size() + 1;
}

std::vector<Package> Chart::GetPackages()
{
    return packages;
}

int Chart::GetKeysound(std::string name)
{
    return keysounds[name];
}