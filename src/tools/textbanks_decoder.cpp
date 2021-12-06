#include "textbanks_decoder.hpp"

TextbanksDecoder::TextbanksDecoder(const md::ROM& rom) 
{
    parse_huffman_trees(rom);
    parse_textbanks(rom);
}

void TextbanksDecoder::parse_huffman_trees(const md::ROM& rom)
{
    const std::pair<uint32_t, uint32_t> HUFFMAN_TREE_OFFSETS(0x23D60, 0x23E38);
    const std::pair<uint32_t, uint32_t> HUFFMAN_TREES(0x23E38,0x2469C);
    
    std::vector<uint16_t> trees_offsets;
    rom.data_chunk(HUFFMAN_TREE_OFFSETS.first, HUFFMAN_TREE_OFFSETS.second, trees_offsets);

    std::vector<uint8_t> trees_data;
    rom.data_chunk(HUFFMAN_TREES.first, HUFFMAN_TREES.second, trees_data);

    for (uint32_t i = 0; i < trees_offsets.size() ; ++i)
    {
        uint16_t tree_offset = trees_offsets[i];
        if (tree_offset == 0xFFFF)
            _trees.emplace_back(nullptr);
        else
            _trees.emplace_back(new HuffmanTree(trees_data, tree_offset));
    }
}

void TextbanksDecoder::parse_textbanks(const md::ROM& rom)
{
    const std::vector<uint32_t> TEXTBANKS_OFFSETS = { 
        0x2B27A, 0x2C29B, 0x2DCC8, 0x2F787, 0x3153E, 
        0x330AB, 0x34830, 0x36087, 0x377E3, 0x38368 
    };
    constexpr uint16_t STRINGS_PER_TEXTBANK = 256;

    for (uint32_t i = 0; i < TEXTBANKS_OFFSETS.size()-1 ; i++)
    {
        std::vector<uint8_t> data;
        rom.data_chunk(TEXTBANKS_OFFSETS[i], TEXTBANKS_OFFSETS[i + 1], data);

        uint32_t current_string_offset = 0;
        for (uint32_t j = 0; j < STRINGS_PER_TEXTBANK; j++)
        {
            uint8_t string_length = data[current_string_offset];
            std::string string = parse_string(data, current_string_offset + 1);
            _strings.emplace_back(string);

            current_string_offset += string_length;
            if (current_string_offset + 1 >= data.size())
                break;
        }
    }
}

std::string TextbanksDecoder::parse_string(const std::vector<uint8_t>& data, short offset)
{
    std::string string;
    uint8_t previous_symbol = 0x55;
    uint32_t string_bit_index = 0;

    while (true)
    {
        uint8_t symbol = parse_next_symbol(_trees[previous_symbol], data, offset, string_bit_index);
        if (symbol == 0x55)
            break;

        string += Symbols::TABLE[symbol];
        previous_symbol = symbol;
    }

    return string;
}

uint8_t TextbanksDecoder::parse_next_symbol(HuffmanTree* huffman_tree, const std::vector<uint8_t>& data, short offset, uint32_t& string_bit_index)
{
    Bitfield bits;

    while (true)
    {
        const uint8_t* symbol = huffman_tree->decode(bits);
        if(symbol)
            return *symbol;

        uint8_t string_byte = data.at(offset + (string_bit_index / 8));
        uint8_t string_bit = (string_byte >> (7 - (string_bit_index % 8))) & 1;
        bits.add(string_bit);
        string_bit_index++;
    }
}
