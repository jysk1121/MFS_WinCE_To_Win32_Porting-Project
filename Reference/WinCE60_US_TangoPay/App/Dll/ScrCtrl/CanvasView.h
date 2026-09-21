#ifndef __CANVAS_VIEW_H__
#define __CANVAS_VIEW_H__

#include ".\Scr\ScrCtrl.h"
#include ".\Dll\NHCollection.h"
#include "QueueData.h"

class CCanvasView : public CScrCtrl
{
public:
	~CCanvasView();
protected:
	CCanvasView(SCREEN_TYPE type);

private:
	BOOL					m_bInit;
	BOOL					m_bSetTopMostWnd;

protected:
	static	BOOL			m_bEnableHideWnd;

protected:
	CNHQueue<CQueueData>	m_FSCmdQueue;
	SCREEN_TYPE				m_eScreenType;
	HWND					m_hFrameWnd;
	HWND					m_hViewWnd;
	CNHEvent				m_eEndOfCreate;
	CNHEvent				m_eWaitObject;
	CString					m_strAppName;
	CString					m_strCurrentScreenNumber;
	int						m_x, m_y, m_nWidth, m_nHeight;

protected:
	void MakeTopMostWindow();
	void MakeBottomWindow();
	virtual	BOOL SendVariable(CString strCommand, CString strData) = 0;
	virtual BOOL SendKeyEvent(CString strKey) = 0;

public:
	virtual BOOL	Initialize();
	virtual void	Deinitialize();
	virtual BOOL	LoadScreens(CString strPath, CString strSection, int nLoadWidth, int nLoadHeight);
	virtual BOOL	MoveScreen(int x, int y, int width, int height, BOOL bMakeTopMost=TRUE);
	virtual	void	SetEnableHideWindow(BOOL bEnable);
	virtual void	HideWindow();
	virtual SCREEN_TYPE GetScreenType();

#ifndef UNDER_CE
	virtual RECT	GetScreenRect();
#endif

	void SetActiveScreen();

	BOOL SetVariable(CString strCommand, CString strData);
	BOOL SetKeyEvent(CString strKey);
	BOOL SetByPassData(CString strData);

	BOOL GetUserInputCheck(DWORD dwTimeout = 20);
	BOOL GetUserInputData(CString &strCommand, CString &strData);
	void ClearUserInputData();

	CString	GetCurrentScreenNumber();
};

#endif //__CANVAS_VIEW_H__