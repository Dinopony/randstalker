#pragma once

#include <vector>
#include <map>
#include <string>
#include <random>

namespace Tools 
{
    template<typename T>
    void shuffle(std::vector<T>& vector, std::mt19937& rng)
    {
        std::map<uint32_t, T> elems;
        for (T elem : vector)
            elems[rng()] = elem;

        vector.clear();
        for (auto [key, value] : elems)
            vector.push_back(value);
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