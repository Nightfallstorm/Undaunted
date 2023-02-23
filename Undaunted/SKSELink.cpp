#include "SKSELink.h"

RE::BSFixedString Undaunted::GetCurrentWorldspaceName()
{
	//You're thinking "Why is this here? we can already get this from the player?"
	//Answer is this call doesn't work on the VR version, so this is centralising the differences.
	return RE::PlayerCharacter::GetSingleton()->GetWorldspace()->editorID;
}

RE::TESObjectREFR* Undaunted::PlaceAtMe(RE::BSScript::Internal::VirtualMachine* registry, int count, RE::TESObjectREFR* ref, RE::TESForm* spawnForm, int something, bool ForcePersist, bool InitiallyDisabled)
{
	return PlaceAtMe_Native(registry, count, ref, spawnForm, 1, ForcePersist, InitiallyDisabled);
}

void Undaunted::MoveRef(RE::TESObjectREFR* object, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, RE::NiPoint3 pos, RE::NiPoint3 rot)
{
	std::uint32_t nullHandle = *g_invalidRefHandle;
	MoveRefrToPosition(object, &nullHandle, cell, worldspace, &pos, &rot);
}
