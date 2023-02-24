#include "SKSELink.h"
#include "UnStringList.h"

namespace Undaunted
{
#ifndef GroupListdef
#define GroupListdef
	class GroupMember {
	public:
		std::uint32_t FormId;
		std::string BountyType;
		RE::TESObjectREFR* objectRef;

		RE::BSFixedString ModelFilepath;

		int IsComplete();
		bool isComplete = false;
		void PreBounty();
		void PostBounty();
	};

	class GroupList {
	public:
		std::string questText;
		GroupMember* data;
		std::uint32_t minLevel;
		std::uint32_t maxLevel;
		std::uint32_t length;
		UnStringlist Tags;
		GroupList* AddItem(GroupMember item);
		GroupList* SwapItem(int first, int second);
		void SetGroupMemberComplete(std::uint32_t id);
	};

#endif
}
