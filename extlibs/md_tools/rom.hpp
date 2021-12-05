#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace md {

    class Code;

    class ROM
    {
    private:
        bool _was_open;
        char* _byte_array;
        std::map<std::string, uint32_t> _stored_addresses;
        std::vector<std::pair<uint32_t, uint32_t>> _empty_chunks;

    public:
        ROM(const std::string& input_path);
        ROM(const ROM& other);
        ~ROM();

        bool is_valid() const { return _was_open; }

        uint8_t get_byte(uint32_t address) const { return _byte_array[address]; }
        uint16_t get_word(uint32_t address) const { return (this->get_byte(address) << 8) + this->get_byte(address+1); }
        uint32_t get_long(uint32_t address) const { return (static_cast<uint32_t>(this->get_word(address)) << 16) + static_cast<uint32_t>(this->get_word(address+2)); }

        void set_byte(uint32_t address, uint8_t byte);
        void set_word(uint32_t address, uint16_t word);
        void set_long(uint32_t address, uint32_t long_word);
        void set_bytes(uint32_t address, std::vector<uint8_t> bytes);
        void set_bytes(uint32_t address, const unsigned char* bytes, size_t bytes_size);
        void set_code(uint32_t address, const Code& code);

        uint32_t inject_bytes(const std::vector<uint8_t>& bytes, const std::string& label = "");
        uint32_t inject_bytes(const unsigned char* bytes, size_t size_to_inject, const std::string& label = "");
        uint32_t inject_code(const Code& code, const std::string& label = "");
        uint32_t reserve_data_block(uint32_t byte_count, const std::string& label = "");

        void store_address(const std::string& name, uint32_t address) { _stored_addresses[name] = address; }
        uint32_t stored_address(const std::string& name) { return _stored_addresses.at(name); }

        void data_chunk(uint32_t begin, uint32_t end, std::vector<uint8_t>& output) const;
        void data_chunk(uint32_t begin, uint32_t end, std::vector<uint16_t>& output) const;
        void data_chunk(uint32_t begin, uint32_t end, std::vector<uint32_t>& output) const;
        std::vector<uint8_t> data_chunk(uint32_t begin, uint32_t end) const;

        void mark_empty_chunk(uint32_t begin, uint32_t end);
        uint32_t remaining_empty_bytes() const;
    
        void write_to_file(std::ofstream& output_file);

    private:
        void update_checksum();
    };

}