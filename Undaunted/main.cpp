#include <MyPlugin.h>
#include <BountyManager.h>
#include <ConfigUtils.h>
#include <SKSELink.h>
#include <StartupManager.h>

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	switch (a_message->type) {
	case SKSE::MessagingInterface::kPreLoadGame:
		//We're loading the game. Clear up any bounty data.
		logger::info("kMessage_PreLoadGame rechieved, clearing bounty data.");
		if (Undaunted::BountyManager::getInstance()->activebounties.length > 0) {
			for (std::uint32_t i = 0; i < Undaunted::BountyManager::getInstance()->activebounties.length; i++) {
				Undaunted::BountyManager::getInstance()->ClearBountyData(i);
			}
		}
	case SKSE::MessagingInterface::kNewGame:
	case SKSE::MessagingInterface::kPostLoad:
	case SKSE::MessagingInterface::kPostLoadGame:
	case SKSE::MessagingInterface::kPostPostLoad:
	default:
		break;
	}

	//Register to recieve interface from Enchantment Framework
	//if (msg->type == SKSEMessagingInterface::kMessage_PostLoad)

	//kMessage_InputLoaded only sent once, on initial Main Menu load
	//else if (msg->type == SKSEMessagingInterface::kMessage_InputLoaded)
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName(Version::PROJECT);
	v.AuthorName("Nightfallstorm");
	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST_AE });
	v.HasNoStructUse(true);

	return v;
}();


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;
	logger::info("Undaunted");
	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();

	return true;
}

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		//stl::report_and_fail("Failed to find standard logging directory"sv); // Doesn't work in VR
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {  // Called by SKSE to load this plugin
		InitializeLog();
		SKSE::Init(a_skse);
		logger::info("Undaunted");
		logger::info("Loading Undaunted..");
		
		//TODO FIX
		//auto messaging = SKSE::GetMessagingInterface();
		//messaging->RegisterListener(MessageHandler);

		//Check if the function registration was a success...
		logger::info("RegisterListener");

		auto papyrus = SKSE::GetPapyrusInterface();
		logger::info("Grabbed Papyrus Interface");
		papyrus->Register(UndauntedPapyrus::RegisterFuncs);
		logger::info("Register Succeeded");

		logger::info("SKSEPlugin_Load Succeeded");
		return true;
}
