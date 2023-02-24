#include "SKSELink.h"
#include <WorldCellList.h>
#include <GroupList.h>

namespace Undaunted
{
#ifndef BountyListdef
#define BountyListdef
	class Bounty {
	public:
		RE::TESObjectREFR* xmarkerref = NULL;
		RE::BGSMessage* bountymessageref = NULL;
		GroupList bountygrouplist;
		WorldCell bountyworldcell;
		int bountywave = 0;
	};

	class BountyList {
	public:
		Bounty* data;
		std::uint32_t length;
		BountyList* AddItem(Bounty item);
	};
#endif
}
