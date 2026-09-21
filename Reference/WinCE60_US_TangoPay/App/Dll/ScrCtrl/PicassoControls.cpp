#include "stdafx.h"
#include ".\PicassoControls.h"
#include ".\PicassoCtrlShare.h"
#include ".\PicassoCtrlDescription.h"

#include ".\Common\ConstDef.h"	// [#2388] US Justin include Constant

#ifndef UNDER_CE
#include <wingdi.h>
#include <windows.h>
#endif

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		0
#define DBG_INFO		0

//------------------------------------------------------------------
//	Define for global variable
//------------------------------------------------------------------

// Text Indext
#define		NH_INDENT				3

// preDefine 50 *
TCHAR	g_strPassAsterisk[] = _T("*************************************************");

#define	SCALE_INIT		-9999
// for swkey
#define MAX_KEY_SCR		11
#define MAX_KEY_DATA	10

// [#77] KGS 2008.4.18 국가별 통화단위 캐릭터 입력 지원
#define STR_POUNT_CHAR		(TCHAR)0xA3
#define STR_YEN_CHAR		(TCHAR)0xA5
//#define STR_EURO_CHAR		(TCHAR)0x8364

CString	g_NSwKeyData[MAX_KEY_SCR][MAX_KEY_DATA] =
{
	// 0 Scr
	{
		_T("0"),_T("1"), _T("2"), _T("3"), _T("4"),
		_T("5"),_T("6"), _T("7"), _T("8"), _T("9")
	},
	
	// 1 Scr
	{
		_T("SPC"),_T("A"), _T("B"), _T("C"), _T("D"),
		_T("E"),_T("F"), _T("G"), _T("H"), _T("I")
	},
	// 2 Scr
	{
		_T("SPC"), _T("J"), _T("K"), _T("L"), _T("M"), 
		_T("N"), _T("O"),_T("P"), _T("Q"), _T("R")
	},
	// 3 Scr
	{
		_T("SPC"), _T("S"),_T("T"), _T("U"), _T("V"),
		_T("W"), _T("X"),_T("Y"),_T("Z"), _T("")
	},

	// 4 scr
	{
		_T("SPC"),_T("a"), _T("b"), _T("c"), _T("d"),
		_T("e"),_T("f"), _T("g"), _T("h"), _T("i")
	},
	// 5 scr
	{
		_T("SPC"), _T("j"), _T("k"), _T("l"), _T("m"), 
		_T("n"), _T("o"),_T("p"), _T("q"), _T("r")
	},
	// 6 scr
	{
		_T("SPC"), _T("s"),_T("t"), _T("u"), _T("v"),
		_T("w"), _T("x"),_T("y"),_T("z"), _T("")
	},

	// 7 Scr
	{
		_T("SPC"),_T("!"),_T("@"), _T("#"), _T("$"), 
		_T("%"),_T("&&"),_T("*"), _T("."), _T("?")
	},
	// 8 Scr
	{
		_T("SPC"),_T("("), _T(")"), _T("-"), _T("<"),
		_T(">"),_T("+"), _T("/"), _T(","), _T("=")
	},
	// 9 Scr
	{
		_T("SPC"),_T("{"), _T("}"), _T("'"), _T("["),
		_T("]"),_T("="), _T("~"), _T("|"), _T("^")
	},
	// 10 Scr
	{
		_T("SPC"), _T(";"),_T(":"), STR_POUNT_CHAR, STR_YEN_CHAR,
		_T(""), _T(""), _T(""), _T(""), _T("")
	}
};


//------------------------------------------------------------------
//	CPicassoRequest Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: CPicassoRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자
-------------------------------------------------------------------*/
CPicassoRequest::CPicassoRequest()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_ReqType = REQ_NONE;
	m_Name = _T("");
	m_Data = _T("");
	m_hDC = NULL;
	m_rcUpdate.left = 0;				// [2ND] NH AIREAT 2008.11.12
	m_rcUpdate.right = 0;				// [2ND] NH AIREAT 2008.11.12
	m_rcUpdate.top = 0;					// [2ND] NH AIREAT 2008.11.12
	m_rcUpdate.bottom = 0;				// [2ND] NH AIREAT 2008.11.12
	m_Point.x = 0;						// [2ND] NH AIREAT 2008.11.12
	m_Point.y = 0;						// [2ND] NH AIREAT 2008.11.12
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: CPicassoRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자
-------------------------------------------------------------------*/
CPicassoRequest::CPicassoRequest(REQ_TYPE type, CString Name, CString Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	Set(type, Name, Data);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: ~CPicassoRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CPicassoRequest::~CPicassoRequest()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}


/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: Set()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Request 정보를 설정한다.
-------------------------------------------------------------------*/
void CPicassoRequest::Set(REQ_TYPE type, LPCTSTR Name, LPCTSTR Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_ReqType = type;
	m_Name = Name;
	m_Data = Data;
	m_hDC = NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: CPicassoRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoRequest::CPicassoRequest(const CPicassoRequest &rRequest)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_ReqType = rRequest.m_ReqType;
	m_Name = rRequest.m_Name;
	m_Data = rRequest.m_Data;
	m_hDC = rRequest.m_hDC;
	m_rcUpdate = rRequest.m_rcUpdate;			// [2ND] NH AIREAT 2008.11.12
	m_Point = rRequest.m_Point;					// [2ND] NH AIREAT 2008.11.12
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: GetKind()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Request 종류를 조회한다
-------------------------------------------------------------------*/
REQ_TYPE CPicassoRequest::GetKind()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_ReqType;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: GetName()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Name을 조회한다.
-------------------------------------------------------------------*/
CString	CPicassoRequest::GetName()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_Name;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: GetData()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Data를 조회한다
-------------------------------------------------------------------*/
CString	CPicassoRequest::GetData()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_Data;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: SetKinD()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Request 종류를 설정한다
-------------------------------------------------------------------*/
void CPicassoRequest::SetKinD(REQ_TYPE type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_ReqType = type;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: SetName()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Name을 설정한다
-------------------------------------------------------------------*/
void CPicassoRequest::SetName(CString Name)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_Name = Name;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoRequest
 FUNCTION NAME: SetData()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Data를 설정한다
-------------------------------------------------------------------*/
void CPicassoRequest::SetData(CString Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_Data = Data;
}


//------------------------------------------------------------------
//	CPicassoEventHandler Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: CPicassoRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEventHandler::CPicassoEventHandler(CPicassoEventHandler *pNext) : m_pNextHandler(pNext)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
	m_pRequests = NULL;
	m_pInvalidRect = NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: ~CPicassoRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEventHandler::~CPicassoEventHandler()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: SetNextHandler()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEventHandler::SetNextHandler(CPicassoEventHandler *pNext)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pNextHandler = pNext;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: SetRequestQueue()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEventHandler::SetRequestQueue(CNHLinkedList<CPicassoRequest> *pRequestQueue)
{
	m_pRequests = pRequestQueue;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: GetRequestQueue()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CNHLinkedList<CPicassoRequest>* CPicassoEventHandler::GetRequestQueue()
{
	return m_pRequests;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: SetInvalidRect()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEventHandler::SetInvalidRect(RECT *pRect)
{
	m_pInvalidRect = pRect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: AddRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::AddRequest(REQ_TYPE type, LPCTSTR Name, LPCTSTR Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoRequest	req;
	req.Set(type, Name, Data);

	return AddRequest(req);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: AddRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::AddRequest(CPicassoRequest &rRequest)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pRequests == NULL)
	{
		return FALSE;
	}

	return m_pRequests->PutOnTailOfList(_T(""), rRequest);				// [#414] AIREAT 2008.09.05
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: GetRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::GetRequest(CPicassoRequest &rRequest)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pRequests == NULL)
		return FALSE;

	return m_pRequests->GetFromHeadOfList(rRequest);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: ClearRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::ClearRequest(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pRequests == NULL)
		return FALSE;

	m_pRequests->DeleteAll();						// [#414] AIREAT 2008.09.05

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: AddInvalidRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEventHandler::AddInvalidRect(RECT rcRect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pInvalidRect == NULL)
		return;

	if (rcRect.left == SCR_COORD_MAX && rcRect.right == SCR_COORD_MIN && 
		rcRect.top == SCR_COORD_MAX && rcRect.bottom == SCR_COORD_MIN)
	{
		*m_pInvalidRect = rcRect;
		return;
	}

	// left
	m_pInvalidRect->left = __min(rcRect.left, m_pInvalidRect->left);

	// right
	m_pInvalidRect->right = __max(rcRect.right, m_pInvalidRect->right);
		
	// top
	m_pInvalidRect->top = __min(rcRect.top, m_pInvalidRect->top);

	// bottom
	m_pInvalidRect->bottom = __max(rcRect.bottom, m_pInvalidRect->bottom);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: GetInvalidRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
RECT CPicassoEventHandler::GetInvalidRect()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pInvalidRect == NULL)
	{
		RECT	rcTemp;
		rcTemp.left = SCR_COORD_MAX;
		rcTemp.top = SCR_COORD_MAX;
		rcTemp.right = SCR_COORD_MIN;
		rcTemp.bottom = SCR_COORD_MIN;

		return rcTemp;
	}

	return *m_pInvalidRect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: ClearInvalidRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEventHandler::ClearInvalidRect()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pInvalidRect != NULL)
	{
		m_pInvalidRect->left = SCR_COORD_MAX;
		m_pInvalidRect->top = SCR_COORD_MAX;

		m_pInvalidRect->right = SCR_COORD_MIN;
		m_pInvalidRect->bottom = SCR_COORD_MIN;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: Handler()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::Handler(CPicassoRequest &Request)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;

	switch(Request.GetKind())
	{
	case REQ_INITIAL:
		Initialize();
		break;

	case REQ_DEINITIAL:
		Deinitialize();
		break;

		// 컨트롤이 한개라도 그려졌으면 TRUE 이다.
	case REQ_DRAW:
		bRes |= DrawControl(Request.m_hDC, Request.m_rcUpdate);
		if (m_pNextHandler != NULL)
			bRes |= m_pNextHandler->Handler(Request);

		return bRes;

	case REQ_LOCALE:
		bRes = SetLocaleNum(Request.m_nLocale);
		break;

	case REQ_MOUSEEVENT:
		bRes = MouseEvent(Request.m_Point);
		break;

	case REQ_KEYEVENT:
		bRes = KeyEvent(Request.m_Data);
		break;

	case REQ_SETVALUE:
		bRes = SetVariable(Request.GetName(), Request.GetData());
		break;

	case REQ_ALLFSCMD:
		MakeAllFSCMD(Request.m_Data);
		break;

	case REQ_SETFOCUS:
		SetCtlFocus(Request.GetName(), Request.GetData());
		break;

	case REQ_BTNFIND:
		bRes = FindActBtn(Request.GetData());
		break;

	case REQ_BINACT:
		bRes = BinAct(Request.GetName());
		break;

	case REQ_BLINKING:
		SetBlinking();
		break;

	default:
		break;
	}

	if (m_pNextHandler && !bRes)
		return m_pNextHandler->Handler(Request);

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pInvalidRect != NULL)
	{
		m_pInvalidRect->left = SCR_COORD_MAX;
		m_pInvalidRect->top = SCR_COORD_MAX;
		m_pInvalidRect->right = SCR_COORD_MIN;
		m_pInvalidRect->bottom = SCR_COORD_MIN;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: Deinitialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::Deinitialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::DrawControl(HDC hDC, RECT &rcUpdate)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: SetLocaleNum()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::SetLocaleNum(int Locale)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: MakeAllFSCMD()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::MakeAllFSCMD(CString &Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: FindActBtn()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::FindActBtn(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: BinAct()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::BinAct(CString &strAct)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: SetBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::SetBlinking(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEventHandler
 FUNCTION NAME: MouseEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEventHandler::MouseEvent(POINT &point)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoEventHandler Implementation
//------------------------------------------------------------------

int		CPicassoControl::m_nLocaleNum = 0;

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: CPicassoControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoControl::CPicassoControl(CTL_TYPE	type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_CtlType = type;
	m_bAutoFocus = FALSE;
	m_bFocus = FALSE;

	memset(&m_rcDisplay, 0, sizeof(m_rcDisplay));

	m_nScaleUnit = SCALE_INIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: ~CPicassoControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoControl::~CPicassoControl()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: GetAdjustScreenReate()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
RECT CPicassoControl::GetAdjustScreenReate(int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	float		fScreenRateX;
	float		fScreenRateY;

	fScreenRateX = (float)((float)nLoadWidth / (float)BASIC_SCREEN_X);

	// [#2300] US KSK 2014.1.04
	if (nLoadWidth == 1024 && nLoadHeight == 768)
		fScreenRateY = (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y);		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
	else
		fScreenRateY = (float)((float)nLoadHeight / (float)BASIC_SCREEN_Y);
	// end of [#2300]

	// 에러 발생시 무조건 640X480으로 화면을 만든다.
	if (fScreenRateX == 0 || fScreenRateY == 0)
	{
		fScreenRateX = 1;
		fScreenRateY = 1;
	}

	RECT tmpRect;

	tmpRect.left = (int)((float)left * fScreenRateX);
	tmpRect.top = (int)((float)top * fScreenRateY);
	tmpRect.right = tmpRect.left + (int)((float)width * fScreenRateX);
	tmpRect.bottom = tmpRect.top + (int)((float)height * fScreenRateY);

	return tmpRect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: GetType()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CTL_TYPE CPicassoControl::GetType()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_CtlType;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: GetName()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
LPCTSTR CPicassoControl::GetName()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_strName;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: SetName()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoControl::SetName(LPCTSTR str)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (str)
		m_strName = str;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: GetLocaleNum()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CPicassoControl::GetLocaleNum()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_nLocaleNum;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: SetLocaleNum()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoControl::SetLocaleNum(int Locale)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nLocaleNum = Locale;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: GetDisplayRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
RECT CPicassoControl::GetDisplayRect()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_nScaleUnit == SCALE_INIT)
		return m_rcDisplay;
	
	CRect newRect = m_rcDisplay;
	
	newRect.InflateRect(m_nScaleUnit, m_nScaleUnit);
	return (RECT)newRect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: GetMousePointRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
RECT CPicassoControl::GetMousePointRect()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_nScaleUnit == SCALE_INIT)
		return m_rcDisplay;
	
	CRect newRect = m_rcDisplay;
	
	newRect.InflateRect(0, 0, 0, 0);
	return (RECT)newRect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: SetDisplayRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoControl::SetDisplayRect(int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_rcDisplay = GetAdjustScreenReate(left, top, width, height, nLoadWidth, nLoadHeight);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: SetAutoFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoControl::SetAutoFocus(BOOL bAuto)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bAutoFocus = bAuto;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: SetFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoControl::SetFocus(BOOL bFocus)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bFocus = bFocus;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: IsUpdateControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoControl::IsUpdateControl(RECT &rcUpdate)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	POINT	point;
	
	// 기준은 Update 영역.
	point.x = m_rcDisplay.left;
	point.y = m_rcDisplay.top;

	if (PtInRect(&rcUpdate, point))
		return TRUE;

	point.x = m_rcDisplay.left;
	point.y = m_rcDisplay.bottom;

	if (PtInRect(&rcUpdate, point))
		return TRUE;

	point.x = m_rcDisplay.right;
	point.y = m_rcDisplay.top;

	if (PtInRect(&rcUpdate, point))
		return TRUE;

	point.x = m_rcDisplay.right;
	point.y = m_rcDisplay.bottom;

	if (PtInRect(&rcUpdate, point))
		return TRUE;

	if (rcUpdate.top <= m_rcDisplay.top && rcUpdate.bottom >= m_rcDisplay.bottom &&
		rcUpdate.left >= m_rcDisplay.left && rcUpdate.right <= m_rcDisplay.right)
		return TRUE;

	if (rcUpdate.top >= m_rcDisplay.top && rcUpdate.bottom <= m_rcDisplay.bottom &&
		rcUpdate.left <= m_rcDisplay.left && rcUpdate.right >= m_rcDisplay.right)
		return TRUE;

	// 기준은 내 영역.
	point.x = rcUpdate.left;
	point.y = rcUpdate.top;

	if (PtInRect(&m_rcDisplay, point))
		return TRUE;

	point.x = rcUpdate.left;
	point.y = rcUpdate.bottom;

	if (PtInRect(&m_rcDisplay, point))
		return TRUE;

	point.x = rcUpdate.right;
	point.y = rcUpdate.top;

	if (PtInRect(&m_rcDisplay, point))
		return TRUE;

	point.x = rcUpdate.right;
	point.y = rcUpdate.bottom;

	if (PtInRect(&m_rcDisplay, point))
		return TRUE;

	if (m_rcDisplay.top <= rcUpdate.top && m_rcDisplay.bottom >= rcUpdate.bottom &&
		m_rcDisplay.left >= rcUpdate.left && m_rcDisplay.right <= rcUpdate.right)
		return TRUE;

	if (m_rcDisplay.top >= rcUpdate.top && m_rcDisplay.bottom <= rcUpdate.bottom &&
		m_rcDisplay.left <= rcUpdate.left && m_rcDisplay.right >= rcUpdate.right)
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoControl
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoControl::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bAutoFocus)
		m_bFocus = TRUE;
	else
		m_bFocus = FALSE;

	m_nScaleUnit = SCALE_INIT;

	return TRUE;
}


//------------------------------------------------------------------
//	CPicassoShape Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoShape
 FUNCTION NAME: CPicassoShape()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoShape::CPicassoShape(CTL_TYPE type) : CPicassoControl(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pPen = m_pFocusPen = NULL;
	m_pBrush = m_pFocusBrush = NULL;
	m_pPicture = NULL;
	m_pIcon = NULL;
	m_nRoundWidth = 0;
	m_nRoundHeight = 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoShape
 FUNCTION NAME: ~CPicassoShape()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoShape::~CPicassoShape()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoShape
 FUNCTION NAME: SetPen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoShape::SetPen(CPicassoResource *pNormal, CPicassoResource *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (pNormal)
	{
		if (pNormal->GetTpye() == RES_PEN)
			m_pPen = pNormal;
	}

	if (pSelect)
	{
		if (pSelect->GetTpye() == RES_PEN)
			m_pFocusPen = pSelect;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoShape
 FUNCTION NAME: SetBrush()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoShape::SetBrush(CPicassoResource *pNormal, CPicassoResource *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (pNormal)
	{
		if (pNormal->GetTpye() == RES_BRUSH)
			m_pBrush = pNormal;
	}

	if (pSelect)
	{
		if (pSelect->GetTpye() == RES_BRUSH)
			m_pFocusBrush = pSelect;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoShape
 FUNCTION NAME: SetPicture()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoShape::SetPicture(CPicassoResource *pPicture)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (pPicture)
	{
		if (pPicture->GetTpye() == RES_PICTURE)
			m_pPicture = (CPicassoPicture*)pPicture;
	}
}

/*-------------------------------------------------------------------
CLASS    NAME: CPicassoShape
FUNCTION NAME: SetIcon()
WRITER       : woooz
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoShape::SetIcon(CPicassoResource *pPicture, int pWidth, int pHeight)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (pPicture)
	{
		if (pWidth > 0 && pHeight > 0)
		{
			m_pIcon = (CPicassoPicture*)pPicture;
			m_nIconWidth = pWidth;
			m_nIconHeight = pHeight;
		}
	}
}


/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoShape
 FUNCTION NAME: SetBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoShape::SetBlinking(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pPicture)
	{
		if (m_pPicture->IsAnimationImage(GetLocaleNum()))
		{
			AddInvalidRect(GetDisplayRect());

			return TRUE;
		}
	}

	return FALSE;
}

void CPicassoShape::SetRoundSize(int nWidth, int nHeight)
{
	m_nRoundWidth = nWidth;
	m_nRoundHeight = nHeight;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoShape
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoShape::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;
	HPEN	OldPen;
	HBRUSH	OldBrush;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// Draw Picture.
	if (m_pPicture)
	{
		RECT DisplayRect = GetDisplayRect();
		m_pPicture->DrawImage(GetLocaleNum(), hDC, DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom);

#ifdef AE_SCREEN_DEGISN
	// Select Pen
	OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
	OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));

	// Draw Rectangle.
	::Rectangle(hDC, DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom);

	// Restore Object
	::SelectObject(hDC, OldBrush);
	::SelectObject(hDC, OldPen);
#endif
		bRes = TRUE;
	}
	// Draw Boarder and Fill Color
	else if (m_pPen || m_pFocusPen || m_pBrush || m_pFocusBrush)
	{
		// Select Pen
#ifndef AE_SCREEN_DEGISN
		if (m_bFocus && m_pFocusPen)
			OldPen = (HPEN)::SelectObject(hDC, (HPEN)m_pFocusPen->GetHandle());
		else if (m_pPen)
			OldPen = (HPEN)::SelectObject(hDC, (HPEN)m_pPen->GetHandle());
		else
			OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(NULL_PEN));
#else
		OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
#endif

		// Select Brush
		if (m_bFocus && m_pFocusBrush)
			OldBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pFocusBrush->GetHandle());
		else if (m_pBrush)
			OldBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pBrush->GetHandle());
		else
			OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
		
		// Draw Rectangle.
		if (m_nRoundWidth == 0 && m_nRoundHeight == 0)
		{
			RECT DisplayRect = GetDisplayRect();
			::Rectangle(hDC, DisplayRect.left, DisplayRect.top, 
				DisplayRect.right, DisplayRect.bottom);
		}
		else
		{
			RECT DisplayRect = GetDisplayRect();
			::RoundRect(hDC, DisplayRect.left, DisplayRect.top, 
				DisplayRect.right, DisplayRect.bottom, m_nRoundWidth, m_nRoundHeight);
		}
		
		// Restore Brush
		::SelectObject(hDC, OldBrush);
		
		// Restore Pen.
		::SelectObject(hDC, OldPen);
		bRes = TRUE;
	}
#ifdef AE_SCREEN_DEGISN
	else
	{
		RECT DisplayRect = GetDisplayRect();

		// Select Pen
		OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
		OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
		
		// Draw Rectangle.
		::Rectangle(hDC, DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom);
		
		// Restore Object
		::SelectObject(hDC, OldBrush);
		::SelectObject(hDC, OldPen);
		bRes = TRUE;
	}
	// AE TEST
#endif

	return bRes;
}


//------------------------------------------------------------------
//	CPicassoTextBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: CPicassoTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTextBox::CPicassoTextBox(CTL_TYPE type) : CPicassoShape(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pFont = m_pFocusFont = NULL;
	m_pLocaleText = NULL;

	m_Align = LEFT;
	m_bMultiline = FALSE;
	m_pColor = m_pFocusColor = NULL;

	m_TextType = TTYPE_TEXT;

	m_bBlinking = FALSE;
	m_bBlinkMode = FALSE;

	m_nBlinkCount = 0;

	m_arActKey.RemoveAll();

	m_nOutlineWidth = 0;
	m_pOutlineColor = NULL;

	//m_pResourcesQueue = NULL;
	m_pResourcesQueueCMap = NULL;

	m_bShow = TRUE;

	// [#2186] US KMK 2013.05.06 폰트 관련 포인터 변수 초기화 추가 (2700T OP 화면 및 Bold 폰트 적용불가 현상 수정)
	for (int i=0; i<MAX_LOCALE; i++)
		m_pFonts[i] = m_pFocusFonts[i] = NULL;
	// end of [#2186]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: ~CPicassoTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTextBox::~CPicassoTextBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

//void CPicassoTextBox::SetResourcesQueue(CNHPtrLinkedList< CPicassoResource >	*pResourcesQueue)
//{
//	m_pResourcesQueue = pResourcesQueue;
//}

void CPicassoTextBox::SetResourcesQueue(CMap < CString, LPCTSTR, CPicassoResource*, CPicassoResource* >	*pResourcesQueueCMap)
{
	m_pResourcesQueueCMap = pResourcesQueueCMap;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetLocaleText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetLocaleText(CPicassoResource *pLocaleText)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pLocaleText = (CPicassoLocaleText*)pLocaleText;
	m_strText = _T("");
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetFont()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetFont(CPicassoFont *pNormal, CPicassoFont *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (pNormal)
	{
		if (pNormal->GetTpye() == RES_FONT)
			m_pFont = pNormal;
	}

	if (pSelect)
	{
		if (pSelect->GetTpye() == RES_FONT)
			m_pFocusFont = pSelect;
	}
}

// [#2186] US KMK 2013.04.09 Asian Language Support
void CPicassoTextBox::SetFonts(CPicassoFont *pNormals[MAX_LOCALE], CPicassoFont *pSelects[MAX_LOCALE])
{
	for (int i=0; i<MAX_LOCALE; i++)
	{
		if (pNormals[i])
		{
			if (pNormals[i]->GetTpye() == RES_FONT)
				m_pFonts[i] = pNormals[i];
		}

		if (pSelects[i])
		{
			if (pSelects[i]->GetTpye() == RES_FONT)
				m_pFocusFonts[i] = pSelects[i];
		}
	}
}
// end of [#2186]

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetBlinkMode()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetBlinkMode(BOOL BlinkOn)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bBlinkMode = BlinkOn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetText(LPCTSTR pText)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_strText = pText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetTextAlign()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetTextAlign(TEXTALIGN Align)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_Align = Align;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetMultiLine()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetMultiLine(BOOL bMultiline)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bMultiline = bMultiline;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetTextColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetTextColor(CPicassoColor *pColor, CPicassoColor *pFocusColor)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pColor = pColor;
	m_pFocusColor = pFocusColor;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetOutline()
 WRITER       : AIREAT (2010.04.29)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetOutline(int nWidth, CPicassoColor *pColor)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
	
	m_nOutlineWidth = nWidth;
	m_pOutlineColor = pColor;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetActKey()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetActKey(LPCTSTR pKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	//m_ActKey += pKey;
	m_arActKey.Add(pKey);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: IsActKey()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTextBox::IsActKey(CString strKey)
{
	for (int i = 0; i < m_arActKey.GetCount(); i++)
	{
		if (strKey == m_arActKey[i])
			return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetAct()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetAct(REQ_TYPE type, LPCTSTR pName, LPCTSTR pData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_Act.Set(type, pName, pData);	
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetTextType()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTextBox::SetTextType(TEXTTYPE type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_TextType = type;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: GetInputText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
LPCTSTR CPicassoTextBox::GetInputText(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return (LPCTSTR)m_strText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTextBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		if (!_tcsncmp(strData, DES_AP_TEXT_ID, 9) && (strData.GetLength() > 9))
		{
			CString strTextID = strData.Mid(9);
			strTextID.MakeUpper();	// KSK 2012.12.21

			//if (m_pResourcesQueue != NULL)
			if (m_pResourcesQueueCMap != NULL)
			{
				CPicassoResource *pResource = NULL;
				//if (m_pResourcesQueue->FindDataOfList(strTextID, &pResource))
				if (m_pResourcesQueueCMap->Lookup(strTextID, pResource) == TRUE)
					SetLocaleText(pResource);
			}
		}
		else if (_tcsncmp(strName, DES_AP_STATE, 7) == 0)
		{
			// On
			if (_tcsncmp(strData, DES_ON, _tcslen(DES_ON)) == 0)
			{
				m_bShow = TRUE;
				NHUIDBG(DBG_CALL, (_T("CPicassoTextBox::SetVariable APSTATE ON\n")));
			}
			// Off
			else if (_tcsncmp(strData, DES_OFF, _tcslen(DES_OFF)) == 0)
			{
				m_bShow = FALSE;
				NHUIDBG(DBG_CALL, (_T("CPicassoTextBox::SetVariable APSTATE OFF\n")));
			}
		}
		else
		{	
			if (strData.Find('&') != -1 && m_bMultiline == TRUE)
			{
				CString strTemp;

				m_strText = _T("");
				for (int i = 0; i < strData.GetLength(); i++)
				{
					strTemp = strData[i];

					if (strTemp == '&')
						strTemp = _T("&&");

					m_strText += strTemp;
				}
			}
			else
				m_strText = strData;

			m_pLocaleText = NULL;
		}
		
		AddInvalidRect(GetDisplayRect());
		
		return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTextBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoShape::Initialize();

	if (m_strName.GetLength() > 0)
	{
		m_pLocaleText = NULL;
		m_strText = _T("");
	}

	m_bBlinking = FALSE;
	m_nBlinkCount = 0;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: SetBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTextBox::SetBlinking(void)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoShape::SetBlinking();

	m_nBlinkCount++;

	if (m_bBlinkMode && ((m_nBlinkCount * BLINKING_TIME) >= 500))
	{
		m_nBlinkCount = 0;

		m_bBlinking = !m_bBlinking;
		AddInvalidRect(GetDisplayRect());

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: GetDrawText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CPicassoTextBox::GetDrawText()
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	CString strDrawMsg;

	if (m_pLocaleText)
	{
		strDrawMsg = m_pLocaleText->GetLocaleText(GetLocaleNum());
	}
	else
	{
		if (m_TextType == TTYPE_PASSWORD)
		{
			strDrawMsg = g_strPassAsterisk;
			strDrawMsg = strDrawMsg.Left(m_strText.GetLength());
		}
		else
		{
			strDrawMsg = m_strText;
		}
	}	

	return strDrawMsg;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL		bRes = FALSE;
	CString		strDrawText;

	if (!m_bShow)
	{
		return FALSE;
	}

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// PARENT CLASS DRAW
	bRes |= CPicassoShape::DrawControl(hDC, rcUpdate);
	
	// Get to Draw Text string
	strDrawText = GetDrawText();

	////////////////////////////////////
	if (strDrawText.GetLength())
	{
		int			OldBkMode;
		HFONT		OldFont = NULL;
		COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
		
		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		
		// Select Font
		if (m_bFocus && m_pFocusFonts[GetLocaleNum()-1])
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFocusFonts[GetLocaleNum()-1]->GetHandle());
		else if (m_pFonts[GetLocaleNum()-1])
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFonts[GetLocaleNum()-1]->GetHandle());

		if (m_bFocus && m_pColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pColor->m_crColor);
		else if (m_pFocusColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pFocusColor->m_crColor);
				
		// Draw Text
		if (!m_bBlinking)
		{
			LOGFONT	lFontResized;	// [#RWC6-262] font downsizing
			UINT	uDrawTextFormat, uTextOutFormat;
			RECT	rcClient;
			RECT	rcTemp = {m_rcDisplay.left, 0, m_rcDisplay.right, 0};
			
			uDrawTextFormat = 0;

			if (!m_bMultiline)
				uDrawTextFormat = DT_SINGLELINE | DT_VCENTER;
			else
				// uDrawTextFormat = DT_WORDBREAK | DT_WORD_ELLIPSIS;
				uDrawTextFormat = DT_WORDBREAK;		// [#RWC6-262] DT_WORD_ELLIPSIS makes 'blahblah' as 'blahb...', we don't need this
			
			if (m_Align == LEFT)
				uDrawTextFormat |= DT_LEFT;
			else if (m_Align == CENTER)
				uDrawTextFormat |= DT_CENTER;
			else if (m_Align == RIGHT)
				uDrawTextFormat |= DT_RIGHT;
				
			// Adjust Draw Rect
			rcClient = GetDisplayRect();
			rcClient.left += NH_INDENT;
			rcClient.right -= NH_INDENT;

			// paddings to avoid text overwrapping with OP button
			if (m_CtlType == CTL_BUTTONBOX)
			{
				rcClient.top += (NH_INDENT + 1);
				rcClient.bottom -= (NH_INDENT + 1);
			}

			// Calc Rect.
			if (strDrawText.GetLength() > 0)
			{
				// [#RWC6-262] DAT integration
				// font resizing, from: https://stackoverflow.com/questions/29825498/sizing-font-to-fit-in-a-rectangle
				int nRes = 0;
				if (m_bFocus && m_pFocusFonts[GetLocaleNum()-1])
					nRes = GetObject((HFONT)m_pFocusFonts[GetLocaleNum()-1]->GetHandle(), sizeof(LOGFONT), (LPVOID*)&lFontResized);
				else if (m_pFonts[GetLocaleNum()-1])
					nRes = GetObject((HFONT)m_pFonts[GetLocaleNum()-1]->GetHandle(), sizeof(LOGFONT), (LPVOID*)&lFontResized);

				if (nRes == 0)
				{
					NHDEBUG(DBG_CALL, (_T("failed to get font object, unavailable to resize! [%d]\n"), GetLastError()));
				}
				else
				{
					int nAreaWidth = rcClient.right - rcClient.left;
					int nAreaHeight = rcClient.bottom - rcClient.top;
					UINT nCalcRectFormat = m_bMultiline ? (DT_CALCRECT | DT_WORDBREAK) : (DT_CALCRECT);

					// 1 of 2) width adjustment, it would be happy if lfWidth supports .5 unit...
					for (int nWidth = lFontResized.lfWidth; nWidth > 2; nWidth--)
					{
						lFontResized.lfWidth = nWidth;
						HFONT fontResized = CreateFontIndirect(&lFontResized);
						HFONT fontOriginal = (HFONT)::SelectObject(hDC, fontResized);

						rcTemp = rcClient;
						int nTextHeight = DrawText(hDC, strDrawText, -1, &rcTemp, nCalcRectFormat);
						int nTextWidth = rcTemp.right - rcTemp.left;

						::SelectObject(hDC, fontOriginal);
						::DeleteObject(fontResized);

						// it fits
						if (nTextWidth <= nAreaWidth)
						{
							break;
						}
						// need another more smaller
						else
						{
							NHDEBUG(DBG_CALL, (_T("width adjustment required\n")));
						}
					}

					// 2 of 2) height adjustment
					for (int nHeight = -lFontResized.lfHeight; nHeight > 2; nHeight--)
					{
						lFontResized.lfHeight = -nHeight;
						HFONT fontResized = CreateFontIndirect(&lFontResized);
						HFONT fontOriginal = (HFONT)::SelectObject(hDC, fontResized);

						rcTemp = rcClient;
						int nTextHeight = DrawText(hDC, strDrawText, -1, &rcTemp, nCalcRectFormat);

						::SelectObject(hDC, fontOriginal);
						::DeleteObject(fontResized);

						if (nTextHeight <= nAreaHeight)
						{
							// vertical center alignment
							rcClient.top += (int)((nAreaHeight - nTextHeight) / 2);
							break;
						}
						else
						{
							NHDEBUG(DBG_CALL, (_T("height adjustment required\n")));
						}
					}
				}
				
				// Draw Text
				HFONT fontFinal = CreateFontIndirect(&lFontResized);
				HFONT fontOriginal = (HFONT)::SelectObject(hDC, fontFinal);

				int nOutlineBasePoint, nOutlineThickness = 0;
				if (m_nOutlineWidth > 0)
				{
					nOutlineBasePoint = -m_nOutlineWidth;
					nOutlineThickness = m_nOutlineWidth;
				}
				else
				{
					nOutlineBasePoint = 0;
					nOutlineThickness = -m_nOutlineWidth;
				}

				if (m_bMultiline)
				{
					if (m_nOutlineWidth != 0)
					{
						RECT		rcOutline;
						COLORREF	hOlddd = 0xffffffff;

						if (m_pOutlineColor != NULL)
							hOlddd = ::SetTextColor(hDC, m_pOutlineColor->m_crColor);
						else
							hOlddd = ::SetTextColor(hDC, RGB(255, 255, 255));

						for (int xx = nOutlineBasePoint; xx <= nOutlineThickness; ++xx)
							for (int yy = nOutlineBasePoint; yy <= nOutlineThickness; ++yy)
						{
							{
								if (xx != 0 && yy != 0)
								{
									rcOutline.left = rcClient.left + xx;
									rcOutline.right = rcClient.right + xx;
									rcOutline.top = rcClient.top + yy;
									rcOutline.bottom = rcClient.bottom + yy;

									DrawText(hDC, strDrawText, -1, &rcOutline, uDrawTextFormat);
								}
							}
						}

						::SetTextColor(hDC, hOlddd);
					}
					DrawText(hDC, strDrawText, -1, &rcClient, uDrawTextFormat);

#ifdef SCREEN_VIEWER_CLIPPING_ALERT
					// 영역체크 20130315 KMK
					// TextBox는 uTextOutFormat 사용하기 때문에 x 대신 rcClient.left를 사용해야함
					// 멀티라인일 때
					if ( (rcClient.left + rcTemp.right) > (rcClient.right + NH_INDENT) )
						AfxMessageBox(strDrawText + _T("\n\nWidth Clipped?"));
					if ( (rcClient.top + rcTemp.bottom) > (rcClient.bottom + NH_INDENT) )
						AfxMessageBox(strDrawText + _T("\n\nHeight Clipped?"));
#endif

				}
				else
				{
					int x;
					
					// Get Align
					if (m_Align == CENTER)
					{
						x = rcClient.left;
						x += (rcClient.right - rcClient.left)/2; 
						uTextOutFormat = TA_CENTER | TA_TOP | TA_NOUPDATECP;
					}
					else if (m_Align == RIGHT)
					{
						x = rcClient.right;
						uTextOutFormat = TA_RIGHT | TA_TOP | TA_NOUPDATECP;
					}
					else
					{
						x = rcClient.left;
						uTextOutFormat = TA_LEFT | TA_TOP | TA_NOUPDATECP;
					}
					
					int	OldAlign = ::SetTextAlign(hDC, uTextOutFormat);
					
					if (m_nOutlineWidth != 0)
					{
						COLORREF	hOlddd = 0xffffffff;

						if (m_pOutlineColor != NULL)
							hOlddd = ::SetTextColor(hDC, m_pOutlineColor->m_crColor);
						else
							hOlddd = ::SetTextColor(hDC, RGB(255, 255, 255));

						for (int xx = nOutlineBasePoint; xx <= nOutlineThickness; ++xx)
							for (int yy = nOutlineBasePoint; yy <= nOutlineThickness; ++yy)
							{
								if (xx != 0 && yy != 0)
									ExtTextOut(hDC, x+xx, rcClient.top+yy, ETO_CLIPPED, &rcClient, strDrawText, strDrawText.GetLength(), NULL);
							}

						::SetTextColor(hDC, hOlddd);
					}

					ExtTextOut(hDC, x, rcClient.top, ETO_CLIPPED, &rcClient, strDrawText, strDrawText.GetLength(), NULL);
					
					::SetTextAlign(hDC, OldAlign);
					
					if (m_pIcon)
					{
						int startPoint = 0;

						if (m_Align == CENTER)
							startPoint = x - (rcTemp.right/2) - m_nIconWidth;
						else if (m_Align == RIGHT)
							startPoint = x - rcTemp.right - m_nIconWidth;
						else
							startPoint = x - m_nIconWidth;

						startPoint -= (NH_INDENT * 2);

						// 아이콘이라면 그림을 여기서 그리자..
						// [#2529] NH Justin 2018.02.28 Icon : Top Align => Bottom Align
						/*
						m_pIcon->DrawImage(GetLocaleNum(),
							hDC,
							startPoint,						// LEFT
							rcClient.top,					// TOP
							startPoint + m_nIconWidth,		// RIGHT
							rcClient.top + m_nIconHeight);	// BOTTOM
						*/
						m_pIcon->DrawImage(GetLocaleNum(),
							hDC,
							startPoint,						// LEFT
							rcClient.bottom - m_nIconHeight,// TOP
							startPoint + m_nIconWidth,		// RIGHT
							rcClient.bottom);				// BOTTOM
						// End of [#2592]
					}

#ifdef SCREEN_VIEWER_CLIPPING_ALERT
					// 영역체크 20130315 KMK
					// TextBox는 uTextOutFormat 사용하기 때문에 x 대신 rcClient.left를 사용해야함
					// 멀티라인 아닐 때
					if ( (rcClient.left + rcTemp.right) > (rcClient.right + NH_INDENT) )
						AfxMessageBox(strDrawText + _T("\n\nWIDTH Clipped?"));
					if ( (rcClient.top + rcTemp.bottom) > (rcClient.bottom + NH_INDENT) )
						AfxMessageBox(strDrawText + _T("\n\nHEIGHT Clipped?"));
#endif
				}

				::SelectObject(hDC, fontOriginal);
				::DeleteObject(fontFinal);
			}
		}

		// Restore Text Color
		if ((m_bFocus && m_pColor != NULL) || (m_pFocusColor != NULL))
			::SetTextColor(hDC, OldTextColor);
		
		// Restore Font
		if (OldFont)
			::SelectObject(hDC, OldFont);	// CPicassoFont would be returned. DO NOT DeleteObject() it as it's common resource!
		
		// Restore Background Mode
		::SetBkMode(hDC, OldBkMode);

		bRes |= TRUE;
	}

	return bRes;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTextBox
 FUNCTION NAME: CPicassoTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoMultiTextBox::CPicassoMultiTextBox(CTL_TYPE type) : CPicassoShape(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	RECT	rcTemp = {0, 0, 0, 0};

	for (int i = 0; i < MAX_MULTI_STRING; i++)
	{
		m_MultiString[i].pColor = NULL;
		m_MultiString[i].pFocusColor = NULL;
		m_MultiString[i].pFont = NULL;
		m_MultiString[i].pFocusFont = NULL;
		m_MultiString[i].pLocaleText = NULL;
		m_MultiString[i].rcDraw = rcTemp;
	}

	m_bCalcDrawTextRect = FALSE;
	m_Align = LEFT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoMulitTextBox
 FUNCTION NAME: ~CPicassoMulitTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoMultiTextBox::~CPicassoMultiTextBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

void CPicassoMultiTextBox::SetMultiText(int index, LPCTSTR	pText)
{
	if ((index > 0) && (index < MAX_MULTI_STRING))
		m_MultiString[index-1].strText = pText;
}

void CPicassoMultiTextBox::SetMultiLocaleText(int index, CPicassoResource *pLocaleText)
{
	if ((index > 0) && (index < MAX_MULTI_STRING))
	{
		m_MultiString[index-1].pLocaleText = (CPicassoLocaleText*)pLocaleText;
		m_MultiString[index-1].strText = _T("");
	}
}

void CPicassoMultiTextBox::SetMultiFont(int index, CPicassoFont *pNormal, CPicassoFont *pSelect)
{
	if ((index > 0) && (index < MAX_MULTI_STRING))
	{
		if (pNormal)
		{
			if (pNormal->GetTpye() == RES_FONT)
				m_MultiString[index-1].pFont = pNormal;
		}

		if (pSelect)
		{
			if (pSelect->GetTpye() == RES_FONT)
				m_MultiString[index-1].pFocusFont = pSelect;
		}
	}
}

void CPicassoMultiTextBox::SetMultiTextColor(int index, CPicassoColor *pColor, CPicassoColor *pFocusColor)
{
	if ((index > 0) && (index < MAX_MULTI_STRING))
	{
		m_MultiString[index-1].pColor = pColor;
		m_MultiString[index-1].pFocusColor = pFocusColor;
	}
}

void CPicassoMultiTextBox::SetMultiTextAlign(TEXTALIGN alignValue)
{
	m_Align = alignValue;
}




/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoMultiTextBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoMultiTextBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoShape::Initialize();

	m_bCalcDrawTextRect = FALSE;

	return TRUE;
}



/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoMultiTextBox
 FUNCTION NAME: GetDrawText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CPicassoMultiTextBox::GetDrawTextLength()
{
	int		length = 0;
	CString	strTemp;

	for (int i = 0; i < MAX_MULTI_STRING; i++)
	{
		if (m_MultiString[i].pLocaleText)
		{
			strTemp = m_MultiString[i].pLocaleText->GetLocaleText(GetLocaleNum());
		}
		else
		{
			strTemp = m_MultiString[i].strText;
		}

		length += strTemp.GetLength();
	}

	return length;
}

CString	CPicassoMultiTextBox::GetDrawText(int index)
{
	CString	strTemp;

	if (m_MultiString[index].pLocaleText)
	{
		strTemp = m_MultiString[index].pLocaleText->GetLocaleText(GetLocaleNum());
	}
	else
	{
		strTemp = m_MultiString[index].strText;
	}

	return strTemp;
}

RECT CPicassoMultiTextBox::GetDrawTextRect()
{
	RECT	rcRect ={0, 0, 0, 0};
	CString	strTemp;


	return rcRect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoMultiTextBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoMultiTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL		bRes = FALSE;
	CString		strDrawText;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// PARENT CLASS DRAW
	bRes |= CPicassoShape::DrawControl(hDC, rcUpdate);
	
	////////////////////////////////////
	if (GetDrawTextLength() > 0)
	{
		int			OldBkMode = 0;
		CString		strDrawText, strTemp;
		HFONT		OldFont = NULL;
		COLORREF	OldTextColor = 0;
		RECT		rcTextRect ={0, 0, 0, 0};
		int			StartX = 0, StartY = 0;
		RECT		rcClient;
	
		// Adjust Draw Rect
		rcClient = GetDisplayRect();
		rcClient.left += NH_INDENT;
		rcClient.right -= NH_INDENT;

		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);

		// get length of all multi string
		if (m_bCalcDrawTextRect == FALSE)
		{
			for (int i = 0; i < MAX_MULTI_STRING; i++)
			{
				OldFont = NULL;

				if (m_MultiString[i].pLocaleText)
					strTemp = m_MultiString[i].pLocaleText->GetLocaleText(GetLocaleNum());
				else
					strTemp = m_MultiString[i].strText;

				if (strTemp.GetLength() == 0)
					continue;

				// Select Font
				if (m_MultiString[i].pFont)
					OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_MultiString[i].pFont->GetHandle());

				
				DrawText(hDC, strTemp, -1, &(m_MultiString[i].rcDraw), DT_CALCRECT);
				rcTextRect.right += (m_MultiString[i].rcDraw.right - m_MultiString[i].rcDraw.left);

				// Restore Font
				if (OldFont != NULL)
					::SelectObject(hDC, OldFont);
			}

			m_bCalcDrawTextRect = TRUE;
		}

		// Get Align
		if (m_Align == CENTER)
		{
			// 창 너비		= 창 우측 좌표값 - 창 좌측 좌표값
			// 텍스트 너비	= 텍스트 우측 좌표값 - 텍스트 좌측 좌표값
			// 가운데시작좌표	= (창너비 - 텍스트너비) / 2
			StartX  = ((rcClient.right - rcClient.left) - (rcTextRect.right - rcTextRect.left))/2;
			StartX += rcClient.left;	// 좌표 보정
		}
		else if (m_Align == RIGHT)
		{
			// 우측 시작좌표	= 창너비 - 텍스트너비
			StartX = (rcClient.right - rcClient.left) - (rcTextRect.right - rcTextRect.left);
			StartX += rcClient.left;	// 좌표 보정
		}
		else	// LEFT
		{
			StartX = rcClient.left;
		}
		
		for (int i = 0; i < MAX_MULTI_STRING; i++)
		{
			OldFont = NULL;

			strDrawText = GetDrawText(i);

			if (strDrawText.GetLength() == 0)
				continue;

			// Select Font
			if (m_bFocus && m_MultiString[i].pFocusFont)
				OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_MultiString[i].pFocusFont->GetHandle());
			else if (m_MultiString[i].pFont)
				OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_MultiString[i].pFont->GetHandle());

			// Select Color
			if (m_bFocus && m_MultiString[i].pColor != NULL)
				OldTextColor = ::SetTextColor(hDC, m_MultiString[i].pColor->m_crColor);
			else if (m_MultiString[i].pFocusColor != NULL)
				OldTextColor = ::SetTextColor(hDC, m_MultiString[i].pFocusColor->m_crColor);

			{
				int	OldAlign = ::SetTextAlign(hDC, TA_TOP | TA_NOUPDATECP);

				// 세로 위치 가운데로 조정
				StartY  = (rcClient.bottom - rcClient.top);
				StartY -= (m_MultiString[i].rcDraw.bottom - m_MultiString[i].rcDraw.top);
				StartY /= 2;
				StartY += rcClient.top;		// 좌표 보정
#ifdef SCREEN_VIEWER_CLIPPING_ALERT
				// 영역체크 20130315 KMK
				if ( (StartX + m_MultiString[i].rcDraw.right) > (rcClient.right + NH_INDENT) )
					AfxMessageBox(m_MultiString[i].strText + "\n\nWIDTH Clipped ?");
				if ( (rcClient.top + m_MultiString[i].rcDraw.bottom) > (rcClient.bottom + NH_INDENT) )
					AfxMessageBox(strDrawText + _T("\n\nHEIGHT Clipped?"));
				// end
#endif

				ExtTextOut(hDC, StartX, StartY, ETO_CLIPPED, &rcClient, strDrawText, strDrawText.GetLength(), NULL);

				StartX += (m_MultiString[i].rcDraw.right - m_MultiString[i].rcDraw.left);

				::SetTextAlign(hDC, OldAlign);
			}

			// Restore Close
			if ((m_bFocus && m_MultiString[i].pColor != NULL) || (m_MultiString[i].pFocusColor != NULL))
				::SetTextColor(hDC, OldTextColor);

			// Restore Font
			if (OldFont != NULL)
				::SelectObject(hDC, OldFont); 

		}
		
		// Restore Background Mode
		::SetBkMode(hDC, OldBkMode);

		bRes |= TRUE;
	}

	return bRes;
}

