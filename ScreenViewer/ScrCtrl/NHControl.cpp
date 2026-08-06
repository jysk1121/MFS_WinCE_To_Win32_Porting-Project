#include "stdafx.h"
#include ".\NHControl.h"
#include ".\ScreenDisplay.h"
#include ".\ScreenDescType.h"

#ifdef UNDER_CE
#include ".\Common\CmnLib.h"
#else
#include <wingdi.h>
#include <windows.h>
#endif

// preDefine 50 *
//TCHAR	g_strPassAsterisk[] = _T("*************************************************");
TCHAR	g_strPassAsterisk[] = _T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

// for swkey
#define MAX_KEY_SCR		11
#define MAX_KEY_DATA	10

// [#77] KGS 2008.4.18 국가별 통화단위 캐릭터 입력 지원
#define STR_POUNT_CHAR		(TCHAR)0xA3
#define STR_YEN_CHAR		(TCHAR)0xA5
//#define STR_EURO_CHAR		(TCHAR)0x8364

int g_nCurrEditMode = NUMBER;
BOOL g_bIsPrevNotNumberMode = FALSE;
BOOL g_bTabKey = FALSE;


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
// end of [#77]
///////////////////////////////////////////////////////////////
//
//	CRequest
//
CRequest::CRequest()
{
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

CRequest::CRequest(REQ_TYPE type, CString Name, CString Data)
{
	Set(type, Name, Data);
}

void CRequest::Set(REQ_TYPE type, LPCTSTR Name, LPCTSTR Data)
{
	m_ReqType = type;
	m_Name = Name;
	m_Data = Data;
	m_hDC = NULL;
}

CRequest::CRequest(const CRequest &rRequest)
{
	m_ReqType = rRequest.m_ReqType;
	m_Name = rRequest.m_Name;
	m_Data = rRequest.m_Data;
	m_hDC = rRequest.m_hDC;
	m_rcUpdate = rRequest.m_rcUpdate;			// [2ND] NH AIREAT 2008.11.12
	m_Point = rRequest.m_Point;					// [2ND] NH AIREAT 2008.11.12
}

REQ_TYPE CRequest::GetKind()
{
	return m_ReqType;
}

CString	CRequest::GetName()
{
	return m_Name;
}

CString	CRequest::GetData()
{
	return m_Data;
}

void CRequest::SetKinD(REQ_TYPE type)
{
	m_ReqType = type;
}

void CRequest::SetName(CString Name)
{
	m_Name = Name;
}

void CRequest::SetData(CString Data)
{
	m_Data = Data;
}



///////////////////////////////////////////////////////////////
//
//	CEventHandler
//

RECT						CEventHandler::m_InvalidRect;
CNHLinkedList<CRequest>	CEventHandler::m_Requests;					// [#414] AIREAT 2008.09.05

CEventHandler::CEventHandler(CEventHandler *pNext) : m_pNextHandler(pNext)
{
}

void CEventHandler::SetNextHandler(CEventHandler *pNext)
{
	m_pNextHandler = pNext;
}

BOOL	CEventHandler::AddRequest(REQ_TYPE type, LPCTSTR Name, LPCTSTR Data)
{
	CRequest	req;
	req.Set(type, Name, Data);

	return AddRequest(req);
}

BOOL	CEventHandler::AddRequest(CRequest &rRequest)
{
	return m_Requests.PutOnTailOfList(_T(""), rRequest);				// [#414] AIREAT 2008.09.05
}

BOOL	CEventHandler::GetRequest(CRequest &rRequest)
{
	return m_Requests.GetFromHeadOfList(rRequest);
}

BOOL	CEventHandler::ClearRequest(void)
{
	m_Requests.DeleteAll();						// [#414] AIREAT 2008.09.05

	return TRUE;
}

// Invalid 영역의 합집합을 구한다.
void	CEventHandler::AddInvalidRect(RECT rcRect)
{
	if (rcRect.left == SCR_COORD_MAX && rcRect.right == SCR_COORD_MIN && 
		rcRect.top == SCR_COORD_MAX && rcRect.bottom == SCR_COORD_MIN)
	{
		m_InvalidRect = rcRect;
		return;
	}

	// left
	m_InvalidRect.left = __min(rcRect.left, m_InvalidRect.left);

	// right
	m_InvalidRect.right = __max(rcRect.right, m_InvalidRect.right);
		
	// top
	m_InvalidRect.top = __min(rcRect.top, m_InvalidRect.top);

	// bottom
	m_InvalidRect.bottom = __max(rcRect.bottom, m_InvalidRect.bottom);
}

RECT	CEventHandler::GetInvalidRect()
{
	return m_InvalidRect;
}

void	CEventHandler::ClearInvalidRect()
{
	m_InvalidRect.left = SCR_COORD_MAX;
	m_InvalidRect.top = SCR_COORD_MAX;

	m_InvalidRect.right = SCR_COORD_MIN;
	m_InvalidRect.bottom = SCR_COORD_MIN;
}

BOOL CEventHandler::Handler(CRequest &Request)
{
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

	case REQ_SETINPUTMODE:
		SetInputMode(Request.GetData());
		break;

	case REQ_KEYDISABLE:
		SetKeyDisable(Request.GetName());
		break;

	case REQ_SETINPUTTYPE:
		SetInputType(Request.GetData());
		break;

	case REQ_SETBLINKMODE:
		SetAPBlinkMode(Asc2Int(Request.GetData()));
		break;

	default:
		break;
	}

	if (m_pNextHandler && !bRes)
		return m_pNextHandler->Handler(Request);

	return bRes;
}

BOOL CEventHandler::Initialize(void)
{
	m_InvalidRect.left = SCR_COORD_MAX;
	m_InvalidRect.top = SCR_COORD_MAX;
	m_InvalidRect.right = SCR_COORD_MIN;
	m_InvalidRect.bottom = SCR_COORD_MIN;

	return TRUE;
}

BOOL CEventHandler::Deinitialize(void)
{
	return TRUE;
}

BOOL CEventHandler::DrawControl(HDC hDC, RECT &rcUpdate)
{
	return FALSE;
}

BOOL CEventHandler::SetVariable(CString &strName, CString &strData)
{
	return FALSE;
}

BOOL CEventHandler::SetLocaleNum(int Locale)
{
	return FALSE;
}

BOOL CEventHandler::KeyEvent(CString &strKey)
{
	return FALSE;
}

BOOL CEventHandler::MakeAllFSCMD(CString &Data)
{
	return FALSE;
}

BOOL CEventHandler::SetCtlFocus(CString &strName, CString &strData)
{
	return FALSE;
}

BOOL CEventHandler::FindActBtn(CString &strKey)
{
	return FALSE;
}

BOOL CEventHandler::BinAct(CString &strAct)
{
	return FALSE;
}

BOOL CEventHandler::SetBlinking(void)
{
	return FALSE;
}

// [2ND] NH AIREAT 2008.11.12
BOOL CEventHandler::MouseEvent(POINT &point)
{
	return FALSE;
}
// end of [2ND]

BOOL CEventHandler::SetInputMode(CString &strData)
{
	return FALSE;
}

BOOL CEventHandler::SetKeyDisable(CString &strName)
{
	return FALSE;
}

BOOL CEventHandler::SetInputType(CString &strData)
{
	return FALSE;
}

BOOL CEventHandler::SetAPBlinkMode(BOOL bBlinkOn)
{
	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CNControl
//

int	CNControl::m_nLocaleNum = 0;
float CNControl::m_fScreenRateX = 0.0;
float CNControl::m_fScreenRateY = 0.0;
int CNControl::m_nRefCount = 0;

CNControl::CNControl(CTL_TYPE	type)
{
	m_CtlType = type;
	m_bAutoFocus = FALSE;
	m_bFocus = FALSE;

	m_rcDisplay.left = 0;
	m_rcDisplay.right = 0;
	m_rcDisplay.top = 0;
	m_rcDisplay.bottom = 0;

	m_nRefCount++;

	m_strNameEx[0] = _T("");
	m_strNameEx[1] = _T("");
}

CNControl::~CNControl()
{
	m_nRefCount--;

	if (m_nRefCount == 0)
	{
		m_nLocaleNum = 0;
		m_fScreenRateX = 0.0;
		m_fScreenRateY = 0.0;
	}
}

RECT CNControl::GetAdjustScreenReate(int left, int top, int width, int height)
{
	if (m_fScreenRateX == 0 || m_fScreenRateY == 0)
	{
		int currentX = g_sizeScreen.cx;
		int currentY = g_sizeScreen.cy;
		
		//m_fScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
		//m_fScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);

		m_fScreenRateX = (float)((float)currentX / (float)g_BaseSizeScreen.cx);
		m_fScreenRateY = (float)((float)currentY / (float)g_BaseSizeScreen.cy);


		// 에러 발생시 비율을 1:1로 변경함
		if (m_fScreenRateX == 0 || m_fScreenRateY == 0)
		{
			m_fScreenRateX = 1;
			m_fScreenRateY = 1;
		}
	}

	RECT tmpRect;

	tmpRect.left = (int)((float)left * m_fScreenRateX);
	//tmpRect.top = (int)((float)top * m_fScreenRateX);	// [2ND] NH AIREAT 2008.11.12
	tmpRect.top = (int)((float)top * m_fScreenRateY);
	tmpRect.right = tmpRect.left + (int)((float)width * m_fScreenRateX);
	//tmpRect.bottom = tmpRect.top + (int)((float)height * m_fScreenRateX); // [2ND] NH AIREAT 2008.11.12
	tmpRect.bottom = tmpRect.top + (int)((float)height * m_fScreenRateY);

	return tmpRect;
}

CTL_TYPE CNControl::GetType()
{
	return m_CtlType;
}

LPCTSTR CNControl::GetName()
{
	return m_strName;
}

void CNControl::SetName(LPCTSTR str)
{
	if (str)
		m_strName = str;
}

int	CNControl::GetLocaleNum()
{
	return m_nLocaleNum;
}

BOOL CNControl::SetLocaleNum(int Locale)
{
	m_nLocaleNum = Locale;

	return FALSE;
}

RECT CNControl::GetDisplayRect()
{
	return m_rcDisplay;
}

void CNControl::SetDisplayRect(int left, int top, int width, int height)
{
	m_rcDisplay = GetAdjustScreenReate(left, top, width, height);
}

void CNControl::SetAutoFocus(BOOL bAuto)
{
	m_bAutoFocus = bAuto;
}

void CNControl::SetFocus(BOOL bFocus)
{
	m_bFocus = bFocus;
}

BOOL CNControl::IsUpdateControl(RECT &rcUpdate)
{
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

BOOL CNControl::Initialize(void)
{
	if (m_bAutoFocus)
		m_bFocus = TRUE;
	else
		m_bFocus = FALSE;

	return TRUE;
}

void CNControl::SetNameEx(LPCTSTR str1, LPCTSTR str2)
{
	if (str1)
		m_strNameEx[0] = str1;

	if (str2)
		m_strNameEx[1] = str2;
}

///////////////////////////////////////////////////////////////
//
//	CNShape
//
CNShape::CNShape(CTL_TYPE type) : CNControl(type)
{
	m_pPen = m_pFocusPen = NULL;
	m_pBrush = m_pFocusBrush = NULL;
	m_pPicture = NULL;
	//*LEH
	m_nRoundWidth = 0;
	m_nRoundHeight = 0;
	////////////////////////
}

CNShape::~CNShape()
{
}

void CNShape::SetPen(CNResource *pNormal, CNResource *pSelect)
{
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

void CNShape::SetBrush(CNResource *pNormal, CNResource *pSelect)
{
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

void CNShape::SetPicture(CNResource *pPicture)
{
	if (pPicture)
	{
		if (pPicture->GetTpye() == RES_PICTURE)
			m_pPicture = (CNPicture*)pPicture;
	}
}

//*LEH
void CNShape::SetRoundSize(int nWidth, int nHeight)
{
	m_nRoundWidth = nWidth;
	m_nRoundHeight = nHeight;
}
///////////////////

BOOL CNShape::SetBlinking(void)
{
	if (m_pPicture)
	{
		if (m_pPicture->IsAnimationImage())
		{
			AddInvalidRect(m_rcDisplay);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNShape::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;
	HPEN	OldPen;
	HBRUSH	OldBrush;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// Draw Picture.
	if (m_pPicture)
	{
		m_pPicture->DrawImage(hDC, m_rcDisplay.left, m_rcDisplay.top, m_rcDisplay.right, m_rcDisplay.bottom);

#ifdef AE_SCREEN_DEGISN
	// Select Pen
	OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
	OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));

	// Draw Rectangle.
	::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, m_rcDisplay.right, m_rcDisplay.bottom);

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
		
		//*LEH
		// Draw Rectangle.
	/*	{
			::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, 
				m_rcDisplay.right, m_rcDisplay.bottom);
		}
	*/

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
		/////////////////////////////////////////
		
		// Restore Brush
		::SelectObject(hDC, OldBrush);
		
		// Restore Pen.
		::SelectObject(hDC, OldPen);
		bRes = TRUE;
	}
#ifdef AE_SCREEN_DEGISN
	else
	{
		// Select Pen
		OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
		OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
		
		// Draw Rectangle.
		::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, m_rcDisplay.right, m_rcDisplay.bottom);
		
		// Restore Object
		::SelectObject(hDC, OldBrush);
		::SelectObject(hDC, OldPen);
		bRes = TRUE;
	}
	// AE TEST
#endif

	return bRes;
}


///////////////////////////////////////////////////////////////
//
//	CNTextBox
//
CNTextBox::CNTextBox(CTL_TYPE type) : CNShape(type)
{
	m_pFont = m_pFocusFont = NULL;
	m_pLocaleText = NULL;

	m_Align = LEFT;
	m_bMultiline = FALSE;
	m_pColor = m_pFocusColor = NULL;
	/*
	m_crTextColor = RGB(0, 0, 0);
	m_crFocusTextColor = RGB(0, 0, 0);
	*/

	m_TextType = TTYPE_TEXT;

	m_bBlinking = FALSE;
	m_bBlinkMode = FALSE;

	m_nBlinkCount = 0;

}

CNTextBox::~CNTextBox()
{
}

void CNTextBox::SetLocaleText(CNResource *pLocaleText)
{
	m_pLocaleText = (CNLocaleText*)pLocaleText;
	m_strText = _T("");
}

void CNTextBox::SetFont(CNFont *pNormal, CNFont *pSelect)
{
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

void CNTextBox::SetBlinkMode(BOOL BlinkOn)
{
	m_bBlinkMode = BlinkOn;
}

void CNTextBox::SetText(LPCTSTR pText)
{
	m_strText = pText;
}

void CNTextBox::SetTextAlign(TEXTALIGN Align)
{
	m_Align = Align;
}

void CNTextBox::SetMultiLine(BOOL bMultiline)
{
	m_bMultiline = bMultiline;
}

/*
void CNTextBox::SetTextColor(COLORREF NormalColor, COLORREF FocusColor)
{
	m_crTextColor = NormalColor;
	m_crFocusTextColor = FocusColor;
}
*/

void CNTextBox::SetTextColor(CNColor *pColor, CNColor *pFocusColor)
{
	m_pColor = pColor;
	m_pFocusColor = pFocusColor;
}

void CNTextBox::SetActKey(LPCTSTR pKey)
{
	m_ActKey += pKey;
}

void CNTextBox::SetAct(REQ_TYPE type, LPCTSTR pName, LPCTSTR pData)
{
	m_Act.Set(type, pName, pData);	
}

void CNTextBox::SetTextType(TEXTTYPE type)
{
	m_TextType = type;
}

LPCTSTR CNTextBox::GetInputText(void)
{
	return (LPCTSTR)m_strText;
}

BOOL CNTextBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		if (!_tcsncmp(strData, DES_AP_TEXT_ID, 9) && (strData.GetLength() > 9))
		{
			CString strTextID = strData.Mid(9);

			// [#414] AIREAT 2008.09.05
			CNResource *pResource = NULL;
			if (g_NHResources.FindDataOfList(strTextID, &pResource))
				SetLocaleText(pResource);
			// end of [#414]
		}
		else
		{
			m_strText = strData;
			m_pLocaleText = NULL;
		}
		
		AddInvalidRect(m_rcDisplay);
		
		return TRUE;
	}
	return FALSE;
}

BOOL CNTextBox::Initialize(void)
{
	CNShape::Initialize();

	if (m_strName.GetLength() > 0)
	{
		m_pLocaleText = NULL;
		m_strText = _T("");
	}

	m_bBlinking = FALSE;
	m_nBlinkCount = 0;

	return TRUE;
}

BOOL CNTextBox::SetBlinking(void)
{
	CNShape::SetBlinking();

	m_nBlinkCount++;

	if (m_bBlinkMode && ((m_nBlinkCount * BLINKING_TIME) >= 500))
	{
		m_nBlinkCount = 0;

		m_bBlinking = !m_bBlinking;
		AddInvalidRect(m_rcDisplay);

		return TRUE;
	}

	return FALSE;
}

CString CNTextBox::GetDrawText()
{
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

BOOL CNTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL		bRes = FALSE;
	CString		strDrawText;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// PARENT CLASS DRAW
	bRes |= CNShape::DrawControl(hDC, rcUpdate);
	
	// Get to Draw Text string
	strDrawText = GetDrawText();

	////////////////////////////////////
	if (strDrawText.GetLength())
	{
		int			OldBkMode;
		HFONT		OldFont = NULL;
//		COLORREF	OldTextColor;
		COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
		
		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		
		// Select Font
		if (m_bFocus && m_pFocusFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFocusFont->GetHandle());
		else if (m_pFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFont->GetHandle());

		/*
		// Set Text Color
		if (m_bFocus)
			OldTextColor = ::SetTextColor(hDC, m_crFocusTextColor);
		else
			OldTextColor = ::SetTextColor(hDC, m_crTextColor);
		*/
		if (m_bFocus && m_pColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pColor->m_crColor);
		else if (m_pFocusColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pFocusColor->m_crColor);
				
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
			rcClient = m_rcDisplay;
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
//		if (m_pColor != NULL || m_pFocusColor != NULL)
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

BOOL CNTextBox::SetAPBlinkMode(BOOL bBlinkOn)
{
	m_bBlinkMode = bBlinkOn;
	if(FALSE == bBlinkOn)
		m_bBlinking = FALSE;

	return TRUE;
}

///////////////////////////////////////////////////////////////
//
//	CNEffectTextBox
//

CNEffectTextBox::CNEffectTextBox(CTL_TYPE type) : CNTextBox(type)
{
	m_EffectType = EFFECT_TYPING_CENTER;
}

CNEffectTextBox::~CNEffectTextBox()
{
}

void CNEffectTextBox::SetEffectType(EFFECTTYPE type)
{
	m_EffectType = type;
}

BOOL CNEffectTextBox::Initialize(void)
{
	CNShape::Initialize();

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

BOOL CNEffectTextBox::SetBlinking(void)
{
	AddInvalidRect(m_rcDisplay);

	return FALSE;
}

BOOL CNEffectTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL		bRes = FALSE;
	CString		strDrawText;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// PARENT CLASS DRAW
	bRes |= CNShape::DrawControl(hDC, rcUpdate);
	
	// Get to Draw Text string
	strDrawText = GetDrawText();

	////////////////////////////////////
	if (strDrawText.GetLength())
	{
		int			OldBkMode;
		HFONT		OldFont = NULL;
//		COLORREF	OldTextColor;
		COLORREF	OldTextColor = 0xffffffff;	// KSK 2009.9.9 Codesonar 지적사항 대책
		
		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		
		// Select Font
		if (m_bFocus && m_pFocusFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFocusFont->GetHandle());
		else if (m_pFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFont->GetHandle());
		
		/*
		// Set Text Color
		if (m_bFocus)
			OldTextColor = ::SetTextColor(hDC, m_crFocusTextColor);
		else
			OldTextColor = ::SetTextColor(hDC, m_crTextColor);
		*/

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
			int orgHeight = m_rcDisplay.bottom - m_rcDisplay.top;
			int orgWidth = m_rcDisplay.right - m_rcDisplay.left;

			// Calc move amount
			m_moveWidth = (int)(m_widthDrawText / strDrawText.GetLength());
			if (m_moveWidth <= 0)
				m_moveWidth = 2;

			// x position
			if (m_EffectType == EFFECT_SLIDING_LEFT)
				m_curPos.x = m_rcDisplay.right;
			else if (m_EffectType == EFFECT_SLIDING_RIGHT)
				m_curPos.x = m_rcDisplay.left - m_widthDrawText;
			else if (m_EffectType == EFFECT_TYPING_LEFT)
				m_curPos.x = m_rcDisplay.left + NH_INDENT;
			else
				m_curPos.x = m_rcDisplay.left + ((m_rcDisplay.right - m_rcDisplay.left) / 2);

			if (m_EffectType == EFFECT_TYPING_LEFT)
				m_uTextOutFormat = TA_LEFT | TA_TOP | TA_NOUPDATECP;
			else if (m_EffectType == EFFECT_TYPING_CENTER)
				m_uTextOutFormat = TA_CENTER | TA_TOP | TA_NOUPDATECP;

			// y position
			m_curPos.y = m_rcDisplay.top;
			if (m_heightDrawText < orgHeight)
				m_curPos.y += (int)((orgHeight - m_heightDrawText) / 2);
		}

		int	OldAlign = ::SetTextAlign(hDC, m_uTextOutFormat);
		
		if (m_EffectType == EFFECT_SLIDING_LEFT)
		{
			ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &m_rcDisplay, strDrawText, strDrawText.GetLength(), NULL);

			if ((m_curPos.x + m_widthDrawText) < m_rcDisplay.left)
				m_curPos.x = m_rcDisplay.right;

			m_curPos.x -= m_moveWidth;
		}
		else if (m_EffectType == EFFECT_SLIDING_RIGHT)
		{
			ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &m_rcDisplay, strDrawText, strDrawText.GetLength(), NULL);

			if (m_curPos.x > m_rcDisplay.right)
				m_curPos.x = m_rcDisplay.left - m_widthDrawText;

			m_curPos.x += m_moveWidth;
		}
		else
		{
			if (m_ShowTextCount > strDrawText.GetLength())
				ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &m_rcDisplay, strDrawText, strDrawText.GetLength(), NULL);
			else
				ExtTextOut(hDC, m_curPos.x, m_curPos.y, ETO_CLIPPED, &m_rcDisplay, strDrawText, m_ShowTextCount, NULL);

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

/** *************************************************************
*	@brief CNEditTextBox 생성자 
*	@param	CTL_TYPE type	control type
*****************************************************************/
CNEditTextBox::CNEditTextBox(CTL_TYPE type) : CNTextBox(type)
{
	m_bAutoRun = FALSE;
	m_nMaxInputCharOrigin = m_nMaxInputChar = 0;
	m_nMinInputChar = 0;	// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	m_bKeyDisable = FALSE;
}

CNEditTextBox::~CNEditTextBox()
{
}

BOOL CNEditTextBox::Initialize(void)
{
	CNShape::Initialize();

	m_strText = _T("");
	m_strTemp = _T("");
	m_nMaxInputChar = m_nMaxInputCharOrigin;

	return TRUE;
}

BOOL CNEditTextBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		if (!_tcsncmp(strData, DES_AP_TEXT_ID, 9) && (strData.GetLength() > 9))
		{
			CString strTextID = strData.Mid(9);

			// [#414] AIREAT 2008.09.05
			CNResource *pResource = NULL;
			if (g_NHResources.FindDataOfList(strTextID, &pResource))
				SetLocaleText(pResource);
			// end of [#414]
		}
		else if (!_tcsncmp(strData, DES_MAX_IN_CHAR, 10) && (strData.GetLength() > 10))
		{
			int nMaxInChar;

			nMaxInChar = _ttoi(strData.Mid(10));
			if (nMaxInChar > 0)
				m_nMaxInputChar = nMaxInChar;
		}
		else
		{
			m_strText = strData;
			m_strText.Replace(_T("&"), _T("&&"));
			m_pLocaleText = NULL;
		}
		
		AddInvalidRect(m_rcDisplay);
		
		return TRUE;
	}
	return FALSE;
}

