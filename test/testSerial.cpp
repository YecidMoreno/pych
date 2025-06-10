#include <iostream>
#include <core/core.h>

#include <unistd.h>

#include "commIO/CANOpenIO/CANOpenIO_defs.h"
#include "deviceIO/EPOS/EPOS_defs.h"

#include <thread>
using namespace std::chrono_literals;

using namespace HH;

int main()
{

    auto &core = Core::instance();

    core.load_json_project("/home/inception/git/commIO/test/testSerial.json");
    core.pm_commIO.printPlugins();
    core.connect_all_devices();

    auto serial0 = core.pm_commIO.get_node("serial0");
    auto enc0 = core.pm_deviceIO.get_node("enc0");

    float value;
    int offset = 1;
    for (int t = 0; t < 80; t++)
    {
        enc0->read(&value,0);
        hh_logn("value: %f",value);
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}