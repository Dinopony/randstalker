#include "textbanks_encoder.hpp"

TextbanksEncoder::TextbanksEncoder(md::ROM& rom, const std::vector<std::string>& strings)
{
    this->build_trees(strings);
    this->produce_textbanks(strings);
}

void TextbanksEncoder::build_trees(const std::vector<std::string>& strings)
{
    std::map<uint8_t, std::vector<SymbolCount>> sorted_symbol_counts;
    this->count_symbols(strings, sorted_symbol_counts);

    for (uint8_t symbol_id = 0; symbol_id < SYMBOL_COUNT; symbol_id++)
    {
        const std::vector<SymbolCount>& next_symbol_counts = sorted_symbol_counts[symbol_id];
        if (next_symbol_counts.empty())
        {
            _trees.push_back(nullptr);
            continue;
        }
        
        _trees.push_back(new HuffmanTree(next_symbol_counts));
    }
}

void TextbanksEncoder::count_symbols(const std::vector<std::string>& strings, std::map<uint8_t, std::vector<SymbolCount>>& sorted_symbol_counts)
{
    uint32_t symbol_counts[SYMBOL_COUNT][SYMBOL_COUNT];
    uint8_t previous_symbol = 0x55;

    for (uint8_t i = 0; i < SYMBOL_COUNT; ++i)
        for (uint8_t j = 0; j < SYMBOL_COUNT; ++j)
            symbol_counts[i][j] = 0;

    for (const std::string& string : strings)
    {
        // Step 1: Convert string into LS character table symbols
        std::vector<uint8_t> string_as_symbols = string_to_symbols(string);
        
        // Step 2: Count symbols in this text line
        uint8_t previous_symbol = 0x55;
        for (uint8_t symbol : string_as_symbols)
        {
            symbol_counts[previous_symbol][symbol]++;
            previous_symbol = symbol;
        }
    }

    // Step 3: For each symbol, sort counts by descending order to find the most common following symbols
    for (uint8_t i = 0; i < SYMBOL_COUNT; ++i)
    {
        std::vector<SymbolCount>& next_symbol_counts = sorted_symbol_counts[i];
        for (uint8_t j = 0; j < SYMBOL_COUNT; ++j)
            if(symbol_counts[i][j])
                next_symbol_counts.push_back(SymbolCount(j, symbol_counts[i][j]));
        std::sort(next_symbol_counts.begin(), next_symbol_counts.end());
    }
}

std::vector<uint8_t> TextbanksEncoder::string_to_symbols(const std::string& string)
{
    std::vector<uint8_t> string_as_symbols;

    for (size_t i = 0; i < string.length(); ++i)
    {
        for (uint8_t symbol_id = 0; symbol_id < SYMBOL_COUNT; symbol_id++)
        {
            char symbol_as_char = Symbols::TABLE[symbol_id];
            char current_character = string[i];

            if (current_character == symbol_as_char)
            {
                string_as_symbols.push_back(symbol_id);
                break;
            }

            if (symbol_id + 1 == SYMBOL_COUNT)
            {
                std::cerr << "Current character '" << string[i] << "' is not recognized as the beginning of a known symbol, and will be ignored.\n";
                ++i;
            }
        }
    }

    string_as_symbols.push_back(0x55);
    return string_as_symbols;
}


void TextbanksEncoder::produce_textbanks(const std::vector<std::string>& strings)
{
    std::vector<std::vector<uint8_t>> compressed_strings_bytes;
    for (const std::string& string : strings)
    {
        std::vector<uint8_t> string_as_symbols = string_to_symbols(string);

        uint8_t previous_symbol = 0x55;
        Bitfield bits;
        for (uint8_t symbol : string_as_symbols)
        {
            try
            {
                bits += _trees[previous_symbol]->encode(symbol);
                previous_symbol = symbol;
            } 
            catch (std::out_of_range&)
            {
                std::cerr << "Symbol '" << Symbols::TABLE[symbol] << "' cannot be after a '" << Symbols::TABLE[previous_symbol] << "' and will be ignored.\n";
            }
        }
        while (bits.size() % 8 != 0)
            bits.add(0);

        compressed_strings_bytes.push_back(bits.to_bytes());
    }

    std::vector<uint8_t> textbank_bytes;
    int string_index = 0;
    for (std::vector<uint8_t> compressed_string : compressed_strings_bytes)
    {
        uint8_t bytes_length = (uint8_t) (compressed_string.size() + 1);
        textbank_bytes.push_back(bytes_length);
        
        textbank_bytes.insert(textbank_bytes.end(), compressed_string.begin(), compressed_string.end());

        string_index++;
        if (string_index % 256 == 0)
        {
            _textbanks.push_back(textbank_bytes);
            textbank_bytes.clear();
        }
    }

    if (!textbank_bytes.empty())
        _textbanks.push_back(textbank_bytes);
}

void TextbanksEncoder::write_to_rom(md::ROM& rom) 
{
    // Write Huffman trees & tree offsets to the ROM
    std::vector<uint16_t> tree_offsets;
    std::vector<uint8_t> tree_data_block;
    uint16_t currentOffset = 0;

    for (uint8_t symbol_id = 0; symbol_id < SYMBOL_COUNT; ++symbol_id)
    {
        HuffmanTree* tree = _trees[symbol_id];
        if (!tree)
        {
            tree_offsets.push_back(0xFFFF);
            continue;
        }

        std::vector<uint8_t> treeSymbols = tree->getSymbolBytes();
        tree_data_block.insert(tree_data_block.end(), treeSymbols.begin(), treeSymbols.end());

        std::vector<uint8_t> treeNodes = tree->getNodeBytes();
        tree_data_block.insert(tree_data_block.end(), treeNodes.begin(), treeNodes.end());

        currentOffset += (uint16_t)treeSymbols.size();
        tree_offsets.push_back(currentOffset);
        currentOffset += (uint16_t)treeNodes.size();
    }

    uint32_t current_address = 0x23D60;
    for (uint16_t treeOffset : tree_offsets)
    {
        rom.set_word(current_address, treeOffset);
        current_address += 0x2;
    }

    rom.set_bytes(0x23E38, tree_data_block);
    if (tree_data_block.size() > 0x2469C - 0x23E38)
        std::cerr << "ERROR : NEW HUFFMAN TREE DATA SIZE IS " << (tree_data_block.size() - (0x2469C - 0x23E38)) << " BYTES BIGGER THAN OLD ONE\n";

    // Write textbanks to the ROM
    current_address = 0x2B27A;
    uint32_t textbank_offset_address = 0x38368;
    for (size_t i = 0; i < _textbanks.size(); i++)
    {
        rom.set_long(textbank_offset_address, current_address);
        textbank_offset_address += 0x4;

        rom.set_bytes(current_address, _textbanks[i]);
        current_address += (uint32_t)_textbanks[i].size();
    }

    // Mark the remaining bytes as "empty" for potential data injection
    rom.mark_empty_chunk(current_address, 0x38368);
}

