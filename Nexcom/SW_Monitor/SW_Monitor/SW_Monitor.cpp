// SW_Monitor.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SW_Monitor.h"

#include <atlstr.h>


#define MAX_LOADSTRING 100

#define WM_USER_POLL_EVT		(WM_USER+0x3476)		// Poll Event 
#define APP_MONITOR_TIMOUT		60000					// 1 min

// 전역 변수:
HINSTANCE			g_hInst;			// 현재 인스턴스입니다.
HWND				g_hWndCommandBar;	// 명령 모음 핸들입니다.


HWND				g_hWndDisplay;			// The command bar handle
HANDLE				ghEvent=NULL;			// never signaled.
HANDLE				g_hThread = NULL;
RECT				g_DrawRect;

DWORD				g_dwStart = 0;				// 기준 시간
BOOL				g_bEageCE_ATM_Alive = TRUE;	// 초기에는 무조건 살아있다고 간주함.

LRESULT CALLBACK	DisplayWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

void				SYSTEM_HW_Reboot();
DWORD				Check_PollingTime();


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	WNDCLASS	wc;
	MSG			msg;

	TCHAR		tszClassName[] = TEXT("__MFS_SW_MONITOR_WND__");

	RETAILMSG(1, (_T("Softrware Monitor is started\n")));

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

	::SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	g_hThread = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Check_PollingTime, NULL, NULL, NULL);

	if (g_hThread == NULL)
		return 0;

	while (GetMessage(&msg, g_hWndDisplay, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
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

	case WM_DESTROY:
		{
			RETAILMSG(1, (_T("[SW_Monitor] RECEIVED DESTROY EVT\n")));

			::SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);


			g_bEageCE_ATM_Alive = FALSE;
			SetEvent(ghEvent);

			// Thread가 종료될때까지 대기 후 종료 처리
			DWORD dwExitCode = STILL_ACTIVE;
			int nCount = 0;

			if (g_hThread != NULL)
			{
				while(dwExitCode == STILL_ACTIVE)
				{
					::GetExitCodeThread(g_hThread, &dwExitCode);

					if (nCount > 500)	// 5초 Timeout
						break;

					nCount++;
					::Sleep(100);
				}

				::CloseHandle(g_hThread);
				g_hThread = NULL;
			}

			RETAILMSG(1, (_T("[SW_Monitor] RECEIVED DESTROY END\n")));

			::PostQuitMessage(0);
			return 0;
		}

	case WM_COPYDATA:
		{
			// Time값 갱신
			g_dwStart = GetTickCount();		// Timeout 시간 갱신

			return 0;
		}
		
	default:	break;
    }

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

void SYSTEM_HW_Reboot()
{
	DCB PortDCB;
	COMMTIMEOUTS CommTimeouts;
	HANDLE hPort;
	LPCTSTR pcszComPort= _T("COM5:");
	char buff[7] = { 0, };
	DWORD dwNumBytesWritten;


	hPort = CreateFile (pcszComPort, GENERIC_READ|GENERIC_WRITE, 0,  NULL,  OPEN_EXISTING, 0, NULL);
	if ( hPort == INVALID_HANDLE_VALUE )
		return;

	PortDCB.DCBlength = sizeof (DCB);    
	GetCommState (hPort, &PortDCB);
	PortDCB.BaudRate = 115200;              // Current baud
	PortDCB.fBinary = TRUE;               // Binary mode; no EOF check
	PortDCB.fParity = TRUE;               // Enable parity checking
	PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control
	PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control
	PortDCB.fDtrControl = FALSE;

	// DTR flow control type
	PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity
	PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx
	PortDCB.fOutX = FALSE;                // No XON/XOFF out flow control
	PortDCB.fInX = FALSE;                 // No XON/XOFF in flow control
	PortDCB.fErrorChar = FALSE;           // Disable error replacement
	PortDCB.fNull = FALSE;                // Disable null stripping
	PortDCB.fRtsControl = FALSE;

	// RTS flow control
	PortDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on
	// error
	PortDCB.ByteSize = 8;                 // Number of bits/byte, 4-8
	PortDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space
	PortDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2

	SetCommState (hPort, &PortDCB);

	GetCommTimeouts (hPort, &CommTimeouts);
	CommTimeouts.ReadIntervalTimeout = MAXDWORD; 
	CommTimeouts.ReadTotalTimeoutMultiplier = 0; 
	CommTimeouts.ReadTotalTimeoutConstant = 0;   
	CommTimeouts.WriteTotalTimeoutMultiplier = 10; 
	CommTimeouts.WriteTotalTimeoutConstant = 1000;   

	SetCommTimeouts (hPort, &CommTimeouts);

	EscapeCommFunction (hPort, SETDTR);
	EscapeCommFunction (hPort, SETRTS);

	buff[0]=1;
	buff[1]=2;
	buff[2]=1;
	buff[3]=5;
	buff[4]=2;
	buff[5]=0;
	buff[6]=0xb;

	WriteFile(hPort, &buff, sizeof(buff), &dwNumBytesWritten, NULL) || dwNumBytesWritten!=sizeof(buff);
}

DWORD Check_PollingTime()
{
	DWORD	dwElapsed = 0;
	int nCount = 40;	// 최초 1번 출력을 위해 기본값 설정

	g_dwStart = GetTickCount();	// Start 시간 갱신

	while(g_bEageCE_ATM_Alive)
	{
		// 1분동안 
		dwElapsed = GetTickCount() - g_dwStart;

		if (dwElapsed > APP_MONITOR_TIMOUT)
		{
			// AP가 Holding으로 간주하여 System 재부팅 수행
			RETAILMSG(1, (_T("[SW_Monitor] Execute Hardware Reboot - Application Freezing\n")));

			// File을 생성하도록 로직 보완
			CString strPath, strLog;
			strPath.Format(_T("\\Trace\\SW_Monitor.log"));

			// open file
			HANDLE hFile = NULL;
			BYTE byLogData[1024] = { 0, };
			DWORD dwWriten = 0;

			hFile = CreateFile (strPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				// File 생성 후 File Write
				strLog = _T("[SW_Monitor] Execute Hardware Reboot - Application Freezing");

				WideCharToMultiByte(CP_ACP, 0, strLog, -1, (char*)byLogData, strLog.GetLength(), NULL, NULL);

				WriteFile(hFile, byLogData, strLog.GetLength(), &dwWriten, NULL);

				CloseHandle(hFile);
				
				WaitForSingleObject(ghEvent, 3000);	// File Write할 시간 Delay (3초)
			}

			SYSTEM_HW_Reboot();

			RETAILMSG(1, (_T("[SW_Monitor] Software Monitor is ended\n")));
		}

		// 10분마다 Debug Message 출력
		nCount++;

		if (nCount > 40)
		{
			nCount = 0;
			RETAILMSG(1, (_T("[SW_Monitor] Software Monitor is alive\n")));
		}

		WaitForSingleObject(ghEvent, 15000);	// 15초마다 종료 여부 확인
	}

	return 0;
}