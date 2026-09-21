#include "stdafx.h"
#include "FileMgr.h"

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

/*-------------------------------------------------------------------
 CLASS    NAME: CFileMgr
 FUNCTION NAME: CFileMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CFileMgr::CFileMgr()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFileMgr
 FUNCTION NAME: ~CFileMgr()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CFileMgr::~CFileMgr()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DeleteFileInDirectory()
 RETURN TYPE  : TRUE	:	삭제 성공.
				FALSE	:	삭제 실패.
 PARAMETER    : lpDstPath	:	삭제할 Directory.
				lpDstFile	:	삭제할 파일 형식.
				bShowCount  :   삭제되는 파일의 개수를 표시 여부
				nCurCnt		:	현재 진행된 카운트.
				nTotalCnt	:	진행율의 전체 개수.
 DESCRIPTION  : Dir에서 지정된 형식의 파일을 삭제 한다.
-------------------------------------------------------------------*/
BOOL CFileMgr::DeleteFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, BOOL bShowCount, int &nCurCnt, int nTotalCnt)
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
	
	wcscpy_s(strSearchDir, sizeof(strSearchDir), lpDstPath);
	wcscat_s(strSearchDir, sizeof(strSearchDir),  L"\\");
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
		NHDEBUG(DBG_INFO, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpDstPath, FileData.cFileName));

		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpDstFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, sizeof(strNewDir));
				
				wcscpy_s(strNewDir, sizeof(strNewDir), lpDstPath);
				wcscat_s(strNewDir, sizeof(strNewDir), L"\\");
				wcscat_s(strNewDir, sizeof(strNewDir), FileData.cFileName);
				
				DeleteFileInDirectory((LPCTSTR)strNewDir, L"*.*", bShowCount, nCurCnt, nTotalCnt);

				if (nTotalCnt > 0)
					nCurCnt++;

				// delete directory
				if (RemoveDirectory((LPCTSTR)strNewDir) == FALSE)
				{
					DWORD dwError = GetLastError();
					NHERROR((_T("RemoveDirectory Failed. error[0x%08X][%ld]\n"), dwError, dwError));
				}
				WaitForMilliSecond(10);
			}
		}
		else
		{
			memset(strSrcFile, 0, sizeof(strSrcFile));

			// make source file full path
			wcscpy_s(strSrcFile, sizeof(strSrcFile), lpDstPath);
			wcscat_s(strSrcFile, sizeof(strSrcFile), L"\\");
			wcscat_s(strSrcFile, sizeof(strSrcFile), FileData.cFileName);

			if (nTotalCnt > 0)
				nCurCnt++;

			// show count on current screen.
			if (bShowCount == TRUE)
			{
				if (nTotalCnt > 0)
				{
					strTemp.Format(L"[ Deleting... ]  %3d%% - %4d/%4d%s%s", 
											(int)((((float)nCurCnt/(float)nTotalCnt)*100.0)+0.5), 
											nCurCnt, nTotalCnt, SCR_CMD_DELIMITER, FileData.cFileName);
				}
				else
				{
					strTemp.Format(L"[ Deleting... ]%s%s", SCR_CMD_DELIMITER, FileData.cFileName);
				}

				// KSK 2011.04.02 Delay를 주어 Copy File Fail 현상 수정
				NHERROR((L"%s\n", strTemp));

				ShowString(2, strTemp);
				UpdateScreen();

				WaitForMilliSecond(10);
			}

			// [#2110] NH KSK 2011.12.22 File Delete시에 Read Only File을 삭제하지 못하는 Bug Fix (Read Only->Read/Write로 변경)
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				SetFileAttributes(strSrcFile, FILE_ATTRIBUTE_NORMAL);
			// end of [#2110]

			if (DeleteFile(strSrcFile) == FALSE)
			{
				DWORD dwError = GetLastError();
				NHERROR((_T("RemoveDirectory Failed. error[0x%08X][%ld]\n"), dwError, dwError));
			}
			WaitForMilliSecond(10);

		}

		if (!FindNextFile(hSearch, &FileData))
		{
			DWORD dwError = GetLastError();
			bFinished = TRUE;
			if (dwError == ERROR_NO_MORE_FILES)
			{
				NHDEBUG(DBG_INFO, (_T("Found all of the files.\n")));
			}
			else
			{
				NHERROR((_T("Unable to fine next file. error[0x%08X][%ld]\n"), dwError));
				bError = TRUE;
			}
		}
	}

	// Close the Search handle
	if (!FindClose(hSearch))
	{
		NHERROR((_T("Unable to close search handle\n")));
	}

	WaitForMilliSecond(10);

	if (bError)
		return FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: IsExistFile()
 RETURN TYPE  : TRUE		:	지정된 Path에 파일에 존재한다. (file or folder)
				FALSE		:	지정된 Path에 파일이 존재하지 않는다.
 PARAMETER    : strFilePath	:	조회하고자 하는 파일 경로.
 DESCRIPTION  : 특정 경로에 파일이 존재하는지 조회한다.
