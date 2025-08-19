#pragma once

#include <core/exprtk_wrapper.h>
#include <cstring>
#include "core/json_api.h"
#include "core/logger.h"

using namespace jsonapi;

struct eval_t
{
    bool enable = false;
    std::string to_eval = "";
    ExprtkParser parser;
    double x = 0.0;
    double t = 0.0;
    double y = 0.0;
    double off_y = 0.0;
    double aux[10];
};


bool get_eval_from_json(json_obj& _cfg, std::string key, eval_t &eval)
{
    std::vector<std::string> eval_arr;
    if (_cfg.get(key, &eval_arr))
    {
        eval.to_eval = "";
        for (size_t i = 0; i < eval_arr.size(); i++)
        {
            eval.to_eval += eval_arr[i] + (i + 1 == eval_arr.size() ? "" : " \n ");
        }
        eval.enable = true;
    }
    else
    {
        _cfg.get(key, &eval.to_eval);
        eval.enable = true;
    }

    if (eval.enable)
    {
        eval.parser.setVariable("aux0", &eval.aux[0]);
        eval.parser.setVariable("aux1", &eval.aux[1]);
        eval.parser.setVariable("aux2", &eval.aux[2]);
        eval.parser.setVariable("aux3", &eval.aux[3]);
        eval.parser.setVariable("aux4", &eval.aux[4]);
        eval.parser.setVariable("aux5", &eval.aux[5]);

        eval.parser.setVariable("x", &eval.x);
        eval.parser.setVariable("t", &eval.t);
        eval.parser.setVariable("off_y", &eval.off_y);
        if (!eval.parser.setExpression(eval.to_eval + " - off_y"))
        {
            hh_loge("Failed to parse expression %s", eval.to_eval.c_str());
            hh_loge(">> %s", eval.parser.error().c_str());
            return false;
        }
    }

    return true;
}