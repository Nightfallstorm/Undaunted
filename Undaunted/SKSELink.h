#ifndef SKSELink
#define SKSELink
#include <time.h>

#include <set>
#include <shlobj.h>				// CSIDL_MYCODUMENTS

namespace Undaunted
{
	RE::TESObjectREFR* PlaceAtMe(RE::BSScript::Internal::VirtualMachine* registry, RE::VMStackID a_stackID, RE::TESObjectREFR* ref, RE::TESForm* spawnForm, std::uint32_t aiCount, bool ForcePersist, bool InitiallyDisabled);
	RE::BSFixedString GetCurrentWorldspaceName();
	RE::TESWorldSpace* GetPlayerWorldspace();
	void MoveRef(RE::TESObjectREFR* object, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, RE::NiPoint3 pos, RE::NiPoint3 rot);
}
#endif
