#include "MyPlugin.h"

namespace Undaunted {
	
	float hook_StartBounty(StaticFunctionTag* base, bool nearby) {
		BountyManager::getInstance()->StartBounty(nearby);
		return 2;
	}

	// Fill out the WorldList
	bool hook_InitSystem(StaticFunctionTag* base)
	{
		if (!BountyManager::getInstance()->isReady)
		{
			DataHandler* dataHandler = DataHandler::GetSingleton();
			_MESSAGE("Mod Count: %08X", dataHandler->modList.loadedMods.count);
			for (int i = 0; i < dataHandler->modList.loadedMods.count; i++)
			{
				ModInfo* mod;
				dataHandler->modList.loadedMods.GetNthItem(i, mod);
				_MESSAGE("Listing Mods: %s ", mod->name);
			}

			BuildWorldList();
			BountyManager::getInstance()->isReady = true;
		}
		return BountyManager::getInstance()->isReady;
	}

	bool hook_isSystemReady(StaticFunctionTag* base)
	{
		return BountyManager::getInstance()->isReady;
	}

	bool hook_isBountyComplete(StaticFunctionTag* base) {
		_MESSAGE("Starting Bounty Check");
		return BountyManager::getInstance()->BountyUpdate();
	}
	bool hook_SetXMarker(StaticFunctionTag* base, RE::TESObjectREFR* marker) {
		BountyManager::getInstance()->xmarkerref = marker;
		return true;
	}

	bool hook_SetBountyMessageRef(StaticFunctionTag* base, RE::BGSMessage* ref) {
		BountyManager::getInstance()->bountymessageref = ref;
		return true;
	}

	bool hook_AddBadRegion(StaticFunctionTag* base, UInt32 region) {
		AddBadRegionToConfig(region);
		return true;
	}

	UInt32 hook_AddGroup(StaticFunctionTag* base, BSFixedString questText){
		return AddGroup(questText.Get());
	}

	void hook_AddMembertoGroup(StaticFunctionTag* base, UInt32 groupid, UInt32 member, BSFixedString BountyType, BSFixedString ModelFilepath) {
		GroupMember newMember = GroupMember();
		newMember.FormId = member;
		newMember.BountyType = BountyType;
		newMember.ModelFilepath = ModelFilepath;
		AddMembertoGroup(groupid, newMember);
	}

	UInt32 hook_GetModForm(StaticFunctionTag* base, BSFixedString ModName, UInt32 FormId){
		DataHandler* dataHandler = DataHandler::GetSingleton();
		const ModInfo* modInfo = dataHandler->LookupModByName(ModName.c_str());
		if (modInfo != NULL)
		{
			FormId = (modInfo->modIndex << 24) + FormId;
			if (modInfo->IsFormInMod(FormId))
			{
				return FormId;
			}
			else
			{
				_MESSAGE("FormId  %08X Not Found in %s", FormId, ModName.Get());
				return UInt32();
			}
		}
		_MESSAGE("Mod Not Found: %s", ModName.Get());
		return UInt32();
	}

	void hook_SpawnRandomReward(StaticFunctionTag* base, RE::TESObjectREFR* target, UInt32 playerlevel)
	{
		TESForm* spawnForm = LookupFormByID(GetReward(playerlevel));
		_PlaceAtMe_Native(BountyManager::getInstance()->_registry, 1, target, spawnForm, 1, false, false);
	}

	void hook_SetGroupMemberComplete(StaticFunctionTag* base, RE::TESObjectREFR* taget)
	{
		BountyManager::getInstance()->bountygrouplist.SetGroupMemberComplete(taget->formID);
	}

	void hook_SetConfigValue(StaticFunctionTag* base, BSFixedString key, BSFixedString value)
	{
		SetConfigValue(key.Get(), value.Get());
	}

