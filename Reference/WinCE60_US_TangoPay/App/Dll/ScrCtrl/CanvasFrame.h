#ifndef __CANVAS_FRAME_H__
#define __CANVAS_FRAME_H__

class CCanvasFrame : public CFrameWnd
{
public:
	CCanvasFrame(CString strName, CWnd *pWnd, int x, int y, int width, int height);
public:
	virtual ~CCanvasFrame();

protected: 
	DECLARE_DYNAMIC(CCanvasFrame)

// Operations
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Data members
protected:
	CString		m_strAppName;
	CWnd*		m_pViewWnd;
	int			m_x, m_y, m_nWidth, m_nHeight;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	
public:
	afx_msg void OnDestroy();
	afx_msg void OnClose();
};


#endif // __SCR_FRM_H__