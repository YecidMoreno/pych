#pragma once

#include <chrono>

class __attribute__((visibility("default"))) CoreRunner
{
private:
    std::chrono::steady_clock::time_point run_time_0;

public:
    CoreRunner();
    ~CoreRunner();

    bool connect_all_devices();
    bool disconnect_all_devices();

    std::chrono::nanoseconds get_run_time();
    double get_run_time_double(std::chrono::nanoseconds units);
    
    std::chrono::steady_clock::time_point get_run_time_0();

    void start();
};
