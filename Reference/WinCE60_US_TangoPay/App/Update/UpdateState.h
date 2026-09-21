#ifndef __NH_STATE_H__
#define __NH_STATE_H__

#include ".\Dll\NHLinkedList.h"
#include ".\Scr\ScrCtrl.h"

#include "UpdateStateDefine.h"
#include "UpdateStateInfo.h"

#include ".\Common\CmnLib.h"


class CUpdateState;

// ----------------------------------------------------------------------------
//	State List를 관리하는 Class
// ----------------------------------------------------------------------------
class CUpdateStateList
{
public:
	CUpdateStateList()
	{
		m_strName = L"";
		m_pState = NULL;
	};

	CUpdateStateList(const CUpdateStateList &Msg)
	{
		m_strName = Msg.m_strName;
		m_pState = Msg.m_pState;
	};

	CUpdateStateList& operator= (const CUpdateStateList &Msg)
	{
		this->m_strName = Msg.m_strName;
		this->m_pState = Msg.m_pState;
		
		return *this;
	};

// Attributes
public:
	CString		m_strName;
	CUpdateState		*m_pState;
};

#define	TEMP_BUF_SIZE		2048


// ----------------------------------------------------------------------------
//	State의 추상 클래스
// ----------------------------------------------------------------------------
class CUpdateState
{
public:
	~CUpdateState();
//protected:
	CUpdateState();

// Attributes
private:
	// State를 관리하는 List
	static CNHLinkedList<CUpdateStateList>	*m_pStateList;
	static BOOL			m_bInit;						// State initialize flag
	static CNHEvent		m_oWaitObject;					// Wait Object
	
	// helper object
protected:
	static HINSTANCE	m_hInstance;
	static CScrCtrl		*m_pScrCtrl;					// Screen Control
	static CNHConfig	m_SystemConfig;

protected:
	// State 이름
	CString				m_strName;						// 서브 클래스들은 생성시 자신의 이름을 저장한다.
//	static char			m_TempBuf[TEMP_BUF_SIZE];		// 임시 버퍼 2K
	
// Operations
protected:
	// State들이 생성될때 자신을 등록하는 Interface
	static BOOL	Register(CString Name, CUpdateState *pState);
	// MilliSecond 만큼 대기한다.
	static void	WaitForMilliSecond(int ms);
public:
	// State의 초기화를 수행한다.
	static BOOL		Initialize(HINSTANCE hInstance);
	static void		Deinitialize();

	// StateName에 해당하는 State를 구한다.
	static CUpdateState* GetState(CString StateName);

	// State들의 처리를 시작한다. -- only Debug
	virtual void StartProcess();
	// State들의 처리를 위임한다.
	virtual CString Process(CUpdateStateInfo &StateInfo);
	// State들의 처리를 종료 한다. -- only Debug
	virtual void EndProcess(CString strNextState);
};

#endif
