#include "stdafx.h"
#include "DevSimPinPadDlg.h"

#ifndef _WIN32_WCE

#include "DeviceSimUtil.h"
#include "CrypteraComm.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

//
// Built with raw Win32 calls rather than a dialog resource: the simulator lives
// entirely in files excluded from the WinCE build, and adding entries to
// EagleCE_Device.rc would touch a file the WinCE build shares.
//

namespace
{
	const TCHAR	PINPAD_WND_CLASS[]	= _T("EagleDevSimPinPad");

	const int	ID_FIRST_KEY		= 3000;

	const UINT	TIMER_KEEP_ON_TOP	= 1;

	// CrypteraComm.h declares EventCallBackFunction with three parameters, but
	// the EPP DLL and CSVC_Manager::EventControlFunc() both use four - the ATM
	// casts the mismatch away when registering. Calling through the stale
	// three parameter form would push the wrong arguments, so the real shape is
	// restated here.
	typedef int (*EPP_EVENT_CALLBACK)(unsigned short usEventID, unsigned short usParam,
									  byte* lpByte, unsigned short usDataLength);

	struct PINPAD_KEY
	{
		unsigned long	ulXFSCode;	// what the EPP reports on the wire
		LPCTSTR			pszLabel;
		BOOL			bDigit;		// masked during secure PIN entry
	};

	// Four columns per row, matching the physical key pad.
	const PINPAD_KEY KEYS[] =
	{
		{ WFS_PIN_FK_1,			_T("1"),		TRUE  },
		{ WFS_PIN_FK_2,			_T("2"),		TRUE  },
		{ WFS_PIN_FK_3,			_T("3"),		TRUE  },
		{ WFS_PIN_FK_ENTER,		_T("ENTER"),	FALSE },

		{ WFS_PIN_FK_4,			_T("4"),		TRUE  },
		{ WFS_PIN_FK_5,			_T("5"),		TRUE  },
		{ WFS_PIN_FK_6,			_T("6"),		TRUE  },
		{ WFS_PIN_FK_CLEAR,		_T("CLEAR"),	FALSE },

		{ WFS_PIN_FK_7,			_T("7"),		TRUE  },
		{ WFS_PIN_FK_8,			_T("8"),		TRUE  },
		{ WFS_PIN_FK_9,			_T("9"),		TRUE  },
		{ WFS_PIN_FK_CANCEL,	_T("CANCEL"),	FALSE },

		{ WFS_PIN_FK_RES1,		_T("-"),		FALSE },
		{ WFS_PIN_FK_0,			_T("0"),		TRUE  },
		{ WFS_PIN_FK_RES2,		_T("+"),		FALSE },
		{ WFS_PIN_FK_RES3,		_T(" "),		FALSE },
	};

	const int KEY_COUNT		= sizeof(KEYS) / sizeof(KEYS[0]);
	const int KEY_COLUMNS	= 4;
	const int KEY_ROWS		= KEY_COUNT / KEY_COLUMNS;

	// Reason code bits read by CSVC_Manager::PressKeyHandler().
	const unsigned short REASON_NONE		= 0x0000;
	const unsigned short REASON_AUTO_END	= 0x0008;

	HWND				g_hPanel		= NULL;
	CWinThread*			g_pPanelThread	= NULL;
	EPP_EVENT_CALLBACK	g_pCallback		= NULL;
	BOOL				g_bPinMode		= FALSE;
	unsigned short		g_usKeyCount	= 0;


	void PutBE16(BYTE* pBuffer, unsigned short usValue)
	{
		pBuffer[0] = (BYTE)((usValue >> 8) & 0xFF);
		pBuffer[1] = (BYTE)(usValue & 0xFF);
	}


	// Reproduces the EVENT_ENTRY payload the EPP DLL sends: four length
	// prefixed fields - key code, XFS key code, key count, reason code.
	void SendKeyEvent(unsigned long ulXFSCode, unsigned short usReasonCode)
	{
		if (g_pCallback == NULL)
		{
			LOG(Error, _T("[Simulator] PIN pad key ignored - no EPP callback registered"));
			return;
		}

		BYTE byEvent[17];

		memset(byEvent, 0, sizeof(byEvent));

		PutBE16(&byEvent[0], sizeof(unsigned char));
		byEvent[2] = 0x00;								// key code - unused by the ATM

		PutBE16(&byEvent[3], sizeof(unsigned long));
		byEvent[5] = (BYTE)((ulXFSCode >> 24) & 0xFF);
		byEvent[6] = (BYTE)((ulXFSCode >> 16) & 0xFF);
		byEvent[7] = (BYTE)((ulXFSCode >> 8) & 0xFF);
		byEvent[8] = (BYTE)(ulXFSCode & 0xFF);

		PutBE16(&byEvent[9], sizeof(unsigned short));
		PutBE16(&byEvent[11], g_usKeyCount);

		PutBE16(&byEvent[13], sizeof(unsigned short));
		PutBE16(&byEvent[15], usReasonCode);

		g_pCallback(EVENT_ENTRY, 0, byEvent, sizeof(byEvent));
	}


