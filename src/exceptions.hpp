#pragma once

#include <exception>
#include <string>

class RandomizerException
{
public:
    RandomizerException() :  _message()
    {}

    RandomizerException(const std::string& message) :  _message(message)
    {}

    const char* what() const noexcept { return _message.c_str(); }

private:
    std::string _message;
};

class WrongVersionException : public RandomizerException
{
public:
    WrongVersionException(const std::string& message) : RandomizerException(message)
    {}
};

class JsonParsingException : public RandomizerException
{
public:
    JsonParsingException(const std::string& message) : RandomizerException(message)
    {}   
};