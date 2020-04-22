#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include "MegadriveTools/MdRom.h"

#include "HuffmanTree.h"

class TextDecoder
{
public:
    TextDecoder(md::ROM& rom) 
    {
        parseTrees(rom);
        parseAllTextbanks(rom);
    }

    const std::vector<std::string>& getStrings() const { return _strings; };
    const std::vector<HuffmanTree*>& getTrees() const { return _trees; };

private:
    void parseTrees(md::ROM& rom)
    {
        const std::pair<uint32_t, uint32_t> HUFFMAN_TREE_OFFSETS(0x23D60, 0x23E38);
        const std::pair<uint32_t, uint32_t> HUFFMAN_TREES(0x23E38,0x2469C);
        
        std::vector<uint16_t> treesOffsets;
        rom.getDataChunk(HUFFMAN_TREE_OFFSETS.first, HUFFMAN_TREE_OFFSETS.second, treesOffsets);

        std::vector<uint8_t> treesData;
        rom.getDataChunk(HUFFMAN_TREES.first, HUFFMAN_TREES.second, treesData);

        for (uint32_t i = 0; i < treesOffsets.size() ; ++i)
        {
            uint16_t treeOffset = treesOffsets[i];
            if (treeOffset == 0xFFFF)
                _trees.push_back(nullptr);
            else
                _trees.push_back(new HuffmanTree(treesData, treeOffset));
        }
    }

    void parseAllTextbanks(md::ROM& rom)
    {
        const int LINES_PER_BANK = 256;
        const std::vector<uint32_t> TEXTBANKS_OFFSETS = { 0x2B27A, 0x2C29B, 0x2DCC8, 0x2F787, 0x3153E, 0x330AB, 0x34830, 0x36087, 0x377E3, 0x38368 };

        for (uint32_t i = 0; i < TEXTBANKS_OFFSETS.size()-1 ; i++)
        {
            std::vector<uint8_t> data;
            rom.getDataChunk(TEXTBANKS_OFFSETS[i], TEXTBANKS_OFFSETS[i + 1], data);

            uint32_t currentStringOffset = 0;
            for (uint32_t j = 0; j < 256; j++)
            {
                int lineLength = data[currentStringOffset];
                std::string textLine = parseString(data, currentStringOffset+1);
                _strings.push_back(textLine);

//              std::cout << "- " << textLine << std::endl;

                currentStringOffset += lineLength;
                if (currentStringOffset + 1 >= data.size())
                    break;
            }
        }
    }

    std::string parseString(std::vector<uint8_t> data, short offset)
    {
        std::string string;
        uint8_t previousSymbol = 0x55;
        uint32_t stringBitIndex = 0;

        while (true)
        {
            uint8_t symbol = parseNextSymbol(_trees[previousSymbol], data, offset, stringBitIndex);
            if (symbol == 0x55)
                break;

            string += Symbols::TABLE[symbol];
            previousSymbol = symbol;
        }

        return string;
    }

    uint8_t parseNextSymbol(HuffmanTree* huffmanTree, std::vector<uint8_t> data, short offset, uint32_t& stringBitIndex)
    {
        Bitfield bits;

        while (true)
        {
            try
            {
                return huffmanTree->decode(bits);
            } catch (std::out_of_range&) {}

            uint8_t stringByte = data[offset + (stringBitIndex / 8)];
            uint8_t stringBit = (stringByte >> (7 - (stringBitIndex % 8))) & 1;
            bits.add(stringBit);
            stringBitIndex++;
        }

        throw std::exception();
    }

    std::vector<std::string> _strings;
    std::vector<HuffmanTree*> _trees;
};