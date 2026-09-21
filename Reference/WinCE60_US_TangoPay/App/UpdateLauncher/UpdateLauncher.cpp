// UpdateLauncher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "UpdateLauncher.h"
#include <commctrl.h>
#include ".\Common\NHConfig.h"

#define MAX_LOADSTRING 100
#define MulDiv(a,b,c)       (((a)*(b))/(c))
#define GET_FONTSIZE(PointSize) (-MulDiv((PointSize), GetDeviceCaps(NULL, LOGPIXELSY), 72))

// Global Variables:
HWND				g_hWndDisplay;			// The command bar handle
HFONT				g_hLocalFont = NULL;
HANDLE				ghEvent=NULL;		// never signaled.

CNHConfig			g_SystemConfig;
RECT				g_DrawRect;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK	DisplayWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{

	WNDCLASS	wc;
	MSG			msg;
	
    TCHAR		tszClassName[] = TEXT("__NH_UPDATE_LAUNCHER_WND__");

	// Registry ClassInfo
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC) DisplayWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = (HINSTANCE) hInstance;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = tszClassName;

    RegisterClass(&wc);

	// Local Mode Font Create.
	{
		LOGFONT	lf;

		memset(&lf, 0, sizeof(lf));

		wcscpy_s(lf.lfFaceName, sizeof(lf.lfFaceName), _T("Arial"));
		lf.lfHeight = GET_FONTSIZE(20);
		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfWeight = FW_BOLD;
		
		g_hLocalFont = ::CreateFontIndirect(&lf);
	}

	NH_SCR_CONFIG eNearConfig = g_SystemConfig.GetScreenConfig(SCR_REAR);

	g_DrawRect.left = 0;
	g_DrawRect.top = 0;
	g_DrawRect.right = eNearConfig.nWidth;
	g_DrawRect.bottom = eNearConfig.nHeight;

	ghEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	// create Client Wnd
	g_hWndDisplay = ::CreateWindowEx(NULL,				/* dwExStyle */
							tszClassName,				/* lpClassName */
							NULL,						/* lpWindowName */
							WS_POPUP,					/* dwStyle */
							eNearConfig.nX,				/* x */
							eNearConfig.nY,				/* y */
							eNearConfig.nWidth,			/* nWidth */
							eNearConfig.nHeight,		/* nHeight */
							NULL,						/* hWndParent */
							NULL,						/* hMenu */
							hInstance,					/* hInstance */
							NULL);						/* lpParam */
	

	// Hide Window - don't initially visible
	ShowWindow(g_hWndDisplay, SW_SHOW);
	UpdateWindow(g_hWndDisplay);

	SetCursor(NULL);

	SetWindowPos(g_hWndDisplay, HWND_TOPMOST, eNearConfig.nX, eNearConfig.nY, eNearConfig.nWidth, eNearConfig.nHeight, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

	SetTimer(g_hWndDisplay, 1000, 1000, NULL);

	while (GetMessage(&msg, g_hWndDisplay, 0, 0)) 
	{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	if (g_hLocalFont) 
		DeleteObject(g_hLocalFont);
	
	if (ghEvent)
		CloseHandle(ghEvent);

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
	HDC hdc;
    PAINTSTRUCT ps;

    switch(wMsg)
    {
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			
			int			nScr = 0;
			int			OldBkMode;
			HFONT		OldFont = NULL;
//			COLORREF	OldTextColor;
			COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
			CString		strInfoMsg;
			
			// Select to Transparent Mode in Background Mode
			OldBkMode = ::SetBkMode(hdc, TRANSPARENT);
			
			// Select Font
			if (g_hLocalFont)
				OldFont = (HFONT)::SelectObject(hdc, g_hLocalFont);
			
			// Set Text Color
			OldTextColor = ::SetTextColor(hdc, RGB(255,255,255));
			
			strInfoMsg = _T("Please wait while processing...");
			
			// Draw..
			DrawText(hdc, (LPCTSTR)strInfoMsg, -1, &g_DrawRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			
			
			// Restore Text Color
			::SetTextColor(hdc, OldTextColor);
			
			// Restore Font
			if (OldFont)
				::SelectObject(hdc, OldFont);
			
			// Restore Background Mode
			::SetBkMode(hdc, OldBkMode);
			
			EndPaint(hWnd, &ps);
			
			return 0;
		}

	case WM_TIMER:
		{
			KillTimer(g_hWndDisplay, wParam);

			// Copy Update.exe to root
			{
				// Copy Files...
				CopyFile(L"\\ATM\\libcrypto-1_1.dll",	L"\\libcrypto-1_1.dll",	FALSE);
				CopyFile(L"\\ATM\\libssl-1_1.dll",		L"\\libssl-1_1.dll",	FALSE);
				CopyFile(L"\\ATM\\libcurl.dll",			L"\\libcurl.dll",	FALSE);				
				CopyFile(L"\\ATM\\MB2500DLL.dll",		L"\\MB2500DLL.dll",	FALSE);
				CopyFile(L"\\ATM\\CmnLib.dll",			L"\\CmnLib.dll",	FALSE);
				CopyFile(L"\\ATM\\ScrCtrl.dll",			L"\\ScrCtrl.dll",	FALSE);
				CopyFile(L"\\ATM\\NetCtrl.dll",			L"\\NetCtrl.dll",	FALSE);
				CopyFile(L"\\ATM\\LibUpdate.dll",		L"\\LibUpdate.dll",	FALSE);
				CopyFile(L"\\ATM\\HSCdmDrv.dll",		L"\\HSCdmDrv.dll",	FALSE);
				CopyFile(L"\\ATM\\DevCrypto.dll",		L"\\DevCrypto.dll",	FALSE);
				CopyFile(L"\\ATM\\MFC90U.DLL",			L"\\MFC90U.DLL",	FALSE);
				CopyFile(L"\\ATM\\MFC90ENU.DLL",		L"\\MFC90ENU.DLL",	FALSE);
				CopyFile(L"\\ATM\\msvcr90.dll",			L"\\msvcr90.dll",	FALSE);
				CopyFile(L"\\ATM\\Update.exe",			L"\\Update.exe",	FALSE);

				WaitForSingleObject(ghEvent, 50);
			}

			// wait for WinAtm exit.
			{
				while(1)
				{
					WaitForSingleObject(ghEvent, 500);

					if ((FindWindow(NULL, L"WinATM") != NULL) ||
						(FindWindow(NULL, L"NH_SIUManager") != NULL) ||
						(FindWindow(NULL, L"MyPinPadManager") != NULL) ||
						(FindWindow(NULL, L"NW_KSPRManager") != NULL) ||
						(FindWindow(NULL, L"NHDIP4WCEManager") != NULL) ||
						(FindWindow(NULL, L"NHIDC4MTKWCEManager") != NULL) ||	// [#11] NH KSK 2010.09.29
						(FindWindow(NULL, L"NHDIP4SANDIPCEManager") != NULL) ||	// [#2073] NH KSK 2011.07.12
						(FindWindow(NULL, L"NHIDC4RFCEManager") != NULL) ||		// [#2325] NH KSK 2015.01.25
						(FindWindow(NULL, L"NHCDMManager") != NULL) ||
						(FindWindow(NULL, L"NH_SIU") != NULL) ||
						(FindWindow(NULL, L"MyPinPad") != NULL) ||
						(FindWindow(NULL, L"NW_KSPR") != NULL) ||
						(FindWindow(NULL, L"NHDIP4WCE") != NULL) ||
						(FindWindow(NULL, L"NHIDC4MTKWCE") != NULL) ||
						(FindWindow(NULL, L"NHDIP4SANDIPCE") != NULL) ||		// [#2073] NH KSK 2011.07.12
						(FindWindow(NULL, L"NHIDC4RFCE") != NULL) ||			// [#2325] NH KSK 2015.01.25
						(FindWindow(NULL, L"NHCDM") != NULL) ||
						(FindWindow(NULL, L"WinATMManager") != NULL))						
						continue;
					else					
						break;					
				}

				// remove unnecessary 5-seconds-waiting.
// 				WaitForSingleObject(ghEvent, 5000);
			}

			// Execute Update.exe
			{
				STARTUPINFO si;
				ZeroMemory (&si, sizeof(si));
				si.cb = sizeof (si);
				si.wShowWindow = SW_HIDE;
				PROCESS_INFORMATION pi;
				ZeroMemory (&pi, sizeof(pi)); // [#2027] NH KJW 2011.03.03
				
				CreateProcess(L"\\Update.exe",NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
			}

			WaitForSingleObject(ghEvent, 3000);
			
			PostMessage(g_hWndDisplay, WM_DESTROY, NULL, NULL);
		}

	case WM_DESTROY:
		{
			SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

			PostQuitMessage(0);
			return 0;
		}
		
	default:	break;
    }

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}