// FlashViewerDlg.h : header file
//

#pragma once

#include ".\Scr\ScrCtrl.h"
#include "afxwin.h"

// CFlashViewerDlg dialog
class CFlashViewerDlg : public CDialog
{
// Construction
public:
	CFlashViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FLASHVIEWER_DIALOG };

	CScrCtrl	*m_pFlash;
	CScrCtrl	*m_pPicasso;

	CString		m_strOutput;

	int RegSetValueExt(HKEY hKey, LPCTSTR hKeyName, LPCTSTR lpszValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonFlashCreate();
	afx_msg void OnBnClickedButtonPicassoCreate();
	CString m_strFlashX;
	CString m_strFlashY;
	CString m_strFlashWidth;
	CString m_strFlashHeight;
	CString m_strPicassoX;
	CString m_strPicassoY;
	CString m_strPicassoWidth;
	CString m_strPicassoHeight;
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonFlashLoadMovie();
	afx_msg void OnBnClickedButtonFlashMoveWindow();
	afx_msg void OnBnClickedButtonPicassoLoadScreen();
	afx_msg void OnBnClickedButtonPicassoLoadScreenWhite();
	afx_msg void OnBnClickedButtonPicassoLoadScreenBlack();
	afx_msg void OnBnClickedButtonPicassoLoadScreenMount();
	afx_msg void OnBnClickedButtonPicassoMoveWindow();
	afx_msg void OnBnClickedButtonPicassoScreenShow();
	afx_msg void OnBnClickedCtlShow();
	afx_msg void OnBnClickedCtlUpdate();
	BOOL m_bFlashEngineSelect;
	CComboBox m_ctlScreenNumber;
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CListBox m_listOutPut;
	CComboBox m_Language;
	CString m_strTimeout;
};
