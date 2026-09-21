#include "stdafx.h"
#include "ScreenDescType.h"
#include "SCR_Screen.h"


CNScreen::CNScreen()
{
	m_strID = _T("");
	m_pBackScreen = NULL;
}

CNScreen::~CNScreen()
{
	Deinitialize();
	DeleteControl();
}

void CNScreen::DeleteControl()
{
	CNControl *pControl = NULL;

	while (m_Controls.GetFromHeadOfList(&pControl, 0))
	{
		if (pControl == NULL)
			continue;

		if (pControl->GetType() == CTL_TEXTBOX)
		{
			CNTextBox *g_pNTextBox = (CNTextBox*)pControl;
			delete g_pNTextBox;
		}
		else if (pControl->GetType() == CTL_EDITTEXTBOX)
		{
			CNEditTextBox *g_pNEditTextBox = (CNEditTextBox*)pControl;
			delete g_pNEditTextBox;
		}
		else if (pControl->GetType() == CTL_BUTTONBOX)
		{
			CNButtonBox	*g_pNButtonBox = (CNButtonBox*)pControl;
			delete g_pNButtonBox;
		}
		else if (pControl->GetType() == CTL_CHANGEPWBOX)
		{
			CNChangePWBox *g_pNChangePWBox = (CNChangePWBox*)pControl;
			delete g_pNChangePWBox;
		}
		else if (pControl->GetType() == CTL_SWKEYBOX)
		{
			CNSWKeyBox *g_pNSWKeyBox= (CNSWKeyBox*)pControl;
			delete g_pNSWKeyBox;
		}
		else if (pControl->GetType() == CTL_TOGGLEBOX)
		{
			CNToggleBox	*g_pNToggleBox = (CNToggleBox*)pControl;
			delete g_pNToggleBox;
		}
		else if (pControl->GetType() == CTL_EJNLBOX)
		{
			CNEJNLBox *g_pNEJNLBox = (CNEJNLBox*)pControl;
			delete g_pNEJNLBox;
		}
		else if (pControl->GetType() == CTL_DATEBOX)
		{
			CNDateBox *g_pNDateBox = (CNDateBox*)pControl;
			delete g_pNDateBox;
		}
		else if (pControl->GetType() == CTL_IPBOX)
		{
			CNIPBox *g_pNIPBox = (CNIPBox*)pControl;
			delete g_pNIPBox;
		}
		else if (pControl->GetType() == CTL_BINBOX)
		{
			CNBinBox *g_pNBinBox = (CNBinBox*)pControl;
			delete g_pNBinBox;
		}
		else if (pControl->GetType() == CTL_CENTBOX)
		{
			CNCentBox *g_pNCentBox = (CNCentBox*)pControl;
			delete g_pNCentBox;
		}
		else if (pControl->GetType() == CTL_DOLLARBOX)
		{
			CNDollarBox *g_pNDollarBox = (CNDollarBox*)pControl;
			delete g_pNDollarBox;
		}
		else if (pControl->GetType() == CTL_ANI)
		{
			CNAnimation *g_pNAniBox = (CNAnimation*)pControl;
			delete g_pNAniBox;
		}
		else if (pControl->GetType() == CTL_ADVBOX)
		{
			CNAdvBox *g_pNAdvBox = (CNAdvBox*)pControl;
			delete g_pNAdvBox;
		}
		else if (pControl->GetType() == CTL_BININDEXBOX)
		{
			CNBinIndexBox  *g_pNBinIndexBox = (CNBinIndexBox*)pControl;
			delete g_pNBinIndexBox;
		}
		else if (pControl->GetType() == CTL_DECIMALBOX)
		{
			CNDecimalBox  *g_pNDecimalBox = (CNDecimalBox*)pControl;
			delete g_pNDecimalBox;
		}
		else if (pControl->GetType() == CTL_CENTCLEARBOX)
		{
			CNCentClearBox  *g_pNCentClearBox = (CNCentClearBox*)pControl;
			delete g_pNCentClearBox;
		}
		// V1.0.2.4 2018.06.26 - GIFBOX Ãß°¡
		else if (pControl->GetType() == CTL_GIFBOX)
		{
			CNGifBox *g_pNGifBox = (CNGifBox*)pControl;
			delete g_pNGifBox;
		}
		//////////////////////////////
		else
			delete pControl;

		pControl = NULL;
	}
}