-------------------------------------------------------------------*/
BOOL CFileMgr::IsExistFile(CString strFilePath)
{
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22

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
BOOL CFileMgr::CountFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCount)
{
	CString strTemp;
	WIN32_FIND_DATA	FileData;
	HANDLE hSearch = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22
	WCHAR	strSearchDir[255] = {};
	WCHAR	strNewDir[255] = {};

	BOOL bFinished = FALSE;
	BOOL bError = FALSE;
	int	 nFileCount = 0;
	
	memset(strSearchDir, 0, sizeof(strSearchDir));
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

 		WaitForMilliSecond(5);

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

	WaitForMilliSecond(10);

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
BOOL CFileMgr::CopyFileInDirectory(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath, BOOL bShowCount, int &nCurCnt, int nTotalCnt)
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
	
	memset(strSearchDir, 0, sizeof(strSearchDir));
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

	while (!bFinished)
	{
		//NHDEBUG(DBG_INFO, (_T("[FIND][%03d] - [%s][%s]\n"), ++nFileCount, lpSrcPath, FileData.cFileName));

		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(lpSrcFile, FileData.cFileName) != 0)
			{
				memset(strNewDir, 0, sizeof(strNewDir));
				
				wcscpy_s(strNewDir, sizeof(strNewDir), lpSrcPath);
				wcscat_s(strNewDir, sizeof(strNewDir), L"\\");
				wcscat_s(strNewDir, sizeof(strNewDir), FileData.cFileName);
				
				// make destination file full path
				wcscpy_s(strDstFile, sizeof(strDstFile), lpDstPath);
				wcscat_s(strDstFile, sizeof(strDstFile), L"\\");
				wcscat_s(strDstFile, sizeof(strDstFile), FileData.cFileName);

				// make new directory
				CreateDirectory(strDstFile, NULL);

				if (nTotalCnt > 0)
					nCurCnt++;

				WaitForMilliSecond(10);

				if (CopyFileInDirectory((LPCTSTR)strNewDir, L"*.*", strDstFile, bShowCount, nCurCnt, nTotalCnt) == FALSE)
				{
					bError = TRUE;
					bFinished = TRUE;
					break;
				}
			}
		}
		else
		{
			memset(strSrcFile, 0, sizeof(strSrcFile));
			memset(strDstFile, 0, sizeof(strDstFile));

			// make source file full path
			wcscpy_s(strSrcFile, sizeof(strSrcFile), lpSrcPath);
			wcscat_s(strSrcFile, sizeof(strSrcFile), L"\\");
			wcscat_s(strSrcFile, sizeof(strSrcFile), FileData.cFileName);
			
			// make destination file full path
			wcscpy_s(strDstFile, sizeof(strDstFile), lpDstPath);
			wcscat_s(strDstFile, sizeof(strDstFile), L"\\");
			wcscat_s(strDstFile, sizeof(strDstFile), FileData.cFileName);

			if (nTotalCnt > 0)
				nCurCnt++;

			if (bShowCount == TRUE)
			{
				if (nTotalCnt > 0)
				{
					strTemp.Format(L"[ Copying... ]  %3d%% - %4d/%4d%sFROM : %s%sTO     : %s",
											(int)((((float)nCurCnt/(float)nTotalCnt)*100.0)+0.5),
											nCurCnt, nTotalCnt,
											SCR_CMD_DELIMITER,
											FileData.cFileName, 
											SCR_CMD_DELIMITER,
											FileData.cFileName);
				}
				else
				{
					strTemp.Format(L"[ Copying... ]%sFROM : %s%sTO     : %s", 
											SCR_CMD_DELIMITER,
											FileData.cFileName, 
											SCR_CMD_DELIMITER,
											FileData.cFileName);
				}

				// KSK 2011.04.02 Delay를 주어 Copy File Fail 현상 수정
				NHERROR((L"%s\n", strTemp));
				
				ShowString(2, strTemp);
				UpdateScreen();

				WaitForMilliSecond(10);
			}

			// [#2110] NH KSK 2011.12.22 File Delete시에 Read Only File을 삭제하지 못하는 Bug Fix (Read Only->Read/Write로 변경)
//			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
//				SetFileAttributes(strDstFile, FILE_ATTRIBUTE_NORMAL);
			// end of [#2110]

			SetFileAttributes(strDstFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFile(strDstFile);
			WaitForMilliSecond(10);

			if (CopyFile(strSrcFile, strDstFile, FALSE) == FALSE)
			{
				DWORD dwError = GetLastError();
				NHERROR((L"[UPDATE] Failed to Copy (0x%08X)(%ld) [%s] -> [%s]\n", dwError, dwError, strSrcFile, strDstFile));
				bError = TRUE;
				bFinished = TRUE;
				break;
			}

//			WaitForMilliSecond(10);
			// [#11] NH KSK 2010.09.30 File Copy시 Fail나는 현상을 위해 Delay값 변경
			strTemp.Format(L"%s", lpDstPath);

			//if (strTemp.Left(4) == L"\\ATM")
			//{
			//	// Flash Rom에 Copy하는 경우 Delay 100ms
			//	WaitForMilliSecond(100);
			//}
			//else
			//{
			//	// RAM 영역에 Copy하는 경우 Delay 10ms
			//	WaitForMilliSecond(10);
			//}
			// end of [#11]
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

	WaitForMilliSecond(10);

	if (bError)
		return FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: IsFolder()
 RETURN TYPE  : BOOL	:	TRUE  - Folder
							FALSE - File or don't exist
 PARAMETER    : strFileName : File Name
 DESCRIPTION  : 현재 File이 Folder or File 인지 확인한다.
-------------------------------------------------------------------*/
BOOL CFileMgr::IsFolder(CString strFileName)
{
	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;	// [#2022] NH KSK 2011.02.22

	hSearch = FindFirstFile(strFileName, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hSearch);

	if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;
	
	return FALSE;
}


BOOL CFileMgr::UnzipFileToDirectory(LPCTSTR lpSrcFile, LPCTSTR lpDstPath)
{
	HZIP		hz;
	ZRESULT		zr;
	ZIPENTRY	ze;
	int			nNumItems, i;
	CString		strTempFileName, strInfo;
	CString		strTemp;

	//PrepareScreen(SCR_FILECTRL);
	//ShowString(1, L"Check update files");
	//ShowString(2, L"");
	//ShowString(3, L"");

	//NH_OS_VERSION	eOSVersion = m_SystemConfig.GetOSVersion();

	//if (eOSVersion == NH_OS_MX5200SE
	//	|| eOSVersion == NH_OS_MX2800SE)
	//	ShowString(6, L"Please wait until ATM reboot");
	//else
	//	ShowString(5, L"Please wait until ATM reboot");

	//ShowScreen(SCR_FILECTRL);

	// create folder
	CreateDirectoryWithIntermediate(lpDstPath);

	// zip file name
	hz = uzOpenZip(lpSrcFile, 0);

	// Get number of files
	uzGetZipItem(hz, -1, &ze);
	nNumItems = ze.index;

	for (i = 0; i < nNumItems; i++)
	{
		uzGetZipItem(hz, i, &ze);

		strTemp = ze.name;
		strTempFileName.Format(L"%s\\%s", lpDstPath, ze.name);

		// Show process information to customer
		strInfo.Format(L"[ Extracting... ]  %3d%% - %4d/%4d%s%s",
			(int)((((float)(i+1)/(float)nNumItems)*100.0)+0.5),
			i+1, nNumItems, SCR_CMD_DELIMITER, ze.name);
		ShowString(2, strInfo);
		UpdateScreen();

		WaitForMilliSecond(10);

		zr = uzUnzipItem(hz, i, strTempFileName);

		if (ZR_OK != zr)
		{
			NHERROR((_T("UnzipItem Failed.. Return(0x%08X)\n"), zr));
			break;
		}

		WaitForMilliSecond(10);
	}

	uzCloseZip(hz);

	// success to validate
	if (nNumItems > 0 && i >= nNumItems)
	{
		return TRUE;
	}

	return FALSE;
}


bool CFileMgr::ShCopyDirectory(LPCTSTR lpSrcPath, LPCTSTR lpDstPath)
{
	TCHAR szFrom[MAX_PATH];
	_tcscpy_s(szFrom, MAX_PATH, lpSrcPath);
	szFrom[_tcsclen(lpSrcPath) + 1] = NULL;

	TCHAR szTo[MAX_PATH];
	_tcscpy_s(szTo, MAX_PATH, lpDstPath);
	szTo[_tcsclen(lpDstPath) + 1] = NULL;

	SHFILEOPSTRUCT fileOp = {0};
	fileOp.wFunc = FO_COPY;
	fileOp.pFrom = szFrom;
	fileOp.pTo = szTo;
#ifdef NH_DEBUG
	fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
#else
	fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
#endif

	NHDEBUG(DBG_INFO, (_T("SHFileOperation() ... COPY [%s] to [%s] ... \n"), lpSrcPath, lpDstPath));
	int result = SHFileOperation(&fileOp);
	NHDEBUG(DBG_INFO, (_T("SHFileOperation() ... DONE [0x%x] \n"), result));
	
	return result == 0;
}

bool CFileMgr::ShDeleteDirectory(LPCTSTR lpDstPath)
{
	TCHAR szFrom[MAX_PATH];
	_tcscpy_s(szFrom, MAX_PATH, lpDstPath);
	szFrom[_tcsclen(lpDstPath) + 1] = NULL;

	SHFILEOPSTRUCT fileOp = {0};
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = szFrom;
#ifdef NH_DEBUG
	fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
#else
	fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
#endif

	NHDEBUG(DBG_INFO, (_T("SHFileOperation() ... DELETE [%s] ... \n"), lpDstPath));
	int result = SHFileOperation(&fileOp);
	NHDEBUG(DBG_INFO, (_T("SHFileOperation() ... DONE [0x%x] \n"), result));

	return result == 0;
}
