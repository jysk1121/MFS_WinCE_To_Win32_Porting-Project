#pragma once

// Win32 desktop device simulator.
//
// On WinCE, these device classes talk to real hardware through vendor DLLs
// (MFS_CDM_WEC7.dll etc.) loaded via LoadLibrary/GetProcAddress. Those DLLs are
// ARM binaries and there is no physical device attached on a desktop PC, so on
// Win32 every device class simulates its hardware responses instead:
//   - Inquiry/status commands are answered automatically from DeviceSim.ini
//     (section = device name, key = command name).
//   - Control/action commands are confirmed interactively through a modal
//     OK/NG dialog, so a developer can drive success/failure paths by hand.
#ifndef _WIN32_WCE

class CDevSim
{
public:
	static CString ReadString(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault);
	static int ReadInt(LPCTSTR pszSection, LPCTSTR pszKey, int nDefault);
	static BYTE ReadByte(LPCTSTR pszSection, LPCTSTR pszKey, BYTE byDefault);

	// Reads a hex string (e.g. "3B6800000073C84013009000") into pBuffer.
	// Returns the number of bytes written, never more than nBufferSize.
	static int ReadHexBytes(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, BYTE* pBuffer, int nBufferSize);

	// Control/action command confirmation. Returns TRUE for OK (simulate success),
	// FALSE for NG (simulate failure/negative-ack).
	static BOOL Confirm(LPCTSTR pszDeviceName, LPCTSTR pszCommand, LPCTSTR pszDetail = _T(""));

private:
	static CString GetIniPath();
};

#endif // !_WIN32_WCE
