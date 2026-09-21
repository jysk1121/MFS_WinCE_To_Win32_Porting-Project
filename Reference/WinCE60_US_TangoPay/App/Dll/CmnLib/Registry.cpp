#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include <winreg.h>

static CNHConfig	g_Config;

int WINAPI RegQueryValueExt(LPCTSTR hKeyName, LPCTSTR lpszValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData)
{

	LONG	lResult;
	HKEY	hKeyResult = 0;
	DWORD	dwDisposition = 0;

	if (dwType == REG_INT)
		dwLen = sizeof(int);
	else
	if (dwType == REG_STR)
	{
		if (dwLen<1) return FALSE;
	}
	
	// Open Key
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);

	if (lResult == ERROR_SUCCESS) 
	{
		lResult = RegQueryValueEx(hKeyResult,lpszValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);

		if (lResult != ERROR_SUCCESS)
		{
			if (dwType == REG_STR)
				dwLen = strlen((char*)lpszData) + 1;
			lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);

			if (lResult == ERROR_SUCCESS) 
			{
				lResult = RegQueryValueEx(hKeyResult,lpszValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);
			}
		}
	}
	else 
	{
		// Create New Key 
		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, hKeyName, 0, NULL,
			  			         REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
								 NULL, &hKeyResult, &dwDisposition);

		if (lResult == ERROR_SUCCESS)
		{
			if (dwType == REG_STR)
				dwLen = strlen((char*)lpszData) + 1;
			lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
		}

		if (lResult == ERROR_SUCCESS) 
		{
			lResult = RegQueryValueEx(hKeyResult,lpszValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);
		}
	}

	// Close Key
	RegCloseKey(hKeyResult);

	if (lResult == ERROR_SUCCESS) 
		return TRUE;

	return FALSE;
}

int WINAPI RegSetValueExt(LPCTSTR hKeyName, LPCTSTR lpszValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData, int nHKeyPos)
{
	HKEY	hKeyResult = 0;
	DWORD	dwDisposition = 0;
	LONG	lResult;

	if (dwType == REG_INT)
		dwLen = sizeof(int);

	// Open Key 
	// [#2325] NH KSK 2015.01.28 1: HKEY_USER
//	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);
	switch(nHKeyPos)
	{
		case 2:
			lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);
			break;

		case 1:
			lResult = RegOpenKeyEx(HKEY_USERS,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);
			break;

		case 0:
		default:
			lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);
			break;
	}
	// end of [#2325]

	if (lResult == ERROR_SUCCESS) 
	{
		// Set Value On Open Key 
		lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
	}
	else 
	{
		// [#2325] NH KSK 2015.01.28
		// Create New Key 
//		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,NULL,
//			REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
//			NULL, &hKeyResult, &dwDisposition);
		switch(nHKeyPos)
		{
			case 2:
				lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT,hKeyName,0,NULL,
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
					NULL, &hKeyResult, &dwDisposition);
				break;

			case 1:
				lResult = RegCreateKeyEx(HKEY_USERS,hKeyName,0,NULL,
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
					NULL, &hKeyResult, &dwDisposition);
				break;

			case 0:
			default:
				lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,NULL,
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
					NULL, &hKeyResult, &dwDisposition);
				break;
		}
		// end of [#2325]

		if (lResult == ERROR_SUCCESS)
			lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
	}

	// Close Key 
	RegCloseKey(hKeyResult);

	if (lResult == ERROR_SUCCESS) 
		return TRUE;
    else 
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
int	WINAPI RegGetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int InitValue)
{
	RegQueryValueExt(hKeyName, lpszValueName, REG_INT, sizeof(int), &InitValue);
	return InitValue;
}

CString	WINAPI RegGetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, LPCTSTR InitValue)
{
	TCHAR	szData[1024] = {0};
	CString	strRet("");
	CString strKeyName(hKeyName);
	CString strValueName(lpszValueName);
	CString strInitValue(InitValue);

	RegQueryValueExt(hKeyName, lpszValueName, REG_STR, sizeof(szData), szData);

	strRet = szData;
	return strRet;
}

