#ifndef __SYSTEM_CHECK_H__
#define __SYSTEM_CHECK_H__

#include ".\Common\NHCtrlThread.h"
#include <ddraw.h>

#pragma comment(lib, "Ddraw.lib")

#define GRAPHIC_WIDTH	(200)
#define GRAPHIC_HEIGHT	(80)
#define SQUARE_SIZE		((int)(GRAPHIC_HEIGHT / 4))
#define MOVE_SIZE		(5)
#define	HISTORY_MAX		((GRAPHIC_WIDTH/MOVE_SIZE) + 1)
#define INFO_WIDTH		(140)

#define SCR_TRANSCOLOR				RGB(255, 0, 255)

class CSystemCheck : protected CNHCtrlThread 
{
public:
	CSystemCheck();
	~CSystemCheck();

private:
	LPDIRECTDRAW			m_pDD;
	LPDIRECTDRAWSURFACE		m_pDDSPrimary;
	LPDIRECTDRAWSURFACE		m_pDDSOverlay;

	HBITMAP					m_hbmOverlay;

	RECT					m_rcOverlay;
	RECT					m_rcPrimary;
	RECT					m_rcGraphic;

	BOOL					m_bDoing;

	HANDLE					m_hIdleThread;
	char					m_chPowHistory[HISTORY_MAX];
	char					m_chMemHistory[HISTORY_MAX];
	DWORD					m_nPhysInitMemory;
	DWORD					m_nPhysCurMemory;
	DWORD					m_nPhysAvailMemory;

	HPEN					m_hpenDarkGreen;
	HPEN					m_hpenYellow;
	HPEN					m_hpenRed;

	HBRUSH					m_hbrushTransparent;

private:
	BOOL	CreateSurface();
	BOOL	CreateIdleThread();
	void	Measure();
	DWORD	GetThreadTick(FILETIME* a, FILETIME* b);
	BOOL	UpdateSurface();
	DWORD	DDColorMatch(IDirectDrawSurface * pdds, COLORREF rgb);

private:
	unsigned ThreadHandlerProc(void);
};

#endif //__SYSTEM_CHECK_H__