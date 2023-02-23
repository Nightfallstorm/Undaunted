#include "MyPlugin.h"
#include <StartupManager.h>
#include <algorithm>
#include <string>
#include "BountyManager.h"
#include "UnStringList.h"

using Bounty = Undaunted::Bounty;
using BountyManager = Undaunted::BountyManager;
using GroupMember = Undaunted::GroupMember;
using GroupList = Undaunted::GroupList;
using WorldCell = Undaunted::WorldCell;

namespace UndauntedPapyrus {
	std::uint32_t CreateBounty(RE::StaticFunctionTag* base) {
		int result = BountyManager::getInstance()->activebounties.length;
		logger::info("hook_CreateBounty result: %08X", result);
		Bounty newBounty = Bounty();
		BountyManager::getInstance()->activebounties.AddItem(newBounty);
		return result;
	}

	// Triggers a new bounty stage to start.
	float StartBounty(RE::StaticFunctionTag* base, std::uint32_t BountyId, bool nearby) {
		logger::info("hook_StartBounty BountyId: %08X", BountyId);
		BountyManager::getInstance()->StartBounty(BountyId,nearby, "",NULL,"");
		return 2;
	}

	// Triggers a new Elite bounty stage to start.
	float StartEliteBounty(RE::StaticFunctionTag* base, std::uint32_t BountyId, bool nearby)
	{
		logger::info("hook_StartEliteBounty BountyId: %08X", BountyId);
		BountyManager::getInstance()->StartBounty(BountyId, nearby, "", NULL, "", "ELITE");
		return 2;
	}

	// Triggers a new bounty stage to start with a certain name.
	float StartNamedBounty(RE::StaticFunctionTag* base, std::uint32_t BountyId, bool nearby, RE::BSFixedString bountyName)
	{
		logger::info("hook_StartNamedBounty BountyId: %08X", BountyId);
		BountyManager::getInstance()->StartBounty(BountyId, nearby, bountyName.c_str(), NULL, "");
		return 2;
	}


	// Triggers a new bounty stage to start with a certain name.
	float RestartNamedBounty(RE::StaticFunctionTag* base, std::uint32_t BountyId, RE::BSFixedString bountyName)
	{
		logger::info("hook_restartNamedBounty BountyId: %08X", BountyId);
		BountyManager::getInstance()->restartBounty(BountyId,bountyName.c_str());
		return 2;
	}

	
	float StartNamedBountyNearRef(RE::StaticFunctionTag* base, std::uint32_t BountyId, bool nearby, RE::BSFixedString bountyName, RE::TESObjectREFR* ref, RE::BSFixedString WorldSpaceName)
	{
		logger::info("hook_StartNamedBountyNearRef BountyId: %08X", BountyId);
		BountyManager::getInstance()->StartBounty(BountyId, nearby, bountyName.c_str(), ref, WorldSpaceName);
		return 2;
	}


	// Fill out the WorldList, this checks the loaded world cells and finds the persistant reference cells.
	// This takes a while so we only do this once at the start
	bool InitSystem(RE::StaticFunctionTag* base, std::uint32_t playerLevel)
	{
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		
		for (auto mod : RE::TESDataHandler::GetSingleton()->files) {
			logger::info("Listing Mods: %s , %i, %i", mod->GetFilename(), mod->GetCompileIndex(), mod->GetSmallFileCompileIndex());
		}

		Undaunted::LoadSettings();
		Undaunted::LoadGroups();
		Undaunted::ShuffleGroupLibary();
		Undaunted::BuildWorldList();
		Undaunted::InitBakedRiftStartMarkers();
		Undaunted::SetPlayerLevel(playerLevel);
		BountyManager::getInstance()->isReady = 2;
		logger::info("ReadyState: %i ", BountyManager::getInstance()->isReady);
		return true;
	}

	// A check to see if the Init call has finished
	std::uint32_t isSystemReady(RE::StaticFunctionTag* base)
	{
		return BountyManager::getInstance()->isReady;
	}

