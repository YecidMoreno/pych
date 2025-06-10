#include <iostream>
#include <string.h>
#include <unistd.h>

#include <linux/can.h>

#include "plugin_loader.h"
#include "core/json_api.h"

#include "src/plugins/commIO/CANOpenIO/CANOpenIO_defs.h"
#include "commIO/commIO_interface.h"

using namespace jsonapi;

int main()
{
    printf("testCanOpenIO\n");

    PluginManager<CommIO_plugin> pm;
    pm.register_plugin("CANOpen", "/home/inception/git/commIO/plugins/commIO/CANOpenIO.so");
    pm.create_node("can0", "CANOpen");

    auto can0 = pm.get_node("can0");

    json_obj j = json_obj::from_file("/home/inception/git/commIO/test/json.json");
    if (!j.valid())
        return 1;

    json_obj cfg;
    if (j.get_object("testCANOpenIO", &cfg) == 0)
    {
        printf("%s\n", cfg.str().c_str());
    }

    int id;

    if (j.get_int("id", &id) == 0)
        std::cout << "id = " << id << "\n";

    if (can0->config(cfg.str()))
    {
        printf("Config is VALID\n");
    }
    else
    {
        printf("Config is NOT valid\n");
    }

    if (can0->connect(""))
    {
        printf("CAN online\n");

        // struct can_frame sync_frame{.can_id = 0x080, .can_dlc = 0};
        // can0->send(static_cast<void *>(&sync_frame), sizeof(can_frame));

        can0->send(nullptr, 0, &canopen_cmd_SYNC);

        usleep(100000);

        arg_CANOpen_receive arg{.NodeID = 5, .TPDO_Index = 1};
        struct can_frame res;

        int r0;

        r0 = can0->receive(static_cast<void *>(&res), 0, static_cast<void *>(&arg));
        printf("TPDO_1 res: %d\n", r0);

        r0 = can0->receive(static_cast<void *>(&res), 0, static_cast<void *>(&arg));
        printf("TPDO_1 res: %d\n", r0);

        r0 = can0->receive(static_cast<void *>(&res), 0, static_cast<void *>(&arg));
        printf("TPDO_1 res: %d\n", r0);

        can0->send(nullptr, 0, &canopen_cmd_SYNC);
        usleep(100000);

        r0 = can0->receive(static_cast<void *>(&res), 0, static_cast<void *>(&arg));
        printf("TPDO_1 res: %d\n", r0);
    }
    else
    {
        printf("CAN offline\n");
    }

    usleep(4000000);

    return 0;
}