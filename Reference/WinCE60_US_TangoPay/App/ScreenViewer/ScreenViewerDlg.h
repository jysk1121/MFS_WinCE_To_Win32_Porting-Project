// ScreenViewerDlg.h : header file
//

#if !defined(AFX_SCREENVIEWERDLG_H__D57596C2_ADF2_48C8_AD07_F7DFB652900A__INCLUDED_)
#define AFX_SCREENVIEWERDLG_H__D57596C2_ADF2_48C8_AD07_F7DFB652900A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CScreenViewerDlg dialog

#include "Scr\ScrCtrl.h"
#include "afxwin.h"
#include <queue>

static int m_nIdApValues[] = {	IDC_APVALUE_1, IDC_APVALUE_2, IDC_APVALUE_3, IDC_APVALUE_4, IDC_APVALUE_5,
									IDC_APVALUE_6, IDC_APVALUE_7, IDC_APVALUE_8, IDC_APVALUE_9, IDC_APVALUE_10,
									IDC_APVALUE_11, IDC_APVALUE_12, IDC_APVALUE_13, IDC_APVALUE_14, IDC_APVALUE_15,
									IDC_APVALUE_16, IDC_APVALUE_17, IDC_APVALUE_18, IDC_APVALUE_19, IDC_APVALUE_20,
									IDC_APVALUE_21, IDC_APVALUE_22, IDC_APVALUE_23, IDC_APVALUE_24, IDC_APVALUE_15,
									IDC_APVALUE_26, IDC_APVALUE_27, IDC_APVALUE_28, IDC_APVALUE_29, IDC_APVALUE_30,
};

static int m_nIdApStates[] = {	IDC_APSTATE_1, IDC_APSTATE_2, IDC_APSTATE_3, IDC_APSTATE_4, IDC_APSTATE_5,
									IDC_APSTATE_6, IDC_APSTATE_7, IDC_APSTATE_8, IDC_APSTATE_9, IDC_APSTATE_10,
									IDC_APSTATE_11, IDC_APSTATE_12, IDC_APSTATE_13, IDC_APSTATE_14, IDC_APSTATE_15,
									IDC_APSTATE_16, IDC_APSTATE_17, IDC_APSTATE_18, IDC_APSTATE_19, IDC_APSTATE_20,
};

enum TYPE_OF_SCREEN {
	SCR_TRX,
	SCR_OP,
	SCR_KEYMGR,
	SCR_COUNT
};

enum TYPE_OF_RESOLUTION {
	RES_640_480,
	RES_720_480,
	RES_800_600,
	RES_1024_600,
	RES_1024_768,
	RES_1024_768_W,
	RES_COUNT
};

enum TYPE_OF_SCREEN_RESOURCE {
	SCREEN_NOT_FOUND,
	FROM_CODE_DAT_SCREEN,
	FROM_MASTER_SCREEN,
	FROM_MASTER_TARBALL
};

#define GET_RES_WIDTH(RES) \
	((RES == RES_640_480) ? (640) : \
	(RES == RES_720_480) ? (720) : \
	(RES == RES_800_600) ? (800) : \
	(RES == RES_1024_600) ? (1024) : \
	(RES == RES_1024_768) ? (1024) : \
	(RES == RES_1024_768_W) ? (1024) : (1024))

#define GET_RES_HEIGHT(RES) \
	((RES == RES_640_480) ? (480) : \
	(RES == RES_720_480) ? (480) : \
	(RES == RES_800_600) ? (600) : \
	(RES == RES_1024_600) ? (600) : \
	(RES == RES_1024_768) ? (768) : \
	(RES == RES_1024_768_W) ? (768) : (600))

#define GET_LANG_STRING(LANG) \
	((LANG == 0) ? _T("ENGLISH") : \
	(LANG == 1) ? _T("SPANISH") : \
	(LANG == 2) ? _T("FRENCH") : \
	(LANG == 3) ? _T("CHINESE") : \
	(LANG == 4) ? _T("KOREAN") : \
	(LANG == 5) ? _T("JAPANESE") : _T("ENGLISH"))

#define GET_SCR_STRING(SCR) \
	((SCR == SCR_TRX) ? _T("Transaction") : \
	(SCR == SCR_OP) ? _T("Operator") : \
	(SCR == SCR_KEYMGR) ? _T("KeyManager") : _T("Unknown"))


