#include "stdafx.h"
#include "Util.h"
#include <memory>
#include <vector>
#include <winioctl.h>

#include "IniFile.h"

// Win32 desktop: GetDHCP_xxx() below reads the live values that
// CSVC_Manager::Initialize() already collected from the real adapter,
// instead of the WinCE-only registry path.
#ifndef _WIN32_WCE
#include "EagleDataManager.h"
#endif	// !_WIN32_WCE

#include "ntddndis.h"

#ifndef _WIN32_WCE
// GetFileVersionInfo/VerQueryValue live in version.dll on the desktop;
// WinCE exposes them through coredll, so no extra import library is needed.
#pragma comment(lib, "version.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/** **************************************************
*	@brief		crc 16 table
*****************************************************/
const unsigned int crc_table[256] = 
{
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440, 
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841, 
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441, 
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840, 
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40, 
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041, 
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441, 
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41, 
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40, 
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440, 
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841, 
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41, 
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};


/** **********************************************************
*	@brief		응용 프로그램 경로 취득
*	@retval		경로 반환
************************************************************/
CString CUtil::GetAppPath(void)
{
	TCHAR path[_MAX_PATH] = { 0, };

	// The 3rd argument is a CHARACTER count, not a byte count: passing
	// sizeof(path) allowed a write of twice the buffer size on UNICODE builds.
	if (0 == GetModuleFileName(NULL, path, _countof(path)))
	{
		return CString();
	}

	path[_countof(path) - 1] = _T('\0');

	CString strPath = path;

	int nPos = strPath.ReverseFind(_T('\\'));

	if (nPos < 0)
	{
		return CString();
	}

	return strPath.Left(nPos);
}


/** **********************************************************
*	@brief		Key Press Wave File 취득
*	@retval		경로 반환
************************************************************/
CString CUtil::GetKeyPressWaveFilePath(void)
{
	CString strPath;

	strPath.Format(_T("%s%s%s"), GetAppPath(), WAVE_FILE_PATH, KEY_PRESSED_WAVE_FILE);

	return strPath;
}


/** **********************************************************
*	@brief		Notice Sound Wave File 취득
*	@retval		경로 반환
************************************************************/
CString CUtil::GetNoticeWaveFilePath(void)
{
	CString strPath;

	strPath.Format(_T("%s%s%s"), GetAppPath(), WAVE_FILE_PATH, NOTICE_WAVE_FILE);

	return strPath;
}


/** **********************************************************
*	@brief		Get Product Name And Product Version
*	@param		CString strResourceLanguageID	: Resource의 Language ID
*	@param		CString& strProductName			: Name
*	@param		CString& strProductVersion		: Version
*	@retval		성공 TRUE, 실패 FALSE
************************************************************/
BOOL CUtil::GetProductAndVersion(CString strResourceLanguageID, CString& strProductName, CString& strProductVersion)
{
    // get the filename of the executable containing the version resource
    TCHAR szFilename[MAX_PATH + 1] = {0};
    if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
    {
        return FALSE;
    }

    // allocate a block of memory for the version info
    DWORD dummy;
    DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
    if (dwSize == 0)
    {
        return FALSE;
    }
    std::vector<BYTE> data(dwSize);

    // load the version info
    if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
    {
        return FALSE;
    }

    // get the name and version strings
    LPVOID pvProductName = NULL;
    unsigned int iProductNameLen = 0;
    LPVOID pvProductVersion = NULL;
    unsigned int iProductVersionLen = 0;

	CString strTempName, strTempVersion;

	strTempName = _T("\\StringFileInfo\\") + strResourceLanguageID + _T("\\ProductName");
	strTempVersion = _T("\\StringFileInfo\\") + strResourceLanguageID + _T("\\ProductVersion");

    if (!VerQueryValue(&data[0], strTempName.GetBuffer(0), &pvProductName, &iProductNameLen) ||
        !VerQueryValue(&data[0], strTempVersion.GetBuffer(0), &pvProductVersion, &iProductVersionLen))
    {
        return FALSE;
    }

    strProductName.SetString((LPCTSTR)pvProductName, iProductNameLen);
    strProductVersion.SetString((LPCTSTR)pvProductVersion, iProductVersionLen);

	// AP Version Change
	if (EAGLE_SCREEN_CX == CS130_WIDTH_RESOLUTION)
		//strProductVersion.Insert(0, _T("0"));		// CS130
		strProductVersion.Insert(0, _T("00"));		// CS130	Version Rule 변경
	else
		//strProductVersion.Insert(0, _T("1"));		// MF200
		strProductVersion.Insert(0, _T("01"));		// MF200	Version Rule 변경

    return TRUE;
}


/** **********************************************************
*	@brief		CString의 값을 CStringArray로 변환하는 함수
*	@param		CString strSource				: CString 값
*	@param		CString strSeparator			: Parsing 기준 값
*	@param		CStringArray &strarrDestinatio	: Parsing후 Array 배열 값
*	@retval		CString Array Count
************************************************************/
int	CUtil::ParsingStringToStringArray(CString strSource, CString strSeparator, CStringArray &strarrDestination)
{
	CString strTemp, strData;
	int		nIndex=0;

	strarrDestination.RemoveAll();
	strData = strSource;

	while (TRUE)
	{
		nIndex = strData.Find(strSeparator);

		if(nIndex == -1)
		{
			if (strData.GetLength() > 0)
				strarrDestination.Add(strData);

			break;
		}
		else
		{
			strTemp = strData.Left(nIndex);
			strData = strData.Right(strData.GetLength() - (nIndex + strSeparator.GetLength()));  
			strarrDestination.Add(strTemp);
		}
	}

	return strarrDestination.GetCount();
}


/** **********************************************************
*	@brief		CString값이 숫자값인지 확인하는 함수
*	@param		CString strValue	: Value
*	@retval		숫자인 경우 TRUE, 아닌 경우 FALSE
************************************************************/
int CUtil::IsNumeric(CString strValue)
{
	if (strValue.GetLength() <= 0)		return FALSE;

	for(int i=0; i<strValue.GetLength(); i++)
	{
		if((strValue.GetAt(i)<'0') || (strValue.GetAt(i)>'9'))
			return FALSE;
	}

	return TRUE;
}


