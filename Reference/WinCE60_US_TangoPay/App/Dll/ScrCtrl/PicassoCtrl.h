#ifndef __PICASSO_CTRL_H__
#define __PICASSO_CTRL_H__

//#include "ScreenEngineDefine.h"
#include ".\Dll\NHReadiniFile.h"
//#include ".\ScreenDescType.h"
#include ".\PicassoCtrlDescription.h"
#include ".\PicassoCtrlShare.h"
#include ".\PicassoScreen.h"
#include "..\..\H\dll\AssetArchive.h"

struct	CACHE_SCREENS
{
	CString		strID;
	HBITMAP		hBitmap;
};
#define	MAX_CACHE_SCREEN		5
#define ID_CACHE_SCREEN			(199900)

#define WM_FS_COMMAND_MSG	WM_USER+1000

// [#2186] US KMK 2013.04.10 Asian Language Support
#define GET_LANGUAGE_SHORT_STRING(SELECT_LANGUAGE)	\
	((SELECT_LANGUAGE==6) ? (_T("JPN")) :	\
	((SELECT_LANGUAGE==5) ? (_T("KOR")) :	\
	((SELECT_LANGUAGE==4) ? (_T("CHN")) : \
	((SELECT_LANGUAGE==3) ? (_T("FRN")) : \
	((SELECT_LANGUAGE==2) ? (_T("SPN")) : (_T("ENG")))))))
// end of [#2186]

UINT ThPicassoCtrl(LPVOID param);

class CPicassoCtrl : public CWnd
{
public:
	CPicassoCtrl();
	~CPicassoCtrl();

public:
	BOOL	Initialize(int nWidth, int nHeight);
	BOOL	SetVariable(CString strCmd, CString strData);
	BOOL	SetVariable(CScrMsg &ApMsg);
	void	SetLoadScreenSize(int nWidth, int nHeight);

	BOOL	IsFontLoaded(int nLocale) { return m_bIsFontLoaded[nLocale]; }		// [#2186] US KMK 2013.04.30

	int					m_nPointCount;
	CPoint				m_arPoint[1024];

	// some articles recommend to use AfxBeginThread() rather than CreateThread() for UI related tasks (they say CreateThread() is not safe)
	// below are modified as 'public' to use AfxBeginThread() in CPicassoCtrl
	bool				m_isWorkingThread;
	int					m_nScreenWidth;
	int					m_nScreenHeight;
	CString				m_strScreenID;
	CNHQueue<CScrMsg>	m_VariableQueue;
	void				SendFSCommand(CString strCommand, CString strData);
	void				ProcEppEvent(CString strKeyCode);
	void				ProcMouseEvent(POINT &pos);
	void				ProcVariableEvent(CString strCmd, CString strData);
	void				PrepareCacheScreen(CString strFileName);
	BOOL				LoadSection(CString strScreenPath, CString strSection);

private:
	CWinThread*			m_pPicassoThread;
	CNHCritSec			m_csNHProcess;
//	CNHCritSec			m_csReDraw;
	BOOL				m_bExit;
	BOOL				m_bInit;
	BOOL				m_bInputEnabled;
	BOOL				m_bDecodeCash;
	BOOL				m_bLoadControl;
	CString				m_strLoadSection;
	CString				m_strLoadCountry;
	CString				m_strSupportLanguage;

	HWND				m_hDisplayWnd;
	HWND				m_hParentWnd;
	int					m_nLogPixelsY;
	int					m_TimeoutMode;
	CString				m_strScreenPath;

	SIZE				m_LoadScreenSize;

	CPicassoScreen*			m_pDisplayScreen;
	CPicassoScreen*			m_pCurrentScreen;
	CPicassoScreen*			m_pNextScreen;

	BOOL				m_bBackRefresh;
	BOOL				m_bBackDrawed;
	CString				m_strBackID;

	HBITMAP				m_bmBackBuffer;
	HBITMAP				m_bmScreenBuffer;
	CACHE_SCREENS		m_eCacheScreens[MAX_CACHE_SCREEN];

