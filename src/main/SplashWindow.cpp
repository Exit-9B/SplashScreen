#include "SplashWindow.h"
#include "SplashImage.h"

bool SplashWindow::IsInitialized()
{
	return _splash != NULL;
}

void SplashWindow::Initialize()
{
	if (_splash) {
		return;
	}

	if (FAILED(::CoInitializeEx(NULL, ::COINIT_MULTITHREADED))) {
		logger::critical("Failed to initialize COM");
		return;
	}

	::HBITMAP hbmpSplash = SplashImage::LoadSplashImage();
	if (!hbmpSplash) {
		return;
	}

	RegisterWindowClass();

	_splash = CreateSplashWindow();
	if (!_splash) {
		logger::critical("Failed to create splash window");
		return;
	}

	SetSplashImage(_splash, hbmpSplash);
}

void SplashWindow::Dismiss()
{
	if (_splash) {
		::DestroyWindow(_splash);
	}
}

void SplashWindow::RegisterWindowClass()
{
	::WNDCLASS wc{};
	wc.lpfnWndProc = ::DefWindowProc;
	wc.hInstance = _hInstance;
	wc.hIcon = static_cast<::HICON>(
		::LoadImage(_hInstance, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = c_szSplashClass;
	if (!::RegisterClass(&wc)) {
		logger::critical("Failed to register window class");
	}
}

::HWND SplashWindow::CreateSplashWindow()
{
	::HWND hwndOwner =
		::CreateWindow(c_szSplashClass, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, _hInstance, NULL);

	if (!hwndOwner) {
		logger::critical("Failed to create owner window");
		return NULL;
	}

	return ::CreateWindowEx(
		WS_EX_LAYERED,
		c_szSplashClass,
		NULL,
		WS_POPUP | WS_VISIBLE,
		0,
		0,
		0,
		0,
		hwndOwner,
		NULL,
		_hInstance,
		NULL);
}

void SplashWindow::SetSplashImage(::HWND hwndSplash, ::HBITMAP hbmpSplash)
{
	::BITMAP bm{};
	if (!::GetObject(hbmpSplash, sizeof(bm), &bm)) {
		logger::critical("Failed to get object information");
		return;
	}

	::SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

	::POINT ptZero{};
	::HMONITOR hmonPrimary = ::MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	::MONITORINFO monitorinfo{};
	monitorinfo.cbSize = sizeof(monitorinfo);
	if (!::GetMonitorInfo(hmonPrimary, &monitorinfo)) {
		logger::critical("Failed to get display monitor information");
		return;
	}

	const ::RECT& rcWork = monitorinfo.rcWork;
	::POINT ptOrigin{};
	ptOrigin.x = rcWork.left + (rcWork.right - rcWork.left - sizeSplash.cx) / 2;
	ptOrigin.y = rcWork.top + (rcWork.bottom - rcWork.top - sizeSplash.cy) / 2;

	DCSharedPtr hdcScreen{ ::GetDC(NULL), &ReleaseDCPtr };
	if (!hdcScreen) {
		logger::critical("Failed to get device context (DC)");
		return;
	}

	DCPtr hdcMem{ ::CreateCompatibleDC(hdcScreen.get()), &::DeleteDC };
	if (!hdcMem) {
		logger::critical("Failed to create memory device context (DC)");
		return;
	}

	::HBITMAP hbmpOld = static_cast<::HBITMAP>(::SelectObject(hdcMem.get(), hbmpSplash));
	if (!hbmpOld || hbmpOld == HGDI_ERROR) {
		logger::critical("Failed to select object");
		return;
	}

	::BLENDFUNCTION blend{};
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	if (!::UpdateLayeredWindow(
		hwndSplash,
		hdcScreen.get(),
		&ptOrigin,
		&sizeSplash,
		hdcMem.get(),
		&ptZero,
		RGB(0, 0, 0),
		&blend,
		ULW_ALPHA)) {

		logger::critical("Failed to update layered window");
	}

	::SelectObject(hdcMem.get(), hbmpOld);
}
