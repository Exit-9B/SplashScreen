#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <ObjIdl.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <comdef.h>
#include <objbase.h>
#include <wincodec.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Windowscodecs.lib")

#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;

namespace logger = SKSE::log;

namespace util
{
	using SKSE::stl::report_and_fail;
}

_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoder, __uuidof(::IWICBitmapDecoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapSource, __uuidof(::IWICBitmapSource));

inline void ReleaseDCPtr(::HDC a_hDC) {
	::ReleaseDC(NULL, a_hDC);
}

using DCPtr = std::unique_ptr<::HDC__, decltype(&::DeleteDC)>;
using DCSharedPtr = std::shared_ptr<::HDC__>;

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"
