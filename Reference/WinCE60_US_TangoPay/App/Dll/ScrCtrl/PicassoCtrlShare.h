#ifndef __SCREEN_ENGINE_DEFINE_H__
#define __SCREEN_ENGINE_DEFINE_H__

//#include ".\NHScreen.h"
#include ".\PicassoScreen.h"
#include ".\Dll\NHCollection.h"

#ifdef UNDER_CE
	//#define NH_DEBUG
	#include ".\Common\CmnLib.h"
	#include ".\Dev\DevDefine.h"
	#include ".\Common\NHCtrlThread.h"

	#define CROSS_WINAPI
#else
	#include "NHCmnFuncForScrCtrl.h"
	#pragma comment(lib, "Msimg32") 

	#define Delay_Msg	Sleep
	#define Asc2Int				NH_Asc2Int
	#define GetCurrencySymbol	NH_GetCurrencySymbol
	#define SplitString			NH_SplitString

	#define CROSS_WINAPI WINAPI
//	#define NHDEBUG(cond,printf_exp) ((void)0)
//	#define NHDEMSG(cond,printf_exp) ((void)0)
//	#define NHERROR(cond,printf_exp) ((void)0)	
	#define RETAILMSG(cond,printf_exp) ((void)0)

	#define ENG_MODE			1						// ENGLISH
	#define SPN_MODE			2						// SPANISH
	#define FRN_MODE			3						// FRENCH
	#define CHN_MODE			4						// CHINESE
	#define KOR_MODE			5						// KOREAN
	#define JPN_MODE			6						// JAPANESE
	#define LANGUAGE_MODE_MAX	6						// Language max

	#include ".\include\C_NHCtrlThread.h"

#endif

///////////////////////////////////////////////////////////////////
// MSG ID
#define		UM_FSCOMMAND			(WM_USER + 1000)
#define		UM_BLINKING_TIME		(WM_USER + 10000)	// SCREEN TIME OUT
#define		UM_ADV_SHOW_TIME		(WM_USER + 10001)	// Advertise ment time
#define		UM_ADV_DECODE_TIME		(WM_USER + 10002)	// Advertise ment time
//#define		BLINKING_TIME			(500)
#define		BLINKING_TIME			(400)
//#define		BLINKING_TIME			(250)
//#define		BLINKING_TIME			(125)

#define		DECODE_INTERVAL			(500)
#define		UM_TIMER_ID_BASE		(110000000)

//////////////////////////////////////////
//	Advertisement Define.
//
#define SUPPORT_ADVERTISE               8               // [#RWC6-23] US William 2019.09.09 Add GivePay Extra Ads

#define REFRESH_TIME_MIN		5		// sec
#define REFRESH_TIME_MAX		30		// sec

typedef struct _ADVERTISE_INFO_
{
	BOOL		bEnable;								// 전체 광고 Enable 여부
	int			nEnableCount;							// Enable된 광고 개수
	CPicassoScreen	*pCurAdvScreen;						// 현재 광고 스크린 포인터, 만약 NULL이면 AP 화면이 뿌려진다.
	int			nCurAdvIndex;							// 현재 광고 Index
	int			nRefreshTime;							// 광고 표시 시간
	BOOL		bEnableAdv[SUPPORT_ADVERTISE+1];		// 각 광고 별 Enable 여부
	CPicassoScreen	*pAdvScreen[SUPPORT_ADVERTISE+1];	// 각 광고 별 스크린 포인터
	CString		strFileName[SUPPORT_ADVERTISE+1];		// 각 광고 Full 경로.
	int			nShowCount;								// 화면을 표시한 카운트.
} ADVERTISE_INFO;

#define	SUPPORT_ADVNOTICE	((SUPPORT_ADVERTISE+1)*2)	// [#RWC6-23] US William 2019.09.09 Add GivePay Extra Ads
struct _ADV_NOTICE_WELCOME_INFO
{
	BOOL			bEnable;
	int				nCurIndex;
	int				nMaxIndex;
	CPicassoScreen	*pScreen[SUPPORT_ADVNOTICE];
};

#define SUPPORT_NOTICE	4
struct _NOTICE_INFO
{
	BOOL			bEnable;
	CPicassoScreen *pScreen;
	CString			strMsg[SUPPORT_NOTICE];
};

class CScrMsg
{
public:
	CScrMsg()
	{
		m_strCmd = _T("");
		m_strData = _T("");
		m_strData2 = _T("");
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
		m_strData2 = Msg.m_strData2;
		m_Point = Msg.m_Point;
	};

	CScrMsg& operator= (const CScrMsg &Msg)
	{
		this->m_strCmd = Msg.m_strCmd;
		this->m_strData = Msg.m_strData;
		this->m_strData2 = Msg.m_strData2;
		this->m_Point = Msg.m_Point;
		
		return *this;
	};

public:
	CString		m_strCmd;
	CString		m_strData;
	CString		m_strData2;
	POINT		m_Point;
};

///////////////////////////////////////////////////////////////////
//	Help Macro Define

#define MulDiv(a,b,c)       (((a)*(b))/(c))
#define GET_FONTSIZE(PointSize) (-MulDiv((PointSize), m_nLogPixelsY, 72))

///////////////////////////////////////////////////////////////////
//	Global Variable Define
///////////////////////////////////////////////////////////////////

#endif //__SCREEN_ENGINE_DEFINE_H__