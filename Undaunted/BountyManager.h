#include "LocationUtils.h"
#include "SpawnUtils.h"
#include "ConfigUtils.h"
#include "BountyList.h"
#include "RefList.h"

namespace Undaunted {
#ifndef BountyManagerdef
#define BountyManagerdef
	class BountyManager {
	public:
		static BountyManager* instance;
		static BountyManager* getInstance();

		RE::BSScript::Internal::VirtualMachine* _registry;
		int isReady = 0;

		BountyList activebounties = BountyList();
		UnDictionary bountiesRan = UnDictionary();
		RefList deleteList = RefList();

		bool BountyUpdate(std::uint32_t BountyID);
		float StartBounty(std::uint32_t BountyID, bool nearby, const char* BountyName, RE::TESObjectREFR* ref, RE::BSFixedString WorldSpaceName, std::string bountyTag = "");
		float restartBounty(std::uint32_t BountyID, const char* BountyName);
		void ClearBountyData(std::uint32_t BountyID);
		void ResetBountiesRan();
		void AddToDeleteList(RE::TESObjectREFR* ref);
		void ClearDeleteList();

		RefList StartRift(std::uint32_t BountyID, RE::TESObjectREFR* Startpoint);

	};
#endif
}