int	WINAPI RegSetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int SetValue, int nHKeyPos)
{
//	return RegSetValueExt(hKeyName, lpszValueName, REG_INT, sizeof(int), &SetValue);
	return RegSetValueExt(hKeyName, lpszValueName, REG_INT, sizeof(int), &SetValue, nHKeyPos);	// [#2325] NH KSK 2015.02.06
}

int	WINAPI RegSetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, CString SetValue, int nHKeyPos)
{
//	return RegSetValueExt(hKeyName, lpszValueName, REG_STR, SetValue.GetLength()*2+1, SetValue.GetBuffer(0));
	return RegSetValueExt(hKeyName, lpszValueName, REG_STR, SetValue.GetLength()*2+1, SetValue.GetBuffer(0), nHKeyPos);	// [#2325] NH KSK 2015.02.06
}

///////////////////////////////////////////////////////////////////////////////
// 5050 CE DEVICE CS8900A1에만 적용됨
///////////////////////////////////////////////////////////////////////////////
#include <ntddndis.h>
#include <winioctl.h>

#define OPLTOUCH_FUNCTION			3000
#define VIBRATOR_POWER_ON	OPLTOUCH_FUNCTION
#define VIBRATOR_POWER_OFF	VIBRATOR_POWER_ON+1
#define IOCTL_VIBRATOR_POWER_ON  CTL_CODE( FILE_DEVICE_HAL, VIBRATOR_POWER_ON, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIBRATOR_POWER_OFF CTL_CODE( FILE_DEVICE_HAL, VIBRATOR_POWER_OFF, METHOD_BUFFERED, FILE_ANY_ACCESS)

int	WINAPI	SetTouchVibration(int bTouchControl)
{
#ifdef UNDER_CE
	HANDLE	hVirPwr = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22

	hVirPwr = CreateFile(_T("TCH1:"),	GENERIC_READ | GENERIC_WRITE, 
										FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0 );

	if (hVirPwr == INVALID_HANDLE_VALUE)
		return 0;

	if (bTouchControl == ENABLE)
		DeviceIoControl(hVirPwr, IOCTL_VIBRATOR_POWER_ON, NULL, 0, NULL, 0, NULL, NULL);
	else
		DeviceIoControl(hVirPwr, IOCTL_VIBRATOR_POWER_OFF, NULL, 0, NULL, 0, NULL, NULL);

	CloseHandle(hVirPwr);
#endif
	return 1;
}


#define IPCONFIGPOSI				_T("Comm\\CS8900A1\\Parms\\Tcpip")
HKEY	m_hKey;

