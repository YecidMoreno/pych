#include "PluginService.h"
#include <type_traits>

template CommIO_plugin *PluginService::add_node<CommIO_plugin>(const std::string &, const std::string &);
template DeviceIO_plugin *PluginService::add_node<DeviceIO_plugin>(const std::string &, const std::string &);
template ControlIO_plugin *PluginService::add_node<ControlIO_plugin>(const std::string &, const std::string &);

template CommIO_plugin *PluginService::get_node<CommIO_plugin>(const std::string &);
template DeviceIO_plugin *PluginService::get_node<DeviceIO_plugin>(const std::string &);
template ControlIO_plugin *PluginService::get_node<ControlIO_plugin>(const std::string &);

template std::vector<std::string> PluginService::getNodes<CommIO_plugin>();
template std::vector<std::string> PluginService::getNodes<DeviceIO_plugin>();
template std::vector<std::string> PluginService::getNodes<ControlIO_plugin>();


template std::vector<CommIO_plugin*> PluginService::getNodes_T<CommIO_plugin>();
template std::vector<DeviceIO_plugin*> PluginService::getNodes_T<DeviceIO_plugin>();
template std::vector<ControlIO_plugin*> PluginService::getNodes_T<ControlIO_plugin>();


template <typename>
struct always_false : std::false_type
{
};

template <typename T>
bool PluginService::register_plugin(const std::string &name, const std::string &path)
{
    bool res = false;
    int count = 0;
    for (const auto &p : pluginPath)
    {
        if constexpr (std::is_same_v<T, CommIO_plugin>)
        {
            res = comm.register_plugin(name, p + path);
        }
        else if constexpr (std::is_same_v<T, DeviceIO_plugin>)
        {
            res = device.register_plugin(name, p + path);
        }
        else if constexpr (std::is_same_v<T, ControlIO_plugin>)
        {
            res = control.register_plugin(name, p + path);
        }
        else
        {
            return false;
            static_assert(always_false<T>::value, "Tipo de plugin no soportado");
        }

        if (res)
        {
            hh_logi("Plugin [%s] found in: path[%2d]", name.c_str(), count);
            return true;
        }
        count++;
    }
    return res;
}

template <typename T>
bool PluginService::register_specific_plugins_from_json(const json_obj &j)
{
    json_obj j_rec;

    if constexpr (std::is_same_v<T, CommIO_plugin>)
    {
        if (!j.get("commIO", &j_rec))
            return false;
    }
    else if constexpr (std::is_same_v<T, DeviceIO_plugin>)
    {
        if (!j.get("deviceIO", &j_rec))
            return false;
    }
    else if constexpr (std::is_same_v<T, ControlIO_plugin>)
    {
        if (!j.get("controlIO", &j_rec))
            return false;
    }
    else
    {
        static_assert(always_false<T>::value, "Tipo de plugin no soportado");
    }

    for (const auto &key : j_rec.keys())
    {
        std::string value;
        if (j_rec.get(key, &value))
        {
            if (!register_plugin<T>(key, value))
            {
                hh_loge("Error registrando plugin %s", key.c_str());
                return false;
            }
        }
    }

    return true;
}

bool PluginService::register_all_plugins_from_json(const json_obj &j)
{
    json_obj j_p_commIO, j_p_deviceIO, j_p_controlIO;

    if (!j.get("commIO", &j_p_commIO))
    {
        hh_loge("json['plugins']['commIO'] maybe not exist");
        return false;
    }

    if (!j.get("deviceIO", &j_p_deviceIO))
    {
        hh_loge("json['plugins']['deviceIO'] maybe not exist");
        return false;
    }

    if (!j.get("controlIO", &j_p_controlIO))
    {
        hh_loge("json['plugins']['controlIO'] maybe not exist");
        return false;
    }

    if (j.get("path", &pluginPath))
    {
        hh_logn("Plugins path:");
        int count = 0;
        for (const auto &p : pluginPath)
        {
            hh_logn("  [%2d] %s",count++, p.c_str());
        }
    }

    if (!register_specific_plugins_from_json<CommIO_plugin>(j))
        return false;

    if (!register_specific_plugins_from_json<DeviceIO_plugin>(j))
        return false;

    if (!register_specific_plugins_from_json<ControlIO_plugin>(j))
        return false;

    return true;
}

