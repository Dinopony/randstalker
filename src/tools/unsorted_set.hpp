#pragma once

#include <vector>
#include <initializer_list>
#include <algorithm>

template<typename T>
class UnsortedSet : public std::vector<T> {
public:
    UnsortedSet() : std::vector<T>()
    {}

    UnsortedSet(std::initializer_list<T> il) : std::vector<T>(il)
    {}

    UnsortedSet(const std::vector<T>& vec) : std::vector<T>(vec)
    {}

    void insert(const T& elem) 
    {
        if (!this->contains(elem))
            this->emplace_back(elem);
    }

    void insert(const UnsortedSet<T>& other) 
    {
        for(const T& elem : other)
            this->insert(elem);
    }

    void erase(const T& elem)
    {
        auto iter = std::find(this->begin(), this->end(), elem);
        if(iter != this->end())
            std::vector<T>::erase(iter);
    }

    bool contains(const T& elem) const
    {
        auto iter = std::find(this->begin(), this->end(), elem);
        return iter != this->end();
    }

    UnsortedSet<T> diff(const UnsortedSet<T>& other) const
    {
        UnsortedSet<T> diff;
        for(const T& elem : *this)
        {
            if(!other.contains(elem))
                diff.insert(other);
        }
        return diff;
    }
};
