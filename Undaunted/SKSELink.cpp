#include "SKSELink.h"
#include "LocationUtils.h"
RE::BSFixedString Undaunted::GetCurrentWorldspaceName()
{
	//You're thinking "Why is this here? we can already get this from the player?"
	//Answer is this call doesn't work on the VR version, so this is centralising the differences.

	return GetPlayerWorldspace()->editorID;
}

RE::TESWorldSpace* Undaunted::GetPlayerWorldspace()
{
	// CLIB-NG: cachedWorldSpace is inaccesible on VR for the moment, so do an ugly reinterpret_cast instead

	std::uintptr_t player = reinterpret_cast<std::uintptr_t>(RE::PlayerCharacter::GetSingleton());
	RE::TESWorldSpace** result = nullptr;
	if (REL::Module::IsAE()) {
		// AE
		if (REL::Module::get().version().patch() > 600) {
			// 1.6.629+
			result = reinterpret_cast<RE::TESWorldSpace**>(player + 0x630);
		} else {
			// 1.6.353 and below
			result = reinterpret_cast<RE::TESWorldSpace**>(player + 0x628);
		}
	} else if (REL::Module::IsSE()) {
		// SE
		result = reinterpret_cast<RE::TESWorldSpace**>(player + 0x628);
	} else {
		// VR
		result = reinterpret_cast<RE::TESWorldSpace**>(player + 0xC18);
	}
	return *result;
}

RE::TESObjectREFR* Undaunted::PlaceAtMe(RE::BSScript::Internal::VirtualMachine* registry, RE::VMStackID a_stackID, RE::TESObjectREFR* ref, RE::TESForm* spawnForm, std::uint32_t aiCount, bool ForcePersist, bool InitiallyDisabled)
{
	using func_t = decltype(&Undaunted::PlaceAtMe);
	REL::Relocation<func_t> func{ REL::RelocationID(55672, 56203) };
	return func(registry, a_stackID, ref, spawnForm, aiCount, ForcePersist, InitiallyDisabled);
}

static void MoveTo_Impl(RE::TESObjectREFR* a_self, const RE::ObjectRefHandle& a_targetHandle, RE::TESObjectCELL* a_targetCell, RE::TESWorldSpace* a_selfWorldSpace, const RE::NiPoint3& a_position, const RE::NiPoint3& a_rotation)
{
	using func_t = decltype(&MoveTo_Impl);
	REL::Relocation<func_t> func{ REL::RelocationID(56227, 56626) };
	func(a_self, a_targetHandle, a_targetCell, a_selfWorldSpace, a_position, a_rotation);
}

void Undaunted::MoveRef(RE::TESObjectREFR* object, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, RE::NiPoint3 pos, RE::NiPoint3 rot)
{
	MoveTo_Impl(object, RE::ObjectRefHandle(), cell, worldspace, pos, rot);
}
