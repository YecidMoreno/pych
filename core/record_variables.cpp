#include "record_variables.h"

#include <fstream>
#include <cstdio>

void RecordVariable::append(double value)
{
    hist.push_back(value);
}

void RecordVariable::append()
{
    if (_ptr)
    {
        hist.push_back(*_ptr);
        return;
    }
    
    if (fnc)
    {
        hist.push_back(fnc());
        return;
    }

    hist.push_back(-99);
}

void RecordVariables::set_file_name(std::string _file_name)
{
    file_name = _file_name;
};

void RecordVariables::clear()
{
    vars.clear();
}

void RecordVariables::loop()
{
    for (auto &v : vars)
    {
        v.append();
    }
}

void RecordVariables::save() const
{
    std::printf("Saving log in: %s\n", file_name.c_str());

    std::ofstream out(file_name);
    if (!out)
        return;

    for (size_t i = 0; i < vars.size(); ++i)
    {
        out << vars[i].name;
        if (i < vars.size() - 1)
            out << "\t";
    }
    out << "\n";

    size_t max_len = 0;
    for (const auto &v : vars)
        if (v.hist.size() > max_len)
            max_len = v.hist.size();

    for (size_t i = 0; i < max_len; ++i)
    {
        for (size_t j = 0; j < vars.size(); ++j)
        {
            if (i < vars[j].hist.size())
            {
                char buffer[64];
                std::snprintf(buffer, sizeof(buffer), vars[j].format.c_str(), vars[j].hist[i]);
                out << buffer;
            }
            if (j < vars.size() - 1)
                out << "\t";
        }
        out << "\n";
    }
}