BOOL CNEditTextBox::KeyEvent(CString &strKey)
{
//	if (m_bFocus)
	if ( (TRUE == m_bFocus) && (FALSE == m_bKeyDisable) )
	{
		// ENTER KEY	[#18]
		if (strKey == DES_ENTER)
		{
			// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
			if( m_nMinInputChar > 0 && m_strText.GetLength() < m_nMinInputChar)
			{
				return TRUE;
			}
			// end of [#182]		
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
					if (m_strText.GetLength() == 1)
						m_Act.m_Data = _T("0") + m_strText;
					else
						m_Act.m_Data = m_strText;

					m_strText = _T("");

					AddRequest(m_Act.GetKind(), NULL, m_Act.m_Data);
				}
				else
					AddRequest(m_Act);
			}

			// kill focus.
			if (!m_bAutoFocus)
				m_bFocus = FALSE;
			
			AddInvalidRect(m_rcDisplay);
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

			AddInvalidRect(m_rcDisplay);
			return TRUE;
		}
		// numeric key.
		//else if ((strKey.GetLength() == 1) && (strKey != DES_CANCEL))							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		else if ((strKey.GetLength() == 1) ||														// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
				 (m_TextType == TTYPE_IPADDRESS && strKey == DES_STAR))							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		{
			if (m_TextType == TTYPE_IPADDRESS && strKey == DES_STAR)								// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
				strKey = _T(".");																	// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.

			if (m_strText.GetLength() < m_nMaxInputChar)
			{
				m_strText += strKey;
				AddInvalidRect(m_rcDisplay);		
			}
				// Shift effect. - only support #901
			else if ((m_strText.GetLength()+1) > m_nMaxInputChar && m_Act.GetKind() == REQ_KEYEVENT)
			{
				m_strText += strKey;
				m_strText = m_strText.Right(m_nMaxInputChar);
				AddInvalidRect(m_rcDisplay);
			}

			// Auto Run..
			if (m_strText.GetLength() == m_nMaxInputChar && m_bAutoRun)
			{
				AddRequest(REQ_KEYEVENT, _T(""), DES_ENTER);
			}

			return TRUE;
		}
		else if (strKey == DES_STAR)
		{
			if (m_strText.GetLength() < m_nMaxInputChar)
			{
				m_strText += _T(".");
				AddInvalidRect(m_rcDisplay);
			}
			
			return TRUE;
		}
		else if (strKey == DES_SHARP)
		{
			if (m_strText.GetLength() < m_nMaxInputChar)
			{
				m_strText += _T(",");
				AddInvalidRect(m_rcDisplay);
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
				
				AddInvalidRect(m_rcDisplay);
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNEditTextBox::MakeAllFSCMD(CString &Data)
{
	Data += SCR_RES_DELIMITER + m_strText;
	
	m_strText = _T("");
				
	return FALSE;
}

BOOL CNEditTextBox::SetCtlFocus(CString &strName, CString &strData)
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

	AddInvalidRect(m_rcDisplay);
		
	return FALSE;
}

void CNEditTextBox::SetMaxInputChar(int nMaxChar)
{
	m_nMaxInputCharOrigin = nMaxChar;
}

// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
void CNEditTextBox::SetMinInputChar(int nChar)
{
	 m_nMinInputChar = nChar;
}
// end of [#182]

void CNEditTextBox::SetAutoRun(BOOL bAutoRun)
{
	m_bAutoRun = bAutoRun;
}

BOOL CNEditTextBox::SetKeyDisable(CString &strName)
{
	if (m_strName == strName)
		m_bKeyDisable = TRUE;
	else
		m_bKeyDisable = FALSE;

	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CNTableEditTextBox
//

CNTableEditTextBox::CNTableEditTextBox(CTL_TYPE type) : CNShape(type)
{
//	m_pPen = NULL;
//	m_pGuideFont = m_pFont;
//	m_pFont = m_pFocusFont = NULL;
//	m_pNormalElementBrush = m_pFocusElementBrush = NULL;
//	m_pNormalTextColor = m_pFocusTextColor = NULL;

	m_nAllowRow = 4;
	m_nAllowColumn = 40;

	int i;
	for (i = 0; i < 2; i++)
	{
		m_pGuideFont[i] = NULL;
		m_pGuideTextColor[i] = NULL;
		m_pGuideBrush[i] = NULL;
	}

	for (i = 0; i < MAX_TYPE; i++)
	{
		m_pElementFont[i] = NULL;
		m_pElementPen[i] = NULL;
		m_pElementBrush[i] = NULL;
		m_pElementTextColor[i] = NULL;
	}

	m_bSameKey = FALSE;
	m_bKeyNotZero = FALSE;


	InitializeValue();
}

CNTableEditTextBox::~CNTableEditTextBox()
{
}

void CNTableEditTextBox::SetAllowAmount(int nRow, int nColumn)
{
	m_nAllowRow = nRow;
	m_nAllowColumn = nColumn;
}

void CNTableEditTextBox::SetGuideFont(CNFont *pNormal, CNFont *pSelect)
{
	m_pGuideFont[0] = pNormal;
	m_pGuideFont[1] = pSelect;
}

void CNTableEditTextBox::SetGuideTextColor(CNColor *pNormal, CNColor *pSelect)
{
	m_pGuideTextColor[0] = pNormal;
	m_pGuideTextColor[1] = pSelect;
}

void CNTableEditTextBox::SetGuideBrush(CNBrush *pNormal, CNBrush *pSelect)
{
	m_pGuideBrush[0] = pNormal;
	m_pGuideBrush[1] = pSelect;
}

void CNTableEditTextBox::SetElementPen(CNPen *pDisable, CNPen *pEnable, CNPen *pSelect)
{
	m_pElementPen[DISABLE_TYPE] = pDisable;
	m_pElementPen[ENABLE_TYPE] = pEnable;
	m_pElementPen[SELECT_TYPE] = pSelect;
}

void CNTableEditTextBox::SetElementFont(CNFont *pDisable, CNFont *pEnable, CNFont *pSelect)
{
	m_pElementFont[DISABLE_TYPE] = pDisable;
	m_pElementFont[ENABLE_TYPE] = pEnable;
	m_pElementFont[SELECT_TYPE] = pSelect;
}

void CNTableEditTextBox::SetElementBrush(CNBrush *pDisable, CNBrush *pEnable, CNBrush *pSelect)
{
	m_pElementBrush[DISABLE_TYPE] = pDisable;
	m_pElementBrush[ENABLE_TYPE] = pEnable;
	m_pElementBrush[SELECT_TYPE] = pSelect;
}

void CNTableEditTextBox::SetElementTextColor(CNColor *pDisable, CNColor *pEnable, CNColor *pSelect)
{
	m_pElementTextColor[DISABLE_TYPE] = pDisable;
	m_pElementTextColor[ENABLE_TYPE] = pEnable;
	m_pElementTextColor[SELECT_TYPE] = pSelect;
}

BOOL CNTableEditTextBox::Initialize(void)
{
	CNShape::Initialize();

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

	HDC hDC = GetDC(NULL);
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

		//nElementWidth = tm.tmAveCharWidth + (tm.tmExternalLeading * 5);
		//nElementWidth = tm.tmMaxCharWidth + tm.tmExternalLeading + tm.tmExternalLeading;
		//nElementWidth = tm.tmMaxCharWidth + tm.tmExternalLeading;
		nElementWidth = tm.tmAveCharWidth*1.7 + (tm.tmExternalLeading * 5); //*LEH
		nElementHeight = tm.tmHeight*1.7 + (tm.tmExternalLeading * 4);

		ReleaseDC(NULL, hDC);
	}
	else
	{
		nElementWidth = 10;
		nElementHeight = 10;
	}

	// get start point
	int	nStartX = 0, nStartY = 0;

	nStartX = m_rcDisplay.left;
	nStartY = m_rcDisplay.top;

	if ((nElementWidth * MAX_TABLE_COL) < (m_rcDisplay.right - m_rcDisplay.left))
		nStartX += (int)(((m_rcDisplay.right - m_rcDisplay.left) - (nElementWidth * MAX_TABLE_COL)) / 2);

	//if ((nElementHeight * MAX_TABLE_ROW) < (m_rcDisplay.bottom - m_rcDisplay.top))
	if ((nElementHeight * (m_nAllowRow+1)) < (m_rcDisplay.bottom - m_rcDisplay.top))
		nStartY += (int)(((m_rcDisplay.bottom - m_rcDisplay.top) - (nElementHeight * (m_nAllowRow+1)))/2);

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
				left = nStartX + ((col+1) * nElementWidth);
				top = nStartY + ((row-1) * nElementHeight);
			}
			else
			{
				if (col == 0)
					left = nStartX-4;
				else
					left = nStartX-10 + ((col-1) * nElementWidth);
				top = nStartY + ((row-2) * nElementHeight) + (3 * (row-2));
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

BOOL CNTableEditTextBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes |= CNShape::DrawControl(hDC, rcUpdate);

	int	row, col;
	HFONT	OldFont = NULL;
	HPEN	OldPen = NULL;

	//////////////////////////////////////////////////////////////////////////
	// DRAW GUIDE STRING

	for (row = 1; row < MAX_TABLE_ROW && row-1 <= m_nAllowRow; row++)
	{
		for (col = 40; col < MAX_TABLE_COL; col++)
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
				HBRUSH		hOldBrush;
				
				// select brush
				if (m_pGuideBrush[nDrawIndex] != NULL)
					hOldBrush = (HBRUSH)::SelectObject(hDC, (HBRUSH)m_pGuideBrush[nDrawIndex]->GetHandle());
				else
					hOldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));

				::Rectangle(hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				
				// Restore brush
				::SelectObject(hDC, hOldBrush);

				// Select Font and Color
				if (m_pGuideFont[nDrawIndex] != NULL)
					hOldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pGuideFont[nDrawIndex]->GetHandle());
				if (m_pGuideTextColor[nDrawIndex] != NULL)
					nOldTextColor = ::SetTextColor(hDC, m_pGuideTextColor[nDrawIndex]->m_crColor);

				// Select to Transparent Mode in Background Mode
				int OldBkMode = ::SetBkMode(hDC, TRANSPARENT);

				if(1 == strText.GetLength())
					DrawText(hDC, strText, 1, lpRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				else
					DrawText(hDC, strText, 2, lpRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

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


				//*LEH
				/*
				::Rectangle(hDC,
							m_TableElement[row][col].rcRect.left,
							m_TableElement[row][col].rcRect.top,
							m_TableElement[row][col].rcRect.right,
							m_TableElement[row][col].rcRect.bottom);
				*/
				// Draw Rectangle.
				if (m_nRoundWidth == 0 && m_nRoundHeight == 0)
				{
					::Rectangle(hDC,
							m_TableElement[row][col].rcRect.left,
							m_TableElement[row][col].rcRect.top,
							m_TableElement[row][col].rcRect.right,
							m_TableElement[row][col].rcRect.bottom);
				}
				else
				{
					::RoundRect(hDC, m_TableElement[row][col].rcRect.left,
							m_TableElement[row][col].rcRect.top,
							m_TableElement[row][col].rcRect.right,
							m_TableElement[row][col].rcRect.bottom,
							m_nRoundWidth, m_nRoundHeight);
				}
				/////////////////////////////////////////

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

BOOL CNTableEditTextBox::KeyEvent(CString &strKey)
{
	{
		CString strNewKeyCode = _T("");
		CStringArray arrSaveText;
		BOOL bMoveCursor = TRUE;
		BOOL bCharKey = FALSE;
		int nCurrCol = 0;
		BOOL bTabAfterBlank = FALSE;

		if(HEXADECIMAL == g_nCurrEditMode)
		{
			if (strKey == _T("CLEARALL"))
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

					InitializeValue();
					SetCurrentTextLength();	// 현재 문자열 길이 산출

					AddInvalidRect(m_rcDisplay);
					return TRUE;
				}
			}
			else
			{
				if (strKey == DES_STAR)
					strKey = _T("A");
				else if (strKey == DES_SHARP)
					strKey = _T("B");
				else if (strKey == DES_CAPS)
					strKey = _T("C");
				else if (strKey == DES_ENTER)
					strKey = _T("D");
				else if (strKey == DES_CLEAR)
					strKey = _T("E");
				else if (strKey == DES_CANCEL)
					strKey = _T("F");

				if (strKey.GetLength() == 1)
				{
					if (m_pCurElement != NULL)
					{
						while (1)
						{
							if (m_pCurElement == &m_TableElement[m_nAllowRow+1][m_nAllowColumn])
							{
								m_pCurElement = &m_TableElement[m_nAllowRow+1][m_nAllowColumn];	//*LEH - Max 40자를 넘지 못하게 하기 위함.
								m_pCurElement->strText = strKey;
							}
							else 
							{	
								m_pCurElement->strText = strKey;
								m_pCurElement++;
							}

							if (m_pCurElement->bAllowSelect == TRUE)
								break;
						}

						SetCurrentTextLength();

						AddInvalidRect(m_rcDisplay);
						return TRUE;
					}
				}
			}
		}
		else
		{
			ConvInputKeyToNewText(strKey, strNewKeyCode, bMoveCursor);	//*LEH

			if (strKey.GetLength() == 1)
			{
				if (m_pCurElement != NULL)
				{
				//	strKey = DES_SHARP;	// move next pos and refresh.
					bCharKey = TRUE;	//*LEH - 0 ~ 9까지의 key 누름 여부

				//	if( (_T("9") == strKey) && (SPECIALCHAR == g_nCurrEditMode) )
					if( ((_T("9") == strKey) && (SPECIALCHAR == g_nCurrEditMode)) || ((_T("0") == strKey) && (NUMBER != g_nCurrEditMode)) )
						m_bCharInputComplete = FALSE;	// 특수문자 '9'(blank) 이후에도 모드 변경시, 커서가 움직이면 안됨. 문자모드에서 "0" 입력시에도 동일하게....
					else
						m_bCharInputComplete = TRUE;

					// 특수문자 입력 후, '9'(blank) insert 이후 특수문자 입력시 커서가 이동되면 안됨.
					// '9'(blank) 이후 다른 문자 입력시에는 Mode Change를 함으로써 bMoveCursor값이 FALSE가 됨.
				//	if( (TRUE == m_bInputCharAfterBlank) && (SPECIALCHAR == g_nCurrEditMode) && (FALSE == g_bTabKey) )
					if( (TRUE == m_bInputCharAfterBlank) && (NUMBER != g_nCurrEditMode) && (FALSE == g_bTabKey) )
					{
						bMoveCursor = FALSE;
						m_bInputCharAfterBlank = FALSE;
					}

					////////////////////////////////////////
					// 특수문자 '9'(blank) 입력 후, mode change 이후 다른 문자 또는 숫자 입력시 커서가 움직이면 안됨.
					if( (TRUE == m_bInputCharAfterBlank) && (SPECIALCHAR != g_nCurrEditMode) )
						bTabAfterBlank = TRUE;
					///////////////////////////////////////////////////////////////////////////////////

					// 문자열이 40자가 넘어갈 경우에는, 문자열 중간에 문자 입력시 insert가 아닌 overwrite가 됨.
					// 커서가 이동하면 안됨.
					if(!m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty())
						bMoveCursor = FALSE;

					m_strPrevInputKey = strKey;
				}
			}
			else if (strKey == DES_CLEAR)	//*LEH - Backspace
			{
				if (m_pCurElement != NULL)
				{
					while (1)
					{
						nCurrCol = m_pCurElement->nCol;
					//	if (m_pCurElement == &m_TableElement[0][0])
						if (m_pCurElement == &m_TableElement[m_nAllowRow+1][1])
						{
							m_pCurElement = &m_TableElement[m_nAllowRow+1][1];

							SaveCurrentText(_T(""), m_bInsertText, arrSaveText, m_bTextInputForBackspace, TRUE);

							m_pCurElement->strText.Empty();
							m_bTextInputForBackspace = FALSE;

							InputSaveText(arrSaveText, nCurrCol, _T(""), m_bInsertText, m_bInputCharAfterBlank, TRUE);
						}
						else
						{
							SaveCurrentText(_T(""), m_bInsertText, arrSaveText, m_bTextInputForBackspace, TRUE);

							// 현재 입력중인 문자는 문자만 삭제하고 커서는 이동하지 않도록 하기 위함.
							if(FALSE == m_bTextInputForBackspace)
							{
								m_pCurElement--;
								m_pCurElement->strText.Empty();
							}
							else
							{
								m_pCurElement->strText.Empty();
								m_bTextInputForBackspace = FALSE;
							}
							////////////////////////////////////////////////

							InputSaveText(arrSaveText, nCurrCol, _T(""), m_bInsertText, m_bInputCharAfterBlank, TRUE);
						}
							
						if (m_pCurElement->bAllowSelect == TRUE)
							break;
					}

					m_bCharInputComplete = FALSE;
					g_bTabKey = FALSE;
					g_bIsPrevNotNumberMode = FALSE;

					SetCurrentTextLength();

					AddInvalidRect(m_rcDisplay);
					return TRUE;
				}
			}

		//	if (strKey == DES_STAR || strKey == DES_SHARP)
			if (strKey == DES_STAR || strKey == DES_SHARP || TRUE == bCharKey)	//*LEH - DES_STAR: LEFT, DES_SHARP: RIGHT
			{
				if(TRUE != bCharKey)
				{
					m_bInsertText = TRUE;
					g_bTabKey = FALSE;
					g_bIsPrevNotNumberMode = FALSE;
				}
				else
				{
					if(!m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty())
						g_bTabKey = FALSE;

					// 숫자 이외의 모드에서는 Mode Change 이후 문자 입력시 커서를 이동시킨 후에 문자가 입력되어야 함.
					if( (TRUE == g_bTabKey) && (TRUE == g_bIsPrevNotNumberMode) && (FALSE == bTabAfterBlank) )
					{
						if (m_pCurElement != NULL)	// KSK 2009.9.9 Codesonar 지적사항 대책
						{
							while (1)
							{
							//	if (m_pCurElement == &m_TableElement[0][0])
								if (m_pCurElement == &m_TableElement[m_nAllowRow+1][m_nAllowColumn])
										m_pCurElement = &m_TableElement[m_nAllowRow+1][m_nAllowColumn];
								// 문자열 중간에서 문자 입력후에만 모드변경시 커서 이동함. 문자열이 40자 이상일 때에는 모드변경시 커서이동 안함.
							//	else if( ((NUMBER != g_nCurrEditMode) || ((NUMBER == g_nCurrEditMode) && (_T("0") == m_strPrevInputKey))) && (TRUE == m_bCharInputComplete) &&
								else if( (NUMBER != g_nCurrEditMode) && (TRUE == m_bCharInputComplete) &&
										 (m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty()) )
									m_pCurElement++;

								if (m_pCurElement->bAllowSelect == TRUE)
									break;
							}

							m_bCharInputComplete = FALSE;
							m_bTextInputForBackspace = FALSE;
						}
					}
				}

				if (m_pCurElement != NULL)	// KSK 2009.9.9 Codesonar 지적사항 대책
				{
					while (1)
					{
						if (DES_STAR == strKey)			// LEFT
						{
							//	if (m_pCurElement == &m_TableElement[0][0])
							if (m_pCurElement == &m_TableElement[m_nAllowRow+1][1])
								m_pCurElement = &m_TableElement[m_nAllowRow+1][1];	//*LEH - 계속 Left 이동시에는 커서가 계속 처음 위치에 있을 수 있게...
							else
								m_pCurElement--;

							m_bCharInputComplete = FALSE;
							m_bTextInputForBackspace = FALSE;
						}
						else if(DES_SHARP == strKey)	// RIGHT
						{
						//	if (m_pCurElement == &m_TableElement[MAX_TABLE_ROW-1][MAX_TABLE_COL-1])
							if (m_pCurElement == &m_TableElement[m_nAllowRow+1][m_nAllowColumn])
								m_pCurElement = &m_TableElement[m_nAllowRow+1][m_nAllowColumn];
							else
								m_pCurElement++;

							m_bCharInputComplete = FALSE;
							m_bTextInputForBackspace = FALSE;
						}
						else
						{
							//	if (m_pCurElement == &m_TableElement[MAX_TABLE_ROW-1][MAX_TABLE_COL-1])
							if (m_pCurElement == &m_TableElement[m_nAllowRow+1][m_nAllowColumn])
							{
								m_pCurElement = &m_TableElement[m_nAllowRow+1][m_nAllowColumn];	//*LEH - Max 40자를 넘지 못하게 하기 위함.
								m_pCurElement->strText = strNewKeyCode;
							}
							else 
							{	
								SetNewText(strKey, strNewKeyCode, bMoveCursor, m_bInsertText, m_bInputCharAfterBlank, bTabAfterBlank);
							}

							// backspace시에 현재 입력중인 문자를 삭제하기 위한 조건 설정
							if(m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty())
							{
							//	if( (NUMBER == g_nCurrEditMode) && (_T("0") != strKey) )
								if(NUMBER == g_nCurrEditMode)
									m_bTextInputForBackspace = FALSE;
								else
									m_bTextInputForBackspace = TRUE;
							}
							else
								m_bTextInputForBackspace = TRUE;
							/////////////////////////////////////////////////////////////////

							if(NUMBER == g_nCurrEditMode)
								g_bIsPrevNotNumberMode = FALSE;
							else
								g_bIsPrevNotNumberMode = TRUE;

							SetCurrentTextLength();

						}

						g_bTabKey = FALSE;
						
						if (m_pCurElement->bAllowSelect == TRUE)
							break;
					}
					
					AddInvalidRect(m_rcDisplay);
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
					
					AddInvalidRect(m_rcDisplay);
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

					InitializeValue();
					SetCurrentTextLength();
					
					AddInvalidRect(m_rcDisplay);
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
					
					AddInvalidRect(m_rcDisplay);
					return TRUE;
				}
			} 
			// Enter 키(OK)
			else if (strKey == _T("ENTER"))
			{
				InitializeValue();
			} 
			// Cancel 키(Cancel)
			else if (strKey == _T("CANCEL"))
			{
				InitializeValue();
			}
		}
	}

	return FALSE;
}

BOOL CNTableEditTextBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		CString			strTemp;
		CString			strValueName;
		CStringArray	strTempArray;
		int	i, j, nInsertIndex;

		int nLen;

		if (!_tcsncmp(strData, DES_MAX_IN_CHAR, 10) && (strData.GetLength() > 10))
		{
			int nMaxInChar;
			
			nMaxInChar = _ttoi(strData.Mid(10));
			if (nMaxInChar > 0)
				m_nAllowColumn = nMaxInChar;

			Initialize();
		}
		else
		{
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

				m_TableElement[m_nAllowRow][MAX_TABLE_COL-1].strText.Format(_T("%d"), nLen);
			}
			
			AddInvalidRect(m_rcDisplay);
		}
		
		return TRUE;
	}

	return FALSE;
}

