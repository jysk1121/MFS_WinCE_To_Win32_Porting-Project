#include "stdafx.h"
#include "DevSimTrack2Dlg.h"

#ifndef _WIN32_WCE

#include "DeviceSimUtil.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

//
// Built with raw Win32 calls rather than a dialog resource: the simulator lives
// entirely in files excluded from the WinCE build, and adding entries to
// EagleCE_Device.rc would touch a file the WinCE build shares.
//

namespace
{
	const TCHAR	TRACK2_WND_CLASS[]	= _T("EagleDevSimTrack2Panel");

	const int	ID_BTN_ACCEPT		= 5001;

	const UINT	TIMER_KEEP_ON_TOP	= 1;

	HWND				g_hPanel		= NULL;
	HWND				g_hEditTrack2	= NULL;
	CWinThread*			g_pPanelThread	= NULL;

	// Signalled by ACCEPT; PromptForTrack2() blocks on this from the calling
	// (ATM) thread. Auto-reset, so a stray signal cannot satisfy a later,
	// unrelated wait.
	HANDLE				g_hEventAccepted = NULL;


	CString DefaultTrack2()
	{
		return CDevSim::ReadString(_T("CDR"), _T("TrackISO2"), _T("111111111111111111=222222222222"));
	}


	void EnsureInit()
	{
		if (g_hEventAccepted == NULL)
			g_hEventAccepted = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}


	// Capture the typed text, hide the panel, and release whichever ATM
	// thread is blocked in PromptForTrack2().
	void AcceptTrack2()
	{
		::ShowWindow(g_hPanel, SW_HIDE);

		LOG(Info, _T("[Simulator] Track2 accepted"));

		::SetEvent(g_hEventAccepted);
	}
}


LRESULT CALLBACK CDevSimTrack2Dlg::PanelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_BTN_ACCEPT)
		{
			AcceptTrack2();
			return 0;
		}
		break;

	case WM_TIMER:
		if (wParam == TIMER_KEEP_ON_TOP)
		{
			// The ATM display is a full screen popup, so the panel has to
			// re-assert the topmost band to stay reachable. SWP_NOACTIVATE
			// keeps it from stealing focus while doing so.
			if (!::IsIconic(hWnd) && ::IsWindowVisible(hWnd))
			{
				::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}

			return 0;
		}
		break;

	case WM_CLOSE:
		// ACCEPT is the only way to hand Track2 back to the caller, which is
		// blocked waiting for it; closing the panel by hand would hang that
		// thread forever, so the close box only hides it.
		::ShowWindow(hWnd, SW_HIDE);
		return 0;

	case WM_DESTROY:
		::KillTimer(hWnd, TIMER_KEEP_ON_TOP);
		g_hPanel = NULL;
		g_hEditTrack2 = NULL;
		::PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


UINT __cdecl CDevSimTrack2Dlg::PanelThread(LPVOID /*pParam*/)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc   = (WNDPROC)PanelWndProc;
	wc.hInstance     = AfxGetInstanceHandle();
	wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = TRACK2_WND_CLASS;

	::RegisterClass(&wc);

	const int nWidth  = 320;
	const int nHeight = 130;
	const int nMargin = 12;

	// Park the panel below the card-present panel, beside the simulated ATM
	// screen. If the desktop is too narrow, fall back to the left edge and
	// let it overlap - being reachable matters more than being out of the way.
	int nLeft = EAGLE_SCREEN_CX + 10;

	if ((nLeft + nWidth) > ::GetSystemMetrics(SM_CXSCREEN))
		nLeft = 20;

	g_hPanel = ::CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		TRACK2_WND_CLASS,
		_T("Device Simulator - Track2"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		nLeft, 600, nWidth, nHeight,
		NULL, NULL, AfxGetInstanceHandle(), NULL);

	if (g_hPanel == NULL)
	{
		LOG(Error, _T("[Simulator] Failed to create the Track2 panel"));
		return 0;
	}

	HWND hLabel = ::CreateWindow(
		_T("STATIC"), _T("Track2:"),
		WS_CHILD | WS_VISIBLE,
		nMargin, nMargin + 3, 50, 20,
		g_hPanel, NULL, AfxGetInstanceHandle(), NULL);

	g_hEditTrack2 = ::CreateWindow(
		_T("EDIT"), DefaultTrack2(),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		nMargin + 55, nMargin, nWidth - (nMargin * 2) - 55, 24,
		g_hPanel, NULL, AfxGetInstanceHandle(), NULL);

	HWND hAccept = ::CreateWindow(
		_T("BUTTON"), _T("ACCEPT"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		nMargin, nMargin + 40, nWidth - (nMargin * 2), 32,
		g_hPanel, (HMENU)(INT_PTR)ID_BTN_ACCEPT, AfxGetInstanceHandle(), NULL);

	if (hLabel != NULL)
		::SendMessage(hLabel, WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	if (g_hEditTrack2 != NULL)
		::SendMessage(g_hEditTrack2, WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	if (hAccept != NULL)
		::SendMessage(hAccept, WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
	::UpdateWindow(g_hPanel);

	::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	::SetTimer(g_hPanel, TIMER_KEEP_ON_TOP, 2000, NULL);

	LOG(Info, _T("[Simulator] Track2 panel ready"));

	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return 0;
}


CString CDevSimTrack2Dlg::PromptForTrack2()
{
	EnsureInit();

	if (g_pPanelThread == NULL)
	{
		// Its own UI thread, so the panel keeps pumping messages while this
		// call blocks the ATM thread below.
		g_pPanelThread = AfxBeginThread(PanelThread, NULL);

		// Wait for PanelThread() to create the window before touching it.
		for (int i = 0; (i < 100) && (g_hPanel == NULL); i++)
			::Sleep(10);
	}
	else if (g_hPanel != NULL)
	{
		::SetWindowText(g_hEditTrack2, DefaultTrack2());
		::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
		::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	::WaitForSingleObject(g_hEventAccepted, INFINITE);

	TCHAR szText[256] = { 0 };

	if (g_hEditTrack2 != NULL)
		::GetWindowText(g_hEditTrack2, szText, _countof(szText));

	return CString(szText);
}

#endif	// !_WIN32_WCE
