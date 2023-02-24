#include "SpawnUtils.h"
#include "ConfigUtils.h"
#include "BountyManager.h"

namespace Undaunted
{
	RE::TESObjectREFR* SpawnMonsterAtRef(RE::BSScript::Internal::VirtualMachine* registry, RE::FormID ID, RE::TESObjectREFR* ref, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace)
	{
		RE::NiPoint3 startingpoint = ref->GetPosition();
		RE::TESForm* spawnForm = RE::TESForm::LookupByID(ID);
		if (spawnForm == NULL)
		{
			logger::info("Failed to Spawn. Form Invalid: {:x}", ID);
			return NULL;
		}
		int spawnradius = GetConfigValueInt("BountyEnemyInteriorSpawnRadius");
		RE::NiPoint3 offset = RE::NiPoint3(rand() & spawnradius, rand() & spawnradius, 0);
		MoveRefToWorldCell(ref, cell, worldspace, ref->GetPosition() + offset, RE::NiPoint3(0, 0, 0));
		RE::TESObjectREFR* spawned = PlaceAtMe(registry, 1, ref, spawnForm, 1, true, false);
		MoveRefToWorldCell(ref, cell, worldspace, startingpoint, RE::NiPoint3(0, 0, 0));
		return spawned;
	}

	RE::TESObjectREFR* SpawnRefAtPosition(RE::FormID Type, WorldCell wcell, RE::NiPoint3 Position)
	{
		RE::TESForm* spawnForm = RE::TESForm::LookupByID(Type);
		if (spawnForm == NULL)
		{
			logger::info("Failed to Spawn. Form Invalid: {:x}", Type);
			return NULL;
		}
		RE::TESObjectREFR* spawned = PlaceAtMe(BountyManager::getInstance()->_registry, 1, RE::PlayerCharacter::GetSingleton(), spawnForm, 1, true, false);
		//MoveRefToWorldCell(spawned, wcell.cell, wcell.world, Position, NiPoint3(0, 0, 0));
		return spawned;
	}

	RE::TESObjectREFR* SpawnMonsterInCell(RE::BSScript::Internal::VirtualMachine* registry, RE::FormID ID, WorldCell wcell)
	{
		RE::TESForm* spawnForm = RE::TESForm::LookupByID(ID);
		if (spawnForm == NULL)
		{
			logger::info("Failed to Spawn. Form Invalid: {:x}", ID);
			return NULL;
		}
		RE::TESObjectREFR* target = GetRandomObjectInCell(wcell);
		RE::TESObjectREFR* spawned = PlaceAtMe(registry, 1, target, spawnForm, 1, false, false);
		return spawned;
	}

	GroupList SpawnGroupInCell(RE::BSScript::Internal::VirtualMachine* registry, GroupList Types, WorldCell wcell)
	{
		RE::TESObjectREFR* target = GetRandomObjectInCell(wcell);
		return SpawnGroupAtTarget(registry, Types, target, wcell.cell, wcell.world,0,1000);
	}

