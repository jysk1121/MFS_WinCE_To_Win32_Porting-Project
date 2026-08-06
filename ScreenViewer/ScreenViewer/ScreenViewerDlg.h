// ScreenViewerDlg.h : header file
//

#if !defined(AFX_SCREENVIEWERDLG_H__D57596C2_ADF2_48C8_AD07_F7DFB652900A__INCLUDED_)
#define AFX_SCREENVIEWERDLG_H__D57596C2_ADF2_48C8_AD07_F7DFB652900A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CScreenViewerDlg dialog

#include "..\ScrCtrl\Dll\ScrCtrl.h"

class CScreenViewerDlg : public CDialog
{
// Construction
public:
	CScreenViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CScreenViewerDlg)
	enum { IDD = IDD_SCREENVIEWER_DIALOG };
	CListBox	m_listOutPut;
	CComboBox	m_comboScrNumber;
	CComboBox	m_comboLanguage;
	int		m_nResolution;
	CString	m_strRootDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenViewerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	CScrCtrl	m_ScrCtrl;
	CString		m_strScreenRootFolder;
	CString		m_strOutput;

	CString		m_strFocusNum;

	//*LEH
	BOOL Operator_Input_Text(CString strTitle, CString strInText, CString &strOutText, int nMaxInNumber=40, CString strInputDefaultMode=_T("NUMBER"));
	///////////////////////////////////////////////////////

	int		m_nVolLevel;
	void ChangeVolumnScreen();

	BOOL Operator_Input_Number(CString strTitleText, CString strInText, CString &strOutText, int nMaxInNumber=20, CString strInputType=_T("0"));
	BOOL Operator_Processing_Screen(CString strProcessingText, BOOL bResultScreen=FALSE, CString strErrorCode=_T("000000"));
	int m_nCurrentFocusIndex;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CScreenViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnLoad();
	afx_msg void OnBtnUnLoad();
	afx_msg void OnBtnDir();
	afx_msg void OnBtnShow();
	afx_msg void OnButton1();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnNext();
	afx_msg void OnBtnPrev();
	afx_msg void OnSelendokComboScrnum();
	afx_msg void OnSetFocusScreen();
	afx_msg void OnSetVolLevel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCREENVIEWERDLG_H__D57596C2_ADF2_48C8_AD07_F7DFB652900A__INCLUDED_)