//------------------------------------------------------------------
//	CPicassoHideTextBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoHideTextBox
 FUNCTION NAME: CPicassoHideTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoHideTextBox::CPicassoHideTextBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	m_bShow = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoHideTextBox
 FUNCTION NAME: ~CPicassoHideTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoHideTextBox::~CPicassoHideTextBox()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoHideTextBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoHideTextBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nScaleUnit = SCALE_INIT;
	m_bBlinking = FALSE;
	m_nBlinkCount = 0;

	if (m_bAutoFocus)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoHideTextBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoHideTextBox::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		m_bShow = TRUE;

		if ((m_Act.GetKind() == REQ_FSCMD) && (m_Act.m_Data.GetLength() == 0))
		{
			AddRequest(m_Act.GetKind(), m_Act.GetName(), m_strText);
		}
		// [#75] NH AIREAT 2008.04.07 Full FS Command 추가
		else if (m_Act.GetKind() == REQ_FULLFSCMD)
		{
			CString strData;

			strData = m_Act.m_Name;
			strData += SCR_RES_DELIMITER;
			
			if (m_Act.m_Data.GetLength() == 0)
				strData += m_strText;
			else
				strData += m_Act.m_Data;

			AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
		}
		// end of [#75]
		else		
			AddRequest(m_Act);

		AddInvalidRect(GetDisplayRect());
	}
	else
		m_bShow = FALSE;

	AddInvalidRect(GetDisplayRect());

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoHideTextBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoHideTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	if (m_bShow)
	{
		return CPicassoTextBox::DrawControl(hDC, rcUpdate);
	}

	return FALSE;
}

//------------------------------------------------------------------
//	CPicassoEffectTextBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEffectTextBox
 FUNCTION NAME: CPicassoEffectTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEffectTextBox::CPicassoEffectTextBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_EffectType = EFFECT_TYPING_CENTER;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEffectTextBox
 FUNCTION NAME: ~CPicassoEffectTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEffectTextBox::~CPicassoEffectTextBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEffectTextBox
 FUNCTION NAME: SetEffectType()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEffectTextBox::SetEffectType(EFFECTTYPE type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_EffectType = type;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEffectTextBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEffectTextBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoShape::Initialize();

	if (m_strName.GetLength() > 0)
	{
		m_pLocaleText = NULL;
		m_strText = _T("");
	}

	m_rcDrawText.left = 0;
	m_rcDrawText.top = 0;
	m_rcDrawText.right = 0;
	m_rcDrawText.bottom = 0;

	m_curPos.x = 0;
	m_curPos.y = 0;

	m_widthDrawText = 0;
	m_heightDrawText = 0;
	m_moveWidth = 0;
	m_ShowTextCount = 0;

	m_uTextOutFormat = TA_LEFT | TA_TOP | TA_NOUPDATECP;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEffectTextBox
 FUNCTION NAME: SetBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEffectTextBox::SetBlinking(void)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	AddInvalidRect(GetDisplayRect());

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEffectTextBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEffectTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL		bRes = FALSE;
	CString		strDrawText;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// PARENT CLASS DRAW
	bRes |= CPicassoShape::DrawControl(hDC, rcUpdate);
	
	// Get to Draw Text string
	strDrawText = GetDrawText();

	////////////////////////////////////
	if (strDrawText.GetLength())
	{
		int			OldBkMode;
		HFONT		OldFont = NULL;
//		COLORREF	OldTextColor;
		COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
		
		RECT DisplayRect = GetDisplayRect();

		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		
		// Select Font
		if (m_bFocus && m_pFocusFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFocusFont->GetHandle());
		else if (m_pFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFont->GetHandle());
		
		if (m_bFocus && m_pColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pColor->m_crColor);
		else if (m_pFocusColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pFocusColor->m_crColor);
				
		if (m_moveWidth == 0)
		{
			// Calc Rect for draw Text
			DrawText(hDC, strDrawText, -1, &m_rcDrawText, DT_CALCRECT);

			m_widthDrawText = m_rcDrawText.right - m_rcDrawText.left; 
			m_heightDrawText = m_rcDrawText.bottom - m_rcDrawText.top;
			int orgHeight = DisplayRect.bottom - DisplayRect.top;
			int orgWidth = DisplayRect.right - DisplayRect.left;

			// Calc move amount
			m_moveWidth = (int)(m_widthDrawText / strDrawText.GetLength());
			if (m_moveWidth <= 0)
				m_moveWidth = 2;

			// x position
			if (m_EffectType == EFFECT_SLIDING_LEFT)
				m_curPos.x = DisplayRect.right;
			else if (m_EffectType == EFFECT_SLIDING_RIGHT)
				m_curPos.x = DisplayRect.left - m_widthDrawText;
			else if (m_EffectType == EFFECT_TYPING_LEFT)
				m_curPos.x = DisplayRect.left + NH_INDENT;
			else
				m_curPos.x = DisplayRect.left + ((DisplayRect.right - DisplayRect.left) / 2);

			if (m_EffectType == EFFECT_TYPING_LEFT)
				m_uTextOutFormat = TA_LEFT | TA_TOP | TA_NOUPDATECP;
			else if (m_EffectType == EFFECT_TYPING_CENTER)
				m_uTextOutFormat = TA_CENTER | TA_TOP | TA_NOUPDATECP;

			// y position
			m_curPos.y = DisplayRect.top;
			if (m_heightDrawText < orgHeight)
				m_curPos.y += (int)((orgHeight - m_heightDrawText) / 2);
		}

		int	OldAlign = ::SetTextAlign(hDC, m_uTextOutFormat);
		
		if (m_EffectType == EFFECT_SLIDING_LEFT)
		{
			ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &DisplayRect, strDrawText, strDrawText.GetLength(), NULL);

			if ((m_curPos.x + m_widthDrawText) < DisplayRect.left)
				m_curPos.x = DisplayRect.right;

			m_curPos.x -= m_moveWidth;
		}
		else if (m_EffectType == EFFECT_SLIDING_RIGHT)
		{
			ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &DisplayRect, strDrawText, strDrawText.GetLength(), NULL);

			if (m_curPos.x > DisplayRect.right)
				m_curPos.x = DisplayRect.left - m_widthDrawText;

			m_curPos.x += m_moveWidth;
		}
		else
		{
			if (m_ShowTextCount > strDrawText.GetLength())
				ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &DisplayRect, strDrawText, strDrawText.GetLength(), NULL);
			else
				ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &DisplayRect, strDrawText, m_ShowTextCount, NULL);

			if (m_ShowTextCount > strDrawText.GetLength() + 3)
				m_ShowTextCount = 0;

			m_ShowTextCount++;
		}

		if (m_pColor != NULL || m_pFocusColor != NULL)
			::SetTextAlign(hDC, OldAlign);

		// Restore Text Color
		if ((m_bFocus && m_pColor != NULL) || (m_pFocusColor != NULL))
			::SetTextColor(hDC, OldTextColor);
		
		// Restore Font
		if (OldFont)
			::SelectObject(hDC, OldFont);
		
		// Restore Background Mode
		::SetBkMode(hDC, OldBkMode);

		bRes |= TRUE;
	}

	return bRes;
}