	bool ClaimStartupLock(RE::StaticFunctionTag* base)
	{
		logger::info("ReadyState: %i ", BountyManager::getInstance()->isReady);
		if (BountyManager::getInstance()->isReady == 0)
		{
			BountyManager::getInstance()->isReady = 1;
			return true;
		}
		return false;
	}

	// Check if all the bounty objectives have been complete
	bool isBountyComplete(RE::StaticFunctionTag* base, std::uint32_t BountyId) {
		logger::info("Starting Bounty Check %08X ", BountyId);
		if (BountyManager::getInstance()->activebounties.length < BountyId)
		{
			return false;
		}
		return BountyManager::getInstance()->BountyUpdate(BountyId);
	}
	// Tell the bounty system that this object should be marked as complete
	void SetGroupMemberComplete(RE::StaticFunctionTag* base, RE::TESObjectREFR* taget)
	{
		logger::info("hook_SetGroupMemberComplete");
		//This actually works as it's using the object reference. The object should only be in one bounty.
		for (int i = 0; i < BountyManager::getInstance()->activebounties.length; i++)
		{
			BountyManager::getInstance()->activebounties.data[i].bountygrouplist.SetGroupMemberComplete(taget->formID);
		}	
	}

	RE::BSFixedString GetBountyName(RE::StaticFunctionTag* base, std::uint32_t BountyId) {
		logger::info("Starting Bounty Check");
		if (BountyManager::getInstance()->activebounties.length > BountyId)
		{
			return BountyManager::getInstance()->activebounties.data[BountyId].bountygrouplist.questText.c_str();
		}
		return "NO BOUNTIES";
	}	

	// Pass the reference to the XMarker that we use as the quest target and the target of the placeatme calls
	bool SetXMarker(RE::StaticFunctionTag* base, std::uint32_t BountyId, RE::TESObjectREFR* marker) {
		logger::info("hook_SetXMarker %08X ", BountyId);
		BountyManager::getInstance()->activebounties.data[BountyId].xmarkerref = marker;
		return true;
	}

	// Pass the reference to the quest objective message. This allows us to edit it from the code.
	bool SetBountyMessageRef(RE::StaticFunctionTag* base, std::uint32_t BountyId, RE::BGSMessage* ref)
	{
		logger::info("hook_SetBountyMessageRef %08X ", BountyId);
		BountyManager::getInstance()->activebounties.data[BountyId].bountymessageref = ref;
		return true;
	}

	// For reasons as yet unknown, some of the regions in memory cause crash to desktops. We have to skip processing these. Hoping to fix this.
	bool AddBadRegion(RE::StaticFunctionTag* base, std::uint32_t region)
	{
		Undaunted::AddBadRegionToConfig(region);
		return true;
	}

	// Process the Group header line. We return the groups position which we can use to add to later.
	std::uint32_t AddGroup(RE::StaticFunctionTag* base, RE::BSFixedString questText, RE::BSFixedString modRequirement, std::uint32_t minLevel, std::uint32_t maxLevel, std::uint32_t playerLevel)
	{
		//Mod required is not loaded

		if (!RE::TESDataHandler::GetSingleton()->LookupModByName(modRequirement))
		{
			logger::info("%s: Mod %s is not loaded", questText.c_str(), modRequirement.c_str());
			return -1;
		}
		return Undaunted::AddGroup(questText.c_str(),minLevel,maxLevel, Undaunted::UnStringlist());
	}

	// Add a member to a group.
	void AddMembertoGroup(RE::StaticFunctionTag* base, std::uint32_t groupid, std::uint32_t member, RE::BSFixedString BountyType, RE::BSFixedString ModelFilepath)
	{
		GroupMember newMember = GroupMember();
		newMember.FormId = member;
		newMember.BountyType = BountyType;
		newMember.ModelFilepath = ModelFilepath;
		AddMembertoGroup(groupid, newMember);
	}

