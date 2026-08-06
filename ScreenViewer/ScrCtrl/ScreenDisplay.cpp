#include "stdafx.h"
#include "ScreenDisplay.h"

///////////////////////////////////////////////////////////////////
//	Global Variable Define
///////////////////////////////////////////////////////////////////

// Control of Threads
HANDLE		g_hDisplayLifeEvent = NULL;
HANDLE		g_hDisplayWndThread = NULL;		// Display Thread Handle (Main Wnd)
HANDLE		g_hInteractThread	= NULL;		// Interact Thread Handle
BOOL		g_bInteractDoing	= FALSE;	// Interact Thread 동작 Flag

// Control of System
CNHCritSec	g_csNHInput;					// AP의 요청에 대한 동기화 (AP->SCREEN)
CNHCritSec	g_csNHOutput;					// AP로 결과 전송에 대한 동기화 (SCREEN->AP)
CNHCritSec	g_csNHScreen;					// AP로 결과 전송에 대한 동기화 (SCREEN->AP)
HINSTANCE	g_hInstance		 = 0;			// Instance Handle.
HWND		g_hWndDisplay	 = NULL;		// Clinet Wnd Handle
CString		g_strLoadSection = _T("");		// Master File에서 로드할 Section
CString		g_strMasterPath  = _T("");		// Master File Path
SIZE		g_sizeScreen	 = {0,0};		// System의 Resolution (Width, Height)
SIZE		g_BaseSizeScreen = {0,0};		// 기준 Screen Resolution (Width, Height)
BOOL        g_bInitialize	 = FALSE;		// if true, SCREEN이 초기화가 되었다.
int			g_nLogPixelSY	 = 0;			// Number of pixel per logical inch along the screen height 

// Control of drawing screen pointer
CNScreen	*g_pDisplayScreen = NULL;		// 현재 화면에 그려지는 Screen pointer	(화면 <-> 광고)
CNScreen	*g_pCurrentScreen = NULL;		// AP 요청에 의해 현재 그려진 Screen	(APCenterMovie)
CNScreen	*g_pNextScreen	  = NULL;		// AP 요청에 의해 다음에 그려질 Screen	(PrevSetScrrenNo)

// Control of draw background
BOOL		g_bBackRefresh	= FALSE;		// if true, 배경을 Refresh 한다. (새로 그림을 그림)
BOOL		g_bBackDrawed	= FALSE;		// if true, 배경이 Refresh 될때 배경이 실제로 그려졌다.
HBITMAP		g_bmBackBuffer	= NULL;			// 배경이 그려질 Bitmap Handle
CString		g_strBackID		= "";			// 현재 Bitmap에 그려진 배경 ID

// Control of draw screen
int			g_nCurrentLocale = NH_ENGLISH;	// Selected Screen Language
int			g_nMaxLocale	 = 0;			// Maximum Support Screen Language
BOOL		g_bLocalMode	 = FALSE;		// if true, Online 모드가 아니다. (AP에서 설정함)
HBITMAP		g_hbmScreen		 = NULL;		// 화면이 그려질 Bitmap Handle
HFONT		g_hLocalFont	 = NULL;		// Online 모드가 아닐때 뿌려줄 Font.
POINT		g_DebugPoint	 = {0,0};		// 화면에 디버깅 메시지를 표시할 위치
CString		g_DebugMsg		 = _T("");		// 화면에 출력할 디버깅 메시지

// Control of Advertisement
ADVERTISE_INFO	g_AdvertiseInfo = {0,0};

// V1.0.2.4 2018.07.19 - Camera의 좌표를 Script의 Pos 값으로 설정하도록 하기 위함.
// Control info of TextBox for Camera
RECT g_rect4Camera = {0, 0, 0, 0};
CString g_strScreenID;
BOOL g_bPosValue4Camera = FALSE;
/////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//	Global Screen and Resource Containers
///////////////////////////////////////////////////////////////////

// 화면 데이터
CNHPtrLinkedList< CNResource >	g_NHResources;
CNHPtrLinkedList< CNScreen >	g_NHScreens;

///////////////////////////////////////////////////////////////////
//	Global Communication Queues
///////////////////////////////////////////////////////////////////

// AP <-> SCR 통신 Queue
CNHQueue<CScrMsg>	g_FSCmdQueue;
CNHQueue<CScrMsg>	g_VariableQueue;


///////////////////////////////////////////////////////////////////
//	Local Function Prototype define.
///////////////////////////////////////////////////////////////////

