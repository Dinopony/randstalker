#include <string>
#include <vector>
#include <array>

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string base64_encode(const std::vector<uint8_t>& input)
{
    std::string encoded;
    std::array<uint8_t, 3> input_buffer {};
    std::array<uint8_t, 4> output_buffer {};

    int i = 0;
    auto it = input.begin();
    while(it != input.end())
    {
        input_buffer[i++] = *(it++);
        if (i == input_buffer.size())
        {
            output_buffer[0] = (input_buffer[0] & 0xfc) >> 2;
            output_buffer[1] = ((input_buffer[0] & 0x03) << 4) + ((input_buffer[1] & 0xf0) >> 4);
            output_buffer[2] = ((input_buffer[1] & 0x0f) << 2) + ((input_buffer[2] & 0xc0) >> 6);
            output_buffer[3] = input_buffer[2] & 0x3f;

            for(uint8_t byte : output_buffer)
                encoded += base64_chars[byte];
            i = 0;
        }
    }

    if (i)
    {
        for(size_t j=i ; j < input_buffer.size() ; ++j)
            input_buffer[j] = 0;

        output_buffer[0] = (input_buffer[0] & 0xfc) >> 2;
        output_buffer[1] = ((input_buffer[0] & 0x03) << 4) + ((input_buffer[1] & 0xf0) >> 4);
        output_buffer[2] = ((input_buffer[1] & 0x0f) << 2) + ((input_buffer[2] & 0xc0) >> 6);
        output_buffer[3] = input_buffer[2] & 0x3f;

        for (size_t j=0 ; j<i+1 ; ++j)
            encoded += base64_chars[output_buffer[j]];
    }

    return encoded;
}

std::vector<uint8_t> base64_decode(const std::string& encoded_string)
{
    int i = 0;
    auto it = encoded_string.begin();
    std::array<uint8_t, 4> input_buffer {};
    std::array<uint8_t, 3> output_buffer {};
    std::vector<uint8_t> decoded;

    while (it != encoded_string.end())
    {
        char c = *(it++);

        auto char_index = base64_chars.find(c);
        if(char_index == std::string::npos)
            break; // Not a valid base64 character
        input_buffer[i++] = static_cast<uint8_t>(char_index);

        if (i == input_buffer.size())
        {
            output_buffer[0] = (input_buffer[0] << 2) + ((input_buffer[1] & 0x30) >> 4);
            output_buffer[1] = ((input_buffer[1] & 0xf) << 4) + ((input_buffer[2] & 0x3c) >> 2);
            output_buffer[2] = ((input_buffer[2] & 0x3) << 6) + input_buffer[3];

            for (uint8_t byte : output_buffer)
                decoded.emplace_back(byte);
            i = 0;
        }
    }

    if (i)
    {
        for (int j=i ; j<input_buffer.size() ; ++j)
            input_buffer[j] = 0;

        output_buffer[0] = (input_buffer[0] << 2) + ((input_buffer[1] & 0x30) >> 4);
        output_buffer[1] = ((input_buffer[1] & 0xf) << 4) + ((input_buffer[2] & 0x3c) >> 2);
        output_buffer[2] = ((input_buffer[2] & 0x3) << 6) + input_buffer[3];

        for (int j=0 ; j < i-1 ; ++j)
            decoded.emplace_back(output_buffer[j]);
    }

    return decoded;
}