int WINAPI RebindNic(CString nicName)
{
#ifdef UNDER_CE
	// Open the NDIS driver.
	// ip address apply
	HANDLE hNdis = CreateFile(_T("NDS0:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL ); 
	if (hNdis == INVALID_HANDLE_VALUE)	// [#2022] NH KSK 2011.02.22
	{
		return FALSE;
	}

	TCHAR	szAdpaterNames[512] = {};
	DWORD	dwWrite = 0;

	// Set Adpater Name
	_tcscpy(szAdpaterNames, nicName);

	// ip address apply
	DeviceIoControl(hNdis, IOCTL_NDIS_REBIND_ADAPTER, (LPVOID)szAdpaterNames, ((::_tcslen(szAdpaterNames)+2)*sizeof(TCHAR)),
		NULL, 0, &dwWrite, NULL );
	
	CloseHandle(hNdis);
#endif
	return TRUE;
}

int WINAPI GetNicLogs(CString nicName)
{
	BOOL result = TRUE;
#ifdef UNDER_CE
	HANDLE hNdis = CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL ); 
	if (hNdis == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// Get driver name
	TCHAR lnbuff[512] = {};
	_tcscpy(lnbuff, nicName);

	// Output buffer
	DWORD bufferSize = 2048;
	void *logBuffer = malloc(bufferSize);

	DWORD returnedLen = 0;
	result = DeviceIoControl(hNdis, IOCTL_NDIS_GET_LOG_DATA, lnbuff, ((::_tcslen(lnbuff)+2)*sizeof(TCHAR)), logBuffer, bufferSize, &returnedLen, NULL);

	if (!result)
	{
		DWORD code = GetLastError();
		RETAILMSG(1, (L"[ERR] GetLogData failed with error: %02x\r\n", GetLastError()));
	}

	CloseHandle(hNdis);
	free(logBuffer);

#endif
	// Ignore the output
	return result;
}

int	WINAPI ReloadLanCard()
{
#ifndef _DEBUG
	const CString nicName = L"CS8900A1";

	// Rebind NIC
	if (!RebindNic(nicName))
	{
		return FALSE;
	}
#endif

	// It all worked!
	return TRUE;
}

int WINAPI AtmGetIpAddress(CString& szIpAddress, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200] = {};

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	szIpAddress.Empty();
	RegQueryValueEx(m_hKey, _T("IPAddress"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	szIpAddress.Format(_T("%s"), (char *)bData);
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (szIpAddress.GetLength() <= 0)
		szIpAddress.Format(_T("0.0.0.0"));

	return TRUE;
}

// [#404] [NH] KSK 2008.8.21
int WINAPI AtmGetDhcpIpAddress(CString& szIpAddress, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200] = {};

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	szIpAddress.Empty();
	RegQueryValueEx(m_hKey, _T("DhcpIPAddress"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	szIpAddress.Format(_T("%s"), (char *)bData);
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (szIpAddress.GetLength() <= 0)
		szIpAddress.Format(_T("0.0.0.0"));

	return TRUE;
}
// end of [#404]

int WINAPI AtmGetGateWay(CString& szGateWay, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200] = {};

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	szGateWay.Empty();
	RegQueryValueEx(m_hKey, _T("DefaultGateway"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	szGateWay.Format(_T("%s"), (char *)bData);
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (szGateWay.GetLength() <= 0)
		szGateWay.Format(_T("0.0.0.0"));

	return TRUE;
}

// [#404] [NH] KSK 2008.8.21
int WINAPI AtmGetDhcpGateWay(CString& szGateWay, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200] = {};
	
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	szGateWay.Empty();
	RegQueryValueEx(m_hKey, _T("DhcpDefaultGateway"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	szGateWay.Format(_T("%s"), (char *)bData);
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (szGateWay.GetLength() <= 0)
		szGateWay.Format(_T("0.0.0.0"));

	return TRUE;
}
// end of [#404]

int WINAPI AtmGetSubnetMask(CString& szSubnetMask, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200] = {};

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	szSubnetMask.Empty();
	RegQueryValueEx(m_hKey, _T("Subnetmask"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	szSubnetMask.Format(_T("%s"), (char *)bData);
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (szSubnetMask.GetLength() <= 0)
		szSubnetMask.Format(_T("0.0.0.0"));

	return TRUE;
}

// [#404] [NH] KSK 2008.8.21
int WINAPI AtmGetDhcpSubnetMask(CString& szSubnetMask, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200] = {};

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	szSubnetMask.Empty();
	RegQueryValueEx(m_hKey, _T("DhcpSubnetmask"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	szSubnetMask.Format(_T("%s"), (char *)bData);
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (szSubnetMask.GetLength() <= 0)
		szSubnetMask.Format(_T("0.0.0.0"));

	return TRUE;
}
// end of [#404]

// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
int	WINAPI AtmGetDNS(CString& pDNS, CString& pDNS2, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 64;
	WCHAR	*bData = new WCHAR[64]();

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);
	RegQueryValueEx(m_hKey, _T("DNS"), NULL, &dwType, (LPBYTE)bData, &dwSize);
	
	pDNS = bData;
	pDNS2 = bData + wcslen(bData) + 1;

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (pDNS.GetLength() <= 0)
		pDNS.Format(_T("0.0.0.0"));

	if (pDNS2.GetLength() <= 0)
		pDNS2.Format(_T("0.0.0.0"));

	delete [] bData;

	return TRUE;
}
// end of [#RWC6-10]

// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
// [#404] [NH] KSK 2008.8.21
int	WINAPI AtmGetDhcpDNS(CString& pDNS, CString& pDNS2, int type)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 64;
	WCHAR	*bData = new WCHAR[64]();

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);
	RegQueryValueEx(m_hKey, _T("DhcpDNS"), NULL, &dwType, (LPBYTE)bData, &dwSize);

	pDNS = bData;
	pDNS2 = bData + wcslen(bData) + 1;

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return하도록 수정
	if (pDNS.GetLength() <= 0)
		pDNS.Format(_T("0.0.0.0"));

	if (pDNS2.GetLength() <= 0)
		pDNS2.Format(_T("0.0.0.0"));

	delete [] bData;

	return TRUE;
}
// end of [#404]
// end of [#RWC6-10]

int WINAPI AtmSetIpAddress(LPCTSTR pIpAddr)
{
	DWORD dw = 0;
	WCHAR	szTemp[257];
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	int nSize = wcslen(pIpAddr);

	memcpy(szTemp, pIpAddr, (15 * 2));
	szTemp[15] = 0x00;

	RegSetValueEx(m_hKey, _T("IPAddress"), 0L, REG_SZ, (BYTE *)szTemp, (15*2));
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}	

	return TRUE;
}

int WINAPI AtmSetGateWay(LPCTSTR szGateWay)
{
	DWORD dw = 0;
	WCHAR	szTemp[257];
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);
	int nSize = wcslen(szGateWay);

	memcpy(szTemp, szGateWay, (15 * 2));
	szTemp[15] = 0x00;

	RegSetValueEx(m_hKey, _T("DefaultGateway"), 0L, REG_SZ, (BYTE *)szTemp, (15*2));
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}	
	return TRUE;
}

int WINAPI AtmSetSubnetMask(LPCTSTR szSubnetMask)
{
	DWORD dw = 0;
	WCHAR	szTemp[257];
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);
	int nSize = wcslen(szSubnetMask);

	memcpy(szTemp, szSubnetMask, (15 * 2));
	szTemp[15] = 0x00;	
	
	RegSetValueEx(m_hKey, _T("Subnetmask"), 0L, REG_SZ, (BYTE *)szTemp, (15*2));
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}	
	return TRUE;
}

// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
int	WINAPI AtmSetDNS(LPCTSTR pDNS, LPCTSTR pDNS2)
{
	DWORD dw = 0;
	CString	strTemp;
	WCHAR  szTemp[257] = { 0, };
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);
	
	/*
	//WCHAR	szTemp[257];
	//int nSize = wcslen(pDNS);
	//strTemp.Format(_T("%s,%s"), pDNS, pDNS2);
	//memcpy(szTemp, strTemp, ((15*2)+2)); // (15 ip addr * 2) + (comma + null) 
	//szTemp[31] = 0x00;
	//RegSetValueEx(m_hKey, _T("DNS"), 0L, REG_SZ, (BYTE *)szTemp, ((15*2)+2));
	*/

	int nLenDNS1 = wcslen(pDNS);
    int nLenDNS2 = wcslen(pDNS2);

	// DNS1(null)DNS2(null)(null)
	strTemp.Format((_T("%s"), pDNS));
	_tcscpy(szTemp, strTemp);

	// szTemp is initialized with 0x00, no need to append null manually.
	strTemp.Format((_T("%s"), pDNS2));
	_tcscpy(&szTemp[nLenDNS1+1], strTemp);

	RegSetValueEx(m_hKey, _T("DNS"), 0L, REG_MULTI_SZ, (BYTE *)szTemp, (nLenDNS1 + 1 + nLenDNS2 + 1 + 1) * sizeof(WCHAR));
	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}	

	return TRUE;
}
// end of [#RWC6-10]

int	WINAPI AtmSetDHCP(DWORD dwDHCP, int type)
{
	DWORD dw = 0;

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, IPCONFIGPOSI, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);
	
	RegSetValueEx(m_hKey, _T("EnableDHCP"), 0L, REG_DWORD, (BYTE *)&dwDHCP, sizeof(dwDHCP));

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	if (dwDHCP == 1)
	{
		AtmSetDNS(L"", L"");
	}

	return TRUE;
}

CNHConfig* WINAPI GetConfigFuncPointer()
{
	return &g_Config;
}
