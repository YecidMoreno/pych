
#include <chrono>
#include <string>
#include <regex>
#include <stdexcept>

#include <thread>

#include <core/tasking.h>
#include <core/logger.h>
#include <core/core.h>
#include <core/json_api.h>

using namespace std::chrono_literals;
using namespace std::chrono;

bool load_task_from_json(std::string j_str, task_struct_t &_task_h)
{
    using namespace jsonapi;

    bool IsOK = false;
    json_obj _j = json_obj::from_string(j_str);

    IsOK = (_j.get("ts", &_task_h._ts) &&
            _j.get("thread", &_task_h._thread_name));

    if(IsOK){
        _j.get("wakeup", &_task_h._wakeup);
        _j.get("name", &_task_h._name);
    }

    return IsOK;
}

void _taks_main_h(std::vector<task_struct_t> &h_tasks)
{
    using namespace std::chrono_literals;
    using namespace std::chrono;

    auto &core = HH::Core::instance();
    // auto now = steady_clock::now();
    auto now = core.run_time_0;

    auto t0 = steady_clock::now();
    auto t1 = steady_clock::now();

    for (auto &h : h_tasks)
    {
        h._next_time = now + h._wakeup;
    }

    while (core.get_state() == HH::AppState::RUNNING)
    {
        now = steady_clock::now();

        for (size_t i = 0; i < h_tasks.size(); ++i)
        {
            if (now >= h_tasks[i]._next_time)
            {
                t0 = steady_clock::now();
                h_tasks[i]._callback();
                h_tasks[i]._next_time += h_tasks[i]._ts;
                t1 = steady_clock::now();

                if ((t1 - t0) > h_tasks[i]._ts*.75)
                {
                    hh_logw("Esta tarea está tardando mas de no necesario: %s",h_tasks[i]._name.c_str());
                }
            }
        }

        std::this_thread::sleep_for(100us);
    }
}
