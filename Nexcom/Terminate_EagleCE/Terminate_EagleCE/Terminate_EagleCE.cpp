// Terminate_EagleCE.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Terminate_EagleCE.h"
#include <Tlhelp32.h>
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

#define		EXE_SOFTWARE_UPDATE_PATH			_T("\\USB\\SoftwarePackage\\SoftwareUpdate.exe")
#define		EXE_SOFTWARE_UPDATE_FROM_AMS_PATH	_T("\\SoftwarePackage\\SoftwareUpdate.exe")

#define MAX_LOADSTRING 100

#define MulDiv(a,b,c)       (((a)*(b))/(c))
#define GET_FONTSIZE(PointSize) (-MulDiv((PointSize), GetDeviceCaps(NULL, LOGPIXELSY), 72))

HWND				g_hWndDisplay;			// The command bar handle
HFONT				g_hLocalFont = NULL;
HANDLE				ghEvent=NULL;		// never signaled.
RECT				g_DrawRect;


// 전역 변수:
HINSTANCE			g_hInst;			// 현재 인스턴스입니다.
HWND				g_hWndCommandBar;	// 명령 모음 핸들입니다.
BOOL				g_bFindProcess = FALSE;

//HWND				g_hEagleCE_ATM;

int CloseProcessByName(const char *szToTerminate, BOOL bTerminate = TRUE);


//BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam);

LRESULT CALLBACK	DisplayWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

