#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <dlfcn.h>
#include <stdexcept>
#include <core/logger.h>
#include <filesystem>

template <typename T>
class PluginManager
{
    using create_t = T *();
    using destroy_t = void(T *);

    struct PluginEntry
    {
        void *handle = nullptr;
        create_t *create_fn = nullptr;
        destroy_t *destroy_fn = nullptr;
    };

    std::unordered_map<std::string, PluginEntry> loaded_plugins;
    std::unordered_map<std::string, std::unique_ptr<T, destroy_t *>> node_instances;

public:
    std::unordered_map<std::string, std::string> node_type;

    bool register_plugin(const std::string &name, const std::string &so_path)
    {
        if (loaded_plugins.count(name))
        {
            hh_logw("Plugin '%s' ya registrado. Ignorando...", name.c_str());
            return true;
        }

        // hh_logi("Cargando plugin '%s' desde: %s", name.c_str(), so_path.c_str());

        if(!std::filesystem::exists(so_path)){
            return false;
        }

        void *handle = dlopen(so_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle)
        {
            hh_loge("Error al cargar %s:\n%s", so_path.c_str(), dlerror());
            // throw std::runtime_error("dlopen failed for: " + so_path);
            return false;
        }

        auto create_fn = reinterpret_cast<create_t *>(dlsym(handle, "create"));
        auto destroy_fn = reinterpret_cast<destroy_t *>(dlsym(handle, "destroy"));

        if (!create_fn || !destroy_fn)
        {
            hh_loge("Faltan símbolos create/destroy en plugin '%s'", name.c_str());
            dlclose(handle);
            // throw std::runtime_error("Símbolos create/destroy no encontrados en: " + name);
            return false;
        }

        loaded_plugins[name] = PluginEntry{handle, create_fn, destroy_fn};
        // hh_logi("Plugin '%s' registrado exitosamente", name.c_str());
        return true;
    }

    void create_node(const std::string &node_id, const std::string &plugin_name)
    {
        auto it = loaded_plugins.find(plugin_name);
        if (it == loaded_plugins.end())
        {
            hh_loge("Plugin '%s' no está cargado", plugin_name.c_str());
            throw std::runtime_error("Plugin no registrado: " + plugin_name);
        }

        T *raw = it->second.create_fn();
        if (!raw)
        {
            hh_loge("create() retornó nullptr para '%s'", plugin_name.c_str());
            throw std::runtime_error("create() failed for plugin: " + plugin_name);
        }

        node_type[node_id] = plugin_name;
        node_instances.emplace(node_id, std::unique_ptr<T, destroy_t *>(raw, it->second.destroy_fn));
        hh_logi("Nodo '%s' creado usando plugin '%s'", node_id.c_str(), plugin_name.c_str());
    }

    T *get_node(const std::string &node_id)
    {
        auto it = node_instances.find(node_id);
        if (it == node_instances.end())
        {
            hh_loge("Nodo '%s' no encontrado", node_id.c_str());
            return nullptr;
            // throw std::out_of_range("Nodo no encontrado: " + node_id);
        }
        return it->second.get();
    }

    ~PluginManager()
    {
        node_instances.clear();
        for (auto &[name, entry] : loaded_plugins)
        {
            hh_logi("Descargando plugin: %s", name.c_str());
            dlclose(entry.handle);
        }
    }

    void printPlugins() const
    {
        hh_logi(" -> Plugins cargados:");
        for (const auto &[p, _] : loaded_plugins)
        {
            hh_logi("    --> %s", p.c_str());
        }
    }

    std::vector<std::string> getNodes()
    {
        std::vector<std::string> keys;
        keys.reserve(node_instances.size());
        for (const auto &pair : node_instances)
        {
            keys.push_back(pair.first);
        }
        return keys;
    }

    std::vector<T*> getNodes_T()
    {
        std::vector<T*> nodes;
        nodes.reserve(node_instances.size());
        for (const auto &pair : node_instances)
        {
            nodes.push_back(pair.second.get());
        }
        return nodes;
    }
};

#define __FINISH_PLUGIN_IO                                      \
    extern "C"                                                  \
    {                                                           \
        PLUGIN_IO_TYPE *create() { return new PLUGIN_IO_NAME; } \
        void destroy(PLUGIN_IO_TYPE *p) { delete p; }           \
    }

#define TO_STR(x) _TO_STR(x)
#define _TO_STR(x) #x
