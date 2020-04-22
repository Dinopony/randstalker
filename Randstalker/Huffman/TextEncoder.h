#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>

#include "HuffmanTree.h"
#include "Symbols.h"
#include "../MegadriveTools/MdRom.h"

class TextEncoder 
{
public:

    TextEncoder(md::ROM& rom, const std::vector<std::string>& textlines, const std::vector<HuffmanTree*>& trees)
    {
        _trees = trees;
        this->produceTextbanks(textlines);
        this->writeToROM(rom);
    }

    TextEncoder(md::ROM& rom, const std::vector<std::string>& textlines)
    {
        this->makeTrees(textlines);
        this->produceTextbanks(textlines);
        this->writeToROM(rom);
    }

    void countSymbols(const std::vector<std::string>& textlines, std::map<uint8_t, std::vector<SymbolCount>>& sortedSymbolCounts)
    {
        uint32_t symbolCounts[SYMBOL_COUNT][SYMBOL_COUNT];
        uint8_t previousSymbol = 0x55;

        for (uint8_t i = 0; i < SYMBOL_COUNT; ++i)
            for (uint8_t j = 0; j < SYMBOL_COUNT; ++j)
                symbolCounts[i][j] = 0;

        for (const std::string& textline : textlines)
        {
            // Step 1: Convert textline into LS character table symbols
            std::vector<uint8_t> textlineAsSymbols = textlineToSymbols(textline);
            
            // Step 2: Count symbols in this text line
            uint8_t previousSymbol = 0x55;
            for (uint8_t symbol : textlineAsSymbols)
            {
                symbolCounts[previousSymbol][symbol]++;
                previousSymbol = symbol;
            }
        }

        // Step 3: For each symbol, sort counts by descending order to find the most common following symbols
        for (uint8_t i = 0; i < SYMBOL_COUNT; ++i)
        {
            std::vector<SymbolCount>& nextSymbolCounts = sortedSymbolCounts[i];
            for (uint8_t j = 0; j < SYMBOL_COUNT; ++j)
                if(symbolCounts[i][j])
                    nextSymbolCounts.push_back(SymbolCount(j, symbolCounts[i][j]));
            std::sort(nextSymbolCounts.begin(), nextSymbolCounts.end());
        }
    }

    void makeTrees(const std::vector<std::string>& textlines)
    {
        std::map<uint8_t, std::vector<SymbolCount>> sortedSymbolCounts;
        this->countSymbols(textlines, sortedSymbolCounts);

        for (uint8_t symbolID = 0; symbolID < SYMBOL_COUNT; symbolID++)
        {
            const std::vector<SymbolCount>& nextSymbolCounts = sortedSymbolCounts[symbolID];
            if (nextSymbolCounts.empty())
            {
                _trees.push_back(nullptr);
                continue;
            }
            
            _trees.push_back(new HuffmanTree(nextSymbolCounts));
        }
    }

    std::vector<uint8_t> textlineToSymbols(const std::string& textline)
    {
        std::vector<uint8_t> textlineAsSymbols;

        for (size_t i = 0; i < textline.length(); ++i)
        {
            for (uint8_t symbolID = 0; symbolID < SYMBOL_COUNT; symbolID++)
            {
                char symbolAsChar = Symbols::TABLE[symbolID];
                char currentCharacter = textline[i];

                if (currentCharacter == symbolAsChar)
                {
                    textlineAsSymbols.push_back(symbolID);
                    break;
                }

                if (symbolID + 1 == SYMBOL_COUNT)
                {
                    std::cerr << "Current character '" << textline[i] << "' is not recognized as the beginning of a known symbol, and will be ignored.\n";
                    ++i;
                }
            }
        }

        textlineAsSymbols.push_back(0x55);
        return textlineAsSymbols;
    }


