#include <iostream>
#include <string.h>
#include <unistd.h>
#include <linux/can.h>

#include "plugin_loader.h"
#include "core/json_api.h"

#include "src/plugins/commIO/CANOpenIO/CANOpenIO_defs.h"
#include "commIO/commIO_interface.h"

#include "deviceIO/deviceIO_interface.h"

#include "src/plugins/deviceIO/EPOS/EPOS_defs.h"

using namespace jsonapi;

int main()
{

    json_obj j = json_obj::from_file("/home/inception/git/commIO/robot.json");
    if (!j.valid())
        return 1;

    json_obj j_robot;
    json_obj j_device;

    json_obj j_epos5;

    if (!j.get("robot", &j_robot))
    {
        printf("Key NOT found: robot\n");
        return -1;
    }

    if (!j_robot.get("deviceIO", &j_device))
    {
        printf("Key NOT found: deviceIO\n");
        return -1;
    }
    if (!j_device.get("epos5", &j_epos5))
    {
        printf("Key NOT found: epos5\n");
        return -1;
    }

    PluginManager<CommIO_plugin> pm_commIO;
    PluginManager<DeviceIO_plugin> pm_deviceIO;

    json_obj j_plugins, j_p_commIO, j_p_deviceIO;
    if (!j.get("plugins", &j_plugins) &
        !j_plugins.get("commIO", &j_p_commIO) &
        !j_plugins.get("deviceIO", &j_p_deviceIO))
    {
        printf("Key NOT found: plugins\n");
        return -1;
    }

    for (const auto &key : j_p_commIO.keys())
    {
        std::string value;
        if (!j_p_commIO.get(key, &value))
        {
            continue;
        }
        printf("commIO: %s -> %s\n", key.c_str(), value.c_str());
        pm_commIO.register_plugin(key, value);
    }

    for (const auto &key : j_p_deviceIO.keys())
    {
        std::string value;
        if (!j_p_deviceIO.get(key, &value))
        {
            continue;
        }
        printf("deviceIO: %s -> %s\n", key.c_str(), value.c_str());
        pm_deviceIO.register_plugin(key, value);
    }

    json_obj j_commIO;
    json_obj j_can0;

    if (!j.get("commIO", &j_commIO))
    {
        printf("Key NOT found: commIO\n");
        return -1;
    }

    for (const auto &key : j_commIO.keys())
    {
        json_obj value;
        json_obj value_cfg;
        std::string type;
        if (!j_commIO.get(key, &value))
        {
            continue;
        }

        if (!value.get("config", &value_cfg) & !value.get("type", &type)){
            continue;
        }

        printf("deviceIO: %s -> %s\n", key.c_str(), type.c_str());
        pm_commIO.create_node(key, type);
        pm_commIO.get_node(key)->config(value.get_object_str("config"));
    }

  
    auto can0 = pm_commIO.get_node("can0");
    if (!can0->connect(""))
    {
        printf("CAN offline\n");
        return -1;
    }

    pm_deviceIO.create_node("epos5", "EPOS");
    auto epos5 = pm_deviceIO.get_node("epos5");

    json_obj j_epos5_config;
    if (!j_epos5.get("config", &j_epos5_config))
    {
        printf("Key NOT found: config j_epos5\n");
        return -1;
    }

    j_epos5_config.set("ptr_commIO", reinterpret_cast<uintptr_t>(can0));
    epos5->config(j_epos5_config.str());

    epos5->command(EPOS_CMD::FAULT_RESET);
    epos5->command(EPOS_CMD::SET_VELOCITY_MODE);
    epos5->command(EPOS_CMD::ENABLE_MOTORS);

    usleep(1000000);

    epos5->command(EPOS_CMD::FAULT_RESET);

    return 0;
}