#include "IntList.h"
#include "ListLibary.h"
#include "UnStringList.h"
#include "SafezoneList.h"

namespace Undaunted
{
	//Regions/Worldspaces
	void AddBadRegionToConfig(std::uint32_t region);
	IntList GetBadRegions();


	//Groups
	int AddGroup(std::string questText, std::uint32_t minlevel, std::uint32_t maxlevel, UnStringlist tags);
	void AddMembertoGroup(int id, GroupMember member);
	GroupList GetRandomGroup();
	GroupList GetRandomTaggedGroup(std::string tag);
	GroupList GetGroup(std::string bountyName);
	int GetGroupCount();
	void ShuffleGroupLibary();

	//General
	void AddConfigValue(std::string key, std::string value);
	std::uint32_t GetConfigValueInt(std::string key);
	void SetPlayerLevel(std::uint32_t level);
	std::uint32_t GetPlayerLevel();

	//RewardBlacklist
	void AddRewardBlacklist(std::string key);
	UnDictionary getRewardBlacklist();

	//Safezones
	void AddSafezone(Safezone zone);
	SafezoneList GetSafezones();


}