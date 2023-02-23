#ifndef SKSELink
#define SKSELink
#include <time.h>

#include <set>
#include <shlobj.h>				// CSIDL_MYCODUMENTS

namespace Undaunted
{
	RE::TESObjectREFR* PlaceAtMe(RE::BSScript::Internal::VirtualMachine* registry, std::uint32_t count, RE::TESObjectREFR* ref, RE::TESForm* spawnForm, int something, bool ForcePersist, bool InitiallyDisabled);
	RE::BSFixedString GetCurrentWorldspaceName();
	void MoveRef(RE::TESObjectREFR* object, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, RE::NiPoint3 pos, RE::NiPoint3 rot);
}
#endif
