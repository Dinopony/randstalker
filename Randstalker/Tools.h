#pragma once

namespace Tools 
{
    /*
    template<typename T>
    T getRandomElementFrom(std::set<T>& set)
    {
        size_t size = set.size();
        uint32_t randomIndex = Random::getRandomInteger(0, size - 1);
     
        auto iter = set.begin();
        for (uint32_t i = 0; i < randomIndex; ++i)
            ++iter;

        return *iter;
    }

    template<typename T>
    T getRandomElementFrom(std::vector<T>& vector)
    {
        size_t size = vector.size();
        uint32_t randomIndex = Random::getRandomInteger(0, size - 1);
        return vector[randomIndex];
    }
    
    template<typename T>
    T extractRandomElementFrom(std::vector<T>& vector)
    {
        size_t size = vector.size();
        uint32_t randomIndex = Random::getRandomInteger(0, size-1);

        T obj = vector[randomIndex];
        vector.erase(vector.begin() + randomIndex);
        return obj;
    }*/

} // namespace end