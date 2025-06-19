
#include "json_api.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <sstream>
#include "tasking.h"

__attribute__((visibility("default")))
std::chrono::nanoseconds
parse_duration(const std::string &str)
{
    std::regex re(R"((\d+)(ns|us|ms|s))");
    std::smatch match;

    if (!std::regex_match(str, match, re))
    {
        throw std::invalid_argument("Invalid duration format: " + str);
    }

    uint64_t value = std::stoull(match[1]);
    std::string unit = match[2];

    if (unit == "ns")
        return std::chrono::nanoseconds(value);
    if (unit == "us")
        return std::chrono::microseconds(value);
    if (unit == "ms")
        return std::chrono::milliseconds(value);
    if (unit == "s")
        return std::chrono::seconds(value);

    return 0ns;
}

using namespace rapidjson;

namespace jsonapi
{

    class json_obj::Impl
    {
    public:
        Document doc;
        bool ok = false;

        Impl()
        {
            doc.SetObject();
            ok = true;
        }

        Document::AllocatorType &alloc()
        {
            return doc.GetAllocator();
        }
    };

    json_obj::json_obj() : impl_(new Impl()) {}
    json_obj::~json_obj() { delete impl_; }

    json_obj::json_obj(json_obj &&other) noexcept
    {
        impl_ = other.impl_;
        other.impl_ = nullptr;
    }

    json_obj &json_obj::operator=(json_obj &&other) noexcept
    {
        if (this != &other)
        {
            delete impl_;
            impl_ = other.impl_;
            other.impl_ = nullptr;
        }
        return *this;
    }

    bool json_obj::valid() const
    {
        return impl_ && impl_->ok;
    }

    json_obj json_obj::from_file(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
            return json_obj();

        std::stringstream buffer;
        buffer << file.rdbuf();
        return from_string(buffer.str());
    }

    json_obj json_obj::from_string(const std::string &json_str)
    {
        Document *temp = new Document();
        temp->Parse(json_str.c_str());

        if (temp->HasParseError())
        {
            delete temp;
            return json_obj();
        }

        json_obj j;
        delete j.impl_;
        j.impl_ = new Impl();
        j.impl_->doc.CopyFrom(*temp, j.impl_->alloc());
        j.impl_->ok = true;
        delete temp;
        return j;
    }

    int json_obj::save_to_file(const std::string &path) const
    {
        std::ofstream out(path);
        if (!out.is_open())
            return -1;

        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        impl_->doc.Accept(writer);

        out << buffer.GetString();
        out.close();
        return 0;
    }

    std::string json_obj::str(bool pretty) const
    {
        StringBuffer buffer;
        if (pretty)
        {
            PrettyWriter<StringBuffer> writer(buffer);
            impl_->doc.Accept(writer);
        }
        else
        {
            Writer<StringBuffer> writer(buffer);
            impl_->doc.Accept(writer);
        }
        return buffer.GetString();
    }

    std::vector<std::string> json_obj::keys() const
    {
        std::vector<std::string> result;
        const auto &doc = impl_->doc;
        if (!doc.IsObject())
            return result;

        for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it)
        {
            result.emplace_back(it->name.GetString(), it->name.GetStringLength());
        }

