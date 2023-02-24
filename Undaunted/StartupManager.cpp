#include <ConfigUtils.h>
#include "StartupManager.h"
#include "RSJparser.tcc"
#include <filesystem>
#include <algorithm>
#include <string>
#include <FormRefList.h>
#include <LocationUtils.h>

namespace Undaunted {
	RSJresource currentfile;
	void LoadJson(const char* filepath)
	{
		logger::info("Loading {}", filepath);
		std::ifstream t(filepath);
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		std::string buffer(size, ' ');
		t.seekg(0);
		t.read(&buffer[0], size);
		RSJresource my_json(buffer.c_str());
		currentfile = my_json;
	}

	void LoadSettings()
	{
		LoadJson("Data/Undaunted/Settings.json");
		RSJresource settings = currentfile; 

		auto data = settings.as_array();
		logger::info("size: {}", data.size());
		for (int i = 0; i < data.size(); i++)
		{
			auto inner = data[i].as_array();
			std::string key = inner[0].as<std::string>("default string");
			std::string value = inner[1].as<std::string>("default string");
			AddConfigValue(key.c_str(), value.c_str());
		}

		LoadJson("Data/Undaunted/RewardModBlacklist.json");
		auto modblacklist = currentfile.as_array();
		for (int i = 0; i < modblacklist.size(); i++)
		{
			std::string modname = modblacklist[i].as<std::string>("default string");
			AddRewardBlacklist(modname);
			logger::info("RewardModBlacklist modname: %s", modname.c_str());
		}

		LoadJson("Data/Undaunted/Safezones.json");
		auto Safezones = currentfile.as_array();
		for (int i = 0; i < Safezones.size(); i++)
		{
			auto obj = Safezones[i].as_object();
			std::string Zonename = obj.at("Zonename").as<std::string>("default string");
			std::string Worldspace = obj.at("Worldspace").as<std::string>("default string");
			int PosX = obj.at("PosX").as<int>();
			int PosY = obj.at("PosY").as<int>();
			int PosZ = obj.at("PosZ").as<int>();
			int Radius = obj.at("Radius").as<int>();
			Safezone zone = Safezone();
			zone.Zonename = Zonename;
			zone.Worldspace = Worldspace;
			zone.PosX = PosX;
			zone.PosY = PosY;
			zone.PosZ = PosZ;
			zone.Radius = Radius;
			AddSafezone(zone);
			logger::info("Safezone {}, {}, {}, {}, {}, {}", Zonename.c_str(), Worldspace.c_str(), PosX, PosY, PosZ, Radius);
		}
	}

	//With creation club there's now a need to load forms from ESL's.
	int getFormId(std::string mod, int form)
	{
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		const RE::TESFile* modInfo = dataHandler->LookupModByName(mod.c_str());
		int tempform = form;
		if (modInfo != NULL)
		{
			tempform = (modInfo->compileIndex << 24) + tempform;
		}
		if (tempform < 0) // Probably trying to load from an ESL.
		{
			//ESL Load order
			for (auto modinfo: dataHandler->compiledFileCollection.smallFiles)
			{
				if (_stricmp(modinfo->fileName, mod.c_str()) == 0)			
				{
					// modIndex lightIndex FormId
					// 0xFE FFF 000
					tempform = (modInfo->compileIndex << 24) + (modinfo->smallFileCompileIndex << 12) + form;
				}
			}
		}
		form = tempform;
		logger::info("modIndex: ", modInfo->compileIndex);
		logger::info("form id: {}", form);
		return form;
	}

	void LoadGroups()
	{
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		logger::info("Loading Groups...");
		std::string path = "Data/Undaunted/Groups";
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::wstring wstring(entry.path().native());
			std::string filename;
			std::transform(wstring.begin(), wstring.end(), std::back_inserter(filename), [](wchar_t c) {
				return (char)std::tolower((char)c);
			});
			logger::info("file: {}", filename.c_str());
			if (entry.is_regular_file())
			{
				LoadJson(filename.c_str());
				RSJresource settings = currentfile;
				auto data = settings.as_array();
				logger::info("size: {}", data.size());
				for (int i = 0; i < data.size(); i++)
				{
					auto group = data[i].as_array();
					std::string groupname = group[0][0].as<std::string>("groupname");
					std::string modreq = group[0][1].as<std::string>("modreq");
					int minlevel = group[0][2].as<int>(0);
					int maxlevel = group[0][3].as<int>(0);
					std::string tags = group[0][4].as<std::string>("notags");
					
					std::string delimiter = ",";
					size_t pos = 0;
					std::string token;
					UnStringlist taglist = UnStringlist();
					while ((pos = tags.find(delimiter)) != std::string::npos) {
						token = tags.substr(0, pos);
						std::transform(token.begin(), token.end(), token.begin(), ::toupper);
						logger::info("tags: {}", token.c_str());
						taglist.AddItem(token);
						tags.erase(0, pos + delimiter.length());
					}
					std::transform(tags.begin(), tags.end(), tags.begin(), ::toupper);
					taglist.AddItem(tags);
					const RE::TESFile* modInfo = dataHandler->LookupModByName(modreq.c_str());
					if (modInfo != NULL && modInfo->compileIndex != 0xFF)
					{
						logger::info("tags: {}", tags.c_str());
						int groupid = AddGroup(groupname, minlevel, maxlevel, taglist);
						for (int j = 1; j < group.size(); j++)
						{
							std::string esp = group[j][1].as<std::string>("esp");
							const RE::TESFile* modInfo = dataHandler->LookupModByName(esp.c_str());
							int form = getFormId(esp,group[j][2].as<int>(0));
							std::string type = group[j][3].as<std::string>("type");
							std::string model = std::string("");
							if (group[j].size() > 3)
							{
								model = group[j][4].as<std::string>("");
							}
							std::transform(type.begin(), type.end(), type.begin(), ::toupper);
							GroupMember newmember = GroupMember();
							newmember.BountyType = type;
							newmember.FormId = form;
							newmember.ModelFilepath = model.c_str();
							AddMembertoGroup(groupid, newmember);
						}
					}
				}
			}
		}
		logger::info("Groups loaded: {}", GetGroupCount());
	}
}