	int					m_nCurrentLocale;
	int					m_nMaxLocale;

	BOOL				m_bLocalMode;
	HFONT				m_hLocalFont;
	CString				m_strLangFonts[MAX_LOCALE];		// [#2186] US KMK 2013.04.08 Asian Language Support
	BOOL				m_bIsFontLoaded[MAX_LOCALE];	// [#2186] US KMK 2013.04.08 Asian Language Support

	CString				m_strDebugMsg;
	CString				m_strModeMsg;

	CPicassoScreen*					m_pNScreen;
	CPicassoTextBox*				m_pNTextBox;
	CPicassoHideTextBox*			m_pNHideTextBox;
	CPicassoEffectTextBox*			m_pNEffectTextBox;
	CPicassoEditTextBox*			m_pNEditTextBox;
	CPicassoChangePasswordBox*		m_pNChangePWBox;
	CPicassoButtonBox*				m_pNButtonBox;
	CPicassoVirtualKeyButtonBox*	m_pNVKeyButtonBox;
	CPicassoSoftwareKeyBox*			m_pNSWKeyBox;
	CPicassoToggleBox*				m_pNToggleBox;
	CPicassoEJNLBox*				m_pNEJNLBox;
	CPicassoDateBox*				m_pNDateBox;
	CPicassoIPBox*					m_pNIPBox;
	CPicassoBinBox*					m_pNBinBox;
	CPicassoCentBox*				m_pNCentBox;
	CPicassoDollarBox*				m_pNDollarBox;
	CPicassoAdvBox*					m_pNAdvBox;
	CPicassoAnimation*				m_pNAniBox;
	CPicassoTableEditTextBox*		m_pNTableEditBox;
	CPicassoAlphaKeyBox*			m_pNAlphaKeyBox;
	CPicassoNumberKeyBox*			m_pNNumberKeyBox;
	CPicassoTableKeyBox*			m_pNTableKeyBox;
	CPicassoThreeStateBox*			m_pNThreeStateBox;
	CPicassoBinIndexBox*			m_pNBinIndexBox;
	CPicassoLocaleText*				m_pNLocaleText;
	CPicassoVKeyBoardBox*			m_pVKeyBoardBox;
	CPicassoImageListBox*			m_pImageListBox;
	CPicassoButtonImageListBox*		m_pButtonImageListBox;
	CPicassoAmountBox*				m_pAmountBox;
	CPicassoKeyListBox*				m_pKeyListBox;
	CPicassoWeatherBox*				m_pWeatherBox;
	CPicassoTranAdvBox*				m_pTranAdvBox;
	CPicassoCustomEditBox*			m_pNCustomEditBox;	// [#2065] NH KJW 2011.05.26
	CPicassoDecimalBox*				m_pNDecimalBox;		// [#2316] NH Justin 2014.12.17
	CPicassoTextSet*				m_pNTextSet;		// [#RWC6-399] US ryan.payton 2022.09.15 DigitalMint
	CPicassoMultiTextBox*			m_pMultiTextBox;

	ADVERTISE_INFO				m_AdvertiseInfo;
	_NOTICE_INFO				m_NoticeInfo;
	_ADV_NOTICE_WELCOME_INFO	m_AdvNoticeWelcomeInfo;
	
	CNHLinkedList<CPicassoRequest>	m_Requests;
	RECT							m_InvalidRect;

	CNHPtrLinkedList< CPicassoResource >	m_PicassoResources;
	CMap< CString, LPCTSTR, CPicassoResource*, CPicassoResource* >			m_PicassoResourcesCMap;
public:
	CMap< CString, LPCTSTR, CPicassoScreen*, CPicassoScreen* >				m_PicassoScreensCMap;

