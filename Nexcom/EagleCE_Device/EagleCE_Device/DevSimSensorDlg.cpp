#include "stdafx.h"
#include "DevSimSensorDlg.h"

#ifndef _WIN32_WCE

#include "DeviceSimUtil.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

//
// The panel is built with raw Win32 calls rather than a dialog resource: the
// simulator lives entirely in files that are excluded from the WinCE build, and
// adding entries to EagleCE_Device.rc would touch a file the WinCE build shares.
//

namespace
{
	const TCHAR	SENSOR_WND_CLASS[]	= _T("EagleDevSimSensorPanel");

	const int	ID_FIRST_CHECK		= 2000;

	const UINT	TIMER_KEEP_ON_TOP	= 1;

	struct SENSOR_ITEM
	{
		int		nBit;
		LPCTSTR	pszLabel;
	};

	// Bit meanings taken from CSVC_Manager::OnDioNotify().
	//
	// The upper frame bit is carried faithfully but CSVC_Manager::OnDioNotify()
	// never looks at it (m_DIO_Upper_Frame_Open is only ever initialised), so
	// the label says so rather than leaving the box looking broken.
	const SENSOR_ITEM SENSORS[] =
	{
		{ 0x01, _T("Front door open") },
		{ 0x02, _T("Safe door open") },
		{ 0x04, _T("Upper frame open (ATM ignores)") },
		{ 0x10, _T("Ear jack inserted") },
	};

	const int SENSOR_COUNT = sizeof(SENSORS) / sizeof(SENSORS[0]);

	// Sensors whose hardware bit is active low: the bit is SET while the sensor
	// is idle and CLEARED once it trips. OnDioNotify() reads the front door that
	// way under FRONT_DOOR_REVERSE, so an all-zero byte means "front door open"
	// to the ATM and the terminal goes out of service with Z20001.
	//
	// Everything above the driver - the check boxes, ApplyToggle(), the ini
	// value - works in plain terms ("bit on = the door is open"); this mask is
	// applied on the way out so the ATM still sees real hardware polarity.
#if (FRONT_DOOR_REVERSE)
	const BYTE	ACTIVE_LOW_MASK	= 0x01;
#else
	const BYTE	ACTIVE_LOW_MASK	= 0x00;
#endif

	// Written by the panel thread, read by the DIO watch thread and by
	// GetSensor(). A critical section keeps the byte consistent; it is only ever
	// held for a couple of instructions, so neither side can stall the other.
	CRITICAL_SECTION	g_csSensor;
	BOOL				g_bSensorInit	= FALSE;
	BYTE				g_bySensor		= 0;

	HWND				g_hPanel		= NULL;
	CWinThread*			g_pPanelThread	= NULL;

	void EnsureInit()
	{
		if (g_bSensorInit)
			return;

		::InitializeCriticalSection(&g_csSensor);

		g_bySensor    = (BYTE)CDevSim::ReadInt(_T("DIO"), _T("SensorStatus"), 0);
		g_bSensorInit = TRUE;
	}

	// What the check boxes show: bit on = that sensor has tripped.
	BYTE ReadLogical()
	{
		EnsureInit();

		::EnterCriticalSection(&g_csSensor);
		BYTE bySensor = g_bySensor;
		::LeaveCriticalSection(&g_csSensor);

		return bySensor;
	}
}


BYTE CDevSimSensorDlg::GetSensorByte()
{
	// What the SIU would report on the wire.
	return (BYTE)(ReadLogical() ^ ACTIVE_LOW_MASK);
}


void CDevSimSensorDlg::ApplyToggle(int nBit, BOOL bOn)
{
	EnsureInit();

	::EnterCriticalSection(&g_csSensor);

	if (bOn)
		g_bySensor |= (BYTE)nBit;
	else
		g_bySensor &= (BYTE)~nBit;

	BYTE bySensor = g_bySensor;

	::LeaveCriticalSection(&g_csSensor);

	// CDEV_DIO::WatchSensor() polls once a second and posts WM_DIO_NOTIFY when
	// the byte changes, so nothing needs to be signalled from here.
	LOG(Info, _T("[Simulator] Sensor changed - panel 0x%02X, wire 0x%02X"),
		bySensor, (BYTE)(bySensor ^ ACTIVE_LOW_MASK));
}


