#include "RewardUtils.h"
#include "ConfigUtils.h"

namespace Undaunted
{
	int GetRewardType() {
		int choice_weight[] = 
		{ 
			GetConfigValueInt("RewardWeaponWeight"),
			GetConfigValueInt("RewardArmourWeight"),
			GetConfigValueInt("RewardPotionWeight"),
			GetConfigValueInt("RewardScrollWeight"),
			GetConfigValueInt("RewardIngredientWeight"),
			GetConfigValueInt("RewardBookWeight"),
			GetConfigValueInt("RewardMiscWeight")
		};
		int numberofchoices = 7;
		int sum_of_weight = 0;
		for (int i = 0; i < numberofchoices; i++) {
			sum_of_weight += choice_weight[i];
		}
		int rnd = rand() % sum_of_weight;
		for (int i = 0; i < numberofchoices; i++) {
			if (rnd < choice_weight[i])
				return i;
			rnd -= choice_weight[i];
		}

		return 0;
	}

	int loopcount = 0;
	bool isFormInBlacklist(std::uint32_t formid)
	{
		auto blacklist = getRewardBlacklist();
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		for (int i = 0; i < blacklist.length; i++)
		{
			auto mod = dataHandler->LookupModByName(blacklist.data[i].value.c_str());
			if (mod != NULL)
			{
				if (mod->IsFormInMod(formid))
				{
					//loopcount++;
					//srand(time(NULL) + loopcount);
					return true;
				}
			}
		}
		return false;
	}

	std::uint32_t LastReward = 0;

	RE::FormID GetReward(std::uint32_t rewardOffset, std::uint32_t playerlevel)
	{
		srand(time(0) + rewardOffset);
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		std::set<RE::TESObjectARMO*> exclude;
		for (auto race : dataHandler->GetFormArray<RE::TESRace>())
		{
			if (race->skin)
				exclude.insert(race->skin);
		}
		for (auto npc : dataHandler->GetFormArray<RE::TESNPC>())
		{
			if (npc->skin)
				exclude.insert(npc->skin);
		}

		while (true)
		{
			int type = 0;
			bool foundvalidrewardtype = false;
			//Always give 1 weapon or 1 armour.
			if (rewardOffset == 1) {
				if (rand() % 100 > 50)
				{
					type = 0;
				}
				else
				{
					type = 1;
				}
				foundvalidrewardtype = true;
			}
			while (!foundvalidrewardtype)
			{
				type = GetRewardType();
				if (type == 6 && GetConfigValueInt("RewardAllowMiscItems") == 0)
				{
					continue;
				}
				foundvalidrewardtype = true;
			}
			auto weapons = dataHandler->GetFormArray<RE::TESObjectWEAP>();
			RE::TESObjectWEAP* weapon = NULL;
			auto armors = dataHandler->GetFormArray<RE::TESObjectARMO>();
			RE::TESObjectARMO* armour = NULL;
			auto potions = dataHandler->GetFormArray<RE::AlchemyItem>();
			RE::AlchemyItem* potion = NULL;
			auto scrolls = dataHandler->GetFormArray<RE::ScrollItem>();
			RE::ScrollItem* scroll = NULL;
			auto ingredients = dataHandler->GetFormArray<RE::IngredientItem>();
			RE::IngredientItem* ingre = NULL;
			auto books = dataHandler->GetFormArray<RE::TESObjectBOOK>();
			RE::TESObjectBOOK* book = NULL;
			auto miscObjects = dataHandler->GetFormArray<RE::TESObjectMISC>();
			RE::TESObjectMISC* misc = NULL;
			//type = 3;
			switch (type)
			{
			case 0:
				armour = armors[rand() % armors.size()];
				if (exclude.find(armour) != exclude.end())
				if (!armour->GetPlayable()) continue;
				if (armour->templateArmor) continue;
				if (armour->value <= 10) continue;
				if (!IsArmourLevelOk(armour, playerlevel))continue;
				if (armour->formID == LastReward) continue;
				if (isFormInBlacklist(armour->formID)) continue;
				LastReward = armour->formID;
				return armour->formID;
			case 1:
				weapon = weapons[rand() % weapons.size()];
				if (!weapon->GetPlayable()) continue;
				if (!weapon->IsBoundObject()) continue;
				if (weapon->value == 0) continue;
				if (weapon->templateWeapon) continue;
				if (!IsWeaponLevelOk(weapon, playerlevel)) continue;
				if (weapon->formID == LastReward) continue;
				if (isFormInBlacklist(weapon->formID)) continue;
				LastReward = weapon->formID;
				return weapon->formID;
			case 2:
				potion = potions[rand() % potions.size()];
				if (potion->formID == LastReward) continue;
				if (isFormInBlacklist(potion->formID)) continue;
				LastReward = potion->formID;
				return potion->formID;
			case 3:
				scroll = scrolls[rand() % scrolls.size()];
				if (!scroll->GetPlayable()) continue;
				if (!scroll->IsBoundObject()) continue;
				if (scroll->value == 0) continue;
				if (scroll->formID == LastReward) continue;
				if (isFormInBlacklist(scroll->formID)) continue;
				LastReward = scroll->formID;
				return scroll->formID;
			case 4:
				ingre = ingredients[rand() % ingredients.size()];
				if (!ingre->GetPlayable())
				continue;
				if (!ingre->IsBoundObject()) continue;
				if (ingre->value == 0) continue;
				if (ingre->formID == LastReward) continue;
				if (isFormInBlacklist(ingre->formID)) continue;
				LastReward = ingre->formID;
				return ingre->formID;
			case 5:
				book = books[rand() % books.size()];
				if (!book->GetPlayable()) continue;
				if (!book->IsBoundObject()) continue;
				if (book->value == 0) continue;
				if (book->value <= 50) continue;
				if (book->value >= 2000) continue;
				if (book->formID == LastReward) continue;
				if (isFormInBlacklist(book->formID)) continue;
				LastReward = book->formID;
				return book->formID;
			case 6:
				misc = miscObjects[rand() % miscObjects.size()];
				if (!misc->GetPlayable()) continue;
				if (!misc->IsBoundObject()) continue;
				if (misc->value == 0) continue;
				if (misc->value <= 50) continue;
				if (misc->formID == LastReward) continue;
				if (isFormInBlacklist(misc->formID)) continue;
				LastReward = misc->formID;
				return misc->formID;
			default:
				weapon = weapons[rand() % weapons.size()];
				if (!weapon->GetPlayable()) continue;
				if (!weapon->IsBoundObject()) continue;
				if (weapon->value == 0) continue;
				if (weapon->templateWeapon) continue;
				if (!IsWeaponLevelOk(weapon, playerlevel)) continue;
				if (weapon->formID == LastReward) continue;
				if (isFormInBlacklist(weapon->formID)) continue;
				LastReward = weapon->formID;
				return weapon->formID;
			}
		}
	}