	// Given a mod name and a FormId - load order, return the actualy form id
	std::uint32_t GetModForm(RE::StaticFunctionTag* base, RE::BSFixedString ModName, std::uint32_t FormId)
	{
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		const RE::TESFile* modInfo = dataHandler->LookupModByName(ModName.c_str());
		if (modInfo != NULL)
		{
			FormId = (modInfo->compileIndex << 24) + FormId;
			if (modInfo->IsFormInMod(FormId))
			{
				return FormId;
			}
			else
			{
				logger::info("FormId  %08X Not Found in %s", FormId, ModName.c_str());
				return std::uint32_t();
			}
		}
		logger::info("Mod Not Found: %s", ModName.c_str());
		return std::uint32_t();
	}

	// Return a reward form. We seed the random data with the offset + time so that we can spawn multiple things at once.
	RE::TESForm* SpawnRandomReward(RE::StaticFunctionTag* base, std::uint32_t rewardOffset, std::uint32_t playerlevel)
	{
		logger::info("hook_SpawnRandomReward");
		RE::FormID rewardid = Undaunted::GetReward(rewardOffset, playerlevel);
		logger::info("RewardID: %08X", rewardid);
		RE::TESForm* spawnForm = RE::TESForm::LookupByID(rewardid);
		return spawnForm;
	}

	// Pass in a config value
	void SetConfigValue(RE::StaticFunctionTag* base, RE::BSFixedString key, RE::BSFixedString value)
	{
		Undaunted::AddConfigValue(key.c_str(), value.c_str());
	}
	// Returns an int that is in the config
	std::uint32_t GetConfigValueInt(RE::StaticFunctionTag* base, RE::BSFixedString key)
	{
		return Undaunted::GetConfigValueInt(key.c_str());
	}
	
	RE::BSFixedString GetPlayerWorldSpaceName(RE::StaticFunctionTag* base)
	{
		logger::info("hook_GetPlayerWorldSpaceName");
		return Undaunted::GetCurrentWorldspaceName().c_str();
	}


	bool isPlayerInWorldSpace(RE::StaticFunctionTag* base, RE::BSFixedString worldspacename)
	{
		logger::info("hook_isPlayerInWorldSpace");
		return _stricmp(Undaunted::GetCurrentWorldspaceName().c_str(), worldspacename.c_str()) == 0;
	}

	// Currently unused, checks if the object reference is in the current bounty.
	bool IsGroupMemberUsed(RE::StaticFunctionTag* base, RE::TESObjectREFR* target)
	{
		logger::info("hook_IsGroupMemberUsed");
		/*
		//Is this reference in the current bounty? If it isn't we can get rid of it.
		for (int i = 0; i < BountyManager::getInstance()->bountygrouplist.length; i++)
		{
			if (BountyManager::getInstance()->bountygrouplist.data[i].objectRef->formID == target->formID)
			{
				return true;
			}
		}*/
		return false;
	}

	// The player has fast travelled. This causes cells which are marked to reset to reset.
	// This means we can take all bounties off the blacklist.
	void PlayerTraveled(RE::StaticFunctionTag* base, float distance)
	{
		logger::info("hook_PlayerTraveled %f hours", distance);
		//If a bounty is running and we fast travel then clean it up.
		//If it hasn't started yet we don't need to worry.
		if (distance > 1.5f)
		{
			BountyManager::getInstance()->ResetBountiesRan();
		}
	}

	// Triggered when leaving a microdungeon. Tells all the doors that the microdungeon has been completed.
	void SetScriptedDoorsComplete(RE::StaticFunctionTag* base)
	{
		logger::info("Starting hook_SetBountyComplete");
		for (int i = 0; i < BountyManager::getInstance()->activebounties.length; i++)
		{
			for (int j = 0; j < BountyManager::getInstance()->activebounties.data[i].bountygrouplist.length; j++)
			{
				const char* type = BountyManager::getInstance()->activebounties.data[i].bountygrouplist.data[j].BountyType.c_str();
				if (strcmp(type, "SCRIPTEDDOOR") == 0)
				{
					BountyManager::getInstance()->activebounties.data[i].bountygrouplist.data[j].isComplete = true;
				}
			}
		}
	}

