#pragma once

#ifndef _WIN32_WCE

//
// Modeless PIN pad for the Win32 simulator.
//
// On a real terminal the Cryptera EPP raises EVENT_ENTRY through the callback
// registered with CCrypteraComm::RegistEventCallBackFunction(), and
// CSVC_Manager::PressKeyHandler() turns the reported XFS key code into a
// KEYMAP_x value. There is no key pad on the desktop, so this panel stands in
// for it: every button raises the same event, with the same 17 byte payload,
// through the same callback.
//
// Layout follows the physical EPP:
//
//     1  2  3  ENTER        - / + / blank are the three unlabelled keys,
//     4  5  6  CLEAR          reported by the EPP as RES1 / RES2 / RES3 and
//     7  8  9  CANCEL         mapped to KEYMAP_LEFT / RIGHT / BLANK.
//     -  0  +  (blank)
//
// The window is modeless and lives on its own UI thread, so pressing a key
// never blocks the ATM, and the ATM never blocks the pad.
//

class CDevSimPinPadDlg
{
public:
	// Shows the pad (first call creates it). bPinMode selects secure PIN entry,
	// where digits are reported without a key code - see the .cpp for why.
	static void	Show(BOOL bPinMode);

	// Hides the pad; the ATM calls this whenever key entry is disabled.
	static void	Hide();

	// Tears the pad down; called from CCrypteraComm::CloseDevice().
	static void	Destroy();

	// Stores the callback the ATM registered, so key presses can be delivered
	// exactly the way the EPP DLL would deliver them.
	static void	SetEventCallback(void* pCallback);
};

#endif	// !_WIN32_WCE
