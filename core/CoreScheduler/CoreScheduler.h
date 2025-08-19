#pragma once
#include <map>
#include <unordered_map>
#include <string>
#include "tasking.h"

class __attribute__((visibility("default"))) CoreScheduler
{
private:
    std::unordered_map<std::string, task_container_t> _tasks_handles;
public:
    CoreScheduler();
    ~CoreScheduler();

    bool add_to_task(std::string _task_name, task_struct_t _t);
    
    bool run_for_time(std::chrono::nanoseconds _time);
    bool run();

    bool stop();   
};