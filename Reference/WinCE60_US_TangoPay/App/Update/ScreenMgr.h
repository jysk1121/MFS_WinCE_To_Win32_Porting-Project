#ifndef __SCREEN_MANAGER_H__
#define __SCREEN_MANAGER_H__

#include "SysMgr.h"
#include "UpdateStateScreen.h"

#define SCR_CMD_DELIMITER	_T("\r")		// [#60] NH AIREAT 2008.3.31 scr cmd delimiter 변경 \n->\r

//////////////////////////////////////////////////////////////////////////
//	구동에 필요한 준비작업을 수행 한다.

class CScrMgr: public CSystemMgr
{
protected:
	CScrMgr();
public:
	~CScrMgr();

protected:
	BOOL	LoadScreen();							// 화면을 로드한다.
	void	CreateFileFromResource(int nResID, CString strSaveFileName);	// TEXT 리소스를 파일로 변환한다.

	CString	GetKeyString(DWORD dwWaitSec = INFINITE);							// 화면으로 부터 입력을 받는다.

	void	PrepareScreen(CString strScreen);		// 화면 표시를 준비 한다.
	void	ShowScreen(CString strScreen);			// 화면을 표시한다.
	void	UpdateScreen();							// 현재 화면을 Refresh 한다.
	void	ShowString(int index, CString strStr);	// 화면의 스트링을 제어한다.
	void	ShowButton(int index, BOOL bOn);		// 화면의 버튼을 제어한다.
};

#endif // __SCREEN_MANAGER_H__
