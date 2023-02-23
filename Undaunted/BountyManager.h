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

		bool BountyUpdate(int BountyID);
		float StartBounty(int BountyID, bool nearby, const char* BountyName, RE::TESObjectREFR* ref, RE::BSFixedString WorldSpaceName, std::string bountyTag = "");
		float restartBounty(int BountyID, const char* BountyName);
		void ClearBountyData(int BountyID);
		void ResetBountiesRan();
		void AddToDeleteList(RE::TESObjectREFR* ref);
		void ClearDeleteList();

		RefList StartRift(int BountyID, RE::TESObjectREFR* Startpoint);

	};
#endif
}
