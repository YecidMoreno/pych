#pragma once

#include <string>
#include <unordered_map>

class ExprtkParser {
public:
    ExprtkParser();
    ~ExprtkParser();

    bool setExpression(const std::string& expr);
    void setVariable(const std::string& name, double* reference);
    double evaluate() const;

private:
    struct Impl;
    Impl* impl;  // PImpl idiom (oculta exprtk.hpp y reduce recompilaciones)
};
