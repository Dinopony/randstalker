#pragma once

#include <vector>
#include <map>
#include <string>
#include <random>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <json.hpp>

namespace tools 
{
    inline uint32_t get_uniform_integer(uint32_t number, uint32_t range_low, uint32_t range_high)
    {
        double proportion = static_cast<double>(number) / (1.0 + UINT32_MAX);
        uint32_t range = range_high - range_low + 1;
        return static_cast<uint32_t>(proportion * range) + range_low;
    }

    template<typename T>
    void shuffle(std::vector<T>& vector, std::mt19937& rng)
    {
        if (vector.size() == 0)
            return;

        std::vector<T> elems;

        while (vector.size() > 1)
        {
            uint32_t random_pos = get_uniform_integer(rng(), 0, static_cast<uint32_t>(vector.size()-1));
            elems.push_back(vector[random_pos]);
            vector.erase(vector.begin() + random_pos);
        }
          
        elems.push_back(*vector.begin());
        vector = elems;
    }

    inline std::vector<std::string> split(const std::string& str, const std::string& delim)
    {
        std::vector<std::string> tokens;
        size_t prev = 0, pos = 0;
        do {
            pos = str.find(delim, prev);
            if (pos == std::string::npos)
                pos = str.length();
            std::string token = str.substr(prev, pos-prev);
            tokens.push_back(token);

            prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());

        return tokens;
    }

    inline std::string join(const std::vector<std::string>& words, const std::string& junction)
    {
        if(words.size() == 0)
            return "";
            
        std::string ret = words[0];
        for(size_t i=1 ; i<words.size() ; ++i)
        {
            ret += junction;
            ret += words[i];
        }

        return ret;
    }

    template<typename T>
    static std::string hexify(const T& val)
    {
        std::ostringstream oss;
        oss << std::uppercase << std::hex << val;
        return oss.str();
    }

    // trim from start (in place)
    inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    inline void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    inline void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }

    inline void to_lower(std::string& s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    inline bool ends_with(const std::string& haystack, const std::string& needle)
    {
        if (needle.size() > haystack.size())
            return false;
        return std::equal(needle.rbegin(), needle.rend(), haystack.rbegin());
    }

    inline bool dump_json_to_file(const Json& json, const std::string& file_path)
    {
        std::ofstream file(file_path);
        if(!file)
            return false;

        file << json.dump(4);
        file.close();
        return true;
    }
}