	GroupList SpawnGroupAtTarget(RE::BSScript::Internal::VirtualMachine* registry, GroupList Types, RE::TESObjectREFR* Target, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, int spawnradius, int HeightDistance)
	{
		RE::TESObjectREFR* spawned = NULL;
		srand(time(NULL));
		RE::NiPoint3 startingpoint = Target->GetPosition();

		for (std::uint32_t i = 0; i < Types.length; i++)
		{
			logger::info("Calling LookupFormByID");
			RE::TESForm* spawnForm = RE::TESForm::LookupByID(Types.data[i].FormId);
			if (spawnForm == NULL)
			{
				logger::info("Failed to Spawn. Form Invalid");
				return Types;
			}
			//If a model file path is set then change the form model.
			if (!strcmp(Types.data[i].ModelFilepath.c_str(), "") == 0)
			{
				RE::TESModel* pWorldModel = skyrim_cast<RE::TESModel*>(spawnForm);
				if (pWorldModel)
				{
					logger::info("GetModelName: %s", pWorldModel->GetModel());
					pWorldModel->SetModel(Types.data[i].ModelFilepath.c_str());
				}
			}
			if (strcmp(Types.data[i].BountyType.c_str(), "ENEMY") == 0 || 
				strcmp(Types.data[i].BountyType.c_str(), "ALLY") == 0 || 
				strcmp(Types.data[i].BountyType.c_str(), "PLACER") == 0)
			{
				bool placedsuccessfully = false;
				int giveupcount = 10; //It's possible that we'll never find anything valid. If that's the case give up. This is quite low as we are spawning something everytime we try this.
				while (!placedsuccessfully)
				{
					logger::info("placedsuccessfully");
					//Random Offset
					RE::NiPoint3 offset = RE::NiPoint3(rand() & spawnradius, rand() & spawnradius, 0);

					MoveRefToWorldCell(Target, cell, worldspace, startingpoint + offset, RE::NiPoint3(0, 0, rand() % 360));
					spawned = PlaceAtMe(registry, 1, Target, spawnForm, 1, true, false);
					
						int heightdist = startingpoint.z - spawned->GetPositionZ();
						//Delete
						if ((heightdist > HeightDistance || heightdist < -HeightDistance) && giveupcount > 0)
						{
							logger::info("Spawn Height is too different. Deleting.");
							MoveRefToWorldCell(spawned, cell, worldspace, RE::NiPoint3(0, 0, 10000), RE::NiPoint3(0, 0, 0));
							BountyManager::getInstance()->AddToDeleteList(spawned);
							giveupcount--;
						}
						else
						{
							placedsuccessfully = true;
						}
					
				}
				Types.data[i].objectRef = spawned;
				Types.data[i].isComplete = false;
			}
			else if (strcmp(Types.data[i].BountyType.c_str(), "BOUNTYDECORATION") == 0 ||
				strcmp(Types.data[i].BountyType.c_str(), "SPAWNEFFECT") == 0 ||
				strcmp(Types.data[i].BountyType.c_str(), "SCRIPTED") == 0 ||
				strcmp(Types.data[i].BountyType.c_str(), "SCRIPTEDDOOR") == 0)
			{
				if (spawned != NULL)
				{
					//Actors jump to the navmesh. Objects don't. This tries to used the jump to find the ground.
					RE::TESObjectREFR* decoration = PlaceAtMe(registry, 1, spawned, spawnForm, 1, true, false);
					Types.data[i].objectRef = decoration;
					Types.data[i].isComplete = false;
					Types.data[i].PreBounty();
				}
				else
				{
					RE::TESObjectREFR* decoration = PlaceAtMe(registry, 1, Target, spawnForm, 1, true, false);
					Types.data[i].objectRef = decoration;
					Types.data[i].isComplete = false;
					Types.data[i].PreBounty();
				}
			}
			else if (strcmp(Types.data[i].BountyType.c_str(), "PHYSICSSCRIPTED") == 0)
			{
				//We don't want these falling through the floor, so we put them in the air.
				RE::TESObjectREFR* PhysicsScripted = PlaceAtMe(registry, 1, spawned, spawnForm, 1, true, false);
				RE::NiPoint3 offset = RE::NiPoint3(0, 0, -1500);
				//MoveRefToWorldCell(PhysicsScripted, cell, worldspace, PhysicsScripted->pos + offset, NiPoint3(0, 0, 0));
				Types.data[i].objectRef = PhysicsScripted;
				Types.data[i].PreBounty();
			}
		}
		return Types;
	}

	std::vector<float> RiftRotations;
	RefList riftobjectrefs = RefList();

	RefList SpawnRift(RE::BSScript::Internal::VirtualMachine* registry, RE::TESObjectREFR* Target, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace)
	{
		//Debug
		srand(time(NULL));
		RE::NiPoint3 startingpoint = Target->GetPosition();  // +NiPoint3(rand() % 1000, rand() % 1000, rand() % 1000);


		riftobjectrefs = RefList();
		FormRefList formlist = GetRandomRift();
		RiftRotations = std::vector<float>();
		for (int i = 0; i < formlist.length; i++)
		{
			RE::TESForm* spawnForm = RE::TESForm::LookupByID(formlist.data[i].formId);
			if (spawnForm == NULL)
			{
				logger::info("Spawnform is null");
				continue;
			}
			RE::NiPoint3 position = startingpoint + formlist.data[i].pos;
			RE::NiPoint3 rotation = formlist.data[i].rot;
			rotation.x = rotation.x* (180.0 / 3.141592653589793238463);
			rotation.y = rotation.y* (180.0 / 3.141592653589793238463);
			rotation.z = rotation.z* (180.0 / 3.141592653589793238463);

			RE::TESObjectREFR* spawned = PlaceAtMe(registry, 1, Target, spawnForm, 1, true, false);
			spawned->GetReferenceRuntimeData().refScale = formlist.data[i].scale;
			spawned->GetPosition() = position;
			spawned->GetAngle() = rotation;
			//			MoveRefToWorldCell(spawned, cell, worldspace, position, rotation);


			logger::info("Spawn details: {}, {}, {}, {}, {}, {}", position.x, position.y, position.z, rotation.x, rotation.y, rotation.z);
			Ref newref = Ref();
			newref.objectRef = spawned;
			riftobjectrefs.AddItem(newref);
			RiftRotations.push_back(rotation.x);
			RiftRotations.push_back(rotation.y);
			RiftRotations.push_back(rotation.z);		
		}
		for (int i = 0; i < 20; i++)
		{
			SpawnMonsterAtRef(registry, 0x00039CFC, riftobjectrefs.data[i].objectRef, cell, worldspace);
		}
		return riftobjectrefs;
	}

	std::vector<float> GetRiftRotations()
	{
		return RiftRotations;
	}
	RefList GetCurrentRiftRefs()
	{
		return riftobjectrefs;
	}
}