BOOL CNTableEditTextBox::SetCtlFocus(CString &strName, CString &strData)
{
	return FALSE;
}

BOOL CNTableEditTextBox::MakeAllFSCMD(CString &Data)
{
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

/** *************************************************************
*	@brief		입력모드에 따른 문자로 변환
*	@details	'0' ~ '9'까지의 KeyPad에 해당하는 대문자/소문자/숫자/특수문자로 변환
*	@param CString strOrgKey	'0' ~ '9'까지의 KeyPad 값
*	@param CString strNewCode	변환된 대문자/소문자/숫자/특수문자
*	@param BOOL bMoveCursor		커서를 이동시켜야 하는지 여부
*	@retval		없음
*****************************************************************/
void CNTableEditTextBox::ConvInputKeyToNewText(CString strOrgKey, CString &strNewCode, BOOL &bMoveCursor)
{
	int nCharIndex = 0;

	m_bSameKey = FALSE;

	// Left 키
	if (strOrgKey.CompareNoCase(_T("STAR")) == 0)
	{
		m_strPrevKey = _T("");	// Left/Right/Mode Change/Backspace이후 문자입력시에는 커서가 이동하는 것을 막기 위함.
		m_bKeyNotZero = FALSE;
	}
	// Right 키
	else if (strOrgKey.CompareNoCase(_T("SHARP")) == 0)
	{
		m_strPrevKey = _T("");	// Left/Right/Mode Change/Backspace이후 문자입력시에는 커서가 이동하는 것을 막기 위함.
		m_bKeyNotZero = FALSE;
	}
	// Mode Change
	else if (strOrgKey.CompareNoCase(_T("CAPS")) == 0)
	{
		m_strPrevKey = _T("");	// Left/Right/Mode Change/Backspace이후 문자입력시에는 커서가 이동하는 것을 막기 위함.
		m_bKeyNotZero = FALSE;
	}
	// Clear 키(Backspace)
	else if (strOrgKey.CompareNoCase(_T("CLEAR")) == 0)
	{
		m_strPrevKey = _T("");	// Left/Right/Mode Change/Backspace이후 문자입력시에는 커서가 이동하는 것을 막기 위함.
		m_bKeyNotZero = FALSE;
	}
	// Enter 키(OK)
	else if (strOrgKey.CompareNoCase(_T("ENTER")) == 0)
	{
		m_strPrevKey = _T("");
		m_strPrevNewChar = _T("");
		m_bKeyNotZero = FALSE;
	} 
	// Cancel 키(Cancel)
	else if (strOrgKey.CompareNoCase(_T("CANCEL")) == 0)
	{
		m_strPrevKey = _T("");
		m_strPrevNewChar = _T("");
		m_bKeyNotZero = FALSE;
	}
	// Clear All 키
	else if (strOrgKey.CompareNoCase(_T("CLEARALL")) == 0)
	{
		m_strPrevKey = _T("");
		m_strPrevNewChar = _T("");
		m_bKeyNotZero = FALSE;
	}
	else if ( (strOrgKey.CompareNoCase(_T("0")) >= 0) && (strOrgKey.CompareNoCase(_T("9")) <= 0) )
	{
		// 대문자
		if ( (UPPERCASE == g_nCurrEditMode) || 
		// 소문자
				  (LOWERCASE == g_nCurrEditMode) ||
				  (SPECIALCHAR == g_nCurrEditMode) )	// @ADD: LEH - 특수문자 추가
		{
			CStringArray mapPinChar;

			if(TRUE == g_bTabKey)
			{
				m_strPrevKey = _T("");	// Left/Right/Mode Change/Backspace이후 문자입력시에는 커서가 이동하는 것을 막기 위함.
				m_bKeyNotZero = FALSE;
			}

			// 대문자
			if (UPPERCASE == g_nCurrEditMode)
			{
				mapPinChar.Add(_T("0"));	// 0
				mapPinChar.Add(_T("ABC"));	// 1
				mapPinChar.Add(_T("DEF"));	// 2
				mapPinChar.Add(_T("GHI"));	// 3
				mapPinChar.Add(_T("JKL"));	// 4
				mapPinChar.Add(_T("MNO"));	// 5
				mapPinChar.Add(_T("PRS"));	// 6
				mapPinChar.Add(_T("TUV"));	// 7
				mapPinChar.Add(_T("WXY"));	// 8
				mapPinChar.Add(_T("QZ"));	// 9
			}

			// 소문자
			else if (LOWERCASE == g_nCurrEditMode)
			{
				mapPinChar.Add(_T("0"));	// 0
				mapPinChar.Add(_T("abc"));	// 1
				mapPinChar.Add(_T("def"));	// 2
				mapPinChar.Add(_T("ghi"));	// 3
				mapPinChar.Add(_T("jkl"));	// 4
				mapPinChar.Add(_T("mno"));	// 5
				mapPinChar.Add(_T("prs"));	// 6
				mapPinChar.Add(_T("tuv"));	// 7
				mapPinChar.Add(_T("wxy"));	// 8
				mapPinChar.Add(_T("qz"));	// 9
			}
			//>>> @ADD: LEH - 특수문자 추가
			else if (SPECIALCHAR == g_nCurrEditMode)
			{
				mapPinChar.Add(_T("0"));		// 0
				mapPinChar.Add(_T("!\"#$"));	// 1
				mapPinChar.Add(_T("%&'("));		// 2
				mapPinChar.Add(_T(")*+,"));		// 3
				mapPinChar.Add(_T("-./:"));		// 4
				mapPinChar.Add(_T(";<=>"));		// 5
				mapPinChar.Add(_T("?@[\\"));	// 6
				mapPinChar.Add(_T("]^_`"));		// 7
				mapPinChar.Add(_T("{|}~"));		// 8
				mapPinChar.Add(_T(" "));		// 9
			}
			//<<< @ADD
	/*		else if(NUMBER == g_nCurrEditMode)
			{
				mapPinChar.Add(_T("0.,"));		// 0
				m_bKeyNotZero = TRUE;	// 숫자모드에서는 "0"을 누를 후에 다른 숫자를 누를 경우 커서가 한칸 더 이동해야 함.
			}
	*/
			nCharIndex = _ttoi(strOrgKey);

			if(m_strPrevKey == strOrgKey)
				m_bSameKey = TRUE;
			else
				m_bSameKey = FALSE;

			if(TRUE == m_bSameKey)	// 동일한 Key가 연속으로 눌렸을 경우
			{
				CString strPinChar = mapPinChar.GetAt(nCharIndex);

				if (-1 != strPinChar.Find(m_strPrevNewChar))
				{
					TCHAR cChangingChar = 0;

					for (int nCnt = 0; nCnt < strPinChar.GetLength(); nCnt++)
					{
						if (m_strPrevNewChar.GetAt(0) == strPinChar[nCnt])
						{
							cChangingChar = strPinChar[(nCnt + 1) % strPinChar.GetLength()];

							break;
						}
					}

					strNewCode.Format(_T("%c"), cChangingChar);
				}
				else
				{
					strNewCode.Format(_T("%c"), mapPinChar.GetAt(nCharIndex).GetAt(0));
				}

				// 특수문자의 '9'는 blank 삽입이기 때문에 연속으로 누르더라도 커서가 이동되어야 함.
				if( (_T("9") == strOrgKey) && (SPECIALCHAR == g_nCurrEditMode) )
					bMoveCursor = TRUE;
				else
					bMoveCursor = FALSE;

				if(_T("0") == strOrgKey)
					bMoveCursor = TRUE;
			}
			else
			{
				strNewCode.Format(_T("%c"), mapPinChar.GetAt(nCharIndex).GetAt(0));

				if(m_strPrevKey == _T(""))
					bMoveCursor = FALSE;
				else
					bMoveCursor = TRUE;

			//	if( (_T("9") == strOrgKey) && (SPECIALCHAR == g_nCurrEditMode) )
			//		bMoveCursor = TRUE;
			}

			m_strPrevKey = strOrgKey;			// 동일한 Key가 눌려졌는지 체크하기 위함.
			m_strPrevNewChar = strNewCode;
		}

		// 숫자
		else if (NUMBER == g_nCurrEditMode)
		{
			strNewCode = strOrgKey;

			bMoveCursor = TRUE;
			m_strPrevKey = _T("");		// 숫자모드 입력 후, 문자모드로 변경시 커서가 이동하면 안됨.
			m_strPrevNewChar = _T("");
		}
	}
}

/** *************************************************************
*	@brief		입력모드에 해당하는 새로운 문자를 TableEditBox에 입력한다.
*	@details	변환한 새로운 문자를 insert 및 blank 처리 등 후 TableEditBox에 입력한다.
*	@param CString strKey		'0' ~ '9'까지의 KeyPad 값
*	@param CString strNewCode	변환된 대문자/소문자/숫자/특수문자
*	@param BOOL &bMoveCursor	커서를 이동시켜야 하는지 여부
*	@param BOOL &bInsertText	Insert 수행 여부
*	@param BOOL &bInputCharAfterBlank		특수문자 '9'(blank) 이후에 특수문자가 입력되는지 여부
*	@param BOOL bTabAfterBlank				특수문자 '9'(blank) 이후에 특수문자 이외의 값이 입력되는지 여부
*	@retval		없음
*****************************************************************/
void CNTableEditTextBox::SetNewText(CString strKey, CString strNewKeyCode, BOOL bMoveCursor, BOOL &bInsertText, BOOL &bInputCharAfterBlank, BOOL bTabAfterBlank)
{
	int nCurrCol = 0;
	CStringArray arrSaveText;
	BOOL bCursorNotMove = FALSE;

	nCurrCol = m_pCurElement->nCol;

	SaveCurrentText(strKey, bInsertText, arrSaveText);

	//*LEH - 숫자모드에서는 숫자를 입력 후 커서가 이동되어야 한다.
//	if( (NUMBER == g_nCurrEditMode) && (_T("0") != strKey) )
	if(NUMBER == g_nCurrEditMode)
	{
		if( (TRUE == g_bTabKey) && (TRUE == g_bIsPrevNotNumberMode) && (FALSE == bTabAfterBlank) )
		{
			m_pCurElement++;
			nCurrCol = m_pCurElement->nCol;
		}

		// 숫자모드에서는 "0"을 누른 후에 다른 숫자를 누를 경우 커서가 한칸 더 이동해야 함. ==> 문자모드에서 문자 입력 후 "0"을 입력할 경우 커서가 한칸 더 이동해야 함.
		if(TRUE == m_bKeyNotZero)
		{
			if( (m_pCurElement->nCol < m_nAllowColumn+1) && (m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty()) )
			{
				m_pCurElement++;
				nCurrCol = m_pCurElement->nCol;
			}
			m_bKeyNotZero = FALSE;
		}
		//////////////////////////////////////////////////////////////////////////////////

		m_pCurElement->strText = strNewKeyCode;

		if(!m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty())
			bCursorNotMove = TRUE;
	}
	//////////////////////////////////////////////////

	if( (TRUE == bMoveCursor) && (FALSE == bCursorNotMove) )
		m_pCurElement++;

	//*LEH - 숫자모드가 아닌 경우에는 문자를 입력 전에 커서가 이동되어야 한다.
//	if( (NUMBER != g_nCurrEditMode) || ((NUMBER == g_nCurrEditMode) && (_T("0") == strKey)) )
	if( (NUMBER != g_nCurrEditMode))
	{
		if (strNewKeyCode == _T("&"))
			m_pCurElement->strText = _T("&&");
		else
			m_pCurElement->strText = strNewKeyCode;
	}
	///////////////////////////////////////////////////

	InputSaveText(arrSaveText, nCurrCol, strKey, bInsertText, bInputCharAfterBlank);
}

/** *************************************************************
*	@brief		현재 위치의 문자부터 마지막 문자까지 임시 저장한다.
*	@details	현재 위치에 문자를 insert하기 위함.
*	@param CString strKey		'0' ~ '9'까지의 KeyPad 값
*	@param BOOL &bInsertText	Insert 수행 여부
*	@param CStringArray &arrSaveText	TableEditBox에 있는 문자열을 저장하는 변수
*	@param BOOL bTextInputForBackspace	현재 입력중인 문자를 backspace 하기 위한 조건
*	@param BOOL bBackspace		현재 동작이 backspace인지 여부
*	@retval		없음
*****************************************************************/
void CNTableEditTextBox::SaveCurrentText(CString strKey, BOOL &bInsertText, CStringArray &arrSaveText, BOOL bTextInputForBackspace/* = FALSE */, BOOL bBackspace/* = FALSE */)
{
//	CString strTempText = _T("");
	int i = 0;
	int nCurrCol = 0;
	int nEndCol = 0;

	nCurrCol = m_pCurElement->nCol;

	// empty가 아닌 index 산출
	for(i=m_nAllowColumn; i>0; i--)
	{
		if(!m_TableElement[m_nAllowRow+1][i].strText.IsEmpty())
		{
			nEndCol = i;
			break;
		}
	}

	// 문자를 문자열 중간에 insert하기 위해 문자열을 임시변수에 저장한다.
	// 문자 입력 후에 임시변수에 저장된 문자들을 다시 TableEdit에 넣어준다.
	if(TRUE == bInsertText)
	{
		// 현재 위치 index가 전체 문자열의 중간에 있을 경우 insert로 판단함.
		if(nCurrCol > nEndCol)
			bInsertText = FALSE;
		else
		{
			for(i=0; i<nEndCol; i++)	// MAX_TABLE_COL = 41
			{
				if(m_TableElement[m_nAllowRow+1][i+1].strText.IsEmpty())
					arrSaveText.Add(_T(""));
				else
					arrSaveText.Add(m_TableElement[m_nAllowRow+1][i+1].strText);
			}

			// 문자열 중간에 문자 insert시, 동일한 버튼을 연속적으로 입력할 경우 해당 위치의 문자는 지우고 다시 쓴다.
			// 특수문자 '9'(blank)는 연속으로 입력해도 띄어쓰기가 계속 되어야 함.
			if( (TRUE == m_bSameKey) && ( ((_T("9") != strKey) || (SPECIALCHAR != g_nCurrEditMode)) && 
										  ((_T("0") != strKey) && (NUMBER != g_nCurrEditMode)) ) )
				arrSaveText.RemoveAt(nCurrCol-1);

			// 문자열 중간에 입력중인 문자를 지우기 위함.
			if( (TRUE == bTextInputForBackspace) && (TRUE == bBackspace) )
				arrSaveText.RemoveAt(nCurrCol-1);
		}
	}
}

/** *************************************************************
*	@brief		문자열 중간에 insert/delete 후, 임시저장된 문자열을 insert/delete 다음 위치부터 채워준다.
*	@details	없음.
*	@param CStringArray &arrSaveText	SaveCurrentText() 함수에서 저장한 Text
*	@param int nCurrCol			Insert 하기 위한 현재 위치
*	@param CString strKey		'0' ~ '9'까지의 KeyPad 값
*	@param BOOL bInsertText		Insert 수행 여부
*	@param BOOL &bInputCharAfterBlank		특수문자 '9'(blank) 이후에 특수문자가 입력되는지 여부
*	@param BOOL bBackspace		현재 동작이 backspace인지 여부
*	@retval		없음
*****************************************************************/
void CNTableEditTextBox::InputSaveText(CStringArray &arrSaveText, int nCurrCol, CString strKey, BOOL bInsertText, BOOL &bInputCharAfterBlank, BOOL bBackspace/* =FALSE */)
{
	CString strTempChar = _T("");
	int i = 0;

	// 문자열이 40자가 넘어갈 경우에는, 문자열 중간에 문자 입력시 insert가 아닌 overwrite가 됨.
	if( (TRUE == bInsertText) && ((TRUE == bBackspace) || ((FALSE == bBackspace) && (m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty()))) )
	{
		// ex) "123456" => '5' 위치까지 커서 이동후 '9'입력시 strTempText.GetLength() = 6, nCurrCol = 5이기 때문에,
		// '5'와 '6'을 뒤로 한칸씩 이동시킨후 커서를 원위치 시키기 위해서는 두칸을 왼쪽으로 이동시켜야 함.
		int nAppendLength = 0;

//		if( (NUMBER != g_nCurrEditMode) || ((NUMBER == g_nCurrEditMode) && (_T("0") == strKey)) )
		if(NUMBER != g_nCurrEditMode)
		{
			if(FALSE == bBackspace)
				nCurrCol = m_pCurElement->nCol;
			nAppendLength = arrSaveText.GetSize() - nCurrCol + 1;
		}
		else
			nAppendLength = arrSaveText.GetSize() - nCurrCol + 1;
		////////////////////////////////////////////////////////////////////////////////////

		// insert 후, 기존 문자들 한칸씩 밀기
		for(i=nCurrCol-1; i<arrSaveText.GetSize(); i++)	// m_pCurElement의 최초 문자의 col(index)은 1이다.
		{
		//	if( (NUMBER == g_nCurrEditMode) && (_T("0") != strKey) )	// 숫자모드
			if(NUMBER == g_nCurrEditMode)	// 숫자모드
			{
				m_pCurElement->strText = arrSaveText.GetAt(i);
				m_pCurElement++;
			}
			else
			{
				if(FALSE == bBackspace)
					m_pCurElement++;	// 문자 insert시에는 커서를 이동시키고 문자를 채움.

				strTempChar = arrSaveText.GetAt(i);
				if (strTempChar == _T("&"))
					m_pCurElement->strText = _T("&&");		//*LEH - 이상하게 적용됨....................................................
				else
					m_pCurElement->strText = strTempChar;

				if(TRUE == bBackspace)
					m_pCurElement++;	// backspace시에는 문자를 채우고 커서를 이동시키야 함.
			}
		}

		if(TRUE == bBackspace)
			m_pCurElement->strText.Empty();	// backspace시 문자들이 하나씩 왼쪽으로 이동하면서 마지막 글자는 지워야 함.

		// 커서를 원래 위치까지 원위치
		for(i=0; i<nAppendLength; i++)
			m_pCurElement--;

		// 특수문자 입력 후, '9'(blank) insert시에는 무조건 커서가 이동되어야 함.
		// 다른문자 입력 후에 '9'(blank) insert 하기 위해서는 입력모드를 바꿔야 하는데, Mode change 버튼을 누르게 되면 커서는 한칸 이동된다.
//		if( (_T("9") == strKey) && (SPECIALCHAR == g_nCurrEditMode) )
		if( ((_T("9") == strKey) && (SPECIALCHAR == g_nCurrEditMode)) ||
			((_T("0") == strKey) && (NUMBER != g_nCurrEditMode)) )	// 문자 입력 후, 바로 "0" 입력시 무조건 커서가 이동되어야 함.
		{
			if(m_pCurElement->nCol < m_nAllowColumn+1)
				m_pCurElement++;

			bInputCharAfterBlank = TRUE;
		}
		///////////////////////////////////////////////////

		// 기본적으로 문자열이 40자가 되는 순간, insert가 아니고 overwrite가 되어야 함.
		// 숫자모드에서는 입력후에 커서가 이동하기 때문에, insert 후 문자열이 40자가 된다면 커서가 이동하면 안됨.
	//	if( (NUMBER == g_nCurrEditMode) && (_T("0") != strKey) && (!m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty()))
		if( (NUMBER == g_nCurrEditMode) && (!m_TableElement[m_nAllowRow+1][m_nAllowColumn].strText.IsEmpty()) && (m_pCurElement->nCol > 1) )
			m_pCurElement--;
	}
}

/** *************************************************************
*	@brief		화면로드시, 원하는 입력모드를 설정할 수 있도록 한다.
*	@details	없음
*	@param CString strInputMode		설정할 입력모드
*	@retval		FALSE
*****************************************************************/
BOOL CNTableEditTextBox::SetInputMode(CString &strInputMode)
{
	if(strInputMode.CompareNoCase(_T("NUMBER")) == 0)
		g_nCurrEditMode = NUMBER;
	else if(strInputMode.CompareNoCase(_T("UPPERCASE")) == 0)
		g_nCurrEditMode = UPPERCASE;
	else if(strInputMode.CompareNoCase(_T("LOWERCASE")) == 0)
		g_nCurrEditMode = LOWERCASE;
	else if(strInputMode.CompareNoCase(_T("SPECIALCHAR")) == 0)
		g_nCurrEditMode = SPECIALCHAR;
	else
		g_nCurrEditMode = NUMBER;

	InitializeValue();

	return FALSE;
}

/** *************************************************************
*	@brief		문자 입력을 위한 관련 변수 초기화
*	@retval		없음
*****************************************************************/
void CNTableEditTextBox::InitializeValue()
{
	m_bCharInputComplete = FALSE;
	m_bInsertText = TRUE;
	m_bInputCharAfterBlank = FALSE;
	m_strPrevInputKey = _T("");
	m_bTextInputForBackspace = FALSE;		// 현재 입력중인 문자열인지 판단하는 변수
	m_strPrevKey = _T("");
	m_strPrevNewChar = _T("");
	g_bTabKey = FALSE;
	g_bIsPrevNotNumberMode = FALSE;
}

/** *************************************************************
*	@brief		입력한 문자열의 길이 산출
*	@retval		없음
*****************************************************************/
void CNTableEditTextBox::SetCurrentTextLength()
{
	int i = 0;
	int nCurrentTextLen = 0;
	// empty가 아닌 index 산출
	for(i=m_nAllowColumn; i>0; i--)
	{
		if(!m_TableElement[m_nAllowRow+1][i].strText.IsEmpty())
		{
			nCurrentTextLen = i;
			break;
		}
	}
	
	m_TableElement[m_nAllowRow][MAX_TABLE_COL-1].strText.Format(_T("%d"), nCurrentTextLen);
}

///////////////////////////////////////////////////////////////
//
//	CNButtonBox
//
CNButtonBox::CNButtonBox(CTL_TYPE type) : CNTextBox(type)
{
	m_bShow = TRUE;
	m_bAlwaysShow = FALSE;
}

CNButtonBox::~CNButtonBox()
{
}

BOOL CNButtonBox::Initialize(void)
{
	CNShape::Initialize();

	if (m_strName.GetLength() > 0)
	{
		// Clear.
		if (_tcsncmp((LPCTSTR)m_strName, DES_AP_VALUE, _tcslen(DES_AP_VALUE)) == 0)
		{
			m_strText = _T("");
			if(FALSE == m_bAlwaysShow)
				m_bShow = FALSE;		// APValue는 값이 설정되어야 보인다.
			else
				m_bShow = TRUE;
		}
		// Hide.
		else if (_tcsncmp((LPCTSTR)m_strName, DES_AP_STATE, _tcslen(DES_AP_STATE)) == 0)
		{
			m_bShow = TRUE;
		}
	}

	return TRUE;
}

BOOL CNButtonBox::FindActBtn(CString &strKey)
{
	return KeyEvent(strKey);
}

// [2ND] NH AIREAT 2008.11.12
BOOL CNButtonBox::MouseEvent(POINT &point)
{
	if (m_bShow)
	{
		if (PtInRect(&m_rcDisplay, point))
		{
			if(m_ActKey.IsEmpty() == TRUE)
				m_ActKey = m_Act.GetData();

			return KeyEvent(m_ActKey);
		}
	}
	
	return FALSE;
}
// end of [2ND]

BOOL CNButtonBox::KeyEvent(CString &strKey)
{
	if (m_bShow)
	{
		// 1자는 [1, F1]을 Find에서 같게 처리함으로 제외함.
		if (strKey.GetLength() > 1 && m_ActKey.Find(strKey) >= 0)
	//	if (m_ActKey.Find(strKey) >= 0)
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
			{
				AddRequest(m_Act);
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNButtonBox::SetVariable(CString &strName, CString &strData)
{
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

			// 만약 actkey가 없으면 이벤트를 처리하지 않은것으로 간주한다.
			if (m_ActKey.GetLength() == 0)
				return FALSE;

			return TRUE;
		}
		// APValueXX
		else
		{
			m_strText = strData;
			
			// 값이 설정되어 있으면 Show, 그렇지 않으면 Hide
			if (m_strText.GetLength() == 0)
			{
				if(FALSE == m_bAlwaysShow)
					m_bShow = FALSE;
				else
					m_bShow = TRUE;
			}
			else
				m_bShow = TRUE;

			AddInvalidRect(m_rcDisplay);
			
			return TRUE;
		}

	}

	return FALSE;
}

BOOL CNButtonBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	if (m_bShow)
	{
		return CNTextBox::DrawControl(hDC, rcUpdate);
	}

	return FALSE;
}

void CNButtonBox::SetTextType(CString strType)
{
	if(DES_ALWAYS_SHOW == strType)
		m_bAlwaysShow = TRUE;
}

BOOL CNButtonBox::SetCtlFocus(CString &strName, CString &strData)
{
	if(TRUE == m_bAlwaysShow)
	{
		if (m_strName == strName)
		{
			m_bFocus = TRUE;
		}
		else
		{
			m_bFocus = FALSE;
		}
		
		AddInvalidRect(m_rcDisplay);
	}
	
	return FALSE;
}

/** *************************************************************
*	@brief	모든 control의 data를 가져오기 위함.
*	@param	CString &Data	결과 data
*	@retval	없음
*****************************************************************/
BOOL CNButtonBox::MakeAllFSCMD(CString &Data)
{
	if(TRUE == m_bAlwaysShow)
	{
		Data += SCR_RES_DELIMITER + m_strText;
		
		m_strText = _T("");
	}
	
	return FALSE;
}


///////////////////////////////////////////////////////////////
//
//	CNVKeyButtonBox
//

CNVKeyButtonBox::CNVKeyButtonBox(CTL_TYPE type) : CNButtonBox(type)
{
}

CNVKeyButtonBox::~CNVKeyButtonBox()
{
}

BOOL CNVKeyButtonBox::Initialize(void)
{
	if (m_bAutoFocus)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	return TRUE;
}

BOOL CNVKeyButtonBox::SetCtlFocus(CString &strName, CString &strData)
{
	if (m_strName == strName)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	AddInvalidRect(m_rcDisplay);

	return FALSE;
}

BOOL CNVKeyButtonBox::SetVariable(CString &strName, CString &strData)
{
	return FALSE;
}

/** *************************************************************
*	@brief		마우스 이벤트
*	@details	VKeyButton 선택시 해당 문자 입력되도록
*	@param POINT &point	선택 좌표
*	@retval		TRUE
*	@retval		FALSE
*****************************************************************/
BOOL CNVKeyButtonBox::MouseEvent(POINT &point)
{
	if (PtInRect(&m_rcDisplay, point))
	{
		if (m_bShow)
		{
			CRequest Req;
			Req.Set(REQ_KEYEVENT, _T(""), m_ActKey);
			AddRequest(Req);
		}
	}
	
	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CNChangePWBox
//

CNChangePWBox::CNChangePWBox(CTL_TYPE type) : CNEditTextBox(type)		// [#16] NH KGS 2008.03.13 CNTextBox -> CNEditTextBox
{
	// Password type.
	m_ChangePW[CPTYPE_CUR].SetTextType(TTYPE_PASSWORD);
	m_ChangePW[CPTYPE_NEW].SetTextType(TTYPE_PASSWORD);
	m_ChangePW[CPTYPE_NEWRE].SetTextType(TTYPE_PASSWORD);


	m_nMaxInputChar = 0;
}

CNChangePWBox::~CNChangePWBox()
{
}

void CNChangePWBox::SetPWBoxRect(CHANGEPWTYPE type, int left, int top, int width, int height)
{
	//if (type > CPTYPE_MAX && type < 0)
	if (type > CPTYPE_MAX)					// [CODESONAR]  /* Redundant Condition (ID: 102) */
		return;

	m_ChangePW[type].SetDisplayRect(left, top, width, height);
}

BOOL CNChangePWBox::Initialize(void)
{
	CNTextBox::Initialize();

	for (int i = CPTYPE_CUR; i < CPTYPE_MAX; i++)
	{
		
		// brush
		m_ChangePW[i].SetBrush(this->m_pBrush, this->m_pFocusBrush);
		
		// Pen
		m_ChangePW[i].SetPen(this->m_pPen, this->m_pFocusPen);
		
		// Font
		m_ChangePW[i].SetFont(this->m_pFont, this->m_pFocusFont);
		
		// Align
		m_ChangePW[i].SetTextAlign(this->m_Align);
		
		// Text Color
		//m_ChangePW[i].SetTextColor(this->m_crTextColor, this->m_crFocusTextColor);
		m_ChangePW[i].SetTextColor(this->m_pColor, this->m_pFocusColor);
		
		// Max Input Char
		m_ChangePW[i].SetMaxInputChar(m_nMaxInputCharOrigin);
		m_ChangePW[i].SetMinInputChar(m_nMinInputChar);		// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가

		m_ChangePW[i].SetRoundSize(this->m_nRoundWidth, this->m_nRoundHeight);	//*LEH
		
		m_ChangePW[i].SetAutoRun(m_bAutoRun);		//	[#16] NH KGS 2008.03.17 Auto Run
		m_ChangePW[i].Initialize();
//		m_ChangePW[i].SetFocus(TRUE);				[#16] NH KGS 2008.03.17 Focus Bug Fix
	}
	
	m_ChangePW[CPTYPE_CUR].SetFocus(TRUE);		// [#16] NH KGS 2008.03.17 	Focus Bug Fix

	m_curInputBox = CPTYPE_CUR;

	return TRUE;
}

BOOL CNChangePWBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL		bRes = FALSE;
	CRequest	req;
	
	bRes |= m_ChangePW[CPTYPE_CUR].DrawControl(hDC, rcUpdate);
	bRes |= m_ChangePW[CPTYPE_NEW].DrawControl(hDC, rcUpdate);
	bRes |= m_ChangePW[CPTYPE_NEWRE].DrawControl(hDC, rcUpdate);

	return bRes;
}

BOOL CNChangePWBox::KeyEvent(CString &strKey)
{
	if (m_bFocus)
	{
		CRequest	req;

		// ENTER KEY
		if (strKey == DES_ENTER)
		{
			if (m_curInputBox < CPTYPE_MAX)
			{
				// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
				if( m_nMinInputChar > 0 && (int)_tcslen(m_ChangePW[m_curInputBox].GetInputText()) < m_nMinInputChar)
				{
					return TRUE;
				}
				// end of [#182]
				if (m_curInputBox == CPTYPE_CUR)
				{
					// [#651] NH SOOK 2010.09.07 
					if( m_strText.GetLength() > m_nMinInputChar ) //Password가 (Login Password, Current Password)두개 왔을 경우임 
					{
						// same current password (or login password) and input password 
						if ( m_strText.Left(6) == m_ChangePW[CPTYPE_CUR].GetInputText()
							|| m_strText.Right(6) == m_ChangePW[CPTYPE_CUR].GetInputText())
						{
							m_curInputBox = CPTYPE_NEW;

							m_ChangePW[CPTYPE_CUR].SetFocus(FALSE);
							m_ChangePW[CPTYPE_NEW].SetFocus(TRUE);
							m_ChangePW[CPTYPE_NEWRE].SetFocus(FALSE);
						}
						else
						{
							AddRequest(REQ_FSCMD, m_Act.GetName(), _T("WRONGPASSWORD"));

							req.Set(REQ_KEYEVENT, _T(""), DES_CLEAR);
							m_ChangePW[CPTYPE_CUR].Handler(req);
						}
					}
					else   //Password가 한개일 경우 
					{
					//end of [#651]
						// same current password and input password
						if (m_strText == m_ChangePW[CPTYPE_CUR].GetInputText())
						{
							m_curInputBox = CPTYPE_NEW;

							m_ChangePW[CPTYPE_CUR].SetFocus(FALSE);
							m_ChangePW[CPTYPE_NEW].SetFocus(TRUE);
							m_ChangePW[CPTYPE_NEWRE].SetFocus(FALSE);
						}
						// don't same current password and input password
						else
						{
							AddRequest(REQ_FSCMD, m_Act.GetName(), _T("WRONGPASSWORD"));

							req.Set(REQ_KEYEVENT, _T(""), DES_CLEAR);
							m_ChangePW[CPTYPE_CUR].Handler(req);
						}
					}
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

						// [#16] NH KGS 2008.03.13 Password Verification 실패시 New Password부터 다시 입력
						req.Set(REQ_INITIAL, _T(""), _T(""));
						// end of [#16]
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
		else if (strKey == DES_CANCEL)//(wcscmp(pkey, DES_CANCEL) == 0)
		{
			return FALSE;
		}
		// [#236] NZ AIREAT 2008.06.04 - TIMEOUT
		else if (strKey == DES_TIMEOVER)
		{
			return FALSE;
		}
		// end of [#236]
		// ELSE
		else
		{
			req.Set(REQ_KEYEVENT, _T(""), strKey);//req.Set(REQ_KEYEVENT, _T(""), pkey);
			m_ChangePW[m_curInputBox].Handler(req);
		}

		// [#16] NH KGS 2008.03.13 Password Verification 실패시 New Password부터 다시 입력
		// AddInvalidRect(m_rcDisplay);
		AddInvalidRect(m_ChangePW[CPTYPE_CUR].GetDisplayRect());
		AddInvalidRect(m_ChangePW[CPTYPE_NEW].GetDisplayRect());
		AddInvalidRect(m_ChangePW[CPTYPE_NEWRE].GetDisplayRect());
		// end of [#16]
		return TRUE;
	}

	return FALSE;
}

BOOL CNChangePWBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		m_strText = strData;
		return TRUE;
	}
	
	return FALSE;
}

BOOL CNChangePWBox::MakeAllFSCMD(CString &Data)
{
	Data += SCR_RES_DELIMITER;
	Data += m_ChangePW[CPTYPE_NEWRE].GetInputText();
	
	return FALSE;
}


///////////////////////////////////////////////////////////////
//
//	CNSWKeyBox
//

CNSWKeyBox::CNSWKeyBox(CTL_TYPE type) : CNTextBox(type)
{
	m_sizeNumber.cx = 0;
	m_sizeNumber.cy = 0;

	m_sizeGuide.cx = 0;
	m_sizeGuide.cy = 0;
	
	m_sizeSpace.cx = 0;
	m_sizeSpace.cy = 0;

	m_nSwKeyDataIndex = 0;
}

CNSWKeyBox::~CNSWKeyBox()
{
}

void CNSWKeyBox::SetNumberSize(SIZE Number)
{
	m_sizeNumber = Number;
}

void CNSWKeyBox::SetGuideSize(SIZE Gudie)
{
	m_sizeGuide = Gudie;
}

void CNSWKeyBox::SetSpaceSize(SIZE Space)
{
	m_sizeSpace = Space;
}

BOOL CNSWKeyBox::Initialize(void)
{
	m_nSwKeyDataIndex = 0;

	return TRUE;
}

BOOL CNSWKeyBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes |= CNShape::DrawControl(hDC, rcUpdate);

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
		
		rectKey.top = m_rcDisplay.top;
		rectKey.left = m_rcDisplay.left;

		// Select to Transparent Mode in Background Mode
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		
		// Select Font
		if (m_pFont)
			OldFont = (HFONT)::SelectObject(hDC, (HFONT)m_pFont->GetHandle());
		
		// Set Text Color
		/*
		if (m_crTextColor >= 0)
			OldTextColor = ::SetTextColor(hDC, m_crTextColor);
		*/
		if (m_pColor != NULL)
			OldTextColor = ::SetTextColor(hDC, m_pColor->m_crColor);
		
		for (line = 0; line < 4; line++)
		{
			// adjust top
			rectKey.top = m_rcDisplay.top + ((m_sizeNumber.cy + m_sizeSpace.cy) * line);
			rectKey.bottom = rectKey.top + m_sizeNumber.cy;
			
			for (col = 0; col < 3; col++)
			{
				// Draw Key Data.
				rectKey.left = m_rcDisplay.left + ((m_sizeNumber.cx + m_sizeSpace.cx) * col);
				rectKey.right = rectKey.left + m_sizeNumber.cx;
				
				// Draw Rectangle.
				//::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, 
				//				m_rcDisplay.right, m_rcDisplay.bottom);			
				
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
		/*
		if (m_crTextColor >= 0)
			::SetTextColor(hDC, OldTextColor);
		*/
		
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

BOOL CNSWKeyBox::KeyEvent(CString &strKey)
{
	//if ((strKey == _T(".")) || (strKey == _T("00")))				// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
	if (strKey == DES_STAR || strKey == DES_SHARP)				// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
	{
		//if (strKey == _T("."))									// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
		if (strKey == DES_STAR)									// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
			m_nSwKeyDataIndex--;
		else 
			m_nSwKeyDataIndex++;
		
		if (m_nSwKeyDataIndex < 0)
			m_nSwKeyDataIndex = MAX_KEY_SCR - 1;
		
		if (m_nSwKeyDataIndex > MAX_KEY_SCR - 1)
			m_nSwKeyDataIndex = 0;
		
		AddInvalidRect(m_rcDisplay);

		return TRUE;
		
	}
	// [#110] NH AIREAT 2008.4.15
	else if ((strKey.GetLength() == 1) && (strKey >= _T("0")) && (strKey <= _T("9")))
	{

		int	index = strKey.GetAt(0) - '0';

		
		if (index >= 0 && index <= 9)
		{
			// [#181] NH KGS 2008.04.29 SWKey 스페이스 입력 오류 수정
			if (!_tcscmp(g_NSwKeyData[m_nSwKeyDataIndex][index] , _T("SPC")))
				strKey = _T(" ");
			// [#183] NH KGS 2008.05.07 SWKey '&' 입력 안되는 버그 수정
			else if (!_tcscmp(g_NSwKeyData[m_nSwKeyDataIndex][index] , _T("&&")))
				strKey = _T("&");
			else
				strKey = g_NSwKeyData[m_nSwKeyDataIndex][index];
		}
	}
	// [#110] NH AIREAT 2008.4.15
	//else if (strKey == _T("000"))							// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
	else if (strKey == DES_CAPS)							// [#470] NH AIREAT 2009.01.06 : '.' 입력 선택 적용.
	{
		if (m_nSwKeyDataIndex >= 1 && m_nSwKeyDataIndex <= 3)
			m_nSwKeyDataIndex += 3;
		else if (m_nSwKeyDataIndex >= 4 && m_nSwKeyDataIndex <= 6)
			m_nSwKeyDataIndex -= 3;

		AddInvalidRect(m_rcDisplay);

		return TRUE;
	}
	// end of [#110]
	
	return FALSE;
}


///////////////////////////////////////////////////////////////
//
//	CNVirtualKeyBox
//
CNVirtualKeyBox::CNVirtualKeyBox(CTL_TYPE type) : CNShape(type)
{
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

CNVirtualKeyBox::~CNVirtualKeyBox()
{
}

void CNVirtualKeyBox::SetGuideFont(CNFont *pNormal, CNFont *pSelect)
{
	m_pGuideNormalFont = pNormal;
	m_pGuideFocusFont = pSelect;

	if (m_pGuideFocusFont == NULL)
		m_pGuideFocusFont = m_pGuideNormalFont;
}

void CNVirtualKeyBox::SetGuideBrush(CNBrush *pNormal, CNBrush *pSelect)
{
	m_pGuideNormalBrush = pNormal;
	m_pGuideFocusBrush = pSelect;

	if (m_pGuideFocusBrush == NULL)
		m_pGuideFocusBrush = m_pGuideNormalBrush;
}

void CNVirtualKeyBox::SetGuidePen(CNPen *pNormal, CNPen *pSelect)
{
	m_pGuideNormalPen = pNormal;
	m_pGuideFocusPen = pSelect;

	if (m_pGuideFocusPen == NULL)
		m_pGuideFocusPen = m_pGuideNormalPen;
}

void CNVirtualKeyBox::SetGuideTextColor(CNColor *pNormal, CNColor *pSelect)
{
	m_pGuideNormalTextColor = pNormal;
	m_pGuideFocusTextColor = pSelect;

	if (m_pGuideFocusTextColor == NULL)
		m_pGuideFocusTextColor = m_pGuideNormalTextColor;
}

void CNVirtualKeyBox::SetVKeyFont(CNFont *pNormal, CNFont *pSelect)
{
	m_pVKeyNormalFont = pNormal;
	m_pVKeyFocusFont = pSelect;

	if (m_pVKeyFocusFont == NULL)
		m_pVKeyFocusFont = m_pVKeyNormalFont;
}

void CNVirtualKeyBox::SetVKeyBrush(CNBrush *pNormal, CNBrush *pSelect)
{
	m_pVKeyNormalBrush = pNormal;
	m_pVKeyFocusBrush = pSelect;

	if (m_pVKeyFocusBrush == NULL)
		m_pVKeyFocusBrush = m_pVKeyNormalBrush;
}

void CNVirtualKeyBox::SetVKeyPen(CNPen *pNormal, CNPen *pSelect)
{
	m_pVKeyNormalPen = pNormal;
	m_pVKeyFocusPen = pSelect;

	if (m_pVKeyFocusPen == NULL)
		m_pVKeyFocusPen = m_pVKeyNormalPen;
}

void CNVirtualKeyBox::SetVKeyTextColor(CNColor *pNormal, CNColor *pSelect)
{
	m_pVKeyNormalTextColor = pNormal;
	m_pVKeyFocusTextColor = pSelect;

	if (m_pVKeyFocusTextColor == NULL)
		m_pVKeyFocusTextColor = m_pVKeyNormalTextColor;
}

BOOL CNVirtualKeyBox::Initialize(void)
{
	CNShape::Initialize();

	int	row, col;

	// get element width, height
	int	nElementWidth = 0, nElementHeight = 0;

	HDC hDC = GetDC(NULL);
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

		ReleaseDC(NULL, hDC);
	}
	else
	{
		nElementWidth = 10;
		nElementHeight = 10;
	}

	// get start point
	int	nStartX = 0, nStartY = 0;

	nStartX = m_rcDisplay.left;
	nStartY = m_rcDisplay.top;

	if (((nElementWidth + 2) * MAX_KEY_COL) < (m_rcDisplay.right - m_rcDisplay.left))
		nStartX += (int)(((m_rcDisplay.right - m_rcDisplay.left) - ((nElementWidth + 2) * MAX_KEY_COL)) / 2);

	if (((nElementHeight+2) * (m_nAllowRow)) < (m_rcDisplay.bottom - m_rcDisplay.top))
		nStartY += (int)(((m_rcDisplay.bottom - m_rcDisplay.top) - ((nElementHeight + 2) * (m_nAllowRow)))/2);

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
				CSize sz((nElementWidth*MAX_KEY_COL + ((MAX_KEY_COL-1)*2)), nElementHeight);
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

BOOL CNVirtualKeyBox::SetCtlFocus(CString &strName, CString &strData)
{
	if (m_strName == strName)
		m_bShow = TRUE;
	else
		m_bShow = FALSE;

	AddInvalidRect(m_rcDisplay);

	return FALSE;
}

BOOL CNVirtualKeyBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;

	if (m_bShow == FALSE || IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes |= CNShape::DrawControl(hDC, rcUpdate);

	int		row, col;

	for (row = 0; row < MAX_KEY_ROW; row++)
	{
		for (col = 0; col < MAX_KEY_COL; col++)
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
			UpperLine.right = m_KeyTable[0][MAX_KEY_COL-1].rcRect.right+2;
			UpperLine.top = m_KeyTable[0][0].rcRect.bottom-1;
			UpperLine.bottom = m_KeyTable[0][0].rcRect.bottom-1;
		}
		else
		{
			UpperLine.left = m_KeyTable[0][0].rcRect.left-2;
			UpperLine.right = m_KeyTable[0][MAX_KEY_COL-1].rcRect.right+2;
			UpperLine.top = m_KeyTable[0][0].rcRect.top-1;
			UpperLine.bottom = m_KeyTable[0][0].rcRect.top-1;
		}

		// UNDER GUIDE LINE
		if (m_KeyTable[MAX_KEY_ROW-1][0].nUseType == USE_KEY_UNDER_GUIDE)
		{
			UnderLine.left = m_KeyTable[MAX_KEY_ROW-1][0].rcRect.left-2;
			UnderLine.right = m_KeyTable[MAX_KEY_ROW-1][MAX_KEY_COL-1].rcRect.right+2;
			UnderLine.top = m_KeyTable[MAX_KEY_ROW-1][0].rcRect.top-1;
			UnderLine.bottom = m_KeyTable[MAX_KEY_ROW-1][0].rcRect.top-1;
		}
		else
		{
			UnderLine.left = m_KeyTable[MAX_KEY_ROW-2][0].rcRect.left-2;
			UnderLine.right = m_KeyTable[MAX_KEY_ROW-2][MAX_KEY_COL-1].rcRect.right+2;
			UnderLine.top = m_KeyTable[MAX_KEY_ROW-2][0].rcRect.top-1;
			UnderLine.bottom = m_KeyTable[MAX_KEY_ROW-2][0].rcRect.top-1;
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
//	CNAlphaKeyBox
//

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
	{_T("CHARACTER SELECT : 0 ~ 9  SELECT : ENTER"),}
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
	{_T("CHARACTER SELECT : 0 ~ 9  SELECT : ENTER"),}
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
	{_T("CHARACTER SELECT : 0 ~ 9  SELECT : ENTER")}
};


CNAlphaKeyBox::CNAlphaKeyBox(CTL_TYPE type) : CNVirtualKeyBox(type)
{
	m_nAlphaKeyType = VKEY_ALPHA_UPPER;
}

CNAlphaKeyBox::~CNAlphaKeyBox()
{
}

void CNAlphaKeyBox::SetAlphaKeyType(VKEY_TYPE type)
{
	m_nAlphaKeyType = type;
}

BOOL CNAlphaKeyBox::Initialize(void)
{
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

	CNVirtualKeyBox::Initialize();

	m_pCurKey = &m_KeyTable[1][0];

	return TRUE;
}

BOOL CNAlphaKeyBox::KeyEvent(CString &strKey)
{
	if(strKey == DES_CAPS)
	{
		if(HEXADECIMAL != g_nCurrEditMode)
		{
			ChangeInputMode();
			g_bTabKey = TRUE;
		}
	}
	else
	{
		if(strKey == _T("LEFT"))
			strKey = DES_STAR;
		else if(strKey == _T("RIGHT"))
			strKey = DES_SHARP;
		else if(strKey == _T("BACKSPACE"))
			strKey = DES_CLEAR;
	}

	return FALSE;
} 

/** *************************************************************
*	@brief		입력모드 변환
*	@details	KeyPad의 'Blank'(Mode Change)를 누를 때 입력모드를 변경시킨다.
*	@retval		없음
*****************************************************************/
void CNAlphaKeyBox::ChangeInputMode()
{
	int nModeSize = 0;
	int nEditMode = NUMBER | UPPERCASE | LOWERCASE | SPECIALCHAR;
	BOOL bFindMode = FALSE;

	if(NUMBER == g_nCurrEditMode)
		g_bIsPrevNotNumberMode = FALSE;

	int EditMode[] = 
	{
		PASSWORD,
		NUMBER,
		UPPERCASE,
		LOWERCASE,
		DATEMODE,
		TIMEMODE,
		SPECIALCHAR,
	};

	nModeSize = sizeof(EditMode)/sizeof(int);

	for (int nCnt = 0; nCnt < nModeSize; nCnt++)
	{
		if (EditMode[nCnt] == g_nCurrEditMode)
		{
			for (int nNCnt = 1; nNCnt < nModeSize; nNCnt++)
			{
				int nNextEditMode = EditMode[(nCnt + nNCnt) % nModeSize];

				if (nNextEditMode & nEditMode)
				{
					g_nCurrEditMode = nNextEditMode;
					bFindMode = TRUE;
					break;
				}
			}

			if(TRUE == bFindMode)
				break;
		}
	}
}

///////////////////////////////////////////////////////////////
//
//	CNTableKeyBox
//

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

CNTableKeyBox::CNTableKeyBox(CTL_TYPE type) : CNVirtualKeyBox(type)
{
}

CNTableKeyBox::~CNTableKeyBox()
{
}

BOOL CNTableKeyBox::Initialize(void)
{
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

	CNVirtualKeyBox::Initialize();

	m_pCurKey = &m_KeyTable[0][0];

	return TRUE;
}

BOOL CNTableKeyBox::KeyEvent(CString &strKey)
{
	if (m_bShow == TRUE)
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

			AddInvalidRect(m_rcDisplay);
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

	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CNToggleBox
//

CNToggleBox::CNToggleBox(CTL_TYPE type) : CNTextBox(type)
{
	m_strEnableText = _T("");
	m_strDisableText = _T("");
}

CNToggleBox::~CNToggleBox()
{
}

void CNToggleBox::SetEnableText(LPCTSTR lpText)
{
	m_strEnableText = lpText;
}

void CNToggleBox::SetDisableText(LPCTSTR lpText)
{
	m_strDisableText = lpText;
}

BOOL CNToggleBox::SetCtlFocus(CString &strName, CString &strData)
{
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
			// [#75] NH AIREAT 2008.04.07 Full FS Command 추가
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
			// end of [#75]
			else if (m_Act.GetKind() == REQ_KEYEVENT)
			{
				//	[#76] NH KGS 2008.04.02 ToolBox 컨트롤: Focus받을 때 반응 수정
//				AddRequest(m_Act.GetKind(), NULL, m_strText);
				AddRequest(m_Act.GetKind(), NULL, strData);
				// end of [#76]
			}
			else
				AddRequest(m_Act);
		}

		AddInvalidRect(m_rcDisplay);
		
	}

	return FALSE;
}

BOOL CNToggleBox::SetBlinking(void)
{
	if (m_bBlinkMode && m_strText == m_strEnableText)
	{
		m_bBlinking = !m_bBlinking;
		AddInvalidRect(m_rcDisplay);

		return TRUE;
	}

	return FALSE;
}

BOOL CNToggleBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;
	if (m_strText == m_strEnableText)
	{
		m_bFocus = TRUE;
		bRes |= CNTextBox::DrawControl(hDC, rcUpdate);
		m_bFocus = FALSE;
	}
	else
	{
		bRes |= CNTextBox::DrawControl(hDC, rcUpdate);
	}

	return bRes;
}

BOOL CNToggleBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		m_strText = strData;
		
		AddInvalidRect(m_rcDisplay);

		return TRUE;
	}

	return FALSE;
}

BOOL CNToggleBox::MakeAllFSCMD(CString &Data)
{
	Data += SCR_RES_DELIMITER + m_strText;

	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CNThreeStateBox
//

CNThreeStateBox::CNThreeStateBox(CTL_TYPE type)
{
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

CNThreeStateBox::~CNThreeStateBox()
{
}

void CNThreeStateBox::SetStateImage(CNPicture *pHide, CNPicture *pOn, CNPicture *pOff)
{
	m_pStateImage[STATE_HIDE] = pHide;
	m_pStateImage[STATE_ON] = pOn;
	m_pStateImage[STATE_OFF] = pOff;
}

void CNThreeStateBox::SetStatePen(CNPen *pHide, CNPen *pOn, CNPen *pOff)
{
	m_pStatePen[STATE_HIDE] = pHide;
	m_pStatePen[STATE_ON] = pOn;
	m_pStatePen[STATE_OFF] = pOff;
}

void CNThreeStateBox::SetStateBrush(CNBrush *pHide, CNBrush *pOn, CNBrush *pOff)
{
	m_pStateBrush[STATE_HIDE] = pHide;
	m_pStateBrush[STATE_ON] = pOn;
	m_pStateBrush[STATE_OFF] = pOff;
}

void CNThreeStateBox::SetStateFont(CNFont *pHide, CNFont *pOn, CNFont *pOff)
{
	m_pStateFont[STATE_HIDE] = pHide;
	m_pStateFont[STATE_ON] = pOn;
	m_pStateFont[STATE_OFF] = pOff;
}

void CNThreeStateBox::SetStateTextColor(CNColor *pHide, CNColor *pOn, CNColor *pOff)
{
	m_pStateTextColor[STATE_HIDE] = pHide;
	m_pStateTextColor[STATE_ON] = pOn;
	m_pStateTextColor[STATE_OFF] = pOff;
}

void CNThreeStateBox::SetStateBlink(BOOL bHide, BOOL bOn, BOOL bOff)
{
	m_bStateBlink[STATE_HIDE] = bHide;
	m_bStateBlink[STATE_ON] = bOn;
	m_bStateBlink[STATE_OFF] = bOff;
}

BOOL CNThreeStateBox::Initialize(void)
{
	m_nCurState = STATE_HIDE;
	m_bBlinking = FALSE;
	m_nBlinkCount = 0;

	return TRUE;
}

BOOL CNThreeStateBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		// APStatusXX
		if (_tcsncmp(strName, DES_AP_STATE, 7) == 0)
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

		AddInvalidRect(m_rcDisplay);
		return TRUE;
	}

	return FALSE;
}

BOOL CNThreeStateBox::SetBlinking(void)
{
	if (m_bStateBlink[m_nCurState] == TRUE)
	{
		m_nBlinkCount++;

		if ((m_nBlinkCount * BLINKING_TIME) >= 500)
		{
			m_nBlinkCount = 0;

			m_bBlinking = !m_bBlinking;
			AddInvalidRect(m_rcDisplay);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNThreeStateBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	/////////////////////////////////////
	//	DRAW BODY
	if (!m_bBlinking)
	{
		HPEN	OldPen;
		HBRUSH	OldBrush;
		
		// Draw Picture.
		if (m_pStateImage[m_nCurState] != NULL)
		{
			m_pStateImage[m_nCurState]->DrawImage(hDC, m_rcDisplay.left, m_rcDisplay.top, m_rcDisplay.right, m_rcDisplay.bottom);
			
#ifdef AE_SCREEN_DEGISN
			// Select Pen
			OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
			OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			
			// Draw Rectangle.
			::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, m_rcDisplay.right, m_rcDisplay.bottom);
			
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
				::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, 
					m_rcDisplay.right, m_rcDisplay.bottom);
			}
			
			// Restore Brush
			::SelectObject(hDC, OldBrush);
			
			// Restore Pen.
			::SelectObject(hDC, OldPen);
		}
#ifdef AE_SCREEN_DEGISN
		else
		{
			// Select Pen
			OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
			OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			
			// Draw Rectangle.
			::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, m_rcDisplay.right, m_rcDisplay.bottom);
			
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
				rcClient = m_rcDisplay;
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

///////////////////////////////////////////////////////////////
//
//	CNEJNLBox
//

CNEJNLBox::CNEJNLBox(CTL_TYPE type) : CNTextBox(type)
{
	m_bMultiline = TRUE;
}

CNEJNLBox::~CNEJNLBox()
{
}

CString CNEJNLBox::GetDrawText()
{
	CString strDrawText;

	strDrawText = CNTextBox::GetDrawText();

	strDrawText.Replace(FIELD_DELIMITER, '\n');
	strDrawText.Replace(_T("&"), _T("&&"));

	return strDrawText;
}

BOOL CNEJNLBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL		bRes = FALSE;
	CString		strDrawText;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	// PARENT CLASS DRAW
	bRes |= CNShape::DrawControl(hDC, rcUpdate);
	
	// Get to Draw Text string
	strDrawText = GetDrawText();

	////////////////////////////////////
	if (strDrawText.GetLength())
	{
		int			OldBkMode;
		HFONT		OldFont = NULL;
//		COLORREF	OldTextColor;
		COLORREF	OldTextColor = 0xffffffff;
		
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
			rcClient = m_rcDisplay;

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

///////////////////////////////////////////////////////////////
//
//	CNDateBox
//

CNDateBox::CNDateBox(CTL_TYPE type) : CNTextBox(type)
{
}

CNDateBox::~CNDateBox()
{
}

BOOL CNDateBox::SetCtlFocus(CString &strName, CString &strData)
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

	AddInvalidRect(m_rcDisplay);
		
	return FALSE;
}

BOOL CNDateBox::KeyEvent(CString &strKey)
{
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
			
			AddInvalidRect(m_rcDisplay);
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

			AddInvalidRect(m_rcDisplay);
			
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

				AddInvalidRect(m_rcDisplay);
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
				
				AddInvalidRect(m_rcDisplay);
			}

			return TRUE;
		}
	}
	
	return FALSE;
}


