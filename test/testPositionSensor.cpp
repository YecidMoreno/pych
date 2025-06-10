#include <iostream>
#include <string.h>
#include <unistd.h>

#include <linux/can.h>

#include "plugin_loader.h"
#include "core/json_api.h"

// #include "src/plugins/commIO/CANOpenIO/CANOpenIO_defs.h"
#include "commIO/commIO_interface.h"

#include "src/plugins/commIO/CANOpenIO/CANOpenIO_defs.h"
#include "deviceIO/deviceIO_interface.h"

using namespace jsonapi;

int main()
{

    json_obj j = json_obj::from_file("/home/inception/git/commIO/robot.json");
    if (!j.valid())
        return 1;

    json_obj j_robot;
    json_obj j_comm;
    json_obj j_sensors;
    if (!j.get("robot", &j_robot))
    {
        printf("Key NOT found: robot\n");
        return -1;
    }

    if (!j.get("commIO", &j_comm))
    {
        printf("Key NOT found: commIO\n");
        return -1;
    }

    json_obj j_can0;
    if (!j_comm.get("can0", &j_can0))
    {
        printf("Key NOT found: can0\n");
        return -1;
    }

    PluginManager<CommIO_plugin> pm;
    pm.register_plugin("CANOpen", "/home/inception/git/commIO/plugins/commIO/CANOpenIO.so");
    pm.create_node("can0", "CANOpen");
    auto can0 = pm.get_node("can0");
    can0->config(j_can0.get_object_str("config"));

    for (const auto &k_comm : j_comm.keys())
    {
        printf("j_comm.keys(): %s\n", k_comm.c_str());
    }
    if (!can0->connect(""))
    {
        printf("CAN offline\n");
        return -1;
    }

    if (!j_robot.get("sensors", &j_sensors))
    {
        printf("Key NOT found: sensors\n");
        return -1;
    }

    for (const auto &k_comm : j_sensors.keys())
    {
        printf("j_sensors.keys(): %s\n", k_comm.c_str());
    }

    json_obj j_enc0;
    if (!j_sensors.get("enc0", &j_enc0))
    {
        printf("Key NOT found: enc0\n");
        return -1;
    }

    PluginManager<DeviceIO_plugin> pmd;
    pmd.register_plugin("PositionSensor", "/home/inception/git/commIO/plugins/deviceIO/PositionSensor.so");
    pmd.create_node("enc0", "PositionSensor");
    auto enc0 = pmd.get_node("enc0");

    json_obj j_enc0_config;
    if (!j_enc0.get("config", &j_enc0_config))
    {
        printf("Key NOT found: config enc0\n");
        return -1;
    }

    j_enc0_config.set("ptr_commIO", reinterpret_cast<uintptr_t>(can0));
    printf("com_aux send %lx\n", reinterpret_cast<uintptr_t>(can0));

    if (!enc0->config(j_enc0_config.str()))
    {
        printf("Configuracion invalida del encoder \n");
        return -1;
    }

    usleep(500000);
    int32_t pos;

    uint32_t count = 0;
    for (int i = 0; i < 1000*5; i++)
    {        
        count += enc0->read(&pos, 0)==0;

        printf("pos: %d     \n", pos);
        can0->send(nullptr, 0, &canopen_cmd_SYNC);
        usleep(1000);
    }

    printf("count: %d\n",count);

    return 0;
}