DWORD	CROSS_WINAPI DisplayWndThread(LPVOID lpParameter);
LRESULT CALLBACK DisplayWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL	DisplayInitialize(CString strLoadSection, CString strMasterPath);
BOOL	DisplayDeinitialize(void);

int GetNextAdvScreen();


///////////////////////////////////////////////////////////////////
//	Export Interface Implement
///////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: CreateDisplayWnd()
 RETURN TYPE  : TRUE : Initialize success
				FLASE : Initialize failed
 PARAMETER    : hInstance : instance handle
				hWndParent : 부모가 있다면 부모 Window의 handle
 DESCRIPTION  : 화면을 표시 할 Window를 만든다. 화면 데이터를 Load 한다.
-------------------------------------------------------------------*/
BOOL	CreateDisplayWnd(HINSTANCE hInstance, CString strLoadSection, CString strMasterPath)
{
	DWORD	nResult;

	NHDEBUG(1, (L"[CScrCtrl-CreateDisplayWnd]\n"));

	g_hInstance = hInstance;
	strLoadSection.MakeUpper();
	g_strLoadSection = strLoadSection;
	g_strMasterPath = strMasterPath;

	// Create Event
	if (!g_hDisplayLifeEvent)
	{
		g_hDisplayLifeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!g_hDisplayLifeEvent)
		{
			NHERROR(1, (_T("[CScrCtrl-CreateDisplayWnd] Filed CreateEvent for hDisplayLifeEvent..\n")));
			goto ErrorReturn;
		}
	}

	// Create Display Wnd.
	g_hDisplayWndThread = CreateThread(NULL, NULL, DisplayWndThread, NULL, NULL, NULL);
	if (!g_hDisplayWndThread)
	{
		NHERROR(1, (_T("[CScrCtrl-CreateDisplayWnd] Failed CreateThread for DisplayWnd..\n")));
		goto ErrorReturn;
	}

	// Wait for Initialize
	nResult = WaitForSingleObject(g_hDisplayLifeEvent, INFINITE);
	if (WAIT_OBJECT_0 != nResult)
	{
		NHERROR(1, (_T("[CScrCtrl-CreateDisplayWnd] Failed Wait for Display Wnd Initialize..\n")));
		goto ErrorReturn;
	}

	// failed create.
	if (!g_hWndDisplay)
	{
		NHERROR(1, (_T("[CScrCtrl-CreateDisplayWnd] Failed Display internal..\n")));
		goto ErrorReturn;
	}

	if (!g_bInitialize)
	{
		NHERROR(1, (_T("[CScrCtrl-CreateDisplayWnd]  Failed Initialize..\n")));
		goto ErrorReturn;
	}

	NHDEBUG(1, (L"[CScrCtrl-CreateDisplayWnd] <-- OUT, OK OK\n"));
	
	return TRUE;

ErrorReturn:

	NHDEBUG(1, (L"[CScrCtrl-CreateDisplayWnd] <-- OUT, NG--\n"));

	return FALSE;
}


/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DestroyDisplayWnd()
 RETURN TYPE  : TRUE : DeInitialize success
 PARAMETER    : 
 DESCRIPTION  : 화면 Window를 종료한다.
-------------------------------------------------------------------*/
BOOL	DestroyDisplayWnd()
{
	DWORD dwExitCode = STILL_ACTIVE;

	NHDEBUG(1, (L"[CScrCtrl-DestroyDisplayWnd] --> IN"));

	// Destroy Interact Thread
	if (g_hInteractThread)
	{
		// end
		g_bInteractDoing = FALSE;

		dwExitCode = STILL_ACTIVE;
		while(dwExitCode == STILL_ACTIVE)
		{
			Delay_Msg(50);
			GetExitCodeThread(g_hInteractThread, &dwExitCode);
		};

		CloseHandle(g_hInteractThread);
		g_hInteractThread = NULL;
	}

	// Destroy Dispaly Wnd
	if (g_hDisplayWndThread)
	{
		if (g_hWndDisplay)
			PostMessage(g_hWndDisplay, WM_DESTROY, 0, 0);

		dwExitCode = STILL_ACTIVE;
		while(dwExitCode == STILL_ACTIVE)
		{
			Delay_Msg(50);
			GetExitCodeThread(g_hDisplayWndThread, &dwExitCode);
		};

		CloseHandle(g_hDisplayWndThread);
		g_hDisplayWndThread = NULL;
	}

	// close DisplayLifeEvent
	if (g_hDisplayLifeEvent)
	{
		CloseHandle(g_hDisplayLifeEvent);
		g_hDisplayLifeEvent = NULL;
	}

	NHDEBUG(1, (L"[CScrCtrl-DestroyDisplayWnd] <-- OUT, OK\n"));

	return TRUE;
}