BOOL CNDateBox::MakeAllFSCMD(CString &Data)
{
	Data += SCR_RES_DELIMITER + m_strText;
	
	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CTL_IPBOX
//

CNIPBox::CNIPBox(CTL_TYPE type) : CNEditTextBox(type)
{
}

CNIPBox::~CNIPBox()
{
}

BOOL CNIPBox::SetCtlFocus(CString &strName, CString &strData)
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

	AddInvalidRect(m_rcDisplay);
		
	return FALSE;
}

BOOL CNIPBox::KeyEvent(CString &strKey)
{
	if (m_bFocus)
	{
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

			AddInvalidRect(m_rcDisplay);
			
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

				AddInvalidRect(m_rcDisplay);
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
				
				AddInvalidRect(m_rcDisplay);
			}

			return TRUE;
		}
	}
	
	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CTL_CENTBOX
//

CNCentBox::CNCentBox(CTL_TYPE type) : CNEditTextBox(type)
{
}

CNCentBox::~CNCentBox()
{
}

CString CNCentBox::GetDrawText()
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
		// [#726] MX KSK 2012.07.17
#if (MX_VERSION)
		strDrawText.Format(_T("%s 0.%02d"), SCREEN_CURRENCY, RightValue);
#else
		strDrawText.Format(_T("%s 0.%02d"), GetCurrencySymbol(), RightValue);
