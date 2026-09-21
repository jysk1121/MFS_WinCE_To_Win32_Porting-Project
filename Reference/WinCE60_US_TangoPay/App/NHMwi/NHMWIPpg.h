#if !defined(AFX_NHMWIPPG_H__D20D5158_75A6_48B1_9979_EAF94E61C0CF__INCLUDED_)
#define AFX_NHMWIPPG_H__D20D5158_75A6_48B1_9979_EAF94E61C0CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// NHMWIPpg.h : Declaration of the CNHMWIPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CNHMWIPropPage : See NHMWIPpg.cpp.cpp for implementation.

class CNHMWIPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CNHMWIPropPage)
	DECLARE_OLECREATE_EX(CNHMWIPropPage)

// Constructor
public:
	CNHMWIPropPage();

// Dialog Data
	//{{AFX_DATA(CNHMWIPropPage)
	enum { IDD = IDD_PROPPAGE_NHMWI };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CNHMWIPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NHMWIPPG_H__D20D5158_75A6_48B1_9979_EAF94E61C0CF__INCLUDED)
