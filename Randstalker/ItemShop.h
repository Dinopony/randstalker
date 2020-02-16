#pragma once

#include "AbstractItemSource.h"

class ItemShop 
{
public:
	ItemShop() :
		_itemSourcesInShop(),
		_itemValueMulitplier(1.f)
	{}

	void addItemSource(AbstractItemSource* itemSource) { _itemSourcesInShop.push_back(itemSource); }
	
	bool isItemAlreadyInShop(Item* item)
	{
		for (AbstractItemSource* source : _itemSourcesInShop)
			if (source->getItem() == item)
				return true;
		return false;
	}

private:
	float _itemValueMulitplier;
	std::vector<AbstractItemSource*> _itemSourcesInShop;
};