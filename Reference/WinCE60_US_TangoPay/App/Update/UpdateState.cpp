#include "StdAfx.h"
#include "UpdateState.h"

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------
CNHLinkedList<CUpdateStateList>*	CUpdateState::m_pStateList = NULL;
BOOL		CUpdateState::m_bInit = FALSE;		// State initialize flag
CNHEvent	CUpdateState::m_oWaitObject(TRUE);	// Manual reset event.
HINSTANCE	CUpdateState::m_hInstance = NULL;
CScrCtrl*	CUpdateState::m_pScrCtrl = NULL;
CNHConfig	CUpdateState::m_SystemConfig;

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: CUpdateState()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUpdateState::CUpdateState()
{
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: ~CUpdateState()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CUpdateState::~CUpdateState()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: Initialize()
 RETURN TYPE  : Nonzero if it is successful; otherwise, it is zero.
 PARAMETER    : 
 DESCRIPTION  : 초기화를 수행한다.
-------------------------------------------------------------------*/
BOOL CUpdateState::Initialize(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
	m_bInit = TRUE;

	NH_SCR_CONFIG eUpdateScreenConfig = m_SystemConfig.GetScreenConfig(SCR_UPDATE);

	// Create Screen
	m_pScrCtrl = CScrCtrl::CreateScreenEngine((SCREEN_TYPE)eUpdateScreenConfig.eEngine,
											  0,
											  0,
											  eUpdateScreenConfig.nWidth,
											  eUpdateScreenConfig.nHeight);

	m_pScrCtrl->Initialize();

	// Move Screen
	m_pScrCtrl->MoveScreen(eUpdateScreenConfig.nX, 
						   eUpdateScreenConfig.nY, 
						   eUpdateScreenConfig.nWidth, 
						   eUpdateScreenConfig.nHeight);

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: Deinitialize()
 RETURN TYPE  : Nonzero if it is successful; otherwise, it is zero.
 PARAMETER    : State Information struct
 DESCRIPTION  : 정리를 수행한다.
-------------------------------------------------------------------*/
void CUpdateState::Deinitialize()
{
	m_bInit = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: Register()
 RETURN TYPE  : Nonzero if it is successful; otherwise, it is zero.
 PARAMETER    : State Information struct
 DESCRIPTION  : State들이 생성될때 자신을 등록하는 Interface.
-------------------------------------------------------------------*/
BOOL CUpdateState::Register(CString Name, CUpdateState *pState)
{
	CUpdateStateList	StateInfo;

	if (m_pStateList == NULL)
	{
		m_pStateList = (CNHLinkedList<CUpdateStateList>*) new CNHLinkedList<CUpdateStateList>;
	}

	StateInfo.m_strName = Name;
	StateInfo.m_pState = pState;
	
	return m_pStateList->PutOnTailOfList(Name, StateInfo);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: WaitForMilliSecond()
 RETURN TYPE  : 
 PARAMETER    : ms : 대기할 MilliSecond
 DESCRIPTION  : MilliSecond 만큼 대기한다.
-------------------------------------------------------------------*/
void CUpdateState::WaitForMilliSecond(int ms)
{
	m_oWaitObject.Wait(ms);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: GetState()
 RETURN TYPE  : A point to the State indicates successful, NULL indicates failure.
 PARAMETER    : State ID
 DESCRIPTION  : ID에 해당하는 State를 반환한다.
-------------------------------------------------------------------*/
CUpdateState* CUpdateState::GetState(CString StateName)
{
	CUpdateStateList StateInfo;

	if (m_bInit == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"STATE DIDN'T INITIAL.."));
		return NULL;
	}

	if (m_pStateList->FindDataOfList(StateName, StateInfo) == FALSE)
	{
		NHDEBUG(DBG_INFO, (L"STATE NOT FIND [%s]", StateName));
		return NULL;
	}

	if (StateInfo.m_pState == NULL)
	{
		NHDEBUG(DBG_INFO, (L"STATE IS NULL [%s]", StateName));
		return NULL;
	}

	return StateInfo.m_pState;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: StartProcess()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : State를 처리하기 전에 Debug Message를 출력한다.
-------------------------------------------------------------------*/
void CUpdateState::StartProcess()
{
	NHDEBUG(DBG_INFO, (L"State : START[%s]\n", m_strName));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: StartProcess()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : State를 처리한다. 
				이 함수의 구현은 서브클래스에서 하므로 호출되서는 안된다.
-------------------------------------------------------------------*/
CString CUpdateState::Process(CUpdateStateInfo &StateInfo)
{
	NHDEBUG(DBG_INFO, (L"NO STATE IMPLE.[%s]", m_strName));

	ASSERT(0);

	return (_T(""));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateState
 FUNCTION NAME: StartProcess()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : State를 종료하기 전에 Debug Message를 출력한다.
-------------------------------------------------------------------*/
void CUpdateState::EndProcess(CString strNextState)
{
	NHDEBUG(DBG_INFO, (L"State : END[%s], NEXT[%s]\n", m_strName, strNextState));
}