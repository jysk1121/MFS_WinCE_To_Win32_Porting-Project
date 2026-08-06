// TestCEDLLDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "CommLib.h"

// CTestCEDLLDlg dialog
class CTestCEDLLDlg : public CDialog
{
// Construction
public:
	CTestCEDLLDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTCEDLL_DIALOG };

	void ShowResult(CString str);
	void ShowHexResult(CString strTag, byte *lpByte, unsigned short usDataLength);


	static int EventControlFunc(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);
	int eventHandler(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);
	int PressKeyHandler(unsigned short usParam, byte *lpByte, unsigned short usDataLength);
	BYTE CalcLRC(unsigned char *pData, DWORD dwLen);


	CCommLib	m_pCommLib;

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
	CComboBox m_ctrlPortNum;
	CListBox m_ctrlResult;
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonGetVersion();
	afx_msg void OnBnClickedButtonStartEntry();
	afx_msg void OnBnClickedButtonStopEntry();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
