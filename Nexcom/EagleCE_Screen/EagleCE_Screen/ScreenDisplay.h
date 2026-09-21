#ifndef __SCREEN_DISPLAY_H__
#define __SCREEN_DISPLAY_H__

#include ".\SCR_Collection.h"
#include ".\SCR_Screen.h"
#include ".\ScreenDescType.h"
#include "../../Template/ReadiniFile.h"

#ifdef UNDER_CE
	#define CROSS_WINAPI
#else
	#include "NHCmnFuncForScrCtrl.h"
	#pragma comment(lib, "Msimg32") 
	
	#define Delay_Msg	Sleep
	#define Asc2Int				NH_Asc2Int
	#define GetCurrencySymbol	NH_GetCurrencySymbol
	#define SplitString			NH_SplitString

	#define CROSS_WINAPI WINAPI
	#define NHDEBUG(cond,printf_exp) ((void)0)
	#define NHDEMSG(cond,printf_exp) ((void)0)
	#define NHERROR(cond,printf_exp) ((void)0)	
	#ifndef RETAILMSG
	#define RETAILMSG(cond,printf_exp) ((void)0)
	#endif
#endif

///////////////////////////////////////////////////////////////////
// MSG ID
#define		UM_FSCOMMAND			(WM_USER + 1000)
#define		UM_BLINKING_TIME		(WM_USER + 10000)	// SCREEN TIME OUT
#define		UM_ADV_SHOW_TIME		(WM_USER + 10001)	// Advertise ment time
#define		UM_ADV_DECODE_TIME		(WM_USER + 10002)	// Advertise ment time
#define		BLINKING_TIME			(250)
#define		DECODE_INTERVAL			(500)
#define		UM_TIMER_ID_BASE		(110000000)

typedef struct _ADVERTISE_INFO_
{
	BOOL		bActiveFlag;						// 상위국에서 제어하기 위한 Active Flag 추가
	BOOL		bEnable;							// 전체 광고 Enable 여부
	int			nEnableCount;						// Enable된 광고 개수
	CNScreen	*pCurAdvScreen;						// 현재 광고 스크린 포인터, 만약 NULL이면 AP 화면이 뿌려진다.
	int			nCurAdvIndex;						// 현재 광고 Index
	int			nRefreshTime;						// 광고 표시 시간
	BOOL		bEnableAdv[SUPPORT_ADVERTISE+1];	// 각 광고 별 Enable 여부
	CNScreen	*pAdvScreen[SUPPORT_ADVERTISE+1];	// 각 광고 별 스크린 포인터
	int			nShowCount;							// 화면을 표시한 카운트.
} ADVERTISE_INFO;

///////////////////////////////////////////////////////////////////
//	Help Macro Define

#define MulDiv(a,b,c)       (((a)*(b))/(c))
#define GET_FONTSIZE(PointSize) (-MulDiv((PointSize), g_nLogPixelSY, 72))

///////////////////////////////////////////////////////////////////
//	Display Function
BOOL	CreateDisplayWnd(HINSTANCE hInstance, CString strLoadSection, CString strMasterPath);		// 화면을 표시 할 Window를 만든다.
BOOL	DestroyDisplayWnd();																		// 화면 Window를 종료한다.
CString	GetCurScreenNumber();


///////////////////////////////////////////////////////////////////
//	Screen Load & Make Function
BOOL	NHScreenMake(CString strLoadSection, CString strMasterPath);								// 화면 데이터를 Load 한다.

// Declared for both platforms: the definitions in SCR_Interact.cpp are not
// guarded, and Win32 call sites need these prototypes.
///////////////////////////////////////////
//	Advertisement Function
void ADV_SetNextScreen();
void ADV_Decode();		// V1.0.2.5 2018.08.06 - ADV 파일 decode
void ADV_Deinitialize();	// V1.0.2.5 2018.08.06 - ADV 파일 release

///////////////////////////////////////////
//	Interact Function (AP <-> SCR)
DWORD	CROSS_WINAPI DisplayInteractThread(LPVOID lpParameter);
void	DisplaySetVariable(CString strCommand, CString strData);
void	DisplayKeyboardEvent(CString strKeyCode, CNScreen *pCurScreen);
void	DisplayMouseEvent(POINT &pos, CNScreen *pCurScreen);

class CScrMsg
{
public:
	CScrMsg()
	{
		m_strCmd = _T("");
		m_strData = _T("");
		m_Point.x = 0;
		m_Point.y = 0;
	};

	CScrMsg(CString Cmd, CString Data)
	{
		m_strCmd = Cmd;
		m_strData = Data;
	};

	CScrMsg(CString Cmd, POINT &Pos)
	{
		m_strCmd = Cmd;
		m_Point = Pos;
	};

