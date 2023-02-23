#include "SKSELink.h"

namespace Undaunted {
	std::uint32_t GetReward(std::uint32_t rewardOffset, std::uint32_t playerlevel);
	bool IsWeaponLevelOk(RE::TESObjectWEAP* weapon, std::uint32_t playerlevel);
	bool IsArmourLevelOk(RE::TESObjectARMO* armour, std::uint32_t playerlevel);
}