/** **********************************************************
*	@brief		CString값이 HEX값인지 확인하는 함수
*	@param		CString strValue	: Value
*	@retval		HEX값인 경우 TRUE, 아닌 경우 FALSE
************************************************************/
int CUtil::IsHexValue(CString strValue)
{
	CString strTemp;

	if (strValue.GetLength() <= 0)		
		return FALSE;

	strTemp = strValue;
	strTemp.MakeUpper();	// 대문자로 변경

	for(int i=0; i<strTemp.GetLength(); i++)
	{
		if((strTemp.GetAt(i) < '0') || (strTemp.GetAt(i) > 'F'))
			return FALSE;
	}

	return TRUE;
}


/** **********************************************************
*	@brief		String을 Int로 변환하는 함수
*	@param		CString strValue	: Value
*	@retval		변환된 int 값
************************************************************/
int	CUtil::StringToInt(CString strValue)
{
	int		ivalue=0, i=0;

	if (strValue.GetLength() <=0 )
		return 0;

	for(i=0; i<strValue.GetLength(); i++)
	{
		if(strValue.GetAt(i) >= '0' && strValue.GetAt(i) <= '9')
		{
			ivalue = (ivalue*10) + (strValue.GetAt(i) - '0');
			continue;
		}
	}

	return ivalue;
}


/** **********************************************************
*	@brief		IntToString
*	@retval		Integer를 CString값으로 변환
************************************************************/
CString	CUtil::IntToString(int nValue)
{
	CString	strReturnValue;
	char buff[1024] = { 0, };

	_itoa(nValue, buff, 10);

	for (int i=0;i<(int)strlen(buff);i++)
	{
		if((buff[i]>='a') && (buff[i]<='z')) 
			buff[i] &= 0xdf;
	}

	strReturnValue.Format(_T("%S"), buff);

	return strReturnValue;
}


/** **********************************************************
*	@brief		String을 BOOL로 변환하는 함수
*	@param		CString strValue	: Value
*	@retval		변환된 int 값
************************************************************/
BOOL CUtil::StringToBOOL(CString strValue)
{
	int		ivalue=0, i=0;

	// 값이 없는 경우는 FALSE로 return
	if (strValue.GetLength() <=0 )
		return FALSE;

	if (strValue.GetAt(0) == '0')
		return FALSE;

	return TRUE;
}


/** **********************************************************
*	@brief		CString값을 Cent를 포함한 문자열로 변환해주는 함수
*	@param		CString strValue	: Value
*	@retval		Amount(Cent포함)로 변환된 값
************************************************************/
CString	CUtil::ConvertFromValueToAmountwithCent(CString strValue, BOOL bUseCent)
{
	CString strTemp, strTemp2, strRetunValue;
	BOOL bIsMinus = FALSE;

	strTemp = strValue;
	strTemp.Replace(_T(" "), _T(""));	// 중간에 Space 제거

	if ((strTemp.GetLength() <= 0) || (_ttoi(strTemp) <= 0))		// 값이 0인 경우 "0.00"으로 Return
	{
		if (bUseCent == TRUE)
			strRetunValue = _T("0.00");
		else
			strRetunValue = _T("0");

		return strRetunValue;
	}

	if (strTemp.GetAt(0) == '-')		// 마이너스 처리
	{
		strTemp = strTemp.Mid(1);		// 마이너스값 제거
		bIsMinus = TRUE;
	}

	// 앞부분의 "0"값을 제거한다
	while(strTemp.GetAt(0) == '0')
	{
		if (strTemp.GetAt(0) == '0')
		{
			strTemp = strTemp.Mid(1);	// 앞에 '0'을 제거
		}
		else
		{
			break;
		}
	}

	// 3자리 이하일때 '0'을 채워서 3자리로 고정
	if ((bUseCent == TRUE) && (strTemp.GetLength() <= 3))
	{
		strTemp2 = strTemp;
		strTemp.Format(_T("%03s"), strTemp2);
	}

	// 센트 구분자('.')을 삽입.
	if (bUseCent == TRUE)
	{
		strRetunValue = _T(".") + strTemp.Right(2);
		strTemp = strTemp.Left(strTemp.GetLength()-2);
	}

	// 세자리마다 쉼표 삽입
	while(strTemp.GetLength() > 0)
	{
		if(strTemp.GetLength() > 3)
		{
			// 3자리 이상일때 3자리씩 짤라서 붙인다
			strRetunValue = _T(",") + strTemp.Right(3) + strRetunValue;
			strTemp = strTemp.Left(strTemp.GetLength()-3);
		}
		else
		{
			// 3자리 이하가 남았을때 나머지를 붙이고 종료
			strRetunValue = strTemp + strRetunValue;
			break;
		}
	}

	// 마이너스 이면 '-' 기호를 붙인다.
	if(bIsMinus)
		strRetunValue = _T("-") + strRetunValue;

	return strRetunValue;

}


/** **********************************************************
*	@brief		IsExistFile
*	@retval		File 존재 유무
************************************************************/
BOOL CUtil::IsExistFile(CString strFilePath)
{
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;

	hSearch = FindFirstFile(strFilePath, &FileData);

	if (hSearch == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hSearch);

	return TRUE;
}


