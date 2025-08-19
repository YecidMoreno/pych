#include "CoreScheduler.h"

#include <core/core.h>

using namespace HH;
using namespace jsonapi;

CoreScheduler::CoreScheduler(/* args */)
{
}

CoreScheduler::~CoreScheduler()
{
}

bool CoreScheduler::add_to_task(std::string _task_name, task_struct_t _t)
{
    if (_tasks_handles.find(_task_name) == _tasks_handles.end())
    {
        task_container_t _c;
        _c.task.clear();
        _c._loop = _taks_main_h;
        _c._thr = std::thread();
        _tasks_handles.emplace(_task_name, std::move(_c));
    }

    if (_tasks_handles.find(_task_name) != _tasks_handles.end())
    {
        _tasks_handles[_task_name].task.push_back(std::move(_t));
        return true;
    }
    else
    {
        return false;
    }
    return true;
}

bool CoreScheduler::run()
{
    auto &core = Core::instance();

    core.runner.start();

    for (auto &[name, val] : this->_tasks_handles)
    {

        val._thr = std::thread([&val]()
                               { val._loop(val.task); });
    }
    return true;
}

bool CoreScheduler::stop()
{
    for (auto &[name, val] : this->_tasks_handles)
    {

        if (val._thr.joinable())
        {
            val._thr.join();
        }
    }
    return true;
}


bool CoreScheduler::run_for_time(std::chrono::nanoseconds _time)
{
    auto &core = Core::instance();

    this->run();

    auto t0 = steady_clock::now();

    while (core.get_state() == HH::AppState::RUNNING)
    {
        if (steady_clock::now() - t0 > _time)
        {
            return true;
            break;
        }

        std::this_thread::sleep_for(500ms);
    }

    return false;
}