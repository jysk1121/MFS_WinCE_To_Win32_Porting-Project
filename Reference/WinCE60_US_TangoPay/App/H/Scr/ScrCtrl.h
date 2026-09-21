#ifndef __SCR_ABSTRACT_CTRL_H__
#define __SCR_ABSTRACT_CTRL_H__

#ifndef _SCRCTRL_OWNER_
#pragma comment(lib, "ScrCtrl.lib")
#endif

#include ".\Scr\ScrCtrlDefine.h"

class AFX_CLASS_EXPORT CScrCtrl
{
public:
	~CScrCtrl();
protected:
	CScrCtrl();

public:
	static CScrCtrl* CreateScreenEngine(SCREEN_TYPE type, int x, int y, int width, int height);
	static void		DestroyScreenEngine(CScrCtrl *pScrCtrl);

public:
	int				m_nFont;
	virtual BOOL	Initialize() = 0;
	virtual void	Deinitialize() = 0;
	virtual BOOL	LoadScreens(CString strPath, CString strSection, int nLoadWidth, int nLoadHeight) = 0;
	virtual void	SetActiveScreen() = 0;
	virtual BOOL	MoveScreen(int x, int y, int width, int height, BOOL bMakeTopMost=TRUE) = 0;
	virtual	void	SetEnableHideWindow(BOOL bEnable) = 0;
	virtual void	HideWindow() = 0;
	virtual SCREEN_TYPE GetScreenType() = 0;

	virtual BOOL	SetVariable(CString	strCommand, CString strData) = 0;
	virtual BOOL	SetKeyEvent(CString strKey) = 0;
	virtual BOOL	SetByPassData(CString strData) = 0;

	virtual BOOL	GetUserInputCheck(DWORD dwTimeout = 20) = 0;
	virtual BOOL	GetUserInputData(CString &strCommand, CString &strData) = 0;
	virtual void	ClearUserInputData() = 0;
	virtual	int		GetFontType(){return m_nFont;};

	virtual CString	GetCurrentScreenNumber() = 0;
//	void	SetScreenLocale(int Locale = 0);						// 화면의 LOCALE을 설정함.
//	int		GetScreenLocale();										// 화면의 LOCALE을 조회함.

	virtual BOOL	IsFontLoaded(int nLocale) { return FALSE; }		// US KMK 2013.05.06 혹시모를 오동작 방지를 위해 FALSE리턴으로 수정.

	/* 하위 호환성 앞으로 사라질 인터페이스틀.. */
public:
	// no implementation
	// 화면 정보 관련 인터페이스
	BOOL	SetDemoMode(BOOL bDemo = FALSE);						// 데모모드 정보를 화면에 표시 하도록 설정
	void	DrawDebugMsg(int x, int y, CString strMsg);

	virtual int		GetScreenCount() = 0;
	virtual CString GetFirstScreenNumber() = 0;
	virtual CString GetNextScreenNumber() = 0;
#ifndef UNDER_CE
	virtual RECT	GetScreenRect() = 0;
#endif
};


#endif __NH_SCR_ABSTRACT_CTRL_H__