/*----------------------------------------------------------------------

	State를 관리하고 호출을 담당하는 클래스.

	작성자 : AIREAT
	작성일 : 2009.01.29
----------------------------------------------------------------------*/
#ifndef __NH_UPDATE_STATE_MANAGER_H__
#define __NH_UPDATE_STATE_MANAGER_H__

#include "UpdateStateInfo.h"
#include "UpdateStateDefine.h"

class CUpdateStateManager
{
// Construction
public:
	CUpdateStateManager();
	~CUpdateStateManager();

// Operations
private:
	CUpdateStateInfo	m_StateInfo;
// Attributes
public:
	// 초기화를 수행한다.
	BOOL	Initialize(HINSTANCE hInstance);
	// 정리를 수행한다.
	void	Deinitialize();
	// State 처리를 시작한다.
	void	Run(CString strInitState=STATE_UP_INIT);
};

#endif