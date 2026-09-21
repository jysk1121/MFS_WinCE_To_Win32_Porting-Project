/*----------------------------------------------------------------------

	State를 관리하고 호출을 담당하는 클래스.

	작성자 : AIREAT
	작성일 : 2009.01.29
----------------------------------------------------------------------*/
#ifndef __NH_KEYMGR_STATE_MANAGER_H__
#define __NH_KEYMGR_STATE_MANAGER_H__

#include ".\Scr\ScrCtrl.h"
#include ".\Dll\KeyMgrStateInfo.h"
#include ".\Dll\KeyMgrStateDefine.h"

class AFX_CLASS_EXPORT CKeyMgrStateManager
{
// Construction
public:
	CKeyMgrStateManager();
	~CKeyMgrStateManager();

// Operations
private:
	CKeyMgrStateInfo	m_StateInfo;

// Attributes
public:
	// 초기화를 수행한다.
	BOOL	Initialize(CScrCtrl *pScrCtrl);
	// 정리를 수행한다.
	void	Deinitialize();
	// State 처리를 시작한다.
	//void	Run(CString strInitState=STATE_KM_INIT);
	CString	Run(CString strInitState=STATE_KM_INIT);
	void	SetLanguage(int nLangKind);
};

#endif //__NH_KEYMGR_STATE_MANAGER_H__