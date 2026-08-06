#pragma once

#ifndef _WIN32_WCE

//
// Modeless card-insert panel for the Win32 simulator.
//
// On a real terminal CDEV_CDR::InsertCard() polls the SANKYO status command
// (SANKYO_CDR_PM_STATUS_INTAKE) until the reader reports a card sitting in
// the throat. There is no reader on the desktop, so this panel stands in for
// it: the tester presses OK, which is what InsertCard() is polling for.
//
// This panel only answers "is a card in the reader" - it carries no Track2
// data. Track2 is a separate concern, prompted for (via CDevSimTrack2Dlg)
// exactly when the ATM actually asks for it: on a successful EMV read
// (Track 2 Equivalent Data, tag 57) or on the plain/fallback magstripe read
// (CDEV_CDR::ReadCardData_ISO2()).
//
// The window is modeless and lives on its own UI thread, so waiting for the
// tester never blocks the ATM, and the ATM's polling never blocks the panel.
//

class CDevSimCDRDlg
{
public:
	// Shows the panel (first call creates it). Safe to call on every
	// InsertCard() poll - it only raises the window, it never re-arms it.
	static void	Show();

	// Same panel, relabelled for the "Take your card" wait: OK now means the
	// customer pulled the card back out, which clears CARD_REAR_DETECT from
	// GetMediaStatus() and lets that wait loop finish. Pass FALSE to take it
	// down if the wait ends some other way.
	static void	ShowForRemoval(BOOL bShow);

	// True once OK has been pressed for the card currently "in" the reader.
	// This is what InsertCard() polls, and what CDEV_CDR::GetMediaStatus()
	// ORs CARD_REAR_DETECT on, so the rest of the ATM sees a card present.
	static BOOL	IsCardDetected();

	// Re-arms the panel for the next card and hides it; called from
	// CDEV_CDR::RemoveCard() and CDEV_CDR::CancelInsertCard().
	static void	Reset();

	// Tears the panel down; called from CDEV_CDR::Close().
	static void	Destroy();

private:
	static UINT __cdecl	PanelThread(LPVOID pParam);
	static LRESULT CALLBACK	PanelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif	// !_WIN32_WCE
