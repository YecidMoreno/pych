#include <core/logger.h>
#include <iostream>
#include <core/core.h>

#include <unistd.h>

#include "commIO/CANOpenIO/CANOpenIO_defs.h"
#include "deviceIO/EPOS/EPOS_defs.h"

#include <thread>
using namespace std::chrono_literals;
using namespace std::chrono;

using namespace HH;
#include <cxxopts.hpp>
#include <linux/can.h>
/*
    auto can0 = core.pm_commIO.get_node("can0");
    auto epos5 = core.pm_deviceIO.get_node("epos5");
    auto enc0 = core.pm_deviceIO.get_node("enc0");
    auto motor0 = core.pm_deviceIO.get_node("motor0");
    auto ctrl0 = core.pm_controlIO.get_node("ctrl0");

    can0->send(nullptr, 0, &canopen_cmd_SYNC);
    std::this_thread::sleep_for(500ms);

    auto can0 = core.pm_commIO.get_node("can0");
    core.add_to_task("thread0", {._callback = [can0]()
                                 { can0->send(nullptr, 0, &canopen_cmd_SYNC); },
                                 ._ts = 1ms,
                                 ._wakeup = 800ns});

*/

std::string file_path = "robot.json";

int main(int argc, char **argv)
{
    cxxopts::Options options("hh_core", "...");
    options.add_options()("f,file", "Archivo de entrada", cxxopts::value<std::string>())("v,verbose", "Modo verboso")("h,help", "Mostrar ayuda");

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("file"))
    {
        file_path = result["file"].as<std::string>();
        hh_logn("loading from arg : %s", file_path.c_str());
    }

    auto &core = Core::instance();

    if (!core.load_json_project(file_path))
    {
        return -1;
    }
    core.pm_commIO.printPlugins();
    core.connect_all_devices();

    // auto can0 = core.pm_commIO.get_node("can0");
    // auto enc = core.pm_deviceIO.get_node("enc");
    // auto mot = core.pm_deviceIO.get_node("mot");

    core.run_tasks();

    auto t0 = steady_clock::now();

    struct can_frame frame;
    
    double val;
    // arg_CANOpen_receive arg_to_read = {.CAN_ID = 0x2901, .lsb_byte = 0, .n_bytes = 4};
    // can0->receive(&val, 0, static_cast<void *>(&arg_to_read));

    int32_t vel = 70;
    while (core.get_state() == HH::AppState::RUNNING)
    {
        if (steady_clock::now() - t0 > 40s)
        {
            break;  
        }

        // enc->read(&val, 0);
        // hh_logi("lbs val %f \n", val);

        // mot->write(&vel,4);

        std::this_thread::sleep_for(100ms);
    }

    core.disconnect_all_devices();

    // core.run_for_time(40s);

    return 0;
}