LRESULT CALLBACK CDevSimSensorDlg::PanelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		{
			int nId = LOWORD(wParam);
			int nIndex = nId - ID_FIRST_CHECK;

			if (nIndex >= 0 && nIndex < SENSOR_COUNT)
			{
				HWND hCheck = ::GetDlgItem(hWnd, nId);
				BOOL bOn = (::SendMessage(hCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);

				ApplyToggle(SENSORS[nIndex].nBit, bOn);
				return 0;
			}
		}
		break;

	case WM_TIMER:
		if (wParam == TIMER_KEEP_ON_TOP)
		{
			// Cheap and idempotent: if the panel is already at the top of the
			// topmost band this is a no-op, and SWP_NOACTIVATE means it never
			// takes the focus away from the ATM.
			if (!::IsIconic(hWnd))
			{
				::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}

			return 0;
		}
		break;

	case WM_CLOSE:
		// Keep the panel alive for the life of the session: it is the only way
		// to drive the sensors, and the ATM keeps polling them.
		::ShowWindow(hWnd, SW_MINIMIZE);
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


UINT __cdecl CDevSimSensorDlg::PanelThread(LPVOID /*pParam*/)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc   = (WNDPROC)PanelWndProc;
	wc.hInstance     = AfxGetInstanceHandle();
	wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = SENSOR_WND_CLASS;

	::RegisterClass(&wc);

	const int nRowHeight = 24;
	const int nWidth     = 240;
	const int nHeight    = (SENSOR_COUNT * nRowHeight) + 60;

	// Park the panel beside the simulated ATM screen so it does not cover it.
	// If the desktop is too narrow for that, put it in the top-left corner and
	// let it overlap - being reachable matters more than being out of the way.
	int nLeft = EAGLE_SCREEN_CX + 10;

	if ((nLeft + nWidth) > ::GetSystemMetrics(SM_CXSCREEN))
		nLeft = 20;

	// WS_EX_TOPMOST is required: the ATM display window is a full-screen popup,
	// so anything below it in the z-order is simply invisible. This is safe here
	// (unlike the activation deadlock seen in ScreenDisplay.cpp) because this
	// window belongs to a thread that runs its own message pump, and it is shown
	// with SW_SHOWNOACTIVATE so it never steals activation.
	g_hPanel = ::CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		SENSOR_WND_CLASS,
		_T("Device Simulator - Sensors"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		nLeft, 20, nWidth, nHeight,
		NULL, NULL, AfxGetInstanceHandle(), NULL);

	if (g_hPanel == NULL)
	{
		LOG(Error, _T("[Simulator] Failed to create the sensor panel"));
		return 0;
	}

	BYTE bySensor = ReadLogical();

	for (int i = 0; i < SENSOR_COUNT; i++)
	{
		HWND hCheck = ::CreateWindow(
			_T("BUTTON"), SENSORS[i].pszLabel,
			WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			16, 14 + (i * nRowHeight), nWidth - 45, 20,
			g_hPanel, (HMENU)(INT_PTR)(ID_FIRST_CHECK + i),
			AfxGetInstanceHandle(), NULL);

		if (hCheck == NULL)
			continue;

		::SendMessage(hCheck, WM_SETFONT,
			(WPARAM)::GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

		if (bySensor & SENSORS[i].nBit)
			::SendMessage(hCheck, BM_SETCHECK, BST_CHECKED, 0);
	}

	::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
	::UpdateWindow(g_hPanel);

	// The ATM display window is created later and will claim the foreground.
	// Re-assert the topmost band here, and again on a timer, so the panel stays
	// reachable no matter what order the windows come up in.
	::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	::SetTimer(g_hPanel, TIMER_KEEP_ON_TOP, 2000, NULL);

	LOG(Info, _T("[Simulator] Sensor panel ready - panel 0x%02X, wire 0x%02X"),
		bySensor, (BYTE)(bySensor ^ ACTIVE_LOW_MASK));

	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return 0;
}


void CDevSimSensorDlg::Show()
{
	EnsureInit();

	if (g_pPanelThread != NULL)
	{
		if (g_hPanel != NULL)
			::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);

		return;
	}

	// Its own UI thread, so the panel keeps pumping messages no matter what the
	// ATM's threads are doing.
	g_pPanelThread = AfxBeginThread(PanelThread, NULL);
}


void CDevSimSensorDlg::Destroy()
{
	if (g_hPanel != NULL)
		::PostMessage(g_hPanel, WM_DESTROY, 0, 0);

	g_pPanelThread = NULL;
}

#endif	// !_WIN32_WCE