///////////////////////////////////////////////////////////////////
//	Display Wnd Impl.
///////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndThread()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Thread Proc.
-------------------------------------------------------------------*/
DWORD CROSS_WINAPI DisplayWndThread(LPVOID lpParameter)
{
	WNDCLASS	wc;
	MSG			msg;
    TCHAR		tszClassName[] = TEXT("__NH_DISPLAY_WND__");
	DWORD		dwStyle;
	CString		strWndName;
	int			nCaptionHeight = 0, nBorderWidth =0, nBorderHeight = 0;
	int			nWndWidth = 0, nWndHeight = 0;

// #ifndef UNDER_CE
// 	nCaptionHeight = GetSystemMetrics(SM_CYCAPTION);
// 	nBorderWidth = GetSystemMetrics(SM_CXBORDER);
// 	nBorderHeight = GetSystemMetrics(SM_CYBORDER);
// #endif

	// Registry ClassInfo
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC) DisplayWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = (HINSTANCE) g_hInstance;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = tszClassName;

    RegisterClass(&wc);

	// Get Screen Size
	if (g_sizeScreen.cx == 0 || g_sizeScreen.cy == 0)
	{
		//g_sizeScreen.cx = GetSystemMetrics(SM_CXSCREEN);
		//g_sizeScreen.cy = GetSystemMetrics(SM_CYSCREEN);

		g_sizeScreen.cx = 1280;
		g_sizeScreen.cy = 800;

		dwStyle = WS_POPUP;
		
		nWndWidth = g_sizeScreen.cx;
		nWndHeight = g_sizeScreen.cy;
 	}
 	else
 	{
// 		strWndName.Format(_T("%d X %d Screen"), g_sizeScreen.cx, g_sizeScreen.cy);
 //		dwStyle = WS_POPUP | WS_CAPTION | WS_VISIBLE;
 //		nWndWidth = g_sizeScreen.cx + 6;
 //		nWndHeight = g_sizeScreen.cy + nCaptionHeight + 6;

		dwStyle = WS_POPUP;	

		nWndWidth = g_sizeScreen.cx;
		nWndHeight = g_sizeScreen.cy;

 	}

	// create Client Wnd
	g_hWndDisplay = ::CreateWindowEx(NULL,						/* dwExStyle */
									tszClassName,				/* lpClassName */
									strWndName,					/* lpWindowName */
									dwStyle,					/* dwStyle */
									0,							/* x */
									0,							/* y */
									0,							/* nWidth */
									0,							/* nHeight */
									NULL,						/* hWndParent */
									NULL,						/* hMenu */
									g_hInstance,				/* hInstance */
									NULL);						/* lpParam */

	
	if (!g_hWndDisplay)
	{
		NHDEBUG(1, (_T("Failed CreateWindowEx for DisplayWnd..\n")));
		SetEvent(g_hDisplayLifeEvent);
		goto ErrorReturn;
	}

	// Create Display Wnd.
	g_hInteractThread = CreateThread(NULL, NULL, DisplayInteractThread, NULL, NULL, NULL);
	if (!g_hInteractThread)
	{
		NHERROR(1, (_T("[CScrCtrl-CreateDisplayWnd] Failed CreateThread for Interact..\n")));
		SetEvent(g_hDisplayLifeEvent);
		goto ErrorReturn;
	}

	{
		HDC hDC = GetDC(g_hWndDisplay);
		g_nLogPixelSY = GetDeviceCaps(hDC, LOGPIXELSY);
		ReleaseDC(g_hWndDisplay, hDC);
	}

		// Initialize
	if (!DisplayInitialize(g_strLoadSection, g_strMasterPath))
	{
		NHDEBUG(1, (_T("Failed Initialize..\n")));
		//goto ErrorReturn;
	}

	// Finished Initialize. so. alter to waiting thread..
	SetEvent(g_hDisplayLifeEvent);

	// Hide Window - don't initially visible
	ShowWindow(g_hWndDisplay, SW_SHOW);
	UpdateWindow(g_hWndDisplay);

	// set Initial or default value.
