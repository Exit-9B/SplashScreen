#include "SplashImage.h"

::HBITMAP SplashImage::LoadSplashImage()
{
	::HBITMAP hbmpSplash = NULL;

	::IStreamPtr ipImageStream = CreateStream(LR"(Data\Interface\splash.png)");
	if (!ipImageStream) {
		return NULL;
	}

	::IWICBitmapSourcePtr ipBitmap = LoadBitmapFromStream(ipImageStream);
	if (!ipBitmap) {
		return NULL;
	}

	hbmpSplash = CreateHBITMAP(ipBitmap);
	return hbmpSplash;
}

::IStreamPtr SplashImage::CreateStream(::LPCWSTR pszFile)
{
	::IStream* pstm;
	if (FAILED(::SHCreateStreamOnFileEx(pszFile, STGM_FAILIFTHERE, 0, FALSE, NULL, &pstm))) {
		logger::critical("Failed to open file");
		return NULL;
	}

	return pstm;
}

::IWICBitmapSourcePtr SplashImage::LoadBitmapFromStream(::IStreamPtr ipImageStream)
{
	::IWICBitmapDecoderPtr ipDecoder;
	if (FAILED(ipDecoder.CreateInstance(::CLSID_WICPngDecoder, NULL, ::CLSCTX_INPROC_SERVER))) {
		logger::critical("Failed to create PNG decoder");
		return NULL;
	}

	if (FAILED(ipDecoder->Initialize(ipImageStream, ::WICDecodeMetadataCacheOnLoad))) {
		logger::critical("Failed to initialize decoder with stream");
		return NULL;
	}

	::UINT nFrameCount = 0;
	if (FAILED(ipDecoder->GetFrameCount(&nFrameCount)) || nFrameCount != 1) {
		logger::critical("Unexpected frame count: {}", nFrameCount);
		return NULL;
	}

	::IWICBitmapFrameDecode* ipFrame = NULL;
	if (FAILED(ipDecoder->GetFrame(0, &ipFrame))) {
		logger::critical("Failed to decode frame");
		return NULL;
	}

	::IWICBitmapSource* ipBitmap = NULL;
	if (FAILED(::WICConvertBitmapSource(::GUID_WICPixelFormat32bppPBGRA, ipFrame, &ipBitmap))) {
		logger::critical("Failed to convert image to 32bpp BGRA format");
		return NULL;
	}

	return ipBitmap;
}

::HBITMAP SplashImage::CreateHBITMAP(::IWICBitmapSourcePtr ipBitmap)
{
	::UINT width = 0;
	::UINT height = 0;
	if (FAILED(ipBitmap->GetSize(&width, &height)) || width == 0 || height == 0) {
		logger::critical("Failed to get bitmap size");
		return NULL;
	}

	::BITMAPINFO bminfo;
	::ZeroMemory(&bminfo, sizeof(bminfo));
	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = width;
	bminfo.bmiHeader.biHeight = -(static_cast<::LONG>(height));
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 32;
	bminfo.bmiHeader.biCompression = BI_RGB;

	void* pvImageBits = NULL;

	::HBITMAP hbmp = NULL;
	{
		DCSharedPtr hdcScreen{ ::GetDC(NULL), ReleaseDCPtr };
		hbmp = ::CreateDIBSection(hdcScreen.get(), &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
	}

	if (!hbmp) {
		logger::critical("Failed to create DIB section");
		return NULL;
	}

	const ::UINT cbStride = width * 4;
	const ::UINT cbImage = cbStride * height;
	if (FAILED(ipBitmap->CopyPixels(NULL, cbStride, cbImage, static_cast<::BYTE*>(pvImageBits)))) {
		::DeleteObject(hbmp);
		logger::critical("Failed to extract image");
		hbmp = NULL;
	}

	return hbmp;
}