void CNScreen::Initialize()
{
	CRequest	Init(REQ_INITIAL, _T(""), _T(""));

	m_nScreenTimeout = 0;

	// clear.
	m_RootHandler.ClearRequest();
	m_RootHandler.Handler(Init);

	m_RootHandler.ClearInvalidRect();

	m_FSCMD.Set(REQ_INITIAL, _T(""), _T(""));
}

void CNScreen::Deinitialize()
{
	m_RootHandler.Handler(CRequest(REQ_DEINITIAL, _T(""), _T("")));
}

BOOL CNScreen::AddRequest(CRequest &rRequest)
{
	return m_RootHandler.AddRequest(rRequest);
}

BOOL CNScreen::ProcessRequest(void)
{
	CRequest	Req;

	while(m_RootHandler.GetRequest(Req))
	{
		m_RootHandler.Handler(Req);

		if ((Req.GetKind() == REQ_FSCMD) || (Req.GetKind() == REQ_ALLFSCMD) || (Req.GetKind() == REQ_FULLFSCMD))
			m_FSCMD = Req;
	}

	return TRUE;
}

CString CNScreen::GetID()
{
	return m_strID;
}

void CNScreen::SetID(CString strID)
{
	m_strID = strID;
}

BOOL CNScreen::GetFSCmd(CRequest &FSCmd)
{
	if ((m_FSCMD.GetKind() == REQ_FSCMD) ||
		(m_FSCMD.GetKind() == REQ_FULLFSCMD) ||
		(m_FSCMD.GetKind() == REQ_ALLFSCMD))
	{
		FSCmd = m_FSCMD;
		m_FSCMD.Set(REQ_INITIAL, _T(""), _T(""));

		return TRUE;
	}

	return FALSE;
}

BOOL CNScreen::IsUpdate(void)
{
	RECT	rcRect;
	rcRect = m_RootHandler.GetInvalidRect();

	if (rcRect.left || rcRect.top || rcRect.right || rcRect.bottom)
		return TRUE;

	return FALSE;
}

BOOL CNScreen::GetUpdateRect(RECT *lpRect)
{
	*lpRect = m_RootHandler.GetInvalidRect();

	m_RootHandler.ClearInvalidRect();

	if (lpRect->left == SCR_COORD_MAX && lpRect->top == SCR_COORD_MAX &&
		lpRect->right == SCR_COORD_MIN && lpRect->bottom == SCR_COORD_MIN)
		return FALSE;

	return TRUE;
}

void CNScreen::ClearUpdateRect(void)
{
	m_RootHandler.ClearInvalidRect();
}

void CNScreen::SetBackScreen(CNScreen *pScreen)
{
	if (pScreen != NULL)
		m_pBackScreen = pScreen;
}

void CNScreen::SetAddControl(CNControl *pControl)
{
	CNControl *pTemp = NULL;

	if (pControl)
	{
		m_Controls.MoveLast(&pTemp);
		
		if (pTemp)
			pTemp->SetNextHandler(pControl);
		else
			m_RootHandler.SetNextHandler(pControl);

		// Add List
		m_Controls.PutOnTailOfList(_T(""), pControl);
	}
}

BOOL CNScreen::Draw(HDC hDC, int LocaleNum, RECT *pUpdateRect, BOOL bDrawBack)
{
	BOOL		bDrawed = FALSE;
	CRequest	req;

	if (bDrawBack)
		bDrawed |= DrawBackground(hDC, LocaleNum, pUpdateRect);

	// set locale information
	req.SetKinD(REQ_LOCALE);
	req.m_nLocale = LocaleNum;
	m_RootHandler.Handler(req);

	// request draw
	req.SetKinD(REQ_DRAW);
	req.m_hDC = hDC;
	req.m_rcUpdate = *pUpdateRect;
	bDrawed |= m_RootHandler.Handler(req);
	
	return bDrawed;
}

BOOL CNScreen::DrawBackground(HDC hDC, int LocaleNum, RECT *pUpdateRect)
{
	if (m_pBackScreen == NULL)
		return FALSE;

	if (m_pBackScreen->Draw(hDC, LocaleNum, pUpdateRect) == TRUE)
		return TRUE;

	return FALSE;
}

UINT CNScreen::GetScreenTimeout()
{
	return m_nScreenTimeout;
}

void CNScreen::SetScreenTimeout(UINT Timeout)
{
	m_nScreenTimeout = Timeout;
}
