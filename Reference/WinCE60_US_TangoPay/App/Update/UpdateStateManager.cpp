#include "stdafx.h"
#include "UpdateState.h"
#include "UpdateStateManager.h"

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateManager
 FUNCTION NAME: CUpdateStateManager()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUpdateStateManager::CUpdateStateManager()
{
//	Initialize();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateManager
 FUNCTION NAME: ~CUpdateStateManager()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CUpdateStateManager::~CUpdateStateManager()
{
	Deinitialize();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateManager
 FUNCTION NAME: Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 초기화를 수행한다.
-------------------------------------------------------------------*/
BOOL CUpdateStateManager::Initialize(HINSTANCE hInstance)
{
	return CUpdateState::Initialize(hInstance);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateManager
 FUNCTION NAME: Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 객체 정리를 수행한다.
-------------------------------------------------------------------*/
void CUpdateStateManager::Deinitialize()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateManager
 FUNCTION NAME: Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 객체 정리를 수행한다.
-------------------------------------------------------------------*/
void CUpdateStateManager::Run(CString strInitState/* =_T("Init" */)
{
	CString strNextState = strInitState;
	CUpdateState	*pState = NULL;

	m_StateInfo.Clear();
	m_StateInfo.GetGlobalVariable();

	while(1)
	{
		pState = CUpdateState::GetState(strNextState);
		if (pState == NULL)
		{
			NHERROR((L"<CUpdateStateManager::Run> Can't find state(%s) !!\n", strNextState));
			break;
		}

		// print debug information
		pState->StartProcess();
	
		strNextState = pState->Process(m_StateInfo);
		
		// print debug information
		pState->EndProcess(strNextState);
	}

	return;
}