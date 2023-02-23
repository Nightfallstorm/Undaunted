#include "LocationUtils.h"
#include <ConfigUtils.h>
#include "WorldCellList.h"
#include "FormRefList.h"
#include <RiftList.h>
#include <RefList.h>

namespace Undaunted {
	WorldCellList worldCellList;
	bool worldCellListBuilt = false;



	RE::TESObjectREFR* GetRefObjectInCurrentCell(std::uint32_t formID)
	{
		RE::TESObjectCELL* parentCell = GetPlayer()->parentCell;
		int numberofRefs = papyrusCell::GetNumRefs(parentCell, 0);
		logger::info("GetObjectInCurrentCell Num Ref: %i", numberofRefs);
		for (int i = 0; i < numberofRefs; i++)
		{
			RE::TESObjectREFR* ref = papyrusCell::GetNthRef(parentCell, i, 0);
			if (ref != NULL)
			{
				if (ref->formID != NULL)
				{
					if (ref->formID == formID)
					{
						logger::info("ref->formID == formID");
						return ref;
					}
				}
			}
		}
		return NULL;
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
		int numberofRefs = papyrusCell::GetNumRefs(worldcell.cell, 0);
		auto safezones = GetSafezones();
		//logger::info("GetRandomObjectInCell Num Ref: %i", numberofRefs);		
		if (numberofRefs == 0)return NULL;
		while (true)
		{
			int Nth = rand() % numberofRefs;
			RE::TESObjectREFR* ref = papyrusCell::GetNthRef(worldcell.cell, Nth, 0);
			bool valid = true;
			for (int i = 0; i < safezones.length; i++)
			{
				if (strcmp(safezones.data[i].Worldspace.c_str(), worldcell.world->editorId.Get()) == 0)
				{
					Vector3 distvector = Vector3(ref->pos.x - safezones.data[i].PosX, ref->pos.y - safezones.data[i].PosY, ref->pos.z - safezones.data[i].PosZ);
					if (distvector.Magnitude() < safezones.data[i].Radius)
					{
						valid = false;
						logger::info("Target in Safezone: %s", safezones.data[i].Zonename.c_str());
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
		logger::info("RegionList Count: %08X", handler->regionList->Count());

		IntList badregions = GetBadRegions();

		std::uint32_t regioncount = handler->regionList->Count();
		for (std::uint32_t i = 0; i < regioncount; i++)
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
				RE::TESRegion* test = (RE::TESRegion*)handler->regionList->GetNthItem(i);
				if (test != NULL)
				{
					if (test->worldSpace == NULL)
					{
						logger::info("worldSpace %08X is null", i);
					}
					else
					{
						logger::info("worldSpace %08X is not null", i);
						logger::info("processing worldSpace %p", test->worldSpace);
						//Yeah. So some regions are really low in the memory stack?
						//They blow up if you try and use them so we filter out worldspace pointers below 000000000002433E
						if ((uintptr_t)test->worldSpace <= 148286)
						{
							logger::info("Low Level Region, Ignoring.");
						}
						else
						{
							RE::TESObjectCELL* cell = test->worldSpace->unk088;
							if (cell != NULL)
							{
								logger::info("unk088 is not null for worldspace %08x", i);
								int numberofRefs = papyrusCell::GetNumRefs(cell, 0);
								if (numberofRefs > 0)
								{
									WorldCell wcell = WorldCell();
									wcell.cell = cell;
									wcell.world = test->worldSpace;

									//Check if we know about this cell
									bool badcell = false;
									for (int i = 0; i < worldCellList.length && !badcell; i++)
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
		}
		logger::info("worldCellList built. %i Entries", worldCellList.length);
		worldCellListBuilt = true;
		for (int i = 0; i < worldCellList.length; i++)
		{
			logger::info("WorldName: %s", worldCellList.data[i].world->editorId.Get());
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
		for (int i = 0; i < worldCellList.length; i++)
		{
			if (strcmp(worldCellList.data[i].world->editorId.Get(), WorldspaceName.c_str()) == 0)
				return worldCellList.data[i];
		}

		logger::info("Named World Cell not found: %s", WorldspaceName.Get());
		return WorldCell();
	}

	void MoveRefToWorldCell(TESObjectREFR* object, TESObjectCELL* cell, TESWorldSpace* worldspace, NiPoint3 pos, NiPoint3 rot)
	{
		if (object != NULL)
		{
			logger::info("Moving %08X to %08X in %s", object->formID, cell->formID, worldspace->editorId.Get());
			NiPoint3 finalPos(pos);
			MoveRef(object, cell, worldspace, finalPos, rot);
		}
	}

	//Expensive...
	WorldCell GetWorldCellFromRef(TESObjectREFR* object)
	{
		NiPoint3 distance;
		TESObjectREFR* ref;
		int numberofRefs;
		for (int i = 0; i < worldCellList.length; i++)
		{
			numberofRefs = papyrusCell::GetNumRefs(worldCellList.data[i].cell, 0);
			for (int j = 0; j < numberofRefs; j++)
			{
				ref = papyrusCell::GetNthRef(worldCellList.data[i].cell, j, 0);
				if (ref != NULL)
				{
					distance = object->pos - ref->pos;
					Vector3 distvector = Vector3(distance.x, distance.y, distance.z);
					if (distvector.Magnitude() < 200)
					{
						return worldCellList.data[i];
					}
				}
			}
		}
		return WorldCell();
	}

	void CaptureArea()
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\UndauntedRift.pas");
		FormRefList list = FormRefList();
		RE::TESObjectCELL* parentCell = RE::PlayerCharacter::GetSingleton()->parentCell;
		int numberofRefs = papyrusCell::GetNumRefs(parentCell, 0);
		//Right so instead of faffing about creating a sse edit script that can read in files we'll just generate an sse script.
		logger::info("unit userscript;uses SkyrimUtils;uses mteFunctions;");
		logger::info("function Process(e: IInterface): integer;");
		logger::info("var cell: IInterface; ref: IInterface;");
		logger::info("begin Result := 0; if not (Signature(e) = 'CELL') then begin Exit; end; cell := createRecord(GetFile(e), 'CELL');");
		logger::info("SetElementEditValues(cell, 'EDID', '%s');","Cell Name");
		logger::info("SetElementEditValues(cell, 'LTMP', '%s');", "0006AB01");//Lighting template
		for (int i = 0; i < numberofRefs; i++)
		{
			RE::TESObjectREFR* ref = papyrusCell::GetNthRef(parentCell, i, 0);
			if (ref != NULL)
			{
				if (ref->formID != NULL)
				{
					FormRef saveref = FormRef();
					saveref.formId = ref->baseForm->formID;
					saveref.pos = ref->pos - GetPlayer()->pos;
					saveref.rot = ref->rot;
					saveref.scale = ref->unk90;
					saveref.type = static_cast<RE::FormType>(ref->baseForm->formType);
					if ((saveref.type == kFormType_Static
						//|| saveref.type == kFormType_LeveledCharacter
						|| saveref.type == kFormType_Activator
						|| saveref.type == kFormType_Sound
						|| saveref.type == kFormType_Light
						|| saveref.type == kFormType_Container
						|| saveref.type == kFormType_MovableStatic
						|| saveref.type == kFormType_Furniture
						|| saveref.type == kFormType_Reference
						|| saveref.type == kFormType_LeveledItem
						//|| saveref.type == kFormType_NPC
						|| saveref.type == kFormType_Hazard
						|| saveref.type == kFormType_Door
						|| saveref.type == kFormType_Flora
						|| saveref.type == kFormType_Tree
						|| saveref.type == kFormType_Grass
						|| saveref.type == kFormType_NAVM
						|| saveref.type == kFormType_NAVI) 
						&& saveref.formId < 4278190000)
					{
						logger::info("ref: = Add(cell, 'REFR', true);");
						logger::info("SetElementEditValues(ref, 'EDID', GetElementEditValues(getRecordByFormID('%08X'), 'EDID'));", saveref.formId);
						logger::info("SetElementEditValues(ref, 'NAME', '%08X');", saveref.formId);
						logger::info("SetElementEditValues(ref, 'XSCL', '%f');", ((float)saveref.scale)/100);
						logger::info("seev(ref, 'DATA\\[0]\\[0]', %f);", saveref.pos.x);
						logger::info("seev(ref, 'DATA\\[0]\\[1]', %f);", saveref.pos.y);
						logger::info("seev(ref, 'DATA\\[0]\\[2]', %f);", saveref.pos.z);
						logger::info("seev(ref, 'DATA\\[1]\\[0]', %f);", saveref.rot.x * (180.0 / 3.141592653589793238463));
						logger::info("seev(ref, 'DATA\\[1]\\[1]', %f);", saveref.rot.y * (180.0 / 3.141592653589793238463));
						logger::info("seev(ref, 'DATA\\[1]\\[2]', %f);", saveref.rot.z * (180.0 / 3.141592653589793238463));
					}
				}
			}
		}
		logger::info("end;function Finalize: integer;begin Result := 0;  FinalizeUtils();end;end.");
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\Undaunted.log");
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
		for (int i = 0; i < RiftStartMarkers.length + 10; i++)
		{
			RiftStartMarkers.SwapItem(rand() % RiftStartMarkers.length, rand() % RiftStartMarkers.length);
		}
	}

	void InitBakedRiftStartMarkers()
	{

		logger::info("Finding all Rift Start Markers");
		RiftStartMarkers = RefList();
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		const ModInfo* modInfo = dataHandler->LookupModByName("Undaunted.esp");
		if (modInfo == NULL)
		{
			logger::info("Can't find Undaunted.esp. What the hell?");
			return;
		}
		std::uint32_t FormId = (modInfo->modIndex << 24) + 915120; //040DF6B0 - 01_Undaunted_RiftEnteranceMarker
		logger::info("FormId: %08X", FormId);
		auto cellcount = dataHandler->cellList.m_size;
		logger::info("cellcount: %08X", cellcount);
		for (int i = 0; i < cellcount; i++)
		{
			RE::TESObjectCELL* parentCell = dataHandler->cellList.m_data[i];
			int numberofRefs = papyrusCell::GetNumRefs(parentCell, 0);
			for (int j = 0; j < numberofRefs; j++)
			{
				RE::TESObjectREFR* ref = papyrusCell::GetNthRef(parentCell, j, 0);
				if (ref != NULL)
				{
					if (ref->formID != NULL)
					{
						if (ref->baseForm->formID == FormId)
						{
							Ref formref = Ref();
							formref.objectRef = ref;
							RiftStartMarkers.AddItem(formref);
						}
					}
				}
			}
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
		logger::info("currentRiftTarget: %i / %i", currentRiftTarget, RiftStartMarkers.length);

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
