#ifndef __PICASSO_VIEW_H__
#define __PICASSO_VIEW_H__

#include "CanvasView.h"
#include "PicassoCtrl.h"
#include "CanvasApp.h"

class CPicassoView : public CWnd, public CCanvasView
{
	DECLARE_MESSAGE_MAP()

public:
	CPicassoView(int x, int y, int width, int height);
	~CPicassoView();

private:
	CCanvasApp*		m_pNewApp;
	CPicassoCtrl	m_PicassoCtrl;
	POSITION		pos;				// 20160711

public:
	virtual BOOL	Initialize();
	virtual void	Deinitialize();
	virtual BOOL	LoadScreens(CString strPath, CString strSection, int nLoadWidth, int nLoadHeight);

	virtual int		GetScreenCount();
	virtual CString GetFirstScreenNumber();
	virtual CString GetNextScreenNumber();

	BOOL	IsFontLoaded(int nLocale) { return m_PicassoCtrl.IsFontLoaded(nLocale); }	// [#2186] US KMK 2013.04.30 

protected:
	BOOL SendVariable(CString strCommand, CString strData);
	BOOL SendKeyEvent(CString strKey);

	void SendKeyboardEvent(CString strKey);
	void SendMouseEvent(POINT &Postion);
	void SendVariableMsg(CString strCmd, CString strData);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd *pOldWnd);

	afx_msg LRESULT OnFSCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void PostNcDestroy();
	afx_msg void OnDestroy();

};

#endif //__PICASSO_VIEW_H__