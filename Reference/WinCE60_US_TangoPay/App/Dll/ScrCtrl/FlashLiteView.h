#ifndef __FLASH_LITE_VIEW_H__
#define __FLASH_LITE_VIEW_H__

#include "CanvasView.h"
#include "FlashLiteCtrl.h"
#include ".\CanvasApp.h"

class CFlashLiteView : public CWnd, public CCanvasView
{
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

public:
	CFlashLiteView(int x, int y, int width, int height);
	~CFlashLiteView();

private:
	CCanvasApp*			m_pNewApp;
	CFlashLiteCtrl		m_FlashCtrl;
	CString				m_strFlashMainPath;
	DWORD				m_dwLastShowTime;
	BOOL				m_bInputEnable;

public:
	virtual BOOL	Initialize();
	virtual void	Deinitialize();
	virtual BOOL	LoadScreens(CString strPath, CString strSection, int nLoadWidth, int nLoadHeight);

	virtual int		GetScreenCount();
	virtual CString GetFirstScreenNumber();
	virtual CString GetNextScreenNumber();

protected:
	BOOL SendVariable(CString strCommand, CString strData);
	BOOL SendKeyEvent(CString strKey);

	void ProcKeyEvent(CString strKey);
	void ProcVariable(CString strCmd, CString strData);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd *pOldWnd);

	afx_msg LRESULT OnFlashCreate(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFSCommandFlash(LPCTSTR pCmd, LPCTSTR pData);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
};

#endif //__FLASH_LITE_VIEW_H__