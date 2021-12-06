#pragma once

#include <cstdint>
#include <algorithm>

#include "../bitfield.hpp"
#include "tree_node.hpp"
#include "symbols.hpp"

class HuffmanTree
{
public:
    // Constructor to build a Huffman tree from ROM data
    HuffmanTree(const std::vector<uint8_t>& data, uint16_t treeOffset) :
        _topNode(new HuffmanTreeNode()),
        _encodingTable(nullptr)
    {
        HuffmanTreeNode* currentNode = _topNode;
        int currentSymbol = 0;

        for (int treeBitCounter = 0 ; true; ++treeBitCounter)
        {
            uint8_t byte = data[treeOffset + (treeBitCounter / 8)];
            uint8_t bit = (byte >> (7 - (treeBitCounter%8))) & 1;

            if (bit == 0)
            {
                // Branch node case
                currentNode->setLeftChild(new HuffmanTreeNode());
                currentNode = currentNode->getLeftChild();
            }
            else
            {
                // Leaf node case
                uint8_t symbol = data[treeOffset - currentSymbol - 1];
                currentNode->setSymbol(symbol);
                currentSymbol++;

                // If current node's parent has a right child, it means we completed this branch of the tree
                // and we need to get back at a higher level
                while (currentNode->getParent() && currentNode->getParent()->getRightChild())
                {
                    currentNode = currentNode->getParent();
                }

                // If we got back to top node, it means the tree is finished
                if (currentNode == _topNode)
                    break;

                currentNode->getParent()->setRightChild(new HuffmanTreeNode());
                currentNode = currentNode->getParent()->getRightChild();
            }
        }

        this->buildEncodingTable();
    }

    // Constructor to build a Huffman tree from a list of symbol counts
    HuffmanTree(const std::vector<SymbolCount>& nextSymbolCounts)
    {
        _topNode = new HuffmanTreeNode();
        this->buildNodesFromArray(_topNode, nextSymbolCounts);
        this->buildEncodingTable();
    }

    ~HuffmanTree()
    {
        delete _topNode;

        for (uint8_t i = 0; i < SYMBOL_COUNT; ++i)
            if (_encodingTable[i])
                delete _encodingTable[i];
        delete[] _encodingTable;
    }

    void buildNodesFromArray(HuffmanTreeNode* fromNode, const std::vector<SymbolCount>& nextSymbolCounts)
    {
        if (nextSymbolCounts.size() >= 2)
        {
            // branch node
            uint32_t halfWeightSum = 0;
            for (const SymbolCount& sc : nextSymbolCounts)
                halfWeightSum += sc.getCount();
            halfWeightSum /= 2;

            uint32_t currentWeightSum = 0;
            uint32_t indexAfterHalfWeight = 0;
            while (indexAfterHalfWeight < nextSymbolCounts.size())
            {
                currentWeightSum += nextSymbolCounts[indexAfterHalfWeight].getCount();
                ++indexAfterHalfWeight;

                if (currentWeightSum >= halfWeightSum)
                    break;
            }

            if (indexAfterHalfWeight == nextSymbolCounts.size())
                indexAfterHalfWeight = (uint32_t)(nextSymbolCounts.size() / 2);

            std::vector<SymbolCount> leftSide(nextSymbolCounts.begin(), nextSymbolCounts.begin() + indexAfterHalfWeight);
            std::vector<SymbolCount> rightSide(nextSymbolCounts.begin() + indexAfterHalfWeight, nextSymbolCounts.end());

            HuffmanTreeNode* leftChild = new HuffmanTreeNode();
            fromNode->setLeftChild(leftChild);
            HuffmanTreeNode* rightChild = new HuffmanTreeNode();
            fromNode->setRightChild(rightChild);

            this->buildNodesFromArray(leftChild, leftSide);
            this->buildNodesFromArray(rightChild, rightSide);
        }
        else
        {
            // leaf node
            fromNode->setSymbol(nextSymbolCounts[0].getSymbol());
        }
    }

    const uint8_t* decode(const Bitfield& bits) const
    {
        HuffmanTreeNode* currentNode = _topNode;
        for (uint16_t i=0 ; i<bits.size() ; ++i)
        {
            if (bits.get(i))
                currentNode = currentNode->getRightChild();
            else
                currentNode = currentNode->getLeftChild();
        }
        
        if(!currentNode->isLeaf())
            return nullptr;

        return &currentNode->getSymbol();
    }

    const Bitfield& encode(uint8_t symbol) const
    {
        Bitfield* bits = _encodingTable[symbol];
        if (!bits)
            throw std::out_of_range("Symbol not represented in HuffmanTree.");
        return *bits;
    }

    std::vector<uint8_t> getSymbolBytes()
    {
        std::vector<uint8_t> symbolBytes;

        HuffmanTreeNode* currentNode = _topNode;
        while (currentNode)
        {
            if (currentNode->isLeaf())
            {
                symbolBytes.emplace_back(currentNode->getSymbol());

                while (currentNode->getParent() && currentNode->getParent()->getRightChild() == currentNode)
                {
                    currentNode = currentNode->getParent();
                }

                // If we got back to top node, it means the tree is finished
                if (currentNode == _topNode)
                    currentNode = nullptr;
                else
                    currentNode = currentNode->getParent()->getRightChild();
            } 
            else
            {
                currentNode = currentNode->getLeftChild();
            }
        }

        std::reverse(symbolBytes.begin(), symbolBytes.end());
        return symbolBytes;
    }

    std::vector<uint8_t> getNodeBytes()
    {
        Bitfield nodeBits;

        HuffmanTreeNode* currentNode = _topNode;
        while (currentNode)
        {
            if (currentNode->isLeaf())
            {
                nodeBits.add(1);

                while (currentNode->getParent() && currentNode->getParent()->getRightChild() == currentNode)
                {
                    currentNode = currentNode->getParent();
                }

                // If we got back to top node, it means the tree is finished
                if (currentNode == _topNode)
                    currentNode = nullptr;
                else
                    currentNode = currentNode->getParent()->getRightChild();
            }
            else
            {
                nodeBits.add(0);
                currentNode = currentNode->getLeftChild();
            }
        }

        while (nodeBits.size() % 8 != 0)
            nodeBits.add(0);

        return nodeBits.to_bytes();
    }

private:
    void buildEncodingTable()
    {
        _encodingTable = new Bitfield* [SYMBOL_COUNT];
        for (uint32_t i=0; i< SYMBOL_COUNT; ++i)
            _encodingTable[i] = nullptr;

        this->buildEncodingTable(_topNode, Bitfield());
    }

    void buildEncodingTable(HuffmanTreeNode* fromNode, Bitfield currentBits)
    {
        if (fromNode->isLeaf())
        {
            _encodingTable[fromNode->getSymbol()] = new Bitfield(currentBits);
        }
        else
        {
            this->buildEncodingTable(fromNode->getLeftChild(), currentBits + '0');
            this->buildEncodingTable(fromNode->getRightChild(), currentBits + '1');
        }
    }

    HuffmanTreeNode* _topNode;
    Bitfield** _encodingTable;
};