/* sort windows with my monitor for now (2560x1440)
+--------++-----++----+
|1024    ||720  ||640 |
|600     ||480  ||480 |
|        |+-----++----+
|        |+-----+
+--------+|800  |
|1024    ||600  |
|768     |+-----+
|        |
|        |
+--------+

// GetSystemMetrics(SM_CXFRAME)		; returns 8 on Win10
// GetSystemMetrics(SM_CYFRAME)		; returns 8 on Win10
// GetSystemMetrics(SM_CXSIZEFRAME)	; returns 8 on Win10

*/
#define SCRPOS_X_OFFSET		100		// well, my taskbar is located on left side to use more height...
#define SCRPOS_Y_OFFSET		0
#define GET_SCRPOS_X(res) \
	((res) == RES_640_480 ? (SCRPOS_X_OFFSET + 1024 + 720 + (GetSystemMetrics(SM_CXFRAME) * 2)) : \
	(res) == RES_720_480 ? (SCRPOS_X_OFFSET + 1024 + (GetSystemMetrics(SM_CXFRAME) * 1)) : \
	(res) == RES_800_600 ? (SCRPOS_X_OFFSET + 1024 + (GetSystemMetrics(SM_CXFRAME) * 1)) : \
	(res) == RES_1024_600 ? (SCRPOS_X_OFFSET + 0) : \
	(res) == RES_1024_768 ? (SCRPOS_X_OFFSET + 0) : \
	(res) == RES_1024_768_W ? (SCRPOS_X_OFFSET + 0) : \
	(0))
#define GET_SCRPOS_Y(res) \
	((res) == RES_640_480 ? (SCRPOS_Y_OFFSET + 0) : \
	(res) == RES_720_480 ? (SCRPOS_Y_OFFSET + 0) : \
	(res) == RES_800_600 ? (SCRPOS_Y_OFFSET + 480 + (GetSystemMetrics(SM_CYFRAME) * 2)+GetSystemMetrics(SM_CYCAPTION)) : \
	(res) == RES_1024_600 ? (SCRPOS_Y_OFFSET + 0) : \
	(res) == RES_1024_768 ? (SCRPOS_Y_OFFSET + 600 + (GetSystemMetrics(SM_CYFRAME) * 2)+GetSystemMetrics(SM_CYCAPTION)) : \
	(res) == RES_1024_768_W ? (SCRPOS_Y_OFFSET + 600 + (GetSystemMetrics(SM_CYFRAME) * 2)+GetSystemMetrics(SM_CYCAPTION)) : \
	(0))


