// WinAtm.h : main header file for the WINATM application
//

#if !defined(AFX_WINATM_H__6661FFFC_CD11_4FC3_AD08_6B3A809CAFF2__INCLUDED_)
#define AFX_WINATM_H__6661FFFC_CD11_4FC3_AD08_6B3A809CAFF2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CWinAtmApp:
// See WinAtm.cpp for the implementation of this class
//

class CWinAtmApp : public CWinApp
{
private:
	bool	InstallNextwareRegistries();

	bool	InstallWinAtmEssentials();
	bool	InstallImageResources();

	bool	InstallAsianFonts();

	bool	InstallCustomerAds();
	bool	InstallCustomerBacks();
	bool	InstallCustomerRcptHeader();
	bool	InstallTTS_DB();			// [#2558] NH Justin 2018.06.12 Fix fof S/W Update Failure after "Format and Update"
	bool	InstallExtendedBinList();
	bool	InstallCAFile();

public:
	CWinAtmApp();

	BOOL	m_bStopReversalRetry;		// [#5] NH PSC 2008.03.12 무한 reversal 을 중지하기 위한 flag

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinAtmApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWinAtmApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINATM_H__6661FFFC_CD11_4FC3_AD08_6B3A809CAFF2__INCLUDED_)