	void PressKey(int nIndex)
	{
		const PINPAD_KEY& key = KEYS[nIndex];

		// Secure PIN entry: the EPP never reveals which digit was pressed, it
		// only reports that a key arrived, and the ATM masks it on screen.
		// Reproducing that keeps PIN screens behaving as they do on hardware.
		if ((g_bPinMode == TRUE) && (key.bDigit == TRUE))
		{
			g_usKeyCount++;
			SendKeyEvent(0, REASON_NONE);
		}
		else if ((g_bPinMode == TRUE) && (key.ulXFSCode == WFS_PIN_FK_ENTER))
		{
			// ENTER terminates a PIN entry, which the EPP signals as auto end.
			SendKeyEvent(0, REASON_AUTO_END);
			g_usKeyCount = 0;
		}
		else
		{
			SendKeyEvent(key.ulXFSCode, REASON_NONE);
		}

		LOG(Info, _T("[Simulator] PIN pad key pressed (%s)%s"),
			key.pszLabel, (g_bPinMode == TRUE) ? _T(" - PIN mode") : _T(""));
	}
}


void CDevSimPinPadDlg::SetEventCallback(void* pCallback)
{
	g_pCallback = (EPP_EVENT_CALLBACK)pCallback;
}


static LRESULT CALLBACK PinPadWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		{
			int nIndex = LOWORD(wParam) - ID_FIRST_KEY;

			if ((nIndex >= 0) && (nIndex < KEY_COUNT))
			{
				PressKey(nIndex);
				return 0;
			}
		}
		break;

	case WM_TIMER:
		if (wParam == TIMER_KEEP_ON_TOP)
		{
			// The ATM display is a full screen popup, so the pad has to
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
		// The ATM decides when the pad is up; closing it by hand would leave no
		// way to type, so the close box only hides it.
		::ShowWindow(hWnd, SW_HIDE);
		return 0;

	case WM_DESTROY:
		::KillTimer(hWnd, TIMER_KEEP_ON_TOP);
		g_hPanel = NULL;
		::PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static UINT __cdecl PinPadThread(LPVOID /*pParam*/)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc   = (WNDPROC)PinPadWndProc;
	wc.hInstance     = AfxGetInstanceHandle();
	wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = PINPAD_WND_CLASS;

	::RegisterClass(&wc);

	const int nKeyWidth  = 62;
	const int nKeyHeight = 40;
	const int nGap       = 6;
	const int nMargin    = 12;

	const int nWidth  = (nMargin * 2) + (KEY_COLUMNS * nKeyWidth) + ((KEY_COLUMNS - 1) * nGap) + 16;
	const int nHeight = (nMargin * 2) + (KEY_ROWS * nKeyHeight) + ((KEY_ROWS - 1) * nGap) + 40;

	// Park the pad beside the simulated ATM screen, below the sensor panel. If
	// the desktop is too narrow, fall back to the left edge and let it overlap -
	// being reachable matters more than being out of the way.
	int nLeft = EAGLE_SCREEN_CX + 10;

	if ((nLeft + nWidth) > ::GetSystemMetrics(SM_CXSCREEN))
		nLeft = 20;

	g_hPanel = ::CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		PINPAD_WND_CLASS,
		_T("Device Simulator - PIN Pad"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		nLeft, 210, nWidth, nHeight,
		NULL, NULL, AfxGetInstanceHandle(), NULL);

	if (g_hPanel == NULL)
	{
		LOG(Error, _T("[Simulator] Failed to create the PIN pad"));
		return 0;
	}

	for (int i = 0; i < KEY_COUNT; i++)
	{
		int nColumn = i % KEY_COLUMNS;
		int nRow    = i / KEY_COLUMNS;

		HWND hKey = ::CreateWindow(
			_T("BUTTON"), KEYS[i].pszLabel,
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			nMargin + (nColumn * (nKeyWidth + nGap)),
			nMargin + (nRow * (nKeyHeight + nGap)),
			nKeyWidth, nKeyHeight,
			g_hPanel, (HMENU)(INT_PTR)(ID_FIRST_KEY + i),
			AfxGetInstanceHandle(), NULL);

		if (hKey == NULL)
			continue;

		::SendMessage(hKey, WM_SETFONT,
			(WPARAM)::GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}

	::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
	::UpdateWindow(g_hPanel);

	::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	::SetTimer(g_hPanel, TIMER_KEEP_ON_TOP, 2000, NULL);

	LOG(Info, _T("[Simulator] PIN pad ready"));

	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return 0;
}


void CDevSimPinPadDlg::Show(BOOL bPinMode)
{
	g_bPinMode   = bPinMode;
	g_usKeyCount = 0;

	if (g_pPanelThread == NULL)
	{
		// Its own UI thread, so the pad keeps pumping messages no matter what
		// the ATM's threads are doing.
		g_pPanelThread = AfxBeginThread(PinPadThread, NULL);
		return;
	}

	if (g_hPanel != NULL)
	{
		::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
		::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}


void CDevSimPinPadDlg::Hide()
{
	if (g_hPanel != NULL)
		::ShowWindow(g_hPanel, SW_HIDE);
}


void CDevSimPinPadDlg::Destroy()
{
	if (g_hPanel != NULL)
		::PostMessage(g_hPanel, WM_DESTROY, 0, 0);

	g_pPanelThread = NULL;
}

#endif	// !_WIN32_WCE
