#pragma once

#ifndef _WIN32_WCE

//
// Blocking Track2-entry panel for the Win32 simulator.
//
// Unlike the card-present panel (CDevSimCDRDlg), which is polled in a loop
// and so must never block, this one backs functions the ATM calls once and
// expects an immediate answer from: CDEV_CDR::ReadCardData_ISO2() (plain
// magstripe read, or the EMV chip-failure fallback reading the same way) and
// the EMV kernel simulator's Tag 57 (Track 2 Equivalent Data) lookup, which a
// successful EMV read also depends on.
//
// PromptForTrack2() shows the panel and blocks the calling (ATM) thread
// until ACCEPT is pressed, then returns what was typed. The panel still runs
// on its own UI thread underneath, so the wait does not freeze the panel
// itself - only the caller, which is exactly what a synchronous device read
// is supposed to do.
//

class CDevSimTrack2Dlg
{
public:
	// Shows the panel (first call creates it) and blocks until ACCEPT is
	// pressed. Returns the Track2 text at that point.
	static CString	PromptForTrack2();

private:
	static UINT __cdecl	PanelThread(LPVOID pParam);
	static LRESULT CALLBACK	PanelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif	// !_WIN32_WCE