#endif
		// end of [#726]
	}
	else
	{
		int Len = 0;
		CString		Temp = _T("");
		// 12345 -> 123.45
		// 123456 -> 1,234.56
		RightValue = Asc2Int(m_strText.Right(2));
		
		// add ','
#if (MX_VERSION)	// [#726] MX KSK 2012.07.17
		strDrawText.Format(_T("%s "), SCREEN_CURRENCY);
#else
		strDrawText.Format(_T("%s "), GetCurrencySymbol());
#endif				// end of [#726]
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

///////////////////////////////////////////////////////////////
//
//	CTL_DOLLARBOX
//

CNDollarBox::CNDollarBox(CTL_TYPE type) : CNEditTextBox(type)
{
}

CNDollarBox::~CNDollarBox()
{
}

CString CNDollarBox::GetDrawText()
{
	CString strDrawText;
	
	// $000
	if (Asc2Int(m_strText) == 0)
	{
		m_strText = _T("");
	}

#if (MX_VERSION)	// [#726] MX KSK 2012.07.17
	strDrawText.Format(_T("%s %d"), SCREEN_CURRENCY, Asc2Int(m_strText));
#else
	strDrawText.Format(_T("%s %d"), GetCurrencySymbol(), Asc2Int(m_strText));
#endif				// end of [#726]
	
	return strDrawText;
}

// [#810] US Justin "CLEAR" => Remove all entered amount
BOOL CNDollarBox::KeyEvent(CString &strKey)
{
	if (m_bFocus)
	{
		// ENTER KEY	[#18]
		if (strKey == DES_ENTER)
		{
			// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
			if( m_nMinInputChar > 0 && m_strText.GetLength() < m_nMinInputChar)
			{
				return TRUE;
			}
			// end of [#182]		
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
					if (m_strText.GetLength() == 1)
						m_Act.m_Data = _T("0") + m_strText;
					else
						m_Act.m_Data = m_strText;

					m_strText = _T("");

					AddRequest(m_Act.GetKind(), NULL, m_Act.m_Data);
				}
				else
					AddRequest(m_Act);
			}

			// kill focus.
			if (!m_bAutoFocus)
				m_bFocus = FALSE;
			
			AddInvalidRect(m_rcDisplay);
			return TRUE;
		}
		// CLEAR KEY
		else if (strKey == DES_CLEAR)
		{
			m_strText = _T("");

			/*
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
			*/

			AddInvalidRect(m_rcDisplay);
			return TRUE;
		}
		// numeric key.
		//else if ((strKey.GetLength() == 1) && (strKey != DES_CANCEL))							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		else if ((strKey.GetLength() == 1) ||													// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
				 (m_TextType == TTYPE_IPADDRESS && strKey == DES_STAR))							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		{
			if (m_TextType == TTYPE_IPADDRESS && strKey == DES_STAR)							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
				strKey = _T(".");																// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.

			if (m_strText.GetLength() < m_nMaxInputChar)
			{
				m_strText += strKey;
				AddInvalidRect(m_rcDisplay);		
			}
				// Shift effect. - only support #901
			else if ((m_strText.GetLength()+1) > m_nMaxInputChar && m_Act.GetKind() == REQ_KEYEVENT)
			{
				m_strText += strKey;
				m_strText = m_strText.Right(m_nMaxInputChar);
				AddInvalidRect(m_rcDisplay);
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
				
				AddInvalidRect(m_rcDisplay);
			}

			return TRUE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////
//
//	CNBinBox
//

CNBinBox::CNBinBox(CTL_TYPE type) : CNTextBox(type)
{
	m_Act.Set(REQ_FSCMD, _T("BINLIST"), _T("BINLIST"));

	m_BinItem[BNTYPE_INDEX].SetMaxInputChar(2);
	m_BinItem[BNTYPE_BIN].SetMaxInputChar(10);
}

CNBinBox::~CNBinBox()
{
}


BOOL CNBinBox::Initialize(void)
{
	CNTextBox::Initialize();

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

	//*LEH
	// Round
	m_BinItem[BNTYPE_INDEX].SetRoundSize(this->m_nRoundWidth, this->m_nRoundHeight);
	m_BinItem[BNTYPE_BIN].SetRoundSize(this->m_nRoundWidth, this->m_nRoundHeight);
	/////

	// Text Color
	/*
	m_BinItem[BNTYPE_INDEX].SetTextColor(this->m_crTextColor, this->m_crFocusTextColor);
	m_BinItem[BNTYPE_BIN].SetTextColor(this->m_crTextColor, this->m_crFocusTextColor);
	*/
	m_BinItem[BNTYPE_INDEX].SetTextColor(this->m_pColor, this->m_pFocusColor);
	m_BinItem[BNTYPE_BIN].SetTextColor(this->m_pColor, this->m_pFocusColor);

	CRequest	req(REQ_INITIAL, _T(""), _T(""));
	m_BinItem[BNTYPE_INDEX].Handler(req);
	m_BinItem[BNTYPE_BIN].Handler(req);

	m_curInput = BNTYPE_MAX;
	m_BinAct = BNTYPE_MAX;

	return TRUE;
}

BOOL CNBinBox::BinAct(CString &strAct)
{
	CRequest	req(REQ_INITIAL, _T(""), _T(""));
	m_BinItem[BNTYPE_INDEX].Handler(req);
	m_BinItem[BNTYPE_BIN].Handler(req);
	// [#23] NH KGS 2008.03.17 BinList
	m_curInput = BNTYPE_MAX;
	m_BinAct = BNTYPE_MAX;
	// end of [#23]	
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

	// [#23] NH KGS 2008.03.17 BinList
	AddInvalidRect(m_BinItem[BNTYPE_INDEX].GetDisplayRect());
	AddInvalidRect(m_BinItem[BNTYPE_BIN].GetDisplayRect());
	// end of [#23]
	
	return TRUE;
}

BOOL CNBinBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL		bRes = FALSE;
	CRequest	req;
	
	req.SetKinD(REQ_DRAW);
	req.m_rcUpdate = rcUpdate;		// 	[#23] NH KGS 2008.03.17 BinList
	req.m_hDC = hDC;
	
	bRes |= m_BinItem[BNTYPE_INDEX].Handler(req);
	bRes |= m_BinItem[BNTYPE_BIN].Handler(req);

	return bRes;
}

