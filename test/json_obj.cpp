#include "core/json_api.h"
#include <iostream>
using namespace jsonapi;

int main()
{

    json_obj j = json_obj::from_file("/home/inception/git/commIO/robot.json");
    if (!j.valid())
        return 1;

    json_obj j_robot;
    if (j.get("robot", &j_robot))
    {
        j_robot.set("aa",44);
        std::string name;
        if (j_robot.get("name", &name))
        {
            printf("robot name: %s\n", name.c_str());
            printf("json: %s\n",j_robot.str().c_str());
        }
        
    }

    return 0;
}