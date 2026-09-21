#ifndef __PICASSO_CONTROLS_H__
#define __PICASSO_CONTROLS_H__

#include ".\PicassoControls.h"
#include ".\Dll\NHPtrLinkedList.h"

class CPicassoScreen
{
public:
	CPicassoScreen();
	~CPicassoScreen();

public:
	CString	GetID();
	void	SetID(CString strID);

	int		GetUniqueNumber();

	UINT	GetScreenTimeout();					// [#236] NZ AIREAT 2008.05.27
	void	SetScreenTimeout(UINT Timeout);		// [#236] NZ AIREAT 2008.05.27
	CString	GetBackScreenID();

	void	SetBackScreen(CPicassoScreen *pScreen);
	void	SetAddControl(CPicassoControl *pControl);

	BOOL	GetFSCmd(CPicassoRequest &FSCmd);

	void	Initialize(void);
	void	Deinitialize(void);
	BOOL	AddRequest(CPicassoRequest &rRequest);
	BOOL	ProcessRequest(void);
	BOOL	Draw(HDC hDC, int LocaleNum, RECT *pUpdateRect, BOOL bDrawBack=FALSE);
	BOOL	DrawBackground(HDC hDC, int LocaleNum, RECT *pUpdateRect);
	BOOL	IsUpdate(void);
	BOOL	GetUpdateRect(RECT *lpRect);
	void	ClearUpdateRect(void);
	BOOL	GetInputDisable();
	void	SetInputEnable();

	void	SetRequestQueue(CNHLinkedList<CPicassoRequest> *pRequestQueue);
	void	SetInvalidRect(RECT *pRect);

private:
	void	DeleteControl();
	
private:
	static	int	m_nCreateCount;
private:
	CNHCritSec	m_csNHScreen;
	CString		m_strID;
	UINT		m_nScreenTimeout;				// [#236] NZ AIREAT 2008.05.27
	BOOL		m_bInputDisable;
	int			m_nUniqueNumber;

	CPicassoScreen	*m_pBackScreen;

	CNHLinkedList<CPicassoRequest>	*m_pRequests;
	RECT							*m_pInvalidRect;

	CNHLinkedList< CPicassoRequest >	m_FSCMD;
	CPicassoEventHandler				m_RootHandler;
	CNHPtrLinkedList< CPicassoControl >	m_Controls;
};

#endif __PICASSO_CONTROLS_H__