BOOL CNBinBox::KeyEvent(CString &strKey)
{
	if (m_bFocus && m_curInput != BNTYPE_MAX)
	{
		CRequest	req;

		// [#23] NH KGS 2008.03.17 BinList
		AddInvalidRect(m_BinItem[BNTYPE_INDEX].GetDisplayRect());
		AddInvalidRect(m_BinItem[BNTYPE_BIN].GetDisplayRect());
		// end of [#23]
		
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
				//strData += _T(":ADD:");
				strData += SCR_RES_DELIMITER;
				strData += _T("ADD");
				strData += SCR_RES_DELIMITER;
				strData += m_BinItem[BNTYPE_BIN].GetInputText();
			}
			else if (m_BinAct == BNTYPE_DEL)
			{
				//strData += _T(":DELETE:");
				strData += SCR_RES_DELIMITER;
				strData += _T("DELETE");
				strData += SCR_RES_DELIMITER;
				strData += m_BinItem[BNTYPE_INDEX].GetInputText();
			}
			else if (m_BinAct == BNTYPE_EDT)
			{
				//strData += _T(":EDIT:");
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
			// [#23] NH KGS 2008.03.17 BinList
			// Clear Input
			req.Set(REQ_INITIAL, _T(""), _T(""));
			
			m_BinItem[BNTYPE_INDEX].Handler(req);
			m_BinItem[BNTYPE_BIN].Handler(req);

			m_BinItem[BNTYPE_INDEX].SetFocus(FALSE);
			m_BinItem[BNTYPE_BIN].SetFocus(FALSE);
			
			m_curInput = BNTYPE_MAX;
			m_BinAct = BNTYPE_MAX;
			// end of [#23] 
			return TRUE;
		}
		// CANCEL
		else if (strKey == DES_CANCEL)
		{
			// [#23] NH KGS 2008.03.17 BinList
			// Clear Input
			req.Set(REQ_INITIAL, _T(""), _T(""));
			
			m_BinItem[BNTYPE_INDEX].Handler(req);
			m_BinItem[BNTYPE_BIN].Handler(req);

			m_BinItem[BNTYPE_INDEX].SetFocus(FALSE);
			m_BinItem[BNTYPE_BIN].SetFocus(FALSE);

			m_curInput = BNTYPE_MAX;
			m_BinAct = BNTYPE_MAX;
			return TRUE;
			// end of [#23]
		}
		// [#236] NZ AIREAT 2008.06.04 - TIMEOUT
		else if (strKey == DES_TIMEOVER)
		{
			return FALSE;
		}
		// end of [#236]
		// ELSE
		else
		{
			req.Set(REQ_KEYEVENT, _T(""), strKey);
			m_BinItem[m_curInput].Handler(req);
			return TRUE;			// [#23] NH KGS 2008.03.17 BinList
		}

	
	}

	return FALSE;
}

