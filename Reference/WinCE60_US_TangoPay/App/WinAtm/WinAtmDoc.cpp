// WinAtmDoc.cpp : implementation of the CWinAtmDoc class
//

#include "stdafx.h"
#include "WinAtm.h"

#include "WinAtmDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinAtmDoc

IMPLEMENT_DYNCREATE(CWinAtmDoc, CDocument)

BEGIN_MESSAGE_MAP(CWinAtmDoc, CDocument)
	//{{AFX_MSG_MAP(CWinAtmDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinAtmDoc construction/destruction

CWinAtmDoc::CWinAtmDoc()
{
	// TODO: add one-time construction code here

}

CWinAtmDoc::~CWinAtmDoc()
{
}

BOOL CWinAtmDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CWinAtmDoc serialization

void CWinAtmDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWinAtmDoc diagnostics

#ifdef _DEBUG
void CWinAtmDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWinAtmDoc commands
