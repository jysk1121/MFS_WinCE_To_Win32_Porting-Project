// WinAtmDoc.h : interface of the CWinAtmDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINATMDOC_H__96AA39B2_BFBE_466A_ABB3_FB52C52DEA3A__INCLUDED_)
#define AFX_WINATMDOC_H__96AA39B2_BFBE_466A_ABB3_FB52C52DEA3A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CWinAtmDoc : public CDocument
{
protected: // create from serialization only
	CWinAtmDoc();
	DECLARE_DYNCREATE(CWinAtmDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinAtmDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWinAtmDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWinAtmDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINATMDOC_H__96AA39B2_BFBE_466A_ABB3_FB52C52DEA3A__INCLUDED_)
