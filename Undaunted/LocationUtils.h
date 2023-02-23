#include "WorldCellList.h"
#include <FormRefList.h>

namespace Undaunted
{
	RE::TESObjectREFR* GetRefObjectInCurrentCell(std::uint32_t formID);
	//TESObjectREFR* GetRefObjectFromWorld(UInt32 formID);
	RE::TESObjectREFR* GetRandomObjectInCell(WorldCell worldcell);
	void BuildWorldList();
	WorldCell GetRandomWorldCell();
	WorldCell GetNamedWorldCell(RE::BSFixedString WorldspaceName);
	void MoveRefToWorldCell(RE::TESObjectREFR* object, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, RE::NiPoint3 pos, RE::NiPoint3 rot);
	WorldCell GetWorldCellFromRef(RE::TESObjectREFR* object);

	void CaptureArea();
	void AddRift(FormRefList reflist);
	FormRefList GetRandomRift();

	void InitBakedRiftStartMarkers();
	RE::TESObjectREFR* GetRandomBakedRiftStartMarker();

}
