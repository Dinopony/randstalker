#include <exception>
#include <string>

class RandomizerException : public std::exception
{
public:
    RandomizerException(const std::string& message) : std::exception(message.c_str())
    {}
};

class WrongVersionException : public RandomizerException
{
public:
    WrongVersionException(const std::string& message) : RandomizerException(message)
    {}
};