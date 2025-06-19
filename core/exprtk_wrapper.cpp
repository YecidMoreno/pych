#include "exprtk_wrapper.h"
#include <exprtk.hpp>

struct ExprtkParser::Impl {
    exprtk::symbol_table<double> symbol_table;
    exprtk::expression<double> expression;
    exprtk::parser<double> parser;

    bool compile(const std::string& expr) {
        expression.register_symbol_table(symbol_table);
        symbol_table.add_constant("M_PI", 3.141592653589793);

        return parser.compile(expr, expression);
    }

    std::string error(){
        return parser.error();
    }

    double eval() const {
        return expression.value();
    }

    void add_variable(const std::string& name, double* ptr) {
        symbol_table.add_variable(name, *ptr);
    }
};

ExprtkParser::ExprtkParser() : impl(new Impl) {}

ExprtkParser::~ExprtkParser() {
    delete impl;
}

bool ExprtkParser::setExpression(const std::string& expr) {
    return impl->compile(expr);
}

std::string ExprtkParser::error() {
    return impl->error();
}

void ExprtkParser::setVariable(const std::string& name, double* reference) {
    impl->add_variable(name, reference);
}

double ExprtkParser::evaluate() const {
    return impl->eval();
}
