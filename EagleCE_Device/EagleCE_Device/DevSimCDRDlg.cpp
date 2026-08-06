#include "stdafx.h"
#include "DevSimCDRDlg.h"

#ifndef _WIN32_WCE

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

//
// Built with raw Win32 calls rather than a dialog resource: the simulator lives
// entirely in files excluded from the WinCE build, and adding entries to
// EagleCE_Device.rc would touch a file the WinCE build shares.
//

namespace
{
	const TCHAR	CDR_WND_CLASS[]		= _T("EagleDevSimCDRPanel");

	const int	ID_BTN_ACCEPT		= 4001;

	const UINT	TIMER_KEEP_ON_TOP	= 1;

	// Written by the panel thread when OK is pressed, read by InsertCard()
	// and GetMediaStatus() from the ATM thread. A critical section keeps it
	// consistent; it is only ever held for a couple of instructions, so
	// neither side can stall the other.
	CRITICAL_SECTION	g_csCard;
	BOOL				g_bCardInit		= FALSE;
	BOOL				g_bCardDetected	= FALSE;

	// TRUE while the ATM is waiting for the customer to take the card back,
	// which flips what the OK button means.
	BOOL				g_bRemovalMode	= FALSE;

	HWND				g_hPanel		= NULL;
	HWND				g_hLabel		= NULL;
	HWND				g_hButton		= NULL;
	CWinThread*			g_pPanelThread	= NULL;

	void EnsureInit()
	{
		if (g_bCardInit)
			return;

		::InitializeCriticalSection(&g_csCard);
		g_bCardInit = TRUE;
	}


	// Relabels the panel for whichever half of the card cycle is running.
	void ApplyMode()
	{
		if ((g_hLabel == NULL) || (g_hButton == NULL))
			return;

		if (g_bRemovalMode == TRUE)
		{
			::SetWindowText(g_hLabel, _T("Card is in the reader..."));
			::SetWindowText(g_hButton, _T("OK (Card Removed)"));
		}
		else
		{
			::SetWindowText(g_hLabel, _T("Waiting for a card..."));
			::SetWindowText(g_hButton, _T("OK (Card Inserted)"));
		}
	}


	// Card in the throat: take the panel down, the way a real reader would
	// pull the card in past the point of no return.
	void AcceptCard()
	{
		EnsureInit();

		::EnterCriticalSection(&g_csCard);
		g_bCardDetected = TRUE;
		::LeaveCriticalSection(&g_csCard);

		::ShowWindow(g_hPanel, SW_HIDE);

		LOG(Info, _T("[Simulator] CDR card inserted"));
	}


	// Customer pulled the card back out: clearing the flag is what makes
	// GetMediaStatus() stop reporting CARD_REAR_DETECT, which is exactly what
	// the "Take your card" wait loop is polling for.
	void RemoveCard()
	{
		EnsureInit();

		::EnterCriticalSection(&g_csCard);
		g_bCardDetected = FALSE;
		::LeaveCriticalSection(&g_csCard);

		::ShowWindow(g_hPanel, SW_HIDE);

		LOG(Info, _T("[Simulator] CDR card removed"));
	}
}


BOOL CDevSimCDRDlg::IsCardDetected()
{
	EnsureInit();

	::EnterCriticalSection(&g_csCard);
	BOOL bDetected = g_bCardDetected;
	::LeaveCriticalSection(&g_csCard);

	return bDetected;
}


