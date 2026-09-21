// WatchDog.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//
#include "stdafx.h"
#define _INC_CRTDEFS
#include "WatchDog.h"
#include <windows.h>

#define MAX_LOGGING_SIZE		(1024 * 4)	// 4KB
#define MAX_MSG_BUFF			1024
#define EXCEPTION_CHECK_TIME	60000		// 1-min
//#define REBOOT_WAIT_TIME		0			// 0-min [#2059] NH KJW 2011.05.12 매크로 미사용 삭제

#define LOG_FOLDER		_T("\\ATM2\\LOG")
#define LOG_FILE_NAME	_T("\\ATM2\\LOG\\WatchDog.log")

TCHAR	gszExceptionName[MAX_PATH+1] = {_T("APPLICATION ERROR")};
TCHAR	gszNetworkExceptionName[MAX_PATH+1] = {_T("WINDOWS CE NETWORKING")};	// [#2156] NH KSK 2012.11.08 DHCP TEST
TCHAR	gszExceptionMsg[MAX_MSG_BUFF] = {0, };
int		gnUseExceptionMsg = 0;

HANDLE	ghEvent = NULL;		// never signaled.
HANDLE	ghSingleton = NULL;

void RunErrorReporting();
void CheckException();
bool IsNameMatch(TCHAR *pszTitle, TCHAR *pszWant);
void SystemReboot();
void WriteLog();

extern DWORD GetNextDumpIndex();
extern void	SetLastDumpIndex(DWORD dwDumpIndex);
extern void	RemoveDumpFile(DWORD dwDumpIndex);
extern void	CopyDumpFile(DWORD dwDumpIndex);

extern int	WINAPI RegGetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int InitValue);
extern int	WINAPI RegSetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int SetValue);
extern int	WINAPI RegSetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, LPCTSTR SetValue);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	////////////////////////////////////////
	// check instance count
	{
		// 만약, 에러가 발생하더라도 주목적을 위하여 계속 진행한다.

		ghSingleton = CreateEvent(NULL, TRUE, FALSE, _T("__NAUTILUS_HYOSUNG_WCE_WATCH_DOG__"));
		if (ghSingleton != NULL)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				CloseHandle(ghSingleton);
				return 0;
			}
		}	
	}

	// create log folder
	CreateDirectory(LOG_FOLDER, NULL);

	////////////////////////////////////////
	//	SETTING FOR SYSTEM DUMP

	RunErrorReporting();


	////////////////////////////////////////
	//	CREATE EVENT
	{
		// create event
		for (int i = 0; i < 10; i++)
		{
			ghEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
			if (ghEvent != NULL)
				break;
		}

		// check event
		if (ghEvent == NULL)
		{
			// 리소스 부족 로그 남기고 종료 함. (사양)
			gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to CreateEvent. exit WatchDog.\n"));
			WriteLog();
			return 0;
		}
	}

	////////////////////////////////////////
	//	MONITORING EXCEPTION WINDOW
	{
		// check exception
		CheckException();

		// Write Log
		//WriteLog(); // [#2059] NH KJW 2011.05.13

		// [#2059] NH KJW 2011.05.12 하기 로직 미사용 삭제
		//// every 1 min.
		//if (ghEvent	!= NULL)
		//	WaitForSingleObject(ghEvent, REBOOT_WAIT_TIME);
		//else
		//	Sleep(REBOOT_WAIT_TIME);
		// end of [#2059]
	}

	// System reboot
	SystemReboot();

	// Release Resource
	if (ghEvent != NULL)
		CloseHandle(ghEvent);

	if (ghSingleton != NULL)
		CloseHandle(ghSingleton);

	return 0;
}