	// Returns the references of all the spawned objects of a certain type
	std::vector<RE::TESObjectREFR*> GetBountyObjectRefs(RE::StaticFunctionTag* base, std::uint32_t BountyId, RE::BSFixedString bountyType)
	{
		logger::info("hook_GetBountyObjectRefs %08X ", BountyId);
		std::string type = bountyType.c_str();
		std::transform(type.begin(), type.end(), type.begin(), ::toupper);

		std::vector<RE::TESObjectREFR*> resultsarray = std::vector<RE::TESObjectREFR*>();

		if (BountyManager::getInstance()->activebounties.length < BountyId)
		{
			return resultsarray;
		}

		if (strcmp("DELETE", type.c_str()) == 0)
		{
			logger::info("hook_GetBountyObjectRefs DELETE");
			for (int i = 0; i < BountyManager::getInstance()->deleteList.length; i++)
			{
				resultsarray.push_back(BountyManager::getInstance()->deleteList.data[i].objectRef);
			}
			BountyManager::getInstance()->ClearDeleteList();
			return resultsarray;
		}

		for (int i = 0; i < BountyManager::getInstance()->activebounties.data[BountyId].bountygrouplist.length; i++)
		{
			if (strcmp(BountyManager::getInstance()->activebounties.data[BountyId].bountygrouplist.data[i].BountyType.c_str(), type.c_str()) == 0 ||
				strcmp(bountyType.c_str(),"ALL") == 0)
			{
				if (BountyManager::getInstance()->activebounties.data[BountyId].bountygrouplist.data[i].objectRef != NULL)
				{
					resultsarray.push_back(BountyManager::getInstance()->activebounties.data[BountyId].bountygrouplist.data[i].objectRef);
				}
			}
		}
		logger::info("hook_GetBountyObjectRefs %08X Success", BountyId);
		return resultsarray;
	}



	RE::BSFixedString GetRandomBountyName(RE::StaticFunctionTag* base)
	{
		logger::info("hook_GetRandomBountyName");
		GroupList list = Undaunted::GetRandomGroup();
		RE::BSFixedString result = RE::BSFixedString();
		result = list.questText.c_str();
		return result;
	}

	void CaptureCellData(RE::StaticFunctionTag* base)
	{
		logger::info("hook_CaptureCellData");
		Undaunted::CaptureArea();		
	}

	std::vector<RE::TESObjectREFR*> SpawnRift(RE::StaticFunctionTag* base, std::uint32_t BountyId, RE::TESObjectREFR* Startpoint)
	{
		logger::info("hook_SpawnRift");
		Undaunted::RefList results = BountyManager::getInstance()->StartRift(BountyId, Startpoint);
		std::vector<RE::TESObjectREFR*> resultsarray = std::vector<RE::TESObjectREFR*>();
		for (int i = 0; i < results.length; i++)
		{
			resultsarray.push_back(results.data[i].objectRef);
		}
		return resultsarray;
	}

	std::vector<float> GetRiftRotations(RE::StaticFunctionTag* base)
	{
		logger::info("hook_GetRiftRotations");
		std::vector<float> resultsarray = std::vector<float>();
		return Undaunted::GetRiftRotations();
	}

	
	std::vector<RE::TESObjectREFR*> GetRiftReferences(RE::StaticFunctionTag* base)
	{
		logger::info("hook_GetRiftReferences");
		Undaunted::RefList results = Undaunted::GetCurrentRiftRefs();
		std::vector<RE::TESObjectREFR*> resultsarray = std::vector<RE::TESObjectREFR*>();
		for (int i = 0; i < results.length; i++)
		{
			resultsarray.push_back(results.data[i].objectRef);
		}
		return resultsarray;
	}

	RE::TESObjectREFR* GetRandomRiftStartMarker(RE::StaticFunctionTag* base)
	{
		logger::info("hook_GetRandomRiftStartMarker");		
		return Undaunted::GetRandomBakedRiftStartMarker();
	}

