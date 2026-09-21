#include "stdafx.h"
#include "CanvasFrame.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1


//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CCanvasFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CCanvasFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasFrame
 FUNCTION NAME: CCanvasFrame()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CCanvasFrame::CCanvasFrame(CString strName, CWnd *pWnd, int x, int y, int width, int height)
{
	NHUIDBG(DBG_CALL, (_T("strName(%s), pWnd(0x%08X), x(%d), y(%d), width(%d), height(%d)\n"),
						strName, pWnd, x, y, width, height));

	m_strAppName = strName;
	m_pViewWnd = pWnd;
	m_x = x;
	m_y = y;
	m_nWidth = width;
	m_nHeight = height;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasFrame
 FUNCTION NAME: ~CCanvasFrame()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CCanvasFrame::~CCanvasFrame()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasFrame
 FUNCTION NAME: OnCreate()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CCanvasFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	NHUIDBG(DBG_CALL, (_T("TRY TO CREATE\n")));

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	NHUIDBG(DBG_CALL, (_T("SUCCESSFUL\n")));

	if (m_pViewWnd != NULL)
	{
		if (!m_pViewWnd->Create(NULL, m_strAppName, WS_CHILD | WS_VISIBLE, CRect(m_x, m_y, m_nWidth, m_nHeight), this, AFX_IDW_PANE_FIRST, NULL))
		{
			NHERROR((_T("WND NAME[%s] - ViewWnd Create Failed\n"), m_strAppName));
			return -1;
		}
	}

	return 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasFrame
 FUNCTION NAME: PreCreateWindow()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;

	cs.lpszClass = ::AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE, 0, 0, 0);

	cs.x = 0;
	cs.y = 0;
	cs.cx = 0;
	cs.cy = 0;

	cs.style &= ~WS_CAPTION;
	cs.style &= ~WS_SYSMENU;

	cs.lpszName = m_strAppName;

	NHUIDBG(DBG_INFO, (_T("Set CanvasView Style\n")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasFrame
 FUNCTION NAME: OnSetFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasFrame::OnSetFocus(CWnd* pOldWnd)
{
	NHUIDBG(DBG_CALL, (_T("pOldWnd(0x%08X)\n"), pOldWnd));

	CFrameWnd::OnSetFocus(pOldWnd);

	if (m_pViewWnd != NULL && ::IsWindow(m_pViewWnd->m_hWnd))
		m_pViewWnd->SetFocus();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasFrame
 FUNCTION NAME: OnCmdMsg()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
BOOL CCanvasFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_pViewWnd != NULL)
	{
		if (m_pViewWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CCanvasFrame::OnDestroy()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CFrameWnd::OnDestroy();

	// TODO: Add your message handler code here
}
void CCanvasFrame::OnClose()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	// TODO: Add your message handler code here and/or call default

	CFrameWnd::OnClose();
}
