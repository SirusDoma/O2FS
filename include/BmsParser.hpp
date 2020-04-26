#ifndef CHART_BMS_PARSER_HPP
#define CHART_BMS_PARSER_HPP

#include <fstream>
#include <vector>
#include <string>

class Chart;
class BmsParser
{
private:
    static std::vector<std::string> SplitString(const std::string& s, char delim);

public:
    static Chart Parse(std::string fileName);
};

#endif