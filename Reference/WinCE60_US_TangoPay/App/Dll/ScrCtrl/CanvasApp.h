#ifndef __CANVAS_APP_H__
#define __CANVAS_APP_H__

#include "CanvasFrame.h"

class CCanvasApp : public CWinThread
{
	DECLARE_DYNCREATE(CCanvasApp)

public:
	CCanvasApp();
	~CCanvasApp();

public:
	void SetWindowInfo(CString strName, CWnd *pWnd, int x, int y, int width, int height);

public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance();
	//virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing

private:
	CCanvasFrame* pFrame;
	CString		m_strAppName;
	CWnd		*m_pWnd;
	int			m_x, m_y, m_nWidth, m_nHeight;
};

#endif // __CANVAS_APP_H__