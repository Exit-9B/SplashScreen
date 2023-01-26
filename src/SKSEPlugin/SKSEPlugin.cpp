#include "main/SplashWindow.h"

namespace
{
	inline static std::optional<std::filesystem::path> LogDirectory;

	std::optional<std::filesystem::path> GetLogDirectory()
	{
		if (LogDirectory.has_value()) {
			return LogDirectory.value();
		}
		else {
			return logger::log_directory();
		}
	}

	void FindLogDirectory(REL::Version a_ver)
	{
		wchar_t* buffer{ nullptr };
		const auto result = ::SHGetKnownFolderPath(
			::FOLDERID_Documents,
			::KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT,
			nullptr,
			std::addressof(buffer));

		std::unique_ptr<wchar_t[], decltype(&::CoTaskMemFree)> knownPath(buffer, ::CoTaskMemFree);
		if (!knownPath || result != S_OK) {
			logger::error("failed to get known folder path"sv);
			return;
		}

		std::filesystem::path path = knownPath.get();
		path /= "My Games"sv;

		if (a_ver == SKSE::RUNTIME_VR_1_4_15_1) {
			path /= "Skyrim VR"sv;
		}
		else {
			path /= "Skyrim Special Edition"sv;
		}

		path /= "SKSE"sv;

		LogDirectory = path;
	}

	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = GetLogDirectory();
		if (!path) {
			util::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log"sv, Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
{
	SKSE::PluginVersionData v{};

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.AuthorName("Parapets"sv);

	v.UsesAddressLibrary(true);
	v.HasNoStructUse(true);
	v.UsesStructsPost629(false);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI
	SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	FindLogDirectory(ver);

	return true;
}

extern "C" DLLEXPORT void APIENTRY Initialize()
{
	if (SplashWindow::IsInitialized())
		return;

	InitializeLog();
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	SplashWindow::Initialize();
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	Initialize();

	SKSE::Init(a_skse);
	SKSE::GetMessagingInterface()->RegisterListener(
		[](auto a_msg)
		{
			switch (a_msg->type) {
			case SKSE::MessagingInterface::kInputLoaded:
				SplashWindow::Dismiss();
				break;
			}
		});

	return true;
}
