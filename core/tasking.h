#pragma once

#include <chrono>
#include <string>
#include <regex>
#include <stdexcept>

#include <thread>
#include "logger.h"
#include "json_api.h"

using namespace std::chrono_literals;
using namespace std::chrono;


struct task_struct_t
{
    std::function<void()> _callback = []()
    { hh_logw("void Callback"); };
    std::chrono::nanoseconds _ts = std::chrono::nanoseconds(0);
    std::chrono::nanoseconds _wakeup = std::chrono::nanoseconds(0);
    std::string _name = "No named task";
    std::string _thread_name = "No thread named task";
    std::chrono::_V2::steady_clock::time_point _next_time;
};

struct task_container_t
{
    std::thread _thr;
    std::vector<task_struct_t> task;
    std::function<void(std::vector<task_struct_t> &)> _loop;
};



void _taks_main_h(std::vector<task_struct_t> &h_tasks);

__attribute__((visibility("default")))
bool load_task_from_json(std::string j_str, task_struct_t &_task_h);