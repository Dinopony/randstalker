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

	void insert(const T& elem) 
	{
		if (!this->contains(elem))
			this->push_back(elem);
	}

	void erase(const T& elem)
	{
		auto iter = std::find(this->begin(), this->end(), elem);
		std::vector<T>::erase(iter);
	}

	bool contains(const T& elem) const
	{
		auto iter = std::find(this->begin(), this->end(), elem);
		return iter != this->end();
	}
};