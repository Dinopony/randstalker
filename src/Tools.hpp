#pragma once

#include <vector>
#include <map>
#include <string>
#include <random>
#include <algorithm>
#include <sstream>

namespace Tools 
{
    inline uint32_t getUniformInteger(uint32_t number, uint32_t rangeLow, uint32_t rangeHigh)
    {
        double proportion = static_cast<double>(number) / (1.0 + UINT32_MAX);
        uint32_t range = rangeHigh - rangeLow + 1;
        return static_cast<uint32_t>(proportion * range) + rangeLow;
    }

    template<typename T>
    void shuffle(std::vector<T>& vector, std::mt19937& rng)
    {
        if (vector.size() == 0)
            return;

        std::vector<T> elems;

        while (vector.size() > 1)
        {
            uint32_t randomPosition = getUniformInteger(rng(), 0, static_cast<uint32_t>(vector.size()-1));
            elems.push_back(vector[randomPosition]);
            vector.erase(vector.begin() + randomPosition);
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
            if (pos == std::string::npos) {
                pos = str.length();
            }
            std::string token = str.substr(prev, pos-prev);
            tokens.push_back(token);

            prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());

        return tokens;
    }

    template<typename T>
    std::string stringify(const T& value)
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
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

    inline void toLower(std::string& s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    }
}