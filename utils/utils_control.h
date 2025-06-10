#pragma once

#include <vector>
#include <chrono>
#include <optional>
#include <cstddef>


class VariableTrace {
public:
    VariableTrace() : _prev_time(std::chrono::steady_clock::now()) {}

    void update(double value, std::optional<double> dt_override = std::nullopt) {
        auto now = std::chrono::steady_clock::now();
        double dt = dt_override.has_value()
                        ? dt_override.value()
                        : std::chrono::duration<double>(now - _prev_time).count();

        if (dt < 1e-9) dt = 1e-3;

        _derivative = _has_prev ? (value - _prev_value) / dt : 0.0;
        _integral  += _has_prev ? 0.5 * (value + _prev_value) * dt : 0.0;

        _prev_value = value;
        _prev_time  = now;
        _has_prev   = true;

        _values.push_back(value);
    }

    double value() const        { return _prev_value; }
    double derivative() const   { return _derivative; }
    double integral() const     { return _integral; }
    double v() const        { return _prev_value; }
    double d() const   { return _derivative; }
    double i() const     { return _integral; }

    void reset_integral(double init = 0.0) { _integral = init; }

    const std::vector<double>& history() const { return _values; }

    // k = 0 es el más reciente, k = 1 anterior, etc.
    double at(size_t k, double fallback = 0.0) const {
        if (k >= _values.size()) return fallback;
        return _values[_values.size() - 1 - k];
    }

    size_t size() const { return _values.size(); }

private:
    std::vector<double> _values;
    double _prev_value     = 0.0;
    double _derivative     = 0.0;
    double _integral       = 0.0;
    bool _has_prev         = false;
    std::chrono::steady_clock::time_point _prev_time;
};
