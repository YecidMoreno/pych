#pragma once

#include <utils/plugin_loader.h>
#include <commIO/commIO_interface.h>
#include <deviceIO/deviceIO_interface.h>
#include <controlIO/controlIO_interface.h>

#include <core/json_api.h>

using namespace jsonapi;

class __attribute__((visibility("default"))) PluginService 
{
public:
    template <typename T>
    T *get_node(const std::string &name);

    template <typename T>
    T *add_node(const std::string &name, const std::string &type);

    template <typename T>
    bool add_node_from_json(const std::string &name, const json_obj &j);

    bool add_nodes_from_json(const json_obj &j);

    template <typename T>
    bool register_plugin(const std::string &name, const std::string &path);

    bool register_all_plugins_from_json(const json_obj &j);
  
    void printStatus();

    template <typename T>
    std::vector<std::string> getNodes();

    template <typename T>
    std::vector<T*> getNodes_T();

private:
    template <typename T>
    bool register_specific_plugins_from_json(const json_obj &j);

    PluginManager<CommIO_plugin> comm;
    PluginManager<DeviceIO_plugin> device;
    PluginManager<ControlIO_plugin> control;
    std::vector<std::string> pluginPath = {""};
};