//------------------------------------------------------------------
//	CPicassoEventHandler Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: CPicassoEditTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEditTextBox::CPicassoEditTextBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	m_bAutoRun = FALSE;
	m_nMaxInputChar = 0;
	m_nMinInputChar = 0;
	m_eEditBoxType = EDIT_BOX_NORMAL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: ~CPicassoEditTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEditTextBox::~CPicassoEditTextBox()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEditTextBox::Initialize(void)
{
	CPicassoShape::Initialize();

	m_strText = _T("");
	m_strTemp = _T("");
	m_nMaxInputChar = m_nMaxInputCharOrigin;
	
	if (m_eEditBoxType != EDIT_BOX_NORMAL)
	{
		m_eEditBoxType = EDIT_BOX_NORMAL;
		SetTextType(TTYPE_TEXT);
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEditTextBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		if (!_tcsncmp(strData, DES_AP_TEXT_ID, 9) && (strData.GetLength() > 9))
		{
			CString strTextID = strData.Mid(9);
			strTextID.MakeUpper();		// KSK 2012.12.21

			//if (m_pResourcesQueue != NULL)
			if (m_pResourcesQueueCMap != NULL)
			{
				CPicassoResource *pResource = NULL;
				//if (m_pResourcesQueue->FindDataOfList(strTextID, &pResource))
				if (m_pResourcesQueueCMap->Lookup(strTextID, pResource) == TRUE)
					SetLocaleText(pResource);
			}
		}
		else if (!_tcsncmp(strData, DES_MAX_IN_CHAR, 10) && (strData.GetLength() > 10))
		{
			int nMaxInChar;

			nMaxInChar = _ttoi(strData.Mid(10));
			if (nMaxInChar > 0)
				m_nMaxInputChar = nMaxInChar;
		}
		else if (!_tcsncmp(strData, DES_EDIT_BOX_TYPE, 12) && (strData.GetLength() > 12))
		{
			CString strType;

			strType = strData.Mid(12);

			if (strType == (_T("NORMAL")))
				m_eEditBoxType = EDIT_BOX_NORMAL;
			else if (strType == (_T("CENT")))
				m_eEditBoxType = EDIT_BOX_CENT;
			else if (strType == (_T("DATE")))
				m_eEditBoxType = EDIT_BOX_DATE;
			else if (strType == (_T("PASSWORD")))
			{
				SetTextType(TTYPE_PASSWORD);
				m_eEditBoxType = EDIT_BOX_PASSWORD;
			}
			// [#2316] US Justin 2014.12.17 Add Decimal Edit
			else if (strType == (_T("DECIMAL")))
				m_eEditBoxType = EDIT_BOX_DECIMAL;
			// End of [#2316]
		}
		else
		{
			m_strText = strData;
			m_pLocaleText = NULL;
		}
		
		AddInvalidRect(GetDisplayRect());
		
		return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: MouseEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEditTextBox::MouseEvent(POINT &point)
{
	if (m_bFocus)
	{
		return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEditTextBox::KeyEvent(CString &strKey)
{
	if (m_bFocus)
	{
		// convert key
		if (strKey == _T("JUM") || (strKey == DES_STAR))
			strKey = _T(".");
		else if (strKey == _T("COMA") || (strKey == DES_SHARP))
			strKey = _T(",");

		if (m_eEditBoxType == EDIT_BOX_DATE)
		{
			// ENTER KEY
			if (strKey == DES_ENTER)
			{
				// 날짜는 꼬옥 10자 이어야 한다.
				if (m_strText.GetLength() != 10)
				{
					return TRUE;
				}

				if (m_Act.GetKind() != REQ_NONE)
				{
					if (m_Act.GetKind() == REQ_FSCMD)
					{
						if (m_Act.m_Data.GetLength() == 0)
							AddRequest(m_Act.GetKind(), m_Act.GetName(), m_strText);
						else
							AddRequest(m_Act);
					}
					// [#75] NH AIREAT 2008.04.07 Full FS Command 추가
					else if (m_Act.GetKind() == REQ_FULLFSCMD)
					{
						CString strData;
						
						strData = m_Act.m_Name;
						strData += SCR_RES_DELIMITER;
						
						if (m_Act.m_Data.GetLength() == 0)
							strData += m_strText;
						else
							strData += m_Act.m_Data;
						
						AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
					}
					// end of [#75]
					else if (m_Act.GetKind() == REQ_KEYEVENT)
					{
						AddRequest(m_Act.GetKind(), NULL, m_strText);
					}
					else
						AddRequest(m_Act);
				}
				
				// kill focus.
				if (!m_bAutoFocus)
					m_bFocus = FALSE;
				
				AddInvalidRect(GetDisplayRect());
				return TRUE;
			}
			// CLEAR KEY
			else if (strKey == DES_CLEAR)
			{
				if (m_strText.GetLength() > 1)
				{
					int ii, nRemove;
					for (ii=0, nRemove=0; ii < m_strText.GetLength(); ii++)
					{
						if (m_strText.Right(1) == _T("/"))
							m_strText = m_strText.Left(m_strText.GetLength()-1);
						else
						{
							if (nRemove >= 1)
								break;

							m_strText = m_strText.Left(m_strText.GetLength()-1);
							nRemove++;
						}
					}
				}
				else
					m_strText = "";

				AddInvalidRect(GetDisplayRect());
				
				return TRUE;
			}
			// numeric key.
			//else if ((strKey.GetLength() == 1) && (strKey != DES_CANCEL))	// [#470] NH AIREAT 2008.12.22 : '.' 입력 막음.
			else if (strKey.GetLength() == 1)									// [#470] NH AIREAT 2008.12.22 : '.' 입력 막음.
			{
				int nCnt = m_strText.GetLength();
				if ( nCnt < 10)
				{
					nCnt = m_strText.GetLength();
	#if DATE_YYYYMMDD
					if (nCnt == 4 || nCnt == 7)		m_strText += _T("/");
	#elif DATE_MMDDYYYY
					if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
	#elif DATE_DDMMYYYY
					if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
	#else
					if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
	#endif

					m_strText += strKey;

					nCnt = m_strText.GetLength();
	#if DATE_YYYYMMDD
					if (nCnt == 4 || nCnt == 7)		m_strText += _T("/");
	#elif DATE_MMDDYYYY
					if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
	#elif DATE_DDMMYYYY
					if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
	#else
					if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
	#endif

					AddInvalidRect(GetDisplayRect());
				}
				
				return TRUE;
			}
			// [#236] NZ AIREAT 2008.06.04 - TIMEOUT
			else if (strKey == DES_TIMEOVER)
			{
				return FALSE;
			}
			// end of [#236]
			else if (m_Act.GetKind() != REQ_KEYEVENT)
			{
				if (strKey == DES_CANCEL)
				{
					m_strText = m_strTemp;
					m_bFocus = FALSE;
					
					AddInvalidRect(GetDisplayRect());
				}

				return TRUE;
			}
		}
		else
		{
			if (strKey == DES_ENTER)
			{
				if( m_nMinInputChar > 0 && m_strText.GetLength() < m_nMinInputChar)
				{
					return TRUE;
				}

				if (m_Act.GetKind() != REQ_NONE)
				{
					if (m_Act.GetKind() == REQ_FSCMD)
					{
						if (m_Act.m_Data.GetLength() == 0)
							AddRequest(m_Act.GetKind(), m_Act.GetName(), m_strText);
						else
							AddRequest(m_Act);
					}
					else if (m_Act.GetKind() == REQ_FULLFSCMD)
					{
						CString strData;
						
						strData = m_Act.m_Name;
						strData += SCR_RES_DELIMITER;
						
						if (m_Act.m_Data.GetLength() == 0)
							strData += m_strText;
						else
							strData += m_Act.m_Data;
						
						AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
					}
					else if (m_Act.GetKind() == REQ_KEYEVENT)
					{
						if (m_strText.GetLength() == 1)
							m_Act.m_Data = _T("0") + m_strText;
						else
							m_Act.m_Data = m_strText;

						//m_strText = _T("");

						AddRequest(m_Act.GetKind(), NULL, m_Act.m_Data);
					}
					else
						AddRequest(m_Act);
				}

				// kill focus.
				if (!m_bAutoFocus)
					m_bFocus = FALSE;
				
				AddInvalidRect(GetDisplayRect());
				return TRUE;
			}
			// CLEAR KEY
			else if (strKey == DES_CLEAR)
			{
				if (m_TextType == TTYPE_PASSWORD)
				{
					m_strText = _T("");
				}
				else
				{
					if (m_strText.GetLength() > 1)
					{
						m_strText = m_strText.Left(m_strText.GetLength()-1);	// [#18]
					}
					else
						m_strText = _T("");
				}

				AddInvalidRect(GetDisplayRect());
				return TRUE;
			}
			// numeric key.
			else if (strKey.GetLength() == 1)
			{
				if (m_strText.GetLength() < m_nMaxInputChar)
				{
					m_strText += strKey;
					AddInvalidRect(GetDisplayRect());		
				}
					// Shift effect. - only support #901
				else if ((m_strText.GetLength()+1) > m_nMaxInputChar && m_Act.GetKind() == REQ_KEYEVENT)
				{
					m_strText += strKey;
					m_strText = m_strText.Right(m_nMaxInputChar);
					AddInvalidRect(GetDisplayRect());
				}

				// Auto Run..
				if (m_strText.GetLength() == m_nMaxInputChar && m_bAutoRun)
				{
					AddRequest(REQ_KEYEVENT, _T(""), DES_ENTER);
				}

				return TRUE;
			}
			else if (strKey == DES_TIMEOVER)
			{
				return FALSE;
			}
			else if ((m_Act.GetKind() != REQ_KEYEVENT) && !m_bAutoFocus)
			{
				if (strKey == DES_CANCEL)
				{
					m_strText = m_strTemp;
					m_bFocus = FALSE;
					
					AddInvalidRect(GetDisplayRect());
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CPicassoEditTextBox::GetDrawText()
{
	switch (m_eEditBoxType)
	{
	case EDIT_BOX_CENT:
	case EDIT_BOX_DECIMAL:			// [#2316] US Justin 2014.12.17 Support Decimal Editting
		{
			CString strDrawText;

			// 000 -> 0
			// 00321 -> 321
			// 320 -> 320
			if (Asc2Int(m_strText) == 0)
			{
				m_strText = _T("");
			}

			int	RightValue = 0;
			if (m_strText.GetLength() <= 2)
			{
				// 0 -> 0.00
				// 10 -> 0.10
				RightValue = Asc2Int(m_strText.Right(2));
				// [#2316] Justin Support Decimal Editting
				if(m_eEditBoxType==EDIT_BOX_DECIMAL)
					strDrawText.Format(_T("0.%02d"), RightValue);
				else
				{

				/////////////////////////////////////////////////////////////////////
				// [#2130] MX PCS 2012.04.19
#if (MX_VERSION)
				// [#2137] MX KSK 2012.07.25
				//1. MX Multi-Currency일 경우, US 달러가 아닌 MX 달러가 나와야 한다.
				//2. 단, 602번 화면에서만 소스와 같이 나타나야 한다.
//				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE) == 1)
//					strDrawText.Format(_T("%s 0.%02d"),CURRENCY_SYMBOL , RightValue);
//				else //2.MX Pesos 방출의 경우 단위가 페소로 나오면 된다.
//					strDrawText.Formt(_T("%s 0.%02d"), GetCurrencySymbol(), RightValue);

				strDrawText.Format(_T("%s 0.%02d"), SCREEN_CURRENCY, RightValue);
				// End of [#2316]
				// end of [#2137]
#else
				strDrawText.Format(_T("%s 0.%02d"), GetCurrencySymbol(), RightValue);
#endif
				}
				//end of [#2130]
			}
			else
			{
				int Len = 0;
				CString		Temp = _T("");
				// 12345 -> 123.45
				// 123456 -> 1,234.56
				RightValue = Asc2Int(m_strText.Right(2));

				// [#2316] Justin Support Decimal Editting
				if(m_eEditBoxType==EDIT_BOX_DECIMAL)
					strDrawText = _T("");
				else
				{				
				//////////////////////////////////////////////////////////////////////
				// [#2130] MX PCS 2012.04.19
#if (MX_VERSION)
				// [#2137] MX KSK 2012.07.25
				//1. MX Multi-Currency일 경우, US 달러가 아닌 MX 달러가 나와야 한다.
//				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE) == 1)
//					strDrawText.Format(_T("%s "), CURRENCY_SYMBOL);
//				else //2.MX Pesos 방출의 경우 단위가 페소로 나오면 된다.
//					strDrawText.Format(_T("%s "), GetCurrencySymbol());
				strDrawText.Format(_T("%s "), SCREEN_CURRENCY);
				// end of [#2137]
#else
				strDrawText.Format(_T("%s "), GetCurrencySymbol());
#endif
				// end of [#2130]
				}

				//////////////////////////////////////////////////////////////////////
				Temp = m_strText.Left(m_strText.GetLength()-2);
				Len = Temp.GetLength();

				for (int i=Len; i>0; i--)
				{
					if (((i % 3) == 0) && (i != Len))		strDrawText += ",";
					strDrawText += Temp.GetAt(Len-i);
				}

				// add cent.
				Temp.Format(_T(".%02d"), RightValue);

				strDrawText += Temp;

			}

			return strDrawText;
		}
		break;
	}

	return CPicassoTextBox::GetDrawText();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: MakeAllFSCMD()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEditTextBox::MakeAllFSCMD(CString &Data)
{
	Data += SCR_RES_DELIMITER + m_strText;
	
//	m_strText = _T("");
				
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEditTextBox::SetCtlFocus(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		m_bFocus = TRUE;
	
		m_strTemp = m_strText;
	}
	else
	{
		m_bFocus = FALSE;
	}

	AddInvalidRect(GetDisplayRect());
		
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: SetMaxInputChar()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEditTextBox::SetMaxInputChar(int nMaxChar)
{
	m_nMaxInputCharOrigin = nMaxChar;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: SetMinInputChar()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEditTextBox::SetMinInputChar(int nChar)
{
	 m_nMinInputChar = nChar;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEditTextBox
 FUNCTION NAME: SetAutoRun()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoEditTextBox::SetAutoRun(BOOL bAutoRun)
{
	m_bAutoRun = bAutoRun;
}

///////////////////////////////////////////////////////////////
//
//	CPicassoWeatherBox
//
/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: CPicassoWeatherBox()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoWeatherBox::CPicassoWeatherBox(CTL_TYPE type)
{
	int i;
	m_pWeatherBackImage[0] = NULL;
	m_pWeatherBackImage[1] = NULL;

	for (i = 0; i < MAX_WEATHER_CODE; i++)
	{
		m_pWeatherCodeImage[i] = NULL;
	}

	memset(&m_rcBack, 0, sizeof(m_rcBack));

	for (i = 0; i < MAX_WEATHER_DATA; i++)
	{
		memset(&m_rcTitle[i], 0, sizeof(m_rcTitle[i]));
		memset(&m_rcDate[i], 0, sizeof(m_rcDate[i]));
		memset(&m_rcWeather[i], 0, sizeof(m_rcWeather[i]));
	}

	for (i = 0; i < 4; i++)
	{
		memset(&m_rcTemperature[i], 0, sizeof(m_rcTemperature[i]));
	}

	m_pNormalFont = NULL;
	m_pAccentFont = NULL;

	m_pNormalColor = NULL;
	m_pAccentColor = NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: ~CPicassoWeatherBox()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoWeatherBox::~CPicassoWeatherBox()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetBackImage()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetBackImage(BOOL bCelsius, CPicassoResource *pImg)
{
	if (bCelsius == TRUE)
		m_pWeatherBackImage[0] = (CPicassoPicture*)pImg;
	else
		m_pWeatherBackImage[1] = (CPicassoPicture*)pImg;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetWeatherCodeImage()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetWeatherCodeImage(int nCode, CPicassoResource *pImg)
{
	if (nCode >= 0 && nCode < MAX_WEATHER_CODE)
	{
		m_pWeatherCodeImage[nCode] = (CPicassoPicture*)pImg;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetBackRect()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetBackRect(int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	m_rcBack = GetAdjustScreenReate(left, top, width, height, nLoadWidth, nLoadHeight);

	CRect rcTemp(m_rcBack);
	CRect rcTemp1(m_rcDisplay);
	CRect rcUnion;
	rcUnion.UnionRect(&rcTemp, &rcTemp1);
	m_rcDisplay = (RECT)rcUnion;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetDateRect()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetTitleRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	if (nIndex >= 0 && nIndex < MAX_WEATHER_DATA)
	{
		m_rcTitle[nIndex] = GetAdjustScreenReate(left, top, width, height, nLoadWidth, nLoadHeight);

		CRect rcTemp(m_rcTitle[nIndex]);
		CRect rcTemp1(m_rcDisplay);
		CRect rcUnion;
		rcUnion.UnionRect(&rcTemp, &rcTemp1);
		m_rcDisplay = (RECT)rcUnion;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetDateRect()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetDateRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	if (nIndex >= 0 && nIndex < MAX_WEATHER_DATA)
	{
		m_rcDate[nIndex] = GetAdjustScreenReate(left, top, width, height, nLoadWidth, nLoadHeight);

		CRect rcTemp(m_rcDate[nIndex]);
		CRect rcTemp1(m_rcDisplay);
		CRect rcUnion;
		rcUnion.UnionRect(&rcTemp, &rcTemp1);
		m_rcDisplay = (RECT)rcUnion;	
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetWeatherRect()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetWeatherRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	if (nIndex >= 0 && nIndex < MAX_WEATHER_DATA)
	{
		m_rcWeather[nIndex] = GetAdjustScreenReate(left, top, width, height, nLoadWidth, nLoadHeight);

		CRect rcTemp(m_rcWeather[nIndex]);
		CRect rcTemp1(m_rcDisplay);
		CRect rcUnion;
		rcUnion.UnionRect(&rcTemp, &rcTemp1);
		m_rcDisplay = (RECT)rcUnion;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetTemperatureRect()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetTemperatureRect(int nIndex, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	if (nIndex >= 0 && nIndex < 4)
	{
		m_rcTemperature[nIndex] = GetAdjustScreenReate(left, top, width, height, nLoadWidth, nLoadHeight);

		CRect rcTemp(m_rcTemperature[nIndex]);
		CRect rcTemp1(m_rcDisplay);
		CRect rcUnion;
		rcUnion.UnionRect(&rcTemp, &rcTemp1);
		m_rcDisplay = (RECT)rcUnion;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetNormalFontColor()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetNormalFontColor(CPicassoResource *pFont, CPicassoResource *pColor)
{
	m_pNormalFont = (CPicassoFont*)pFont;
	m_pNormalColor = (CPicassoColor*)pColor;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetAccentFontColor()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoWeatherBox::SetAccentFontColor(CPicassoResource *pFont, CPicassoResource *pColor)
{
	m_pAccentFont = (CPicassoFont*)pFont;
	m_pAccentColor = (CPicassoColor*)pColor;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoWeatherBox::Initialize(void)
{
	for (int i = 0; i < MAX_WEATHER_DATA; i++)
	{
		m_sWeatherData[i].nYear = -1;
		m_sWeatherData[i].nMonth = -1;
		m_sWeatherData[i].nDay = -1;
		m_sWeatherData[i].strDayOfWeek = L"";
		m_sWeatherData[i].bCelsius = FALSE;
		m_sWeatherData[i].nMax = 0;
		m_sWeatherData[i].nMin = 0;
	}

	m_bShow = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoWeatherBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if ((_tcsncmp(strName, DES_AP_WEATHER, 9) == 0) && strName.GetLength() >= 10)
	{
		int				nIndex;
		CStringArray	arWeatherData;

		nIndex = Asc2Int(strName.Right(1));
		SplitString(strData, '|', arWeatherData);

		// Today
		if (nIndex == 1)
		{
			if (arWeatherData.GetSize() >= 8)
			{
				m_sWeatherData[0].nYear = Asc2Int(arWeatherData[0]);
				m_sWeatherData[0].nMonth = Asc2Int(arWeatherData[1]);
				m_sWeatherData[0].nDay = Asc2Int(arWeatherData[2]);
				m_sWeatherData[0].strDayOfWeek = arWeatherData[3];
				m_sWeatherData[0].nWeatherCode = Asc2Int(arWeatherData[4]);
				if (arWeatherData[5] == _T("C"))
					m_sWeatherData[0].bCelsius = TRUE;
				else
					m_sWeatherData[0].bCelsius = FALSE;
				m_sWeatherData[0].nMax = Asc2Int(arWeatherData[6]);
				m_sWeatherData[0].nMin = Asc2Int(arWeatherData[7]);

				m_bShow = TRUE;
			}
		}
		// Tomorrow
		else if (nIndex == 2)
		{
			if (arWeatherData.GetSize() >= 8)
			{
				m_sWeatherData[1].nYear = Asc2Int(arWeatherData[0]);
				m_sWeatherData[1].nMonth = Asc2Int(arWeatherData[1]);
				m_sWeatherData[1].nDay = Asc2Int(arWeatherData[2]);
				m_sWeatherData[1].strDayOfWeek = arWeatherData[3];
				m_sWeatherData[1].nWeatherCode = Asc2Int(arWeatherData[4]);
				if (arWeatherData[5] == _T("C"))
					m_sWeatherData[1].bCelsius = TRUE;
				else
					m_sWeatherData[1].bCelsius = FALSE;
				m_sWeatherData[1].nMax = Asc2Int(arWeatherData[6]);
				m_sWeatherData[1].nMin = Asc2Int(arWeatherData[7]);

				m_bShow = TRUE;
			}
		}

		AddInvalidRect(GetDisplayRect());

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoWeatherBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2010.04.19)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoWeatherBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	if (m_bShow == TRUE)
	{
		// DRAW BACK
		{
			CPicassoPicture *pImg;

			if (m_sWeatherData[0].bCelsius == TRUE)
				pImg = m_pWeatherBackImage[0];
			else
				pImg = m_pWeatherBackImage[1];
		
			if (pImg != NULL)
				pImg->DrawImage(GetLocaleNum(), hDC, m_rcBack.left, m_rcBack.top, m_rcBack.right, m_rcBack.bottom);
		}
		
		for(int i=0; i < MAX_WEATHER_DATA; i++)
		{
			// DRAW WEATHER INFORMATION
			int nWeatherCode = m_sWeatherData[i].nWeatherCode;
			if (nWeatherCode < MAX_WEATHER_CODE && nWeatherCode >= 0)
				m_pWeatherCodeImage[nWeatherCode]->DrawImage(GetLocaleNum(), 
															 hDC, 
															 m_rcWeather[i].left, 
															 m_rcWeather[i].top, 
															 m_rcWeather[i].right, 
															 m_rcWeather[i].bottom);

			// DRAW DATE
			{
				CString strTitle;

				if (i == 0)
					strTitle = _T("Today");
				else
					strTitle = _T("Tomorrow");

				PrintText(hDC, strTitle, DT_CENTER, &m_rcTitle[i], m_pAccentFont, m_pAccentColor);
			}

			// DRAW DATE
			COleDateTime dt;
			if (dt.SetDate(m_sWeatherData[i].nYear, m_sWeatherData[i].nMonth, m_sWeatherData[i].nDay) == 0)
			{
				CString strDate = dt.Format(_T("%B %d, %Y"));
				PrintText(hDC, strDate, DT_CENTER, &m_rcDate[i], m_pNormalFont, m_pNormalColor);
			}

//#ifdef UNDER_CE
			// DRAW TEMPERATURE
			{
				CString strTemper;
				
				strTemper.Format(_T("%d"), m_sWeatherData[i].nMax);
				PrintText(hDC, strTemper, DT_RIGHT, &m_rcTemperature[(i*2)], m_pAccentFont, m_pAccentColor);

				strTemper.Format(_T("%d"), m_sWeatherData[i].nMin);
				PrintText(hDC, strTemper, DT_RIGHT, &m_rcTemperature[(i*2)+1], m_pAccentFont, m_pAccentColor);
			}
//#endif
		}

		bRes = TRUE;
	}

	return bRes;
}

void CPicassoWeatherBox::PrintText(HDC hDC, CString strText, UINT nAlign, RECT *pRect, CPicassoFont *pFont, CPicassoColor *pColor)
{
	int			OldBkMode;
	HFONT		OldFont = NULL;
	COLORREF	OldTextColor = 0xffffffff;

	OldBkMode = ::SetBkMode(hDC, TRANSPARENT);

	if (pFont != NULL)
		OldFont = (HFONT)::SelectObject(hDC, (HFONT)pFont->GetHandle());

	if (pColor != NULL)
		OldTextColor = ::SetTextColor(hDC, pColor->m_crColor);
	
	// Draw Text
	{
		RECT	rcClient;
		RECT	rcTemp;
		int		calcHeight, orgHeight;
		
		// Adjust Draw Rect
		rcClient = *pRect;
		rcClient.left += NH_INDENT;
		rcClient.right -= NH_INDENT;

		// Calc Rect.
		if (strText.GetLength() > 0)
		{
			// Calc Rect for draw Text
			DrawText(hDC, strText, -1, &rcTemp, DT_CALCRECT);
			
			calcHeight = rcTemp.bottom - rcTemp.top;
			orgHeight = rcClient.bottom - rcClient.top;
			
			if (calcHeight < orgHeight)
				rcClient.top += (int)((orgHeight - calcHeight) / 2);
			
			DrawText(hDC, strText, -1, &rcClient, nAlign);
		}
	}

	// Restore Text Color
	if (pColor != NULL)
		::SetTextColor(hDC, OldTextColor);

	// Restore Font
	if (OldFont != NULL)
		::SelectObject(hDC, OldFont);

	// Restore Background Mode
	::SetBkMode(hDC, OldBkMode);
}

///////////////////////////////////////////////////////////////
//
//	CPicassoVKeyBoardBox
//

CString gstrVKeyBoardString[MAX_KEYBOARD_TYPE][MAX_KEYBOARD_ROW][MAX_KEYBOARD_COL] = 
{
	// 0 type : Upper
	{
		{_T("Q"), _T("W"), _T("E"), _T("R"), _T("T"), _T("Y"), _T("U"), _T("I"), _T("O"), _T("P")},
		{_T("A"), _T("S"), _T("D"), _T("F"), _T("G"), _T("H"), _T("J"), _T("K"), _T("L"), _T("CLEAR")},
		{_T("abc"),  _T("Z"), _T("X"), _T("C"), _T("V"), _T("B"), _T("N"), _T("M"), _T("UP"),  _T("")},
		{_T("123.."), _T("SPC"), _T("LEFT"), _T("DOWN"), _T("RIGHT"), _T(""), _T(""), _T(""), _T(""), _T("")}
	},
	// 1 type : Lower
	{
		{_T("q"), _T("w"), _T("e"), _T("r"), _T("t"), _T("y"), _T("u"), _T("i"), _T("o"), _T("p")},
		{_T("a"), _T("s"), _T("d"), _T("f"), _T("g"), _T("h"), _T("j"), _T("k"), _T("l"), _T("CLEAR")},
		{_T("ABC"),  _T("z"), _T("x"), _T("c"), _T("v"), _T("b"), _T("n"), _T("m"), _T("UP"),  _T("")},
		{_T("123.."), _T("SPC"), _T("LEFT"), _T("DOWN"), _T("RIGHT"), _T(""), _T(""), _T(""), _T(""), _T("")}
	},
	// 2 type : Numeric #1
	{
		{_T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9"), _T("0")},
		{_T("~"), _T("!"), _T("@"), _T("#"), _T("$"), _T("%"), _T("^"), _T("&"), _T("*"), _T("CLEAR")},
		{_T("=,/"),  _T("+"), _T("-"), _T(":"), _T(";"), _T("<"), _T(">"), _T("?"), _T("UP"),  _T("")},
		{_T("ABC.."), _T("SPC"), _T("LEFT"), _T("DOWN"), _T("RIGHT"), _T(""), _T(""), _T(""), _T(""), _T("")}
	},
	// 3 type : Numeric #2
	{
		{_T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9"), _T("0")},
		{_T("("), _T(")"), _T("{"), _T("}"), _T("["), _T("]"), _T("_"), _T("="), _T("\""), _T("CLEAR")},
		{_T("~!@"),  _T("'"), _T(","), _T("."), _T("/"), _T("|"), _T("\\"), STR_POUNT_CHAR, _T("UP"),  _T("")},
		{_T("ABC.."), _T("SPC"), _T("LEFT"), _T("DOWN"), _T("RIGHT"), _T(""), _T(""), _T(""), _T(""), _T("")}
	}
};

CPicassoVKeyBoardBox::CPicassoVKeyBoardBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	int	i, j;

	for (i = 0; i < MAX_KEYBOARD_ROW; i++)
	{
		for (j = 0; j < MAX_KEYBOARD_COL; j++)
		{
			m_stKeyBoardButton[i][j].rcRect = CRect(0, 0, 0, 0);

			m_stKeyBoardButton[i][j].bShow = FALSE;
			m_stKeyBoardButton[i][j].pImage = NULL;

			m_stKeyBoardButton[i][j].bDrawText = FALSE;
			m_stKeyBoardButton[i][j].pFont = NULL;
			m_stKeyBoardButton[i][j].pColor = NULL;
		}
	}

	m_bSetButtonPosition = FALSE;

	m_nKeyboardType = 0;
	
	m_nButtonGap = 0;
	m_nLineHeight = 0;

	m_pNormalButtonImage = NULL;
//	m_pControlButtonImage = NULL;
	m_pSpaceButtonImage = NULL;
	m_pUpArrowImage = NULL;
	m_pDownArrowImage = NULL;
	m_pLeftArrowImage = NULL;
	m_pRightArrowImage = NULL;
	m_pClearButtonImage = NULL;
	m_p123ButtonImage = NULL;
	m_pABCButtonImage = NULL;
	m_pCapsButtonImage = NULL;
	m_pSymbol1ButtonImage = NULL;
	m_pSymbol2ButtonImage = NULL;

	m_pNormalButtonFont = NULL;
	m_pNormalButtonTextColor = NULL;

//	m_pControlButtonFont = NULL;
//	m_pControlButtonTextColor = NULL;

	m_stNormalButton.cx = 0;
	m_stNormalButton.cy = 0;
	m_stControlButton.cx = 0;
	m_stControlButton.cy = 0;
	m_stSpaceButton.cx = 0;
	m_stSpaceButton.cy = 0;
	m_stArrowButton.cx = 0;
	m_stArrowButton.cy = 0;

}

CPicassoVKeyBoardBox::~CPicassoVKeyBoardBox()
{
}

void CPicassoVKeyBoardBox::SetButtonGap(int nGap, int nLoadWidth, int nLoadHeight)
{
	m_nButtonGap = (int)((float)nGap * (float)((float)nLoadWidth / (float)BASIC_SCREEN_X));
}

void CPicassoVKeyBoardBox::SetLineHeight(int nHeight, int nLoadWidth, int nLoadHeight)
{
	// [#2300] US KSK 2014.1.04
	if (nLoadWidth == 1024 && nLoadHeight == 768)
		m_nLineHeight = (int)((float)nHeight * (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y));		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
	else
		m_nLineHeight = (int)((float)nHeight * (float)((float)nLoadHeight / (float)BASIC_SCREEN_Y));
	// end of [#2300]
}

void CPicassoVKeyBoardBox::SetNormalButtonImage(CPicassoPicture *pImage)
{
	m_pNormalButtonImage = pImage;
}

void CPicassoVKeyBoardBox::SetNormalButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight)
{
	m_stNormalButton.cx = (int)((float)nWidth* (float)((float)nLoadWidth / (float)BASIC_SCREEN_X));

	// [#2300] US KSK 2014.1.04
	if (nLoadWidth == 1024 && nLoadHeight == 768)
		m_stNormalButton.cy = (int)((float)nHeight* (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y));		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
	else
		m_stNormalButton.cy = (int)((float)nHeight* (float)((float)nLoadHeight / (float)BASIC_SCREEN_Y));
	// end of [#2300]
}

void CPicassoVKeyBoardBox::SetNormalButtonFont(CPicassoFont *pFont)
{
	m_pNormalButtonFont = pFont;
}

void CPicassoVKeyBoardBox::SetNormalButtonTextColor(CPicassoColor *pColor)
{
	m_pNormalButtonTextColor = pColor;
}

//void CPicassoVKeyBoardBox::SetControlButtonImage(CPicassoPicture *pImage)
//{
//	m_pControlButtonImage = pImage;
//}

void CPicassoVKeyBoardBox::SetControlButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight)
{
	m_stControlButton.cx = (int)((float)nWidth* (float)((float)nLoadWidth / (float)BASIC_SCREEN_X));

	// [#2300] US KSK 2014.1.04
	if (nLoadWidth == 1024 && nLoadHeight == 768)
		m_stControlButton.cy = (int)((float)nHeight* (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y));		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
	else
		m_stControlButton.cy = (int)((float)nHeight* (float)((float)nLoadHeight / (float)BASIC_SCREEN_Y));
	// end of [#2300]
}

//void CPicassoVKeyBoardBox::SetControlButtonFont(CPicassoFont *pFont)
//{
//	m_pControlButtonFont = pFont;
//}

//void CPicassoVKeyBoardBox::SetControlButtonTextColor(CPicassoColor *pColor)
//{
//	m_pControlButtonTextColor = pColor;
//}

void CPicassoVKeyBoardBox::SetClearButtonImage(CPicassoPicture *pImage)
{
	m_pClearButtonImage = pImage;
}

void CPicassoVKeyBoardBox::Set123ButtonImage(CPicassoPicture *pImage)
{
	m_p123ButtonImage = pImage;
}

void CPicassoVKeyBoardBox::SetABCButtonImage(CPicassoPicture *pImage)
{
	m_pABCButtonImage = pImage;
}

void CPicassoVKeyBoardBox::SetCapsButtonImage(CPicassoPicture *pImage)
{
	m_pCapsButtonImage = pImage;
}

void CPicassoVKeyBoardBox::SetSymbol1ButtonImage(CPicassoPicture *pImage)
{
	m_pSymbol1ButtonImage = pImage;
}

void CPicassoVKeyBoardBox::SetSymbol2ButtonImage(CPicassoPicture *pImage)
{
	m_pSymbol2ButtonImage = pImage;
}


void CPicassoVKeyBoardBox::SetSpaceButtonImage(CPicassoPicture *pImage)
{
	m_pSpaceButtonImage = pImage;
}

void CPicassoVKeyBoardBox::SetSpaceButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight)
{
	m_stSpaceButton.cx = (int)((float)nWidth* (float)((float)nLoadWidth / (float)BASIC_SCREEN_X));

	// [#2300] US KSK 2014.1.04
	if (nLoadWidth == 1024 && nLoadHeight == 768)
		m_stSpaceButton.cy = (int)((float)nHeight* (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y));		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
	else
		m_stSpaceButton.cy = (int)((float)nHeight* (float)((float)nLoadHeight / (float)BASIC_SCREEN_Y));
	// end of [#2300]
}

void CPicassoVKeyBoardBox::SetUpArrowImage(CPicassoPicture *pImage)
{
	m_pUpArrowImage = pImage;
}

void CPicassoVKeyBoardBox::SetDownArrowImage(CPicassoPicture *pImage)
{
	m_pDownArrowImage = pImage;
}

void CPicassoVKeyBoardBox::SetLeftArrowImage(CPicassoPicture *pImage)
{
	m_pLeftArrowImage = pImage;
}

void CPicassoVKeyBoardBox::SetRightArrowImage(CPicassoPicture *pImage)
{
	m_pRightArrowImage = pImage;
}

void CPicassoVKeyBoardBox::SetArrowButtonSize(int nWidth, int nHeight, int nLoadWidth, int nLoadHeight)
{
	m_stArrowButton.cx = (int)((float)nWidth* (float)((float)nLoadWidth / (float)BASIC_SCREEN_X));

	// [#2300] US KSK 2014.1.04
	if (nLoadWidth == 1024 && nLoadHeight == 768)
		m_stArrowButton.cy = (int)((float)nHeight* (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y));		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
	else
		m_stArrowButton.cy = (int)((float)nHeight* (float)((float)nLoadHeight / (float)BASIC_SCREEN_Y));
	// end of [#2300]
}

BOOL CPicassoVKeyBoardBox::Initialize(void)
{
	int j;

	CPicassoTextBox::Initialize();

	// calc position for each button
	if (m_bSetButtonPosition == FALSE)
	{
		int	nStartX, nStartY;
		int	left, top, right, bottom;

		RECT rcArea = GetDisplayRect();

		// 1 Row
		nStartX = rcArea.left;
		nStartY = rcArea.top;
		top = nStartY;
		bottom = top + m_stNormalButton.cy;
		for (j = 0; j < 10; j++)
		{
			left = nStartX + (j * m_stNormalButton.cx) + (j * m_nButtonGap);
			right = left + m_stNormalButton.cx;
			
			m_stKeyBoardButton[0][j].rcRect = CRect(left, top, right, bottom);

			m_stKeyBoardButton[0][j].bShow = TRUE;
			m_stKeyBoardButton[0][j].pImage = m_pNormalButtonImage;			

			m_stKeyBoardButton[0][j].bDrawText = TRUE;
			m_stKeyBoardButton[0][j].pFont = m_pNormalButtonFont;
			m_stKeyBoardButton[0][j].pColor = m_pNormalButtonTextColor;
		}

		// 2 Row
		nStartX = rcArea.left + (int)(m_stControlButton.cx*0.3);
		nStartY = rcArea.top + m_nLineHeight;
		top = nStartY;
		bottom = top + m_stNormalButton.cy;
		for (j = 0; j < 9; j++)
		{
			left = nStartX + (j * m_stNormalButton.cx) + (j * m_nButtonGap);
			right = left + m_stNormalButton.cx;
			
			m_stKeyBoardButton[1][j].rcRect = CRect(left, top, right, bottom);

			m_stKeyBoardButton[1][j].bShow = TRUE;
			m_stKeyBoardButton[1][j].pImage = m_pNormalButtonImage;			

			m_stKeyBoardButton[1][j].bDrawText = TRUE;
			m_stKeyBoardButton[1][j].pFont = m_pNormalButtonFont;
			m_stKeyBoardButton[1][j].pColor = m_pNormalButtonTextColor;
		}

		// 3 Row
		nStartX = m_stKeyBoardButton[1][1].rcRect.left + + (int)(m_stControlButton.cx*0.2);
		nStartY = m_stKeyBoardButton[1][1].rcRect.top + m_nLineHeight;
		top = nStartY;
		bottom = top + m_stNormalButton.cy;
		for (j = 1; j < 8; j ++)
		{
			left = nStartX + ((j-1) * m_stNormalButton.cx) + ((j-1) * m_nButtonGap);
			right = left + m_stNormalButton.cx;
			
			m_stKeyBoardButton[2][j].rcRect = CRect(left, top, right, bottom);

			m_stKeyBoardButton[2][j].bShow = TRUE;
			m_stKeyBoardButton[2][j].pImage = m_pNormalButtonImage;			

			m_stKeyBoardButton[2][j].bDrawText = TRUE;
			m_stKeyBoardButton[2][j].pFont = m_pNormalButtonFont;
			m_stKeyBoardButton[2][j].pColor = m_pNormalButtonTextColor;
		}

		// CLEAR KEY
		right = m_stKeyBoardButton[0][9].rcRect.right + m_nButtonGap + m_stNormalButton.cx;
		left = right - m_stControlButton.cx;
		top =  m_stKeyBoardButton[0][0].rcRect.top + m_nLineHeight;
		bottom = top + m_stControlButton.cy;		
		m_stKeyBoardButton[1][9].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[1][9].bShow = TRUE;
		m_stKeyBoardButton[1][9].pImage = m_pClearButtonImage;
		m_stKeyBoardButton[1][9].bDrawText = FALSE;

		// SHIFT KEY
		left = rcArea.left;
		top = rcArea.top + (m_nLineHeight * 2);
		right = left + m_stControlButton.cx;
		bottom = top + m_stControlButton.cy;			
		m_stKeyBoardButton[2][0].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[2][0].bShow = TRUE;
		m_stKeyBoardButton[2][0].pImage = m_pCapsButtonImage;
		m_stKeyBoardButton[2][0].bDrawText = FALSE;

		// TYPE CHANGE KEY
		left = rcArea.left;
		top = rcArea.top + (m_nLineHeight * 3);
		right = left + m_stControlButton.cx;
		bottom = top + m_stControlButton.cy;			
		m_stKeyBoardButton[3][0].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[3][0].bShow = TRUE;
		m_stKeyBoardButton[3][0].pImage = m_p123ButtonImage;
		m_stKeyBoardButton[3][0].bDrawText = FALSE;

		// SPACE KEY
		left = m_stKeyBoardButton[2][1].rcRect.left;
		top =  m_stKeyBoardButton[2][1].rcRect.top + m_nLineHeight;
		right = left + m_stSpaceButton.cx;
		bottom = top + m_stSpaceButton.cy;			
		m_stKeyBoardButton[3][1].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[3][1].bShow = TRUE;
		m_stKeyBoardButton[3][1].pImage = m_pSpaceButtonImage;
		m_stKeyBoardButton[3][1].bDrawText = FALSE;

		// UP KEY
		left = m_stKeyBoardButton[0][9].rcRect.left;
		top =  m_stKeyBoardButton[0][9].rcRect.top + (m_nLineHeight*2);
		right = left + m_stArrowButton.cx;
		bottom = top + m_stArrowButton.cy;			
		m_stKeyBoardButton[2][8].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[2][8].bShow = TRUE;
		m_stKeyBoardButton[2][8].pImage = m_pUpArrowImage;
		m_stKeyBoardButton[2][8].bDrawText = FALSE;

		// DOWN KEY
		left = m_stKeyBoardButton[2][8].rcRect.left;
		top =  m_stKeyBoardButton[2][8].rcRect.top + m_nLineHeight;
		right = left + m_stArrowButton.cx;
		bottom = top + m_stArrowButton.cy;			
		m_stKeyBoardButton[3][3].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[3][3].bShow = TRUE;
		m_stKeyBoardButton[3][3].pImage = m_pDownArrowImage;
		m_stKeyBoardButton[3][3].bDrawText = FALSE;

		// LEFT KEY
		left = m_stKeyBoardButton[3][3].rcRect.left - (m_stArrowButton.cx + m_nButtonGap);
		top =  m_stKeyBoardButton[3][3].rcRect.top;
		right = left + m_stArrowButton.cx;
		bottom = top + m_stArrowButton.cy;			
		m_stKeyBoardButton[3][2].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[3][2].bShow = TRUE;
		m_stKeyBoardButton[3][2].pImage = m_pLeftArrowImage;
		m_stKeyBoardButton[3][2].bDrawText = FALSE;

		// RIGHT KEY
		left = m_stKeyBoardButton[3][3].rcRect.left + (m_stArrowButton.cx + m_nButtonGap);
		top =  m_stKeyBoardButton[3][3].rcRect.top;
		right = left + m_stArrowButton.cx;
		bottom = top + m_stArrowButton.cy;			
		m_stKeyBoardButton[3][4].rcRect = CRect(left, top, right, bottom);
		m_stKeyBoardButton[3][4].bShow = TRUE;
		m_stKeyBoardButton[3][4].pImage = m_pRightArrowImage;
		m_stKeyBoardButton[3][4].bDrawText = FALSE;

		m_bSetButtonPosition = TRUE;
	}

	m_nKeyboardType = 0;
	m_stKeyBoardButton[2][0].pImage = m_pCapsButtonImage;
	m_stKeyBoardButton[3][0].pImage = m_p123ButtonImage;

	return TRUE;
}

BOOL CPicassoVKeyBoardBox::MouseEvent(POINT &point)
{
	int i, j;

	for (i = 0; i < MAX_KEYBOARD_ROW; i++)
	{
		for (j = 0; j < MAX_KEYBOARD_COL; j++)
		{
			if (m_stKeyBoardButton[i][j].bShow == TRUE)
			{
				if(m_stKeyBoardButton[i][j].rcRect.PtInRect(point) == TRUE)
				{
					CString strKey = gstrVKeyBoardString[m_nKeyboardType][i][j];
					
					if (strKey.GetLength() > 0)
					{
						if (strKey == _T("abc"))
						{
							m_nKeyboardType = 1;
							m_stKeyBoardButton[2][0].pImage = m_pCapsButtonImage;
							m_stKeyBoardButton[3][0].pImage = m_p123ButtonImage;
							AddInvalidRect(GetDisplayRect());
							AddRequest(REQ_FSCMD, _T("DING"), _T(""));
						}
						else if (strKey == _T("ABC"))
						{
							m_nKeyboardType = 0;
							m_stKeyBoardButton[2][0].pImage = m_pCapsButtonImage;
							m_stKeyBoardButton[3][0].pImage = m_p123ButtonImage;
							AddInvalidRect(GetDisplayRect());
							AddRequest(REQ_FSCMD, _T("DING"), _T(""));
						}
						else if (strKey == _T("=,/"))
						{
							m_nKeyboardType = 3;
							m_stKeyBoardButton[2][0].pImage = m_pSymbol1ButtonImage;
							m_stKeyBoardButton[3][0].pImage = m_pABCButtonImage;
							AddInvalidRect(GetDisplayRect());
							AddRequest(REQ_FSCMD, _T("DING"), _T(""));
						}
						else if (strKey == _T("~!@"))
						{
							m_nKeyboardType = 2;
							m_stKeyBoardButton[2][0].pImage = m_pSymbol2ButtonImage;
							m_stKeyBoardButton[3][0].pImage = m_pABCButtonImage;
							AddInvalidRect(GetDisplayRect());
							AddRequest(REQ_FSCMD, _T("DING"), _T(""));
						}
						else if (strKey == _T("ABC.."))
						{
							m_nKeyboardType = 0;
							m_stKeyBoardButton[2][0].pImage = m_pCapsButtonImage;
							m_stKeyBoardButton[3][0].pImage = m_p123ButtonImage;
							AddInvalidRect(GetDisplayRect());
							AddRequest(REQ_FSCMD, _T("DING"), _T(""));
						}
						else if (strKey == _T("123.."))
						{
							m_nKeyboardType = 2;
							m_stKeyBoardButton[2][0].pImage = m_pSymbol2ButtonImage;
							m_stKeyBoardButton[3][0].pImage = m_pABCButtonImage;
							AddInvalidRect(GetDisplayRect());
							AddRequest(REQ_FSCMD, _T("DING"), _T(""));
						}
						else
						{
							if (strKey == _T("SPC"))
								strKey = _T(" ");

							AddRequest(REQ_FSCMD, _T("DING"), _T(""));
							AddRequest(REQ_KEYEVENT, NULL, strKey);
						}
					}

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL CPicassoVKeyBoardBox::KeyEvent(CString &strKey)
{
	return FALSE;
}

BOOL CPicassoVKeyBoardBox::SetVariable(CString &strName, CString &strData)
{
	return FALSE;
}

BOOL CPicassoVKeyBoardBox::SetBlinking(void)
{
	return FALSE;
}

BOOL CPicassoVKeyBoardBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	int	i, j;
	BOOL	bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	///////////////////////////////////
	// DRAW IMAGE
	{
		for (i = 0; i < MAX_KEYBOARD_ROW; i++)
		{
			for (j = 0; j < MAX_KEYBOARD_COL; j++)
			{
				if (m_stKeyBoardButton[i][j].bShow)
				{
					// Draw Image
					if (m_stKeyBoardButton[i][j].pImage != NULL)
					{
						m_stKeyBoardButton[i][j].pImage->DrawImage(GetLocaleNum(),
																   hDC, 
																   m_stKeyBoardButton[i][j].rcRect.left,
																   m_stKeyBoardButton[i][j].rcRect.top,
																   m_stKeyBoardButton[i][j].rcRect.right,
																   m_stKeyBoardButton[i][j].rcRect.bottom);
						bRes |= TRUE;
					}
				}
			}
		}
	}

	///////////////////////////////////
	// DRAW TEXT
	{
		CString		strDrawText;
		int			OldBkMode;
		HFONT		OldFont = NULL;
		COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책

		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);

		for (i = 0; i < MAX_KEYBOARD_ROW; i++)
		{
			for (j = 0; j < MAX_KEYBOARD_COL; j++)
			{
				strDrawText = gstrVKeyBoardString[m_nKeyboardType][i][j];

				// Draw Text
				if (m_stKeyBoardButton[i][j].bDrawText == TRUE &&
					strDrawText.GetLength() > 0)
				{
					UINT	uTextOutFormat;
					RECT	rcClient;
					RECT	rcTemp;
					
					// Adjust Draw Rect
					rcClient = (RECT)m_stKeyBoardButton[i][j].rcRect;

					// Calc Rect for draw Text
					DrawText(hDC, strDrawText, -1, &rcTemp, DT_CALCRECT);
					
					// Select Font
					if (m_stKeyBoardButton[i][j].pFont != NULL)
						OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_stKeyBoardButton[i][j].pFont->GetHandle());
					
					// Select Text Color
					if (m_stKeyBoardButton[i][j].pColor != NULL)
						OldTextColor = (COLORREF)::SetTextColor(hDC, m_stKeyBoardButton[i][j].pColor->m_crColor);

					{
						int x, y;
						
						// Set Align
						x = rcClient.left + (int)((rcClient.right - rcClient.left) / 2);
						y = rcClient.top + (int)((rcClient.bottom - rcClient.top) / 2);
						//y -= (int)((rcTemp.bottom-rcTemp.top) / 2);
						y -= (int)((rcTemp.bottom-rcTemp.top) * 0.8);

						uTextOutFormat = TA_CENTER | TA_NOUPDATECP;

						
						int	OldAlign = ::SetTextAlign(hDC, uTextOutFormat);
						
						ExtTextOut(hDC, x, y, ETO_CLIPPED, &rcClient, strDrawText, strDrawText.GetLength(), NULL);
						
						::SetTextAlign(hDC, OldAlign);
					}

					// Restore Text Color
					if (m_stKeyBoardButton[i][j].pColor != NULL)
						::SetTextColor(hDC, OldTextColor);

					// Restore Font
					if (m_stKeyBoardButton[i][j].pFont != NULL)
						::SelectObject(hDC, OldFont);
					
					bRes |= TRUE;
				}

			}
		}

		// Restore Background Mode
		::SetBkMode(hDC, OldBkMode);
	}

	return bRes;
}

//------------------------------------------------------------------
//	CPicassoTableEditTextBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: CPicassoTableEditTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTableEditTextBox::CPicassoTableEditTextBox(CTL_TYPE type) : CPicassoShape(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nAllowRow = 4;
	m_nAllowColumn = 40;

	int i;
	for (i = 0; i < 2; i++)
	{
		m_pGuideFont[i] = NULL;
		m_pGuideTextColor[i] = NULL;
	}

	for (i = 0; i < MAX_TYPE; i++)
	{
		m_pElementFont[i] = NULL;
		m_pElementPen[i] = NULL;
		m_pElementBrush[i] = NULL;
		m_pElementTextColor[i] = NULL;
	}

}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: ~CPicassoTableEditTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTableEditTextBox::~CPicassoTableEditTextBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetAllowAmount()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTableEditTextBox::SetAllowAmount(int nRow, int nColumn)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nAllowRow = nRow;
	m_nAllowColumn = nColumn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetGuideFont()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTableEditTextBox::SetGuideFont(CPicassoFont *pNormal, CPicassoFont *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pGuideFont[0] = pNormal;
	m_pGuideFont[1] = pSelect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetGuideTextColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTableEditTextBox::SetGuideTextColor(CPicassoColor *pNormal, CPicassoColor *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pGuideTextColor[0] = pNormal;
	m_pGuideTextColor[1] = pSelect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetElementPen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTableEditTextBox::SetElementPen(CPicassoPen *pDisable, CPicassoPen *pEnable, CPicassoPen *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pElementPen[DISABLE_TYPE] = pDisable;
	m_pElementPen[ENABLE_TYPE] = pEnable;
	m_pElementPen[SELECT_TYPE] = pSelect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetElementFont()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTableEditTextBox::SetElementFont(CPicassoFont *pDisable, CPicassoFont *pEnable, CPicassoFont *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pElementFont[DISABLE_TYPE] = pDisable;
	m_pElementFont[ENABLE_TYPE] = pEnable;
	m_pElementFont[SELECT_TYPE] = pSelect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetElementBrush()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTableEditTextBox::SetElementBrush(CPicassoBrush *pDisable, CPicassoBrush *pEnable, CPicassoBrush *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pElementBrush[DISABLE_TYPE] = pDisable;
	m_pElementBrush[ENABLE_TYPE] = pEnable;
	m_pElementBrush[SELECT_TYPE] = pSelect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetElementTextColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoTableEditTextBox::SetElementTextColor(CPicassoColor *pDisable, CPicassoColor *pEnable, CPicassoColor *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pElementTextColor[DISABLE_TYPE] = pDisable;
	m_pElementTextColor[ENABLE_TYPE] = pEnable;
	m_pElementTextColor[SELECT_TYPE] = pSelect;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableEditTextBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoShape::Initialize();

	int	row, col;

	// Reset
	for (row = 0; row < MAX_TABLE_ROW; row++)
	{
		for (col = 0; col < MAX_TABLE_COL; col++)
		{
			m_TableElement[row][col].nRow = row;
			m_TableElement[row][col].nCol = col;
			m_TableElement[row][col].bAllowSelect = FALSE;
			m_TableElement[row][col].rcRect = CRect(0, 0, 0, 0);
			
			if (row == 0 && col != 0)
			{
				if ((col%10) == 0)
					m_TableElement[row][col].strText.Format(_T("%d"), (col/10));
			}
			else if (row == 1 && col != 0)
			{
				m_TableElement[row][col].strText.Format(_T("%d"), (col%10));
			}
			else if (row > 1 && col == 0)
			{
				m_TableElement[row][col].strText.Format(_T("%d"), row-1);
			}
			else
				m_TableElement[row][col].strText.Empty();
		}
	}

	// get element width, height
	int	nElementWidth = 0, nElementHeight = 0;

	HDC hDC = ::GetDC(NULL);
	if (hDC != NULL)
	{
		TEXTMETRIC	tm;
		HFONT	OldFont = NULL;
		
		// Select Font
		if (m_pElementFont[0] != NULL)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pElementFont[0]->GetHandle());
		
		GetTextMetrics(hDC, &tm);

		// Restore Font
		if (m_pElementFont[0] != NULL)
			::SelectObject(hDC, OldFont);

		nElementWidth = tm.tmAveCharWidth + (tm.tmExternalLeading * 5);
		nElementHeight = tm.tmHeight + (tm.tmExternalLeading * 4);

		::ReleaseDC(NULL, hDC);
	}
	else
	{
		nElementWidth = 10;
		nElementHeight = 10;
	}

	// get start point
	int	nStartX = 0, nStartY = 0;
	RECT DisplayRect = GetDisplayRect();

	nStartX = DisplayRect.left;
	nStartY = DisplayRect.top;

	if ((nElementWidth * MAX_TABLE_COL) < (DisplayRect.right - DisplayRect.left))
		nStartX += (int)(((DisplayRect.right - DisplayRect.left) - (nElementWidth * MAX_TABLE_COL)) / 2);

	if ((nElementHeight * (m_nAllowRow+1)) < (DisplayRect.bottom - DisplayRect.top))
		nStartY += (int)(((DisplayRect.bottom - DisplayRect.top) - (nElementHeight * (m_nAllowRow+1)))/2);

	int	left, top;
	// calc element location
	for (row = 1; row < MAX_TABLE_ROW; row++)
	{
		for (col = 0; col < MAX_TABLE_COL; col++)
		{
			if (((row-1) <= m_nAllowRow && row > 1) &&	// 1 - X
				(col <= m_nAllowColumn && col > 0))			// 0 - (X-1)
			{
				m_TableElement[row][col].bAllowSelect = TRUE;
			}

			if (row < 2)
			{
				left = nStartX + (col * nElementWidth);
				top = nStartY + ((row-1) * nElementHeight);
			}
			else
			{
				if (col == 0)
					left = nStartX-4;
				else
					left = nStartX + (col * nElementWidth);
				top = nStartY + ((row-1) * nElementHeight) + (3 * (row-1));
			}

			CPoint pos(left, top);
			CSize sz(nElementWidth, nElementHeight);

			m_TableElement[row][col].rcRect = CRect(pos, sz);
		}
	}

	// set cur element
	m_pCurElement = &m_TableElement[2][1];


	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableEditTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes |= CPicassoShape::DrawControl(hDC, rcUpdate);

	int	row, col;
	HFONT	OldFont = NULL;
	HPEN	OldPen = NULL;

	//////////////////////////////////////////////////////////////////////////
	// DRAW GUIDE STRING

	for (row = 1; row < MAX_TABLE_ROW && row-1 <= m_nAllowRow; row++)
	{
		for (col = 0; col < MAX_TABLE_COL; col++)
		{
			int	nDrawIndex = 0;
			CString	strText;
			LPRECT	lpRect = NULL;
			BOOL	bDraw = FALSE;

			if (row < 2 && col != 0)
			{
				strText = m_TableElement[row][col].strText;
				lpRect = (LPRECT)m_TableElement[row][col].rcRect;
				
				if (col == m_pCurElement->nCol)
					nDrawIndex = 1;

				bDraw = TRUE;
			}
			else if (col == 0 && row != 1)
			{
				strText = m_TableElement[row][col].strText;
				lpRect = (LPRECT)m_TableElement[row][col].rcRect;
				
				if (row == m_pCurElement->nRow)
					nDrawIndex = 1;

				bDraw = TRUE;
			}

			if (bDraw == TRUE)
			{
				HFONT		hOldFont;
//				COLORREF	nOldTextColor;
				COLORREF	nOldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책

				// Select Font and Color
				if (m_pGuideFont[nDrawIndex] != NULL)
					hOldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pGuideFont[nDrawIndex]->GetHandle());
				if (m_pGuideTextColor[nDrawIndex] != NULL)
					nOldTextColor = ::SetTextColor(hDC, m_pGuideTextColor[nDrawIndex]->m_crColor);

				// Select to Transparent Mode in Background Mode
				int OldBkMode = ::SetBkMode(hDC, TRANSPARENT);

				DrawText(hDC,
						 strText,
						 1,
						 lpRect,
						 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				// Restore Background Mode
				::SetBkMode(hDC, OldBkMode);

				// Restore Font and Color
				if (m_pGuideFont[nDrawIndex] != NULL)
					::SelectObject(hDC, hOldFont);
				if (m_pGuideTextColor[nDrawIndex] != NULL)
					::SetTextColor(hDC, nOldTextColor);
			}

			bDraw = FALSE;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// DRAW ELEMENT STRING

	for (row = 2; row < MAX_TABLE_ROW && row-1 <= m_nAllowRow; row++)
	{
		for (col = 1; col < MAX_TABLE_COL; col++)
		{
			int			nDrawIndex = DISABLE_TYPE;

			if ((row == m_pCurElement->nRow) &&	(col == m_pCurElement->nCol))
				nDrawIndex = SELECT_TYPE;
			else if (m_TableElement[row][col].bAllowSelect == TRUE)
				nDrawIndex = ENABLE_TYPE;

			// DrawBackground
			{
				HBRUSH	hOldBrush;
				HPEN	hOldPen;

				// select brush
				if (m_pElementBrush[nDrawIndex] != NULL)
					hOldBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pElementBrush[nDrawIndex]->GetHandle());
				else
					hOldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));

				// Select Pen
				if (m_pElementPen[nDrawIndex] != NULL)
					hOldPen = (HPEN)::SelectObject(hDC, (HPEN)m_pElementPen[nDrawIndex]->GetHandle());
				else
					hOldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(NULL_PEN));


				::Rectangle(hDC,
							m_TableElement[row][col].rcRect.left,
							m_TableElement[row][col].rcRect.top,
							m_TableElement[row][col].rcRect.right,
							m_TableElement[row][col].rcRect.bottom);

				// Restore Pen
				::SelectObject(hDC, hOldPen);

				// Restore brush
				::SelectObject(hDC, hOldBrush);
			}

			// Draw Text
			if (m_TableElement[row][col].strText.IsEmpty() == FALSE)
			{
				HFONT		hOldFont;
//				COLORREF	nOldTextColor;
				COLORREF	nOldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책

				// Select Font and Color
				if (m_pElementFont[nDrawIndex] != NULL)
					hOldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pElementFont[nDrawIndex]->GetHandle());
				if (m_pElementTextColor[nDrawIndex] != NULL)
					nOldTextColor = ::SetTextColor(hDC, m_pElementTextColor[nDrawIndex]->m_crColor);

				// Select to Transparent Mode in Background Mode
				int OldBkMode = ::SetBkMode(hDC, TRANSPARENT);

				DrawText(hDC,
					     m_TableElement[row][col].strText,
						 -1,
						 m_TableElement[row][col].rcRect,
						 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				// Restore Background Mode
				::SetBkMode(hDC, OldBkMode);

				// Restore Font and Color
				if (m_pElementFont[nDrawIndex] != NULL)
					::SelectObject(hDC, hOldFont);
				if (m_pElementTextColor[nDrawIndex] != NULL)
					::SetTextColor(hDC, nOldTextColor);
			}
		}
	}

	bRes |= TRUE;

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableEditTextBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	{
		if (strKey.GetLength() == 1)
		{
			if (m_pCurElement != NULL)
			{
				if (strKey == _T("&"))
					m_pCurElement->strText = _T("&&");
				else
					m_pCurElement->strText = strKey;
				
				strKey = DES_SHARP;	// move next pos and refresh.
			}
		}
		else if (strKey == DES_CLEAR)
		{
			if (m_pCurElement != NULL)
			{
				while(1)
				{
					if (m_pCurElement == &m_TableElement[0][0])
						m_pCurElement = &m_TableElement[MAX_TABLE_ROW-1][MAX_TABLE_COL-1];
					else
						m_pCurElement--;

					if (m_pCurElement->bAllowSelect == TRUE)
						break;
				}

				m_pCurElement->strText.Empty();

				AddInvalidRect(GetDisplayRect());
				return TRUE;				
			}
		}

		if ((strKey == DES_STAR || strKey == DES_SHARP) ||
			(strKey == DES_LEFT || strKey == DES_RIGHT))
		{
			if (m_pCurElement != NULL)	// KSK 2009.9.9 Codesonar 지적사항 대책
			{
				while (1)
				{
					if ((strKey == DES_STAR) || (strKey == DES_LEFT))
					{
						if (m_pCurElement == &m_TableElement[0][0])
							m_pCurElement = &m_TableElement[MAX_TABLE_ROW-1][MAX_TABLE_COL-1];
						else
							m_pCurElement--;
					}
					else
					{
						if (m_pCurElement == &m_TableElement[MAX_TABLE_ROW-1][MAX_TABLE_COL-1])
							m_pCurElement = &m_TableElement[0][0];
						else
							m_pCurElement++;
					}
					
					if (m_pCurElement->bAllowSelect == TRUE)
						break;
				}
				
				AddInvalidRect(GetDisplayRect());
				return TRUE;
			}
		}
		else if (strKey == DES_UP || strKey == DES_DOWN)
		{
			if (m_pCurElement != NULL)	// KSK 2009.9.9 Codesonar 지적사항 대책
			{
				int	row, col;
				while(1)
				{
					if (strKey == DES_DOWN)
					{
						row = m_pCurElement->nRow + 1;
						col = m_pCurElement->nCol;
						
						if (row >= (m_nAllowRow + 2))
							row = 2;
					}
					else
					{
						row = m_pCurElement->nRow - 1;
						col = m_pCurElement->nCol;
						
						if (row < 0)
							row = (m_nAllowRow + 1);
					}
					
					m_pCurElement = &m_TableElement[row][col];
					
					if (m_pCurElement->bAllowSelect == TRUE)
						break;
				}
				
				AddInvalidRect(GetDisplayRect());
				return TRUE;
			}
		}
		else if (strKey == _T("CLEARALL"))
		{
			if (m_pCurElement != NULL)	// KSK 2009.9.9 Codesonar 지적사항 대책
			{
				for (int row = 0; row < MAX_TABLE_ROW; row++)
				{
					for (int col = 0; col < MAX_TABLE_COL; col++)
					{
						if (m_TableElement[row][col].bAllowSelect == TRUE)
							m_TableElement[row][col].strText.Empty();
					}
				}
				
				m_pCurElement = &m_TableElement[2][1];
				
				AddInvalidRect(GetDisplayRect());
				return TRUE;
			}
		}
		else if (strKey == _T("CLEARLINE"))
		{
			if (m_pCurElement != NULL)	// KSK 2009.9.9 Codesonar 지적사항 대책
			{
				for (int col = 0; col < MAX_TABLE_COL; col++)
				{
					if (m_TableElement[m_pCurElement->nRow][col].bAllowSelect == TRUE)
						m_TableElement[m_pCurElement->nRow][col].strText.Empty();
				}
				
				m_pCurElement = &m_TableElement[m_pCurElement->nRow][1];
				
				AddInvalidRect(GetDisplayRect());
				return TRUE;
			}
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableEditTextBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		if (!_tcsncmp(strData, DES_ALLOW_ROWCOL, 12) && (strData.GetLength() > 12))
		{
			CStringArray	strTempArray;
			SplitString(strData, _T(":"), strTempArray);

			if (strTempArray.GetSize() >= 2)
			{
				m_nAllowRow = _ttoi(strTempArray[1]);
				m_nAllowColumn = _ttoi(strTempArray[2]);
				Initialize();
			}
		}
		else
		{
			CString			strTemp;
			CString			strValueName;
			CStringArray	strTempArray;
			int	i, j, nInsertIndex;

			int nLen;

			// Split String.
			SplitString(strData, _T("\n"), strTempArray);
		
			for (i = 0; i < strTempArray.GetSize() && i+2 < MAX_TABLE_ROW; i++)
			{
				nLen = strTempArray[i].GetLength();
				for(j = 0, nInsertIndex = 0; j < MAX_TABLE_COL; j++)
				{
					if (m_TableElement[i+2][j].bAllowSelect == TRUE)
					{
						if (nInsertIndex < nLen)
						{
							strTemp = strTempArray[i].GetAt(nInsertIndex++);
							if (strTemp == _T("&"))
								m_TableElement[i+2][j].strText = _T("&&");
							else
								m_TableElement[i+2][j].strText = strTemp;
						}
						else
							m_TableElement[i+2][j].strText.Empty();
					}
				}
			}
		}

		AddInvalidRect(GetDisplayRect());

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableEditTextBox::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableEditTextBox
 FUNCTION NAME: MakeAllFSCMD()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableEditTextBox::MakeAllFSCMD(CString &Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CString strInputData;

	for (int row = 0; row < MAX_TABLE_ROW; row++)
	{
		strInputData.Empty();

		for (int col=0; col < MAX_TABLE_COL; col++)
		{
			if (m_TableElement[row][col].bAllowSelect == TRUE)
			{
				if (m_TableElement[row][col].strText.IsEmpty())
					strInputData += _T(" ");
				else if (m_TableElement[row][col].strText == _T("&&"))
					strInputData += _T("&");
				else
					strInputData += m_TableElement[row][col].strText;
			}
		}

		if (strInputData.GetLength() > 0)
		{
			Data += SCR_RES_DELIMITER + strInputData;
		}
	}

	return FALSE;
}

//------------------------------------------------------------------
//	CPicassoButtonBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: CPicassoButtonBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoButtonBox::CPicassoButtonBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bShow = TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: ~CPicassoButtonBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoButtonBox::~CPicassoButtonBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("CPicassoButtonBox::Initialize\n")));

	CPicassoShape::Initialize();

	if (m_strName.GetLength() > 0)
	{
		// Clear.
		if (_tcsncmp((LPCTSTR)m_strName, DES_AP_VALUE, _tcslen(DES_AP_VALUE)) == 0)
		{
			m_strText = _T("");
			m_bShow = FALSE;		// APValue는 값이 설정되어야 보인다.
		}
		// Hide.
		else if (_tcsncmp((LPCTSTR)m_strName, DES_AP_STATE, _tcslen(DES_AP_STATE)) == 0)
		{
			m_bShow = TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: FindActBtn()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonBox::FindActBtn(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return KeyEvent(strKey);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: MouseEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonBox::MouseEvent(POINT &point)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bShow)
	{
		RECT DisplayRect = GetMousePointRect();
		if (PtInRect(&DisplayRect, point))
		{
			CString strVKey = _T("MOUSE_ACT");

			if (m_Act.GetKind() != REQ_NONE)
			{
				AddRequest(REQ_FSCMD, _T("DING"), _T(""));
				KeyEvent(strVKey);
			}
			return TRUE;
		}
	}
	
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bShow)
	{
		// 1자는 [1, F1]을 Find에서 같게 처리함으로 제외함.
		//if ((strKey.GetLength() > 1 && m_ActKey.Find(strKey) >= 0) ||
		//	(strKey == _T("MOUSE_ACT")))
		if ((IsActKey(strKey) == TRUE) || (strKey == _T("MOUSE_ACT")))
		{
			if ((m_Act.GetKind() == REQ_FSCMD) && (m_Act.m_Data.GetLength() == 0))
			{
				AddRequest(m_Act.GetKind(), m_Act.GetName(), m_strText);
			}
			// [#75] NH AIREAT 2008.04.07 Full FS Command 추가
			else if (m_Act.GetKind() == REQ_FULLFSCMD)
			{
				CString strData;

				strData = m_Act.m_Name;
				strData += SCR_RES_DELIMITER;
				
				if (m_Act.m_Data.GetLength() == 0)
					strData += m_strText;
				else
					strData += m_Act.m_Data;

				AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
			}
			// end of [#75]
			else		
				AddRequest(m_Act);

			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("CPicassoButtonBox::SetVariable\n")));

	if (m_strName == strName)
	{
		if (!_tcsncmp(strData, DES_AP_TEXT_ID, 9) && (strData.GetLength() > 9))
		{
			CString strTextID = strData.Mid(9);
			strTextID.MakeUpper();		// KSK 2012.12.21

			//if (m_pResourcesQueue != NULL)
			if (m_pResourcesQueueCMap != NULL)
			{
				CPicassoResource *pResource = NULL;
				//if (m_pResourcesQueue->FindDataOfList(strTextID, &pResource))
				if (m_pResourcesQueueCMap->Lookup(strTextID, pResource) == TRUE)
				{
					CPicassoLocaleText* pLocaleText = (CPicassoLocaleText*)pResource;
					m_strText = pLocaleText->GetLocaleText(GetLocaleNum());

					// 값이 설정되어 있으면 Show, 그렇지 않으면 Hide
					if (m_strText.GetLength() == 0)
						m_bShow = FALSE;
					else
						m_bShow = TRUE;

					AddInvalidRect(GetDisplayRect());
				}
			}

			return TRUE;
		}
		// APStatusXX
		else if (_tcsncmp(strName, DES_AP_STATE, 7) == 0)
		{
			// On
			if (_tcsncmp(strData, DES_ON, _tcslen(DES_ON)) == 0)
			{
				m_bShow = TRUE;
				NHUIDBG(DBG_CALL, (_T("CPicassoButtonBox::SetVariable APSTATE ON\n")));
			}
			// Off
			else if (_tcsncmp(strData, DES_OFF, _tcslen(DES_OFF)) == 0)
			{
				m_bShow = FALSE;
				NHUIDBG(DBG_CALL, (_T("CPicassoButtonBox::SetVariable APSTATE OFF\n")));
			}

			// 만약 actkey가 없으면 이벤트를 처리하지 않은것으로 간주한다.
			//if (m_ActKey.GetLength() == 0)
			//	return FALSE;
			if (m_arActKey.GetCount() == 0)
				return FALSE;

			return TRUE;
		}
		// APValueXX
		else
		{
			m_strText = strData;
			
			// 값이 설정되어 있으면 Show, 그렇지 않으면 Hide
			if (m_strText.GetLength() == 0)
				m_bShow = FALSE;
			else
				m_bShow = TRUE;

			AddInvalidRect(GetDisplayRect());
			
			return TRUE;
		}

	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bShow)
	{
		return CPicassoTextBox::DrawControl(hDC, rcUpdate);
	}

	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoVirtualKeyButtonBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyButtonBox
 FUNCTION NAME: CPicassoVirtualKeyButtonBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoVirtualKeyButtonBox::CPicassoVirtualKeyButtonBox(CTL_TYPE type) : CPicassoButtonBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyButtonBox
 FUNCTION NAME: ~CPicassoVirtualKeyButtonBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoVirtualKeyButtonBox::~CPicassoVirtualKeyButtonBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyButtonBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoVirtualKeyButtonBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoButtonBox::Initialize();

	if (m_bAutoFocus)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyButtonBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoVirtualKeyButtonBox::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	AddInvalidRect(GetDisplayRect());

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyButtonBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoVirtualKeyButtonBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoChangePasswordBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: CPicassoChangePasswordBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoChangePasswordBox::CPicassoChangePasswordBox(CTL_TYPE type) : CPicassoEditTextBox(type)		// [#16] NH KGS 2008.03.13 CPicassoTextBox -> CPicassoEditTextBox
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	// Password type.
	m_ChangePW[CPTYPE_CUR].SetTextType(TTYPE_PASSWORD);
	m_ChangePW[CPTYPE_NEW].SetTextType(TTYPE_PASSWORD);
	m_ChangePW[CPTYPE_NEWRE].SetTextType(TTYPE_PASSWORD);


	m_nMaxInputChar = 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: ~CPicassoChangePasswordBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoChangePasswordBox::~CPicassoChangePasswordBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: SetPWBoxRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoChangePasswordBox::SetPWBoxRect(CHANGEPWTYPE type, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (type > CPTYPE_MAX)
		return;

	m_ChangePW[type].SetDisplayRect(left, top, width, height, nLoadWidth, nLoadHeight);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoChangePasswordBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoTextBox::Initialize();

	for (int i = CPTYPE_CUR; i < CPTYPE_MAX; i++)
	{
		m_ChangePW[i].SetRequestQueue(GetRequestQueue());
		
		// brush
		m_ChangePW[i].SetBrush(this->m_pBrush, this->m_pFocusBrush);
		
		// Pen
		m_ChangePW[i].SetPen(this->m_pPen, this->m_pFocusPen);
		
		// Font
		m_ChangePW[i].SetFont(this->m_pFont, this->m_pFocusFont);
		m_ChangePW[i].SetFonts(this->m_pFonts, this->m_pFocusFonts);	// [#2186] US KMK 2013.08.29 비밀번호 변경 화면에서 폰트 bold 적용 안되던 문제 수정
		
		// Align
		m_ChangePW[i].SetTextAlign(this->m_Align);
		
		// Text Color
		m_ChangePW[i].SetTextColor(this->m_pColor, this->m_pFocusColor);
		
		// Max Input Char
		m_ChangePW[i].SetMaxInputChar(m_nMaxInputCharOrigin);
		m_ChangePW[i].SetMinInputChar(m_nMinInputChar);

		// Round
		m_ChangePW[i].SetRoundSize(this->m_nRoundWidth, this->m_nRoundHeight);
		
		m_ChangePW[i].SetAutoRun(m_bAutoRun);
		m_ChangePW[i].Initialize();
	}
	
//	m_ChangePW[CPTYPE_CUR].SetFocus(TRUE);
	m_ChangePW[CPTYPE_CUR].SetFocus(m_bAutoFocus);

	m_curInputBox = CPTYPE_CUR;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoChangePasswordBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL		bRes = FALSE;
	CPicassoRequest	req;
	
	bRes |= m_ChangePW[CPTYPE_CUR].DrawControl(hDC, rcUpdate);
	bRes |= m_ChangePW[CPTYPE_NEW].DrawControl(hDC, rcUpdate);
	bRes |= m_ChangePW[CPTYPE_NEWRE].DrawControl(hDC, rcUpdate);

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoChangePasswordBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bFocus)
	{
		CPicassoRequest	req;

		// ENTER KEY
		if (strKey == DES_ENTER)
		{
			if (m_curInputBox < CPTYPE_MAX)
			{
				if( m_nMinInputChar > 0 && (int)_tcslen(m_ChangePW[m_curInputBox].GetInputText()) < m_nMinInputChar)
				{
					return TRUE;
				}

				if (m_curInputBox == CPTYPE_CUR)
				{
					// [#RWC6-56] US William 2019.12.26 Hash passwords
					m_curInputBox = CPTYPE_NEW;

					m_ChangePW[CPTYPE_CUR].SetFocus(FALSE);
					m_ChangePW[CPTYPE_NEW].SetFocus(TRUE);
					m_ChangePW[CPTYPE_NEWRE].SetFocus(FALSE);
				}
				// input new password.
				else if (m_curInputBox == CPTYPE_NEW)
				{
					if (_tcslen(m_ChangePW[CPTYPE_NEW].GetInputText()) > 0)
					{
						m_curInputBox = CPTYPE_NEWRE;

						m_ChangePW[CPTYPE_CUR].SetFocus(FALSE);
						m_ChangePW[CPTYPE_NEW].SetFocus(FALSE);
						m_ChangePW[CPTYPE_NEWRE].SetFocus(TRUE);
					}
				}
				// confirm new password
				else
				{
					if (_tcscmp(m_ChangePW[CPTYPE_NEW].GetInputText(), 
								m_ChangePW[CPTYPE_NEWRE].GetInputText()) == 0)
					{
						// ALL FS CMD로 가정.
						AddRequest(m_Act);
					}
					// wrong password
					else
					{
						m_curInputBox = CPTYPE_NEW;

						AddRequest(REQ_FSCMD, m_Act.GetName(), _T("VERIFICATIONERROR"));

						req.Set(REQ_INITIAL, _T(""), _T(""));
						m_ChangePW[CPTYPE_NEW].Handler(req);
						m_ChangePW[CPTYPE_NEWRE].Handler(req);

						m_ChangePW[CPTYPE_CUR].SetFocus(FALSE);
						m_ChangePW[CPTYPE_NEW].SetFocus(TRUE);
						m_ChangePW[CPTYPE_NEWRE].SetFocus(FALSE);
					}
				}
			}
		}
		// CANCEL
		else if (strKey == DES_CANCEL)
		{
			if (!m_bAutoFocus)
			{
				m_bFocus = FALSE;

				m_ChangePW[CPTYPE_CUR].SetFocus(FALSE);
				m_ChangePW[CPTYPE_NEW].SetFocus(FALSE);
				m_ChangePW[CPTYPE_NEWRE].SetFocus(FALSE);

				AddInvalidRect(m_ChangePW[CPTYPE_CUR].GetDisplayRect());
				AddInvalidRect(m_ChangePW[CPTYPE_NEW].GetDisplayRect());
				AddInvalidRect(m_ChangePW[CPTYPE_NEWRE].GetDisplayRect());
				return TRUE;
			}

			return FALSE;
		}
		else if (strKey == DES_TIMEOVER)
		{
			return FALSE;
		}
		// ELSE
		else
		{
			req.Set(REQ_KEYEVENT, _T(""), strKey);
			m_ChangePW[m_curInputBox].Handler(req);
		}

		AddInvalidRect(m_ChangePW[CPTYPE_CUR].GetDisplayRect());
		AddInvalidRect(m_ChangePW[CPTYPE_NEW].GetDisplayRect());
		AddInvalidRect(m_ChangePW[CPTYPE_NEWRE].GetDisplayRect());

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoChangePasswordBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		return TRUE;
	}
	
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoChangePasswordBox
 FUNCTION NAME: MakeAllFSCMD()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoChangePasswordBox::MakeAllFSCMD(CString &Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	Data += SCR_RES_DELIMITER;
	Data += m_ChangePW[CPTYPE_NEWRE].GetInputText();

	// [#RWC6-56] US William 2019.12.26 Hash passwords
	Data += SCR_RES_DELIMITER;
	Data += m_ChangePW[CPTYPE_CUR].GetInputText();
	
	return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CPicassoChangePasswordBox
FUNCTION NAME: SetCtlFocus()
WRITER       :
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoChangePasswordBox::SetCtlFocus(CString &strName, CString &strData)
{
	if (strName == m_strName)
	{
		m_bFocus = TRUE;

		m_ChangePW[CPTYPE_CUR].SetFocus(TRUE);
		m_ChangePW[CPTYPE_NEW].SetFocus(FALSE);
		m_ChangePW[CPTYPE_NEWRE].SetFocus(FALSE);

		AddInvalidRect(m_ChangePW[CPTYPE_CUR].GetDisplayRect());
		AddInvalidRect(m_ChangePW[CPTYPE_NEW].GetDisplayRect());
		AddInvalidRect(m_ChangePW[CPTYPE_NEWRE].GetDisplayRect());
	}
	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoEventHandler Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: CPicassoSoftwareKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoSoftwareKeyBox::CPicassoSoftwareKeyBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_sizeNumber.cx = 0;
	m_sizeNumber.cy = 0;

	m_sizeGuide.cx = 0;
	m_sizeGuide.cy = 0;
	
	m_sizeSpace.cx = 0;
	m_sizeSpace.cy = 0;

	m_nSwKeyDataIndex = 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: ~CPicassoSoftwareKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoSoftwareKeyBox::~CPicassoSoftwareKeyBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: SetNumberSize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoSoftwareKeyBox::SetNumberSize(SIZE Number)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
	
	m_sizeNumber = Number;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: SetGuideSize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoSoftwareKeyBox::SetGuideSize(SIZE Gudie)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_sizeGuide = Gudie;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: SetSpaceSize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoSoftwareKeyBox::SetSpaceSize(SIZE Space)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_sizeSpace = Space;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoSoftwareKeyBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nSwKeyDataIndex = 0;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoSoftwareKeyBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes |= CPicassoShape::DrawControl(hDC, rcUpdate);

	/////////////////////////////////////////
	// Draw SWkey
	{
		int			OldBkMode;
		HFONT		OldFont = NULL;
//		COLORREF	OldTextColor;
		COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
		RECT	rectKey;
		UINT	uFormat=0;
		int		nDataIndex_1;
		int		line, col;
		
		nDataIndex_1 = 1;
		
		uFormat = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		
		RECT DisplayRect = GetDisplayRect();
		rectKey.top = DisplayRect.top;
		rectKey.left = DisplayRect.left;

		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		
		// Select Font
		if (m_pFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFont->GetHandle());
		
		// Set Text Color
		if (m_pColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pColor->m_crColor);
		
		for (line = 0; line < 4; line++)
		{
			// adjust top
			rectKey.top = DisplayRect.top + ((m_sizeNumber.cy + m_sizeSpace.cy) * line);
			rectKey.bottom = rectKey.top + m_sizeNumber.cy;
			
			for (col = 0; col < 3; col++)
			{
				// Draw Key Data.
				rectKey.left = DisplayRect.left + ((m_sizeNumber.cx + m_sizeSpace.cx) * col);
				rectKey.right = rectKey.left + m_sizeNumber.cx;
				
				// for '<', '>'
				if (line == 3)
					nDataIndex_1 = 0;
				
				if (!(line == 3 && (col == 0 || col == 2)))
				{
					if (g_NSwKeyData[m_nSwKeyDataIndex][nDataIndex_1].GetLength()>0)
						DrawText(hDC, g_NSwKeyData[m_nSwKeyDataIndex][nDataIndex_1++], -1, &rectKey, uFormat);
					else
						nDataIndex_1++;
				}
			}
			
		}
		
		// Restore Text Color	
		if (m_pColor != NULL)
			::SetTextColor(hDC, OldTextColor);

		// Restore Font
		if (OldFont)
			::SelectObject(hDC, OldFont);
		
		// Restore Background Mode
		::SetBkMode(hDC, OldBkMode);

		bRes |= TRUE;
		
	}

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoSoftwareKeyBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoSoftwareKeyBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (strKey == DES_STAR || strKey == DES_SHARP)
	{
		if (strKey == DES_STAR)
			m_nSwKeyDataIndex--;
		else 
			m_nSwKeyDataIndex++;
		
		if (m_nSwKeyDataIndex < 0)
			m_nSwKeyDataIndex = MAX_KEY_SCR - 1;
		
		if (m_nSwKeyDataIndex > MAX_KEY_SCR - 1)
			m_nSwKeyDataIndex = 0;
		
		AddInvalidRect(GetDisplayRect());

		return TRUE;
		
	}
	else if ((strKey.GetLength() == 1) && (strKey >= _T("0")) && (strKey <= _T("9")))
	{

		int	index = strKey.GetAt(0) - '0';

		
		if (index >= 0 && index <= 9)
		{
			if (!_tcscmp(g_NSwKeyData[m_nSwKeyDataIndex][index] , _T("SPC")))
				strKey = _T(" ");
			else if (!_tcscmp(g_NSwKeyData[m_nSwKeyDataIndex][index] , _T("&&")))
				strKey = _T("&");
			else
				strKey = g_NSwKeyData[m_nSwKeyDataIndex][index];
		}
	}
	else if (strKey == DES_CAPS)
	{
		if (m_nSwKeyDataIndex >= 1 && m_nSwKeyDataIndex <= 3)
			m_nSwKeyDataIndex += 3;
		else if (m_nSwKeyDataIndex >= 4 && m_nSwKeyDataIndex <= 6)
			m_nSwKeyDataIndex -= 3;

		AddInvalidRect(GetDisplayRect());

		return TRUE;
	}
	
	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoVirtualKeyBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: CPicassoVirtualKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoVirtualKeyBox::CPicassoVirtualKeyBox(CTL_TYPE type) : CPicassoShape(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pCurKey = NULL;

	m_bShow = FALSE;

	m_nAllowRow = MAX_KEY_ROW;
	m_nAllowCol = MAX_KEY_COL;

	m_pGuideNormalFont = m_pGuideFocusFont = NULL;
	m_pGuideNormalBrush = m_pGuideFocusBrush = NULL;
	m_pGuideNormalPen = m_pGuideFocusPen = NULL;
	m_pGuideNormalTextColor = m_pGuideFocusTextColor = NULL;
	
	m_pVKeyNormalFont = m_pVKeyFocusFont = NULL;
	m_pVKeyNormalBrush = m_pVKeyFocusBrush = NULL;
	m_pVKeyNormalPen = m_pVKeyFocusPen = NULL;
	m_pVKeyNormalTextColor = m_pVKeyFocusTextColor = NULL;

	// Reset
	int	row, col;
	for (row = 0; row < MAX_KEY_ROW; row++)
	{
		for (col = 0; col < MAX_KEY_COL; col++)
		{
			m_KeyTable[row][col].nRow = row;
			m_KeyTable[row][col].nCol = col;
			m_KeyTable[row][col].nUseType = USE_KEY_NONE;
			m_KeyTable[row][col].rcRect = CRect(0, 0, 0, 0);
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: ~CPicassoVirtualKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoVirtualKeyBox::~CPicassoVirtualKeyBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetGuideFont()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetGuideFont(CPicassoFont *pNormal, CPicassoFont *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pGuideNormalFont = pNormal;
	m_pGuideFocusFont = pSelect;

	if (m_pGuideFocusFont == NULL)
		m_pGuideFocusFont = m_pGuideNormalFont;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetGuideBrush()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetGuideBrush(CPicassoBrush *pNormal, CPicassoBrush *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pGuideNormalBrush = pNormal;
	m_pGuideFocusBrush = pSelect;

	if (m_pGuideFocusBrush == NULL)
		m_pGuideFocusBrush = m_pGuideNormalBrush;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetGuidePen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetGuidePen(CPicassoPen *pNormal, CPicassoPen *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pGuideNormalPen = pNormal;
	m_pGuideFocusPen = pSelect;

	if (m_pGuideFocusPen == NULL)
		m_pGuideFocusPen = m_pGuideNormalPen;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetGuideTextColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetGuideTextColor(CPicassoColor *pNormal, CPicassoColor *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pGuideNormalTextColor = pNormal;
	m_pGuideFocusTextColor = pSelect;

	if (m_pGuideFocusTextColor == NULL)
		m_pGuideFocusTextColor = m_pGuideNormalTextColor;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetVKeyFont()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetVKeyFont(CPicassoFont *pNormal, CPicassoFont *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pVKeyNormalFont = pNormal;
	m_pVKeyFocusFont = pSelect;

	if (m_pVKeyFocusFont == NULL)
		m_pVKeyFocusFont = m_pVKeyNormalFont;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetVKeyBrush()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetVKeyBrush(CPicassoBrush *pNormal, CPicassoBrush *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pVKeyNormalBrush = pNormal;
	m_pVKeyFocusBrush = pSelect;

	if (m_pVKeyFocusBrush == NULL)
		m_pVKeyFocusBrush = m_pVKeyNormalBrush;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetVKeyPen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetVKeyPen(CPicassoPen *pNormal, CPicassoPen *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pVKeyNormalPen = pNormal;
	m_pVKeyFocusPen = pSelect;

	if (m_pVKeyFocusPen == NULL)
		m_pVKeyFocusPen = m_pVKeyNormalPen;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetVKeyTextColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoVirtualKeyBox::SetVKeyTextColor(CPicassoColor *pNormal, CPicassoColor *pSelect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pVKeyNormalTextColor = pNormal;
	m_pVKeyFocusTextColor = pSelect;

	if (m_pVKeyFocusTextColor == NULL)
		m_pVKeyFocusTextColor = m_pVKeyNormalTextColor;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoVirtualKeyBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoShape::Initialize();

	int	row, col;

	// get element width, height
	int	nElementWidth = 0, nElementHeight = 0;

	HDC hDC = ::GetDC(NULL);
	if (hDC != NULL)
	{
		TEXTMETRIC	tm;
		HFONT	OldFont = NULL;
		
		// Select Font
		if (m_pVKeyNormalFont != NULL)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pVKeyNormalFont->GetHandle());
		
		GetTextMetrics(hDC, &tm);

		// Restore Font
		if (m_pVKeyNormalFont != NULL)
			::SelectObject(hDC, OldFont);

		nElementWidth = (tm.tmAveCharWidth * 2) + (tm.tmExternalLeading * 4);
		nElementHeight = tm.tmHeight + (tm.tmExternalLeading * 4);

		::ReleaseDC(NULL, hDC);
	}
	else
	{
		nElementWidth = 10;
		nElementHeight = 10;
	}

	// get start point
	int	nStartX = 0, nStartY = 0;
	RECT DisplayRect = GetDisplayRect();

	nStartX = DisplayRect.left;
	nStartY = DisplayRect.top;

	if (((nElementWidth + 2) * m_nAllowCol) < (DisplayRect.right - DisplayRect.left))
		nStartX += (int)(((DisplayRect.right - DisplayRect.left) - ((nElementWidth + 2) * m_nAllowCol)) / 2);

	if (((nElementHeight+2) * (m_nAllowRow)) < (DisplayRect.bottom - DisplayRect.top))
		nStartY += (int)(((DisplayRect.bottom - DisplayRect.top) - ((nElementHeight + 2) * (m_nAllowRow)))/2);

	int	left, top;

	// calc element location
	for (row = 0; row < MAX_KEY_ROW; row++)
	{
		for (col = 0; col < MAX_KEY_COL; col++)
		{
			if (m_KeyTable[row][col].nUseType == USE_KEY_UNDER_GUIDE)
			{
				left = nStartX + (col * nElementWidth) + (2 * col);
				top = nStartY + (row * nElementHeight) + (2 * row);

				CPoint pos(left, top);
				CSize sz((nElementWidth*m_nAllowCol + ((m_nAllowCol-1)*2)), nElementHeight);
				m_KeyTable[row][0].rcRect = CRect(pos, sz);
			}
			else
			{
				left = nStartX + (col * nElementWidth) + (2 * col);
				top = nStartY + (row * nElementHeight) + (2 * row);

				CPoint pos(left, top);
				CSize sz(nElementWidth, nElementHeight);

				m_KeyTable[row][col].rcRect = CRect(pos, sz);
			}
		}
	}

	if (m_bAutoFocus == TRUE)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoVirtualKeyBox::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	AddInvalidRect(GetDisplayRect());

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoVirtualKeyBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoVirtualKeyBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;

	if (m_bShow == FALSE || IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes |= CPicassoShape::DrawControl(hDC, rcUpdate);

	int		row, col;

	for (row = 0; row < m_nAllowRow; row++)
	{
		for (col = 0; col < m_nAllowCol; col++)
		{
			BOOL	bFocused = FALSE;

			if ((m_pCurKey != NULL) &&
				(row == m_pCurKey->nRow && col == m_pCurKey->nCol))
				bFocused = TRUE;

			////////////////////////////////
			// DRAW SHAPE

			if (m_KeyTable[row][col].nUseType != USE_KEY_NONE)
			{
				BOOL	bDraw = TRUE;
				HBRUSH	OldKeyBrush;
				HPEN	OldKeyPen;

				// Select Brush and Pen
				if (bFocused == TRUE)
				{
					if ((m_KeyTable[row][col].nUseType == USE_KEY_GUIDE) ||
						(m_KeyTable[row][col].nUseType == USE_KEY_UNDER_GUIDE))
					{
						if (m_pGuideFocusBrush != NULL)
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pGuideFocusBrush->GetHandle());
						else
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, GetStockObject(NULL_BRUSH));

						if (m_pGuideFocusPen != NULL)
							OldKeyPen = (HPEN)::SelectObject(hDC, (HPEN)m_pGuideFocusPen->GetHandle());
						else
							OldKeyPen = (HPEN)::SelectObject(hDC, GetStockObject(NULL_PEN));
					}
					else
					{
						if (m_pVKeyFocusBrush != NULL)
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pVKeyFocusBrush->GetHandle());
						else
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, GetStockObject(NULL_BRUSH));

						if (m_pVKeyFocusPen != NULL)
							OldKeyPen = (HPEN)::SelectObject(hDC, (HPEN)m_pVKeyFocusPen->GetHandle());
						else
							OldKeyPen = (HPEN)::SelectObject(hDC, GetStockObject(NULL_PEN));
					}
				}
				else
				{
					if ((m_KeyTable[row][col].nUseType == USE_KEY_GUIDE) ||
						(m_KeyTable[row][col].nUseType == USE_KEY_UNDER_GUIDE))
					{
						if (m_pGuideNormalBrush != NULL)
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pGuideNormalBrush->GetHandle());
						else
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, GetStockObject(NULL_BRUSH));

						if (m_pGuideNormalPen != NULL)
							OldKeyPen = (HPEN)::SelectObject(hDC, (HPEN)m_pGuideNormalPen->GetHandle());
						else
							OldKeyPen = (HPEN)::SelectObject(hDC, GetStockObject(NULL_PEN));
					}
					else
					{
						if (m_pVKeyNormalBrush != NULL)
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pVKeyNormalBrush->GetHandle());
						else
							OldKeyBrush = (HBRUSH)::SelectObject(hDC, GetStockObject(NULL_BRUSH));

						if (m_pVKeyNormalPen != NULL)
							OldKeyPen = (HPEN)::SelectObject(hDC, (HPEN)m_pVKeyNormalPen->GetHandle());
						else
							OldKeyPen = (HPEN)::SelectObject(hDC, GetStockObject(NULL_PEN));
					}
				}
				
				::Rectangle(hDC,
							m_KeyTable[row][col].rcRect.left,
							m_KeyTable[row][col].rcRect.top,
							m_KeyTable[row][col].rcRect.right,
							m_KeyTable[row][col].rcRect.bottom);

				// Restore Brush and Pen
				::SelectObject(hDC, OldKeyBrush);
				::SelectObject(hDC, OldKeyPen);
				
			}

			////////////////////////////////
			// DRAW TEXT

			if (m_KeyTable[row][col].nUseType != USE_KEY_NONE)
			{
				int			OldBkMode;
//				HFONT		OldKeyFont;
//				COLORREF	OldKeyColor;
				HFONT		OldKeyFont = NULL;			// KSK 2009.9.9 Codesonar 지적사항 대책
				COLORREF	OldKeyColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책

				// Select Font and Color
				if (bFocused == TRUE)
				{
					if ((m_KeyTable[row][col].nUseType == USE_KEY_GUIDE) ||
						(m_KeyTable[row][col].nUseType == USE_KEY_UNDER_GUIDE))
					{
						if (m_pGuideFocusFont != NULL)
							OldKeyFont = (HFONT)::SelectObject(hDC, (HFONT)m_pGuideFocusFont->GetHandle());
						if (m_pGuideFocusTextColor != NULL)
							OldKeyColor = ::SetTextColor(hDC, m_pGuideFocusTextColor->m_crColor);
					}
					else
					{
						if (m_pVKeyFocusFont != NULL)
							OldKeyFont = (HFONT)::SelectObject(hDC, (HFONT)m_pVKeyFocusFont->GetHandle());
						if (m_pVKeyFocusTextColor != NULL)
							OldKeyColor = ::SetTextColor(hDC, m_pVKeyFocusTextColor->m_crColor);
					}
				}
				else
				{
					if ((m_KeyTable[row][col].nUseType == USE_KEY_GUIDE) ||
						(m_KeyTable[row][col].nUseType == USE_KEY_UNDER_GUIDE))
					{
						if (m_pGuideNormalFont != NULL)
							OldKeyFont = (HFONT)::SelectObject(hDC, (HFONT)m_pGuideNormalFont->GetHandle());
						if (m_pGuideNormalTextColor != NULL)
							OldKeyColor = ::SetTextColor(hDC, m_pGuideNormalTextColor->m_crColor);
					}
					else
					{
						if (m_pVKeyNormalFont != NULL)
							OldKeyFont = (HFONT)::SelectObject(hDC, (HFONT)m_pVKeyNormalFont->GetHandle());
						if (m_pVKeyNormalTextColor != NULL)
							OldKeyColor = ::SetTextColor(hDC, m_pVKeyNormalTextColor->m_crColor);
					}
				}

				// Select to Transparent Mode in Background Mode
				OldBkMode = ::SetBkMode(hDC, TRANSPARENT);

				DrawText(hDC,
						 m_KeyTable[row][col].strText,
						 -1,
						 m_KeyTable[row][col].rcRect,
						 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				// Restore Background Mode
				::SetBkMode(hDC, OldBkMode);

				// Restore Font and Color
				if (bFocused == TRUE)
				{
					if (m_KeyTable[row][col].nUseType == USE_KEY_GUIDE)
					{
						if (m_pGuideFocusFont != NULL)
							::SelectObject(hDC, OldKeyFont);
						if (m_pGuideFocusTextColor != NULL)
							::SetTextColor(hDC, OldKeyColor);
					}
					else
					{
						if (m_pVKeyFocusFont != NULL)
							::SelectObject(hDC, OldKeyFont);
						if (m_pVKeyFocusTextColor != NULL)
							::SetTextColor(hDC, OldKeyColor);
					}
				}
				else
				{
					if (m_KeyTable[row][col].nUseType == USE_KEY_GUIDE)
					{
						if (m_pGuideNormalFont != NULL)
							::SelectObject(hDC, OldKeyFont);
						if (m_pGuideNormalTextColor != NULL)
							::SetTextColor(hDC, OldKeyColor);
					}
					else
					{
						if (m_pVKeyNormalFont != NULL)
							::SelectObject(hDC, OldKeyFont);
						if (m_pVKeyNormalTextColor != NULL)
							::SetTextColor(hDC, OldKeyColor);
					}
				}

				bRes |= TRUE;
			}

			//// -- DESIGN
			if (0)
			{
				// Select Pen
				HPEN rOldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
				HBRUSH rOldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
				
				// Draw Rectangle.
				::Rectangle(hDC, 
					m_KeyTable[row][col].rcRect.left, 
					m_KeyTable[row][col].rcRect.top, 
					m_KeyTable[row][col].rcRect.right, 
					m_KeyTable[row][col].rcRect.bottom);
				
				// Restore Object
				::SelectObject(hDC, rOldBrush);
				::SelectObject(hDC, rOldPen);
			}
			//// -- DESIGN
		}
	}

	// DRAW GUIDE LINE
	{
		CRect	UpperLine, UnderLine;

		// UPPER GUIDE LINE
		if (m_KeyTable[0][0].nUseType == USE_KEY_GUIDE)
		{
			UpperLine.left = m_KeyTable[0][0].rcRect.left-2;
			UpperLine.right = m_KeyTable[0][m_nAllowCol-1].rcRect.right+2;
			UpperLine.top = m_KeyTable[0][0].rcRect.bottom-1;
			UpperLine.bottom = m_KeyTable[0][0].rcRect.bottom-1;
		}
		else
		{
			UpperLine.left = m_KeyTable[0][0].rcRect.left-2;
			UpperLine.right = m_KeyTable[0][m_nAllowCol-1].rcRect.right+2;
			UpperLine.top = m_KeyTable[0][0].rcRect.top-1;
			UpperLine.bottom = m_KeyTable[0][0].rcRect.top-1;
		}

		// UNDER GUIDE LINE
		if (m_KeyTable[m_nAllowRow-1][0].nUseType == USE_KEY_UNDER_GUIDE)
		{
			UnderLine.left = m_KeyTable[m_nAllowRow-1][0].rcRect.left-2;
			UnderLine.right = m_KeyTable[m_nAllowRow-1][m_nAllowCol-1].rcRect.right+2;
			UnderLine.top = m_KeyTable[m_nAllowRow-1][0].rcRect.top-1;
			UnderLine.bottom = m_KeyTable[m_nAllowRow-1][0].rcRect.top-1;
		}
		else
		{
			UnderLine.left = m_KeyTable[m_nAllowRow-2][0].rcRect.left-2;
			UnderLine.right = m_KeyTable[m_nAllowRow-2][m_nAllowCol-1].rcRect.right+2;
			UnderLine.top = m_KeyTable[m_nAllowRow-2][0].rcRect.top-1;
			UnderLine.bottom = m_KeyTable[m_nAllowRow-2][0].rcRect.top-1;
		}

		// Select Pen
		HPEN rOldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
		HBRUSH rOldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
		
		MoveToEx(hDC, UpperLine.left, UpperLine.top, NULL);
		LineTo(hDC, UpperLine.right, UpperLine.bottom);

		MoveToEx(hDC, UnderLine.left, UnderLine.top, NULL);
		LineTo(hDC, UnderLine.right, UnderLine.bottom);

		// Restore Object
		::SelectObject(hDC, rOldBrush);
		::SelectObject(hDC, rOldPen);
	}
	

	return bRes;
}

///////////////////////////////////////////////////////////////
//
//	CNNumberKeyBox
//

CString strShortNumberKey[2][10] =
{
	// row 0
	{ _T("0"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9") },
	// row 1
	{_T("CHARACTER SELECT : 0 ~ 9  SELECT : ENTER")}
};

CString strTeleNumberKey[2][11] =
{
	// row 0
	{ _T("0"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9"), _T(",") },
	// row 1
	{_T("CHARACTER SELECT : 0 ~ 9  SELECT : ENTER")}
};

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoNumberKeyBox
 FUNCTION NAME: CPicassoNumberKeyBox()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoNumberKeyBox::CPicassoNumberKeyBox(CTL_TYPE type) : CPicassoVirtualKeyBox(type)
{
	m_bAutoFocus = TRUE;
	m_nNumberKeyType = NUMBER_KEY_NUMBER;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoNumberKeyBox
 FUNCTION NAME: ~CPicassoNumberKeyBox()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoNumberKeyBox::~CPicassoNumberKeyBox()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoNumberKeyBox
 FUNCTION NAME: CPicassoNumberKeyBox()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoNumberKeyBox::SetNumberKeyType(VNUMBERKEY_TYPE type)
{
	m_nNumberKeyType = type;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoNumberKeyBox
 FUNCTION NAME: CPicassoNumberKeyBox()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoNumberKeyBox::Initialize()
{
	if (m_nNumberKeyType == NUMBER_KEY_TELE_NUMBER)
	{
		m_nAllowRow = 2;
		m_nAllowCol = 11;
	}
	else
	{
		m_nAllowRow = 2;
		m_nAllowCol = 10;
	}

	int	row, col, KeyIndex;

	// Set Use Type
	for (row = 0; row < m_nAllowRow; row++)
	{
		KeyIndex = 0;
		for (col = 0; col < m_nAllowCol; col++)
		{
			if (row == 0)
			{
				m_KeyTable[row][col].nUseType = USE_KEY_VKEY;
				if (m_nNumberKeyType == NUMBER_KEY_NUMBER)
					m_KeyTable[row][col].strText = strShortNumberKey[row][KeyIndex++];
				else
					m_KeyTable[row][col].strText = strTeleNumberKey[row][KeyIndex++];
			}
			else if (row == 1)
			{
				m_KeyTable[row][col].nUseType = USE_KEY_UNDER_GUIDE;
				if (m_nNumberKeyType == NUMBER_KEY_NUMBER)
					m_KeyTable[row][col].strText = strShortNumberKey[1][0];
				else
					m_KeyTable[row][col].strText = strTeleNumberKey[1][0];
				break;
			}
		}
	}

	CPicassoVirtualKeyBox::Initialize();

	m_pCurKey = &m_KeyTable[0][0];

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoNumberKeyBox
 FUNCTION NAME: CPicassoNumberKeyBox()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoNumberKeyBox::SetCtlFocus(CString &strName, CString &strData)
{
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoNumberKeyBox
 FUNCTION NAME: CPicassoNumberKeyBox()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoNumberKeyBox::KeyEvent(CString &strKey)
{
	if (m_bShow == TRUE)
	{
		if (strKey == DES_LEFT || strKey == DES_RIGHT)
		{
			int	row, col;
			VKeyElement	*pElement;

			row = m_pCurKey->nRow;
			col = m_pCurKey->nCol;

			while(1)
			{
				if (strKey == DES_LEFT)
				{
					col -= 1;
					if (col < 0)
						col = MAX_KEY_COL - 1;
				}
				else if (strKey == DES_RIGHT)
				{
					col += 1;
					if (col >= MAX_KEY_COL)
						col = 0;
				}
				
				pElement = &m_KeyTable[row][col];
				if (pElement->nUseType == USE_KEY_VKEY)
					break;
			}
			
			m_pCurKey = &m_KeyTable[row][col];
			AddInvalidRect(GetDisplayRect());
			return TRUE;
		}
		else if (strKey == DES_SELECT)
		{
			if (m_pCurKey != NULL)
			{
				strKey = m_pCurKey->strText;
			}
		}
	}

	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoAlphaKeyBox Implementation
//------------------------------------------------------------------

CString	strUpperKeys[5][10] = 
{
	// row 0
	{ (TCHAR)0x30, (TCHAR)0x31, (TCHAR)0x32, (TCHAR)0x33, (TCHAR)0x34, (TCHAR)0x35, (TCHAR)0x36, (TCHAR)0x37, (TCHAR)0x38, (TCHAR)0x39 },
	// row 1
	{ _T("+"), _T(" "), _T("A"), _T("D"), _T("G"), _T("J"), _T("M"), _T("P"), _T("T"), _T("W") },
	// row 2
	{ _T("-"), _T("Q"), _T("B"), _T("E"), _T("H"), _T("K"), _T("N"), _T("R"), _T("U"), _T("X") },
	// row 3
	{ _T("="), _T("Z"), _T("C"), _T("F"), _T("I"), _T("L"), _T("O"), _T("S"), _T("V"), _T("Y") },
	// row 4
	{_T("CHARACTER SELECT : 0~9, <, >  SELECT : ENTER"),}
};

CString	strLowerKeys[5][10] = 
{
	// row 0
	{ _T("0"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9") },
	// row 1
	{ _T("+"), _T(" "), _T("a"), _T("d"), _T("g"), _T("j"), _T("m"), _T("p"), _T("t"), _T("w") },
	// row 2
	{ _T("-"), _T("q"), _T("b"), _T("e"), _T("h"), _T("k"), _T("n"), _T("r"), _T("u"), _T("x") },
	// row 3
	{ _T("="), _T("z"), _T("c"), _T("f"), _T("i"), _T("l"), _T("o"), _T("s"), _T("v"), _T("y") },
	// row 4
	{_T("CHARACTER SELECT : 0~9, <, >  SELECT : ENTER"),}
};

CString	strNumberKeys[5][10] = 
{
	// row 0
	{ _T("0"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9") },
	// row 1
	{ _T("0"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9") },
	// row 2
	{ _T("("), _T("["), _T("{"), _T("<"), _T(","), _T("!"), _T("`"), _T("%"), _T(":"), _T("?") },
	// row 3
	{ _T(")"), _T("]"), _T("}"), _T(">"), _T("."), _T("$"), (TCHAR)0x22, _T("*"), _T(";"), _T("/") },
	// row 4
	{_T("CHARACTER SELECT : 0~9, <, >  SELECT : ENTER")}
};

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAlphaKeyBox
 FUNCTION NAME: CPicassoAlphaKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoAlphaKeyBox::CPicassoAlphaKeyBox(CTL_TYPE type) : CPicassoVirtualKeyBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nAlphaKeyType = VKEY_ALPHA_UPPER;
	m_bKeyByMouse = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAlphaKeyBox
 FUNCTION NAME: ~CPicassoAlphaKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoAlphaKeyBox::~CPicassoAlphaKeyBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAlphaKeyBox
 FUNCTION NAME: SetAlphaKeyType()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoAlphaKeyBox::SetAlphaKeyType(VKEY_TYPE type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nAlphaKeyType = type;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAlphaKeyBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAlphaKeyBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nAllowRow = 5;
	m_nAllowCol = 19;

	int	row, col, KeyIndex;

	// Set Use Type
	for (row = 0; row < m_nAllowRow; row++)
	{
		KeyIndex = 0;
		for (col = 0; col < MAX_KEY_COL; col++)
		{
			if (row < m_nAllowRow - 1)
			{
				if ((col % 2) == 0)
				{
					if (row == 0)
						m_KeyTable[row][col].nUseType = USE_KEY_GUIDE;
					else
						m_KeyTable[row][col].nUseType = USE_KEY_VKEY;

					if (m_nAlphaKeyType == VKEY_ALPHA_UPPER)
						m_KeyTable[row][col].strText = strUpperKeys[row][KeyIndex++];
					else if (m_nAlphaKeyType == VKEY_ALPHA_LOWER)
						m_KeyTable[row][col].strText = strLowerKeys[row][KeyIndex++];
					else if (m_nAlphaKeyType == VKEY_ALPHA_NUMBER)
						m_KeyTable[row][col].strText = strNumberKeys[row][KeyIndex++];
				}
			}
			else
			{
				if (col == 0)
				{
					m_KeyTable[row][col].nUseType = USE_KEY_UNDER_GUIDE;
					if (m_nAlphaKeyType == VKEY_ALPHA_UPPER)
						m_KeyTable[row][col].strText = strUpperKeys[row][KeyIndex++];
					else if (m_nAlphaKeyType == VKEY_ALPHA_LOWER)
						m_KeyTable[row][col].strText = strLowerKeys[row][KeyIndex++];
					else if (m_nAlphaKeyType == VKEY_ALPHA_NUMBER)
						m_KeyTable[row][col].strText = strNumberKeys[row][KeyIndex++];
					break;
				}
			}
		}
	}

	CPicassoVirtualKeyBox::Initialize();

	m_pCurKey = &m_KeyTable[1][0];
	m_bKeyByMouse = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAlphaKeyBox
 FUNCTION NAME: MouseEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAlphaKeyBox::MouseEvent(POINT &point)
{
/*
	if (m_bShow == TRUE)
	{
		int	row, col;

		for (row = 0; row < m_nAllowRow; row++)
		{
			for (col = 0; col < MAX_KEY_COL; col++)
			{
				if (m_KeyTable[row][col].nUseType == USE_KEY_VKEY)
				{
					if (m_KeyTable[row][col].rcRect.PtInRect(point))
					{
						m_pCurKey = &m_KeyTable[row][col];

						CPicassoRequest	req;
						CString		strKey;

						strKey = m_pCurKey->strText;

						req.Set(REQ_KEYEVENT, _T(""), strKey);

						AddRequest(req);

						AddInvalidRect(GetDisplayRect());
						m_bKeyByMouse = TRUE;

						return TRUE;
					}
				}
			}
		}
	}
*/
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAlphaKeyBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAlphaKeyBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bShow == TRUE && m_bKeyByMouse == FALSE)
	{
		if ((strKey.GetLength() == 1) && (strKey >= _T("0")) && (strKey <= _T("9")))
		{
			for (int i = 0; i < MAX_KEY_COL; i++)
			{
				if (m_KeyTable[0][i].strText == strKey)
				{
					int row, col;

					if (i == m_pCurKey->nCol)
					{
						row = m_pCurKey->nRow + 1;
						col = m_pCurKey->nCol;
						
						if (row >= m_nAllowRow-1)
							row = 1;
					}
					else
					{
						row = 1;
						col = i;
					}
					
					m_pCurKey = &m_KeyTable[row][col];

					AddInvalidRect(GetDisplayRect());
					return TRUE;
				}
			}
		}
		else if (strKey == DES_ENTER)
		{
			if (m_pCurKey != NULL)
			{
				strKey = m_pCurKey->strText;
			}
		}
	}
	else if (m_bKeyByMouse == TRUE)
	{
		m_bKeyByMouse = FALSE;
	}

	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoTableKeyBox Implementation
//------------------------------------------------------------------

CString	strUnderGuideText0 = _T("CHARACTER SELECT : <, >   SELECT : ENTER");

CString	strTableKeys[6][19] = 
{ 
	// row 0
	{ (TCHAR)0x20, (TCHAR)0x21, (TCHAR)0x22, (TCHAR)0x23, (TCHAR)0x24, (TCHAR)0x25,    _T("&&"), (TCHAR)0x27, (TCHAR)0x28, (TCHAR)0x29,
	  (TCHAR)0x2A, (TCHAR)0x2B, (TCHAR)0x2C, (TCHAR)0x2D, (TCHAR)0x2E, (TCHAR)0x2F, (TCHAR)0x30, (TCHAR)0x31, (TCHAR)0x32 },
	// row 1
	{ (TCHAR)0x33, (TCHAR)0x34, (TCHAR)0x35, (TCHAR)0x36, (TCHAR)0x37, (TCHAR)0x38, (TCHAR)0x39, (TCHAR)0x3A, (TCHAR)0x3B, (TCHAR)0x3C,
	  (TCHAR)0x3D, (TCHAR)0x3E, (TCHAR)0x3F, (TCHAR)0x40, (TCHAR)0x41, (TCHAR)0x42, (TCHAR)0x43, (TCHAR)0x44, (TCHAR)0x45 },
	// row 2
	{ (TCHAR)0x46, (TCHAR)0x47, (TCHAR)0x48, (TCHAR)0x49, (TCHAR)0x4A, (TCHAR)0x4B, (TCHAR)0x4C, (TCHAR)0x4D, (TCHAR)0x4E, (TCHAR)0x4F,
	  (TCHAR)0x50, (TCHAR)0x51, (TCHAR)0x52, (TCHAR)0x53, (TCHAR)0x54, (TCHAR)0x55, (TCHAR)0x56, (TCHAR)0x57, (TCHAR)0x58 },
	// row 3
	{ (TCHAR)0x59, (TCHAR)0x5A, (TCHAR)0x5B, (TCHAR)0x5C, (TCHAR)0x5D, (TCHAR)0x5E, (TCHAR)0x5F, (TCHAR)0x60, (TCHAR)0x61, (TCHAR)0x62,
	  (TCHAR)0x63, (TCHAR)0x64, (TCHAR)0x65, (TCHAR)0x66, (TCHAR)0x67, (TCHAR)0x68, (TCHAR)0x69, (TCHAR)0x6A, (TCHAR)0x6B },
	// row 4
	{ (TCHAR)0x6C, (TCHAR)0x6D, (TCHAR)0x6E, (TCHAR)0x6F, (TCHAR)0x70, (TCHAR)0x71, (TCHAR)0x72, (TCHAR)0x73, (TCHAR)0x74, (TCHAR)0x75,
	  (TCHAR)0x76, (TCHAR)0x77, (TCHAR)0x78, (TCHAR)0x79, (TCHAR)0x7A, (TCHAR)0x7B, (TCHAR)0x7C, (TCHAR)0x7D, (TCHAR)0x7E },
	// row 5
	{_T("CHARACTER SELECT : <, >   SELECT : ENTER")}
};

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableKeyBox
 FUNCTION NAME: CPicassoTableKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTableKeyBox::CPicassoTableKeyBox(CTL_TYPE type) : CPicassoVirtualKeyBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableKeyBox
 FUNCTION NAME: ~CPicassoTableKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTableKeyBox::~CPicassoTableKeyBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableKeyBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableKeyBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nAllowRow = 6;
	m_nAllowCol = 19;

	int	row, col, KeyIndex;

	// Set Use Type
	for (row = 0; row < m_nAllowRow; row++)
	{
		KeyIndex = 0;
		for (col = 0; col < MAX_KEY_COL; col++)
		{
			if (row < m_nAllowRow - 1)
			{
				m_KeyTable[row][col].nUseType = USE_KEY_VKEY;
				m_KeyTable[row][col].strText = strTableKeys[row][KeyIndex++];
			}
			else
			{
				if (col == 0)
				{
					m_KeyTable[row][0].nUseType = USE_KEY_UNDER_GUIDE;
					m_KeyTable[row][0].strText = strTableKeys[row][KeyIndex++];
				}
			}
		}
	}

	CPicassoVirtualKeyBox::Initialize();

	m_pCurKey = &m_KeyTable[0][0];
	m_bKeyByMouse = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableKeyBox
 FUNCTION NAME: MouseEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableKeyBox::MouseEvent(POINT &point)
{
/*
	if (m_bShow == TRUE)
	{
		int	row, col;

		for (row = 0; row < m_nAllowRow; row++)
		{
			for (col = 0; col < MAX_KEY_COL; col++)
			{
				if (m_KeyTable[row][col].nUseType == USE_KEY_VKEY)
				{
					if (m_KeyTable[row][col].rcRect.PtInRect(point))
					{
						m_pCurKey = &m_KeyTable[row][col];

						CPicassoRequest	req;
						CString		strKey;

						strKey = m_pCurKey->strText;
						if (strKey == _T("&&"))
							strKey = _T("&");
						req.Set(REQ_KEYEVENT, _T(""), strKey);

						AddRequest(req);

						AddInvalidRect(GetDisplayRect());
						m_bKeyByMouse = TRUE;

						return TRUE;
					}
				}
			}
		}
	}
*/
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTableKeyBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTableKeyBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bShow == TRUE && m_bKeyByMouse == FALSE)
	{
		if (strKey == DES_STAR || strKey == DES_SHARP)
		{
			while (1)
			{
				if (strKey == DES_STAR)
				{
					if (m_pCurKey == &m_KeyTable[0][0])
						m_pCurKey = &m_KeyTable[MAX_KEY_ROW-1][MAX_KEY_COL-1];
					else
						m_pCurKey--;
				}
				else
				{
					if (m_pCurKey == &m_KeyTable[MAX_KEY_ROW-1][MAX_KEY_COL-1])
						m_pCurKey = &m_KeyTable[0][0];
					else
						m_pCurKey++;
				}

				if (m_pCurKey->nUseType == USE_KEY_VKEY)
					break;
			}

			AddInvalidRect(GetDisplayRect());
			return TRUE;
		}
		else if (strKey == DES_ENTER)
		{
			if (m_pCurKey != NULL)
			{
				strKey = m_pCurKey->strText;
				if (strKey == _T("&&"))
					strKey = _T("&");
			}
			return FALSE;
		}
		else if ((strKey.GetLength() == 1) && (strKey >= _T("0")) && (strKey <= _T("9")))
		{
			return TRUE;
		}
	}
	else if (m_bKeyByMouse == TRUE)
	{
		m_bKeyByMouse = FALSE;
	}

	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoToggleBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: CPicassoToggleBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoToggleBox::CPicassoToggleBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_strEnableText = _T("");
	m_strDisableText = _T("");
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: ~CPicassoToggleBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoToggleBox::~CPicassoToggleBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: SetEnableText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoToggleBox::SetEnableText(LPCTSTR lpText)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_strEnableText = lpText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: SetDisableText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoToggleBox::SetDisableText(LPCTSTR lpText)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_strDisableText = lpText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoToggleBox::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		if (strData.GetLength() == 0)
		{
			if (m_strText == m_strEnableText)
				strData = m_strDisableText;
			else 
				strData = m_strEnableText;
		}

		if (m_Act.GetKind() != REQ_NONE)
		{
			if (m_Act.GetKind() == REQ_FSCMD)
			{
				if (m_Act.m_Data.GetLength() == 0)
					AddRequest(m_Act.GetKind(), m_Act.GetName(), strData);
				else
					AddRequest(m_Act);
			}
			else if (m_Act.GetKind() == REQ_FULLFSCMD)
			{
				CString strFSData;

				strFSData = m_Act.m_Name;
				strFSData += SCR_RES_DELIMITER;

				if (m_Act.m_Data.GetLength() == 0)
					strFSData += strData;
				else
					strFSData += m_Act.m_Data;

				AddRequest(m_Act.GetKind(), m_Act.GetName(), strFSData);
			}
			else if (m_Act.GetKind() == REQ_KEYEVENT)
			{
				AddRequest(m_Act.GetKind(), NULL, strData);
			}
			else
				AddRequest(m_Act);
		}

		AddInvalidRect(GetDisplayRect());
		
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: SetBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoToggleBox::SetBlinking(void)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bBlinkMode && m_strText == m_strEnableText)
	{
		m_bBlinking = !m_bBlinking;
		AddInvalidRect(GetDisplayRect());

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoToggleBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;
	if (m_strText == m_strEnableText)
	{
		m_bFocus = TRUE;
		bRes |= CPicassoTextBox::DrawControl(hDC, rcUpdate);
		m_bFocus = FALSE;
	}
	else
	{
		bRes |= CPicassoTextBox::DrawControl(hDC, rcUpdate);
	}

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoToggleBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		m_strText = strData;
		
		AddInvalidRect(GetDisplayRect());

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoToggleBox
 FUNCTION NAME: MakeAllFSCMD()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoToggleBox::MakeAllFSCMD(CString &Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	Data += SCR_RES_DELIMITER + m_strText;

	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoThreeStateBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: CPicassoThreeStateBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoThreeStateBox::CPicassoThreeStateBox(CTL_TYPE type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	for (int i = 0; i < 3; i++)
	{
		m_pStateImage[i] = NULL;
		m_pStatePen[i] = NULL;
		m_pStateBrush[i] = NULL;
		m_pStateFont[i] = NULL;
		m_pStateTextColor[i] = NULL;
		m_bStateBlink[i] = FALSE;
	}

	m_nCurState = STATE_HIDE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: ~CPicassoThreeStateBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoThreeStateBox::~CPicassoThreeStateBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetStateImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoThreeStateBox::SetStateImage(CPicassoPicture *pHide, CPicassoPicture *pOn, CPicassoPicture *pOff)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pStateImage[STATE_HIDE] = pHide;
	m_pStateImage[STATE_ON] = pOn;
	m_pStateImage[STATE_OFF] = pOff;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetStatePen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoThreeStateBox::SetStatePen(CPicassoPen *pHide, CPicassoPen *pOn, CPicassoPen *pOff)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pStatePen[STATE_HIDE] = pHide;
	m_pStatePen[STATE_ON] = pOn;
	m_pStatePen[STATE_OFF] = pOff;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetStateBrush()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoThreeStateBox::SetStateBrush(CPicassoBrush *pHide, CPicassoBrush *pOn, CPicassoBrush *pOff)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pStateBrush[STATE_HIDE] = pHide;
	m_pStateBrush[STATE_ON] = pOn;
	m_pStateBrush[STATE_OFF] = pOff;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetStateFont()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoThreeStateBox::SetStateFont(CPicassoFont *pHide, CPicassoFont *pOn, CPicassoFont *pOff)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pStateFont[STATE_HIDE] = pHide;
	m_pStateFont[STATE_ON] = pOn;
	m_pStateFont[STATE_OFF] = pOff;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetStateTextColor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoThreeStateBox::SetStateTextColor(CPicassoColor *pHide, CPicassoColor *pOn, CPicassoColor *pOff)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pStateTextColor[STATE_HIDE] = pHide;
	m_pStateTextColor[STATE_ON] = pOn;
	m_pStateTextColor[STATE_OFF] = pOff;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetStateBlink()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoThreeStateBox::SetStateBlink(BOOL bHide, BOOL bOn, BOOL bOff)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bStateBlink[STATE_HIDE] = bHide;
	m_bStateBlink[STATE_ON] = bOn;
	m_bStateBlink[STATE_OFF] = bOff;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoThreeStateBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nCurState = STATE_HIDE;
	m_bBlinking = FALSE;
	m_nBlinkCount = 0;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoThreeStateBox::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		if (!_tcsncmp(strData, DES_AP_TEXT_ID, 9) && (strData.GetLength() > 9))
		{
			CString strTextID = strData.Mid(9);
			strTextID.MakeUpper();		// KSK 2012.12.21

			//if (m_pResourcesQueue != NULL)
			if (m_pResourcesQueueCMap != NULL)
			{
				CPicassoResource *pResource = NULL;
				//if (m_pResourcesQueue->FindDataOfList(strTextID, &pResource))
				if (m_pResourcesQueueCMap->Lookup(strTextID, pResource))

					SetLocaleText(pResource);
			}
		}
		// APStatusXX
		else if (_tcsncmp(strName, DES_AP_STATE, 7) == 0)
		{
			// Hide
			if (_tcsncmp(strData, DES_HIDE, _tcslen(DES_HIDE)) == 0)
			{
				m_nCurState = STATE_HIDE;
				m_bBlinking = FALSE;
				m_nBlinkCount = 0;
			}
			// On
			else if (_tcsncmp(strData, DES_ON, _tcslen(DES_ON)) == 0)
			{
				m_nCurState = STATE_ON;
				m_bBlinking = FALSE;
				m_nBlinkCount = 0;
			}
			// Off
			else if (_tcsncmp(strData, DES_OFF, _tcslen(DES_OFF)) == 0)
			{
				m_nCurState = STATE_OFF;
				m_bBlinking = FALSE;
				m_nBlinkCount = 0;
			}
		}
		// APValueXX
		else
		{
			m_strText = strData;
		}

		AddInvalidRect(GetDisplayRect());
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: SetBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoThreeStateBox::SetBlinking(void)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bStateBlink[m_nCurState] == TRUE)
	{
		m_nBlinkCount++;

		if ((m_nBlinkCount * BLINKING_TIME) >= 500)
		{
			m_nBlinkCount = 0;

			m_bBlinking = !m_bBlinking;
			AddInvalidRect(GetDisplayRect());

			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoThreeStateBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoThreeStateBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	/////////////////////////////////////
	//	DRAW BODY
	if (!m_bBlinking)
	{
		HPEN	OldPen;
		HBRUSH	OldBrush;

		RECT DisplayRect = GetDisplayRect();
		
		// Draw Picture.
		if (m_pStateImage[m_nCurState] != NULL)
		{
			m_pStateImage[m_nCurState]->DrawImage(GetLocaleNum(), hDC, DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom);
			
#ifdef AE_SCREEN_DEGISN
			// Select Pen
			OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
			OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			
			// Draw Rectangle.
			::Rectangle(hDC, DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom);
			
			// Restore Object
			::SelectObject(hDC, OldBrush);
			::SelectObject(hDC, OldPen);
#endif
		}
		// Draw Boarder and Fill Color
		else if (m_pStatePen[m_nCurState] != NULL || m_pStateBrush[m_nCurState] != NULL)
		{
			// Select Pen
#ifndef AE_SCREEN_DEGISN
			if (m_pStatePen[m_nCurState] != NULL)
				OldPen = (HPEN)::SelectObject(hDC, (HPEN)m_pStatePen[m_nCurState]->GetHandle());
			else
				OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(NULL_PEN));
#else
			OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
#endif
			
			// Select Brush
			if (m_pStateBrush[m_nCurState] != NULL)
				OldBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pStateBrush[m_nCurState]->GetHandle());
			else
				OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			
			// Draw Rectangle.
			{
				::Rectangle(hDC, DisplayRect.left, DisplayRect.top, 
					DisplayRect.right, DisplayRect.bottom);
			}
			
			// Restore Brush
			::SelectObject(hDC, OldBrush);
			
			// Restore Pen.
			::SelectObject(hDC, OldPen);
		}
#ifdef AE_SCREEN_DEGISN
		else
		{
			RECT DisplayRect = GetDisplayRect();

			// Select Pen
			OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
			OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			
			// Draw Rectangle.
			::Rectangle(hDC, DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom);
			
			// Restore Object
			::SelectObject(hDC, OldBrush);
			::SelectObject(hDC, OldPen);
		}
		// AE TEST
#endif
	}


	////////////////////////////////////
	//	DRAW TEXT
	{
		CString		strDrawText;
		
		// Get to Draw Text string
		strDrawText = GetDrawText();
		RECT DisplayRect = GetDisplayRect();
		
		////////////////////////////////////
		if (strDrawText.GetLength())
		{
			int			OldBkMode;
			HFONT		OldFont = NULL;
//			COLORREF	OldTextColor;
			COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
			
			// Select to Transparent Mode in Background Mode
			OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
			
			// Select Font
			if (m_pStateFont[m_nCurState] != NULL)
				OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pStateFont[m_nCurState]->GetHandle());

			// Select Text Color
			if (m_pStateTextColor[m_nCurState] != NULL)
				OldTextColor = ::SetTextColor(hDC, m_pStateTextColor[m_nCurState]->m_crColor);
			
			// Draw Text
			if (!m_bBlinking)
			{
				UINT	uDrawTextFormat, uTextOutFormat;
				RECT	rcClient;
				RECT	rcTemp;
				int		calcHeight, orgHeight;
				
				uDrawTextFormat = 0;
				
				if (!m_bMultiline)
					uDrawTextFormat = DT_SINGLELINE | DT_VCENTER;
				
				if (m_Align == LEFT)
					uDrawTextFormat |= DT_LEFT;
				else if (m_Align == CENTER)
					uDrawTextFormat |= DT_CENTER;
				else if (m_Align == RIGHT)
					uDrawTextFormat |= DT_RIGHT;
				
				// Adjust Draw Rect
				rcClient = DisplayRect;
				rcClient.left += NH_INDENT;
				rcClient.right -= NH_INDENT;
				
				// Calc Rect.
				if (strDrawText.GetLength() > 0)
				{
					// Calc Rect for draw Text
					DrawText(hDC, strDrawText, -1, &rcTemp, DT_CALCRECT);
					
					calcHeight = rcTemp.bottom - rcTemp.top;
					orgHeight = rcClient.bottom - rcClient.top;
					
					if (calcHeight < orgHeight)
						rcClient.top += (int)((orgHeight - calcHeight) / 2);
					
					// Draw Text
					if (m_bMultiline)
					{
						DrawText(hDC, strDrawText, -1, &rcClient, uDrawTextFormat);
					}
					else
					{
						int x;
						
						// Get Align
						if (m_Align == CENTER)
						{
							x = rcClient.left;
							x += (rcClient.right - rcClient.left)/2; 
							uTextOutFormat = TA_CENTER | TA_TOP | TA_NOUPDATECP;
						}
						else if (m_Align == RIGHT)
						{
							x = rcClient.right;
							uTextOutFormat = TA_RIGHT | TA_TOP | TA_NOUPDATECP;
						}
						else
						{
							x = rcClient.left;
							uTextOutFormat = TA_LEFT | TA_TOP | TA_NOUPDATECP;
						}
						
						int	OldAlign = ::SetTextAlign(hDC, uTextOutFormat);
						
						ExtTextOut(hDC, x, rcClient.top, ETO_CLIPPED, &rcClient, strDrawText, strDrawText.GetLength(), NULL);
						
						::SetTextAlign(hDC, OldAlign);
					}
				}
				
			}
			
			// Restore Text Color
			if (m_pStateTextColor[m_nCurState] != NULL)
				::SetTextColor(hDC, OldTextColor);
			
			// Restore Font
			if (m_pStateFont[m_nCurState] != NULL)
				::SelectObject(hDC, OldFont);
			
			// Restore Background Mode
			::SetBkMode(hDC, OldBkMode);
		}	
	}

	return TRUE;
}


//------------------------------------------------------------------
//	CPicassoEJNLBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEJNLBox
 FUNCTION NAME: CPicassoEJNLBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEJNLBox::CPicassoEJNLBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bMultiline = TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEJNLBox
 FUNCTION NAME: ~CPicassoEJNLBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoEJNLBox::~CPicassoEJNLBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEJNLBox
 FUNCTION NAME: GetDrawText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CPicassoEJNLBox::GetDrawText()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CString strDrawText;

	strDrawText = CPicassoTextBox::GetDrawText();

	strDrawText.Replace(FIELD_DELIMITER, '\n');
//	strDrawText.Replace(_T("&"), _T("&&"));	// KSK BUG FIX

	return strDrawText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoEJNLBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoEJNLBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL		bRes = FALSE;
	CString		strDrawText;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// PARENT CLASS DRAW
	bRes |= CPicassoShape::DrawControl(hDC, rcUpdate);
	
	// Get to Draw Text string
	strDrawText = GetDrawText();

	////////////////////////////////////
	if (strDrawText.GetLength())
	{
		int			OldBkMode;
		HFONT		OldFont = NULL;
//		COLORREF	OldTextColor;
		COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책

		RECT DisplayRect = GetDisplayRect();
		
		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		
		// Select Font
		if (m_bFocus && m_pFocusFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFocusFont->GetHandle());
		else if (m_pFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFont->GetHandle());
		
		// Set Text Color
		//OldTextColor = ::SetTextColor(hDC, m_crTextColor);
		if (m_pColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pColor->m_crColor);
				
		// Draw Text
		if (!m_bBlinking)
		{
			UINT	uDrawTextFormat;
			RECT	rcClient;
			RECT	rcTemp;
			int		calcHeight, orgHeight;
			int		calcWidth, orgWidth;
			
			uDrawTextFormat = DT_LEFT;
				
			// Adjust Draw Rect
			rcClient = DisplayRect;

			// Calc Rect.
			if (strDrawText.GetLength() > 0)
			{
				CString strWidth('1',40);
				CString strHeight(L'\n', 25);
				strHeight += _T("1");

				// Calc Rect for draw Text with height - 40자
				DrawText(hDC, strWidth, -1, &rcTemp, DT_CALCRECT);
				
				calcWidth = rcTemp.right - rcTemp.left;
				orgWidth = rcClient.right - rcClient.left;

				if (calcWidth < orgWidth)
					rcClient.left += (int)((orgWidth - calcWidth) / 2);
				
				// Calc Rect for draw Text with height - 20줄
				DrawText(hDC, strHeight, -1, &rcTemp, DT_CALCRECT);
				
				calcHeight = rcTemp.bottom - rcTemp.top;
				orgHeight = rcClient.bottom - rcClient.top;
				
				if (calcHeight < orgHeight)
					rcClient.top += (int)((orgHeight - calcHeight) / 2);
			
				// Draw Text
				DrawText(hDC, strDrawText, -1, &rcClient, uDrawTextFormat);
			}

		}

		// Restore Text Color
		if (m_pColor != NULL)
			::SetTextColor(hDC, OldTextColor);
		
		// Restore Font
		if (OldFont)
			::SelectObject(hDC, OldFont);
		
		// Restore Background Mode
		::SetBkMode(hDC, OldBkMode);

		bRes |= TRUE;
	}

	return bRes;
}


//------------------------------------------------------------------
//	CPicassoDateBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDateBox
 FUNCTION NAME: CPicassoDateBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoDateBox::CPicassoDateBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDateBox
 FUNCTION NAME: ~CPicassoDateBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoDateBox::~CPicassoDateBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDateBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoDateBox::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		m_bFocus = TRUE;
	
		m_strTemp = m_strText;
	}
	else
	{
		m_bFocus = FALSE;
	}

	AddInvalidRect(GetDisplayRect());
		
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDateBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoDateBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bFocus)
	{
		// ENTER KEY
		if (strKey == DES_ENTER)
		{
			// 날짜는 꼬옥 10자 이어야 한다.
			if (m_strText.GetLength() != 10)
			{
				return TRUE;
			}

			if (m_Act.GetKind() != REQ_NONE)
			{
				if (m_Act.GetKind() == REQ_FSCMD)
				{
					if (m_Act.m_Data.GetLength() == 0)
						AddRequest(m_Act.GetKind(), m_Act.GetName(), m_strText);
					else
						AddRequest(m_Act);
				}
				// [#75] NH AIREAT 2008.04.07 Full FS Command 추가
				else if (m_Act.GetKind() == REQ_FULLFSCMD)
				{
					CString strData;
					
					strData = m_Act.m_Name;
					strData += SCR_RES_DELIMITER;
					
					if (m_Act.m_Data.GetLength() == 0)
						strData += m_strText;
					else
						strData += m_Act.m_Data;
					
					AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
				}
				// end of [#75]
				else if (m_Act.GetKind() == REQ_KEYEVENT)
				{
					AddRequest(m_Act.GetKind(), NULL, m_strText);
				}
				else
					AddRequest(m_Act);
			}
			
			// kill focus.
			if (!m_bAutoFocus)
				m_bFocus = FALSE;
			
			AddInvalidRect(GetDisplayRect());
			return TRUE;
		}
		// CLEAR KEY
		else if (strKey == DES_CLEAR)
		{
			if (m_strText.GetLength() > 1)
			{
				int ii, nRemove;
				for (ii=0, nRemove=0; ii < m_strText.GetLength(); ii++)
				{
					if (m_strText.Right(1) == _T("/"))
						m_strText = m_strText.Left(m_strText.GetLength()-1);
					else
					{
						if (nRemove >= 1)
							break;

						m_strText = m_strText.Left(m_strText.GetLength()-1);
						nRemove++;
					}
				}
			}
			else
				m_strText = "";

			AddInvalidRect(GetDisplayRect());
			
			return TRUE;
		}
		// numeric key.
		//else if ((strKey.GetLength() == 1) && (strKey != DES_CANCEL))	// [#470] NH AIREAT 2008.12.22 : '.' 입력 막음.
		else if (strKey.GetLength() == 1)									// [#470] NH AIREAT 2008.12.22 : '.' 입력 막음.
		{
			int nCnt = m_strText.GetLength();
			if ( nCnt < 10)
			{
				nCnt = m_strText.GetLength();
#if DATE_YYYYMMDD
				if (nCnt == 4 || nCnt == 7)		m_strText += _T("/");
#elif DATE_MMDDYYYY
				if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
#elif DATE_DDMMYYYY
				if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
#else
				if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
#endif

				m_strText += strKey;

				nCnt = m_strText.GetLength();
#if DATE_YYYYMMDD
				if (nCnt == 4 || nCnt == 7)		m_strText += _T("/");
#elif DATE_MMDDYYYY
				if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
#elif DATE_DDMMYYYY
				if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
#else
				if (nCnt == 2 || nCnt == 5)		m_strText += _T("/");
#endif

				AddInvalidRect(GetDisplayRect());
			}
			
			return TRUE;
		}
		// [#236] NZ AIREAT 2008.06.04 - TIMEOUT
		else if (strKey == DES_TIMEOVER)
		{
			return FALSE;
		}
		// end of [#236]
		else if (m_Act.GetKind() != REQ_KEYEVENT)
		{
			if (strKey == DES_CANCEL)
			{
				m_strText = m_strTemp;
				m_bFocus = FALSE;
				
				AddInvalidRect(GetDisplayRect());
			}

			return TRUE;
		}
	}
	
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDateBox
 FUNCTION NAME: MakeAllFSCMD()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoDateBox::MakeAllFSCMD(CString &Data)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	Data += SCR_RES_DELIMITER + m_strText;
	
	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoIPBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoIPBox
 FUNCTION NAME: CPicassoIPBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoIPBox::CPicassoIPBox(CTL_TYPE type) : CPicassoEditTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoIPBox
 FUNCTION NAME: ~CPicassoIPBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoIPBox::~CPicassoIPBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoIPBox
 FUNCTION NAME: SetCtlFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoIPBox::SetCtlFocus(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		m_bFocus = TRUE;
	
		m_strTemp = m_strText;
	}
	else
	{
		m_bFocus = FALSE;
	}

	AddInvalidRect(GetDisplayRect());
		
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoIPBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoIPBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bFocus)
	{
		// convert key
		if (strKey == _T("JUM"))
			strKey = _T(".");
		else if (strKey == _T("COMA"))
			strKey = _T(",");

		// ENTER KEY
		if (strKey == DES_ENTER)
		{
			if (m_Act.GetKind() != REQ_NONE)
			{
				if (m_Act.GetKind() == REQ_FSCMD)
				{
					if (m_Act.m_Data.GetLength() == 0)
						AddRequest(m_Act.GetKind(), m_Act.GetName(), m_strText);
					else
						AddRequest(m_Act);
				}
				// [#75] NH AIREAT 2008.04.07 Full FS Command 추가
				else if (m_Act.GetKind() == REQ_FULLFSCMD)
				{
					CString strData;
					
					strData = m_Act.m_Name;
					strData += SCR_RES_DELIMITER;
					
					if (m_Act.m_Data.GetLength() == 0)
						strData += m_strText;
					else
						strData += m_Act.m_Data;
					
					AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
				}
				// end of [#75]
				else if (m_Act.GetKind() == REQ_KEYEVENT)
				{
					AddRequest(m_Act.GetKind(), NULL, m_strText);
				}
				else
					AddRequest(m_Act);
			}
			
			// kill focus.
			if (!m_bAutoFocus)
				m_bFocus = FALSE;
			
			return TRUE;
		}
		// CLEAR KEY
		else if (strKey == DES_CLEAR)
		{
			if (m_strText.GetLength() > 1)
			{
				m_strText = m_strText.Left(m_strText.GetLength()-1);
			}
			else
				m_strText = "";

			AddInvalidRect(GetDisplayRect());
			
			return TRUE;
		}
		// numeric key.
		//else if ((strKey.GetLength() == 1) && (strKey != DES_CANCEL))	// [#470] NH AIREAT 2008.12.22 : '.' 입력 막음.
		else if (strKey.GetLength() == 1)									// [#470] NH AIREAT 2008.12.22 : '.' 입력 막음.
		{
			int nCnt = m_strText.GetLength();
			if (nCnt < 15)
			{
				nCnt = m_strText.GetLength();
				if (nCnt == 3 || nCnt == 7 || nCnt == 11)
					m_strText += _T(".");
				
				m_strText += strKey;

				nCnt = m_strText.GetLength();
				if (nCnt == 3 || nCnt == 7 || nCnt == 11)
					m_strText += _T(".");

				AddInvalidRect(GetDisplayRect());
			}
			
			return TRUE;
		}
		// [#236] NZ AIREAT 2008.06.04 - TIMEOUT
		else if (strKey == DES_TIMEOVER)
		{
			return FALSE;
		}
		// end of [#236]
		else if (m_Act.GetKind() != REQ_KEYEVENT)
		{
			if (strKey == DES_CANCEL)
			{
				m_strText = m_strTemp;
				m_bFocus = FALSE;
				
				AddInvalidRect(GetDisplayRect());
			}

			return TRUE;
		}
	}
	
	return FALSE;
}


//------------------------------------------------------------------
//	CPicassoCentBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCentBox
 FUNCTION NAME: CPicassoCentBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoCentBox::CPicassoCentBox(CTL_TYPE type) : CPicassoEditTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCentBox
 FUNCTION NAME: ~CPicassoCentBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoCentBox::~CPicassoCentBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCentBox
 FUNCTION NAME: GetDrawText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CPicassoCentBox::GetDrawText()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CString strDrawText;

	// 000 -> 0
	// 00321 -> 321
	// 320 -> 320
	if (Asc2Int(m_strText) == 0)
	{
		m_strText = _T("");
	}

	int	RightValue = 0;
	if (m_strText.GetLength() <= 2)
	{
		// 0 -> 0.00
		// 10 -> 0.10
		RightValue = Asc2Int(m_strText.Right(2));
#if (MX_VERSION)
		strDrawText.Format(_T("%s 0.%02d"), SCREEN_CURRENCY, RightValue); //[#2137] MX PCS 2012.07.19
#else
		strDrawText.Format(_T("%s 0.%02d"), GetCurrencySymbol(), RightValue);
#endif
	}
	else
	{
		int Len = 0;
		CString		Temp = _T("");
		// 12345 -> 123.45
		// 123456 -> 1,234.56
		RightValue = Asc2Int(m_strText.Right(2));
		
		// add '.'
// #if (MX_VERSION)	// [#2115] MX KSK 2012.02.22 MX에서는 Surcharge 관련 부분은 모두 MX$로 표시함. (추후 MX에 CentBox가 권종을 따라가게 되면 변경 필요)
// 		strDrawText.Format(_T("%s "), CURRENCY_SYMBOL);
// #else
// 		strDrawText.Format(_T("%s "), GetCurrencySymbol());
// #endif				// end of [#2115]
#if (MX_VERSION)	//[#2137] MX PCS 2012.07.19
		strDrawText.Format(_T("%s "), SCREEN_CURRENCY);
#else
		strDrawText.Format(_T("%s "), GetCurrencySymbol()); //[#2137] MX PCS 2012.07.19
#endif				// end of [#2137]

		Temp = m_strText.Left(m_strText.GetLength()-2);
		Len = Temp.GetLength();

		for (int i=Len; i>0; i--)
		{
			if (((i % 3) == 0) && (i != Len))		strDrawText += ",";
			strDrawText += Temp.GetAt(Len-i);
		}

		// add cent.
		Temp.Format(_T(".%02d"), RightValue);

		strDrawText += Temp;

	}

	return strDrawText;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCentBox
 FUNCTION NAME: DrawControl()
 WRITER       : William (2020.04.24)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCentBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strText == L"X")
	{
		return FALSE;
	}

	return CPicassoEditTextBox::DrawControl(hDC, rcUpdate);
}

//------------------------------------------------------------------
//	CPicassoDollarBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDollarBox
 FUNCTION NAME: CPicassoDollarBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoDollarBox::CPicassoDollarBox(CTL_TYPE type) : CPicassoEditTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDollarBox
 FUNCTION NAME: ~CPicassoDollarBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoDollarBox::~CPicassoDollarBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDollarBox
 FUNCTION NAME: GetDrawText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CPicassoDollarBox::GetDrawText()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CString strDrawText;
	
	// $000
	if (Asc2Int(m_strText) == 0)
	{
		m_strText = _T("");
	}

//	strDrawText.Format(_T("%s %d"), GetCurrencySymbol(), Asc2Int(m_strText));
#if (MX_VERSION)	//[#2137] MX PCS 2012.07.19
	strDrawText.Format(_T("%s %d"), SCREEN_CURRENCY, Asc2Int(m_strText));
#else
	strDrawText.Format(_T("%s %d"), GetCurrencySymbol(), Asc2Int(m_strText));
#endif				// end of [#2137]
	
	return strDrawText;
}

//------------------------------------------------------------------
//	CPicassoAmountBox Implementation
//------------------------------------------------------------------

CPicassoAmountBox::CPicassoAmountBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_strCurrencySymbolName = _T("");
	m_eSymbolType = SYMBOL_NONE;
	m_strAmountTypeName = _T("");
	m_eAmountType = AMOUNT_INT;
	m_strInputMinName = _T("");
	m_nInputMin = 0;
	m_strInputMaxName = _T("");
	m_nInputMax = 0;
	m_strAutoRunName = _T("");
	m_bAutoRun = FALSE;
}

CPicassoAmountBox::~CPicassoAmountBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

void CPicassoAmountBox::SetCurrencySymbolName(CString strName)
{
	m_strCurrencySymbolName = strName;
}

void CPicassoAmountBox::SetAmountTypeName(CString strName)
{
	m_strAmountTypeName = strName;
}

void CPicassoAmountBox::SetInputMinName(CString strName)
{
	m_strInputMinName = strName;
}

void CPicassoAmountBox::SetInputMaxName(CString strName)
{
	m_strInputMaxName = strName;
}

void CPicassoAmountBox::SetAutoRunName(CString strName)
{
	m_strAutoRunName = strName;
}

BOOL CPicassoAmountBox::Initialize(void)
{
	CPicassoShape::Initialize();

	m_eSymbolType = SYMBOL_NONE;
	m_eAmountType = AMOUNT_INT;
	m_nInputMin = 0;
	m_nInputMax = 0;
	m_bAutoRun = FALSE;
	m_strText = _T("");
	m_strTemp = _T("");

	return TRUE;
}

BOOL CPicassoAmountBox::MouseEvent(POINT &point)
{
	if (m_bFocus)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CPicassoAmountBox::KeyEvent(CString &strKey)
{
	if (m_bFocus == FALSE)
		return FALSE;

	if (strKey == DES_ENTER)
	{
		if( m_nInputMin > 0 && m_strText.GetLength() < m_nInputMin)
		{
			return TRUE;
		}

		if (m_Act.GetKind() != REQ_NONE)
		{
			CString strTextData = m_strText;

			if (m_Act.GetKind() == REQ_FSCMD)
			{
				if (m_Act.m_Data.GetLength() == 0)
					AddRequest(m_Act.GetKind(), m_Act.GetName(), strTextData);
				else
					AddRequest(m_Act);
			}
			else if (m_Act.GetKind() == REQ_FULLFSCMD)
			{
				CString strData;
				
				strData = m_Act.m_Name;
				strData += SCR_RES_DELIMITER;
				
				if (m_Act.m_Data.GetLength() == 0)
					strData += strTextData;
				else
					strData += m_Act.m_Data;
				
				AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
			}
			else if (m_Act.GetKind() == REQ_KEYEVENT)
			{
				if (strTextData.GetLength() == 1)
					m_Act.m_Data = _T("0") + strTextData;
				else
					m_Act.m_Data = strTextData;

				m_strText = _T("");

				AddRequest(m_Act.GetKind(), NULL, m_Act.m_Data);
			}
			else
				AddRequest(m_Act);
		}

		// kill focus.
		if (!m_bAutoFocus)
			m_bFocus = FALSE;
		
		AddInvalidRect(GetDisplayRect());
		return TRUE;
	}
	// CLEAR KEY
	else if (strKey == DES_CLEAR)
	{
		m_strText = _T("");

		AddInvalidRect(GetDisplayRect());
		return TRUE;
	}
	// numeric key.
	else if (strKey.GetLength() == 1)
	{
		if (m_strText.GetLength() < m_nInputMax)
		{
			m_strText += strKey;
			AddInvalidRect(GetDisplayRect());		
		}

		// Auto Run..
		if (m_strText.GetLength() == m_nInputMax && m_bAutoRun)
		{
			AddRequest(REQ_KEYEVENT, _T(""), DES_ENTER);
		}

		return TRUE;
	}
	else if (strKey == DES_TIMEOVER)
	{
		return FALSE;
	}
	else if ((m_Act.GetKind() != REQ_KEYEVENT) && !m_bAutoFocus)
	{
		if (strKey == DES_CANCEL)
		{
			m_strText = m_strTemp;
			m_bFocus = FALSE;
			
			AddInvalidRect(GetDisplayRect());
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CPicassoAmountBox::SetVariable(CString &strName, CString &strData)
{
	BOOL	bUpdate = FALSE;

	if (strName.GetLength() <= 0)
		return FALSE;

	if (strName == m_strName)
	{
		m_strText = strData;
		m_pLocaleText = NULL;

		bUpdate = TRUE;
	}
	else if (strName == m_strCurrencySymbolName)
	{
		if (strData == _T("DOLLAR"))
			m_eSymbolType = SYMBOL_DOLLAR;

		bUpdate = TRUE;
	}
	else if (strName == m_strAmountTypeName)
	{
		if (strData == _T("INT"))
			m_eAmountType = AMOUNT_INT;
		else if (strData == _T("INT_CENT"))
			m_eAmountType = AMOUNT_INT_CENT;
		else if (strData == _T("INT_DOLLAR"))
			m_eAmountType = AMOUNT_INT_DOLLAR;
		else if (strData == _T("STRING"))
			m_eAmountType = AMOUNT_STRING;

		bUpdate = TRUE;
	}
	else if (strName == m_strInputMinName)
	{
		m_nInputMin = _ttoi(strData);
		bUpdate = TRUE;
	}
	else if (strName == m_strInputMaxName)
	{
		m_nInputMax = _ttoi(strData);
		bUpdate = TRUE;
	}
	else if (strName == m_strAutoRunName)
	{
		if (strData == _T("on"))
			m_bAutoRun = TRUE;
		bUpdate = TRUE;
	}

	if (bUpdate == TRUE)
	{
		AddInvalidRect(GetDisplayRect());
		return TRUE;
	}

	return FALSE;
}

BOOL CPicassoAmountBox::SetCtlFocus(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		m_bFocus = TRUE;
	
		m_strTemp = m_strText;
	}
	else
	{
		m_bFocus = FALSE;
	}

	AddInvalidRect(GetDisplayRect());
		
	return FALSE;
}

BOOL CPicassoAmountBox::MakeAllFSCMD(CString &Data)
{
	Data += SCR_RES_DELIMITER + m_strText;

	if (m_eAmountType == AMOUNT_INT_DOLLAR)
		Data += _T("00");

	m_strText = _T("");
				
	return FALSE;
}

CString CPicassoAmountBox::GetDrawText()
{
	CString	strSymbol;
	CString	strAmount;
	CString strDrawText;

	switch (m_eSymbolType)
	{
#if (MX_VERSION)	// [#2137] MX KSK 2012.07.25
	case SYMBOL_DOLLAR:		strSymbol.Format(_T("%s  "), SCREEN_CURRENCY); 	break;
#else
	case SYMBOL_DOLLAR:		strSymbol.Format(_T("%s  "), GetCurrencySymbol()); 	break;
#endif				// end of [#2137]
	}

	strDrawText += strSymbol;


	switch (m_eAmountType)
	{
	case AMOUNT_INT:
		{
			if (Asc2Int(m_strText) == 0)
				m_strText = _T("");

			strAmount.Format(_T("%d"), Asc2Int(m_strText));
		}
		break;

	case AMOUNT_INT_CENT:
		{
			if (Asc2Int(m_strText) == 0)
			{
				m_strText = _T("");
			}

			int	RightValue = 0;
			if (m_strText.GetLength() <= 2)
			{
				// 0 -> 0.00
				// 10 -> 0.10
				RightValue = Asc2Int(m_strText.Right(2));
				strAmount.Format(_T("0.%02d"), RightValue);
			}
			else
			{
				int Len = 0;
				CString		Temp = _T("");
				// 12345 -> 123.45
				// 123456 -> 1,234.56
				RightValue = Asc2Int(m_strText.Right(2));
				
				// add ','
				Temp = m_strText.Left(m_strText.GetLength()-2);
				Len = Temp.GetLength();

				for (int i=Len; i>0; i--)
				{
					if (((i % 3) == 0) && (i != Len))		strAmount += ",";
					strAmount += Temp.GetAt(Len-i);
				}

				// add cent.
				Temp.Format(_T(".%02d"), RightValue);

				strAmount += Temp;
			}
		}
		break;

	case AMOUNT_INT_DOLLAR:
		{
			strAmount.Format(_T("%d.00"), Asc2Int(m_strText));
		}
		break;

	case AMOUNT_STRING:
		{
			strAmount.Format(_T("%s"), m_strText);
		}
		break;

	default:
		{
			strAmount = m_strText;
		}
		break;
	}

	strDrawText += strAmount;


	return strDrawText;
}

//------------------------------------------------------------------
//	CPicassoBinBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinBox
 FUNCTION NAME: CPicassoBinBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoBinBox::CPicassoBinBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_Act.Set(REQ_FSCMD, _T("BINLIST"), _T("BINLIST"));

	m_BinItem[BNTYPE_INDEX].SetMaxInputChar(2);
	m_BinItem[BNTYPE_BIN].SetMaxInputChar(10);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinBox
 FUNCTION NAME: ~CPicassoBinBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoBinBox::~CPicassoBinBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoBinBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoTextBox::Initialize();

	m_BinItem[BNTYPE_INDEX].SetRequestQueue(GetRequestQueue());
	m_BinItem[BNTYPE_BIN].SetRequestQueue(GetRequestQueue());
	m_BinItem[BNTYPE_ADD].SetRequestQueue(GetRequestQueue());
	m_BinItem[BNTYPE_DEL].SetRequestQueue(GetRequestQueue());
	m_BinItem[BNTYPE_EDT].SetRequestQueue(GetRequestQueue());

	// brush
	m_BinItem[BNTYPE_INDEX].SetBrush(this->m_pBrush, this->m_pFocusBrush);
	m_BinItem[BNTYPE_BIN].SetBrush(this->m_pBrush, this->m_pFocusBrush);

	// Pen
	m_BinItem[BNTYPE_INDEX].SetPen(this->m_pPen, this->m_pFocusPen);
	m_BinItem[BNTYPE_BIN].SetPen(this->m_pPen, this->m_pFocusPen);

	// Font
	m_BinItem[BNTYPE_INDEX].SetFont(this->m_pFont, this->m_pFocusFont);
	m_BinItem[BNTYPE_BIN].SetFont(this->m_pFont, this->m_pFocusFont);

	// Align
	m_BinItem[BNTYPE_INDEX].SetTextAlign(this->m_Align);
	m_BinItem[BNTYPE_BIN].SetTextAlign(this->m_Align);

	// Round
	m_BinItem[BNTYPE_INDEX].SetRoundSize(this->m_nRoundWidth, this->m_nRoundHeight);
	m_BinItem[BNTYPE_BIN].SetRoundSize(this->m_nRoundWidth, this->m_nRoundHeight);

	// Text Color
	m_BinItem[BNTYPE_INDEX].SetTextColor(this->m_pColor, this->m_pFocusColor);
	m_BinItem[BNTYPE_BIN].SetTextColor(this->m_pColor, this->m_pFocusColor);

	CPicassoRequest	req(REQ_INITIAL, _T(""), _T(""));
	m_BinItem[BNTYPE_INDEX].Handler(req);
	m_BinItem[BNTYPE_BIN].Handler(req);

	m_curInput = BNTYPE_MAX;
	m_BinAct = BNTYPE_MAX;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinBox
 FUNCTION NAME: BinAct()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoBinBox::BinAct(CString &strAct)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CPicassoRequest	req(REQ_INITIAL, _T(""), _T(""));
	m_BinItem[BNTYPE_INDEX].Handler(req);
	m_BinItem[BNTYPE_BIN].Handler(req);
	
	m_curInput = BNTYPE_MAX;
	m_BinAct = BNTYPE_MAX;
	
	if (strAct == DES_BIN_ADD)
	{
		m_curInput = BNTYPE_BIN;
		m_BinAct = BNTYPE_ADD;
		
		m_BinItem[BNTYPE_INDEX].SetFocus(FALSE);
		m_BinItem[BNTYPE_BIN].SetFocus(TRUE);
	}
	else if (strAct == DES_BIN_DEL)
	{
		m_curInput = BNTYPE_INDEX;
		m_BinAct = BNTYPE_DEL;
		
		m_BinItem[BNTYPE_INDEX].SetFocus(TRUE);
		m_BinItem[BNTYPE_BIN].SetFocus(FALSE);
	}
	else if (strAct == DES_BIN_EDT)
	{
		m_curInput = BNTYPE_INDEX;
		m_BinAct = BNTYPE_EDT;
		
		m_BinItem[BNTYPE_INDEX].SetFocus(TRUE);
		m_BinItem[BNTYPE_BIN].SetFocus(FALSE);
	}
	else
		return FALSE;

	AddInvalidRect(m_BinItem[BNTYPE_INDEX].GetDisplayRect());
	AddInvalidRect(m_BinItem[BNTYPE_BIN].GetDisplayRect());
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoBinBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL		bRes = FALSE;
	CPicassoRequest	req;
	
	req.SetKinD(REQ_DRAW);
	req.m_rcUpdate = rcUpdate;
	req.m_hDC = hDC;
	
	bRes |= m_BinItem[BNTYPE_INDEX].Handler(req);
	bRes |= m_BinItem[BNTYPE_BIN].Handler(req);

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoBinBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bFocus && m_curInput != BNTYPE_MAX)
	{
		CPicassoRequest	req;

		AddInvalidRect(m_BinItem[BNTYPE_INDEX].GetDisplayRect());
		AddInvalidRect(m_BinItem[BNTYPE_BIN].GetDisplayRect());
		
		// ENTER KEY
		if (strKey == DES_ENTER)
		{
			CString	strData;

			// set focus Bin.
			if (m_curInput == BNTYPE_INDEX && m_BinAct == BNTYPE_EDT)
			{
				m_curInput = BNTYPE_BIN;

				m_BinItem[BNTYPE_INDEX].SetFocus(FALSE);
				m_BinItem[BNTYPE_BIN].SetFocus(TRUE);

				return TRUE;
			}

			// make FSCmd
			strData = m_Act.GetData();

			if (m_BinAct == BNTYPE_ADD)
			{
				strData += SCR_RES_DELIMITER;
				strData += _T("ADD");
				strData += SCR_RES_DELIMITER;
				strData += m_BinItem[BNTYPE_BIN].GetInputText();
			}
			else if (m_BinAct == BNTYPE_DEL)
			{
				strData += SCR_RES_DELIMITER;
				strData += _T("DELETE");
				strData += SCR_RES_DELIMITER;
				strData += m_BinItem[BNTYPE_INDEX].GetInputText();
			}
			else if (m_BinAct == BNTYPE_EDT)
			{
				strData += SCR_RES_DELIMITER;
				strData += _T("EDIT");
				strData += SCR_RES_DELIMITER;
				strData += m_BinItem[BNTYPE_INDEX].GetInputText();
				strData += SCR_RES_DELIMITER;
				strData += m_BinItem[BNTYPE_BIN].GetInputText();
			}
			else
				return FALSE;

			// Clear Input
			req.Set(REQ_INITIAL, _T(""), _T(""));
			
			m_BinItem[BNTYPE_INDEX].Handler(req);
			m_BinItem[BNTYPE_BIN].Handler(req);

			m_BinItem[BNTYPE_INDEX].SetFocus(FALSE);
			m_BinItem[BNTYPE_BIN].SetFocus(FALSE);

			// GO FSCMD
			AddRequest(m_Act.GetKind(), m_Act.GetName(), strData);
			
			// Clear Input
			req.Set(REQ_INITIAL, _T(""), _T(""));
			
			m_BinItem[BNTYPE_INDEX].Handler(req);
			m_BinItem[BNTYPE_BIN].Handler(req);

			m_BinItem[BNTYPE_INDEX].SetFocus(FALSE);
			m_BinItem[BNTYPE_BIN].SetFocus(FALSE);
			
			m_curInput = BNTYPE_MAX;
			m_BinAct = BNTYPE_MAX;
			
			return TRUE;
		}
		// CANCEL
		else if (strKey == DES_CANCEL)
		{
			// Clear Input
			req.Set(REQ_INITIAL, _T(""), _T(""));
			
			m_BinItem[BNTYPE_INDEX].Handler(req);
			m_BinItem[BNTYPE_BIN].Handler(req);

			m_BinItem[BNTYPE_INDEX].SetFocus(FALSE);
			m_BinItem[BNTYPE_BIN].SetFocus(FALSE);

			m_curInput = BNTYPE_MAX;
			m_BinAct = BNTYPE_MAX;
			return TRUE;
		}
		else if (strKey == DES_TIMEOVER)
		{
			return FALSE;
		}
		// ELSE
		else
		{
			req.Set(REQ_KEYEVENT, _T(""), strKey);
			m_BinItem[m_curInput].Handler(req);
			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinBox
 FUNCTION NAME: SetBinBoxRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoBinBox::SetBinBoxRect(BINTYPE type, int left, int top, int width, int height, int nLoadWidth, int nLoadHeight)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (type > BNTYPE_MAX)
		return;

	m_BinItem[type].SetDisplayRect(left, top, width, height, nLoadWidth, nLoadHeight);
}


//------------------------------------------------------------------
//	CPicassoAnimation Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: CPicassoAnimation()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoAnimation::CPicassoAnimation(CTL_TYPE type/*=CTL_ANI*/) : CPicassoShape(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nImageIndex = 0;
	m_nLoadImage = 0;

	m_bShow = TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: ~CPicassoAnimation()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoAnimation::~CPicassoAnimation()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
	ImageRelease();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: ImageRelease()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoAnimation::ImageRelease()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	for (int i = 0; i < m_nLoadImage; i++)
		m_Image.ReleaseImage(i);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAnimation::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bShow = TRUE;
	m_nImageIndex = 0;
	m_nBlinkCount = 0;

	SetBlinking();

	return TRUE;
}

BOOL CPicassoAnimation::Deinitialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	// Keep images as animations are cached
	//ImageRelease();

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: SetAniImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAnimation::SetAniImage(int Count, CString strPrefix, CString strExt)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	LPCTSTR pAniFile[MAX_ANI]= { NULL, };
	CString	strFileName[MAX_ANI];

	m_nLoadImage = 0;

	for (int i=0; i < Count; i++)
	{
		if (m_nLoadImage > MAX_ANI)
			break;

		strFileName[i].Format(_T("%s%d.%s"),strPrefix, i+1, strExt);
		pAniFile[i] = (LPCTSTR)strFileName[i];

		m_nLoadImage = i+1;
	}

	m_Image.SetDecodeInfo(pAniFile, m_nLoadImage, FALSE);
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAnimation::DrawControl(HDC hDC, RECT &rcUpdate)
{
//	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;

	if (m_bShow == FALSE)
		return FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	RECT DisplayRect = GetDisplayRect();
	bRes = m_Image.Draw(hDC, m_nImageIndex, &DisplayRect);

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: SetBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAnimation::SetBlinking(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (!m_bShow)
		return TRUE;

	{
		m_nImageIndex++;
		if (m_nImageIndex >= m_nLoadImage)
			m_nImageIndex = 0;

		if (m_Image.GetImageState(m_nImageIndex) != 0)
			AddInvalidRect(GetDisplayRect());
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAnimation
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAnimation::SetVariable(CString &strName, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_strName == strName)
	{
		// APStatusXX
		if (_tcsncmp(strName, DES_AP_STATE, 7) == 0)
		{
			// On
			if (_tcsncmp(strData, DES_ON, _tcslen(DES_ON)) == 0)
			{
				m_bShow = TRUE;
			}
			// Off
			else if (_tcsncmp(strData, DES_OFF, _tcslen(DES_OFF)) == 0)
			{
				m_bShow = FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

//------------------------------------------------------------------
//	CPicassoAdvBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTranAdvBox
 FUNCTION NAME: CPicassoTranAdvBox()
 WRITER       : AIREAT (2010.04.22)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTranAdvBox::CPicassoTranAdvBox(CTL_TYPE type)
{
	m_nTranAdvCount = 0;
	m_dwTranAdvShowTime = 0;
	m_nCurIndex = 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTranAdvBox
 FUNCTION NAME: ~CPicassoTranAdvBox()
 WRITER       : AIREAT (2010.04.22)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoTranAdvBox::~CPicassoTranAdvBox()
{
/*
	for (int i = 0; i < MAX_SUPPORT_TRANADV; i++)
	{
		m_Image[i].ReleaseImage(0);
	}
*/
	Deinitialize();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTranAdvBox
 FUNCTION NAME: CPicassoTranAdvBox()
 WRITER       : AIREAT (2010.04.22)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTranAdvBox::Initialize(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	for (int i = 0; i < MAX_SUPPORT_TRANADV; i++)
	{
		m_Image[i].ReleaseImage(0);
	}

	m_nTranAdvCount = 0;
	m_dwTranAdvShowTime = 0;
	m_nCurIndex = 0;
	m_dwShowTime = GetTickCount();

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTranAdvBox
 FUNCTION NAME: CPicassoTranAdvBox()
 WRITER       : AIREAT (2010.04.22)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTranAdvBox::Deinitialize(void)
{
	for (int i = 0; i < MAX_SUPPORT_TRANADV; i++)
	{
		m_Image[i].ReleaseImage(0);
	}
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTranAdvBox
 FUNCTION NAME: CPicassoTranAdvBox()
 WRITER       : AIREAT (2010.04.22)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTranAdvBox::SetVariable(CString &strName, CString &strData)
{
	if (strName.Left(9) == _T("APTranAdv"))
	{
		if (strName == _T("APTranAdv"))
		{
			m_nTranAdvCount = Asc2Int(strData);
		}
		else if (strName == _T("APTranAdvTime"))
		{
			int nTime = Asc2Int(strData);
			if (nTime > 0)
				m_dwTranAdvShowTime = (DWORD)nTime;
		}
		else if (strName.Left(9) == _T("APTranAdv") && strName.GetLength() == 10)
		{
			int		nIndex;
			LPCTSTR pFileName[1];

			nIndex = Asc2Int(strName.Right(1));

			if (nIndex > 0 && nIndex <= MAX_SUPPORT_TRANADV)
			{
				pFileName[0] = (LPCTSTR)strData;
				m_Image[nIndex-1].SetDecodeInfo(pFileName, 1, FALSE);
			}
		}

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTranAdvBox
 FUNCTION NAME: CPicassoTranAdvBox()
 WRITER       : AIREAT (2010.04.22)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTranAdvBox::SetBlinking(void)
{
	if (m_nTranAdvCount > 1)
	{
		DWORD dwElapseSec = (DWORD)((GetTickCount() - m_dwShowTime) / 1000);
		
		if (dwElapseSec >= m_dwTranAdvShowTime)
		{
			//RETAILMSG(1, (L"Change TranADV\n"));

			m_nCurIndex++;
			if (m_nCurIndex >= m_nTranAdvCount)
				m_nCurIndex = 0;

			m_dwShowTime = GetTickCount();
			AddInvalidRect(GetDisplayRect());
			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoTranAdvBox
 FUNCTION NAME: CPicassoTranAdvBox()
 WRITER       : AIREAT (2010.04.22)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoTranAdvBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	if (m_nTranAdvCount > 0)
	{
		RECT DisplayRect = GetDisplayRect();
		RECT rcRect = {DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom};
		bRes = m_Image[m_nCurIndex].Draw(hDC, 0, &rcRect);
	}

	return bRes;
}


//------------------------------------------------------------------
//	CPicassoAdvBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: CPicassoAdvBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoAdvBox::CPicassoAdvBox(CTL_TYPE type/* =CTL_ADVBOX */) : CPicassoTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_strFileName = _T("");
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: ~CPicassoAdvBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoAdvBox::~CPicassoAdvBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

//	ImageRelease();
	Deinitialize();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: SetFileName()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoAdvBox::SetFileName(CString strFileName)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	LPCTSTR pFileName[1];

	pFileName[0] = (LPCTSTR)strFileName;
	
	m_Image.SetDecodeInfo(pFileName, 1, FALSE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: DecodeImage()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAdvBox::DecodeImage()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: IsDecode()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAdvBox::IsDecode()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAdvBox::Initialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	ImageRelease();

	if (m_strName.GetLength() == 0)
		m_Image.DecodeImage(0);

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: Deinitialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAdvBox::Deinitialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

//	m_Image.ReleaseImage(0);
	ImageRelease();

	return TRUE;
}

BOOL CPicassoAdvBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		m_Image.ReleaseImage(0);
		SetFileName(strData);
		m_Image.DecodeImage(0);

		return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoAdvBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	BOOL	bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	RECT DisplayRect = GetDisplayRect();
	RECT rcRect = {DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom};
	bRes = m_Image.Draw(hDC, 0, &rcRect);

	if (bRes == TRUE)
		AddInvalidRect(rcUpdate);

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoAdvBox
 FUNCTION NAME: ImageRelease()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoAdvBox::ImageRelease()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
	m_Image.ReleaseImage(0);
}


//------------------------------------------------------------------
//	CPicassoBinIndexBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinIndexBox
 FUNCTION NAME: CPicassoBinIndexBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoBinIndexBox::CPicassoBinIndexBox(CTL_TYPE type) : CPicassoEditTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_Act.Set(REQ_FSCMD, _T("BINLIST"), _T("BINLIST"));
	m_nMinInputChar = 1;
	m_nMaxInputCharOrigin=m_nMaxInputChar = 4;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinIndexBox
 FUNCTION NAME: ~CPicassoBinIndexBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoBinIndexBox::~CPicassoBinIndexBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinIndexBox
 FUNCTION NAME: BinAct()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoBinIndexBox::BinAct(CString &strAct)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_BinAct = BNTYPE_MAX;

	SetFocus(TRUE);
	if (strAct == DES_BIN_DEL)
	{
		m_BinAct = BNTYPE_DEL;	
	}
	else if (strAct == DES_BIN_EDT)
	{
		m_BinAct = BNTYPE_EDT;		
	}
	else
		return FALSE;

	AddInvalidRect(GetDisplayRect());

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoBinIndexBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoBinIndexBox::KeyEvent(CString &strKey)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bFocus)
	{
		CPicassoRequest	req;
		// ENTER KEY	[#18]
		if (strKey == DES_ENTER)
		{
			if( m_nMinInputChar > 0 && m_strText.GetLength() < m_nMinInputChar)
			{
				return TRUE;
			}

			CString	strData;

			// make FSCmd
			strData = m_Act.GetData();

			 if (m_BinAct == BNTYPE_DEL)
			{
				//strData += _T(":DELETE:");
				strData += SCR_RES_DELIMITER;
				strData += _T("DELETE");
				strData += SCR_RES_DELIMITER;
				strData += GetInputText();
			}
			else if (m_BinAct == BNTYPE_EDT)
			{
				//strData += _T(":EDIT:");
				strData += SCR_RES_DELIMITER;
				strData += _T("EDIT");
				strData += SCR_RES_DELIMITER;
				strData += GetInputText();
			}
			else
				return FALSE;

			AddRequest(m_Act.GetKind(), m_Act.GetName(), strData);

			req.Set(REQ_INITIAL, _T(""), _T(""));
			m_BinAct = BNTYPE_MAX;
			
			m_strText = _T("");

			// kill focus.
			if (!m_bAutoFocus)
				m_bFocus = FALSE;
			
			AddInvalidRect(GetDisplayRect());
			return TRUE;
		}
		// CLEAR KEY
		else if (strKey == DES_CLEAR)
		{
			if (m_strText.GetLength() > 1)
			{
				m_strText = m_strText.Left(m_strText.GetLength()-1);	// [#18]
			}
			else
				m_strText = _T("");

			AddInvalidRect(GetDisplayRect());
			return TRUE;
		}
		// numeric key.
		else if (strKey.GetLength() == 1)
		{
			if (m_strText.GetLength() < m_nMaxInputChar)
			{
				m_strText += strKey;
				AddInvalidRect(GetDisplayRect());		
			}
				// Shift effect. - only support #901
			else if ((m_strText.GetLength()+1) > m_nMaxInputChar && m_Act.GetKind() == REQ_KEYEVENT)
			{
				m_strText += strKey;
				m_strText = m_strText.Right(m_nMaxInputChar);
				AddInvalidRect(GetDisplayRect());
			}

			// Auto Run..
			if (m_strText.GetLength() == m_nMaxInputChar && m_bAutoRun)
			{
				AddRequest(REQ_KEYEVENT, _T(""), DES_ENTER);
			}

			return TRUE;
		}
		// [#236] NZ AIREAT 2008.06.04 - TIMEOUT
		else if (strKey == DES_TIMEOVER)
		{
			return FALSE;
		}
		// end of [#236]
		else if ((m_Act.GetKind() != REQ_KEYEVENT) && !m_bAutoFocus)
		{
			if (strKey == DES_CANCEL)
			{
				m_strText = m_strTemp;
				m_bFocus = FALSE;
				
				AddInvalidRect(GetDisplayRect());
			}

			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
//	CPicassoImageListBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImageListBox
 FUNCTION NAME: CPicassoImageListBox()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoImageListBox::CPicassoImageListBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	int i;
	for (i = 0; i < MAX_IMAGE_LIST; i++)
	{
		m_arImageList[i].bSet = FALSE;
		m_arImageList[i].strName = _T("");
		m_arImageList[i].pImage = NULL;
	}
	
	m_nCurImageList = -1;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImageListBox
 FUNCTION NAME: ~CPicassoImageListBox()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoImageListBox::~CPicassoImageListBox()
{
	for (int i = 0; i < MAX_IMAGE_LIST; i++)
	{
		m_arImageList[i].bSet = FALSE;
		m_arImageList[i].strName = _T("");
		m_arImageList[i].pImage = NULL;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImageListBox
 FUNCTION NAME: SetImageList()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoImageListBox::SetImageList(CString strName, CPicassoPicture *pImage)
{
	for (int i = 0; i < MAX_IMAGE_LIST; i++)
	{
		if (m_arImageList[i].bSet == FALSE)
		{
			m_arImageList[i].bSet = TRUE;
			m_arImageList[i].strName = strName;
			m_arImageList[i].pImage = pImage;
			break;
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImageListBox
 FUNCTION NAME: SetInitList()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoImageListBox::SetInitList(CString strName)
{
	m_strInitName = strName;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImageListBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoImageListBox::Initialize(void)
{
	if (m_strInitName.GetLength() > 0)
	{
		for(int i=0; i < MAX_IMAGE_LIST; i++)
		{
			if (m_arImageList[i].strName == m_strInitName)
				m_nCurImageList = i;
		}
	}
	else
		m_nCurImageList = -1;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImageListBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoImageListBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		if (strData.GetLength() > 0)
		{
			for(int i=0; i < MAX_IMAGE_LIST; i++)
			{
				if (m_arImageList[i].strName == strData)
				{
					m_nCurImageList = i;
					AddInvalidRect(GetDisplayRect());

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoImageListBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoImageListBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	if (m_nCurImageList < 0 ||
		m_nCurImageList > MAX_IMAGE_LIST)
		return FALSE;

	if (m_arImageList[m_nCurImageList].pImage == NULL)
		return FALSE;

	// DRAW;;
	RECT DisplayRect = GetDisplayRect();
	m_arImageList[m_nCurImageList].pImage->DrawImage(GetLocaleNum(), hDC, DisplayRect.left, DisplayRect.top, DisplayRect.right, DisplayRect.bottom);

	return TRUE;
}

//------------------------------------------------------------------
//	CPicassoButtonImageListBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: CPicassoButtonImageListBox()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoButtonImageListBox::CPicassoButtonImageListBox(CTL_TYPE type) : CPicassoImageListBox(type)
{
	m_bShow = TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: ~CPicassoButtonImageListBox()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoButtonImageListBox::~CPicassoButtonImageListBox()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: ()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonImageListBox::Initialize(void)
{
	CPicassoImageListBox::Initialize();

	if (m_nCurImageList >= 0)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: ()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonImageListBox::SetVariable(CString &strName, CString &strData)
{
	BOOL bRes = CPicassoImageListBox::SetVariable(strName, strData);

	if (m_nCurImageList >= 0)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: ()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonImageListBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	if (m_bShow)
		return CPicassoImageListBox::DrawControl(hDC, rcUpdate);

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: ()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonImageListBox::FindActBtn(CString &strKey)
{
	return KeyEvent(strKey);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: ()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonImageListBox::MouseEvent(POINT &point)
{
	if (m_bShow)
	{
		RECT DisplayRect = GetDisplayRect();
		if (PtInRect(&DisplayRect, point))
		{
			CString strVKey = _T("MOUSE_ACT");

			if (m_Act.GetKind() != REQ_NONE)
			{
				AddRequest(REQ_FSCMD, _T("DING"), _T(""));
				KeyEvent(strVKey);
			}
			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoButtonImageListBox
 FUNCTION NAME: ()
 WRITER       : AIREAT (2010.01.12)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoButtonImageListBox::KeyEvent(CString &strKey)
{
	if (m_bShow)
	{
		// 1자는 [1, F1]을 Find에서 같게 처리함으로 제외함.
		//if ((strKey.GetLength() > 1 && m_ActKey.Find(strKey) >= 0) ||
		//	(strKey == _T("MOUSE_ACT")))
		if ((IsActKey(strKey) == TRUE) || (strKey == _T("MOUSE_ACT")))
		{
			if ((m_Act.GetKind() == REQ_FSCMD) && (m_Act.m_Data.GetLength() == 0))
			{
				AddRequest(m_Act.GetKind(), m_Act.GetName(), m_arImageList[m_nCurImageList].strName);
			}
			// [#75] NH AIREAT 2008.04.07 Full FS Command 추가
			else if (m_Act.GetKind() == REQ_FULLFSCMD)
			{
				CString strData;

				strData = m_Act.m_Name;
				strData += SCR_RES_DELIMITER;
				
				if (m_Act.m_Data.GetLength() == 0)
					strData += m_arImageList[m_nCurImageList].strName;
				else
					strData += m_Act.m_Data;

				AddRequest(m_Act.GetKind(), m_Act.m_Name, strData);
			}
			// end of [#75]
			else		
				AddRequest(m_Act);

			return TRUE;
		}
	}

	return FALSE;
}

//------------------------------------------------------------------
//	CPicassoKeyListBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoKeyListBox
 FUNCTION NAME: CPicassoKeyListBox()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoKeyListBox::CPicassoKeyListBox(CTL_TYPE type) : CPicassoTextBox(type)
{
	for (int i = 0; i < MAX_KEY_LIST; i++)
	{
		m_KeyList[i].bSet = FALSE;
		m_KeyList[i].strName = _T("");
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoKeyListBox
 FUNCTION NAME: ~CPicassoKeyListBox()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoKeyListBox::~CPicassoKeyListBox()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoKeyListBox
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoKeyListBox::Initialize(void)
{
	CPicassoTextBox::Initialize();

	for (int i = 0; i < MAX_KEY_LIST; i++)
	{
		m_KeyList[i].bSet = FALSE;
		m_KeyList[i].strName = _T("");
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoKeyListBox
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoKeyListBox::SetVariable(CString &strName, CString &strData)
{
	if (!_tcsncmp(strName, DES_AP_VALUE, 7) && (strData.GetLength() > 0))
	{
		for (int i = 0; i < MAX_KEY_LIST; i++)
		{
			if (strData == m_KeyList[i].strName)
			{
				m_KeyList[i].bSet = TRUE;
				return TRUE;
			}
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoKeyListBox
 FUNCTION NAME: SetKeyList()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoKeyListBox::SetKeyList(CString strKeyName)
{
	for (int i = 0; i < MAX_KEY_LIST; i++)
	{
		if (m_KeyList[i].strName.GetLength() == 0)
		{
			m_KeyList[i].strName = strKeyName;
			return;
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoKeyListBox
 FUNCTION NAME: DrawControl()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoKeyListBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoKeyListBox
 FUNCTION NAME: KeyEvent()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoKeyListBox::KeyEvent(CString &strKey)
{
	if (strKey.GetLength() > 0)
	{
		for (int i = 0; i < MAX_KEY_LIST; i++)
		{
			if ((m_KeyList[i].bSet == TRUE) && (strKey == m_KeyList[i].strName))
			{
				AddRequest(m_Act.GetKind(), m_Act.GetName(), m_KeyList[i].strName);
				return TRUE;
			}
		}
	}

	return FALSE;
}

// [#2065] NH KJW 2011.05.26		// [#2350] US Justin Change Name
//------------------------------------------------------------------
//	CPicassoCustomEditBox Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCustomEditBox
 FUNCTION NAME: CPicassoCustomEditBox()
 WRITER       : KJW (2011.05.26)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Telephone Number Box Control for 10 digits
-------------------------------------------------------------------*/
CPicassoCustomEditBox::CPicassoCustomEditBox(CTL_TYPE type) : CPicassoEditTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
	m_strDisplayFormat = _T("");			// [#2350] US Justin Support Custom Display
	m_nDefinedDisplayNumber = 0;			// [#2350] US Justin Support Custom Display
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCustomEditBox
 FUNCTION NAME: ~CPicassoCustomEditBox()
 WRITER       : KJW (2011.05.26)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoCustomEditBox::~CPicassoCustomEditBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

// [#2350] US Justin Support Custom Display 
void CPicassoCustomEditBox::SetDisplayFormat(CString strDispFormat)
{
	m_strDisplayFormat = strDispFormat;

	// Number : X
	m_nDefinedDisplayNumber = 0;
	CString strTemp;
	for(int i=0; i<m_strDisplayFormat.GetLength(); i++)
	{
		strTemp = m_strDisplayFormat.Mid(i,1);
		if( (strTemp == L"N")||(strTemp == L"*") )
			m_nDefinedDisplayNumber++;
	}
}
// End of [#2350]

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCustomEditBox
 FUNCTION NAME: GetDrawText()
 WRITER       : KJW (2011.05.26)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CPicassoCustomEditBox::GetDrawText()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CString strDrawText = _T("");

	// [#2350] US Justin Support Custom Edit Box
	/*
	// 10자리 초과 -> 10자리
	if( m_strText.GetLength() > 10 )
		m_strText = m_strText.Left(10);

	// 1 -> 1
	// 12 -> 12
	// 123 -> 123
	// 1234 -> 123-4
	// 12345 -> 123-45
	// 123456 -> 123-456
	// 1234567 -> 123-456-7
	// 12345678 -> 123-456-78
	// 123456789 -> 123-456-789
	// 1234567890 -> 123-456-7890

	if( m_strText.GetLength() < 4 )
	{
		strDrawText = m_strText;
	}
	else if( m_strText.GetLength() < 7 )
	{
		strDrawText.Format(_T("%3.3s-%s"), m_strText.Left(3), m_strText.Mid(3) );
	}
	else
	{
		strDrawText.Format(_T("%3.3s-%3.3s-%s"), m_strText.Left(3), m_strText.Mid(3,3), m_strText.Mid(6) );
	}
	*/

	// if "Format" is not defined, use "as is" (Max length)
	if(m_strDisplayFormat.GetLength() <=0)				
	{
		if( m_strText.GetLength() > m_nMaxInputChar)
			m_strText = m_strText.Left(m_nMaxInputChar);	
		strDrawText.Format(_T("%s"), m_strText);
		return strDrawText;
	}

	int nMaxDisp = min(m_nMaxInputChar, m_nDefinedDisplayNumber);
	if( m_strText.GetLength() > nMaxDisp)
		m_strText = m_strText.Left(nMaxDisp);	
	
	if(m_strText.GetLength() > 0)
	{
		int nVal = 0;
		CString strTemp;
		for(int nFmt=0; nFmt<m_strDisplayFormat.GetLength(); nFmt++)
		{
			strTemp = m_strDisplayFormat.Mid(nFmt,1);
			if( (strTemp == L"N")||(strTemp == L"*") )		// Assign number("N") or Mask ("*"...)
			{
				if(strTemp == L"N")		strDrawText += m_strText.Mid(nVal, 1);
				else					strDrawText += L"*";
				nVal++;
			}
			else											// Add other characters
				strDrawText += strTemp;

			// Check next Character (Auto Fill Next "-")
			if(m_strDisplayFormat.GetLength() > (nFmt+2) )
			{
				strTemp = m_strDisplayFormat.Mid(nFmt+1,1);
				if( (strTemp!="N")&&(strTemp!="*") )
				{
					strDrawText += strTemp;
					nFmt++;
				}
			}

			// record until 
			if( nVal>= m_strText.GetLength() )
				break;
		}
	}
	// End of [#2350]
	return strDrawText;
}
// end of [#2065]

// [#2316] US Justin 2014.12.17 
/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoDecimalBox
 FUNCTION NAME: GetDrawText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/

CPicassoDecimalBox::CPicassoDecimalBox(CTL_TYPE type) : CPicassoEditTextBox(type)
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}


CPicassoDecimalBox::~CPicassoDecimalBox()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

CString CPicassoDecimalBox::GetDrawText()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CString strDrawText = _T("");

	if (Asc2Int(m_strText) == 0)
		m_strText = _T("");

	int	RightValue = 0;
	if (m_strText.GetLength() <= 2)
	{
		RightValue = Asc2Int(m_strText.Right(2));
		strDrawText.Format(_T("0.%02d"), RightValue);
	}
	else
	{
		RightValue = Asc2Int(m_strText.Right(2));

		CString Temp = m_strText.Left(m_strText.GetLength()-2);
		int Len = Temp.GetLength();
		for (int i=Len; i>0; i--)
		{
			if (((i % 3) == 0) && (i != Len))		strDrawText += ",";
			strDrawText += Temp.GetAt(Len-i);
		}

		Temp.Format(_T(".%02d"), RightValue);
		strDrawText += Temp;
	}
	return strDrawText;
}

// End of [#2316]