	bool hook_IsGroupMemberUsed(StaticFunctionTag* base, RE::TESObjectREFR* target)
	{
		//Is this reference in the current bounty? If it isn't we can get rid of it.
		for (int i = 0; i < BountyManager::getInstance()->bountygrouplist.length; i++)
		{
			if (BountyManager::getInstance()->bountygrouplist.data[i].objectRef->formID == target->formID)
			{
				return true;
			}
		}
		return false;
	}

	VMResultArray<RE::TESObjectREFR*> hook_GetBountyObjectRefs(StaticFunctionTag* base, BSFixedString bountyType)
	{
		VMResultArray<RE::TESObjectREFR*> allies = VMResultArray<RE::TESObjectREFR*>();
		for (int i = 0; i < BountyManager::getInstance()->bountygrouplist.length; i++)
		{
			if (strcmp(BountyManager::getInstance()->bountygrouplist.data[i].BountyType.Get(), bountyType.Get()) == 0)
			{
				if (BountyManager::getInstance()->bountygrouplist.data[i].objectRef != NULL)
				{
					allies.push_back(BountyManager::getInstance()->bountygrouplist.data[i].objectRef);
				}
			}
		}
		return allies;
	}

	UInt32 hook_GetConfigValueInt(StaticFunctionTag* base, BSFixedString key)
	{
		return GetConfigValueInt(key.Get());
	}

	bool RegisterFuncs(VMClassRegistry* registry) {

		BountyManager::getInstance()->_registry = registry;
		//General
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, float,bool>("StartBounty", "Undaunted_SystemScript", Undaunted::hook_StartBounty, registry));
		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, bool>("isBountyComplete", "Undaunted_SystemScript", Undaunted::hook_isBountyComplete, registry));
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, RE::TESObjectREFR*>("SetXMarker", "Undaunted_SystemScript", Undaunted::hook_SetXMarker, registry));
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, RE::BGSMessage*>("SetBountyMessageRef", "Undaunted_SystemScript", Undaunted::hook_SetBountyMessageRef, registry));

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, bool>("isSystemReady", "Undaunted_SystemScript", Undaunted::hook_isSystemReady, registry));

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, bool>("InitSystem", "Undaunted_SystemScript", Undaunted::hook_InitSystem, registry));

		//Config

		registry->RegisterFunction(
			new NativeFunction2 <StaticFunctionTag, void, BSFixedString, BSFixedString>("SetConfigValue", "Undaunted_SystemScript", Undaunted::hook_SetConfigValue, registry));
		
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, UInt32, BSFixedString>("GetConfigValueInt", "Undaunted_SystemScript", Undaunted::hook_GetConfigValueInt, registry));

		//Regions
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, UInt32>("AddBadRegion", "Undaunted_SystemScript", Undaunted::hook_AddBadRegion, registry));

		//Groups
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, UInt32, BSFixedString>("AddGroup", "Undaunted_SystemScript", Undaunted::hook_AddGroup, registry));

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void,UInt32,UInt32, BSFixedString, BSFixedString>("AddMembertoGroup", "Undaunted_SystemScript", Undaunted::hook_AddMembertoGroup, registry));

		registry->RegisterFunction(
			new NativeFunction2 <StaticFunctionTag, UInt32, BSFixedString, UInt32>("GetModForm", "Undaunted_SystemScript", Undaunted::hook_GetModForm, registry));

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, TESObjectREFR*>("SetGroupMemberComplete", "Undaunted_SystemScript", Undaunted::hook_SetGroupMemberComplete, registry));

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, TESObjectREFR*>("IsGroupMemberUsed", "Undaunted_SystemScript", Undaunted::hook_IsGroupMemberUsed, registry));

		registry->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, VMResultArray<TESObjectREFR*>, BSFixedString>("GetBountyObjectRefs", "Undaunted_SystemScript", Undaunted::hook_GetBountyObjectRefs, registry));


		//Rewards
		registry->RegisterFunction(
			new NativeFunction2 <StaticFunctionTag, void, TESObjectREFR*, UInt32>("SpawnRandomReward", "Undaunted_SystemScript", Undaunted::hook_SpawnRandomReward, registry));


		return true;
	}
}