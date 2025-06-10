#pragma once

#include <cstring>
#include <string>
#include <ctype.h>
#include <unordered_map>
#include <stdint.h>

enum class Var_type : uint8_t
{
    _NO_TYPE = 0,
    _INT_TYPE,    // int64_t
    _INT32_TYPE,  // int32_t
    _FLOAT_TYPE,  // float
    _DOUBLE_TYPE, // double
    _UINT16_TYPE  // uint16_t
};

Var_type parse_var_type(const std::string &str)
{
    static const std::unordered_map<std::string, Var_type> type_map = {
        {"int", Var_type::_INT_TYPE},
        {"int32", Var_type::_INT32_TYPE},
        {"float", Var_type::_FLOAT_TYPE},
        {"double", Var_type::_DOUBLE_TYPE},
        {"uint16", Var_type::_UINT16_TYPE},
        {"none", Var_type::_NO_TYPE}};

    auto it = type_map.find(str);
    return (it != type_map.end()) ? it->second : Var_type::_NO_TYPE;
}

template<typename T>
inline double convert_value(void* tmp_buffer)
{
    T value;
    ::memcpy(&value, tmp_buffer, sizeof(T));
    return static_cast<double>(value);
}

inline bool convert_buffer_to_double(void* tmp_buffer, double& x, Var_type _type)
{
    switch (_type)
    {
    case Var_type::_INT_TYPE:
        x = convert_value<int64_t>(tmp_buffer);
        break;

    case Var_type::_INT32_TYPE:
        x = convert_value<int32_t>(tmp_buffer);
        break;

    case Var_type::_FLOAT_TYPE:
        x = convert_value<float>(tmp_buffer);
        break;

    case Var_type::_DOUBLE_TYPE:
        x = convert_value<double>(tmp_buffer);
        break;

    case Var_type::_UINT16_TYPE:
        x = convert_value<uint16_t>(tmp_buffer);
        break;

    default:
        return false;
    }

    return true;
}