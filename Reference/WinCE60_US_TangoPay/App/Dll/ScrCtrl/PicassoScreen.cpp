#include "stdafx.h"
#include "PicassoScreen.h"
#include "PicassoCtrlDescription.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		0
#define DBG_INFO		1


//------------------------------------------------------------------
//	Global Variable
//------------------------------------------------------------------
int		CPicassoScreen::m_nCreateCount = 0;

//------------------------------------------------------------------
//	CPicassoScreen Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: CPicassoScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoScreen::CPicassoScreen()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nUniqueNumber = m_nCreateCount++;
	m_strID = _T("");
	m_pBackScreen = NULL;

	m_pRequests = NULL;
	m_pInvalidRect = NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: ~CPicassoScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CPicassoScreen::~CPicassoScreen()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	Deinitialize();
	DeleteControl();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: DeleteControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::DeleteControl()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	CPicassoControl *pControl = NULL;

	while (m_Controls.GetFromHeadOfList(&pControl, 0))
	{
		if (pControl == NULL)
			continue;

		CTL_TYPE ctlType = pControl->GetType();
		switch (ctlType)
		{
		case CTL_TEXTBOX: delete ((CPicassoTextBox*)pControl); break;
		case CTL_MULTITEXTBOX: delete ((CPicassoMultiTextBox*)pControl); break;
		case CTL_HIDETEXTBOX: delete ((CPicassoHideTextBox*)pControl); break;
		case CTL_EFFECTTEXTBOX: delete ((CPicassoEffectTextBox*)pControl); break;
		case CTL_EDITTEXTBOX: delete ((CPicassoEditTextBox*)pControl); break;
		case CTL_TABLEEDITTEXTBOX: delete ((CPicassoTableEditTextBox*)pControl); break;
		case CTL_BUTTONBOX: delete ((CPicassoButtonBox*)pControl); break;
		case CTL_VKEYBUTTONBOX: delete ((CPicassoVirtualKeyButtonBox*)pControl); break;
		case CTL_CHANGEPWBOX: delete ((CPicassoChangePasswordBox*)pControl); break;
		case CTL_SWKEYBOX: delete ((CPicassoSoftwareKeyBox*)pControl); break;
		case CTL_ALPHAKEYBOX: delete ((CPicassoAlphaKeyBox*)pControl); break;
		case CTL_TABLEKEYBOX: delete ((CPicassoTableKeyBox*)pControl); break;
		case CTL_NUMBERKEYBOX: delete ((CPicassoNumberKeyBox*)pControl); break;
		case CTL_TOGGLEBOX: delete ((CPicassoToggleBox*)pControl); break;
		case CTL_THREESTATEBOX: delete ((CPicassoThreeStateBox*)pControl); break;
		case CTL_EJNLBOX: delete ((CPicassoEJNLBox*)pControl); break;
		case CTL_DATEBOX: delete ((CPicassoDateBox*)pControl); break;
		case CTL_IPBOX: delete ((CPicassoIPBox*)pControl); break;
		case CTL_BINBOX: delete ((CPicassoBinBox*)pControl); break;
		case CTL_CENTBOX: delete ((CPicassoCentBox*)pControl); break;
		case CTL_DOLLARBOX: delete ((CPicassoDollarBox*)pControl); break;
		case CTL_ANI: delete ((CPicassoAnimation*)pControl); break;
		case CTL_ADVBOX: delete ((CPicassoAdvBox*)pControl); break;
		case CTL_TRANADVBOX: delete ((CPicassoTranAdvBox*)pControl); break;
		case CTL_BININDEXBOX: delete ((CPicassoBinIndexBox*)pControl); break;
		case CTL_WEATHERBOX: delete ((CPicassoWeatherBox*)pControl); break;
		case CTL_VKEYBOARDBOX: delete ((CPicassoVKeyBoardBox*)pControl); break;
		case CTL_IMAGELISTBOX: delete ((CPicassoImageListBox*)pControl); break;
		case CTL_BUTTONIMAGELISTBOX: delete ((CPicassoButtonImageListBox*)pControl); break;
		case CTL_AMOUNTBOX: delete ((CPicassoAmountBox*)pControl); break;
		case CTL_KEYLISTBOX: delete ((CPicassoKeyListBox*)pControl); break;
		case CTL_CUSTOMEDITBOX: delete ((CPicassoCustomEditBox*)pControl); break;
		case CTL_DECIMALBOX: delete ((CPicassoDecimalBox*)pControl); break;
		default:
			delete pControl;
		}

		//delete pControl;
		pControl = NULL;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: SetRequestQueue()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::SetRequestQueue(CNHLinkedList<CPicassoRequest> *pRequestQueue)
{
	CNHAutoLock	AutoSync(m_csNHScreen);
	m_pRequests = pRequestQueue;
	m_RootHandler.SetRequestQueue(m_pRequests);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: SetInvalidRect()
 WRITER       : AIREAT (2009.12.14)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::SetInvalidRect(RECT *pRect)
{
	CNHAutoLock	AutoSync(m_csNHScreen);
	m_pInvalidRect = pRect;
	m_RootHandler.SetInvalidRect(m_pInvalidRect);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::Initialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	CPicassoRequest	Init(REQ_INITIAL, _T(""), _T(""));

	m_nScreenTimeout = 0;

	// clear.
	m_RootHandler.ClearRequest();
	m_RootHandler.Handler(Init);

	m_RootHandler.ClearInvalidRect();

	m_FSCMD.DeleteAll();
	m_bInputDisable = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: Deinitialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::Deinitialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	m_RootHandler.Handler(CPicassoRequest(REQ_DEINITIAL, _T(""), _T("")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: AddRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoScreen::AddRequest(CPicassoRequest &rRequest)
{
/*
	if (((rRequest.GetKind() == REQ_MOUSEEVENT) || (rRequest.GetKind() == REQ_KEYEVENT))
		&& (m_bInputDisable == TRUE))
	{
		return FALSE;	
	}
*/	
	CNHAutoLock	AutoSync(m_csNHScreen);

	return m_RootHandler.AddRequest(rRequest);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: ProcessRequest()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  :  TRUE : 요청을 계속 처리 해야 한다.
                FALSE : 요청 처리가 끝났다.
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoScreen::ProcessRequest(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	CPicassoRequest	Req;

	while(m_RootHandler.GetRequest(Req))
	{
		m_RootHandler.Handler(Req);

		if ((Req.GetKind() == REQ_FSCMD) || (Req.GetKind() == REQ_ALLFSCMD) || (Req.GetKind() == REQ_FULLFSCMD))
		{
			m_bInputDisable = TRUE;
			m_FSCMD.PutOnTailOfList(_T(""), Req);
		}
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: GetID()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString CPicassoScreen::GetID()
{
	CNHAutoLock	AutoSync(m_csNHScreen);

	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_strID;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: SetID()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::SetID(CString strID)
{
	CNHAutoLock	AutoSync(m_csNHScreen);

	NHUIDBG(DBG_CALL, (_T("\n")));

	m_strID = strID;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: GetUniqueNumber()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CPicassoScreen::GetUniqueNumber()
{
	CNHAutoLock	AutoSync(m_csNHScreen);

	return m_nUniqueNumber;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: GetFSCmd()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoScreen::GetFSCmd(CPicassoRequest &FSCmd)
{
	CNHAutoLock	AutoSync(m_csNHScreen);

	NHUIDBG(DBG_CALL, (_T("\n")));

	return m_FSCMD.GetFromHeadOfList(FSCmd);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: IsUpdate()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoScreen::IsUpdate(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	RECT	rcRect;
	rcRect = m_RootHandler.GetInvalidRect();

	if (rcRect.left || rcRect.top || rcRect.right || rcRect.bottom)
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: GetUpdateRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoScreen::GetUpdateRect(RECT *lpRect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	*lpRect = m_RootHandler.GetInvalidRect();

	m_RootHandler.ClearInvalidRect();

	if (lpRect->left == SCR_COORD_MAX && lpRect->top == SCR_COORD_MAX &&
		lpRect->right == SCR_COORD_MIN && lpRect->bottom == SCR_COORD_MIN)
		return FALSE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: ClearUpdateRect()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::ClearUpdateRect(void)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	m_RootHandler.ClearInvalidRect();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: SetBackScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::SetBackScreen(CPicassoScreen *pScreen)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	if (pScreen != NULL)
		m_pBackScreen = pScreen;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: SetAddControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::SetAddControl(CPicassoControl *pControl)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	CPicassoControl *pTemp = NULL;

	if (pControl)
	{
		m_Controls.MoveLast(&pTemp);
		
		if (pTemp)
			pTemp->SetNextHandler(pControl);
		else
			m_RootHandler.SetNextHandler(pControl);

		pControl->SetRequestQueue(m_pRequests);
		pControl->SetInvalidRect(m_pInvalidRect);

		// Add List
		m_Controls.PutOnTailOfList(_T(""), pControl);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: Draw()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoScreen::Draw(HDC hDC, int LocaleNum, RECT *pUpdateRect, BOOL bDrawBack)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	BOOL		bDrawed = FALSE;
	CPicassoRequest	req;

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

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: DrawBackground()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoScreen::DrawBackground(HDC hDC, int LocaleNum, RECT *pUpdateRect)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	if (m_pBackScreen == NULL)
		return FALSE;

	if (m_pBackScreen->Draw(hDC, LocaleNum, pUpdateRect) == TRUE)
		return TRUE;

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: GetScreenTimeout()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
UINT CPicassoScreen::GetScreenTimeout()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	return m_nScreenTimeout;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoScreen
 FUNCTION NAME: SetScreenTimeout()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoScreen::SetScreenTimeout(UINT Timeout)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CNHAutoLock	AutoSync(m_csNHScreen);

	m_nScreenTimeout = Timeout;
}

BOOL CPicassoScreen::GetInputDisable()
{
	CNHAutoLock	AutoSync(m_csNHScreen);

	return m_bInputDisable;
}

void CPicassoScreen::SetInputEnable()
{
	CNHAutoLock	AutoSync(m_csNHScreen);

	m_bInputDisable = FALSE;
}

CString	CPicassoScreen::GetBackScreenID()
{
	CNHAutoLock	AutoSync(m_csNHScreen);

	if (m_pBackScreen != NULL)
	{
		return m_pBackScreen->GetID();
	}

	return L"";
}