/** **********************************************************
*	@brief		DeleteFileInDirectory : Folder안의 File을 모두 Delete
*	@param		LPCTSTR lpDstPath : 
*	@param		LPCTSTR lpDstFile : 
*	@param		int &nCurCnt	  : 
*	@param		int nTotalCn	  : Delete할 총 File 개수 
*	@retval		성공 : TRUE, 실패 : FALSE
************************************************************/
BOOL CUtil::DeleteFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile)
{
	CString strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch;
	WCHAR	strSearchDir[255];
	WCHAR	strNewDir[255];
	WCHAR	strSrcFile[255];

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;

	memset(strSearchDir, 0, sizeof(strSearchDir));
	wcscpy(strSearchDir, lpDstPath);
	wcscat(strSearchDir, _T("\\"));
	wcscat(strSearchDir, lpDstFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	while (!bFinished)
	{
		RETAILMSG(0, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpDstPath, FileData.cFileName));

		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// The test below compares the name against the SEARCH PATTERN, which on
			// Win32 never matches the "." and ".." pseudo entries that FindFirstFile()
			// reports there but WinCE does not. Without the extra test the function
			// recurses into <dir>\. until the stack overflows.
			if (wcscmp(lpDstFile, FileData.cFileName) != 0
#ifndef _WIN32_WCE
				&& wcscmp(FileData.cFileName, _T(".")) != 0
				&& wcscmp(FileData.cFileName, _T("..")) != 0
#endif	// _WIN32_WCE
				)
			{
				memset(strNewDir, 0, sizeof(strNewDir));

				wcscpy(strNewDir, lpDstPath);
				wcscat(strNewDir, _T("\\"));
				wcscat(strNewDir, FileData.cFileName);

				DeleteFileInDirectory((LPCTSTR)strNewDir, _T("*.*"));

				// delete directory
				RemoveDirectory((LPCTSTR)strNewDir);

				Sleep(10);
			}
		}
		else
		{
			memset(strSrcFile, 0, sizeof(strSrcFile));

			// make source file full path
			wcscpy(strSrcFile, lpDstPath);
			wcscat(strSrcFile, _T("\\"));
			wcscat(strSrcFile, FileData.cFileName);

			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				SetFileAttributes(strSrcFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFile(strSrcFile);
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			bFinished = TRUE;
			if (GetLastError() != ERROR_NO_MORE_FILES)
			{
				//LOG(Info, _T("Unable to fine next file."));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	FindClose(hSearch);

	Sleep(10);

	if (bError)
		return FALSE;

	return TRUE;
}


/** **********************************************************
*	@brief		CopyFileInDirectory : Directory의 모든 File을 복사
*	@param		LPCTSTR lpDstPath : 
*	@param		LPCTSTR lpDstFile : 
*	@retval		성공 : TRUE, 실패 : FALSE
************************************************************/
BOOL CUtil::CopyFileInDirectory(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath)
{
	CString	strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch;
	WCHAR	strSearchDir[255];
	WCHAR	strNewDir[255];
	WCHAR	strSrcFile[255];
	WCHAR	strDstFile[255];

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;

	memset(strSearchDir, 0, sizeof(strSearchDir));
	wcscpy(strSearchDir, lpSrcPath);
	wcscat(strSearchDir, _T("\\"));
	wcscat(strSearchDir, lpSrcFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	while (!bFinished)
	{
		RETAILMSG(0, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpSrcPath, FileData.cFileName));

		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// The test below compares the name against the SEARCH PATTERN, which on
			// Win32 never matches the "." and ".." pseudo entries that FindFirstFile()
			// reports there but WinCE does not. Without the extra test the function
			// recurses into <dir>\. until the stack overflows.
			if (wcscmp(lpSrcFile, FileData.cFileName) != 0
#ifndef _WIN32_WCE
				&& wcscmp(FileData.cFileName, _T(".")) != 0
				&& wcscmp(FileData.cFileName, _T("..")) != 0
#endif	// _WIN32_WCE
				)
			{
				memset(strNewDir, 0, sizeof(strNewDir));

				wcscpy(strNewDir, lpSrcPath);
				wcscat(strNewDir, _T("\\"));
				wcscat(strNewDir, FileData.cFileName);

				// make destination file full path
				wcscpy(strDstFile, lpDstPath);
				wcscat(strDstFile, _T("\\"));
				wcscat(strDstFile, FileData.cFileName);

				// make new directory
				CreateDirectory(strDstFile, NULL);

				Sleep(10);

				CopyFileInDirectory((LPCTSTR)strNewDir, _T("*.*"), strDstFile);
			}
		}
		else
		{
			memset(strSrcFile, 0, sizeof(strSrcFile));
			memset(strDstFile, 0, sizeof(strDstFile));

			// make source file full path
			wcscpy(strSrcFile, lpSrcPath);
			wcscat(strSrcFile, _T("\\"));
			wcscat(strSrcFile, FileData.cFileName);

			// make destination file full path
			wcscpy(strDstFile, lpDstPath);
			wcscat(strDstFile, _T("\\"));
			wcscat(strDstFile, FileData.cFileName);


			SetFileAttributes(strDstFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFile(strDstFile);

			if (!CopyFile(strSrcFile, strDstFile, FALSE))
			{
				//LOG(Error, _T("Failed to Copy [%s] -> [%s]"), strSrcFile, strDstFile);
				bError = TRUE;
			}
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			bFinished = TRUE;
			if (GetLastError() != ERROR_NO_MORE_FILES)
			{
				//LOG(Info, _T("Unable to fine next file."));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	FindClose(hSearch);

	Sleep(10);

	if (bError)
		return FALSE;

	return TRUE;
}


/** **********************************************************
*	@brief		Sleep_Wait : Message Pumping 함수
*	@param		DWORD dwTime : Wait Time
*	@retval		없음
************************************************************/
void CUtil::Sleep_Wait(DWORD dwTime)
{
	DWORD	dwStart = GetTickCount();
	DWORD	dwElapsed;

	while ((dwElapsed = GetTickCount() - dwStart) < dwTime)
	{
		DWORD dwStatus = MsgWaitForMultipleObjectsEx(0, NULL,
													(dwTime - dwElapsed), QS_ALLINPUT | QS_ALLEVENTS,
											 		 MWMO_INPUTAVAILABLE);

		if (dwStatus == WAIT_OBJECT_0)
		{
			MSG msg;

			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if(!AfxGetApp()->PreTranslateMessage(&msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}
	}
}


/** **********************************************************
*	@brief		SwapEndian : Endian16으로 변환
*	@param		UINT16 val : UINT Type 값
*	@retval		변환된 UINT16 값
************************************************************/
UINT16 CUtil::SwapEndian(UINT16 val)
{
	val = (val<<8) | (val>>8);
	return val;
}


/** **********************************************************
*	@brief		SwapEndian24 : Endian24으로 변환
*	@param		UINT32 val : UINT Type 값
*	@retval		변환된 UINT32 값
************************************************************/
UINT32 CUtilSwapEndian24(UINT32 val)
{
	val = (0x00ffffff) & (((val<<16) & 0x00ff0000) | (val & 0x0000ff00) | ((val>>16) & 0x000000ff));
	return val;
}


/** **********************************************************
*	@brief		SwapEndian : Endian32으로 변환
*	@param		UINT32 val : UINT Type 값
*	@retval		변환된 UINT32 값
************************************************************/
UINT32 CUtil::SwapEndian(UINT32 val)
{
	val = (val<<24) | ((val<<8) & 0x00ff0000) | ((val>>8) & 0x0000ff00) | (val>>24);
	return val;
}


/** **********************************************************
*	@brief		ZippingFromFolder : folder를 압축하는 함수
*	@param		CString ZipFile : zip file full path & name
*	@param		CString SrcFolder : 압축 해야할 folder
*	@retval		성공 TRUE, 실패 FALSE
************************************************************/
BOOL CUtil::ZippingFromFolder(CString SrcFolder, CString ZipFile)
{
	HZIP		hz;
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;
	CString			strSrcFileName, strDestFileName;

	strSrcFileName.Format(_T("%s\\*.*"), SrcFolder);

	hSearch = FindFirstFile(strSrcFileName, &FileData);

	if (hSearch == INVALID_HANDLE_VALUE)
		return FALSE;

	hz = CUtil::Wrapper_CreateZip(ZipFile, 0);

	while(TRUE)
	{
		strDestFileName.Format(_T("%s\\%s"), SrcFolder, FileData.cFileName);
		CUtil::Wrapper_ZipAdd(hz, FileData.cFileName, strDestFileName);

		if (!FindNextFile(hSearch, &FileData))
			break;
	}

	FindClose(hSearch);
	CUtil::Wrapper_CloseZip(hz);

	return TRUE;
}


BOOL CUtil::ZippingFromFile(CString strSrcFolder, CString strSrcFileName, CString ZipFileName)
{
	HZIP		hz;
	CString		strDestFileName;
	WCHAR		wchFileName[256];

	memset(wchFileName, 0, sizeof(wchFileName));
	wcscpy(wchFileName, strSrcFileName);

	strDestFileName.Format(_T("%s\\%s"), strSrcFolder, strSrcFileName);

	hz = CUtil::Wrapper_CreateZip(ZipFileName, 0);

	CUtil::Wrapper_ZipAdd(hz, wchFileName, strDestFileName);

	CUtil::Wrapper_CloseZip(hz);

	return TRUE;
}


/** **********************************************************
*	@brief		UnzippingToFolder : zip file을 folder에 unzip하는 함수
*	@param		CString ZipFile : zip file name
*	@param		CString DestFolder : zip file을 unzip할 folder
*	@retval		성공 TRUE, 실패 FALSE
************************************************************/
BOOL CUtil::UnzippingToFolder(CString ZipFile, CString DestFolder)
{
	HZIP		hz;
	ZRESULT		zr;
	ZIPENTRY	ze;
	int			numItems;
	CString		strFileName;
	CString		strTemp;
	int			i;

	hz = CUtil::Wrapper_OpenZip(ZipFile, 0);

	CUtil::Wrapper_GetZipItem(hz, -1, &ze);
	numItems = ze.index;

	Sleep_Wait(10);

	for (i = 0; i < numItems; i++)
	{
		CUtil::Wrapper_GetZipItem(hz, i, &ze);

		strFileName.Format(_T("%s\\%s"), DestFolder, ze.name);

		Sleep_Wait(10);

		zr = CUtil::Wrapper_UnzipItem(hz, i, (LPCTSTR)strFileName);

		if (ZR_OK != zr)
			break;
	}

	CUtil::Wrapper_CloseZip(hz);

	if (numItems == 0 || i < numItems)
		return FALSE;

	return TRUE;
}


HZIP CUtil::Wrapper_CreateZip(void *buf,unsigned int len, const char *password)
{
	return CreateZip(buf, len, password);
}


HZIP CUtil::Wrapper_CreateZip(const TCHAR *fn, const char *password)
{
	return CreateZip(fn, password);
}


ZRESULT CUtil::Wrapper_ZipAdd(HZIP hz,const TCHAR *dstzn, const TCHAR *fn)
{
	return ZipAdd(hz, dstzn, fn);
}


ZRESULT CUtil::Wrapper_ZipAdd(HZIP hz,const TCHAR *dstzn, void *src,unsigned int len)
{
	return ZipAdd(hz, dstzn, src, len);
}


ZRESULT CUtil::Wrapper_ZipAddFolder(HZIP hz,const TCHAR *dstzn)
{
	return ZipAddFolder(hz, dstzn);
}


ZRESULT CUtil::Wrapper_ZipGetMemory(HZIP hz, void **buf, unsigned long *len)
{
	return ZipGetMemory(hz, buf, len);
}


ZRESULT CUtil::Wrapper_CloseZip(HZIP hz)
{
	return CloseZip(hz);
}


HZIP CUtil::Wrapper_OpenZip(const TCHAR *fn, const char *password)
{
	return OpenZip(fn, password);
}


HZIP CUtil::Wrapper_OpenZip(void *z,unsigned int len, const char *password)
{
	return OpenZip(z, len, password);
}


ZRESULT CUtil::Wrapper_GetZipItem(HZIP hz, int index, ZIPENTRY *ze)
{
	return GetZipItem(hz, index, ze);
}


ZRESULT CUtil::Wrapper_UnzipItem(HZIP hz, int index, const TCHAR *fn)
{
	return UnzipItem(hz, index, fn);
}


ZRESULT CUtil::Wrapper_UnzipItem(HZIP hz, int index, void *z,unsigned int len)
{
	return UnzipItem(hz, index, z, len);
}

UINT CUtil::Wrapper_FormatZipMessage(ZRESULT code, TCHAR *buf,unsigned int len)
{
	return FormatZipMessageU(code, buf, len);
}


HKEY	m_hKey = NULL;

// Set IP Address
BOOL CUtil::SetIPAddress(CString strIPAddress)
{
	String_SetRegistry(ETHERNET_RESIGTRY_PATH, _T("IPAddress"), strIPAddress);

	return TRUE;
}

// Set Gateway
BOOL CUtil::SetGateway(CString strGateway)
{
	String_SetRegistry(ETHERNET_RESIGTRY_PATH, _T("DefaultGateway"), strGateway);

	return TRUE;
}

// Set Subnet Mask
BOOL CUtil::SetSubnetMask(CString strSubnetMask)
{
	String_SetRegistry(ETHERNET_RESIGTRY_PATH, _T("Subnetmask"), strSubnetMask);

	return TRUE;
}

// Set DNS
BOOL CUtil::SetDNS(CString strDNS)
{
	String_SetRegistry(ETHERNET_RESIGTRY_PATH, _T("DNS"), strDNS);

	return TRUE;
}

// Set DHCP
BOOL CUtil::SetDHCP(DWORD dwDHCP)
{
	Int_SetRegistry(ETHERNET_RESIGTRY_PATH, _T("EnableDHCP"), dwDHCP);

	// OS 설치후 Registry가 Clear되었는지 여부를 확인하기 위한 Registry값 Setting
	Int_SetRegistry(CHECK_ETHERNET_INFO_REG_PATH, _T("AP_IPSetting"), 1);

	return TRUE;
}

// Set LCD roation Info
BOOL CUtil::SetLCDLotationInfo(DWORD dwValue)
{
	Int_SetRegistry(LCD_ROTATION_INFO_PATH, _T("rotation"), dwValue);
	return TRUE;
}

// Check IP Information
BOOL CUtil::Check_IP_Information()
{
	if (Int_GetRegistry(CHECK_ETHERNET_INFO_REG_PATH, _T("AP_IPSetting")) == 1)
		return TRUE;

	return FALSE;
}

// Get DHCP
BOOL CUtil::GetDHCP()
{
	if (Int_GetRegistry(ETHERNET_RESIGTRY_PATH, _T("EnableDHCP"), 0) == 1)
		return TRUE;

	return FALSE;
}

// Get LCD rotation Information
int	CUtil::GetLCDLotationInfo()
{
	return Int_GetRegistry(LCD_ROTATION_INFO_PATH, _T("rotation"), 0);
}


// Get Static IP Address
BOOL CUtil::GetStaticIPAddress(CString &strIPAddress)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strIPAddress.Empty();
	RegQueryValueEx(m_hKey, _T("IPAddress"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strIPAddress.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strIPAddress.GetLength() <= 0)
		strIPAddress.Format(_T("0.0.0.0"));

	return TRUE;
}

// Get Static Subnet Mask
BOOL CUtil::GetStaticSubnetMask(CString &strSubnetMask)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strSubnetMask.Empty();
	RegQueryValueEx(m_hKey, _T("Subnetmask"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strSubnetMask.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strSubnetMask.GetLength() <= 0)
		strSubnetMask.Format(_T("0.0.0.0"));

	return TRUE;
}

// Get Static Gateway
BOOL CUtil::GetStaticGateway(CString &strGateWay)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strGateWay.Empty();
	RegQueryValueEx(m_hKey, _T("DefaultGateway"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strGateWay.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strGateWay.GetLength() <= 0)
		strGateWay.Format(_T("0.0.0.0"));

	return TRUE;
}

// Get Static DNS
BOOL CUtil::GetStaticDNS(CString &strDNS)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strDNS.Empty();
	RegQueryValueEx(m_hKey, _T("DNS"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strDNS.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strDNS.GetLength() <= 0)
		strDNS.Format(_T("0.0.0.0"));

	return TRUE;
}



// Get DHCP IP ADDRESS
BOOL CUtil::GetDHCP_IP(CString &strIPAddress)
{
#ifndef _WIN32_WCE
	strIPAddress = CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip;
	return TRUE;
#endif	// !_WIN32_WCE

	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strIPAddress.Empty();
	RegQueryValueEx(m_hKey, _T("DhcpIPAddress"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strIPAddress.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strIPAddress.GetLength() <= 0)
		strIPAddress.Format(_T("0.0.0.0"));

	return TRUE;
}

// Get DHCP Subnet Mask
BOOL CUtil::GetDHCP_SubnetMask(CString &strSubnetMask)
{
#ifndef _WIN32_WCE
	strSubnetMask = CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet;
	return TRUE;
#endif	// !_WIN32_WCE

	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strSubnetMask.Empty();
	RegQueryValueEx(m_hKey, _T("DhcpSubnetmask"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strSubnetMask.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strSubnetMask.GetLength() <= 0)
		strSubnetMask.Format(_T("0.0.0.0"));

	return TRUE;
}

// Get DHCP GATEWAY
BOOL CUtil::GetDHCP_GateWay(CString &strGateWay)
{
#ifndef _WIN32_WCE
	strGateWay = CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway;
	return TRUE;
#endif	// !_WIN32_WCE

	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strGateWay.Empty();
	RegQueryValueEx(m_hKey, _T("DhcpDefaultGateway"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strGateWay.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strGateWay.GetLength() <= 0)
		strGateWay.Format(_T("0.0.0.0"));

	return TRUE;
}

// Get DHCP IP ADDRESS
BOOL CUtil::GetDHCP_DNS(CString &strDNS)
{
#ifndef _WIN32_WCE
	strDNS = CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS;
	return TRUE;
#endif	// !_WIN32_WCE

	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, ETHERNET_RESIGTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strDNS.Empty();
	RegQueryValueEx(m_hKey, _T("DhcpDNS"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strDNS.Format(_T("%s"), (char *)bData);

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	// Address 정보를 Get하지 못할 경우 0.0.0.0으로 return
	if (strDNS.GetLength() <= 0)
		strDNS.Format(_T("0.0.0.0"));

	return TRUE;
}

// Reset Ethernet Device
BOOL CUtil::ResetEthernetDevice()
{

#if (NETWORK_OFFLINE_MODE)
	return TRUE;
#endif

#ifdef _WIN32_WCE
	 // Open the NDIS driver.
	 // ip address apply
	//HANDLE hNdis = CreateFile(_T("NDS0:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
	HANDLE hNdis = CreateFile(_T("NDS0:"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

	if (hNdis == INVALID_HANDLE_VALUE)
		return FALSE;

	TCHAR	szAdapterNames[512];
	DWORD	dwWrite = 0;

	// Set Adapter Name
	memset(szAdapterNames, 0, sizeof(szAdapterNames));
	_tcscpy(szAdapterNames, ETHERNET_ADAPTER_NAME);
	szAdapterNames[::_tcslen(szAdapterNames)+1] = L'\0';

	// ip address apply
	DeviceIoControl(hNdis, IOCTL_NDIS_REBIND_ADAPTER, (LPVOID)szAdapterNames, ((::_tcslen(szAdapterNames)+2)*sizeof(TCHAR)), NULL, 0, &dwWrite, NULL );

	CloseHandle(hNdis);

	return TRUE;
#else
	// Win32 desktop: no NDS0 stream driver, treat as no-op success.
	return TRUE;
#endif // _WIN32_WCE
}


int CUtil::RegQueryValueExt(CString strKeyName, CString strValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData)
{
	LONG	lResult;
	HKEY	hKeyResult = 0;
	DWORD	dwDisposition = 0;

	if (dwType == REG_DWORD_LITTLE_ENDIAN)
	{
		dwLen = sizeof(int);
	}
	else if (dwType == REG_SZ)
	{
		if (dwLen<1) return FALSE;
	}

	// Open Key
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,strKeyName,0,KEY_ALL_ACCESS,&hKeyResult);

	if (lResult == ERROR_SUCCESS) 
	{
		lResult = RegQueryValueEx(hKeyResult,strValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);

		if (lResult != ERROR_SUCCESS)
		{
			if (dwType == REG_SZ)
				dwLen = strlen((char*)lpszData) + 1;

			lResult = RegSetValueEx(hKeyResult,strValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);

			if (lResult == ERROR_SUCCESS) 
				lResult = RegQueryValueEx(hKeyResult,strValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);
		}
	}
	else 
	{
		// Create New Key 
		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, strKeyName, 0, NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS, NULL, &hKeyResult, &dwDisposition);

		if (lResult == ERROR_SUCCESS)
		{
			if (dwType == REG_SZ)
				dwLen = strlen((char*)lpszData) + 1;

			lResult = RegSetValueEx(hKeyResult,strValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
		}

		if (lResult == ERROR_SUCCESS) 
			lResult = RegQueryValueEx(hKeyResult,strValueName,0,(LPDWORD)&dwType,(LPBYTE)lpszData,(LPDWORD)&dwLen);
	}

	// Close Key
	RegCloseKey(hKeyResult);

	if (lResult == ERROR_SUCCESS) 
		return TRUE;

	return FALSE;
}

int CUtil::RegSetValueExt(CString strKeyName, CString strValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData, int nHKeyPos)
{
	HKEY	hKeyResult = 0;
	DWORD	dwDisposition = 0;
	LONG	lResult;

	if (dwType == REG_DWORD_LITTLE_ENDIAN)
		dwLen = sizeof(int);

	// Open Key 
	switch(nHKeyPos)
	{
	case 2:
		lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,strKeyName,0,KEY_ALL_ACCESS,&hKeyResult);
		break;

	case 1:
		lResult = RegOpenKeyEx(HKEY_USERS,strKeyName,0,KEY_ALL_ACCESS,&hKeyResult);
		break;

	case 0:
	default:
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,strKeyName,0,KEY_ALL_ACCESS,&hKeyResult);
		break;
	}

	if (lResult == ERROR_SUCCESS) 
	{
		// Set Value On Open Key 
		lResult = RegSetValueEx(hKeyResult,strValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
	}
	else 
	{
		switch(nHKeyPos)
		{
		case 2:
			lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT,strKeyName,0,NULL,
				REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
				NULL, &hKeyResult, &dwDisposition);
			break;

		case 1:
			lResult = RegCreateKeyEx(HKEY_USERS,strKeyName,0,NULL,
				REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
				NULL, &hKeyResult, &dwDisposition);
			break;

		case 0:
		default:
			lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,strKeyName,0,NULL,
				REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
				NULL, &hKeyResult, &dwDisposition);
			break;
		}

		if (lResult == ERROR_SUCCESS)
		{
			lResult = RegSetValueEx(hKeyResult,strValueName,0,dwType,(CONST BYTE*)lpszData,dwLen);
		}
	}

	// Close Key 
	RegCloseKey(hKeyResult);

	if (lResult == ERROR_SUCCESS) 
		return TRUE;
	else 
		return FALSE;
}


// Set Registry Value
BOOL CUtil::Int_SetRegistry(CString strKeyName, CString strValueName, int nValue)
{
	return RegSetValueExt(strKeyName, strValueName, REG_DWORD_LITTLE_ENDIAN, sizeof(int), &nValue);
}

// Set Registry Value
BOOL CUtil::String_SetRegistry(CString strKeyName, CString strValueName, CString strValue)
{
	return RegSetValueExt(strKeyName, strValueName, REG_SZ, (strValue.GetLength()*2)+1, strValue.GetBuffer(0));
}

// Get Registry Value
int	CUtil::Int_GetRegistry(CString strKeyName, CString strValueName, int nValue)
{
	int nTempValue = 0;

	if (RegQueryValueExt(strKeyName, strValueName, REG_DWORD_LITTLE_ENDIAN, sizeof(int), &nTempValue) == FALSE)
		nTempValue = 0;

	return nTempValue;
}

// Get Registry Value
CString	CUtil::String_GetRegistry(CString strKeyName, CString strValueName)
{
	TCHAR	szData[1024] = { 0, };
	CString strResult;

	if (RegQueryValueExt(strKeyName, strValueName, REG_SZ, sizeof(szData), szData) == TRUE)
		strResult = szData;

	return strResult;
}

	// Convert from Hex value to String
CString	CUtil::ConvertHexToString(BYTE *pByte, int nLength)
{
	CString strTemp, strValue;

	for(int i = 0; i < nLength; i++)
	{
		strTemp.Format(_T("%02X"), (pByte[i] & 0xFF));
		strValue += strTemp;
	}

	return strValue;
}


// Make Pack
BYTE CUtil::MakePack(BYTE bySrc)
{
	BYTE byRet = 0;

	if ((bySrc >= 'a') && (bySrc <= 'f'))
		byRet = ((bySrc - 0x57) & 0x0f);
	else if ((bySrc >= 'A') && (bySrc <= 'F'))
		byRet = ((bySrc - 0x37) & 0x0f);
	else
		byRet = (bySrc & 0x0f);	

	return byRet;
}


void CUtil::ConvertStringToHex(CString strSrc, BYTE *pByDest)
{
	int nSrcLength = strSrc.GetLength() / 2;

	for(int i = 0; i < nSrcLength; i++)
	{
		pByDest[i] = (BYTE)(((MakePack((BYTE)strSrc.GetAt(2*i)) << 4)) | ((MakePack((BYTE)strSrc.GetAt((2*i)+1)) & 0x0F)));
	}
}


/** **********************************************************
*	@brief		변환한 CString 앞에 '0'을 채움.
*	@retval		Integer를 CString값으로 변환
************************************************************/
CString CUtil::IntToStringEx(int nValue, int nStringLen)
{
	CString	strReturnValue;
	char buff[1024] = { 0, };
	int i = 0;

	_itoa(nValue, buff, 10);

	for (i = 0; i<(int)strlen(buff); i++)
	{
		if ((buff[i] >= 'a') && (buff[i] <= 'z'))
			buff[i] &= 0xdf;
	}

	strReturnValue.Format(_T("%S"), buff);

	for (i = 0; i < (nStringLen - (int)strlen(buff)); i++)
		strReturnValue.Insert(0, _T("0"));

	return strReturnValue;
}

BOOL CUtil::SetSWUpdateType(DWORD dwValue)
{
	Int_SetRegistry(SOFTWARE_REGISTRY_PATH, _T("SWUpdateType"), dwValue);

	return TRUE;
}

BOOL CUtil::GetSWUpdateType(DWORD &dwValue)
{
	HKEY	hKeyResult = NULL;
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, SOFTWARE_REGISTRY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyResult, &dw);

	RegQueryValueEx(hKeyResult, _T("SWUpdateType"), NULL, &dwType, (BYTE*)&dwValue, &dwSize);

	if(hKeyResult) {
		RegCloseKey(hKeyResult);
		hKeyResult = NULL;
	}

	return TRUE;
}


BOOL CUtil::SetSWUpdateType(CString strValue)
{
	// INI 파일 열기
	CIniFile iniFile(SOFTWARE_UPDATE_TYPE_NAME);

	iniFile.WriteString(_T("SW_UPDATE_INFO"), _T("SWTYPE"), strValue);

	return TRUE;
}

CString CUtil::GetSWUpdateType()
{
	// INI 파일 열기
	CIniFile iniFile(SOFTWARE_UPDATE_TYPE_NAME);


	return iniFile.ReadString(_T("SW_UPDATE_INFO"), _T("SWTYPE"), _T("0"));
}


BOOL CUtil::GetUniqueKey(CString &strUniqueKey)
{
	DWORD	dw = 0;
	DWORD	dwType = 0;
	DWORD	dwSize = 200;
	BYTE	bData[200];
	CString strTempUnique1, strTempUnique2;

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, UNIQUE_KEY_PATH, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dw);

	memset(bData, 0x00, sizeof(bData));
	strUniqueKey.Empty();

	RegQueryValueEx(m_hKey, _T("UID_Str1"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strTempUnique1.Format(_T("%s"), (char *)bData);

	// 정보를 Get하지 못할 경우 "00000000"으로 return
	if (strTempUnique1.GetLength() <= 0)
		strTempUnique1.Format(_T("00000000"));

	memset(bData, 0x00, sizeof(bData));
	RegQueryValueEx(m_hKey, _T("UID_Str2"), NULL, &dwType, (BYTE*)&bData, &dwSize);
	strTempUnique2.Format(_T("%s"), (char *)bData);

	// 정보를 Get하지 못할 경우 "00000000"으로 return
	if (strTempUnique2.GetLength() <= 0)
		strTempUnique2.Format(_T("00000000"));

	if(m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	strUniqueKey = strTempUnique1 + strTempUnique2;

	return TRUE;
}

void CUtil::EncryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey)
{
	int nRet = 0;
	CDES pDes;
	int nTimes = nInDataSize / 8;
	BYTE byTmp1[8] = {0,}, byTmp2[8] = {0,}, byTmp3[8] = {0,};

	for(int i = 0; i < nTimes; i++)
	{
		memcpy(byTmp1, &pbyInData[i*8], 8);

		pDes.ENCRYPT(byTmp1, byTmp2, pbyKey);
		pDes.DECRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.ENCRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&pbyOutData[i*8], byTmp1, 8);
	}
}


void CUtil::DecryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey)
{
	int nRet = 0;
	CDES pDes;
	int nTimes = nInDataSize / 8;
	BYTE byTmp1[8] = {0,}, byTmp2[8] = {0,}, byTmp3[8] = {0,};

	for(int i = 0; i < nTimes; i++)
	{
		memcpy(byTmp1, &pbyInData[i*8], 8);
		pDes.DECRYPT(byTmp1, byTmp2, pbyKey);
		pDes.ENCRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.DECRYPT(byTmp3, byTmp2, pbyKey);

		memcpy(&pbyOutData[i*8], byTmp2, 8);
	}
}

void CUtil::GetCheckSumForCryptoKey(BYTE *pbyOutData, BYTE *pbyKey)
{
	int i = 0;
	CDES pDes;
	int nScrSize = 0;
	BYTE byTmp1[8] = { 0, }, byTmp2[8] = { 0, }, byTmp3[8] = { 0, };
	BYTE byScrData[8] = { 0, };

	nScrSize = 8;

	for (i = 0; i < (nScrSize / 8); i++)
	{
		memcpy(byTmp1, &byScrData[i*8], 8);

		pDes.ENCRYPT(byTmp1, byTmp2, pbyKey);
		pDes.DECRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.ENCRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&pbyOutData[i*8], byTmp1, 8);
	}
}

/*------ Base64 Encoding Table ------*/
static const char MimeBase64[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

/*------ Base64 Decoding Table ------*/
static int DecodeMimeBase64[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
};


int CUtil::Base64_Encoded(char *text, int numBytes, char *encodedText)
{
	unsigned char input[3]  = {0,0,0};
	unsigned char output[4] = {0,0,0,0};
	int   index, i, j, size;
	char *p, *plen;

	plen           = text + numBytes - 1;
	size           = (4 * (numBytes / 3)) + (numBytes % 3? 4 : 0) + 1;
	j              = 0;

	for  (i = 0, p = text;p <= plen; i++, p++) {
		index = i % 3;
		input[index] = *p;

		if (index == 2 || p == plen) {
			output[0] = ((input[0] & 0xFC) >> 2);
			output[1] = ((input[0] & 0x3) << 4) | ((input[1] & 0xF0) >> 4);
			output[2] = ((input[1] & 0xF) << 2) | ((input[2] & 0xC0) >> 6);
			output[3] = (input[2] & 0x3F);

			encodedText[j++] = MimeBase64[output[0]];
			encodedText[j++] = MimeBase64[output[1]];
			encodedText[j++] = index == 0? '=' : MimeBase64[output[2]];
			encodedText[j++] = index <  2? '=' : MimeBase64[output[3]];

			input[0] = input[1] = input[2] = 0;
		}
	}

	encodedText[j] = '\0';

	return j;
}


int CUtil::Base64_Decoded(char *text, unsigned char *dst, int numBytes)
{
	const char* cp;
	int space_idx = 0, phase;
	int d, prev_d = 0;
	unsigned char c;

	space_idx = 0;
	phase = 0;

	for ( cp = text; *cp != '\0'; ++cp ) {
		d = DecodeMimeBase64[(int) *cp];
		if ( d != -1 ) {
			switch ( phase ) {
				case 0:
					++phase;
					break;
				case 1:
					c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
					if ( space_idx < numBytes )
						dst[space_idx++] = c;
					++phase;
					break;
				case 2:
					c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
					if ( space_idx < numBytes )
						dst[space_idx++] = c;
					++phase;
					break;
				case 3:
					c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
					if ( space_idx < numBytes )
						dst[space_idx++] = c;
					phase = 0;
					break;
			}
			prev_d = d;
		}
	}

	return space_idx;
}


void CUtil::CreateFileFromResource(HINSTANCE hInstance, int nResID, CString strSaveFileName, CString strResourceFormat/*=RESOURCE_FORMAT_TXT*/)
{
	FILE			*Stream;

	DeleteFile(strSaveFileName);

	// 리소스 ID와 타입으로 리소스 정보를 읽어 온다.
	HRSRC	resInfo = FindResource(hInstance, MAKEINTRESOURCE(nResID), strResourceFormat);

	HGLOBAL	hRes = LoadResource(hInstance, resInfo);

	LPSTR	lpRes = (LPSTR)LockResource(hRes);

	int nSize = SizeofResource(hInstance, resInfo);

	if ( (Stream = _wfopen(strSaveFileName, _T("w+b"))) != NULL)	// 바이너리 타입으로 저장하지 않으면, 유니코드로 저장되기 때문에 파일read시 키 정보를 찾지 못함.
	{
		fwrite ((char*)lpRes, sizeof(char), nSize, Stream);

		fflush(Stream);
		fclose(Stream);
	}
}


BOOL CUtil::Check_Valid_IP_Format(CString strValue)
{
	CStringArray strArrData;
	CString strData;
	int i = 0;

	strArrData.RemoveAll();
	strData = strValue;
	strData.TrimLeft();
	strData.TrimRight();

	ParsingStringToStringArray(strValue, _T("."), strArrData);

	if(strArrData.GetCount() == 4)
	{
		for(i=0; i<strArrData.GetCount(); i++)
		{
			if (FALSE == IsNumeric(strArrData.GetAt(i)))	// 숫자만 가능
			{
				return FALSE;
			}
		}

		for(i=0; i<strArrData.GetCount(); i++)
		{
			int nTemp = StringToInt(strArrData.GetAt(i));

			if ( nTemp < 0 || nTemp > 255)
			{
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}


/** *********************************************************
*	@brief		crc 16 generate code
************************************************************/
unsigned int CUtil::crc16 (unsigned char *string, int length ) 
{ 
	unsigned int crc; 
	crc = 0x0000;	  /* initial CRC value 0x0000 */ 

	while (length--) 
	{ 
		crc = ( crc >> 8 ) ^ crc_table[(crc ^ (int) *string++) & 0xff];   
		/* This performs the modulo-2 division by using the look up table. */ 
	} 

	return(crc); /* return the calculated CRC */ 
}