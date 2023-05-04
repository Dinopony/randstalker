#include <string>
#include <vector>

std::string base64_encode(const std::vector<uint8_t>& input);
std::vector<uint8_t> base64_decode(const std::string& encoded_string);