template <typename T>
T *PluginService::get_node(const std::string &name)
{
    if constexpr (std::is_same_v<T, CommIO_plugin>)
    {
        return comm.get_node(name);
    }
    else if constexpr (std::is_same_v<T, DeviceIO_plugin>)
    {
        return device.get_node(name);
    }
    else if constexpr (std::is_same_v<T, ControlIO_plugin>)
    {
        return control.get_node(name);
    }
    else
    {
        return nullptr;
        static_assert(always_false<T>::value, "Tipo de plugin no soportado");
    }
    return nullptr;
}

template <typename T>
T *PluginService::add_node(const std::string &name, const std::string &type)
{
    if constexpr (std::is_same_v<T, CommIO_plugin>)
    {
        comm.create_node(name, type);
        return comm.get_node(name);
    }
    else if constexpr (std::is_same_v<T, DeviceIO_plugin>)
    {
        device.create_node(name, type);
        return device.get_node(name);
    }
    else if constexpr (std::is_same_v<T, ControlIO_plugin>)
    {
        control.create_node(name, type);
        return control.get_node(name);
    }
    else
    {
        return nullptr;
        static_assert(always_false<T>::value, "Tipo de plugin no soportado");
    }
    return nullptr;
}

template <typename T>
bool PluginService::add_node_from_json(const std::string &name, const json_obj &j)
{
    if constexpr (std::is_same_v<T, CommIO_plugin> ||
                  std::is_same_v<T, DeviceIO_plugin> ||
                  std::is_same_v<T, ControlIO_plugin>)
    {
        json_obj value_cfg;
        std::string type;

        if (!j.get("config", &value_cfg) || !j.get("type", &type))
        {
            hh_loge("Faltan campos 'config' o 'type' en el nodo %s", name.c_str());
            return false;
        }

        if (add_node<T>(name, type) == nullptr)
            return false;

        if (!get_node<T>(name)->config(j.get_object_str("config")))
            return false;

        return true;
    }
    else
    {
        static_assert(always_false<T>::value, "Tipo no soportado en add_node_from_json");
        return false;
    }

    return false;
}

bool PluginService::add_nodes_from_json(const json_obj &j)
{

    json_obj j_aux;
    if (j.get("commIO", &j_aux))
    {
        for (const auto &key : j_aux.keys())
        {
            json_obj value;
            if (!j_aux.get(key, &value))
                continue;

            if (!add_node_from_json<CommIO_plugin>(key, value))
                return false;
        }
    }

    if (j.get("deviceIO", &j_aux))
    {
        for (const auto &key : j_aux.keys())
        {
            json_obj value;
            if (!j_aux.get(key, &value))
                continue;

            if (!add_node_from_json<DeviceIO_plugin>(key, value))
                return false;
        }
    }

    if (j.get("controlIO", &j_aux))
    {
        for (const auto &key : j_aux.keys())
        {
            json_obj value;
            if (!j_aux.get(key, &value))
                continue;

            if (!add_node_from_json<ControlIO_plugin>(key, value))
                return false;
        }
    }

    return true;
}

void PluginService::printStatus()
{
    std::string res;

    res += "PluginService::printStatus() \n";
    res += "commIO: ";
    for (const auto &n : this->comm.getNodes())
    {
        res += n + ", ";
    }
    res += "\n";

    res += "deviceIO: ";
    for (const auto &n : this->device.getNodes())
    {
        res += n + ", ";
    }
    res += "\n";

    res += "controlIO: ";
    for (const auto &n : this->control.getNodes())
    {
        res += n + ", ";
    }
    res += "\n";

    this->comm.printPlugins();
    this->device.printPlugins();
    this->control.printPlugins();

    hh_logi("%s", res.c_str());
}

template <typename T>
std::vector<std::string> PluginService::getNodes()
{
    if constexpr (std::is_same_v<T, CommIO_plugin>)
    {
        return comm.getNodes();
    }
    else if constexpr (std::is_same_v<T, DeviceIO_plugin>)
    {
        return device.getNodes();
    }
    else if constexpr (std::is_same_v<T, ControlIO_plugin>)
    {
        return control.getNodes();
    }
    else
    {
        static_assert(always_false<T>::value, "Tipo de plugin no soportado");
    }
}

template <typename T>
std::vector<T*> PluginService::getNodes_T()
{
    if constexpr (std::is_same_v<T, CommIO_plugin>)
    {
        return comm.getNodes_T();
    }
    else if constexpr (std::is_same_v<T, DeviceIO_plugin>)
    {
        return device.getNodes_T();
    }
    else if constexpr (std::is_same_v<T, ControlIO_plugin>)
    {
        return control.getNodes_T();
    }
    else
    {
        static_assert(always_false<T>::value, "Tipo de plugin no soportado");
    }
}