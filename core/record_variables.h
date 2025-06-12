#pragma once

#include <fstream>
#include <sstream>
#include <vector>
#include <functional>

struct RecordVariable
{
    std::vector<double> hist;
    std::string name = "NULL";
    std::string format = "%e";
    double *_ptr = nullptr;
    std::function<double()> fnc = nullptr;

    void append(double value);
    void append();
};

class RecordVariables
{
private:
    std::string file_name;

public:
    std::vector<RecordVariable> vars;
    RecordVariables(){}
    ~RecordVariables() { save(); }

    void set_file_name(std::string _file_name);

    void loop();
    void save() const;
    void clear();
};