void CNBinBox::SetBinBoxRect(BINTYPE type, int left, int top, int width, int height)
{
	//if (type > BNTYPE_MAX && type < 0)
	if (type > BNTYPE_MAX)					// [CODESONAR] /* Redundant Condition (ID: 136) */
		return;

	m_BinItem[type].SetDisplayRect(left, top, width, height);
}

///////////////////////////////////////////////////////////////
//
//	CNAnimation
//

CNAnimation::CNAnimation(CTL_TYPE type/*=CTL_ANI*/) : CNShape(type)
{
	m_nImageIndex = 0;
	m_nLoadImage = 0;

	m_bShow = TRUE;
}

CNAnimation::~CNAnimation()
{
}

void CNAnimation::ImageRelease()
{
}

BOOL CNAnimation::Initialize(void)
{
	m_bShow = TRUE;
	m_nImageIndex = 0;
	m_nBlinkCount = 0;

	SetBlinking();	// animation이 처음부터 화면에 표시 되도록.

	return TRUE;
}

BOOL CNAnimation::SetAniImage(int Count, CString strPrefix)
{
	LPCTSTR pAniFile[10]= { NULL, };
	CString	strFileName[10];

	m_nLoadImage = 0;

	for (int i=0; i < Count; i++)
	{
		if (m_nLoadImage > MAX_ANI)
			break;

		strFileName[i].Format(_T("%s%d.bmp"),strPrefix, i+1);
		//m_strImageName[i] = strFileName;
		pAniFile[i] = (LPCTSTR)strFileName[i];

		m_nLoadImage = i+1;
	}

	m_Image.SetDecodeInfo(pAniFile, m_nLoadImage, FALSE);
	
	return TRUE;
}

BOOL CNAnimation::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;

	if (m_bShow == FALSE)
		return FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes = m_Image.Draw(hDC, m_nImageIndex, &m_rcDisplay);

//	AddInvalidRect(rcUpdate);

	return bRes;
}

BOOL CNAnimation::SetBlinking(void)
{
	if (!m_bShow)
		return TRUE;

//	if (m_nBlinkCount >= 2)
	{
		m_nImageIndex++;
		if (m_nImageIndex >= m_nLoadImage)
			m_nImageIndex = 0;

		//	m_Image.DecodeImage(m_nImageIndex);

		AddInvalidRect(m_rcDisplay);
//		m_nBlinkCount = 0;
	}
//	else
//		m_nBlinkCount++;

	return TRUE;
}

BOOL CNAnimation::SetVariable(CString &strName, CString &strData)
{
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

///////////////////////////////////////////////////////////////
//
//	CNAdvBox
//

CNAdvBox::CNAdvBox(CTL_TYPE type/* =CTL_ADVBOX */) : CNTextBox(type)
{
	m_strFileName = _T("");
//	m_pImage = NULL;
//	m_bDecode = FALSE;
}

CNAdvBox::~CNAdvBox()
{
	ImageRelease();
}

void CNAdvBox::SetFileName(CString strFileName)
{
	LPCTSTR pFileName[1];

	pFileName[0] = (LPCTSTR)strFileName;
	
	m_Image.SetDecodeInfo(pFileName, 1, FALSE);
}

BOOL CNAdvBox::DecodeImage()
{
//	m_bDecode = FALSE;

	return TRUE;
}

BOOL CNAdvBox::IsDecode()
{
	return TRUE;
}

BOOL CNAdvBox::Initialize()
{
	ImageRelease();

	m_Image.DecodeImage(0);

	return TRUE;
}

BOOL CNAdvBox::Deinitialize()
{
	m_Image.ReleaseImage();

	return TRUE;
}

BOOL CNAdvBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;

	bRes = m_Image.Draw(hDC, 0, &rcUpdate);

	if (bRes == TRUE)
		AddInvalidRect(rcUpdate);

	return bRes;
}

void CNAdvBox::ImageRelease()
{
}

///////////////////////////////////////////////////////////////
//
//	CNBinIndexBox
//
//[#543] SOOK 2009.07.19 BIN EDIT DELETE를 공통으로 쓰기 위해 BINBINDEXBOX 컨트롤 제작함 
CNBinIndexBox::CNBinIndexBox(CTL_TYPE type) : CNEditTextBox(type)
{
	m_Act.Set(REQ_FSCMD, _T("BINLIST"), _T("BINLIST"));
	m_nMinInputChar = 1;
	m_nMaxInputCharOrigin=m_nMaxInputChar = 4;
}

CNBinIndexBox::~CNBinIndexBox()
{
}


BOOL CNBinIndexBox::BinAct(CString &strAct)
{
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


BOOL CNBinIndexBox::KeyEvent(CString &strKey)
{
	if (m_bFocus)
	{
		CRequest	req;
		// ENTER KEY	[#18]
		if (strKey == DES_ENTER)
		{

			// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
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
			
			AddInvalidRect(m_rcDisplay);
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

			AddInvalidRect(m_rcDisplay);
			return TRUE;
		}
		// numeric key.
		//else if ((strKey.GetLength() == 1) && (strKey != DES_CANCEL))							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		else if (strKey.GetLength() == 1) 															// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		{
			if (m_strText.GetLength() < m_nMaxInputChar)
			{
				m_strText += strKey;
				AddInvalidRect(m_rcDisplay);		
			}
				// Shift effect. - only support #901
			else if ((m_strText.GetLength()+1) > m_nMaxInputChar && m_Act.GetKind() == REQ_KEYEVENT)
			{
				m_strText += strKey;
				m_strText = m_strText.Right(m_nMaxInputChar);
				AddInvalidRect(m_rcDisplay);
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
				
				AddInvalidRect(m_rcDisplay);
			}

			return TRUE;
		}
	}
	return FALSE;
}
//end of [#543] SOOK 2009.07.19

// [#774] US Justin 2014.12.16 Add Decimal Box
///////////////////////////////////////////////////////////////
//
//	CTL_DECIMALBOX
//

CNDecimalBox::CNDecimalBox(CTL_TYPE type) : CNEditTextBox(type)
{
}

CNDecimalBox::~CNDecimalBox()
{
}

CString CNDecimalBox::GetDrawText()
{
	CString strDrawText = _T("");

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
		strDrawText.Format(_T("0.%02d"), RightValue);
	}
	else
	{
		// 12345 -> 123.45
		// 123456 -> 1,234.56
		RightValue = Asc2Int(m_strText.Right(2));

		CString Temp = m_strText.Left(m_strText.GetLength()-2);
		int Len = Temp.GetLength();
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
// End of [#774]


// [#810] US Justin 2015.10.09 Cent Clear Box
///////////////////////////////////////////////////////////////
//
//	CTL_CENTCLEARBOX
//
CNCentClearBox::CNCentClearBox(CTL_TYPE type) : CNEditTextBox(type)
{
}

CNCentClearBox::~CNCentClearBox()
{
}

CString CNCentClearBox::GetDrawText()
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
		// [#726] MX KSK 2012.07.17
#if (MX_VERSION)
		strDrawText.Format(_T("%s 0.%02d"), SCREEN_CURRENCY, RightValue);
#else
		strDrawText.Format(_T("%s 0.%02d"), GetCurrencySymbol(), RightValue);
#endif
		// end of [#726]
	}
	else
	{
		int Len = 0;
		CString		Temp = _T("");
		// 12345 -> 123.45
		// 123456 -> 1,234.56
		RightValue = Asc2Int(m_strText.Right(2));
		
		// add ','
#if (MX_VERSION)	// [#726] MX KSK 2012.07.17
		strDrawText.Format(_T("%s "), SCREEN_CURRENCY);
#else
		strDrawText.Format(_T("%s "), GetCurrencySymbol());
#endif				// end of [#726]
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

BOOL CNCentClearBox::KeyEvent(CString &strKey)
{
	if (m_bFocus)
	{
		// ENTER KEY	[#18]
		if (strKey == DES_ENTER)
		{
			// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
			if( m_nMinInputChar > 0 && m_strText.GetLength() < m_nMinInputChar)
			{
				return TRUE;
			}
			// end of [#182]		
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
					if (m_strText.GetLength() == 1)
						m_Act.m_Data = _T("0") + m_strText;
					else
						m_Act.m_Data = m_strText;

					m_strText = _T("");

					AddRequest(m_Act.GetKind(), NULL, m_Act.m_Data);
				}
				else
					AddRequest(m_Act);
			}

			// kill focus.
			if (!m_bAutoFocus)
				m_bFocus = FALSE;
			
			AddInvalidRect(m_rcDisplay);
			return TRUE;
		}
		// CLEAR KEY
		else if (strKey == DES_CLEAR)
		{
			m_strText = _T("");

			/*
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
			*/

			AddInvalidRect(m_rcDisplay);
			return TRUE;
		}
		// numeric key.
		//else if ((strKey.GetLength() == 1) && (strKey != DES_CANCEL))							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		else if ((strKey.GetLength() == 1) ||													// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
				 (m_TextType == TTYPE_IPADDRESS && strKey == DES_STAR))							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
		{
			if (m_TextType == TTYPE_IPADDRESS && strKey == DES_STAR)							// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.
				strKey = _T(".");																// [#470] NH AIREAT 2008.12.22 : '.' 입력 선택 적용.

			if (m_strText.GetLength() < m_nMaxInputChar)
			{
				m_strText += strKey;
				AddInvalidRect(m_rcDisplay);		
			}
				// Shift effect. - only support #901
			else if ((m_strText.GetLength()+1) > m_nMaxInputChar && m_Act.GetKind() == REQ_KEYEVENT)
			{
				m_strText += strKey;
				m_strText = m_strText.Right(m_nMaxInputChar);
				AddInvalidRect(m_rcDisplay);
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
				
				AddInvalidRect(m_rcDisplay);
			}

			return TRUE;
		}
	}

	return FALSE;
}
// End of [#810]

/** *************************************************************
*	@brief	KeyTestBox 생성자
*	@param	CTL_TYPE type control type
*	@retval	없음
*****************************************************************/
CNKeyTestBox::CNKeyTestBox(CTL_TYPE type) : CNEditTextBox(type)
{
}

/** *************************************************************
*	@brief	KeyTestBox 소멸자
*	@retval	없음
*****************************************************************/
CNKeyTestBox::~CNKeyTestBox()
{
}