void RunErrorReporting()
{
	bool	bCopyed = false;

	for (int i = 0; i < 5; i++)
	{
		if (CopyFile(_T("\\ATM\\DwXfer.dll"), _T("\\Windows\\DwXfer.dll"), FALSE) == TRUE)
		{
			bCopyed = true;
			break;
		}
	}

	if (bCopyed == false)
	{
		// 실패시 로그 남김
		gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to Copy(). DwXfer.dll. err(%d)\n"), GetLastError());
		WriteLog();
		gnUseExceptionMsg = 0;

		return; // [#2059] NH KJW 2011.05.12 드라이버 파일 복사실패 시 아래 로직은 의미 없으므로 return 처리함.
	}

	// Registry Setting (DumpSettings)
	RegSetInt(L"System\\ErrorReporting\\DumpSettings", L"DumpEnabled", 0x1);
	RegSetInt(L"System\\ErrorReporting\\DumpSettings", L"MaxLogFiles", 0x2);
	RegSetInt(L"System\\ErrorReporting\\DumpSettings", L"MaxDiskUsage", 0xA0000);
	RegSetStr(L"System\\ErrorReporting\\DumpSettings", L"DumpDirectory", L"\\Windows\\DumpFiles");
	RegSetStr(L"System\\ErrorReporting\\DumpSettings", L"ExtraFilesDirectory", L"\\Windows\\ExtraDumpFiles");

	// Registry Setting (UploadSettings)
	RegSetInt(L"System\\ErrorReporting\\UploadSettings", L"NoConsentRequired", 0);
	RegSetInt(L"System\\ErrorReporting\\UploadSettings", L"DontUpload", 1);
	RegSetInt(L"System\\ErrorReporting\\UploadSettings", L"MaxWeeklyReports", 0);
	RegSetInt(L"System\\ErrorReporting\\UploadSettings", L"MaxDailyReports", 0);
	RegSetInt(L"System\\ErrorReporting\\UploadSettings", L"UploadSucceededDlg", 0);
	RegSetInt(L"System\\ErrorReporting\\UploadSettings", L"UploadFailedDlg", 0);

	// Registry Setting (ErrorReporting Driver)
	RegSetStr(L"Drivers\\BuiltIn\\ErrorReporting", L"Dll", L"DwXfer.dll");
	RegSetStr(L"Drivers\\BuiltIn\\ErrorReporting", L"FriendlyName", L"Dump File Transfer");
	RegSetInt(L"Drivers\\BuiltIn\\ErrorReporting", L"PollInterval", 0x000927c0);
	RegSetInt(L"Drivers\\BuiltIn\\ErrorReporting", L"PollPriority256", 0x000000f9);
	RegSetInt(L"Drivers\\BuiltIn\\ErrorReporting", L"Index", 0x00000001);
	RegSetInt(L"Drivers\\BuiltIn\\ErrorReporting", L"Order", 0x00000000);
	RegSetInt(L"Drivers\\BuiltIn\\ErrorReporting", L"Flags", 0x00000002);

	HANDLE hDriver = INVALID_HANDLE_VALUE;

	hDriver = ActivateDeviceEx( _T("Drivers\\Builtin\\ErrorReporting"), NULL, 0, NULL);

	if (hDriver == INVALID_HANDLE_VALUE || hDriver == 0)
	{
		// 실패시 로그 남김
		gnUseExceptionMsg = _stprintf(gszExceptionMsg, _T("Failed to ActivateDeviceEx(). err(%d)\n"), GetLastError());
		WriteLog();
		gnUseExceptionMsg = 0;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: CheckException
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Exception이 발생 했는지 check
-------------------------------------------------------------------*/
void CheckException()
{
	WCHAR	szWndTitle[MAX_PATH];
	HWND	hTopWnd = NULL;
	int		nCount = 0;

	while(1)
	{
		// check window title
		hTopWnd = GetWindow(GetDesktopWindow(), GW_HWNDFIRST);
		if (hTopWnd != NULL)
		{
			do
			{
				if (GetWindowText(hTopWnd, szWndTitle, MAX_PATH) > 0)
				{
					if (IsNameMatch(szWndTitle, gszExceptionName) == true)
					{
						HWND hChildWnd = GetWindow(hTopWnd, GW_CHILD);

						// Get Windows Text
						while (hChildWnd != NULL)
						{
							nCount = GetWindowText(hChildWnd, (TCHAR*)&gszExceptionMsg[gnUseExceptionMsg], (MAX_MSG_BUFF-gnUseExceptionMsg));
							if (nCount > 0)
								gnUseExceptionMsg += nCount;

							hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);
						}

						// [#2059] NH KJW 2011.05.13
						gszExceptionMsg[gnUseExceptionMsg] = 0;
						WriteLog();
						gnUseExceptionMsg = 0;
						// end of [#2059]

						//SendMessage(hTopWnd, WM_CLOSE, 0, 0); // [#2059] NH KJW 2011.05.12 SendMessage에서 Hang가능성 있으므로, 필수가 아닌 로직은 삭제함.

						// Copy dump files
						{
							DWORD	dwDumpIndex = 0;

							// Get Next Dump Index
							dwDumpIndex = GetNextDumpIndex();

							// Remove Dump File
							RemoveDumpFile(dwDumpIndex);

							// Copy Dump File
							CopyDumpFile(dwDumpIndex);

							// Set Last Dump Index
							SetLastDumpIndex(dwDumpIndex);

							gnUseExceptionMsg = _stprintf(&gszExceptionMsg[gnUseExceptionMsg], _T("After copying dump. DumpIndex(%d)\n"), dwDumpIndex);
							gszExceptionMsg[gnUseExceptionMsg] = 0;
							WriteLog();
							gnUseExceptionMsg = 0;

							if (ghEvent	!= NULL)
								WaitForSingleObject(ghEvent, 30000);	// KSK 2011.06.13 NH2700T일 경우 File Write가 안되는 현상 수정을 위해 File Write후 30초 Delay 적용
							
						}

						//gszExceptionMsg[gnUseExceptionMsg] = 0; // [#2059] NH KJW 2011.05.13
						return;
					}
					else if (IsNameMatch(szWndTitle, gszNetworkExceptionName) == true)
					{
						// [#2156] NH KSK 2012.11.08
						// DHCP 관련 Message 발생
						HWND hChildWnd = GetWindow(hTopWnd, GW_CHILD);

						// Get Windows Text
						while (hChildWnd != NULL)
						{
							nCount = GetWindowText(hChildWnd, (TCHAR*)&gszExceptionMsg[gnUseExceptionMsg], (MAX_MSG_BUFF-gnUseExceptionMsg));
							if (nCount > 0)
								gnUseExceptionMsg += nCount;

							hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);
						}

						gszExceptionMsg[gnUseExceptionMsg] = 0;
						WriteLog();
						gnUseExceptionMsg = 0;

						// POP UP MESSAGE CLOSE
						SendMessage(hTopWnd, WM_CLOSE, 0, 0);
						// end of [#2156]
					}
				}

				hTopWnd = GetWindow(hTopWnd, GW_HWNDNEXT);
				
			} while (hTopWnd != NULL);
		}

		// every 1sec.
		if (ghEvent	!= NULL)
			WaitForSingleObject(ghEvent, EXCEPTION_CHECK_TIME);
		else
			Sleep(EXCEPTION_CHECK_TIME);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: IsNameMatch
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : pszTitle에 pszWant 문자열이 있는지 검사 (대문자로 변환 후)
-------------------------------------------------------------------*/
bool IsNameMatch(TCHAR *pszTitle, TCHAR *pszWant)
{
	int		i = 0;
	int		nCount = 0;
	TCHAR	szUpperTitle[MAX_PATH+1]={0,};
	TCHAR	szUpperWant[MAX_PATH+1]={0,};

	if ((pszTitle == NULL) || (pszWant == NULL))
		return false;

	// make upper case 
	nCount = _tcslen(pszTitle);
	for(i = 0; i < nCount; i++)
		szUpperTitle[i] = (TCHAR)toupper((TCHAR)pszTitle[i]);

	nCount = _tcslen(pszWant);
	for(i = 0; i < nCount; i++)
		szUpperWant[i] = (TCHAR)toupper((TCHAR)pszWant[i]);
	
	if (_tcsstr(szUpperTitle, szUpperWant) == NULL)
		return false;

	return true;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: WriteLog
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Log를 기록한다.
-------------------------------------------------------------------*/
void WriteLog()
{
	FILE				*fileHandle;
	char				szErrorMsg[MAX_MSG_BUFF];
	int					nUseErrorMsg = 0;
	long				offset = 0;

	SYSTEMTIME	ti;
	GetLocalTime(&ti);

	// timestamp
	nUseErrorMsg += sprintf_s(szErrorMsg, MAX_MSG_BUFF, "[%04d/%02d/%02d %02d:%02d:%02d] ", ti.wYear, ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond);

	// convert ascii to unicode
	if (WideCharToMultiByte(CP_ACP, 0, gszExceptionMsg, gnUseExceptionMsg, &szErrorMsg[nUseErrorMsg], (MAX_MSG_BUFF-nUseErrorMsg), NULL, NULL) == 0)
		return;

	// update size
	nUseErrorMsg += gnUseExceptionMsg;

	// insert return
	szErrorMsg[nUseErrorMsg] = '\n';
	nUseErrorMsg++;

	// check folder
	CreateDirectory(LOG_FOLDER, NULL);

	// Get File Size
	offset = 0;
	fileHandle = _tfopen(LOG_FILE_NAME, _T("rb"));
	if (fileHandle != NULL)
	{
		fseek(fileHandle, 0, SEEK_SET);
		fread(&offset, sizeof(long), 1, fileHandle);
		fclose(fileHandle);
	}

	// file Open
	fileHandle = _tfopen(LOG_FILE_NAME, _T("r+b"));
	if (fileHandle == NULL)
		fileHandle = _tfopen(LOG_FILE_NAME, _T("w+b"));

	if (fileHandle != NULL)
	{
		if (offset == 0)
		{
			offset = 4;
			fseek(fileHandle, 0, SEEK_SET);
			fwrite(&offset, sizeof(long), 1, fileHandle);
		}

		if ((offset + nUseErrorMsg) > MAX_LOGGING_SIZE)
			offset = 4;

		// move last
		fseek(fileHandle, offset, SEEK_SET);

		// write log
		offset += fwrite(szErrorMsg, 1, nUseErrorMsg, fileHandle);

		// write offset
		{
			fseek(fileHandle, 0, SEEK_SET);
			fwrite(&offset, sizeof(long), 1, fileHandle);
		}

		// file Close
		fflush(fileHandle);
		fclose(fileHandle);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: SystemReboot
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 시스템을 Reboot 함
-------------------------------------------------------------------*/
void SystemReboot()
{
	#define IOCTL_SYSTEM_RESET	1
	#define IOCTL_MODEM_RESET	2

	HANDLE hReset = CreateFile(L"RST1:", GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
	if (hReset != INVALID_HANDLE_VALUE)
	{
		DWORD dwBuffer = IOCTL_SYSTEM_RESET;
		DWORD dwWritten;
		WriteFile(hReset, &dwBuffer, 1, &dwWritten, NULL);
		CloseHandle(hReset);
	}
}