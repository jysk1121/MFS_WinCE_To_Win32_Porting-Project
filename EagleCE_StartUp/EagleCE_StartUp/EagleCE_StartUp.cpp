// EagleCE_StartUp.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "EagleCE_StartUp.h"
#include <Tlhelp32.h>

#ifdef _WIN32_WCE

#define	EXE_EAGLECE_ATM_PATH		_T("\\NAND\\EAGLECE_ATM\\EagleCE_ATM.exe")

#if (MAINBOARD_TYPE)
	#define	EXE_SOFTWARE_UPDATE_PATH	_T("\\USB\\SoftwarePackage\\SoftwareUpdate.exe")
#else
	#define	EXE_SOFTWARE_UPDATE_PATH	_T("\\Hard Disk\\SoftwarePackage\\SoftwareUpdate.exe")
#endif

#else	// Win32 desktop

// The CE paths point at flash/USB volumes that do not exist on a desktop PC.
// Use bare file names so CreateProcess resolves them next to this executable.
#define	EXE_EAGLECE_ATM_PATH		_T("EagleCE_ATM.exe")
#define	EXE_SOFTWARE_UPDATE_PATH	_T("SoftwareUpdate.exe")

#endif	// _WIN32_WCE

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

BOOL				SearchByName(const char *szToTerminate);
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

	TCHAR		tszClassName[] = TEXT("__MFS_START_UP_WND__");

	RETAILMSG(1, (_T("Start Up is started\n")));

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

	// Local Mode Font Create.
	{
		LOGFONT	lf;

		memset(&lf, 0, sizeof(lf));

		wcscpy(lf.lfFaceName, _T("Arial"));
		lf.lfHeight = GET_FONTSIZE(30);
		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfWeight = FW_BOLD;

		g_hLocalFont = ::CreateFontIndirect(&lf);
	}

	g_DrawRect.left = 0;
	g_DrawRect.top = 0;
	g_DrawRect.right = GetSystemMetrics(SM_CXSCREEN);
	g_DrawRect.bottom = GetSystemMetrics(SM_CYSCREEN);

	ghEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	// create Client Wnd
	g_hWndDisplay = ::CreateWindowEx(NULL,								/* dwExStyle */
		tszClassName,						/* lpClassName */
		NULL,								/* lpWindowName */
		WS_POPUP,							/* dwStyle */
		0,									/* x */
		0,									/* y */
		GetSystemMetrics(SM_CXSCREEN),		/* nWidth */
		GetSystemMetrics(SM_CYSCREEN),		/* nHeight */
		NULL,								/* hWndParent */
		NULL,								/* hMenu */
		hInstance,							/* hInstance */
		NULL);								/* lpParam */

	// Hide Window - don't initially visible
	::ShowWindow(g_hWndDisplay, SW_HIDE);
	::UpdateWindow(g_hWndDisplay);

	::SetCursor(NULL);

	::SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);


	// execute explore.exe
	// NOTE: WinCE only. On CE this starts the shell that the ATM UI runs on top of.
	// On a desktop PC the shell is already running, and "\Windows\Explorer.exe"
	// would resolve to the real C:\Windows\Explorer.exe and spawn an extra shell
	// window, so the whole block is compiled out for Win32.
#if !(MAINBOARD_TYPE) && defined(_WIN32_WCE)
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = _T("\\Windows\\Explorer.exe");
	ShExecInfo.lpParameters = NULL;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
#endif

	::SetTimer(g_hWndDisplay, 1000, 1000, NULL);

	while (GetMessage(&msg, g_hWndDisplay, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (g_hLocalFont)
		DeleteObject(g_hLocalFont);

	if (ghEvent)
		CloseHandle(ghEvent);

	RETAILMSG(1, (_T("Start Up is ended\n")));

	return 0;
}


BOOL SearchByName(const char *szToTerminate)
{
	HANDLE         hProcessSnap = INVALID_HANDLE_VALUE;
	BOOL           bRet      = FALSE; 
	PROCESSENTRY32 pe32      = {0}; 

	int iLenP,indx;
	char szToTermUpper[128];
	char szTemp[128];

	iLenP=strlen(szToTerminate);

	for(indx=0;indx<iLenP;indx++)	szToTermUpper[indx]=toupper(szToTerminate[indx]);
	szToTermUpper[iLenP]=0;

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
			hProcess = ::OpenProcess (PROCESS_ALL_ACCESS, 
				FALSE, pe32.th32ProcessID); 

			memset(szTemp, NULL, sizeof(szTemp));
			WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, -1, szTemp, sizeof(szTemp), NULL, NULL);

			pCharFound=NULL;
			pCharFound = strstr(_strupr(szTemp), szToTermUpper);

			CString strTemp;
			strTemp.Format(_T("Process Name : (%s) (%S) (%x)\n"), pe32.szExeFile, szToTermUpper, pCharFound);
			RETAILMSG(1,(strTemp));

			if(pCharFound != NULL)
			{
				if (hProcess != NULL)
				{
					bRet = TRUE;

					RETAILMSG(1, (_T("Found EagleCE_ATM process\n")));
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
	} 

#ifdef _WIN32_WCE
	::CloseToolhelp32Snapshot(hProcessSnap);
#else
	::CloseHandle(hProcessSnap);
#endif

	return bRet;

}


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

			// Execute EagleCE_ATM.exe
			{
				STARTUPINFO si;
				ZeroMemory (&si, sizeof(si));
				si.cb = sizeof (si);
				si.wShowWindow = SW_HIDE;
				PROCESS_INFORMATION pi;
				ZeroMemory (&pi, sizeof(pi));

				CreateProcess(EXE_EAGLECE_ATM_PATH, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
			}

			WaitForSingleObject(ghEvent, 3000);	// 3초 Dealy

			BOOL bResult = SearchByName("EagleCE_ATM");

			if (bResult == FALSE)
			{
				// 아닌 경우 USB에 있는 Software Update 실행
				// Execute EagleCE_ATM.exe
				{
					STARTUPINFO si;
					ZeroMemory (&si, sizeof(si));
					si.cb = sizeof (si);
					si.wShowWindow = SW_HIDE;
					PROCESS_INFORMATION pi;
					ZeroMemory (&pi, sizeof(pi));

					CreateProcess(EXE_SOFTWARE_UPDATE_PATH, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
				}

				WaitForSingleObject(ghEvent, 3000);	// 3초 Dealy

				bResult = SearchByName("SoftwareUpdate");

				if (bResult == TRUE)
					RETAILMSG(1, (_T("Execute success Softwareupdate")));
				else
					RETAILMSG(1, (_T("Execute failed Softwareupdate")));

			}

			RETAILMSG(1,(_T("WM_TIMER end\n")));

			// 프로그램 종료
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