        return result;
    }

    // Internal getters
    int json_obj::get_chrono_ns(const std::string &key, std::chrono::nanoseconds *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsString())
            return -1;
        try
        {
            *out = parse_duration(doc[key.c_str()].GetString());
            return 0;
        }
        catch (...)
        {
            return -1;
        }
    }

    int json_obj::get_int(const std::string &key, int *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsInt())
            return -1;
        *out = doc[key.c_str()].GetInt();
        return 0;
    }
    int json_obj::get_bool(const std::string &key, bool *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsBool())
            return -1;
        *out = doc[key.c_str()].GetBool();
        return 0;
    }
    int json_obj::get_uint64(const std::string &key, uintptr_t *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsUint64())
            return -1;
        *out = static_cast<uintptr_t>(doc[key.c_str()].GetUint64());
        return 0;
    }

    int json_obj::get_uint32(const std::string &key, uint32_t *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsUint())
            return -1;
        *out = static_cast<uintptr_t>(doc[key.c_str()].GetUint());
        return 0;
    }

    template <>
    bool json_obj::get<uintptr_t>(const std::string &key, uintptr_t *out) const
    {
        return get_uint64(key, out) == 0;
    }

    template <>
    bool json_obj::get<uint32_t>(const std::string &key, uint32_t *out) const
    {
        return get_uint32(key, out) == 0;
    }

    bool json_obj::contain(std::string _key){
        auto items = this->keys();
        return std::find(items.begin(), items.end(), _key) != items.end();
    }

    template <>
    bool json_obj::get<std::chrono::nanoseconds>(const std::string &key, std::chrono::nanoseconds *out) const
    {
        return get_chrono_ns(key, out) == 0;
    }

    int json_obj::get_float(const std::string &key, float *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsNumber())
            return -1;
        *out = doc[key.c_str()].GetFloat();
        return 0;
    }

    int json_obj::get_double(const std::string &key, double *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsNumber())
            return -1;
        *out = doc[key.c_str()].GetDouble();
        return 0;
    }

    int json_obj::get_string(const std::string &key, std::string *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsString())
            return -1;
        *out = doc[key.c_str()].GetString();
        return 0;
    }

    std::string json_obj::get_object_str(const std::string &key, bool pretty) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsObject())
            return "";

        rapidjson::StringBuffer buffer;
        if (pretty)
        {
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            doc[key.c_str()].Accept(writer);
        }
        else
        {
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc[key.c_str()].Accept(writer);
        }
        return std::string(buffer.GetString(), buffer.GetSize());
    }

    int json_obj::get_array(const std::string &key, std::vector<float> *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsArray())
            return -1;

        const auto &arr = doc[key.c_str()].GetArray();
        out->clear();
        out->reserve(arr.Size());

        for (const auto &v : arr)
        {
            if (!v.IsNumber())
                return -2;
            out->emplace_back(v.GetFloat());
        }

        return 0;
    }

    int json_obj::get_array(const std::string &key, std::vector<std::string> *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsArray())
            return -1;

        const auto &arr = doc[key.c_str()].GetArray();
        out->clear();
        out->reserve(arr.Size());

        for (const auto &v : arr)
        {
            if (!v.IsString())
                return -2;
            out->emplace_back(v.GetString(), v.GetStringLength());
        }

        return 0;
    }

    int json_obj::get_object(const std::string &key, json_obj *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsObject())
            return -1;

        json_obj j;
        delete j.impl_;
        j.impl_ = new Impl();
        j.impl_->doc.CopyFrom(doc[key.c_str()], j.impl_->alloc());
        j.impl_->ok = true;

        *out = std::move(j);
        return out->valid() ? 0 : -1;
    }

    // Template specializations
    template <>
    bool json_obj::get<int>(const std::string &key, int *out) const
    {
        return get_int(key, out) == 0;
    }

    template <>
    bool json_obj::get<bool>(const std::string &key, bool *out) const
    {
        return get_bool(key, out) == 0;
    }

    template <>
    bool json_obj::get<float>(const std::string &key, float *out) const
    {
        return get_float(key, out) == 0;
    }

    template <>
    bool json_obj::get<double>(const std::string &key, double *out) const
    {
        return get_double(key, out) == 0;
    }

    template <>
    bool json_obj::get<std::string>(const std::string &key, std::string *out) const
    {
        return get_string(key, out) == 0;
    }

    template <>
    bool json_obj::get<std::vector<float>>(const std::string &key, std::vector<float> *out) const
    {
        return get_array(key, out) == 0;
    }

    template <>
    bool json_obj::get<std::vector<std::string>>(const std::string &key, std::vector<std::string> *out) const
    {
        return get_array(key, out) == 0;
    }

    template <>
    bool json_obj::get<std::vector<json_obj>>(const std::string &key, std::vector<json_obj> *out) const
    {
        return get_array(key, out) == 0;
    }

    template <>
    bool json_obj::get<json_obj>(const std::string &key, json_obj *out) const
    {
        return get_object(key, out) == 0;
    }

    template <>
    void json_obj::set<int>(const std::string &key, const int &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        doc.AddMember(Value(key.c_str(), alloc), Value(value), alloc);
    }

    template <>
    void json_obj::set<uintptr_t>(const std::string &key, const uintptr_t &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        doc.AddMember(Value(key.c_str(), alloc), Value(value), alloc);
    }

    template <>
    void json_obj::set<float>(const std::string &key, const float &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        Value val;
        val.SetFloat(value);
        doc.AddMember(Value(key.c_str(), alloc), val, alloc);
    }

    template <>
    void json_obj::set<double>(const std::string &key, const double &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        Value val;
        val.SetDouble(value);
        doc.AddMember(Value(key.c_str(), alloc), val, alloc);
    }

    template <>
    void json_obj::set<std::string>(const std::string &key, const std::string &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        doc.AddMember(Value(key.c_str(), alloc), Value(value.c_str(), alloc), alloc);
    }

    template <>
    void json_obj::set<std::vector<float>>(const std::string &key, const std::vector<float> &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        Value arr(kArrayType);
        for (float f : value)
            arr.PushBack(Value().SetFloat(f), alloc);
        doc.AddMember(Value(key.c_str(), alloc), arr, alloc);
    }

    template <>
    void json_obj::set<std::vector<std::string>>(const std::string &key, const std::vector<std::string> &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        Value arr(kArrayType);
        for (const auto &s : value)
            arr.PushBack(Value(s.c_str(), alloc), alloc);
        doc.AddMember(Value(key.c_str(), alloc), arr, alloc);
    }

    template <>
    void json_obj::set<json_obj>(const std::string &key, const json_obj &value)
    {
        auto &alloc = impl_->alloc();
        auto &doc = impl_->doc;
        if (doc.HasMember(key.c_str()))
            doc.RemoveMember(key.c_str());
        Value obj(kObjectType);
        obj.CopyFrom(value.impl_->doc, alloc);
        doc.AddMember(Value(key.c_str(), alloc), obj, alloc);
    }

    int json_obj::get_array(const std::string &key, std::vector<json_obj> *out) const
    {
        const auto &doc = impl_->doc;
        if (!doc.HasMember(key.c_str()) || !doc[key.c_str()].IsArray())
            return -1;

        const auto &arr = doc[key.c_str()].GetArray();
        out->clear();
        out->reserve(arr.Size());

        for (const auto &v : arr)
        {
            if (!v.IsObject())
                return -2;

            json_obj j;
            delete j.impl_;
            j.impl_ = new Impl();
            j.impl_->doc.CopyFrom(v, j.impl_->alloc());
            j.impl_->ok = true;
            out->emplace_back(std::move(j));
        }

        return 0;
    }

    bool get_json_key(const json_obj &parent, const std::string &key, json_obj *out)
    {
        if (!parent.get(key, out))
        {
            printf("Key NOT found: %s\n", key.c_str());
            return false;
        }
        return true;
    }
}