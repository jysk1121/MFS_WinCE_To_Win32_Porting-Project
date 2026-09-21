// WinAtmView.cpp : implementation of the CWinAtmView class
//

#include "stdafx.h"
#include "WinAtm.h"

#include "WinAtmDoc.h"
#include "WinAtmView.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinAtmView

IMPLEMENT_DYNCREATE(CWinAtmView, CView)

BEGIN_MESSAGE_MAP(CWinAtmView, CView)
	//{{AFX_MSG_MAP(CWinAtmView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinAtmView construction/destruction

CWinAtmView::CWinAtmView()
{
}

CWinAtmView::~CWinAtmView()
{
}

BOOL CWinAtmView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CWinAtmView drawing

void CWinAtmView::OnDraw(CDC* pDC)
{
	CWinAtmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CWinAtmView diagnostics

#ifdef _DEBUG
void CWinAtmView::AssertValid() const
{
	CView::AssertValid();
}


CWinAtmDoc* CWinAtmView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWinAtmDoc)));
	return (CWinAtmDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWinAtmView message handlers