LRESULT CALLBACK CDevSimCDRDlg::PanelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_BTN_ACCEPT)
		{
			if (g_bRemovalMode == TRUE)
				RemoveCard();
			else
				AcceptCard();

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
		// InsertCard() is the only way to accept a card; closing the panel by
		// hand would leave no way to proceed, so the close box only hides it.
		::ShowWindow(hWnd, SW_HIDE);
		return 0;

	case WM_DESTROY:
		::KillTimer(hWnd, TIMER_KEEP_ON_TOP);
		g_hPanel = NULL;
		g_hLabel = NULL;
		g_hButton = NULL;
		::PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


UINT __cdecl CDevSimCDRDlg::PanelThread(LPVOID /*pParam*/)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc   = (WNDPROC)PanelWndProc;
	wc.hInstance     = AfxGetInstanceHandle();
	wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = CDR_WND_CLASS;

	::RegisterClass(&wc);

	const int nWidth  = 260;
	const int nHeight = 100;
	const int nMargin = 12;

	// Park the panel below the PIN pad, beside the simulated ATM screen. If
	// the desktop is too narrow, fall back to the left edge and let it
	// overlap - being reachable matters more than being out of the way.
	int nLeft = EAGLE_SCREEN_CX + 10;

	if ((nLeft + nWidth) > ::GetSystemMetrics(SM_CXSCREEN))
		nLeft = 20;

	g_hPanel = ::CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		CDR_WND_CLASS,
		_T("Device Simulator - Card Reader"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		nLeft, 460, nWidth, nHeight,
		NULL, NULL, AfxGetInstanceHandle(), NULL);

	if (g_hPanel == NULL)
	{
		LOG(Error, _T("[Simulator] Failed to create the CDR panel"));
		return 0;
	}

	g_hLabel = ::CreateWindow(
		_T("STATIC"), _T("Waiting for a card..."),
		WS_CHILD | WS_VISIBLE,
		nMargin, nMargin + 3, nWidth - (nMargin * 2), 20,
		g_hPanel, NULL, AfxGetInstanceHandle(), NULL);

	g_hButton = ::CreateWindow(
		_T("BUTTON"), _T("OK (Card Inserted)"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		nMargin, nMargin + 32, nWidth - (nMargin * 2), 32,
		g_hPanel, (HMENU)(INT_PTR)ID_BTN_ACCEPT, AfxGetInstanceHandle(), NULL);

	if (g_hLabel != NULL)
		::SendMessage(g_hLabel, WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	if (g_hButton != NULL)
		::SendMessage(g_hButton, WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	ApplyMode();

	::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
	::UpdateWindow(g_hPanel);

	::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	::SetTimer(g_hPanel, TIMER_KEEP_ON_TOP, 2000, NULL);

	LOG(Info, _T("[Simulator] CDR panel ready"));

	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return 0;
}


void CDevSimCDRDlg::ShowForRemoval(BOOL bShow)
{
	EnsureInit();

	if (bShow == FALSE)
	{
		// Wait ended some other way (timeout, user input) - drop back to
		// insert mode so the next transaction starts clean.
		g_bRemovalMode = FALSE;

		if (g_hPanel != NULL)
		{
			ApplyMode();
			::ShowWindow(g_hPanel, SW_HIDE);
		}

		return;
	}

	// Nothing to take back out.
	if (IsCardDetected() == FALSE)
		return;

	g_bRemovalMode = TRUE;

	if (g_pPanelThread == NULL)
	{
		g_pPanelThread = AfxBeginThread(PanelThread, NULL);
		return;
	}

	if (g_hPanel != NULL)
	{
		ApplyMode();
		::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
		::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}


void CDevSimCDRDlg::Show()
{
	EnsureInit();

	// Insert half of the cycle.
	if (g_bRemovalMode == TRUE)
	{
		g_bRemovalMode = FALSE;
		ApplyMode();
	}

	// InsertCard() calls this on every poll, including the one that finally
	// observes the card. Without this guard that poll would re-show the
	// panel AcceptCard() has just hidden, leaving it stuck on screen for the
	// rest of the transaction.
	if (IsCardDetected())
		return;

	if (g_pPanelThread == NULL)
	{
		// Its own UI thread, so the panel keeps pumping messages no matter
		// what the ATM's threads (InsertCard() polls this repeatedly) are
		// doing.
		g_pPanelThread = AfxBeginThread(PanelThread, NULL);
		return;
	}

	if (g_hPanel != NULL)
	{
		::ShowWindow(g_hPanel, SW_SHOWNOACTIVATE);
		::SetWindowPos(g_hPanel, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}


void CDevSimCDRDlg::Reset()
{
	EnsureInit();

	::EnterCriticalSection(&g_csCard);
	g_bCardDetected = FALSE;
	::LeaveCriticalSection(&g_csCard);

	g_bRemovalMode = FALSE;

	if (g_hPanel != NULL)
	{
		ApplyMode();
		::ShowWindow(g_hPanel, SW_HIDE);
	}
}


void CDevSimCDRDlg::Destroy()
{
	if (g_hPanel != NULL)
		::PostMessage(g_hPanel, WM_DESTROY, 0, 0);

	g_pPanelThread = NULL;
}

#endif	// !_WIN32_WCE