#ifdef _WIN32_WCE
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
#else
// Desktop <winbase.h> declares WinMain with LPSTR; using LPTSTR (LPWSTR under
// UNICODE) would be a conflicting overload.
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nCmdShow)
#endif
{
	WNDCLASS	wc;
	MSG			msg;

	TCHAR		tszClassName[] = TEXT("__MFS_TERMINATE_WND__");

	RETAILMSG(1, (_T("Terminate Process is started\n")));

	// Registry ClassInfo
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC) DisplayWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE) hInstance;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = tszClassName;

	RegisterClass(&wc);

	g_DrawRect.left = 0;
	g_DrawRect.top = 0;
	g_DrawRect.right = 0;
	g_DrawRect.bottom = 0;

	ghEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	// create Client Wnd
	g_hWndDisplay = ::CreateWindowEx(NULL,								/* dwExStyle */
									tszClassName,						/* lpClassName */
									NULL,								/* lpWindowName */
									0,							/* dwStyle */
									0,									/* x */
									0,									/* y */
									0,									/* nWidth */
									0,									/* nHeight */
									NULL,								/* hWndParent */
									NULL,								/* hMenu */
									hInstance,							/* hInstance */
									NULL);								/* lpParam */

	// Hide Window - don't initially visible
	::ShowWindow(g_hWndDisplay, SW_HIDE);

	::SetCursor(NULL);

	::SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	CString strUpdateType;
	strUpdateType = CUtil::GetSWUpdateType();

	CString strTemp;
	strTemp.Format(_T("Terminate Process => SWUpdate type : [%s]\n"), strUpdateType);
	RETAILMSG(1,(strTemp));

	// Execute SoftwareUpdate.exe
	{
		STARTUPINFO si;
		ZeroMemory (&si, sizeof(si));
		si.cb = sizeof (si);
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		ZeroMemory (&pi, sizeof(pi));

		if(_T("2") == strUpdateType)	// AMS 원격에 의한 Update라면 "\\SoftwarePackage" 있는 파일 실행
		{
			CreateProcess(EXE_SOFTWARE_UPDATE_FROM_AMS_PATH, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
		}
		else
		{
			CreateProcess(EXE_SOFTWARE_UPDATE_PATH, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
		}
	}

	// Softwareupdate.exe 기동 후 1초 후에 EagleCE ATM 종료 처리
	::SetTimer(g_hWndDisplay, 1000, 1000, NULL);

	while (GetMessage(&msg, g_hWndDisplay, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (ghEvent)
		CloseHandle(ghEvent);

	RETAILMSG(1, (_T("Terminate Process is ended\n")));

	return 0;
}


int CloseProcessByName(const char *szToTerminate, BOOL bTerminate)
{
	HANDLE         hProcessSnap = INVALID_HANDLE_VALUE;
	BOOL           bRet      = 0; 
	PROCESSENTRY32 pe32      = {0}; 

	int iLenP,indx;
	char szToTermUpper[128] = { 0, };
	char szTemp[128] = { 0, };
	BOOL bResult = FALSE;

	iLenP=strlen(szToTerminate);

	for(indx=0;indx<iLenP;indx++)
	{
		szToTermUpper[indx]=toupper(szToTerminate[indx]);
	}

	g_bFindProcess = FALSE;

	hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

	if (hProcessSnap == INVALID_HANDLE_VALUE)		return 0;

	pe32.dwSize = sizeof(PROCESSENTRY32); 

	if (::Process32First(hProcessSnap, &pe32)) 
	{ 
		BOOL         bGotModule = FALSE; 
		MODULEENTRY32	me32       = {0}; 
		char*			pCharFound;

		do 
		{ 
			HANDLE hProcess = NULL;

			// Get the actual priority class. 
			hProcess = ::OpenProcess (PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID); 

			memset(szTemp, NULL, sizeof(szTemp));
			WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, -1, szTemp, sizeof(szTemp), NULL, NULL);

			pCharFound=NULL;
			pCharFound = strstr(_strupr(szTemp), szToTermUpper);

			CString strTemp;
			strTemp.Format(_T("Process Name : (%s) (%S)\n"), pe32.szExeFile, szToTermUpper);
			RETAILMSG(1,(strTemp));
			
			if(pCharFound != NULL)
			{
				if (hProcess != NULL)
				{
					g_bFindProcess = TRUE;

					RETAILMSG(1, (_T("Found EagleCE_ATM or SW_Monitor process\n")));

					if (bTerminate == TRUE)
					{

						bResult = ::TerminateProcess(hProcess, 0);

						if (bResult == FALSE)
						{
							RETAILMSG(1, (_T("TerminateProcess is failed - try again -- \n")));

							// Retry TerminateProcess
							WaitForSingleObject(ghEvent, 1000);

							bResult = ::TerminateProcess(hProcess, 0);

							if (bResult == FALSE)
							{
								RETAILMSG(1, (_T("[Retry] TerminateProcess is failed - error \n")));
							}
							else
							{
								RETAILMSG(1, (_T("[Retry] TerminateProcess is successful \n")));
							}
						}

						// Terminate 후에 1초 대기
						WaitForSingleObject(ghEvent, 1000);

						RETAILMSG(1, (_T("Terminate EagleCE_ATM or SW_Monitor process\n")));
					}

					::CloseHandle (hProcess);
					break;
				}
			}
			else
			{
				if (hProcess != NULL)
					CloseHandle(hProcess);
			}

		} 
		while (::Process32Next(hProcessSnap, &pe32)); 

		bRet = 1; 
	} 
	else         bRet = 0;

#ifdef _WIN32_WCE
	::CloseToolhelp32Snapshot(hProcessSnap);
#else
	::CloseHandle(hProcessSnap);
#endif

	return bRet;

}

//BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam)
//{
//	CString strTemp;
//	TCHAR name[256] = { 0, };
//	TCHAR title[256] = { 0, };
//
//	if (hwnd == NULL)
//		return TRUE;
//
//	GetClassName( hwnd, name, 256 );
//	GetWindowText( hwnd, title, 256 );
//
//	strTemp.Format(_T("EnumWindowCallBack hwnd : [0x%x], ClassName : [%s], WindowText : [%s]\n"), hwnd, name, title);
//	RETAILMSG(1,(strTemp));
//
//	// SVC Manage를 찾는 경우 중지시킨다.
//	CString strFindClassName;
//
//	strFindClassName = name;
//	strFindClassName.MakeUpper();
//
//	if (strFindClassName == _T("DIALOG"))
//	{
//		RETAILMSG(1,(_T("Found Dialog Class Name\n")));
//
//		::PostMessage(hwnd, WM_DESTROY, 0, 0);
//
//		strTemp.Format(_T("Post Message to Dialog (0x%x)\n"), hwnd);
//		RETAILMSG(1,(strTemp));
//	}
//
//	return TRUE;
//}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
LRESULT CALLBACK DisplayWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch(wMsg)
    {
	case WM_PAINT:
		{		
			return 0;
		}

	case WM_TIMER:
		{
			RETAILMSG(1,(_T("WM_TIMER start\n")));

			::KillTimer(g_hWndDisplay, wParam);

			// Software Update 프로그램에서 NAND로 File Copy할 때까지 대기
			while(TRUE)
			{
				if (CUtil::Int_GetRegistry(UPDATE_PROC_REG_PATH, _T("FILE_PROC"), 0) == 1)
					break;

				WaitForSingleObject(ghEvent, 1000);
			}

			WaitForSingleObject(ghEvent, 5000);	// Device Thread 종료를 위해 5초 대기 후 진행

			// Find Window로 1차 종료 후 안될 경우 하기 2차로 하도록 로직 보완
			// EagleCE ATM은 강제 종료 처리
			//::EnumWindows(EnumWindowCallBack, NULL);

			HWND hWnd = NULL;
			CString strTemp;
			int nCount = 0;

			BOOL bExitEagleATM = FALSE;
			BOOL bExitSWMonitor = FALSE;

			// Close EagleCE ATM
			if (bExitEagleATM == FALSE)
			{
				CloseProcessByName("EagleCE_ATM");
			}

			hWnd = ::FindWindow(_T("__MFS_SW_MONITOR_WND__"), NULL);

			if (hWnd != NULL)
			{
				::PostMessage(hWnd, WM_DESTROY, 0, 0);

				// 5초간 종료될때까지 대기
				nCount = 0;

				while(TRUE)
				{
					if (nCount > 5)
						break;

					hWnd = ::FindWindow(_T("__MFS_SW_MONITOR_WND__"), NULL);

					if (hWnd == NULL)
					{
						bExitSWMonitor = TRUE;
						break;
					}

					nCount++;
					WaitForSingleObject(ghEvent, 1000);
				}

				hWnd = NULL;
			}
			else
			{
				bExitSWMonitor = TRUE;	// 실행되지 않았을 경우에도 강제종료하지 않도록 로직 수정
			}

			// Close SW_Monitor
			if (bExitSWMonitor == FALSE)
			{
				strTemp.Format(_T("Execute Forced SW_Monitor Kill Process\n"));
				RETAILMSG(1, (strTemp));

				WaitForSingleObject(ghEvent, 1000);
				CloseProcessByName("SW_Monitor");
			}

			// wait for EagleCE_ATM exit.
			//if (g_bFindProcess == TRUE)
			{
				// Process 종료 처리를 한 경우에만 처리
				while(TRUE)
				{
					// 프로세스가 종료될 때 까지 확인
					WaitForSingleObject(ghEvent, 1000);			// 1초마다 종료 여부 확인

					CloseProcessByName("EagleCE_ATM", FALSE);	// Check EagleCE_ATM Process

					if ((g_bFindProcess == FALSE) && (bExitSWMonitor == FALSE))
					{
						WaitForSingleObject(ghEvent, 1000);			// 1초마다 종료 여부 확인

						// EagleCE_ATM 종류 확인 후 SW_Monitor도 종료가 되었는지 한번 더 Check한다.
						CloseProcessByName("SW_Monitor", FALSE);	// Check SW_Monitor Process
					}

					if (g_bFindProcess == FALSE)
						break;
				}
			}

			RETAILMSG(1,(_T("WM_TIMER end\n")));
			::PostMessage(g_hWndDisplay, WM_DESTROY, NULL, NULL);
		}

	case WM_DESTROY:
		{
			RETAILMSG(1,(_T("WM_DISTORROY\n")));
			::SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

			::PostQuitMessage(0);
			return 0;
		}
		
	default:	break;
    }

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}