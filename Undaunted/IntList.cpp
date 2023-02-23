#include "IntList.h"

namespace Undaunted
{
	IntList* IntList::AddItem(std::uint32_t item)
	{
		IntList* currentlist = this;
		IntList newlist = IntList();
		newlist.length = currentlist->length + 1;
		newlist.data = new std::uint32_t[newlist.length];
		for (int i = 0; i < currentlist->length; i++)
		{
			newlist.data[i] = currentlist->data[i];
		}
		newlist.data[currentlist->length] = item;
		currentlist->data = newlist.data;
		currentlist->length = newlist.length;
		return currentlist;
	}
}