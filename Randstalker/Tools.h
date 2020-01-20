#pragma once

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
}