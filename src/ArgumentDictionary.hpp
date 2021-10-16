#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include "Tools.hpp"

class ArgumentDictionary {
private:
	std::map<std::string, std::string> _argsMap;

public:
    ArgumentDictionary(int argc, char* argv[])
    {
    	for (int i = 1; i < argc; ++i)
        {
            std::string param = argv[i];
            if (param[0] != '-' || param[1] != '-')
                continue;

            auto tokenIter = std::find(param.begin() + 2, param.end(), '=');
            std::string paramName(param.begin() + 2, tokenIter);
            Tools::toLower(paramName);
            if (tokenIter != param.end())
                _argsMap[paramName] = std::string(tokenIter + 1, param.end());
            else
                _argsMap[paramName] = "";
        }
    }

    bool contains(const std::string& name) const {
        return _argsMap.count(name) > 0;
    }

    std::string getString(const std::string& name, const std::string& defaultValue = "") const
    {
        try {
            return _argsMap.at(name);
        } catch (std::out_of_range&) {
            return defaultValue;
        }
    }

    int getInteger(const std::string& name, int defaultValue = 0) const
    {
        try {
            return std::stoi(_argsMap.at(name));
        } 
        catch (std::out_of_range&) {}
        catch (std::invalid_argument&) {}

        return defaultValue;
    }

    double getDouble(const std::string& name, double defaultValue = 0.0) const
    {
        try {
            return std::stod(_argsMap.at(name));
        } 
        catch (std::out_of_range&) {}
        catch (std::invalid_argument&) {}

        return defaultValue;
    }

    bool getBoolean(const std::string& name, bool defaultValue = false) const
    {
        // "--noParam" <==> "--param=false"
        std::string negationParam = "no" + name;
        if (_argsMap.count(negationParam))
            return false;

        try {
            std::string contents = _argsMap.at(name);
            Tools::toLower(contents);

            // "--param=false"
            if (contents == "false")
                return false;

            // "--param=true" or "--param"
            return true;
        } catch (std::out_of_range&) {
            // No trace of "--param" or "--noParam", return default value
            return defaultValue;
        }
    }
};