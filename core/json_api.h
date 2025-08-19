
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <variant>

#include <chrono>

__attribute__((visibility("default")))
std::chrono::nanoseconds
parse_duration(const std::string &str);

#define _CFG_FIELD_GET(f) _cfg.get(#f, &f)
#define _CFG_FIELD_GET_DEST(f,d) _cfg.get(#f, &d)
#define _CFG_FIELD_GET_DEST_STRUCT(f, d) _cfg.get(#f, &d.f)

namespace jsonapi
{

    class json_obj
    {
    public:
        json_obj();
        ~json_obj();

        json_obj(const json_obj &) = delete;
        json_obj &operator=(const json_obj &) = delete;
        json_obj(json_obj &&other) noexcept;
        json_obj &operator=(json_obj &&other) noexcept;

        std::string str(bool pretty = true) const;
        int save_to_file(const std::string &path) const;
        static json_obj from_file(const std::string &path);
        static json_obj from_string(const std::string &json_str);
        bool valid() const;

        std::vector<std::string> keys() const;

        template <typename T>
        bool get(const std::string &key, T *out) const;

        std::string get_object_str(const std::string& key, bool pretty = true) const;

        bool contain(std::string _key);

        template <typename T>
        void set(const std::string &key, const T &value);

    private:
        class Impl;
        Impl *impl_;

        int get_bool(const std::string &key, bool *out) const;
        int get_int(const std::string &key, int *out) const;
        int get_uint64(const std::string& key, uintptr_t* out) const;
        int get_uint32(const std::string& key, uint32_t* out) const;
        int get_float(const std::string &key, float *out) const;
        int get_double(const std::string &key, double *out) const;
        int get_string(const std::string &key, std::string *out) const;
        int get_array(const std::string &key, std::vector<float> *out) const;
        int get_array(const std::string &key, std::vector<double> *out) const;
        int get_array(const std::string &key, std::vector<std::string> *out) const;
        int get_array(const std::string& key, std::vector<json_obj>* out) const;
        int get_object(const std::string &key, json_obj *out) const;
        int get_chrono_ns(const std::string &key, std::chrono::nanoseconds * out) const;
    };


    bool get_json_key(const json_obj& parent, const std::string& key, json_obj* out);
}