#include "stdafx.h"
#include <windows.h>

#define REG_INT				REG_DWORD_LITTLE_ENDIAN
#define REG_STR				REG_SZ

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

int WINAPI RegSetValueExt(LPCTSTR hKeyName, LPCTSTR lpszValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData)
{
	HKEY	hKeyResult = 0;
	DWORD	dwDisposition = 0;
	LONG	lResult;

	if (dwType == REG_INT)
		dwLen = sizeof(int);

	// Open Key 
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);

	if (lResult == ERROR_SUCCESS) 
	{
		// Set Value On Open Key 
		lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
	}
	else 
	{
		// Create New Key 
		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,NULL,
			  			         REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
								 NULL, &hKeyResult, &dwDisposition);

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


int	WINAPI RegSetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int SetValue)
{
	return RegSetValueExt(hKeyName, lpszValueName, REG_INT, sizeof(int), &SetValue);
}

int	WINAPI RegSetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, LPCTSTR SetValue)
{
	return RegSetValueExt(hKeyName, lpszValueName, REG_STR, (_tcslen(SetValue) * sizeof(TCHAR) + 1), (LPVOID)SetValue);
}