	bool IsWeaponLevelOk(RE::TESObjectWEAP* weapon, std::uint32_t playerlevel)
	{
		std::uint16_t attackDamage = weapon->attackDamage * 100;
		std::uint32_t Moneyvalue = weapon->value;
		int targetMaxLevel = GetConfigValueInt("RewardTargetMaxLevel");
		float levelcoeffient = playerlevel + GetConfigValueInt("RewardPlayerLevelBoost");
		float minDamage = GetConfigValueInt("RewardWeaponMinDamage");
		float maxDamage = GetConfigValueInt("RewardWeaponMaxDamage");
		float partcoeffient = (maxDamage - minDamage) / targetMaxLevel;
		//logger::info("levelcoeffient: %f , partcoeffient: %f", levelcoeffient, partcoeffient);

		int minValueForPart = GetConfigValueInt("RewardWeaponMinValue");
		int maxValueForPart = GetConfigValueInt("RewardWeaponMaxValue");
		float valuecoeffient = (maxValueForPart - minValueForPart) / targetMaxLevel;
		for (std::uint32_t i = 0; i < weapon->numKeywords; i++)
		{			
			if (_stricmp(weapon->keywords[i]->formEditorID.c_str(), "DaedricArtifact") == 0 && GetConfigValueInt("RewardAllowDaedricArtifacts") == 1)
				return false;
			if (_stricmp(weapon->keywords[i]->formEditorID.c_str(), "WeapTypeStaff") == 0)
				return false;  //Staffs have low attack damage, but we don't want to spawn them to early.
		}
			
		if (weapon->formEnchanting)
		{
			if (weapon->amountofEnchantment)
			{
				Moneyvalue += weapon->amountofEnchantment;
			}
		}
		if (attackDamage < minDamage + (levelcoeffient * partcoeffient) && Moneyvalue < minValueForPart + (levelcoeffient * partcoeffient))
		{
			return true;
		}
		return false;
	}