	bool							m_bOverrideResourceValue;

private:
	void PrepareScreen(CString strScreenNumber, BOOL PrevSetScreen);
	void SetValueState(CPicassoScreen *pScreen, CString &strCommand, CString &strData, BOOL bValueType);
	void SetApMaxTime(CPicassoScreen *pScreen, CString &strCommand, CString &strData);
	void SetApTimeoutMode(CPicassoScreen *pScreen, CString &strCommand, CString &strData);
	void SetApUpdate(CPicassoScreen *pScreen, CString &strCommand, CString &strData);
	void SetOtherSetting(CPicassoScreen *pScreen, CString &strCommand, CString &strData);
	void ChangeBlinking(CPicassoScreen *pScreen, CString &strCommand, CString &strData);
	void UpdateScreen(RECT *prcRect, BOOL bErase);
	
private:
	void AdvertiseInitialize();
	void AdvertiseDeinitialize();
	int  AdvertiseGetNextScreen();

private:
	BOOL LoadScreenValue(LPCTSTR FileName, CNHReadiniFile::OpModeFlags Type);
	BOOL CreateResource(PINIVALUE pIniValue);
	BOOL CreateScreen(PINIVALUE	pIniValue);
	BOOL CreateTextBox(PINIVALUE pIniValue);
	BOOL CreateHideTextBox(PINIVALUE pIniValue);
	BOOL CreateEffectTextBox(PINIVALUE pIniValue);
	BOOL CreateEditBox(PINIVALUE pIniValue);
	BOOL CreateChangePWBox(PINIVALUE pIniValue);
	BOOL CreateButtonBox(PINIVALUE pIniValue);
	BOOL CreateVKeyButtonBox(PINIVALUE pIniValue);
	BOOL CreateSWKeyBox(PINIVALUE pIniValue);
	BOOL CreateEnDisableBox(PINIVALUE pIniValue);
	BOOL CreateEJNLBox(PINIVALUE pIniValue);
	BOOL CreateDateBox(PINIVALUE pIniValue);
	BOOL CreateIPBox(PINIVALUE pIniValue);
	BOOL CreateBinBox(PINIVALUE	pIniValue);
	BOOL CreateCentBox(PINIVALUE pIniValue);
	BOOL CreateDollarBox(PINIVALUE pIniValue);
	BOOL CreateAdvBox(PINIVALUE	pIniValue);
	BOOL CreateTranAdvBox(PINIVALUE pIniValue);
	BOOL CreateAniBox(PINIVALUE	pIniValue);
	BOOL CreateTableEditBox(PINIVALUE pIniValue);
	BOOL CreateAlphaKeyBox(PINIVALUE pIniValue);
	BOOL CreateNumberKeyBox(PINIVALUE pIniValue);
	BOOL CreateTableKeyBox(PINIVALUE pIniValue);
	BOOL CreateThreeStateBox(PINIVALUE pIniValue);
	BOOL CreateBinIndexBox(PINIVALUE pIniValue);
	BOOL CreateVKeyBoardBox(PINIVALUE pIniValue);
	BOOL CreateImageListBox(PINIVALUE pIniValue);
	BOOL CreateButtonImageListBox(PINIVALUE pIniValue);
	BOOL CreateAmountBox(PINIVALUE pIniValue);
	BOOL CreateKeyListBox(PINIVALUE pIniValue);
	BOOL CreateWeatherBox(PINIVALUE pIniValue);
	BOOL CreateCustomEditBox(PINIVALUE pIniValue);	// [#2065] NH KJW 2011.05.26	// [#2350] US Justin Change Name
	BOOL CreateDecimalBox(PINIVALUE pIniValue);		// [#2316] NH Justin 2014.12.17
	BOOL CreateMultiTextBox(PINIVALUE pIniValue);

	BOOL LoadControlSetting(PINIVALUE pIniValue);

	BOOL SettingControl(CPicassoControl *pControl, PINIVALUE	pIniValue);
	BOOL SettingShape(CPicassoShape *pShape, PINIVALUE pIniValue);
	BOOL SettingText(CPicassoTextBox *pTextBox, PINIVALUE	pIniValue);
	BOOL SettingVKey(CPicassoVirtualKeyBox *pVKeyBox, PINIVALUE pIniValue);

	BOOL LoadScreenText(LPCTSTR FileName);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};

#endif //__PICASSO_CTRL_H__