	CScrMsg(CString Cmd, int x, int y)
	{
		m_strCmd = Cmd;
		m_Point.x = x;
		m_Point.y = y;
	};
	
	CScrMsg(const CScrMsg &Msg)
	{
		m_strCmd = Msg.m_strCmd;
		m_strData = Msg.m_strData;
		m_Point = Msg.m_Point;	
	};

	CScrMsg& operator= (const CScrMsg &Msg)
	{
		this->m_strCmd = Msg.m_strCmd;
		this->m_strData = Msg.m_strData;
		this->m_Point = Msg.m_Point;
		
		return *this;
	};

public:
	CString		m_strCmd;
	CString		m_strData;
	POINT		m_Point;
};


///////////////////////////////////////////////////////////////////
//	Global Variable Define
///////////////////////////////////////////////////////////////////

// Control of Threads
extern HANDLE		g_hDisplayLifeEvent;
extern HANDLE		g_hDisplayWndThread;	// Display Thread Handle (Main Wnd)
extern HANDLE		g_hInteractThread;		// Interact Thread Handle
extern BOOL			g_bInteractDoing;		// Interact Thread 동작 Flag

// Control of System
extern CNHCritSec	g_csNHInput;			// AP의 요청에 대한 동기화 (AP->SCREEN)
extern CNHCritSec	g_csNHOutput;			// AP로 결과 전송에 대한 동기화 (SCREEN->AP)
extern CNHCritSec	g_csNHScreen;			// AP로 결과 전송에 대한 동기화 (SCREEN->AP)
extern HINSTANCE	g_hInstance;			// Instance Handle.
extern HWND			g_hWndDisplay;			// Clinet Wnd Handle
extern CString		g_strLoadSection;		// Master File에서 로드할 Section
extern CString		g_strMasterPath;		// Master File Path
extern SIZE			g_sizeScreen;			// System의 Resolution (Width, Height)
extern BOOL         g_bInitialize;			// if true, SCREEN이 초기화가 되었다.
extern int			g_nLogPixelSY;			// Number of pixel per logical inch along the screen height 

// Control of drawing screen pointer
extern CNScreen		*g_pDisplayScreen;		// 현재 화면에 그려지는 Screen pointer	(화면 <-> 광고)
extern CNScreen		*g_pCurrentScreen;		// AP 요청에 의해 현재 그려진 Screen	(APCenterMovie)
extern CNScreen		*g_pNextScreen;			// AP 요청에 의해 다음에 그려질 Screen	(PrevSetScrrenNo)

// Control of draw background
extern BOOL			g_bBackRefresh;			// if true, 배경을 Refresh 한다. (새로 그림을 그림)
extern BOOL			g_bBackDrawed;			// if true, 배경이 Refresh 될때 배경이 실제로 그려졌다.
extern HBITMAP		g_bmBackBuffer;			// 배경이 그려질 Bitmap Handle
extern CString		g_strBackID;			// 현재 Bitmap에 그려진 배경 ID

// Control of draw screen
extern int			g_nCurrentLocale;		// Selected Screen Language
extern int			g_nMaxLocale;			// Maximum Support Screen Language
extern BOOL			g_bLocalMode;			// if true, Online 모드가 아니다. (AP에서 설정함)
extern HBITMAP		g_hbmScreen;			// 화면이 그려질 Bitmap Handle
extern HFONT		g_hLocalFont;			// Online 모드가 아닐때 뿌려줄 Font.
extern POINT		g_DebugPoint;			// 화면에 디버깅 메시지를 표시할 위치
extern CString		g_DebugMsg;				// 화면에 출력할 디버깅 메시지

// Control of Advertisement
extern ADVERTISE_INFO	g_AdvertiseInfo;

// V1.0.2.4 2018.07.19 - Camera의 좌표를 Script의 Pos 값으로 설정하도록 하기 위함.
// Control info of TextBox for Camera
extern RECT g_rect4Camera;
//extern CString g_strScreenID;
extern BOOL g_bPosValue4Camera;	// Camera Viewer Pos인지 여부
///////////////////////////////////////


///////////////////////////////////////////////////////////////////
//	Global Screen and Resource Containers
///////////////////////////////////////////////////////////////////

// 화면 데이터
extern CNHPtrLinkedList< CNResource >	g_NHResources;
extern CNHPtrLinkedList< CNScreen >		g_NHScreens;

///////////////////////////////////////////////////////////////////
//	Global Communication Queues
///////////////////////////////////////////////////////////////////

// AP <-> SCR 통신 Queue
extern CNHQueue<CScrMsg>	g_FSCmdQueue;
extern CNHQueue<CScrMsg>	g_VariableQueue;


#endif //__SCREEN_DISPLAY_H__