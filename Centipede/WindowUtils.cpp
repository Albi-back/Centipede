
#include <cassert>

#include "WindowUtils.h"
#include "D3D.h"
#include "D3DUtil.h"

using namespace std;

//__int64 WinUtil::sTime1 = 0;


HINSTANCE WinUtil::GetAppInst()
{
	return mWinData.hAppInst;
}

HWND WinUtil::GetMainWnd()
{
	return mWinData.hMainWnd;
}

void WinUtil::GetClientExtents(int& width, int& height) const
{
	width = mWinData.clientWidth;
	height = mWinData.clientHeight;
}

LRESULT WinUtil::RealDefaultMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mWinData.appPaused = true;
		}
		else
		{
			mWinData.appPaused = false;
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mWinData.clientWidth = LOWORD(lParam);
		mWinData.clientHeight = HIWORD(lParam);
		if (mpMyD3D && mpMyD3D->GetDeviceReady())
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mWinData.appPaused = true;
				mWinData.minimized = true;
				mWinData.maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mWinData.appPaused = false;
				mWinData.minimized = false;
				mWinData.maximized = true;
				mpMyD3D->OnResize(mWinData.clientWidth, mWinData.clientHeight, *mpMyD3D);
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mWinData.minimized)
				{
					mWinData.appPaused = false;
					mWinData.minimized = false;
					mpMyD3D->OnResize(mWinData.clientWidth, mWinData.clientHeight,*mpMyD3D);
				}

				// Restoring from maximized state?
				else if (mWinData.maximized)
				{
					mWinData.appPaused = false;
					mWinData.maximized = false;
					mpMyD3D->OnResize(mWinData.clientWidth, mWinData.clientHeight,*mpMyD3D);
				}
				else if (mWinData.resizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					mpMyD3D->OnResize(mWinData.clientWidth, mWinData.clientHeight, *mpMyD3D);
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mWinData.appPaused = true;
		mWinData.resizing = true;
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mWinData.appPaused = false;
		mWinData.resizing = false;
		if(mpMyD3D)
			mpMyD3D->OnResize(mWinData.clientWidth, mWinData.clientHeight, *mpMyD3D);
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}



bool WinUtil::InitMainWindow(int width, int height, HINSTANCE hInstance, const std::string& appName, WNDPROC mssgHandler, bool centred)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	assert(mssgHandler);
	mWinData.mainWndCaption = appName;
	mWinData.hAppInst = hInstance;
	mWinData.clientWidth = width;
	mWinData.clientHeight = height;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = mssgHandler;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mWinData.hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mWinData.clientWidth, mWinData.clientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int w = R.right - R.left;
	int h = R.bottom - R.top;

	int offsetX = CW_USEDEFAULT, offsetY = CW_USEDEFAULT;
	if (centred) {
		offsetY = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
		offsetX = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
	}
	mWinData.hMainWnd = CreateWindow("D3DWndClassName", mWinData.mainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, offsetX, offsetY, w, h, 0, 0, mWinData.hAppInst, 0);
	if (!mWinData.hMainWnd)
	{
		MessageBox(0, "CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mWinData.hMainWnd, SW_SHOW);
	UpdateWindow(mWinData.hMainWnd);

	return true;
}




bool WinUtil::BeginLoop(bool& canUpdateRender)
{
	MSG msg = { 0 };
	canUpdateRender = false;

	// If there are Window messages then process them.
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return false;
		return true;
	}

	if (!mWinData.appPaused)
	{
		//QueryPerformanceCounter((LARGE_INTEGER*)&sTime1);
		canUpdateRender = true;
	}

	return true;
}


float WinUtil::EndLoop(bool didUpdateRender)
{
	if (!didUpdateRender)
		return 0;
	float deltaTime = 0;
	if (!mWinData.appPaused)
	{
		__int64 countsPerSec;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		double secondsPerCount = 1.0 / (double)countsPerSec;
		static __int64 sTime1 = 0;
		__int64 time2;
		QueryPerformanceCounter((LARGE_INTEGER*)&time2);
		if (sTime1 != 0)
			deltaTime = (float)((time2 - sTime1)*secondsPerCount);
		sTime1 = time2;
		AddSecToClock(deltaTime);

	}
	else
	{
		Sleep(100);
	}
	return deltaTime;
}

int WinUtil::Run(void(*pUpdate)(float), void(*pRender)(float))
{
	MSG msg = { 0 };
	assert(pUpdate && pRender);

	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	double secondsPerCount = 1.0 / (double)countsPerSec;

	float deltaTime = 0;
	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			if (!mWinData.appPaused)
			{

				if (!mWinData.appPaused)
					pUpdate(deltaTime);
				pRender(deltaTime);

				static __int64 sTime1=0;
				__int64 time2;
				QueryPerformanceCounter((LARGE_INTEGER*)&time2);
				if(sTime1!=0)
					deltaTime = (float)((time2 - sTime1)*secondsPerCount);
				sTime1 = time2;
				AddSecToClock(deltaTime);
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return (int)msg.wParam;
}

void WinUtil::ChooseRes(int& w, int& h, int defaults[], int numPairs)
{
	assert(defaults && numPairs > 0);

	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);

	w = 0;
	h = 0;
	for (int i = 0; i < numPairs; ++i)
	{
		int newW = defaults[i * 2], newH = defaults[i*2+1];

		if (w < newW && newW < sw && newH < sh)
		{
			w = newW;
			h = newH;
		}
	}
	assert(w > 0);
}

