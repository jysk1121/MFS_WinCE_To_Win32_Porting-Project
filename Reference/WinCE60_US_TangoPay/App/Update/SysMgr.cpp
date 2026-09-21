#include "stdafx.h"
#include "SysMgr.h"

#define REG_INT				REG_DWORD_LITTLE_ENDIAN
#define REG_STR				REG_SZ

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CSystemMgr
 FUNCTION NAME: CSystemMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CSystemMgr::CSystemMgr()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSystemMgr
 FUNCTION NAME: ~CSystemMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CSystemMgr::~CSystemMgr()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: RegGetInt()
 RETURN TYPE  :
 PARAMETER    :
 DESCRIPTION  : Registry에서 int값을 조회한다.
-------------------------------------------------------------------*/
int	CSystemMgr::RegGetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int InitValue)
{
	RegQueryValueExt(hKeyName, lpszValueName, REG_INT, sizeof(int), &InitValue);
	return InitValue;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: RegGetStr()
 RETURN TYPE  :
 PARAMETER    :
 DESCRIPTION  : Registry에서 String값을 조회한다.
-------------------------------------------------------------------*/
CString	CSystemMgr::RegGetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, LPCTSTR InitValue)
{
	TCHAR	szData[1024] = {0};
	CString	strRet("");

	memset(szData, 0, sizeof(szData));

	RegQueryValueExt(hKeyName, lpszValueName, REG_STR, sizeof(szData), szData);

	strRet = szData;
	return strRet;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: RegQueryValueExt()
 RETURN TYPE  :
 PARAMETER    :
 DESCRIPTION  : Registry를 조회한다.
-------------------------------------------------------------------*/
int CSystemMgr::RegQueryValueExt(LPCTSTR hKeyName, LPCTSTR lpszValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData)
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
//		memset(lpszData,NULL,dwLen);							// 초기화 무처리 : 2003.05.18
	}

	DWORD	dwTypeSave = dwType;								// Query시 변경됨 : 2003.05.23
	DWORD	dwLenSave = dwLen;

	// Open Key
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,hKeyName,0,KEY_ALL_ACCESS,&hKeyResult);

	if (lResult == ERROR_SUCCESS) 
	{
		dwType = dwTypeSave;
		dwLen = dwLenSave;
		lResult = RegQueryValueEx(hKeyResult,lpszValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);

		if (lResult != ERROR_SUCCESS)
		{
			dwType = dwTypeSave;
			dwLen = dwLenSave;
			if (dwType == REG_STR)
				dwLen = strlen((char*)lpszData) + 1;
			lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);

			if (lResult == ERROR_SUCCESS) 
			{
				dwType = dwTypeSave;
				dwLen = dwLenSave;
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
			dwType = dwTypeSave;
			dwLen = dwLenSave;
			if (dwType == REG_STR)
				dwLen = strlen((char*)lpszData) + 1;
			lResult = RegSetValueEx(hKeyResult,lpszValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
		}

		if (lResult == ERROR_SUCCESS) 
		{
			dwType = dwTypeSave;
			dwLen = dwLenSave;
			lResult = RegQueryValueEx(hKeyResult,lpszValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);
		}
	}

	// Close Key
	RegCloseKey(hKeyResult);

	if (lResult == ERROR_SUCCESS) 
		return TRUE;
    else 
		return FALSE;
}

CString CSystemMgr::GetUpdateFileName(UpdateType eType, CString strPath)
{
	CString strFileName;
	CString strMaster, strPatch;

	switch (m_SystemConfig.GetCEVersion())	// [#GLDV-2853] AU Kook 2021.03.03 Support MX2800SE / WEC7
	{
	case WINCE_7:
		strMaster = MASTER_70;
		strPatch = PATCH_70;
		break;

	case WINCE_6:
	default:
		strMaster = MASTER_60;
		strPatch = PATCH_60;
		break;
	}

	if (eType == UT_MASTER_ZIP)					strFileName.Format(L"%s\\%s.ZIP", strPath, strMaster);
	else if (eType == UT_MASTER_FILE)			strFileName.Format(L"%s\\%s", strPath, strMaster);
	else if (eType == UT_PATCH_ZIP)				strFileName.Format(L"%s\\%s.ZIP", strPath, strPatch);
	else if (eType == UT_PATCH_FILE)			strFileName.Format(L"%s\\%s", strPath, strPatch);

	return strFileName;
}