#ifndef __NH_SCREEN_H__
#define __NH_SCREEN_H__

#include "NHControl.h"
#include ".\Dll\NHPtrLinkedList.h"

class CNScreen
{
public:
	CNScreen();
	~CNScreen();

public:
	CString	GetID();
	void	SetID(CString strID);

	UINT	GetScreenTimeout();					// [#236] NZ AIREAT 2008.05.27
	void	SetScreenTimeout(UINT Timeout);		// [#236] NZ AIREAT 2008.05.27

	void	SetBackScreen(CNScreen *pScreen);
	void	SetAddControl(CNControl *pControl);

	BOOL	GetFSCmd(CRequest &FSCmd);

	void	Initialize(void);
	void	Deinitialize(void);
	BOOL	AddRequest(CRequest &rRequest);
	BOOL	ProcessRequest(void);
	BOOL	Draw(HDC hDC, int LocaleNum, RECT *pUpdateRect, BOOL bDrawBack=FALSE);
	BOOL	DrawBackground(HDC hDC, int LocaleNum, RECT *pUpdateRect);
	BOOL	IsUpdate(void);
	BOOL	GetUpdateRect(RECT *lpRect);
	void	ClearUpdateRect(void);

private:
	void	DeleteControl();
	
private:
	CString		m_strID;
	UINT		m_nScreenTimeout;				// [#236] NZ AIREAT 2008.05.27

	CNScreen	*m_pBackScreen;

	CRequest		m_FSCMD;
	CEventHandler	m_RootHandler;
	CNHPtrLinkedList< CNControl >	m_Controls;
};

#endif __NH_SCREEN_H__