#ifdef UNDER_CE
		SetCursor(NULL);
		SetWindowPos(g_hWndDisplay, HWND_TOPMOST, 0, 0, nWndWidth, nWndHeight, SWP_SHOWWINDOW);
		//SetWindowPos(g_hWndDisplay, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
#else
//		SetWindowPos(g_hWndDisplay, HWND_NOTOPMOST, 0, 0, nWndWidth, nWndHeight, SWP_SHOWWINDOW);
		MoveWindow(g_hWndDisplay, 0, 0, nWndWidth, nWndHeight, TRUE);

#endif
		
	SetTimer(g_hWndDisplay, UM_BLINKING_TIME, BLINKING_TIME, NULL);
	
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Deinitialize
	DisplayDeinitialize();

	return msg.wParam;

ErrorReturn:
	DisplayDeinitialize();

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
    PAINTSTRUCT ps;
	RECT	rcScreenRect = {0, 0, g_sizeScreen.cx, g_sizeScreen.cy};
	RECT	rcUpdate;

    switch(wMsg)
    {
	case WM_PAINT:
		{
			CNHAutoLock	AutoSync(g_csNHScreen);

			//  초기화가 되지 않은 경우 그리지 않음
			if (g_bInitialize == FALSE)
				return 0;

			HDC hDC = BeginPaint(hWnd, &ps);
			if (hDC == NULL)
				return 0;

			rcUpdate = ps.rcPaint;

			// Create bitmap buffer for background.
			if (g_bmBackBuffer == FALSE)
				g_bmBackBuffer = CreateCompatibleBitmap(hDC, g_sizeScreen.cx, g_sizeScreen.cy);
				
			// Create bitmap buffer for screen.
			if (g_hbmScreen == FALSE)
				g_hbmScreen = CreateCompatibleBitmap(hDC, g_sizeScreen.cx, g_sizeScreen.cy);

			HDC hBufDC = CreateCompatibleDC(hDC);
			if (hBufDC != NULL)
			{
				BOOL	bDrawed = FALSE;
				HGDIOBJ hOldScr = SelectObject(hBufDC, g_hbmScreen);

				// 화면을 그린다
				if (g_pDisplayScreen != NULL)
				{
					////////////////////////////////////
					// Draw Background Image on screen buffer.
					HDC hBgrDC = CreateCompatibleDC(hDC);
					if (hBgrDC != NULL)
					{
						HGDIOBJ hOldBgr = SelectObject(hBgrDC, g_bmBackBuffer);

						// 배경 Refresh and 이전에 그려놓은 것과 다른 배경일때
						if ((g_bBackRefresh == TRUE) && (g_strBackID != g_pDisplayScreen->GetID()))
						{
							g_bBackDrawed = g_pDisplayScreen->DrawBackground(hBgrDC, g_nCurrentLocale, &rcUpdate);

							NHDEBUG(1, (_T("BACKGROUND DRAWD(%d)\n"), g_bBackDrawed));

							g_bBackRefresh = FALSE;
							g_strBackID = g_pDisplayScreen->GetID();
						}

						// Draw on Screen.
						if (g_bBackDrawed == TRUE)
						{
							BitBlt(	hBufDC,	rcUpdate.left, rcUpdate.top, (rcUpdate.right - rcUpdate.left), (rcUpdate.bottom - rcUpdate.top),	
									hBgrDC, rcUpdate.left, rcUpdate.top, 
									SRCCOPY);
							bDrawed |= TRUE;
						}

						SelectObject(hBgrDC, hOldBgr);
						DeleteObject(hBgrDC);
					}

					////////////////////////////////////
					// Draw Screen Image on screen buffer
					if (g_pDisplayScreen->Draw(hBufDC, g_nCurrentLocale, &rcUpdate) == TRUE)
						bDrawed |= TRUE;

					////////////////////////////////////
					// Draw "DEMO VERSION" on screen buffer
					if (g_bLocalMode)
					{
						CString		strScrNum = _T("");
						int			OldBkMode;
						HFONT		OldFont = NULL;
//						COLORREF	OldTextColor;
						COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
//						RECT		rcDemo = {5, 0, g_sizeScreen.cx, 50};
						RECT		rcDemo = {5, 0, g_sizeScreen.cx, 72};
						CString		strDemoInfo, strDemoTemp;
						
						// Select to Transparent Mode in Background Mode
						OldBkMode = ::SetBkMode(hBufDC, TRANSPARENT);
						
						// Select Font
						if (g_hLocalFont)
							OldFont = (HFONT)::SelectObject(hBufDC, g_hLocalFont);
						
						// Set Text Color
						OldTextColor = ::SetTextColor(hBufDC, RGB(255,0,0));
						
						if (g_pCurrentScreen)
							strScrNum = g_pCurrentScreen->GetID();
						
						strDemoInfo.Format(_T("DEMO"));

						// Draw..
						DrawText(hBufDC, (LPCTSTR)strDemoInfo, -1, &rcDemo, DT_CENTER | DT_SINGLELINE | DT_VCENTER);	
						
						// Restore Text Color
						::SetTextColor(hBufDC, OldTextColor);
						
						// Restore Font
						if (OldFont)
							::SelectObject(hBufDC, OldFont);
						
						// Restore Background Mode
						::SetBkMode(hBufDC, OldBkMode);
					}

					////////////////////////////////////
					// Debug Message on screen buffer
					if (g_DebugMsg.GetLength() != 0)
					{
						int			OldBkMode;
//						COLORREF	OldTextColor;
						COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책

						// Select to Transparent Mode in Background Mode
						OldBkMode = ::SetBkMode(hBufDC, TRANSPARENT);
						
						// Set Text Color
						OldTextColor = ::SetTextColor(hBufDC, RGB(255,0,0));
						
						// Draw..
						ExtTextOut(hBufDC, g_DebugPoint.x, g_DebugPoint.y, 0, NULL, g_DebugMsg, g_DebugMsg.GetLength(), NULL);
						
						// Restore Text Color
						::SetTextColor(hBufDC, OldTextColor);
						
						// Restore Background Mode
						::SetBkMode(hBufDC, OldBkMode);
					}
				}
#ifndef UNDER_CE
				// 초기 로딩화면을 그린다.
				else if (g_pDisplayScreen == NULL || g_pCurrentScreen == NULL)
				{
					int			nScr = 0;
					int			OldBkMode;
					HFONT		OldFont = NULL;
//					COLORREF	OldTextColor;
					COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
					RECT		rcDemo;

					CString		strTextInfo = _T("Temporarily\nout of service");
					
					// Select to Transparent Mode in Background Mode
					OldBkMode = ::SetBkMode(hBufDC, TRANSPARENT);
					
					// Select Font
					if (g_hLocalFont != NULL)
						OldFont = (HFONT)::SelectObject(hBufDC, g_hLocalFont);
					
					// Set Text Color
					OldTextColor = ::SetTextColor(hBufDC, RGB(255,255,255));
					
					GetClientRect(g_hWndDisplay, &rcDemo);

					FillRect(hBufDC, &rcScreenRect, (HBRUSH) GetStockObject(BLACK_BRUSH));

					// Calc Rect for draw Text
					{
						RECT rcTemp, rcDraw;
						int  calcHeight, orgHeight;
						
						rcDraw = rcDemo;
						DrawText(hBufDC, strTextInfo, -1, &rcTemp, DT_CALCRECT);
						
						calcHeight = rcTemp.bottom - rcTemp.top;
						orgHeight = rcDraw.bottom - rcDraw.top;
						
						if (calcHeight < orgHeight)
						{
							rcDraw.top += (int)((orgHeight - calcHeight) / 2);
							rcDraw.bottom = rcDraw.top + calcHeight;
						}

						// Draw..
						DrawText(hBufDC, (LPCTSTR)strTextInfo, -1, &rcDraw, DT_CENTER);
					}
					
					// Restore Text Color
					::SetTextColor(hBufDC, OldTextColor);
					
					// Restore Font
					if (OldFont)
						::SelectObject(hBufDC, OldFont);
					
					// Restore Background Mode
					::SetBkMode(hBufDC, OldBkMode);

					bDrawed = TRUE;
				}
#endif // !UNDER_CE

				////////////////////////////////////
				// Draw scree from buffer to Device
				if (bDrawed == TRUE)
				{
					// Draw Screen
					BitBlt(	hDC,	rcUpdate.left, rcUpdate.top, (rcUpdate.right - rcUpdate.left), (rcUpdate.bottom - rcUpdate.top),	
							hBufDC, rcUpdate.left, rcUpdate.top, 
							SRCCOPY);

					NHDEBUG(1, (_T("***SCR*** Current Screen Locale(%3d) Update(%3d, %3d, %3d, %3d)\n"),
										g_nCurrentLocale, rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom));
				}

				SelectObject(hBufDC, hOldScr);
				DeleteDC(hBufDC);
			}

			EndPaint(hWnd, &ps);

			return 0;
		}

	case WM_ERASEBKGND:
		{
			g_bBackRefresh = TRUE;
			return 0;
		}

	case WM_TIMER:
		{
			int	nTimerID = wParam;
			
			if (nTimerID != UM_BLINKING_TIME)
				KillTimer(g_hWndDisplay, nTimerID);

			// 완전히 로딩된 후에.
			if (g_pCurrentScreen == NULL || g_pNextScreen != NULL)
				return 0;

			// Blinking 
			if (nTimerID == UM_BLINKING_TIME)
			{
				BOOL bSetVariable = TRUE;

				if (g_pDisplayScreen == g_pCurrentScreen)
					DisplaySetVariable(DES_BLINKING, _T(""));
			}
// Cross Compile
#ifdef UNDER_CE
			// Advertisement.
			else if (nTimerID == UM_ADV_SHOW_TIME)
			{
				if (g_AdvertiseInfo.bEnable == FALSE)
					return 0;

				///////////////////////////////////////
				// 광고 or 대기화면 Draw
				if (g_AdvertiseInfo.nShowCount > 0)
				{
					BOOL	bAdvShowTime = FALSE;

					// [#642] NH KSK 2010.06.30 Memory 문제로 인해 AP -> AD -> AD -> AP 로직을 AP -> AD -> AP로 사양 변경
//					// AP -> AD -> AP
//					if (g_AdvertiseInfo.nEnableCount == 1 && (g_AdvertiseInfo.nShowCount % 2) == 0)
//						bAdvShowTime = TRUE;
//					// AP -> AD -> AD -> AP
//					else if (g_AdvertiseInfo.nEnableCount >= 2 && (g_AdvertiseInfo.nShowCount % 3) != 1)
//						bAdvShowTime = TRUE;
					if (g_AdvertiseInfo.nEnableCount > 0 && (g_AdvertiseInfo.nShowCount % 2) == 0)
						bAdvShowTime = TRUE;

					CNHAutoLock	AutoSync(g_csNHScreen);

					if (bAdvShowTime == TRUE)
					{
						g_pDisplayScreen = g_AdvertiseInfo.pCurAdvScreen;

						NHDEBUG(1, (_T("ADVERTISEMENT SHOW TIME\n")));
					}
					else
					{
						g_pDisplayScreen = g_pCurrentScreen;
						NHDEBUG(1, (_T("APPLICATION SHOW TIME\n")));
					}
					// end of [#642]

					// Draw
					::InvalidateRect(g_hWndDisplay, NULL, TRUE);
				}
				
				SetTimer(g_hWndDisplay, UM_ADV_DECODE_TIME, DECODE_INTERVAL, NULL);
			}
			else if (nTimerID == UM_ADV_DECODE_TIME)
			{
				int			nShowInterval = 0;
				DWORD		nBefore = 0, nDiff = 0;

				if (g_AdvertiseInfo.bEnable == FALSE)
					return 0;

				nBefore = GetTickCount();

				// 처음 시작은 광고 부터이다.
				if (g_AdvertiseInfo.nShowCount == 0)
					g_AdvertiseInfo.nShowCount = 2;
				else
					g_AdvertiseInfo.nShowCount++;

				// [#642] NH KSK 2010.06.30 ADV_SetNextScreen()으로 위치 변경
				// 현재 보여진 화면이 광고이면 메모리 해제한다.
//				if (g_pDisplayScreen != g_pCurrentScreen)
//				{
//					// 광고가 한개라면 여러번 하지 않는다.
//					if (g_AdvertiseInfo.pCurAdvScreen != NULL && g_AdvertiseInfo.nEnableCount >= 2)
//						g_AdvertiseInfo.pCurAdvScreen->Deinitialize();
//				}
				// end of [#642]

				///////////////////////////////////////
				// 다음이 화면이 광고일 경우 decoding을 시작

				// [#642] NH KSK 2010.06.30 사양 변경 광고 2개 이상인 경우 AP -> AD -> AD -> AP 이었으나, Memory문제로 인해 AP -> AD -> AP -> AD로 변경
//				if ((g_AdvertiseInfo.nEnableCount == 1 && (g_AdvertiseInfo.nShowCount % 2) == 0) ||
//					(g_AdvertiseInfo.nEnableCount >= 2 && (g_AdvertiseInfo.nShowCount % 3) != 1))
				if ((g_AdvertiseInfo.nEnableCount > 0) && (g_AdvertiseInfo.nShowCount % 2) == 0)
				{
					ADV_SetNextScreen();

					if (g_AdvertiseInfo.pCurAdvScreen != NULL)
					{
						CRequest		Req;
						CString			strName, strData;

						g_AdvertiseInfo.pCurAdvScreen->Initialize();

						// HIDE ADVERTISEMENT BASIC IMAGE
						strName.Format(_T("%s1"), DES_AP_STATE);
						Req.Set(REQ_SETVALUE, strName, _T("off"));
						g_AdvertiseInfo.pCurAdvScreen->AddRequest(Req);

						// HIDE ADVERTISEMENT BASIC IMAGE
						strName.Format(_T("%s2"), DES_AP_STATE);
						Req.Set(REQ_SETVALUE, strName, _T("off"));
						g_AdvertiseInfo.pCurAdvScreen->AddRequest(Req);

						g_AdvertiseInfo.pCurAdvScreen->ProcessRequest();
					}
				}

				nDiff = GetTickCount() - nBefore;
				if (nDiff < 0)
					nDiff  = 0;

				nShowInterval = g_AdvertiseInfo.nRefreshTime * 1000;
				nShowInterval -= (nDiff + DECODE_INTERVAL);

				SetTimer(g_hWndDisplay, UM_ADV_SHOW_TIME, nShowInterval, NULL);
			}
#endif //UNDER_CE
			// occur time out.
			else if (_ttoi(g_pCurrentScreen->GetID()) == (nTimerID - UM_TIMER_ID_BASE))
			{
				DisplayKeyboardEvent(DES_TIMEOVER, NULL);
			}

			return 0;
		}

#ifdef UNDER_CE
	case WM_RBUTTONUP:
		SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		return 0;
#else
	case WM_SETFOCUS:
		::InvalidateRect(g_hWndDisplay, NULL, TRUE);
		return 0;
#endif

	case WM_LBUTTONUP:	//*LEH
		{
			CPoint tempPos, pos;
			::GetCursorPos(&tempPos);
			pos.x = tempPos.x;
			pos.y = tempPos.y;

			if (!g_FSCmdQueue.IsHasData(0))
				g_VariableQueue.Enqueue(CScrMsg(_T("MouseKey"), pos));
			return 0;
		}

	case WM_KEYUP:
		{
			int nVerKey = (int)wParam;
			
			if ((nVerKey == VK_RETURN || nVerKey == VK_BACK || nVerKey == VK_DELETE ||
				nVerKey == VK_LEFT || nVerKey == VK_RIGHT || nVerKey == VK_CAPITAL || nVerKey == VK_TAB) ||	//*LEH
				(nVerKey >= VK_F1 && nVerKey <= VK_F8) ||
				(nVerKey >= '0' && nVerKey <= '9') ||
				(nVerKey >= VK_NUMPAD0 && nVerKey <= VK_NUMPAD9))
			{
				CString strKey;
				
				if (nVerKey == VK_RETURN)
					strKey = _T("ENTER");
				else if (nVerKey == VK_BACK)
					strKey = _T("CANCEL");
				else if (nVerKey == VK_DELETE)
					strKey = _T("CLEAR");
				else if (nVerKey == VK_LEFT)
					strKey = DES_STAR;			//_T("A");
				else if (nVerKey == VK_RIGHT)
					strKey = DES_SHARP;			//_T("B");
				else if (nVerKey == VK_CAPITAL)
					strKey = DES_CAPS;			//_T("C");
//				else if (nVerKey >= VK_F1 && nVerKey <= VK_F8)
//				{
//					nVerKey = (nVerKey - VK_F1) + KEYMAP_FDK_L1;
//					strKey.Format(_T("%c"), nVerKey);
//				}
				else if (nVerKey >= VK_F1 && nVerKey <= VK_F4)
				{
					nVerKey -= VK_F1;
					strKey.Format(_T("L%d"), nVerKey + 1);
				}
				else if (nVerKey >= VK_F5 && nVerKey <= VK_F8)
				{
					nVerKey -= VK_F5;
					strKey.Format(_T("R%d"), nVerKey + 1);
				}
				else if (nVerKey >= '0' && nVerKey <= '9')
					strKey.Format(_T("%c"), nVerKey);
				else
				{
					nVerKey -= VK_NUMPAD0;
					strKey.Format(_T("%d"), nVerKey);
				}
				
				if (!g_FSCmdQueue.IsHasData(0))
					g_VariableQueue.Enqueue(CScrMsg(_T("APKey"), strKey));
				
			}
			return 0;
		}
		
	case WM_DESTROY:
		{
#ifdef UNDER_CE
			SetWindowPos(g_hWndDisplay, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
#endif
			if (g_bmBackBuffer)
			{
				DeleteObject(g_bmBackBuffer);
				g_bmBackBuffer = NULL;
			}

			if (g_hbmScreen)
			{
				DeleteObject(g_hbmScreen);
				g_hbmScreen = NULL;
			}


			g_pCurrentScreen = NULL;

			PostQuitMessage(0);
			return 0;
		}
	default:	break;
    }
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayInitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Display Wnd를 위한 화면 Data, 초기 작업 수행
-------------------------------------------------------------------*/
BOOL	DisplayInitialize(CString strLoadSection, CString strMasterPath)
{
	g_bInitialize = FALSE;
	g_pDisplayScreen = NULL;
	g_pCurrentScreen = NULL;
	g_pNextScreen = NULL;
	g_bBackDrawed = FALSE;
	g_bBackRefresh = FALSE;

#ifdef UNDER_CE
	// COM initialize.
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

	// Initialize CriticalSection
//	InitializeCriticalSection(&g_csNHInput);
//	InitializeCriticalSection(&g_csNHOutput);

	// Make Scrren data.
	if (!NHScreenMake(strLoadSection, strMasterPath))
		return FALSE;

	// Local Mode Font Create.
	{
		LOGFONT	lf;

		memset(&lf, 0, sizeof(lf));
		
		// Face Name
		_tcscpy(lf.lfFaceName, _T("Arial"));
		// SIZE
//		lf.lfHeight = GET_FONTSIZE(60);
		lf.lfHeight = GET_FONTSIZE(40);
		// BOLD
		lf.lfWeight = FW_BOLD;
		
		g_hLocalFont = ::CreateFontIndirect(&lf);
	}

	g_bInitialize = TRUE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayDeinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Display Wnd를 위한 화면 Data, 종료 작업 수행
-------------------------------------------------------------------*/
BOOL	DisplayDeinitialize(void)
{
	if (g_hWndDisplay)
		DestroyWindow(g_hWndDisplay);

	if (g_hLocalFont)
		DeleteObject(g_hLocalFont);

	CNScreen *pScreen = NULL;
	while(g_NHScreens.GetFromHeadOfList(&pScreen, 0))
	{
		if (pScreen == NULL)
			continue;

		delete pScreen;
		pScreen = NULL;
	}

	CNResource *pRes = NULL;
	while(g_NHResources.GetFromHeadOfList(&pRes, 0))
	{
		if (pRes == NULL)
			continue;

		if (pRes->GetTpye() == RES_PEN)
		{
			CNPen *pPen = (CNPen*)pRes;
			delete pPen;
		}
		else if (pRes->GetTpye() == RES_FONT)
		{
			CNFont *pFont = (CNFont*)pRes;
			delete pFont;
		}
		else if (pRes->GetTpye() == RES_BRUSH)
		{
			CNBrush *pBrush = (CNBrush*)pRes;
			delete pBrush;
		}
		else if (pRes->GetTpye() == RES_PICTURE)
		{
			CNPicture *pPicture = (CNPicture*)pRes;
			delete pPicture;
		}
		else if (pRes->GetTpye() == RES_LOCALETEXT)
		{
			CNLocaleText *pLocalText = (CNLocaleText*)pRes;
			delete pLocalText;
		}
		else
			delete pRes;

		pRes = NULL;
	}

	// Deinitialize CriticalSection
//	DeleteCriticalSection(&g_csNHInput);
//	DeleteCriticalSection(&g_csNHOutput);

	g_sizeScreen.cx = 0;
	g_sizeScreen.cy = 0;

	g_BaseSizeScreen.cx = 0;
	g_BaseSizeScreen.cy = 0;

	g_bInitialize = FALSE;

	g_pDisplayScreen = NULL;
	g_pCurrentScreen = NULL;
	g_pNextScreen = NULL;

	g_bBackDrawed = FALSE;
	g_bBackRefresh = FALSE;

#ifdef UNDER_CE
	// COM Deinitialize.
	CoUninitialize();
#endif

	return TRUE;
}

CString	GetCurScreenNumber()
{
	if (g_pCurrentScreen)
		return g_pCurrentScreen->GetID();

	return _T("");
}