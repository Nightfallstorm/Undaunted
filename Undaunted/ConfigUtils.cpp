#include "ConfigUtils.h"
#include <time.h>
#include "SafezoneList.h"

namespace Undaunted
{
	std::string s_configPath;
	IntList BadRegionList;
	ListLibary GroupLibary;
	UnDictionary SettingsList;

	//Regions
	void AddBadRegionToConfig(std::uint32_t region)
	{
		logger::info("Adding %08X to Bad Region List", region);
		BadRegionList.AddItem(region);
	}

	IntList GetBadRegions() {
		return BadRegionList;
	}

	//Groups
	int AddGroup(std::string questText, std::uint32_t minlevel, std::uint32_t maxlevel, UnStringlist tags)
	{
		logger::info("Adding bounty to GroupLibary: %s", questText.c_str());
		GroupList newGroup = GroupList();
		newGroup.questText = questText;
		newGroup.minLevel = minlevel;
		newGroup.maxLevel = maxlevel;
		newGroup.Tags = tags;
		GroupLibary.AddItem(newGroup);
		return GroupLibary.length - 1;
	}

	void AddMembertoGroup(int id, GroupMember member)
	{
		//logger::info("Adding %08X to %i of BountyType %s",member.FormId, id,member.BountyType.Get());
		GroupLibary.data[id].AddItem(member);
	}

	GroupList GetGroup(std::string bountyName)
	{
		for (int i = 0; i < GroupLibary.length; i++)
		{
			if (GroupLibary.data[i].questText.compare(bountyName) == 0)
			{
				return GroupLibary.data[i];
			}
		}
		//All else fails return something at least.
		return GetRandomGroup();
	}

	int GetGroupCount()
	{
		return GroupLibary.length;
	}

	void ShuffleGroupLibary()
	{
		srand(time(NULL));
		for (int i = 0; i < GroupLibary.length + 100; i++)
		{
			GroupLibary.SwapItem(rand() % GroupLibary.length, rand() % GroupLibary.length);
		}
	}

	
	int GroupLibaryIndex = 0;
	GroupList GetRandomGroup()
	{
		std::uint32_t playerLevel = GetPlayerLevel();
		while (true)
		{
			int groupid = GroupLibaryIndex++;
			logger::info("Random Group: %i", groupid);
			if (groupid >= GroupLibary.length)
			{
				ShuffleGroupLibary();
				GroupLibaryIndex = 0;
			}
			logger::info("Random Member Count: %i", GroupLibary.data[groupid].length);
			//Player is too low level for this bounty
			if (playerLevel + GetConfigValueInt("BountyLevelCache") < GroupLibary.data[groupid].minLevel && GroupLibary.data[groupid].minLevel != 0)
			{
				continue;
			}
			//Player is too high level for this bounty
			if (playerLevel > GroupLibary.data[groupid].maxLevel && GroupLibary.data[groupid].maxLevel != 0)
			{
				continue;
			}
			return GroupLibary.data[groupid];
		}
	}

	GroupList GetRandomTaggedGroup(std::string tag)
	{
		std::uint32_t playerLevel = GetPlayerLevel();
		int startingGroupLibaryIndex = GroupLibaryIndex;
		while(true)
		{
			int groupid = GroupLibaryIndex++;
			if (groupid >= GroupLibary.length)
			{
				ShuffleGroupLibary();
				GroupLibaryIndex = 0;
				startingGroupLibaryIndex = 0;
			}
			logger::info("Random Group: %i", groupid);
			logger::info("Random Member Count: %i", GroupLibary.data[groupid].length);
			//Player is too low level for this bounty
			if (playerLevel + GetConfigValueInt("BountyLevelCache") < GroupLibary.data[groupid].minLevel && GroupLibary.data[groupid].minLevel != 0)
			{
				continue;
			}
			//Player is too high level for this bounty
			if (playerLevel > GroupLibary.data[groupid].maxLevel && GroupLibary.data[groupid].maxLevel != 0)
			{
				continue;
			}
			
			for (int i = 0; i < GroupLibary.data[groupid].Tags.length; i++)
			{
				logger::info("Comparing Tag: %s = %s", GroupLibary.data[groupid].Tags.data[i].c_str(), tag.c_str());
				if (GroupLibary.data[groupid].Tags.data[i].compare(tag) == 0)
				{
					logger::info("Found Tag: %s", GroupLibary.data[groupid].Tags.data[i].c_str());
					if (GroupLibaryIndex > GroupLibary.length)
					{
						ShuffleGroupLibary();
						GroupLibaryIndex = 0;
					}
					//Take the tagged card out of the pack and place it on top, then draw it.
					GroupLibary.SwapItem(groupid, startingGroupLibaryIndex);
					GroupLibaryIndex = startingGroupLibaryIndex + 1;
					return GroupLibary.data[startingGroupLibaryIndex];
				}
			}
		}
	}

	void AddConfigValue(std::string key, std::string value)
	{
		//logger::info("CONFIGLENGTH: %i", SettingsList.length);
		//check if it exists		
		for (int i = 0; i < SettingsList.length; i++)
		{
			if (SettingsList.data[i].key.compare(key) == 0)
			{
				SettingsList.data[i].value = value;
				//logger::info("SET: %s : %s", key, value);
				return;
			}
		}
		//doesn't exist
		UnKeyValue setting = UnKeyValue();
		setting.key = key;
		setting.value = value;
		SettingsList.AddItem(setting);
		//logger::info("ADD: %s : %s", key.c_str(), value.c_str());
	}

	std::uint32_t GetConfigValueInt(std::string key)
	{
		for (int i = 0; i < SettingsList.length; i++)
		{
			//logger::info("Comparing %s : %s", key.c_str(), SettingsList.data[i].key.c_str());
			if (SettingsList.data[i].key.compare(key) == 0)
			{
				//logger::info("Found Key %s : %s", key.c_str(), SettingsList.data[i].value.c_str());
				return atoi(SettingsList.data[i].value.c_str());
			}
		}
		//Not found.
		return 0;
	}

	std::uint32_t Playerlevel;
	void SetPlayerLevel(std::uint32_t level)
	{
		Playerlevel = level;
	}

	std::uint32_t GetPlayerLevel()
	{
		return Playerlevel;
	}

	UnDictionary RewardBlacklist = UnDictionary();
	void AddRewardBlacklist(std::string key)
	{
		UnKeyValue data = UnKeyValue();
		data.value = key;
		RewardBlacklist.AddItem(data);
	}

	UnDictionary getRewardBlacklist()
	{
		return RewardBlacklist;
	}

	SafezoneList szlist = SafezoneList();

	void AddSafezone(Safezone zone)
	{
		szlist.AddItem(zone);
	}

	SafezoneList GetSafezones()
	{
		return szlist;
	}

}