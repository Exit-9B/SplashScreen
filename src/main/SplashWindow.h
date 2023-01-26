#pragma once

#include <windef.h>

class SplashWindow
{
public:
	static bool IsInitialized();
	static void Initialize();
	static void Dismiss();

private:
	inline static const ::TCHAR* c_szSplashClass = TEXT("SplashWindow");

	static void RegisterWindowClass();
	static ::HWND CreateSplashWindow();
	static void SetSplashImage(::HWND hwndSplash, ::HBITMAP hbmpSplash);

	inline static ::HINSTANCE _hInstance = NULL;
	inline static ::HWND _splash = NULL;
};