class CScreenViewerDlg : public CDialog
{
// Construction
public:
	CScreenViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CScreenViewerDlg)
	enum { IDD = IDD_SCREENVIEWER_DIALOG };
	CListBox		m_lsbStatusMessage;
	CComboBox		m_comboCountry;
	int				m_nScreenResourceLocation;
	
	CString			m_strRootDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenViewerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


	CString			m_strScreenRootFolder;
	CArray<CString>	m_arAPValues;
	CArray<int>		m_arAPStates;

	CComboBox		m_cboScrList_Trx;
	CComboBox		m_cboScrList_Op;
	CComboBox		m_cboScrList_KeyMgr;

	CScrCtrl*		m_pScreens_Trx[RES_COUNT];
	CScrCtrl*		m_pScreens_Op[RES_COUNT];
	CScrCtrl*		m_pScreens_KeyMgr[RES_COUNT];

	BOOL			m_bShowTrxScreens[RES_COUNT];
	BOOL			m_bShowOpScreens[RES_COUNT];
	BOOL			m_bShowKeyMgrScreens[RES_COUNT];

	int				m_nLanguage[SCR_COUNT];
	BOOL			m_bScreenLoaded[SCR_COUNT];
	BOOL			m_bResourceReady;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CScreenViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnLoadScreen_Trx();
	afx_msg void OnBtnDir();
	afx_msg void OnBtnApplyValues();
	afx_msg void OnButtonClearData();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnNextScr_Trx();
	afx_msg void OnBtnPrevScr_Trx();
	afx_msg void OnSelendokComboScrnum();
	

	void		InitControls();
	void			InitLogger();
	void		InitScreenList(TYPE_OF_SCREEN scr);

	BOOL		LoadScreen(TYPE_OF_SCREEN scr);
	BOOL			PrepareScreenResource();
	void				DetectResourceType();
	void				ConstructResourceFolder();
	void				DestructResourceFolder();
	BOOL		UnloadScreen(TYPE_OF_SCREEN scr);

	void		ShowPrevScreen(TYPE_OF_SCREEN scr);
	void		ShowNextScreen(TYPE_OF_SCREEN scr);
	void		ApplyScreenValues(TYPE_OF_SCREEN scr);
	void		LoadPresetValues(TYPE_OF_SCREEN scr);
	void		UpdateScreenVisibility(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res);

	// Helpers
	BOOL		 IsScreenVisible(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res);
	BOOL		 IsResolutionAvailable(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res);
	CScrCtrl**	 GetScrCtrlPtr(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res);
	CString		 GetScreenSection(TYPE_OF_SCREEN scr, TYPE_OF_RESOLUTION res);
	CComboBox*	 GetCBOScrListPtr(TYPE_OF_SCREEN scr);
	void		 ReadChosenResolutions();
	CString		 ReadCurrentAPValues();
	CString		 ReadCurrentAPStates();
	void		 ActivateScreenControls(TYPE_OF_SCREEN scr, BOOL bOnOff);


	std::queue<CString>			m_qLogMessages;
	HANDLE						m_hLogger;
	DWORD						m_nLoggerThreadId;
	static DWORD _stdcall		LoggerThread( void *pThreadHandler );

	void		 WriteStatus(LPCWSTR lpszFmt, ...);

	int			 GetRequiredComboDroppedWidth(CComboBox& rCombo);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSetVariable();
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnLoadOp();
	afx_msg void OnCbnSelchangeComboScrnumOp();
	afx_msg void OnBnClickedCheckOp1024600();
	afx_msg void OnBnClickedCheckOp800600();
	afx_msg void OnBnClickedCheckOp1024768();
	afx_msg void OnBnClickedCheckOp1024768_W();
	afx_msg void OnBnClickedCheckOp720480();
	afx_msg void OnBnClickedCheckOp640480();
	CButton m_chkRes800600_Op;
	CButton m_chkRes1024600_Op;
	CButton m_chkRes1024768_Op;
	CButton m_chkRes1024768_W_Op;
	CButton m_chkRes640480_Op;
	CButton m_chkRes720480_Op;
	afx_msg void OnBnClickedButtonLanguageOp();
	CButton m_btnLang_Op;
	afx_msg void OnBnClickedBtnLoadKeymgr();
	afx_msg void OnBnClickedCheckTrx640480();
	CButton m_chkRes640480_KeyMgr;
	CButton m_chkRes800600_KeyMgr;
	CButton m_chkRes1024600_KeyMgr;
	CButton m_chkRes1024768_KeyMgr;
	CButton m_chkRes640480_Trx;
	CButton m_chkRes1024768_Trx;
	CButton m_chkRes1024768_W_Trx;
	CButton m_chkRes800600_Trx;
	CButton m_chkRes1024600_Trx;
	afx_msg void OnBnClickedButtonLanguageTrx();
	CButton m_btnLang_Trx;
	afx_msg void OnBnClickedButtonLanguageKeymgr();
	CButton m_btnLang_KeyMgr;
	afx_msg void OnBnClickedCheckTrx1024768();
	afx_msg void OnBnClickedCheckTrx1024768_W();
	afx_msg void OnBnClickedCheckTrx800600();
	afx_msg void OnBnClickedCheckTrx1024600();
	afx_msg void OnBnClickedBtnPrevOp();
	afx_msg void OnBnClickedBtnNextOp();
	afx_msg void OnBnClickedBtnPrevKeymgr();
	afx_msg void OnBnClickedBtnNextKeymgr();
	afx_msg void OnBnClickedCheckKeymgr640480();
	afx_msg void OnBnClickedCheckKeymgr800600();
	afx_msg void OnBnClickedCheckKeymgr1024600();
	afx_msg void OnBnClickedCheckKeymgr1024768();
	afx_msg void OnBnClickedApState();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCREENVIEWERDLG_H__D57596C2_ADF2_48C8_AD07_F7DFB652900A__INCLUDED_)
