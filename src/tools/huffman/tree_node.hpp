#pragma once

#include <cstdint>

class HuffmanTreeNode
{
private:
    HuffmanTreeNode* _leftChild;
    HuffmanTreeNode* _rightChild;
    HuffmanTreeNode* _parent;
    uint8_t _symbol;

public:
    // Branch constructor
    HuffmanTreeNode() :
        _leftChild(nullptr),
        _rightChild(nullptr),
        _parent(nullptr),
        _symbol(0x00)
    {}

    ~HuffmanTreeNode()
    {
        if (_leftChild)
            delete _leftChild;
        if (_rightChild)
            delete _rightChild;
    }

    void setSymbol(uint8_t symbol) { _symbol = symbol; }
    const uint8_t& getSymbol() const { return _symbol; }

    void setLeftChild(HuffmanTreeNode* node)
    { 
        _leftChild = node;
        node->_parent = this; 
    }

    void setRightChild(HuffmanTreeNode* node) 
    {
        _rightChild = node; 
        node->_parent = this; 
    }

    HuffmanTreeNode* getLeftChild() const { return _leftChild; }
    HuffmanTreeNode* getRightChild() const { return _rightChild; }
    HuffmanTreeNode* getParent() const { return _parent; }

    bool isLeaf() const { return !_leftChild && !_rightChild; }
};