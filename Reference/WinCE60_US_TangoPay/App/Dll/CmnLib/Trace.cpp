#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include ".\Common\CmnLib.h"

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"
// [#2032] NH KJW 2011.03.15 Common과 Dll 폴더에 동일한 파일이 존재하여, Dll파일로 단일화함
//#include ".\Common\CeFileFind.h"											// FileFind Class Add for WinCE
#include ".\Dll\CeFileFind.h"											// FileFind Class Add for WinCE
// end of [#2032]
//#include ".\Common\MB2500DLL.h"						// [#573] NH AIREAT 2009.10.26 SW NVRAM

#define DBG_CALL		1
#define DBG_INFO		1

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: NHDebugTimeStampPrint()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 시간은 Serial로 Print 한다.
				NHDbgApi.h에서 사용 한다. 사용시 참고요망.
 HISTORY LIST : 2007.10.03, AIREAT : 작성.
-------------------------------------------------------------------*/
void WINAPI NHDebugTimeStampPrint(void)
{
	// Print Time
#if 1
	DWORD dwMs = GetTickCount() % 1000;		// use tickcount as milliseconds. It's not accurate, of course.
	SYSTEMTIME	ti;
 	GetLocalTime(&ti);

	NHDebugPrintf(_T("[%02d/%02d %02d:%02d:%02d.%03d]"), ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond, dwMs);
#else
	NHDebugPrintf(_T("[%08d] "), GetTickCount());
#endif
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: NHDebugPrintf()
 RETURN TYPE  : 
 PARAMETER    : 스트링 리스트. ex(_T("%d"),10)
 DESCRIPTION  : 파라미터의 내용을 Serial로 Print 한다.
				NHDbgApi.h에서 사용 한다. 사용시 참고요망.
 HISTORY LIST : 2007.10.03, AIREAT : 작성.
-------------------------------------------------------------------*/
void WINAPI NHDebugPrintf(LPCWSTR lpszFmt, ...)
{
// 	WCHAR tszInfo[2048];
	WCHAR tszInfo[STRBUFFSIZE] = L"";

	va_list args;
	va_start(args, lpszFmt);
// 	vswprintf (tszInfo, lpszFmt, args);
	vswprintf_s(tszInfo, STRBUFFSIZE, lpszFmt, args);
	va_end(args);
	
	OutputDebugString(tszInfo);
}

void WINAPI NVDumpF(char cStatus, char szSubUnit, char* szFunction, LPCTSTR szErr, LPCWSTR lpszFmt, ...)
{
	WCHAR tszInfo[STRBUFFSIZE] = L"";

	va_list args;
	va_start(args, lpszFmt);
	vswprintf_s(tszInfo, STRBUFFSIZE, lpszFmt, args);
	va_end(args);

	NVDump(cStatus, szSubUnit, szFunction, szErr, tszInfo, 0);
}

void WINAPI NVDump(char cStatus, char szSubUnit, char* szFunction, LPCTSTR szErr, LPCTSTR szDummy, int nTraceLevel)
{
#ifdef UNDER_CE
	char szTemp[64] = { 0, };		// [#2022] NH KSK 2011.02.22
	BYTE LogData[64] = { 0, };		// [#2022] NH KSK 2011.02.22
	
	//1. 상태 구분(1)
	LogData[0] = cStatus;
	
	//2. 시(1)분(1)초(1)
    SYSTEMTIME  lTime;
    GetLocalTime(&lTime);
	LogData[1] = (BYTE)lTime.wHour;
	LogData[2] = (BYTE)lTime.wMinute;
	LogData[3] = (BYTE)lTime.wSecond;

	//3. Unit 구분
	LogData[4] = NVP_UNIT1_AP;	// 1 = AP log
	LogData[5] = szSubUnit;

	//4. 함수 구분
	LogData[6] = szFunction[0];
	LogData[7] = szFunction[1];

	//5. 장애 코드
	if(wcslen(szErr) == 0)
		memcpy(&LogData[8], "0000000", 7);
	else
	{
		WideToMulti(szTemp, szErr, sizeof(szTemp));
		memcpy(&LogData[8], szTemp, 7);
	}

	//6. Dummy & Field 
	if(wcslen(szDummy) != 0)
	{
		//Memory 사용량 logging
		if(wcscmp(szDummy, _T("MEM_STATUS")) == 0 )
		{
///////////////////////// Memory Information /////////////////////////////
			MEMORYSTATUS memStatus;
			memStatus.dwLength = sizeof(MEMORYSTATUS);
			GlobalMemoryStatus(&memStatus);
			DWORD uvm;
			uvm = memStatus.dwTotalVirtual - memStatus.dwAvailVirtual;
//////////////////////////////////////////////////////////////////////////
			CString strTemp;
			strTemp.Format(L"VM=%d,%d%%", (uvm / 1024), memStatus.dwMemoryLoad);
			NHDEBUG(1, (_T("MEMORY LOAD[%d%%]\n"), memStatus.dwMemoryLoad));
			NHDEBUG(1, (_T("VIRTUAL MEMORY USAGE[%dKB]\n"), (uvm / 1024)));

			memset(szTemp, NULL, sizeof(szTemp));
			WideToMulti(szTemp, strTemp, sizeof(szTemp));

		}
		else
		{
			memset(szTemp, NULL, sizeof(szTemp));
			WideToMulti(szTemp, szDummy, sizeof(szTemp));
		}
		
		memset(&LogData[8+7], 0x20, 16);

		if (strlen(szTemp) > 31)		// [#2022] NH KSK 2011.02.22
			return;

		memcpy(&LogData[8+7], szTemp, strlen(szTemp));
		WriteLogNVRAM(LogData, 31, nTraceLevel);
	}
	else
	{
		WriteLogNVRAM(LogData, 15, nTraceLevel);
	}
#else	// Win32
	#ifdef DEBUG
		NHDEBUG(1, (_T("%s\n"), szDummy));
	#endif
#endif // UNDER_CE
}

// [#2452] NH KSK 2016.11.10
void WINAPI HexaDump(BYTE *pData, int nLen, int nSection, BOOL bConvertedBinary, int nDumpType, BOOL bInitFlag)
{
#if (EMV_TEST_MODE)
	CString strTemp, strTime;
	CString strLog;

	SYSTEMTIME	ti;
	GetLocalTime(&ti);
	strTime.Format(L"[%04d/%02d/%02d %02d:%02d:%02d] ", ti.wYear, ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond);

	switch(nSection)
	{
	case 1:
		strLog.Format(L"\r\n%sSOURCE---> HOST SEND DATA\r\n", strTime);
		break;

	case 2:
		strLog.Format(L"\r\n%sSOURCE---> HOST RECEIVE DATA\r\n", strTime);		
		break;

	case 3:
		strLog.Format(L"\r\n%sSOURCE---> IFD SEND DATA\r\n", strTime);		
		break;

	case 4:
		strLog.Format(L"\r\n%sSOURCE---> ICC RECEIVE DATA\r\n", strTime);		
		break;

	default:
		break;
	}
	
	for (int i = 1; i <= nLen; i++)
	{
		if (bConvertedBinary == TRUE)
			strTemp.Format(L"0x%02X ", pData[i-1]);
		else
			strTemp.Format(L"%c ", pData[i-1]);
		strLog += strTemp;

		if ((i % 10) == 0)
		{
			strTemp = L"\r\n";
			strLog += strTemp;
		}
	}

	switch(nDumpType)
	{
	case SERIAL_LOG_TYPE:
		RETAILMSG(1, (strLog));
		break;

	case FILE_LOG_TYPE:
		{
			if (bInitFlag == TRUE)
				DeleteFile(L"\\ATM2\\LOG\\EmvL2.log");

			if (nLen == 0)
				return;

			FILE *Stream = NULL;
			long offset = 0;

			offset = 0;
			Stream = _tfopen(L"\\ATM2\\LOG\\EmvL2.log", _T("rb"));

			if (Stream != NULL)
			{
				fseek(Stream, 0, SEEK_SET);
				fread(&offset, sizeof(long), 1, Stream);
				fclose(Stream);
			}

			Stream = _tfopen(L"\\ATM2\\LOG\\EmvL2.log", _T("r+b"));
			if (Stream == NULL)
				Stream = _tfopen(L"\\ATM2\\LOG\\EmvL2.log", _T("w+b"));

			if (Stream != NULL)
			{
				char buf[4096];
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%S", strLog);

				if (offset == 0)
				{
					offset = 4;
					fseek(Stream, 0, SEEK_SET);
					fwrite(&offset, sizeof(long), 1, Stream);
				}

				fseek(Stream, offset, SEEK_SET);
				offset += fwrite(buf, sizeof(char), strlen(buf), Stream);

				// write offset
				{
					fseek(Stream, 0, SEEK_SET);
					fwrite(&offset, sizeof(long), 1, Stream);
				}

				fflush(Stream);
				fclose(Stream);
			}
		}
		break;

	default:
		break;
	}
#endif
}
// end of [#2452]

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: String2Hex()
 RETURN TYPE  : CString	:	hex-encoded ASCII representation of the string
 PARAMETER    : input	:	the string to convert
 DESCRIPTION  : Converts the input string into hex-encoded ASCII
-------------------------------------------------------------------*/
CString	WINAPI String2Hex(CString input)
{
	CString output;

	for (int i = 0; i < input.GetLength(); i++)
	{
		output.AppendFormat(L"%x", input.GetAt(i));
	}

	return output;
}

// [#178] [NH] KSK 2008.04.29
/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DeleteFileInDirectory()
 RETURN TYPE  : TRUE	:	삭제 성공.
				FALSE	:	삭제 실패.
 PARAMETER    : lpDstPath	:	삭제할 Directory.
				lpDstFile	:	삭제할 파일 형식.
 DESCRIPTION  : Dir에서 지정된 형식의 파일을 삭제 한다.
-------------------------------------------------------------------*/
BOOL WINAPI DeleteFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile)
{
	CString strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
	WCHAR	strSearchDir[255] = {};
	WCHAR	strNewDir[255] = {};
	WCHAR	strSrcFile[255] = {};

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;
	
	wcscpy_s(strSearchDir, _countof(strSearchDir), lpDstPath);
	wcscat_s(strSearchDir, _countof(strSearchDir), L"\\");
	wcscat_s(strSearchDir, _countof(strSearchDir), lpDstFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	while (wcscmp(L".", FileData.cFileName) == 0 || wcscmp(L"..", FileData.cFileName) == 0)
	{
		FindNextFile(hSearch, &FileData);
	}

	while (!bFinished)
	{
		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpDstFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, _countof(strNewDir));
				
				wcscpy_s(strNewDir, _countof(strNewDir), lpDstPath);
				wcscat_s(strNewDir, _countof(strNewDir), L"\\");
				wcscat_s(strNewDir, _countof(strNewDir), FileData.cFileName);
				
				DeleteFileInDirectory((LPCTSTR)strNewDir, L"*.*");

				// delete directory
				RemoveDirectory((LPCTSTR)strNewDir);
			}
		}
		else
		{
			memset(strSrcFile, 0, _countof(strSrcFile));

			// make source file full path
			wcscpy_s(strSrcFile, _countof(strSrcFile), lpDstPath);
			wcscat_s(strSrcFile, _countof(strSrcFile), L"\\");
			wcscat_s(strSrcFile, _countof(strSrcFile), FileData.cFileName);

			// [#2110] NH KSK 2011.12.22 File Delete시에 Read Only File을 삭제하지 못하는 Bug Fix (Read Only->Read/Write로 변경)
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				SetFileAttributes(strSrcFile, FILE_ATTRIBUTE_NORMAL);
			// end of [#2110]
			
			DeleteFile(strSrcFile);
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			bFinished = TRUE;
			if (GetLastError() != ERROR_NO_MORE_FILES)
			{
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	FindClose(hSearch);

	if (bError)
		return FALSE;

	return TRUE;
}

// [#2186] US KSK 2013.04.30
/*-------------------------------------------------------------------
CLASS    NAME: 
FUNCTION NAME: IsExistFile()
RETURN TYPE  : TRUE		:	지정된 Path에 파일에 존재한다. (file or folder)
FALSE		:	지정된 Path에 파일이 존재하지 않는다.
PARAMETER    : strFilePath	:	조회하고자 하는 파일 경로.
DESCRIPTION  : 특정 경로에 파일이 존재하는지 조회한다.
-------------------------------------------------------------------*/
BOOL WINAPI IsExistFile(CString strFilePath)
{
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;

	hSearch = FindFirstFile(strFilePath, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hSearch);

	return TRUE;
}


/*-------------------------------------------------------------------
CLASS    NAME: 
FUNCTION NAME: CountFileInDirectory()
RETURN TYPE  : TRUE	:	삭제 성공.
FALSE	:	삭제 실패.
PARAMETER    : lpDstPath	:	삭제할 Directory.
lpDstFile	:	삭제할 파일 형식.
DESCRIPTION  : Dir에서 지정된 형식의 파일을 Count 한다.
-------------------------------------------------------------------*/
BOOL WINAPI CountFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCount)
{
	CString strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22
	WCHAR	strSearchDir[255] = {};
	WCHAR	strNewDir[255] = {};

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;

	wcscpy_s(strSearchDir, sizeof(strSearchDir), lpDstPath);
	wcscat_s(strSearchDir, sizeof(strSearchDir), L"\\");
	wcscat_s(strSearchDir, sizeof(strSearchDir), lpDstFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		NHERROR((_T("No. [%s\\%s] SearhDir[%s] Directory. Error(0x%08X)(%ld)\n"), lpDstPath, lpDstFile, strSearchDir, dwError, dwError));
		return FALSE;
	}

	while (!bFinished)
	{
		//NHDEBUG(DBG_INFO, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpDstPath, FileData.cFileName));

		// Directory..
		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpDstFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, sizeof(strNewDir));

				wcscpy_s(strNewDir, sizeof(strNewDir), lpDstPath);
				wcscat_s(strNewDir, sizeof(strNewDir), L"\\");
				wcscat_s(strNewDir, sizeof(strNewDir), FileData.cFileName);

				CountFileInDirectory((LPCTSTR)strNewDir, L"*.*", nCount);

				nCount++;
			}
		}
		// File..
		else
		{
			nCount++;
		}

		Delay_Msg(5);

		if (!FindNextFile(hSearch, &FileData))
		{
			DWORD	dwError = GetLastError();
			bFinished = TRUE;
			if (dwError == ERROR_NO_MORE_FILES)
			{
				NHDEBUG(DBG_INFO, (_T("Found all of the files.\n")));
			}
			else
			{
				NHERROR((_T("Unable to fine next file. error[0x%08X][%ld]\n"), dwError, dwError));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	if (!FindClose(hSearch))
	{
		NHERROR((_T("Unable to close search handle\n")));
	}

	Delay_Msg(10);

	if (bError)
		return FALSE;

	return TRUE;
}


/*-------------------------------------------------------------------
CLASS    NAME: 
FUNCTION NAME: CopyFileInDirectory()
RETURN TYPE  : TRUE	:	복사 성공.
FALSE	:	복사 실패.
PARAMETER    : lpSrcPath	:	복사의 Source Directory.
lpScrFile	:	복사할 파일 형식.
lpDstPath	:	복사의 Destination Directory.
DESCRIPTION  : Src Dir에서 Dst Dir로 지정된 형식의 파일을 복사한다.
-------------------------------------------------------------------*/
BOOL WINAPI CopyFileInDirectory(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath)
{
	CString	strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
	WCHAR	strSearchDir[255] = {};
	WCHAR	strNewDir[255] = {};
	WCHAR	strSrcFile[255] = {};
	WCHAR	strDstFile[255] = {};

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;

	wcscpy_s(strSearchDir, sizeof(strSearchDir), lpSrcPath);
	wcscat_s(strSearchDir, sizeof(strSearchDir), L"\\");
	wcscat_s(strSearchDir, sizeof(strSearchDir), lpSrcFile);

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		NHERROR((_T("No. [%s\\%s] Directory. Error(0x08X)(%ld)\n"), lpSrcPath, lpSrcFile, dwError, dwError));
		return FALSE;
	}

	while (wcscmp(L".", FileData.cFileName) == 0 || wcscmp(L"..", FileData.cFileName) == 0)
	{
		FindNextFile(hSearch, &FileData);
	}

	while (!bFinished)
	{
		NHDEBUG(DBG_INFO, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpSrcPath, FileData.cFileName));

		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpSrcFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, _countof(strNewDir));

				wcscpy_s(strNewDir, _countof(strNewDir), lpSrcPath);
				wcscat_s(strNewDir, _countof(strNewDir), L"\\");
				wcscat_s(strNewDir, _countof(strNewDir), FileData.cFileName);

				// make destination file full path
				wcscpy_s(strDstFile, _countof(strDstFile), lpDstPath);
				wcscat_s(strDstFile, _countof(strDstFile), L"\\");
				wcscat_s(strDstFile, _countof(strDstFile), FileData.cFileName);

				// make new directory
				CreateDirectory(strDstFile, NULL);

				Delay_Msg(10);

				if (CopyFileInDirectory((LPCTSTR)strNewDir, L"*.*", strDstFile) == FALSE)
				{
					bError = TRUE;
					bFinished = TRUE;
					break;
				}
			}
		}
		else
		{
			memset(strSrcFile, 0, _countof(strSrcFile));
			memset(strDstFile, 0, _countof(strDstFile));

			// make source file full path
			wcscpy_s(strSrcFile, _countof(strSrcFile), lpSrcPath);
			wcscat_s(strSrcFile, _countof(strSrcFile), L"\\");
			wcscat_s(strSrcFile, _countof(strSrcFile), FileData.cFileName);

			// make destination file full path
			wcscpy_s(strDstFile, _countof(strDstFile), lpDstPath);
			wcscat_s(strDstFile, _countof(strDstFile), L"\\");
			wcscat_s(strDstFile, _countof(strDstFile), FileData.cFileName);

			// [#2110] NH KSK 2011.12.22 File Delete시에 Read Only File을 삭제하지 못하는 Bug Fix (Read Only->Read/Write로 변경)
//			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
//				SetFileAttributes(strDstFile, FILE_ATTRIBUTE_NORMAL);
			// end of [#2110]
			SetFileAttributes(strDstFile, FILE_ATTRIBUTE_NORMAL);	// Delete할 File 속성을 NORMAL로 변경

			DeleteFile(strDstFile);
			Delay_Msg(10);

			if (CopyFile(strSrcFile, strDstFile, FALSE) == FALSE)
			{
				DWORD dwError = GetLastError();
				NHERROR((L"[UPDATE] Failed to Copy (0x%08X)(%ld) [%s] -> [%s]\n", dwError, dwError, strSrcFile, strDstFile));
				bError = TRUE;
				bFinished = TRUE;
				break;
			}
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			DWORD	dwError = GetLastError();
			bFinished = TRUE;

			if (dwError == ERROR_NO_MORE_FILES)
			{
				NHDEBUG(DBG_INFO, (_T("Found all of the files.\n")));
			}
			else
			{
				NHDEBUG(DBG_INFO, (_T("Unable to fine next file. error[0x%08X][%ld]\n"), dwError, dwError));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	if (!FindClose(hSearch))
	{
		NHERROR((_T("Unable to close search handle\n")));
	}

	Delay_Msg(10);

	if (bError)
		return FALSE;

	return TRUE;
}
// end of [#2186]


DllUseport BOOL WINAPI CreateDirectoryWithIntermediate(LPCTSTR lpszPath)
{
	TCHAR szPathBuffer[MAX_PATH];

	size_t len = _tcslen( lpszPath );

	for ( size_t i = 0 ; i < len ; i++ )
	{
		szPathBuffer[i] = *( lpszPath + i );
		if ( szPathBuffer[i] == _T('\\') || szPathBuffer[i] == _T('/') )
		{
			szPathBuffer[i + 1] = NULL;
			if ( ! IsExistFile( szPathBuffer ) )
			{
				NHDEBUG(DBG_CALL, (L"  ===> Creating Folder [%s]... !\n", szPathBuffer));

				if ( ! ::CreateDirectory( szPathBuffer, NULL ) )
				{
					if ( GetLastError() != ERROR_ALREADY_EXISTS )
						return FALSE;
				}
			}
		}
	}

	::CreateDirectory( lpszPath, NULL );

	return TRUE;
}

DllUseport BOOL WINAPI GetFilesInDirectory(LPCTSTR lpSrcPath, CStringArray &paFileList)
{
	CString strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22
	WCHAR	strSearchDir[255] = {};
	WCHAR	strNewDir[255] = {};

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;

	wcscpy_s(strSearchDir, sizeof(strSearchDir), lpSrcPath);
	wcscat_s(strSearchDir, sizeof(strSearchDir), L"\\*.*");

	hSearch = FindFirstFile(strSearchDir, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		NHERROR((_T("No. [%s] Directory. Error(0x%08X)(%ld)\n"), lpSrcPath, dwError, dwError));
		return FALSE;
	}

	while (!bFinished)
	{
		if (wcscmp(FileData.cFileName, L".") != 0 && wcscmp(FileData.cFileName, L"..")) 
		{
			// Directory..
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				memset(strNewDir, 0, sizeof(strNewDir));

				wcscpy_s(strNewDir, sizeof(strNewDir), lpSrcPath);
				wcscat_s(strNewDir, sizeof(strNewDir), L"\\");
				wcscat_s(strNewDir, sizeof(strNewDir), FileData.cFileName);

				GetFilesInDirectory((LPCTSTR)strNewDir, paFileList);
			}
			// File..
			else
			{
				CString filePath = lpSrcPath;
				filePath += L"\\";
				filePath += FileData.cFileName;

				paFileList.Add(filePath);
			}
		}

		if (!FindNextFile(hSearch, &FileData))
		{
			DWORD	dwError = GetLastError();
			bFinished = TRUE;
			if (dwError == ERROR_NO_MORE_FILES)
			{
				NHDEBUG(DBG_INFO, (_T("Found all of the files.\n")));
			}
			else
			{
				NHERROR((_T("Unable to fine next file. error[0x%08X][%ld]\n"), dwError, dwError));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	if (!FindClose(hSearch))
	{
		NHERROR((_T("Unable to close search handle\n")));
	}

	if (bError)
		return FALSE;

	return TRUE;
}

// [#2545] US Justin 2018.04.13
DllUseport  BOOL	WINAPI ExtractZIPAll(CString pZipFileName, CString pDestFolder, bool pDeleteZipAfterExtract)
{
	if (!IsExistFile(pZipFileName))
	{
		NHDEBUG(DBG_CALL, (L"  ===> File not found [%s] !!\n", pZipFileName));
		return false;
	}

	HZIP		hz;
	ZRESULT		zr;
	ZIPENTRY	ze;

	int			nNumItems = 0;
	CString		strResourceFolderPath = _T("");
	CString		strDestFileName = _T("");

	CreateDirectoryWithIntermediate(pDestFolder);
	if (IsExistFile(pDestFolder))
	{
		NHDEBUG(DBG_CALL, (L"  ===> Extracting [%s] to [%s]...\n", pZipFileName, pDestFolder));

		hz = uzOpenZip(pZipFileName, 0);
		uzGetZipItem(hz, -1, &ze);
		nNumItems = ze.index;

		for (int i = 0; i < nNumItems; i++)
		{
			uzGetZipItem(hz, i, &ze);
			strDestFileName.Format(L"%s\\%s", pDestFolder, ze.name);
			strDestFileName.Replace(L"\\\\", L"\\");
			zr = uzUnzipItem(hz, i, strDestFileName);

			NHDEBUG(DBG_CALL, (L"  ===> Extracted: [%s]\n", ze.name, strDestFileName));
		}

		uzCloseZip(hz);

		if (pDeleteZipAfterExtract)
			DeleteFile(pZipFileName);
	}
	return TRUE;
}
// End of [#2545]

// [#2497] NH woooZ 2017.09.26  ALPHI server
VOID WINAPI WriteAlphiLogToFile(CString pMessage)
{
#if ALPHI_DEBUG

#define	ALPHI_LOG_HEADER_LENGTH		8
#define ALPHI_LOG_MAXSIZE			(512 * UNIT_KILOBYTE)

// [#2575] AU HJAHN 2018.09.07 codesonar 지적 사항 수정
	
	//HANDLE hFile = CreateFile (
	HANDLE hFile = INVALID_HANDLE_VALUE;
	
	hFile = CreateFile (
		ALPHI_LOG_FILE,
		GENERIC_WRITE|GENERIC_READ,          // Open for writing
		0,                      // Do not share
		NULL,                   // No security
		OPEN_ALWAYS,			// Open or create
		FILE_ATTRIBUTE_NORMAL,  // Normal file
		NULL);                  // No template file
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD	dwWritten=0;
		DWORD	dwLastWrittenPos=0;
// End of [#2575]
		char	arTempBuf[1024];
		CString strAlphiLog;

		SYSTEMTIME	ti;
		GetLocalTime(&ti);

		strAlphiLog.Format(L"[%02d/%02d %02d:%02d:%02d]%s\n", 
			ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond, pMessage);

		// get last written position.
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if (ReadFile(hFile, arTempBuf, ALPHI_LOG_HEADER_LENGTH, &dwWritten, NULL))
		{
			dwLastWrittenPos = atoi(arTempBuf);		// 0 will be returned if it is invalid header.

			// no length found? set by default (0) and reset log file.
			if (dwLastWrittenPos < ALPHI_LOG_HEADER_LENGTH)
			{
				dwLastWrittenPos = ALPHI_LOG_HEADER_LENGTH;
				char index[ALPHI_LOG_HEADER_LENGTH] = "";
				_itoa_s(dwLastWrittenPos, index, ALPHI_LOG_HEADER_LENGTH, 10);

				WriteFile(hFile, index, ALPHI_LOG_HEADER_LENGTH, &dwWritten, NULL);
				SetEndOfFile(hFile);
			}

			// reset position when it exceeds max.
			if (dwLastWrittenPos > ALPHI_LOG_MAXSIZE)
				dwLastWrittenPos = ALPHI_LOG_HEADER_LENGTH;

			SetFilePointer(hFile, dwLastWrittenPos, NULL, FILE_BEGIN);
		}

		// write a log at proper position.
		if (WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strAlphiLog, -1, 
			(LPSTR)&arTempBuf[0], 512,
			NULL, NULL) != 0)
		{
			SetFilePointer(hFile, dwLastWrittenPos, NULL, FILE_BEGIN);
			if (WriteFile(hFile, arTempBuf, strAlphiLog.GetLength(), &dwWritten, NULL))
			{
				// update last written position at header.
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
				dwLastWrittenPos += dwWritten;
				_itoa_s(dwLastWrittenPos, arTempBuf, ALPHI_LOG_HEADER_LENGTH, 10);
				WriteFile(hFile, arTempBuf, ALPHI_LOG_HEADER_LENGTH, &dwWritten, NULL);
			}

		}

		CloseHandle(hFile);
	}

#endif
}
// end of [#2497]
