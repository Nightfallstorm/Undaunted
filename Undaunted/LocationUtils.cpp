#include "LocationUtils.h"
#include <ConfigUtils.h>
#include "WorldCellList.h"
#include "FormRefList.h"
#include <RiftList.h>
#include <RefList.h>
#include "Vector.h"

namespace Undaunted {
	WorldCellList worldCellList;
	bool worldCellListBuilt = false;



	RE::TESObjectREFR* GetRefObjectInCurrentCell(std::uint32_t formID)
	{
		RE::TESObjectCELL* parentCell = RE::PlayerCharacter::GetSingleton()->parentCell;
		RE::TESObjectREFR* refResult = nullptr;
		parentCell->ForEachReference([&](RE::TESObjectREFR& ref) {
			if (ref.formID && ref.formID == formID) {
				logger::info("ref->formID == formID");
				refResult = &ref;
				return RE::BSContainer::ForEachResult::kStop;
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
		return refResult;
	}

	//Use Sparingly.
	/*
	TESObjectREFR* GetRefObjectFromWorld(std::uint32_t formID)
	{
		DataHandler* handler = GetDataHandler();
		logger::info("RegionList Count: %08X", handler->regionList->Count());
		std::uint32_t regioncount = handler->regionList->Count();
		for (std::uint32_t i = 0; i < regioncount; i++)
		{
			//Some regions are dodgy
			if (i != 0x00000033 && i != 0x00000036 && i != 0x0000009B && i != 0x00000110)
			{
				logger::info("processing worldSpace %08X", i);
				TESRegion* test = (TESRegion*)handler->regionList->GetNthItem(i);
				if (test != NULL)
				{
					if (test->worldSpace == NULL)
					{
						logger::info("worldSpace %08X is null", i);
					}
					else
					{
						TESObjectCELL* cell = test->worldSpace->unk088;
						if (cell != NULL)
						{
							logger::info("Regioncell form id %08X", cell->formID);
							if (cell->formID == 0x00000D74)
							{
								int numberofRefs = papyrusCell::GetNumRefs(cell, 0);
								if (numberofRefs > 0)
								{
									for (int i = 0; i < numberofRefs; i++)
									{
										TESObjectREFR* ref = papyrusCell::GetNthRef(cell, i, 0);
										if (ref != NULL)
										{
											if (ref->formID != NULL)
											{
												if (ref->formID == formID)
												{
													return ref;
												}
											}
										}
									}
								}
							}
						}
						else
						{
							logger::info("unk088 is null", i);
						}
					}

				}
				else
				{
					logger::info("RegionList %08X is null", i);
				}
			}
		}
		return NULL;
	}*/

	RE::TESObjectREFR* GetRandomObjectInCell(WorldCell worldcell)
	{
		int numberofRefs = worldcell.cell->GetRuntimeData().references.size(); 
		auto safezones = GetSafezones();
		//logger::info("GetRandomObjectInCell Num Ref: %i", numberofRefs);		
		if (numberofRefs == 0)return NULL;
		while (true)
		{
			int Nth = rand() % numberofRefs;

			// CLIB-NG TODO: Possible behavior change?
			auto it = std::begin(worldcell.cell->GetRuntimeData().references);
			std::advance(it, Nth);
			
			RE::TESObjectREFR* ref = (*it).get();

			bool valid = true;
			for (int i = 0; i < safezones.length; i++)
			{
				if (strcmp(safezones.data[i].Worldspace.c_str(), worldcell.world->editorID.c_str()) == 0)
				{
					Vector3 distvector = Vector3(ref->GetPositionX() - safezones.data[i].PosX, ref->GetPositionY() - safezones.data[i].PosY, ref->GetPositionZ() - safezones.data[i].PosZ);
					if (distvector.Magnitude() < safezones.data[i].Radius)
					{
						valid = false;
						logger::info("Target in Safezone: {}", safezones.data[i].Zonename.c_str());
						break;
					}
				}
			}
			if (ref != NULL && valid)
			{
				return ref;
			}
		}
		return NULL;
	}

	void BuildWorldList()
	{
		if (worldCellListBuilt)
		{
			return;
		}
		auto handler = RE::TESDataHandler::GetSingleton();
		//logger::info("RegionList Count: %08X", handler->regionList->Count());

		IntList badregions = GetBadRegions();

		std::uint32_t i = 0;
		for (auto test : *handler->regionList)
		{
			
			//Check for badregion
			bool badRegion = false;
			for (std::uint32_t j = 0; j < badregions.length; j++)
			{
				if (badregions.data[j] == i)
				{
					badRegion = true;
				}
			}
			//Some regions are dodgy
			if (!badRegion)
			{
				logger::info("processing worldSpace %08X", i);

				if (test != NULL)
				{
					if (test->worldSpace == NULL)
					{
						logger::info("worldSpace %08X is null", i);
					}
					else
					{
						logger::info("worldSpace %08X is not null", i);
						//logger::info("processing worldSpace {}", test->worldSpace->GetFullName());
						//Yeah. So some regions are really low in the memory stack?
						//They blow up if you try and use them so we filter out worldspace pointers below 000000000002433E
						if ((uintptr_t)test->worldSpace <= 148286)
						{
							logger::info("Low Level Region, Ignoring.");
						}
						else
						{
							RE::TESObjectCELL* cell = test->worldSpace->persistentCell;
							if (cell != NULL)
							{
								logger::info("unk088 is not null for worldspace %08x", i);
								int numberofRefs = cell->GetRuntimeData().references.size();
								if (numberofRefs > 0)
								{
									WorldCell wcell = WorldCell();
									wcell.cell = cell;
									wcell.world = test->worldSpace;

									//Check if we know about this cell
									bool badcell = false;
									for (std::uint32_t i = 0; i < worldCellList.length && !badcell; i++)
									{
										if (worldCellList.data[i].cell->formID == wcell.cell->formID)
										{
											badcell = true;
										}
									}

									if (!badcell)
									{
										worldCellList.AddItem(wcell);
									}
								}
							}
							else
							{
								logger::info("unk088 is null for worldspace %08x", i);
							}
						}
					}
				}
				else
				{
					logger::info("RegionList %08X is null", i);
				}
			}

			i++;
		}
		logger::info("worldCellList built. %i Entries", worldCellList.length);
		worldCellListBuilt = true;
		for (std::uint32_t i = 0; i < worldCellList.length; i++)
		{
			logger::info("WorldName: %s", worldCellList.data[i].world->editorID.c_str());
		}
	}

	WorldCell GetRandomWorldCell()
	{
		srand(time(NULL));
		int worldcellid = rand() % worldCellList.length;
		return worldCellList.data[worldcellid];
	}

	WorldCell GetNamedWorldCell(RE::BSFixedString WorldspaceName)
	{
		for (std::uint32_t i = 0; i < worldCellList.length; i++)
		{
			if (strcmp(worldCellList.data[i].world->editorID.c_str(), WorldspaceName.c_str()) == 0)
				return worldCellList.data[i];
		}

		logger::info("Named World Cell not found: {}", WorldspaceName.c_str());
		return WorldCell();
	}

	void MoveRefToWorldCell(RE::TESObjectREFR* object, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, RE::NiPoint3 pos, RE::NiPoint3 rot)
	{
		if (object != NULL)
		{
			logger::info("Moving {:x} to {:x} in {}", object->formID, cell->formID, worldspace->editorID.c_str());
			RE::NiPoint3 finalPos(pos);
			MoveRef(object, cell, worldspace, finalPos, rot);
		}
	}

	//Expensive...
	WorldCell GetWorldCellFromRef(RE::TESObjectREFR* object)
	{
		RE::NiPoint3 distance;
		bool foundResult = false;
		WorldCell result = WorldCell();
		for (std::uint32_t i = 0; i < worldCellList.length; i++)
		{
			worldCellList.data[i].cell->ForEachReference([&](RE::TESObjectREFR& ref) {

				if (foundResult) {
					return RE::BSContainer::ForEachResult::kStop;
				}

				distance = object->GetPosition() - ref.GetPosition();
				Vector3 distvector = Vector3(distance.x, distance.y, distance.z);
				if (distvector.Magnitude() < 200) {
					result = worldCellList.data[i];
					foundResult = true;
					return RE::BSContainer::ForEachResult::kStop;
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}
		return WorldCell();
	}

	void CaptureArea()
	{
		// CLIB-NG TODO: Removed this but may be needed
		//gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\UndauntedRift.pas");
		FormRefList list = FormRefList();
		RE::TESObjectCELL* parentCell = RE::PlayerCharacter::GetSingleton()->parentCell;

		//Right so instead of faffing about creating a sse edit script that can read in files we'll just generate an sse script.
		logger::info("unit userscript;uses SkyrimUtils;uses mteFunctions;");
		logger::info("function Process(e: IInterface): integer;");
		logger::info("var cell: IInterface; ref: IInterface;");
		logger::info("begin Result := 0; if not (Signature(e) = 'CELL') then begin Exit; end; cell := createRecord(GetFile(e), 'CELL');");
		logger::info("SetElementEditValues(cell, 'EDID', '{}');","Cell Name");
		logger::info("SetElementEditValues(cell, 'LTMP', '{}');", "0006AB01");//Lighting template
		parentCell->ForEachReference([&](RE::TESObjectREFR& ref) {
			if (ref.formID != NULL) {
				FormRef saveref = FormRef();
				saveref.formId = ref.GetBaseObject()->formID;
				saveref.pos = ref.GetPosition() - RE::PlayerCharacter::GetSingleton()->GetPosition();
				saveref.rot = ref.GetAngle();
				saveref.scale = ref.GetScale();
				saveref.type = ref.GetBaseObject()->formType.get();
				if ((saveref.type == RE::FormType::Static
						//|| saveref.type == kFormType_LeveledCharacter
						|| saveref.type == RE::FormType::Activator || saveref.type == RE::FormType::Sound || saveref.type == RE::FormType::Light || saveref.type == RE::FormType::Container || saveref.type == RE::FormType::MovableStatic || saveref.type == RE::FormType::Furniture || saveref.type == RE::FormType::Reference || saveref.type == RE::FormType::LeveledItem
						//|| saveref.type == RE::FormType::NPC
						|| saveref.type == RE::FormType::Hazard || saveref.type == RE::FormType::Door || saveref.type == RE::FormType::Flora || saveref.type == RE::FormType::Tree || saveref.type == RE::FormType::Grass || saveref.type == RE::FormType::Navigation || saveref.type == RE::FormType::NavMesh) &&
					saveref.formId < 4278190000) {
					logger::info("ref: = Add(cell, 'REFR', true);");
					logger::info("SetElementEditValues(ref, 'EDID', GetElementEditValues(getRecordByFormID('{:x}'), 'EDID'));", saveref.formId);
					logger::info("SetElementEditValues(ref, 'NAME', '{:x}');", saveref.formId);
					logger::info("SetElementEditValues(ref, 'XSCL', '{}');", ((float)saveref.scale) / 100);
					logger::info("seev(ref, 'DATA\\[0]\\[0]', {});", saveref.pos.x);
					logger::info("seev(ref, 'DATA\\[0]\\[1]', {});", saveref.pos.y);
					logger::info("seev(ref, 'DATA\\[0]\\[2]', {});", saveref.pos.z);
					logger::info("seev(ref, 'DATA\\[1]\\[0]', {});", saveref.rot.x * (180.0 / 3.141592653589793238463));
					logger::info("seev(ref, 'DATA\\[1]\\[1]', {});", saveref.rot.y * (180.0 / 3.141592653589793238463));
					logger::info("seev(ref, 'DATA\\[1]\\[2]', {});", saveref.rot.z * (180.0 / 3.141592653589793238463));
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
		
		logger::info("end;function Finalize: integer;begin Result := 0;  FinalizeUtils();end;end.");
		// CLIB-NG TODO: Removed this but may be needed
		//gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\Undaunted.log");
	}

	//Raw Rifts

	RiftList riftList = RiftList();
	void AddRift(FormRefList reflist)
	{
		RiftRef newref = RiftRef();
		newref.reflist = reflist;
		riftList.AddItem(newref);
	}

	FormRefList GetRandomRift()
	{
		return riftList.data[rand() % riftList.length].reflist;
	}

	// Baked Rifts
	RefList RiftStartMarkers = RefList();
	void ShuffleBakedRifts()
	{
		srand(time(NULL));
		for (std::uint32_t i = 0; i < RiftStartMarkers.length + 10; i++)
		{
			RiftStartMarkers.SwapItem(rand() % RiftStartMarkers.length, rand() % RiftStartMarkers.length);
		}
	}

	void InitBakedRiftStartMarkers()
	{

		logger::info("Finding all Rift Start Markers");
		RiftStartMarkers = RefList();
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		const RE::TESFile* modInfo = dataHandler->LookupModByName("Undaunted.esp");
		if (modInfo == NULL)
		{
			logger::info("Can't find Undaunted.esp. What the hell?");
			return;
		}
		std::uint32_t FormId = (modInfo->compileIndex << 24) + 915120; //040DF6B0 - 01_Undaunted_RiftEnteranceMarker
		logger::info("FormId: {:x}", FormId);
		auto cellcount = dataHandler->interiorCells.size();
		logger::info("cellcount: {:x}", cellcount);
		for (std::uint32_t i = 0; i < cellcount; i++)
		{
			RE::TESObjectCELL* parentCell = dataHandler->interiorCells[i];
			parentCell->ForEachReference([&](RE::TESObjectREFR& ref) {
				if (ref.formID != NULL) {
					if (ref.GetBaseObject()->formID == FormId) {
						Ref formref = Ref();
						formref.objectRef = &ref;
						RiftStartMarkers.AddItem(formref);
					}
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}
		ShuffleBakedRifts();
	}



	int currentRiftTarget = 0;
	RE::TESObjectREFR* GetRandomBakedRiftStartMarker()
	{
		srand(time(NULL));
		currentRiftTarget++;
		if (currentRiftTarget >= RiftStartMarkers.length)
		{
			//Reshuffle deck
			logger::info("Reshuffle deck");
			ShuffleBakedRifts();
			currentRiftTarget = 0;
		}
		logger::info("currentRiftTarget: {} / {}", currentRiftTarget, RiftStartMarkers.length);

		Ref target = RiftStartMarkers.data[currentRiftTarget];

		return target.objectRef;
	}

	//Interiors
/*
if (SpawnLocref == NULL)
{
	TESObjectCELL* here = GetPlayer()->parentCell;
	logger::info("Here form id %08X", here->formID);
	logger::info("Cell list Size %08X", GetDataHandler()->cellList.m_size);
	std::uint32_t cellcount = GetDataHandler()->cellList.m_size;
	for (int i = 0; i < cellcount; i++)
	{
		TESObjectCELL* parentCell = GetDataHandler()->cellList.m_data[i];
		logger::info("Cell form id %08X", parentCell->formID);
		logger::info("Cell form id %08X", parentCell->formID);
		int numberofRefs = papyrusCell::GetNumRefs(parentCell, 0);
		logger::info("Num Ref: %i", numberofRefs);
		SpawnMonstersInCell(1, 0x06001DFC, parentCell);
	}
}
*/

}
