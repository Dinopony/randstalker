#pragma once

#include <string>
#include <vector>
#include "../tools/megadrive/rom.hpp"
#include "huffman/tree.hpp"

class TextbanksDecoder
{
private:
    std::vector<std::string> _strings;
    std::vector<HuffmanTree*> _trees;

public:
    TextbanksDecoder(const md::ROM& rom);

    const std::vector<std::string>& strings() const { return _strings; };
    const std::vector<HuffmanTree*>& trees() const { return _trees; };

private:
    void parse_huffman_trees(const md::ROM& rom);
    void parse_textbanks(const md::ROM& rom);
    std::string parse_string(const std::vector<uint8_t>& data, short offset);
    uint8_t parse_next_symbol(HuffmanTree* huffman_tree, const std::vector<uint8_t>& data, short offset, uint32_t& string_bit_index);
};