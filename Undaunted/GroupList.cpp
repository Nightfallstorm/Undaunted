#include "IntList.h"
#include "GroupList.h"
#include "LocationUtils.h"
#include "BountyManager.h"

namespace Undaunted
{
	GroupList* GroupList::AddItem(GroupMember item)
	{
		GroupList* currentlist = this;
		GroupList newlist = GroupList();
		newlist.length = currentlist->length + 1;
		newlist.data = new GroupMember[newlist.length];
		for (std::uint32_t i = 0; i < currentlist->length; i++)
		{
			newlist.data[i] = currentlist->data[i];
		}
		newlist.data[currentlist->length] = item;
		currentlist->data = newlist.data;
		currentlist->length = newlist.length;
		return currentlist;
	}

	GroupList* GroupList::SwapItem(int first, int second)
	{
		GroupMember First = this->data[first];
		GroupMember Second = this->data[second];

		this->data[first] = Second;
		this->data[second] = First;
		return this;
	}

	void GroupList::SetGroupMemberComplete(std::uint32_t id)
	{
		for (std::uint32_t i = 0; i < this->length; i++)
		{
			if (this->data[i].objectRef != NULL)
			{
				if (id == this->data[i].objectRef->formID)
				{
					this->data[i].isComplete = true;
					logger::info("SetGroupMemberComplete: {:x} ", id);
					return;
				}
			}
		}
	}

	int GroupMember::IsComplete()
	{
		const char* type = this->BountyType.c_str();
		if (strcmp(type, "PHYSICSSCRIPTED") == 0 || strcmp(type, "SCRIPTED") == 0 || strcmp(type, "ENEMY") == 0 || strcmp(type, "SCRIPTEDDOOR") == 0)
		{
			return isComplete;
		}
		//In case I don't know what's happenening just mark it as done.
		//Doing so due to an error from wyongcan2019 where the Ally type became lowercase for some reason.
		return 1;
	}

	void GroupMember::PreBounty()
	{

	}

	void GroupMember::PostBounty()
	{
		const char* type = this->BountyType.c_str();
		if (strcmp(type, "EndEffect") == 0)
		{
			RE::TESForm* spawnForm = RE::TESForm::LookupByID(this->FormId);
			if (spawnForm == NULL)
			{
				logger::info("Failed to Spawn. Form Invalid");
			}
			else
			{
//				PlaceAtMe_Native(BountyManager::getInstance()->_registry, 1, BountyManager::getInstance()->xmarkerref, spawnForm, 1, false, false);
			}
		}
	}
}
