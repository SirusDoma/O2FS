#ifndef CHART_OJN_WRITER_HPP
#define CHART_OJN_WRITER_HPP

#include <vector>

class Chart;
class OjnWriter
{
public:
    static std::vector<char> Write(Chart chart, std::vector<char> headerData, std::vector<char> coverData);
};

#endif