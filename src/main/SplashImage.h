#pragma once

class SplashImage final
{
public:
	static ::HBITMAP LoadSplashImage();

private:
	static ::IStreamPtr CreateStream(::LPCWSTR pszFile);
	static ::IWICBitmapSourcePtr LoadBitmapFromStream(::IStreamPtr ipImageStream);
	static ::HBITMAP CreateHBITMAP(::IWICBitmapSourcePtr ipBitmap);
};

