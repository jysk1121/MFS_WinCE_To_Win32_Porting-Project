#include "stdafx.h"
#include <windows.h>

#define LOG_PATH			_T("\\ATM2\\LOG")
#define MAX_DUMP_INDEX		10

BOOL	GetFileName(LPCTSTR lpszFullPath, LPTSTR lpszFileName, int FileNameSize);
DWORD	GetNextDumpIndex();
void	SetLastDumpIndex(DWORD dwDumpIndex);
void	RemoveDumpFile(DWORD dwDumpIndex);
void	CopyDumpFile(DWORD dwDumpIndex);

// [#2059] NH KJW 2011.05.12 DumpFile.cpp에서 로깅 기능 추가
extern TCHAR	gszExceptionMsg[];
extern int		gnUseExceptionMsg;
extern void WriteLog();
// end of [#2059]

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: GetFileName
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Full Path에서 파일 이름을 추출 한다.
				맨 뒤에서 부터 \ 다음을 파일로 가정함.
-------------------------------------------------------------------*/
BOOL GetFileName(LPCTSTR lpszFullPath, LPTSTR lpszFileName, int FileNameSize)
{
	int	nFullPathSize = 0;

	if (lpszFullPath == NULL || lpszFileName == NULL)
		return false;

	// find '\'
	nFullPathSize = _tcslen(lpszFullPath);
	for (int i = nFullPathSize; i >= 0; i--)
	{
		if (lpszFullPath[i] == '\\')
		{
			int	nCopySize = nFullPathSize - i;

			if (FileNameSize < nCopySize)
				return FALSE;

			memcpy(lpszFileName, &lpszFullPath[i+1], (nCopySize * sizeof(TCHAR)));
			lpszFileName[nCopySize] = 0x0;

			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: RemoveDumpFile
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 주어진 Index가 들어간 Dump 파일을 삭제 한다.
-------------------------------------------------------------------*/
void RemoveDumpFile(DWORD dwDumpIndex)
{
	TCHAR	szFindName[MAX_PATH];
	HANDLE	hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	FindData;
	int		curIndex = 1;
	BOOL	bFindNext = TRUE;

	_stprintf(szFindName, _T("%s\\Dump_%02d_*.log"), LOG_PATH, dwDumpIndex);

	hFind = ::FindFirstFile(szFindName, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		bFindNext = TRUE;

		while(bFindNext == TRUE)
		{
			if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				_stprintf(szFindName, _T("%s\\%s"), LOG_PATH, FindData.cFileName);
				
				DeleteFile(szFindName);
			}

			bFindNext = ::FindNextFile(hFind, &FindData);
		}

		FindClose(hFind);
	}
}

// [#2059] NH KJW 2011.05.13
BOOL CopyFileSync( LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName )
{
	// source
	HANDLE hFileSrc = INVALID_HANDLE_VALUE;
	if( (hFileSrc = ::CreateFile(lpExistingFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE )
	{
		gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to open source(%s). err(%d)\n"), lpExistingFileName, GetLastError());
		WriteLog();
		gnUseExceptionMsg = 0;

		return FALSE;
	}

	// target
	HANDLE hFileTgt = INVALID_HANDLE_VALUE;

	if( (hFileTgt = ::CreateFile(lpNewFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE )
	{
		gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to open target(%s). err(%d)\n"), lpNewFileName, GetLastError());
		WriteLog();
		gnUseExceptionMsg = 0;

		CloseHandle( hFileSrc );

		return FALSE;
	}

	// 복사
	BOOL bCompleted = FALSE;
	BYTE bytBuffer[1024] = {0,};
	DWORD nNumberOfBytesRead = 0;
	while( (::ReadFile(hFileSrc, bytBuffer, sizeof(bytBuffer), &nNumberOfBytesRead, NULL) == TRUE) && (nNumberOfBytesRead > 0) )
	{
		bCompleted = TRUE;

		DWORD nNumberOfBytesWritten = 0;
		if( (::WriteFile(hFileTgt, bytBuffer, nNumberOfBytesRead, &nNumberOfBytesWritten, NULL) == FALSE) || (nNumberOfBytesRead != nNumberOfBytesWritten) )
		{
			gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to write target. nNumberOfBytesRead(%d), nNumberOfBytesWritten(%d), err(%d)\n"), nNumberOfBytesRead, nNumberOfBytesWritten, GetLastError());
			WriteLog();
			gnUseExceptionMsg = 0;
			bCompleted = FALSE;
			break;
		}
	}

	CloseHandle( hFileSrc );
	CloseHandle( hFileTgt );

	// 복사 실패시 target 파일 삭제
	if( bCompleted == FALSE )
	{
		DeleteFile( lpNewFileName );
	}

	return bCompleted;
}

// end of [#2059]


/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: CopyDumpFile
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 주어진 Index로 Dump가 남겨진 모든 파일을 복사한다.
				Windows 폴더 -> LOG 폴더로
-------------------------------------------------------------------*/
void CopyDumpFile(DWORD dwDumpIndex)
{
	HKEY	hKey = NULL;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\ErrorReporting\\DumpFiles"), 0, NULL, &hKey) == ERROR_SUCCESS)
	{
		LONG	Result;
		DWORD	dwIndex = 0;
		TCHAR	szFileName[MAX_PATH+1];
		TCHAR	szDumpName[MAX_PATH+1];
		TCHAR	szKeyName[MAX_PATH-20];
		DWORD	szKeyNameSize = MAX_PATH;
		DWORD	Type = 0;
		BYTE	Data;
		DWORD	DataSize = 1;

		SYSTEMTIME	ti;

		Result = RegEnumValue(hKey, dwIndex, szKeyName, &szKeyNameSize, NULL, &Type, &Data, &DataSize);

		// [#2059] NH KJW 2011.05.12
		if( Result != ERROR_SUCCESS )
		{
			gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to RegEnumValue(%s). err(%d)\n"), szKeyName, GetLastError());
			WriteLog();
			gnUseExceptionMsg = 0;
		}
		// end of [#2059

		while(Result == ERROR_SUCCESS)
		{
			szKeyName[szKeyNameSize] = 0x0;

			if (GetFileName(szKeyName, szFileName, sizeof(szFileName)) == TRUE)
			{
				GetLocalTime(&ti);

				memset(szDumpName, 0x0, sizeof(szDumpName));
				_stprintf(szDumpName, _T("%s\\Dump_%02d_%s_%02d%02d%02d_%02d.log"), LOG_PATH, dwDumpIndex, szFileName, ti.wHour, ti.wMinute, ti.wSecond, (dwIndex+1));

				// [#2059] NH KJW 2011.05.12 Dump 파일 복사 방식을 변경. Async CopyFile() -> Sync ReadFile() & WriteFile()
				//CopyFile(szKeyName, szDumpName, FALSE);
				if( CopyFileSync(szKeyName, szDumpName) == FALSE )
				{
					gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to CopyFile(%s,%s). err(%d)\n"), szKeyName, szDumpName, GetLastError());
					WriteLog();
					gnUseExceptionMsg = 0;
				}
				// end of [#2059]
			}
			// [#2059] NH KJW 2011.05.12 Dump 파일 생성 실패 지점 로그 기록
			else
			{
				gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to GetFileName(%s,%s,%d). err(%d)\n"), szKeyName, szFileName, sizeof(szFileName), GetLastError());
				WriteLog();
				gnUseExceptionMsg = 0;
			}
			// end of [#2059]

			// next value 
			memset(szKeyName, 0x0, sizeof(szKeyName));
			szKeyNameSize = sizeof(szKeyName);
			DataSize = 1;
			dwIndex++;

			Result = RegEnumValue(hKey, dwIndex, szKeyName, &szKeyNameSize, NULL, &Type, &Data, &DataSize);
		}//while(Result == ERROR_SUCCESS)

		RegCloseKey(hKey);
	}
	// [#2059] NH KJW 2011.05.12 Dump 파일 복사 실패시 로그 기록
	else
	{
		gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to RegOpenKeyEx(%s). err(%d)\n"), _T("System\\ErrorReporting\\DumpFiles"), GetLastError());
		WriteLog();
		gnUseExceptionMsg = 0;
	}
	// end of [#2059]
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: GetNextDumpIndex
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 저장할 다음 Dump Index를 조회 한다.
-------------------------------------------------------------------*/
DWORD GetNextDumpIndex()
{
	TCHAR	szFindName[MAX_PATH];
	HANDLE	hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	FindData;
	int		curIndex = 0;

	// [#2059] NH KJW 2011.05.12 Dump_Last_xx.log 길이가 아닐 경우 오작동 가능성있어, * -> ?? 로 2자리 길이 명시함.
	//_stprintf(szFindName, _T("%s\\Dump_Last_*.log"), LOG_PATH);
	_stprintf(szFindName, _T("%s\\Dump_Last_??.log"), LOG_PATH);
	// end of [#2059]

	hFind = ::FindFirstFile(szFindName, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			int		tmpIndex = 0;

			TCHAR	szIndexString[3];
			
			// need XX of "Dump_Last_XX.." format
			szIndexString[0] = FindData.cFileName[10];
			szIndexString[1] = FindData.cFileName[11];
			szIndexString[2] = 0x0;

			tmpIndex = _ttoi(szIndexString);

			if (tmpIndex > curIndex)
				curIndex = tmpIndex;
		}

		FindClose(hFind);
	}

	// next index
	curIndex++;

	// check max.
	if (curIndex > MAX_DUMP_INDEX)
	{
		curIndex = 1;

		// [#2059] NH KJW 2011.05.12 Dump File Index가 변경되는 지점 로깅
		gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("curIndex > MAX_DUMP_INDEX -> curIndex = 1\n"));
		WriteLog();
		gnUseExceptionMsg = 0;
		// end of [#2059]
	}

	return (DWORD)curIndex;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: SetLastDumpIndex
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 마지막 저장한 Dump Index를 저장한다.
-------------------------------------------------------------------*/
void SetLastDumpIndex(DWORD dwDumpIndex)
{
	TCHAR	szFileName[MAX_PATH];
	HANDLE	hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	FindData;
	int		curIndex = 1;

	////////////////////////////////
	// remove previous last index file

	// [#2059] NH KJW 2011.05.12 Dump_Last_xx.log 길이가 아닐 경우 오작동 가능성있어, * -> ?? 로 2자리 길이 명시함.
	//_stprintf(szFileName, _T("%s\\Dump_Last_*.log"), LOG_PATH);
	_stprintf(szFileName, _T("%s\\Dump_Last_??.log"), LOG_PATH);
	// end of [#2059]
	
	hFind = ::FindFirstFile(szFileName, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			_stprintf(szFileName, _T("%s\\%s"), LOG_PATH, FindData.cFileName);
			
			DeleteFile(szFileName);
		}

		FindClose(hFind);
	}

	////////////////////////////////
	// create new last index file

	HANDLE hFile = INVALID_HANDLE_VALUE;

	_stprintf(szFileName, _T("%s\\Dump_Last_%02d.log"), LOG_PATH, dwDumpIndex);

	hFile = ::CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	// [#2059] NH KJW 2011.05.12 Dump_Last_xx.log 생성 실패시 로깅
	else
	{
		gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to CreateFile(%s). err(%d)\n"), szFileName, GetLastError());
		WriteLog();
		gnUseExceptionMsg = 0;
	}
	// end of [#2059]
}