	bool IsArmourLevelOk(RE::TESObjectARMO* armour, std::uint32_t playerlevel)
	{
		std::uint32_t Armourvalue = armour->armorRating;
		std::uint32_t Moneyvalue = armour->value;
		auto mask = armour->bipedModelData.bipedObjectSlots;
		auto weightClass = armour->bipedModelData.armorType.get();
		int targetMaxLevel = GetConfigValueInt("RewardTargetMaxLevel");
		float levelcoeffient = playerlevel;
		//logger::info("Level {:x}, weightClass: {:x}, Value: {}, mask: {:x},  Moneyvalue: {:x}", playerlevel, weightClass, Armourvalue, mask, Moneyvalue);
		int minArmourForPart = 0;
		int maxArmourForPart = 0;
		for (std::uint32_t i = 0; i < armour->numKeywords; i++)
		{
			if (_stricmp(armour->keywords[i]->formEditorID.c_str(), "DaedricArtifact") == 0 && GetConfigValueInt("RewardAllowDaedricArtifacts") == 0)
				return false;
			if (_stricmp(armour->keywords[i]->formEditorID.c_str(), "ArmorShield") == 0 && GetConfigValueInt("RewardAllowShields") == 0)
				return false;
			if (_stricmp(armour->keywords[i]->formEditorID.c_str(), "Dummy") == 0)
				return false;
		}
		//To calculate if an armour is in our level range we figure out a per level amour value for each slot then compare to the item.
		//So if the item is better than what we should have at our level we don't use it.
		//Light Armour
		auto none = RE::BIPED_MODEL::BipedObjectSlot::kNone;
		if (weightClass == RE::BIPED_MODEL::ArmorType::kLightArmor)
		{
			
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kBody) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Light_Chest_Value_Min");//Hide
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Light_Chest_Value_Max");;//Dragonscale 
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kFeet) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Light_Boot_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Light_Boot_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kHands) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Light_Hand_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Light_Hand_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kHead) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Light_Head_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Light_Head_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kShield) != none)
			{
				if (GetConfigValueInt("RewardAllowShields") == 0) return false;
				minArmourForPart = GetConfigValueInt("Reward_Armour_Light_Shield_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Light_Shield_Value_Max");
			}
			float partcoeffient = (maxArmourForPart - minArmourForPart) / targetMaxLevel;
//			logger::info("levelcoeffient: %f , partcoeffient: %f", levelcoeffient, partcoeffient);
			if (Armourvalue < minArmourForPart + (levelcoeffient * partcoeffient))
			{
				return true;
			}
		}
		//Heavy Armour
		if (weightClass == RE::BIPED_MODEL::ArmorType::kHeavyArmor)
		{
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kBody) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Chest_Value_Min");//Iron
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Chest_Value_Max");//Daedric 
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kFeet) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Boot_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Boot_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kHands) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Hand_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Hand_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kHead) != none)
			{
				minArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Head_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Head_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kShield) != none)
			{
				if (GetConfigValueInt("RewardAllowShields") == 0) return false;
				minArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Shield_Value_Min");
				maxArmourForPart = GetConfigValueInt("Reward_Armour_Heavy_Shield_Value_Max");
			}
			float partcoeffient = (maxArmourForPart - minArmourForPart) / targetMaxLevel;
//			logger::info("levelcoeffient: %f , partcoeffient: %f", levelcoeffient, partcoeffient);
			if (Armourvalue < minArmourForPart + (levelcoeffient * partcoeffient))
			{
				return true;
			}
		}
		//Clothes
		int minValueForPart = 1;
		int maxValueForPart = 2563;
		if (weightClass == RE::BIPED_MODEL::ArmorType::kClothing && GetConfigValueInt("RewardAllowClothes") == 1)
		{
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kBody) != none)
			{
				minValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Chest_Value_Min");
				maxValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Chest_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kFeet) != none)
			{
				minValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Boot_Value_Min");
				maxValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Boot_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kHands) != none)
			{
				minValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Hand_Value_Min");
				maxValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Hand_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kHead) != none)
			{
				minValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Head_Value_Min");
				maxValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Head_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kRing) != none)
			{
				minValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Ring_Value_Min");
				maxValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Ring_Value_Max");
			}
			if ((mask & RE::BIPED_MODEL::BipedObjectSlot::kCirclet) != none)
			{
				minValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Circlet_Value_Min");
				maxValueForPart = GetConfigValueInt("Reward_Armour_Clothes_Circlet_Value_Max");
			}
			float partcoeffient = (maxValueForPart - minValueForPart) / targetMaxLevel;
//			logger::info("levelcoeffient: %f , partcoeffient: %f", levelcoeffient, partcoeffient);
			if (Moneyvalue < minValueForPart + (levelcoeffient * partcoeffient))
			{
				return true;
			}
		}
		return false;
	}
}

