#include "BountyManager.h"

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


	bool BountyManager::BountyUpdate(int BountyID)
	{
		//The bounty list has changed. Probably from a reload.
		if (BountyID > activebounties.length)
		{
			return false;
		}
		Bounty* bounty = &activebounties.data[BountyID];
		logger::info("BountyUpdate BountyID: %08X", BountyID);
		logger::info("BountyID bountywave: %i", bounty->bountywave);
		if (bounty->bountywave == 0 && bounty->bountyworldcell.world != NULL)
		{
			//Is the player in the right worldspace?
			if (_stricmp(GetCurrentWorldspaceName().Get(), bounty->bountyworldcell.world->editorId.Get()) == 0)
			{
				logger::info("Player in Worldspace");
				//Check the distance to the XMarker
				RE::NiPoint3 distance = GetPlayer()->pos - bounty->xmarkerref->GetPosition();
				Vector3 distvector = Vector3(distance.x, distance.y, distance.z);
				int startdis = GetConfigValueInt("BountyStartDistance");
				logger::info("Distance to marker: %f / %i", distvector.Magnitude(), startdis);
				if (distvector.Magnitude() < startdis)
				{
					logger::info("Calling SpawnGroupAtTarget");
					bounty->bountygrouplist = SpawnGroupAtTarget(_registry, bounty->bountygrouplist, bounty->xmarkerref, bounty->bountyworldcell.cell, bounty->bountyworldcell.world, 
						GetConfigValueInt("BountyEnemyExteriorSpawnRadius"), GetConfigValueInt("BountyEnemyPlacementHeightDistance"));
					logger::info("Enemy Count : %08X ", bounty->bountygrouplist.length);
					bounty->bountywave = 1;
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

		if (bounty->bountygrouplist.length == 0)
			return false;

//		bool NoncompleteObj = false;
		int NonComplete = 0;
		for (std::uint32_t i = 0; i < bounty->bountygrouplist.length; i++)
		{
			logger::info("Type, Member, complete: %s, %08X , %i", bounty->bountygrouplist.data[i].BountyType.c_str(), bounty->bountygrouplist.data[i].FormId, bounty->bountygrouplist.data[i].IsComplete());
			if (bounty->bountygrouplist.data[i].IsComplete() != 1)
			{
				NonComplete++;
				if (bounty->bountygrouplist.data[i].objectRef != NULL)
				{
					MoveRefToWorldCell(bounty->xmarkerref, bounty->bountyworldcell.cell, bounty->bountyworldcell.world, bounty->bountygrouplist.data[i].objectRef->pos, NiPoint3(0, 0, 0));
				}
			}
		}
		if(NonComplete > 0)
			return false;

//		if (NoncompleteObj)
	//		return false;

		logger::info("Starting PostBounty");
		for (std::uint32_t i = 0; i < bounty->bountygrouplist.length; i++)
		{
			bounty->bountygrouplist.data[i].PostBounty();
		}
		return true;
	}


	FormRefList* previoustargets;
	int cardinality = -1;

	float BountyManager::StartBounty(int BountyID,bool nearby, const char* BountyName,TESObjectREFR* ref,BSFixedString WorldSpaceName, std::string bountyTag)
	{
		Bounty* bounty = &activebounties.data[BountyID];
		srand(time(NULL));
		logger::info("time %i", time(NULL));
		if (previoustargets == NULL)
		{
			previoustargets = new FormRefList();
		}
		else
		{
			logger::info("locations in memory: %i", previoustargets->length);
		}
		if (bounty->xmarkerref == NULL)
		{
			logger::info("NO XMARKER SET");
			return 0;
		}
		if (bounty->bountymessageref == NULL)
		{
			logger::info("NO BOUNTYMESSAGEREF SET");
			return 0;
		}
		if (!isReady)
		{
			logger::info("System not initialised, run InitSystem before starting any bounties");
			return 0;
		}
		if (cardinality == -1)
		{
			//Ok so bounties currently spawn randomly without this setting.
			//This can lead to a lot of backtracking which doesn't feel great.
			//Cardinality is an attempt to force the bounties to spawn in a general compass direction, meaning you'll continually move forward.
			cardinality = rand() % 4; // 0 = N, 1 = E, 2 = S, 3 = W
		}
		//Cleanup previous bounties
		ClearBountyData(BountyID);

		TESObjectREFR* target = NULL;
		if (!nearby )
		{
			if (ref == NULL)
			{
				logger::info("ref == NULL");
				bounty->bountyworldcell = GetNamedWorldCell(GetCurrentWorldspaceName().Get());
			}
			else
			{
				bounty->bountyworldcell = GetNamedWorldCell(WorldSpaceName.Get());
			}
			target = GetRandomObjectInCell(bounty->bountyworldcell);
		}
		else
		{
			int loopcounts = 0;
			int BountyMinSpawnDistance = GetConfigValueInt("BountyMinSpawnDistance");
			int BountyMaxSpawnDistance = GetConfigValueInt("BountyMaxSpawnDistance");
			int BountySearchAttempts = GetConfigValueInt("BountySearchAttempts");
			int BountyMaxHeight = GetConfigValueInt("BountyMaxHeight");
			int BountyMinHeight = GetConfigValueInt("BountyMinHeight");
			bool foundtarget = false;
			logger::info("Searching for next location");
			while (!foundtarget)
			{
				NiPoint3 distance;
				if (ref == NULL)
				{
					//logger::info("ref == NULL");
					bounty->bountyworldcell = GetNamedWorldCell(GetCurrentWorldspaceName().Get());
					target = GetRandomObjectInCell(bounty->bountyworldcell);
					distance = GetPlayer()->pos - target->pos;
				}
				else
				{
					//logger::info("ref != NULL ");
					//logger::info("WorldSpaceName: %s", WorldSpaceName.Get());
					bounty->bountyworldcell = GetNamedWorldCell(WorldSpaceName.Get());
					target = GetRandomObjectInCell(bounty->bountyworldcell);
					distance = ref->pos - target->pos;
				}
				Vector3 distvector = Vector3(distance.x, distance.y, distance.z);
				//logger::info("Distance to Bounty: %f", distvector.Magnitude());
				//logger::info("Distance %f, Height: %f", distvector.Magnitude(), target->pos.z);
				if (distvector.Magnitude() > BountyMinSpawnDistance && 
					distvector.Magnitude() < BountyMaxSpawnDistance && 
					target->pos.z < GetPlayer()->pos.z + BountyMaxHeight &&
					target->pos.z > GetPlayer()->pos.z - BountyMinHeight)
				{
					//Check if we've used this location before in memory.
					bool usedalready = false;
					for (int i = 0; i < previoustargets->length; i++)
					{
						if (target->pos.x == previoustargets->data[i].pos.x &&
							target->pos.y == previoustargets->data[i].pos.y &&
							target->pos.z == previoustargets->data[i].pos.z &&
							target->rot.x == previoustargets->data[i].rot.x &&
							target->rot.y == previoustargets->data[i].rot.y &&
							target->rot.z == previoustargets->data[i].rot.z)
						{
							usedalready = true;
							break;
						}
					}

					//Cardinality is an attempt to force the bounties to spawn in a general compass direction, meaning you'll continually move forward.
					if (previoustargets->length > 0)
					{
						switch (cardinality)
						{
						case 0: //North
							if (target->pos.y < previoustargets->data[previoustargets->length - 1].pos.y)
							{
								usedalready = true;
							}
							break;
						case 1: //East
							if (target->pos.x < previoustargets->data[previoustargets->length - 1].pos.x)
							{
								usedalready = true;
							}
						case 2: //South
							if (target->pos.y > previoustargets->data[previoustargets->length - 1].pos.y)
							{
								usedalready = true;
							}
							break;
						case 3: //West
							if (target->pos.x > previoustargets->data[previoustargets->length - 1].pos.x)
							{
								usedalready = true;
							}
							break;

						default:
							break;
						}
					}

					if (!usedalready)
					{
						foundtarget = true;
						//We capture the form we're using, so we can prevent reuse.
						FormRef* targetForm = new FormRef();
						targetForm->pos = target->pos;
						targetForm->rot = target->rot;
						previoustargets->AddItem(*targetForm);
					}
				}
				loopcounts++;
				if (loopcounts == BountySearchAttempts)
				{
					logger::info("Having trouble finding anything at all. Try resetting cardinailty and memory");
					cardinality = rand() % 4; // 0 = N, 1 = E, 2 = S, 3 = W
					previoustargets = new FormRefList();
				}
				if (loopcounts > BountySearchAttempts * 2)
				{
					logger::info("Can't find anything. Give up and use any cell");
					if (strcmp(WorldSpaceName.Get(), "") != 0)
					{
						bounty->bountyworldcell = GetNamedWorldCell(WorldSpaceName.Get());
					}
					else
					{
						bounty->bountyworldcell = GetNamedWorldCell(GetCurrentWorldspaceName().Get());
					}
					target = GetRandomObjectInCell(bounty->bountyworldcell);
					foundtarget = true;
					//This could happen if you reach the end of the map and there's nothing to find in your current travel direction. So reset the cardinailty.
					cardinality = -1;

					//We capture the form we're using, so we can prevent reuse.
					FormRef* targetForm = new FormRef();
					targetForm->pos = target->pos;
					targetForm->rot = target->rot;
					previoustargets->AddItem(*targetForm);
				}
			}
		}
		logger::info("target is set. Moving marker: WorldSpace: %s Cell: %08X ", bounty->bountyworldcell.world->editorId.Get(), bounty->bountyworldcell.cell->formID);
		MoveRefToWorldCell(bounty->xmarkerref, bounty->bountyworldcell.cell, bounty->bountyworldcell.world, target->pos, NiPoint3(0, 0, 0));

		bool foundbounty = false;
		//We do our best but if someone has ran 50 bounties without traveling there's not much we can do.
		logger::info("BountyName == %s", BountyName);
		for (int i = 0; i < 50 && !foundbounty; i++)
		{
			if (_stricmp(BountyName, "") == 0)
			{
				if (_stricmp(bountyTag.c_str(), "") == 0)
				{
					bounty->bountygrouplist = GetRandomGroup();
				}
				else
				{
					bounty->bountygrouplist = GetRandomTaggedGroup(bountyTag);
				}
			}
			else
			{
				bounty->bountygrouplist = GetGroup(std::string(BountyName));
			}
			bool bountyran = false;
			for (int j = 0; j < bountiesRan.length; j++)
			{				
				if (bountiesRan.data[j].key.compare(bounty->bountygrouplist.questText) == 0)
				{
					bountyran = true;
				}
			}
			if (!bountyran)
			{
				foundbounty = true;
			}
		}
		UnKeyValue bountydata = UnKeyValue();
		bountydata.key = bounty->bountygrouplist.questText;

		bool repeatable = false;
		for (int i = 0; i < bounty->bountygrouplist.Tags.length; i++)
		{
			if (bounty->bountygrouplist.Tags.data[i].compare("REPEATABLE") == 0)
			{
				logger::info("Found Tag: %s", bounty->bountygrouplist.Tags.data[i].c_str());
				repeatable = true;
			}
		}
		if (!repeatable)
		{
			bountiesRan.AddItem(bountydata);
		}

		logger::info("Setting Bounty Message: %s", bounty->bountygrouplist.questText.c_str());
		bounty->bountymessageref->fullName.name = bounty->bountygrouplist.questText.c_str();
		logger::info("PlayerPos %f, %f, %f", GetPlayer()->pos.x, GetPlayer()->pos.y, GetPlayer()->pos.z);
		return 0;
	}

	float BountyManager::restartBounty(int BountyID,const char* BountyName)
	{
		Bounty* bounty = &activebounties.data[BountyID];
		srand(time(NULL));
		ClearBountyData(BountyID);
		bool foundbounty = false;
		bounty->bountygrouplist = GetGroup(std::string(BountyName));
		bounty->bountyworldcell = GetWorldCellFromRef(bounty->xmarkerref);
		logger::info("GetWorldCellFromRef World: %s", bounty->bountyworldcell.world->editorId.Get());
		bounty->bountymessageref->fullName.name = bounty->bountygrouplist.questText.c_str();
		//BountyUpdate();
		return 0.0f;
	}

	void BountyManager::ClearBountyData(int BountyID) {
		Bounty bounty = activebounties.data[BountyID];
		for (int i = 0; i < bounty.bountygrouplist.length; i++)
		{
			//Clear all completed flags
			bounty.bountygrouplist.data[i].isComplete = false;
			bounty.bountygrouplist.data[i].objectRef = NULL;
		}
		bounty.bountywave = 0;
		bounty.bountygrouplist = GroupList();
		/*
		//If there's been a reload then the bounty currently breaks. Inform the user.
		if (isReady)
		{
			bountymessageref->fullName.name = "The Bounty has moved on, start a new Bounty";
		}*/
		logger::info("ClearBountyData Complete");
	}

	void BountyManager::ResetBountiesRan()
	{
		//This plays a more important role than the code suggests.
		//Micro dungeons require a cell reset to repopulate the enemies.
		//They are flagged as waiting to reset once you enter/leave them, however this only happens when the cell is unloaded.
		//Unloading the cell happens on game load OR when the player fast travels a certain distance.
		//So we watch to see when the player fast travels far enough and then allow the microdungeon back on the allowed bounty list.
		bountiesRan = UnDictionary();
	}

	void BountyManager::AddToDeleteList(TESObjectREFR* ref)
	{
		logger::info("AddToDeleteList");
		Ref newref = Ref();
		newref.objectRef = ref;
		deleteList.AddItem(newref);
	}

	void BountyManager::ClearDeleteList()
	{
		logger::info("ClearDeleteList");
		deleteList = RefList();
	}

	RefList BountyManager::StartRift(int BountyID, TESObjectREFR* Startpoint)
	{
		//Bounty bounty = activebounties.data[BountyID];
		RefList refs = SpawnRift(_registry, Startpoint, Startpoint->parentCell, GetPlayer()->currentWorldSpace);
		return refs;
	}

}