	RE::TESObjectREFR* SpawnMonsterInCell(RE::StaticFunctionTag* base, std::uint32_t formid)
	{
		logger::info("Finding all Rift Battle Markers");
		Undaunted::RefList RiftBattleMarkers = Undaunted::RefList();
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		const RE::TESFile* modInfo = dataHandler->LookupModByName("Undaunted.esp");
		if (modInfo == NULL)
		{
			logger::info("Can't find Undaunted.esp. What the hell?");
			return NULL;
		}
		std::uint32_t FormId = (modInfo->GetCompileIndex() << 24) + 1085677; //041090ED - 01_Undaunted_RiftBattleMarker
		auto player = RE::PlayerCharacter::GetSingleton();
		WorldCell wcell = WorldCell();
		wcell.cell = player->parentCell;
		wcell.world = player->GetWorldspace();
		int numberofRefs = papyrusCell::GetNumRefs(wcell.cell, 0);
		for (int i = 0; i < numberofRefs; i++)
		{
			RE::TESObjectREFR* ref = papyrusCell::GetNthRef(wcell.cell, i, 0);
			if (ref != NULL)
			{
				if (ref->formID != NULL)
				{
					if (ref->GetBaseObject()->formID == FormId)
					{
						Undaunted::Ref formref = Undaunted::Ref();
						formref.objectRef = ref;
						RiftBattleMarkers.AddItem(formref);
					}
				}
			}
		}
		logger::info("Spawning groups at each Rift Battle Marker");
		int spawnradius = Undaunted::GetConfigValueInt("BountyEnemyInteriorSpawnRadius");

		for (int i = 0; i < 2; i++)
		{
			GroupList riftlist = Undaunted::GetRandomTaggedGroup("RIFT");
			for (int j = 0; j < riftlist.length; j++)
			{
				Undaunted::SpawnMonsterInCell(BountyManager::getInstance()->_registry, riftlist.data[j].FormId, wcell);
			}
		}
		
		for (int i = 0; i < RiftBattleMarkers.length; i++)
		{
			GroupList riftlist = Undaunted::GetRandomTaggedGroup("RIFT");
			Undaunted::SpawnGroupAtTarget(BountyManager::getInstance()->_registry, riftlist, RiftBattleMarkers.data[i].objectRef, wcell.cell, wcell.world, spawnradius, 1000);
		}

		return NULL;
	}


	bool RegisterFuncs(VM* a_vm)
	{

		BountyManager::getInstance()->_registry = a_vm;
		//General

		BIND(CreateBounty);

		BIND(StartBounty);

		BIND(StartEliteBounty);		

		BIND(StartNamedBounty);

		BIND(RestartNamedBounty);


		BIND(StartNamedBountyNearRef);

		BIND(GetBountyName);


		BIND(isBountyComplete);
		BIND(SetXMarker);
		BIND(SetBountyMessageRef);

		BIND(isSystemReady);

		BIND(ClaimStartupLock);

	

		BIND(InitSystem);

		BIND(GetRandomBountyName);


		BIND(PlayerTraveled);


		BIND(GetPlayerWorldSpaceName);

		BIND(isPlayerInWorldSpace);

		//Config
		BIND(SetConfigValue);
		
		BIND(GetConfigValueInt);

		//Regions
		BIND(AddBadRegion);

		//Groups
		BIND(AddGroup);

		BIND(AddMembertoGroup);

		BIND(GetModForm);

		BIND(SetGroupMemberComplete);

		BIND(IsGroupMemberUsed);

		BIND(GetBountyObjectRefs);

		BIND(SetScriptedDoorsComplete);

		//Rewards
		BIND(SpawnRandomReward);


		//Rifts
		BIND(CaptureCellData);

		BIND(SpawnRift);

		BIND(GetRandomRiftStartMarker);


		BIND(GetRiftRotations);

		BIND(GetRiftReferences);

		BIND(SpawnMonsterInCell);

		return true;
	}
}