    void produceTextbanks(const std::vector<std::string>& textlines)
    {
        std::vector<std::vector<uint8_t>> compressedTextlinesBytes;
        for (const std::string& textline : textlines)
        {
            std::vector<uint8_t> textlineAsSymbols = textlineToSymbols(textline);

            uint8_t previousSymbol = 0x55;
            Bitfield bits;
            for (uint8_t symbol : textlineAsSymbols)
            {
                try
                {
                    bits += _trees[previousSymbol]->encode(symbol);
                    previousSymbol = symbol;
                } 
                catch (std::out_of_range&)
                {
                    std::cerr << "Symbol '" << Symbols::TABLE[symbol] << "' cannot be after a '" << Symbols::TABLE[previousSymbol] << "' and will be ignored.\n";
                }
            }
            while (bits.size() % 8 != 0)
                bits.add(0);

            compressedTextlinesBytes.push_back(bits.toValues());
            //std::cout << "Textline '" << textline << "' encoded as " << bits.toValues().size() << " bytes\n";
        }

        std::vector<uint8_t> textbankBytes;
        int stringIndex = 0;
        for (std::vector<uint8_t> compressedTextline : compressedTextlinesBytes)
        {
            uint8_t stringBytesLength = (uint8_t) (compressedTextline.size() + 1);
            textbankBytes.push_back(stringBytesLength);
            textbankBytes.insert(textbankBytes.end(), compressedTextline.begin(), compressedTextline.end());

            stringIndex++;
            if (stringIndex % 256 == 0)
            {
                _textbanks.push_back(textbankBytes);
                textbankBytes.clear();
            }
        }

        if (!textbankBytes.empty())
            _textbanks.push_back(textbankBytes);
    }

    void writeToROM(md::ROM& rom) 
    {
        std::cout << "Outputting text to ROM...\n";

        // Write Huffman trees & tree offsets to the ROM
        std::vector<uint16_t> treeOffsets;
        std::vector<uint8_t> globalTreeDataBlock;
        uint16_t currentOffset = 0;

        for (uint8_t symbolID = 0; symbolID < SYMBOL_COUNT; ++symbolID)
        {
            HuffmanTree* tree = _trees[symbolID];
            if (!tree)
            {
                treeOffsets.push_back(0xFFFF);
                continue;
            }

            std::vector<uint8_t> treeSymbols = tree->getSymbolBytes();
            globalTreeDataBlock.insert(globalTreeDataBlock.end(), treeSymbols.begin(), treeSymbols.end());

            std::vector<uint8_t> treeNodes = tree->getNodeBytes();
            globalTreeDataBlock.insert(globalTreeDataBlock.end(), treeNodes.begin(), treeNodes.end());

            currentOffset += (uint16_t)treeSymbols.size();
            treeOffsets.push_back(currentOffset);
            currentOffset += (uint16_t)treeNodes.size();
        }

        uint32_t currentWritingAddress = 0x23D60;
        for (uint16_t treeOffset : treeOffsets)
        {
            rom.setWord(currentWritingAddress, treeOffset);
            currentWritingAddress += 0x2;
        }

        rom.setBytes(0x23E38, globalTreeDataBlock);
        if (globalTreeDataBlock.size() > 0x2469C - 0x23E38)
            std::cerr << "ERROR : NEW HUFFMAN TREE DATA SIZE IS " << (globalTreeDataBlock.size() - (0x2469C - 0x23E38)) << " BYTES BIGGER THAN OLD ONE\n";

        // Write textbanks to the ROM
        uint32_t writingAddress = 0x2B27A;
        uint32_t textbankOffsetWritingAddress = 0x38368;
        for (size_t i = 0; i < _textbanks.size(); i++)
        {
            rom.setLong(textbankOffsetWritingAddress, writingAddress);
            textbankOffsetWritingAddress += 0x4;

            rom.setBytes(writingAddress, _textbanks[i]);
            writingAddress += _textbanks[i].size();
        }

        int sizeDiff = 0x38368 - writingAddress;
        std::cout << "Saved " << sizeDiff << " bytes on textbanks.\n";

        while (writingAddress < 0x38368)
        {
            rom.setByte(writingAddress, 0xFF);
            writingAddress++;
        }
    }

private:
    std::vector<HuffmanTree*> _trees;
    std::vector<std::vector<uint8_t>> _textbanks;
};
