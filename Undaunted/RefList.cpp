#include "RefList.h"
namespace Undaunted
{
	RefList* RefList::AddItem(Ref item)
	{
		RefList* currentlist = this;
		RefList newlist = RefList();
		newlist.length = currentlist->length + 1;
		newlist.data = new Ref[newlist.length];
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