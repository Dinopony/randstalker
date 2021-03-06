#include <exception>
#include <string>

class RandomizerException : public std::exception
{
public:
    RandomizerException(const std::string& message) : std::exception(), _message(message)
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