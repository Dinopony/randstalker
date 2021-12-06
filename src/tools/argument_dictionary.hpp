#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include "tools.hpp"

class ArgumentDictionary {
private:
    std::map<std::string, std::string> _args_map;

public:
    ArgumentDictionary(int argc, char* argv[])
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string param = argv[i];
            if (param[0] != '-' || param[1] != '-')
                continue;

            auto token_iter = std::find(param.begin() + 2, param.end(), '=');
            std::string param_name(param.begin() + 2, token_iter);
            tools::to_lower(param_name);
            if (token_iter != param.end())
                _args_map[param_name] = std::string(token_iter + 1, param.end());
            else
                _args_map[param_name] = "";
        }
    }

    [[nodiscard]] bool contains(const std::string& name) const {
        return _args_map.count(name) || _args_map.count("no" + name);
    }

    [[nodiscard]] std::string get_string(const std::string& name, const std::string& default_value = "") const
    {
        try {
            return _args_map.at(name);
        } catch (std::out_of_range&) {
            return default_value;
        }
    }

    [[nodiscard]] int get_integer(const std::string& name, int default_value = 0) const
    {
        try {
            return std::stoi(_args_map.at(name));
        } 
        catch (std::out_of_range&) {}
        catch (std::invalid_argument&) {}

        return default_value;
    }

    [[nodiscard]] double get_double(const std::string& name, double default_value = 0.0) const
    {
        try {
            return std::stod(_args_map.at(name));
        } 
        catch (std::out_of_range&) {}
        catch (std::invalid_argument&) {}

        return default_value;
    }

    [[nodiscard]] bool get_boolean(const std::string& name, bool default_value = false) const
    {
        // "--noParam" <==> "--param=false"
        std::string negation_param = "no" + name;
        if (_args_map.count(negation_param))
            return false;

        try {
            std::string contents = _args_map.at(name);
            tools::to_lower(contents);

            // "--param=false"
            if (contents == "false")
                return false;

            // "--param=true" or "--param"
            return true;
        } catch (std::out_of_range&) {
            // No trace of "--param" or "--noParam", return default value
            return default_value;
        }
    }
};