#include "stdafx.h"
#include "DeviceSimUtil.h"

#ifndef _WIN32_WCE

#include "../../EagleCE_Framework/EagleCE_Framework/IniFile.h"

CString CDevSim::GetIniPath()
{
	TCHAR szModulePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	CString strPath = szModulePath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos >= 0)
		strPath = strPath.Left(nPos + 1);

	strPath += _T("DeviceSim.ini");
	return strPath;
}

CString CDevSim::ReadString(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault)
{
	CIniFile ini(GetIniPath());
	return ini.ReadString(pszSection, pszKey, pszDefault);
}

int CDevSim::ReadInt(LPCTSTR pszSection, LPCTSTR pszKey, int nDefault)
{
	TCHAR szDefault[16];
	_stprintf_s(szDefault, _countof(szDefault), _T("%d"), nDefault);

	CString strValue = ReadString(pszSection, pszKey, szDefault);
	return _ttoi(strValue);
}

BYTE CDevSim::ReadByte(LPCTSTR pszSection, LPCTSTR pszKey, BYTE byDefault)
{
	return (BYTE)ReadInt(pszSection, pszKey, (int)byDefault);
}

// Returns 0-15, or -1 when the character is not a hex digit.
static int HexDigit(TCHAR ch)
{
	if (ch >= _T('0') && ch <= _T('9'))
		return ch - _T('0');

	if (ch >= _T('a') && ch <= _T('f'))
		return ch - _T('a') + 10;

	if (ch >= _T('A') && ch <= _T('F'))
		return ch - _T('A') + 10;

	return -1;
}

int CDevSim::ReadHexBytes(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, BYTE* pBuffer, int nBufferSize)
{
	if (pBuffer == NULL || nBufferSize <= 0)
		return 0;

	memset(pBuffer, 0, nBufferSize);

	CString strHex = ReadString(pszSection, pszKey, pszDefault);
	strHex.Remove(_T(' '));

	int nWritten = 0;

	for (int i = 0; (i + 1) < strHex.GetLength() && nWritten < nBufferSize; i += 2)
	{
		int nHi = HexDigit(strHex[i]);
		int nLo = HexDigit(strHex[i + 1]);

		if (nHi < 0 || nLo < 0)
			break;

		pBuffer[nWritten++] = (BYTE)((nHi << 4) | nLo);
	}

	return nWritten;
}

BOOL CDevSim::Confirm(LPCTSTR pszDeviceName, LPCTSTR pszCommand, LPCTSTR pszDetail)
{
	CString strMsg;
	strMsg.Format(
		_T("[Device Simulator]\n\nDevice  : %s\nCommand : %s\n%s\n\nOK = success (positive ack)\nCancel = NG (negative ack / error)"),
		pszDeviceName, pszCommand, pszDetail);

	int nRet = ::MessageBox(NULL, strMsg, _T("Device Simulator - Confirm Command"),
		MB_OKCANCEL | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST);

	return (nRet == IDOK);
}

#endif // !_WIN32_WCE