/** *************************************************************
*	@brief	PINPAD의 특수키(-, +, BLANK)를 화면에 보여주기 위함.
*	@retval	화면에 보여줄 문자
*****************************************************************/
CString CNKeyTestBox::GetDrawText()
{
	CString strDrawText = _T("");

	if(DES_STAR == m_strText)		// "-"
		strDrawText = _T("-");
	else if(DES_SHARP == m_strText)	// "+"
		strDrawText = _T("+");
	else if(DES_CAPS == m_strText)	// "BLANK"
		strDrawText = _T("BLANK");
	else if( (EMUL_FUNCKEY_L1 == m_strText) ||
			 (EMUL_FUNCKEY_L2 == m_strText) ||
			 (EMUL_FUNCKEY_L3 == m_strText) ||
			 (EMUL_FUNCKEY_L4 == m_strText) ||
			 (EMUL_FUNCKEY_R1 == m_strText) ||
			 (EMUL_FUNCKEY_R2 == m_strText) ||
			 (EMUL_FUNCKEY_R3 == m_strText) ||
			 (EMUL_FUNCKEY_R4 == m_strText) )
		strDrawText = EMUL_FUNCKEY_PREFIX + m_strText;
	else
		strDrawText = m_strText;

	return strDrawText;
}

/** *************************************************************
*	@brief	KeyEvent
*	@param	CString &strKey	입력 받은 Key
*	@retval	없음
*****************************************************************/
BOOL CNKeyTestBox::KeyEvent(CString &strKey)
{
	static int nBlankCount = 0;
	if (m_bFocus)
	{
		m_strText = strKey;
		AddInvalidRect(m_rcDisplay);

		if(DES_CAPS == m_strText)
		{
			nBlankCount++;
			if(4 == nBlankCount)
			{
				g_FSCmdQueue.Enqueue(CScrMsg(_T("KEYTEST"), _T("EXIT")));
				nBlankCount = 0;
			}
		}
		else if (strKey == DES_TIMEOVER)
		{
			nBlankCount = 0;
			return FALSE;
		}
		else
		{
			nBlankCount = 0;
		}

		return TRUE;
	}

	return FALSE;
}

/** *************************************************************
*	@brief	InputEditBox 생성자
*	@param	CTL_TYPE type control type
*	@retval	없음
*****************************************************************/
CNInputEditBox::CNInputEditBox(CTL_TYPE type) : CNEditTextBox(type)
{
	m_nInputType = 0;
}

/** *************************************************************
*	@brief	InputEditBox 소멸자
*	@retval	없음
*****************************************************************/
CNInputEditBox::~CNInputEditBox()
{
}

/** *************************************************************
*	@brief	AP에서 선택한 옵션에 따라 EditBox에 보여지는 문자열이 달라짐.
*	@retval	화면에 보여줄 문자
*****************************************************************/
CString CNInputEditBox::GetDrawText()
{
	CString strDrawText = _T("");
	int	RightValue = 0;

	if(m_nInputType != 0)
	{
		if(0 == Asc2Int(m_strText))
			m_strText.Empty();
	}

	if(1 == m_nInputType)	//	dollar
	{
#if (MX_VERSION)	// [#726] MX KSK 2012.07.17
		strDrawText.Format(_T("%s %d"), SCREEN_CURRENCY, Asc2Int(m_strText));
#else
		strDrawText.Format(_T("%s %d"), GetCurrencySymbol(), Asc2Int(m_strText));
#endif				// end of [#726]
	}
	else if(2 == m_nInputType)	// cent
	{
		if (m_strText.GetLength() <= 2)
		{
			// 0 -> 0.00
			// 10 -> 0.10
			RightValue = Asc2Int(m_strText.Right(2));
			// [#726] MX KSK 2012.07.17
	#if (MX_VERSION)
			strDrawText.Format(_T("%s 0.%02d"), SCREEN_CURRENCY, RightValue);
	#else
			strDrawText.Format(_T("%s 0.%02d"), GetCurrencySymbol(), RightValue);
	#endif
			// end of [#726]
		}
		else
		{
			int Len = 0;
			CString		Temp = _T("");
			// 12345 -> 123.45
			// 123456 -> 1,234.56
			RightValue = Asc2Int(m_strText.Right(2));
			
			// add ','
	#if (MX_VERSION)	// [#726] MX KSK 2012.07.17
			strDrawText.Format(_T("%s "), SCREEN_CURRENCY);
	#else
			strDrawText.Format(_T("%s "), GetCurrencySymbol());
	#endif				// end of [#726]
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
	}
	else if(3 == m_nInputType)
	{
		if (m_strText.GetLength() <= 2)
		{
			// 0 -> 0.00
			// 10 -> 0.10
			RightValue = Asc2Int(m_strText.Right(2));
			strDrawText.Format(_T("0.%02d"), RightValue);
		}
		else
		{
			// 12345 -> 123.45
			// 123456 -> 1,234.56
			RightValue = Asc2Int(m_strText.Right(2));

			CString Temp = m_strText.Left(m_strText.GetLength()-2);
			int Len = Temp.GetLength();
			for (int i=Len; i>0; i--)
			{
				if (((i % 3) == 0) && (i != Len))		strDrawText += ",";
				strDrawText += Temp.GetAt(Len-i);
			}

			// add cent.
			Temp.Format(_T(".%02d"), RightValue);

			strDrawText += Temp;
		}

		strDrawText += _T(" %");
	}
	else	// normal
	{
		strDrawText += m_strText;
	}

	return strDrawText;
}

/** *************************************************************
*	@brief		숫자 입력화면에서 dollar, cent 등을 표시할 수 있는 type 설정
*	@details	없음
*	@param CString strInputMode		설정할 입력모드
*	@retval		FALSE
*****************************************************************/
BOOL CNInputEditBox::SetInputType(CString &strData)
{
	m_nInputType = Asc2Int(strData);

	return FALSE;
}

/** *************************************************************
*	@brief	ThreeTypeBox 생성자
*	@param	CTL_TYPE type  resource control type
*	@retval	없음
*****************************************************************/
CNThreeTypeBox::CNThreeTypeBox(CTL_TYPE type) : CNEditTextBox(type)
{
	for (int i = 0; i < 3; i++)
	{
		m_pStatePen[i] = NULL;
		m_pStateBrush[i] = NULL;
		m_pStateFont[i] = NULL;
		m_pStateTextColor[i] = NULL;
	}

	m_nCurState = STATE_ENABLE;
}

/** *************************************************************
*	@brief	InputEditBox 소멸자
*	@retval	없음
*****************************************************************/
CNThreeTypeBox::~CNThreeTypeBox()
{
}

/** *************************************************************
*	@brief	Pen Color 설정
*	@param	CNPen *pDisable		비활성화 되었을 때의 Pen Color
*	@param	CNPen *pEnable		활성화 되었을 때의 Pen Color
*	@param	CNPen *pSelect		선택 되었을 때의 Pen Color
*	@retval	없음
*****************************************************************/
void CNThreeTypeBox::SetStatePen(CNPen *pDisable, CNPen *pEnable, CNPen *pSelect)
{
	m_pStatePen[STATE_DISABLE] = pDisable;
	m_pStatePen[STATE_ENABLE] = pEnable;
	m_pStatePen[STATE_SELECT] = pSelect;
}

/** *************************************************************
*	@brief	Brush Color 설정
*	@param	CNBrush *pDisable	비활성화 되었을 때의 Brush Color
*	@param	CNBrush *pEnable	활성화 되었을 때의 Brush Color
*	@param	CNBrush *pSelect	선택 되었을 때의 Brush Color
*	@retval	없음
*****************************************************************/
void CNThreeTypeBox::SetStateBrush(CNBrush *pDisable, CNBrush *pEnable, CNBrush *pSelect)
{
	m_pStateBrush[STATE_DISABLE] = pDisable;
	m_pStateBrush[STATE_ENABLE] = pEnable;
	m_pStateBrush[STATE_SELECT] = pSelect;
}

/** *************************************************************
*	@brief	Font 설정
*	@param	CNFont *pDisable	비활성화 되었을 때의 Font
*	@param	CNFont *pEnable	활성화 되었을 때의 Font
*	@param	CNFont *pSelect	선택 되었을 때의 Font
*	@retval	없음
*****************************************************************/
void CNThreeTypeBox::SetStateFont(CNFont *pDisable, CNFont *pEnable, CNFont *pSelect)
{
	m_pStateFont[STATE_DISABLE] = pDisable;
	m_pStateFont[STATE_ENABLE] = pEnable;
	m_pStateFont[STATE_SELECT] = pSelect;
}

/** *************************************************************
*	@brief	Font Color 설정
*	@param	CNColor *pDisable	비활성화 되었을 때의 Font Color
*	@param	CNColor *pEnable	활성화 되었을 때의 Font Color
*	@param	CNColor *pSelect	선택 되었을 때의 Font Color
*	@retval	없음
*****************************************************************/
void CNThreeTypeBox::SetStateTextColor(CNColor *pDisable, CNColor *pEnable, CNColor *pSelect)
{
	m_pStateTextColor[STATE_DISABLE] = pDisable;
	m_pStateTextColor[STATE_ENABLE] = pEnable;
	m_pStateTextColor[STATE_SELECT] = pSelect;
}

/** *************************************************************
*	@brief	변수 초기화
*	@retval	없음
*****************************************************************/
BOOL CNThreeTypeBox::Initialize(void)
{
	m_nCurState = STATE_DISABLE;

	return TRUE;
}

BOOL CNThreeTypeBox::SetVariable(CString &strName, CString &strData)
{
	if (m_strNameEx[0] == strName)
	{
		strData.MakeLower();

		// Disable
		if (_tcsncmp(strData, DES_DISABLE_TYPE, _tcslen(DES_DISABLE_TYPE)) == 0)
		{
			m_nCurState = STATE_DISABLE;
		}
		// Enable
		else if (_tcsncmp(strData, DES_ENABLE_TYPE, _tcslen(DES_ENABLE_TYPE)) == 0)
		{
			m_nCurState = STATE_ENABLE;
		}
		// Select
		else if (_tcsncmp(strData, DES_SELECT_TYPE, _tcslen(DES_SELECT_TYPE)) == 0)
		{
			m_nCurState = STATE_SELECT;
		}

		AddInvalidRect(m_rcDisplay);
		return TRUE;
	}
	else if(m_strNameEx[1] == strName)
	{
		m_strText = strData;

		AddInvalidRect(m_rcDisplay);
		return TRUE;
	}

	return FALSE;
}

BOOL CNThreeTypeBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	/////////////////////////////////////
	//	DRAW BODY
	{
		HPEN	OldPen;
		HBRUSH	OldBrush;
		
		// Draw Boarder and Fill Color
		if (m_pStatePen[m_nCurState] != NULL || m_pStateBrush[m_nCurState] != NULL)
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
				::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, 
					m_rcDisplay.right, m_rcDisplay.bottom);
			}
			
			// Restore Brush
			::SelectObject(hDC, OldBrush);
			
			// Restore Pen.
			::SelectObject(hDC, OldPen);
		}
#ifdef AE_SCREEN_DEGISN
		else
		{
			// Select Pen
			OldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
			OldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			
			// Draw Rectangle.
			::Rectangle(hDC, m_rcDisplay.left, m_rcDisplay.top, m_rcDisplay.right, m_rcDisplay.bottom);
			
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
				rcClient = m_rcDisplay;
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

///////////////////////////////////////////////////////////////
//
//	CNGifBox
//

/** *************************************************************
*	@brief	CNGifBox 생성자
*	@param	CTL_TYPE type  resource control type
*	@retval	없음
*****************************************************************/
CNGifBox::CNGifBox(CTL_TYPE type/*=CTL_GIFBOX*/) : CNShape(type)
{
	m_nImageIndex = 0;
	m_nLoadImage = 0;

	m_bShow = TRUE;
}

/** *************************************************************
*	@brief	CNGifBox 소멸자
*	@retval	없음
*****************************************************************/
CNGifBox::~CNGifBox()
{

}

/** *************************************************************
*	@brief	초기화
*	@retval	없음
*****************************************************************/
BOOL CNGifBox::Initialize(void)
{
	m_bShow = TRUE;
	m_nImageIndex = 0;
	m_nBlinkCount = 0;

	SetBlinking();	// animation이 처음부터 화면에 표시 되도록.

	return TRUE;
}

/** *************************************************************
*	@brief	gif 파일 정보 세팅
*	@param	CString strFileName  gif 파일 이름
*	@retval	없음
*****************************************************************/
BOOL CNGifBox::SetAniImage(CString strFileName)
{
	LPCTSTR pFileName[1];

	pFileName[0] = (LPCTSTR)strFileName;

	m_Image.SetDecodeInfo(pFileName, 1, FALSE);
	m_Image.DecodeImage4GIF();
	m_nLoadImage = m_Image.GetImageCount();
	
	return TRUE;
}

/** *************************************************************
*	@brief	이미지 draw
*	@param	HDC hDC			hDC
*	@param	RECT &rcUpdate  rect
*	@retval	없음
*****************************************************************/
BOOL CNGifBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;

	if (m_bShow == FALSE)
		return FALSE;

	if (IsUpdateControl(rcUpdate) == FALSE)
		return FALSE;

	bRes = m_Image.Draw(hDC, m_nImageIndex, &m_rcDisplay);

	return bRes;
}

BOOL CNGifBox::SetBlinking(void)
{
	if (!m_bShow)
		return TRUE;

	m_nImageIndex++;
	if (m_nImageIndex >= m_nLoadImage)
		m_nImageIndex = 0;

	AddInvalidRect(m_rcDisplay);

	return TRUE;
}

/** *************************************************************
*	@brief	이미지 display 여부
*	@param	CString &strName	strName
*	@param	CString &strData	strData
*	@retval	없음
*****************************************************************/
BOOL CNGifBox::SetVariable(CString &strName, CString &strData)
{
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


///////////////////////////////////////////////////////////////
//
//	CNImageBox
//

CNImageBox::CNImageBox(CTL_TYPE type/* =CTL_IMAGEBOX */) : CNTextBox(type)
{
	m_bShow = TRUE;
	m_bImageShow = TRUE;
}

CNImageBox::~CNImageBox()
{
	m_Image.ReleaseImage();
}

BOOL CNImageBox::Initialize()
{
	m_Image.ReleaseImage();

	return TRUE;
}

BOOL CNImageBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	BOOL	bRes = FALSE;

	if (TRUE == m_bShow)
	{
		if (TRUE == m_bImageShow)
		{
			if (IsUpdateControl(rcUpdate) == FALSE)
				return FALSE;

			bRes = m_Image.Draw(hDC, 0, &m_rcDisplay);
			if (bRes == TRUE)
				AddInvalidRect(rcUpdate);
		}
		else
		{
			return CNTextBox::DrawControl(hDC, rcUpdate);
		}
	}

	return bRes;
}

BOOL CNImageBox::SetVariable(CString &strName, CString &strData)
{
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

			// 만약 actkey가 없으면 이벤트를 처리하지 않은것으로 간주한다.
			if (m_ActKey.GetLength() == 0)
				return FALSE;

			return TRUE;
		}
		// APValueXX
		else
		{
			m_strText = strData;

			// 값이 설정되어 있으면 Show, 그렇지 않으면 Hide
			if (m_strText.GetLength() == 0)
			{
				m_bShow = FALSE;
			}
			else
			{
				m_bShow = TRUE;

				LPCTSTR pFileName[1];

				pFileName[0] = (LPCTSTR)strData;

				m_Image.SetDecodeInfo(pFileName, 1, FALSE);

				if (!m_Image.DecodeImage(0))
				{
					m_strText.Empty();
					m_strText = _T("NO IMAGE");
					m_bImageShow = FALSE;
				}
				else
				{
					m_bImageShow = TRUE;
				}
			}

			AddInvalidRect(m_rcDisplay);

			return TRUE;
		}
	}

	return FALSE;
}


///////////////////////////////////////////////////////////////
//
//	CNEditButtonBox
//
CNEditButtonBox::CNEditButtonBox(CTL_TYPE type) : CNTextBox(type)
{
	m_bShow = TRUE;
}

CNEditButtonBox::~CNEditButtonBox()
{
}

BOOL CNEditButtonBox::Initialize(void)
{
	CNShape::Initialize();

	if (m_strName.GetLength() > 0)
	{
		if (_tcsncmp((LPCTSTR)m_strName, DES_AP_STATE, _tcslen(DES_AP_STATE)) == 0)
		{
			m_bShow = TRUE;
		}
	}

	return TRUE;
}

BOOL CNEditButtonBox::FindActBtn(CString &strKey)
{
	return KeyEvent(strKey);
}

BOOL CNEditButtonBox::MouseEvent(POINT &point)
{
	if (PtInRect(&m_rcDisplay, point))
		return KeyEvent(m_ActKey);
	
	return FALSE;
}

BOOL CNEditButtonBox::KeyEvent(CString &strKey)
{
	if (m_bShow)
	{
		// 1자는 [1, F1]을 Find에서 같게 처리함으로 제외함.
		if (strKey.GetLength() > 1 && m_ActKey.Find(strKey) >= 0)
	//	if (m_ActKey.Find(strKey) >= 0)
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
			{
				AddRequest(m_Act);
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNEditButtonBox::SetVariable(CString &strName, CString &strData)
{
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

			// 만약 actkey가 없으면 이벤트를 처리하지 않은것으로 간주한다.
			if (m_ActKey.GetLength() == 0)
				return FALSE;

			return TRUE;
		}
		// APValueXX
		else
		{
			m_strText = strData;

			AddInvalidRect(m_rcDisplay);
			
			return TRUE;
		}

	}

	return FALSE;
}

BOOL CNEditButtonBox::DrawControl(HDC hDC, RECT &rcUpdate)
{
	if (m_bShow)
	{
		return CNTextBox::DrawControl(hDC, rcUpdate);
	}

	return FALSE;
}

BOOL CNEditButtonBox::SetCtlFocus(CString &strName, CString &strData)
{
	if (m_strName == strName)
	{
		m_bFocus = TRUE;
	}
	else
	{
		m_bFocus = FALSE;
	}
	
	AddInvalidRect(m_rcDisplay);
	
	return FALSE;
}