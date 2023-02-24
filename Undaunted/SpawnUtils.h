#include "SKSELink.h"
#include "ConfigUtils.h"
#include "RefList.h"
#include "LocationUtils.h"

namespace Undaunted
{
	RE::TESObjectREFR* SpawnMonsterAtRef(RE::BSScript::Internal::VirtualMachine* registry, std::uint32_t Type, RE::TESObjectREFR* ref, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace);
	RE::TESObjectREFR* SpawnRefAtPosition(RE::FormID ID, WorldCell wcell, RE::NiPoint3 Position);
	RE::TESObjectREFR* SpawnMonsterInCell(RE::BSScript::Internal::VirtualMachine* registry, RE::FormID ID, WorldCell wcell);
	GroupList SpawnGroupInCell(RE::BSScript::Internal::VirtualMachine* registry, GroupList Types, WorldCell wcell);
	GroupList SpawnGroupAtTarget(RE::BSScript::Internal::VirtualMachine* registry, GroupList Types, RE::TESObjectREFR* Target, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, int spawnradius, int HeightDistance);
	RefList SpawnRift(RE::BSScript::Internal::VirtualMachine* registry, RE::TESObjectREFR* Target, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace);
	std::vector<float> GetRiftRotations();
	RefList GetCurrentRiftRefs();
}
