#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>

#include "huffman/tree.hpp"
#include "huffman/symbols.hpp"
#include "megadrive/rom.hpp"

class TextbanksEncoder 
{
private:
    std::vector<HuffmanTree*> _trees;
    std::vector<std::vector<uint8_t>> _textbanks;

public:
    TextbanksEncoder(md::ROM& rom, const std::vector<std::string>& strings);

    void write_to_rom(md::ROM& rom);

private:
    void build_trees(const std::vector<std::string>& strings);
    void count_symbols(const std::vector<std::string>& strings, std::map<uint8_t, std::vector<SymbolCount>>& sortedSymbolCounts);
    std::vector<uint8_t> string_to_symbols(const std::string& strings);
    void produce_textbanks(const std::vector<std::string>& strings);
};
