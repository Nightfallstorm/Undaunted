#include "BountyManager.h"
#include <common\ITypes.h>
#include "RE/PlayerCharacter.h"
#include "RE/TESWorldSpace.h"
#include "RE/BGSMessage.h"

namespace Undaunted {
	BountyManager* BountyManager::instance = 0;
	BountyManager* BountyManager::getInstance()
	{
		if (instance == 0)
		{
			instance = new BountyManager();
		}

		return instance;
	}


	bool BountyManager::BountyUpdate()
	{
		if (bountywave == 0 && bountyworldcell.world != NULL)
		{
			//Is the player in the right worldspace?

			RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
			
			if (_stricmp(player->GetWorldspace()->GetFullName(), bountyworldcell.world->GetFormEditorID()) == 0)
			{
				_MESSAGE("Player in Worldspace");
				//Check the distance to the XMarker
				Vector3 pos = Vector3(player->GetPosition().x, player->GetPosition().y, player->GetPosition().z);
				Vector3 markref = Vector3(xmarkerref->GetPosition().x, xmarkerref->GetPosition().y, xmarkerref->GetPosition().z);
				Vector3 distance = pos - markref;
				Vector3 distvector = Vector3(distance.x, distance.y, distance.z);
				_MESSAGE("Distance to marker: %f", distvector.Magnitude());
				
				if (distvector.Magnitude() < GetConfigValueInt("BountyStartDistance"))
				{
					bountygrouplist = SpawnGroupAtTarget(_registry, bountygrouplist, xmarkerref, bountyworldcell.cell, bountyworldcell.world);
					_MESSAGE("Enemy Count : %08X ", bountygrouplist.length);
					bountywave = 1;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		if (bountygrouplist.length == 0)
			return false;

		for (UInt32 i = 0; i < bountygrouplist.length; i++)
		{
			if (bountygrouplist.data[i].IsComplete() != 1)
			{
				MoveRefToWorldCell(xmarkerref, bountyworldcell.cell, bountyworldcell.world, bountygrouplist.data[i].objectRef->GetPosition(), RE::NiPoint3(0, 0, 0));
				return false;
			}
					
		}
		_MESSAGE("Starting PostBounty");
		for (UInt32 i = 0; i < bountygrouplist.length; i++)
		{
			bountygrouplist.data[i].PostBounty();
		}
		return true;
	}

	float BountyManager::StartBounty(bool nearby)
	{
		srand(time(NULL));
		if (xmarkerref == NULL)
		{
			_MESSAGE("NO XMARKER SET");
			return 0;
		}
		if (bountymessageref == NULL)
		{
			_MESSAGE("NO BOUNTYMESSAGEREF SET");
			return 0;
		}
		if (!isReady)
		{
			_MESSAGE("System not initialised, run InitSystem before starting any bounties");
			return 0;
		}
		//Cleanup previous bounties
		ClearBountyData();

		RE::TESObjectREFR* target = NULL;
		if (!nearby )
		{	
			bountyworldcell = GetNamedWorldCell((*g_thePlayer)->currentWorldSpace->editorId.Get());
			target = GetRandomObjectInCell(bountyworldcell.cell);
		}
		else
		{
			int loopcounts = 0;
			int BountyMinSpawnDistance = GetConfigValueInt("BountyMinSpawnDistance");
			int BountyMaxSpawnDistance = GetConfigValueInt("BountyMaxSpawnDistance");
			int BountySearchAttempts = GetConfigValueInt("BountySearchAttempts");

			RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

			bool foundtarget = false;
			while (!foundtarget)
			{
				bountyworldcell = GetNamedWorldCell(player->GetWorldspace()->GetFormEditorID());
				target = GetRandomObjectInCell(bountyworldcell.cell);
				RE::NiPoint3 distance = player->GetPosition() - target->GetPosition();
				Vector3 distvector = Vector3(distance.x, distance.y, distance.z);
				_MESSAGE("Distance to Bounty: %f", distvector.Magnitude());
				if (distvector.Magnitude() > BountyMinSpawnDistance && distvector.Magnitude() < BountyMaxSpawnDistance)
				{
					foundtarget = true;
				}
				loopcounts++;
				if (loopcounts > BountySearchAttempts)
				{
					//Can't find anything. Give up and use this cell.
					bountyworldcell.cell = player->parentCell;
					bountyworldcell.world = player->GetWorldspace();
					target = GetRandomObjectInCell(bountyworldcell.cell);
					foundtarget = true;
				}
			}
		}
		_MESSAGE("target is set. Moving marker: WorldSpace: %s Cell: %08X ", bountyworldcell.world->GetFormEditorID(), bountyworldcell.cell->formID);
		MoveRefToWorldCell(xmarkerref, bountyworldcell.cell, bountyworldcell.world, target->GetPosition(), RE::NiPoint3(0, 0, 0));

		bountygrouplist = GetRandomGroup();
		_MESSAGE("Setting Bounty Message: %s", bountygrouplist.questText);
		bountymessageref->fullName = bountygrouplist.questText;

		return 0;
	}

	void BountyManager::ClearBountyData() {

		for (int i = 0; i < bountygrouplist.length; i++)
		{
			//Clear all completed flags
			bountygrouplist.data[i].isComplete = false;
			bountygrouplist.data[i].objectRef = NULL;
		}
		bountywave = 0;
		bountygrouplist = GroupList();

		//If there's been a reload then the bounty currently breaks. Inform the user.
		if (isReady)
		{
			_MESSAGE("Setting Bounty Message: The Bounty has moved on, return to the Undaunted Camp to start a new Bounty");
			//bountymessageref->fullName.name = "The Bounty has moved on, return to the Undaunted Camp to start a new Bounty";
		}
	}

}
