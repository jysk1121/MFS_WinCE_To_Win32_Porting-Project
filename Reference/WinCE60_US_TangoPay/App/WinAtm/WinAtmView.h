// WinAtmView.h : interface of the CWinAtmView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINATMVIEW_H__697DEC2C_AA07_43FF_965D_582CD8BB541C__INCLUDED_)
#define AFX_WINATMVIEW_H__697DEC2C_AA07_43FF_965D_582CD8BB541C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CMainFrame;

class CWinAtmView : public CView
{
protected: // create from serialization only
	CWinAtmView();
	DECLARE_DYNCREATE(CWinAtmView)

// Attributes
public:
	CWinAtmDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinAtmView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWinAtmView();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWinAtmView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#ifndef _DEBUG  // debug version in WinAtmView.cpp
inline CWinAtmDoc* CWinAtmView::GetDocument()
   { return (CWinAtmDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINATMVIEW_H__697DEC2C_AA07_43FF_965D_582CD8BB541C__INCLUDED_)
