// EvtQCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "..\nhmwi.h"
#include "DevCtrl.h"

#include "..\System\EventQue.h"
#include ".\Common\CommDef.h"
#include ".\Common\ConstDef.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDevCtrl

// ----------------------------------------------------------------------------
// 함 수 명 : CEventQue* CDevCtrl::GetEventQByDevID(int nDevID)
// DESCRIPT : 장치ID를 이용하여 장치와 연관된 이벤트큐의 포인터를 얻어온다.
// 매개변수 : 장치ID
// 반 환 값 : 이벤트큐의 포인터
// ----------------------------------------------------------------------------
CEventQue* CDevCtrl::GetEventQByDevID(int nDevID)
{
	CEventQue*	pQue = NULL;

	switch(nDevID)
	{
		case DEV_MCU		:	pQue = m_pMcuEventQ;		break;
		case DEV_RFID		:	pQue = m_pRFIDEventQ;		break;	// [#2325] NH KSK 2015.01.21
		case DEV_SPR		:	pQue = m_pSprEventQ;		break;
//-NOUSE		case DEV_JPR		:	pQue = m_pJprEventQ;		break;			// AIREAT 2009.07.14
		case DEV_CDU		:	pQue = m_pCduEventQ;		break;	// 2004.01.16_1.2_2003
		case DEV_PIN		:	pQue = m_pPinEventQ;		break;	// 2004.01.16_1.2_2003
		case DEV_DOR		:								break;
		case DEV_SIU		:								break;
		case DEV_SCR		:	pQue = m_pScrEventQ;		break;
		case DEV_BCR		:	pQue = m_pBcrEventQ;		break;	// [#GLDV-3005] US Kook 2021.12.30 Side Car
		case DEV_BNA		:	pQue = m_pBnaEventQ;		break;	// [#GLDV-3005] US Kook 2021.12.30 Side Car
		default				:								break;
	}

	return pQue;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::EvtQInit(LPCTSTR	szOwner)
// DESCRIPT : 큐를 초기화한다.
// 매개변수 : 오너 문자열값
// 반 환 값 : TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQInit(int nDevID, LPCSTR	szOwner)
{
	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(nDevID);

	if (pEventQ == NULL)
	{
		return FALSE;
	}

	// 초기화를 수행한다
	if (strlen((LPCSTR)szOwner) == 0)	pEventQ->Init();
	else						pEventQ->Init(szOwner);

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::EvtQReset()
// DESCRIPT : 이벤트큐를 초기화한다.(데이타 삭제처리만 수행)
// 매개변수 : 없음
// 반 환 값 : TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQReset(int nDevID)
{
	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(nDevID);

	if (pEventQ == NULL)
	{
		return FALSE;
	}

	// 버퍼초기화를 수행한다
	pEventQ->ResetQue();

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int	 CDevCtrl::EvtQGetCount()
// DESCRIPT : 이벤트큐의 적재되어 있는 이벤트의 갯수를 조회한다.
// 매개변수 : 없음
// 반 환 값 : 적재된 이벤트의 갯수
// ----------------------------------------------------------------------------
int	 CDevCtrl::EvtQGetCount(int nDevID)
{
	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(nDevID);

	if (pEventQ == NULL)
	{
		return 0;
	}

	// 조회
	return pEventQ->GetEventCount();
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::EvtQIsEmpty()
// DESCRIPT : 이벤트큐가 비어있는 지를 확인한다.
// 매개변수 : 없음
// 반 환 값 : TRUE if EventQ is Empty...
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQIsEmpty(int nDevID)
{
	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(nDevID);

	if (pEventQ == NULL)
	{
		return FALSE;
	}

	// 확인
	return pEventQ->IsEmpty();
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::EvtQPutData(LPCTSTR szType, LPCTSTR szName, LPCTSTR szEvent, LPCTSTR szValue)
// DESCRIPT : 이벤트큐에 이벤트를 적재한다.
// 매개변수 : 
// 반 환 값 : 
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQPutData(char* szType, char* szName, char* szEvent, char* szValue)
{
	// 삽입되는 이벤트데이타를 검사한다.
	if ((strlen(szType) == 0) || (strlen(szName) == 0))		//[CS#005] NH AIREAT 2008.03.14 szType -> szName 변경
	{
		return FALSE;
	}

	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(GetDevIDByDevName(szName));

	if (pEventQ == NULL)
	{
		return FALSE;
	}

	pEventQ->PutData(szType, szName, szEvent, szValue);

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::EvtQGetData(LPTSTR szType,  LPTSTR szName,  LPTSTR szEvent,  LPTSTR szValue)
// DESCRIPT : 이벤트큐에서 이벤트를 가져온다.
// 매개변수 : 
// 반 환 값 : 
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQGetData(char* szType,  char* szName,  char* szEvent,  char* szValue)
{
	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(GetDevIDByDevName(szName));

	if (pEventQ == NULL)
	{
		return FALSE;
	}

	pEventQ->GetData(szType,szName, szEvent, szValue);

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::EvtQCheckDeviceDataExist(LPCTSTR szType,  LPCTSTR szName,  LPCTSTR szEvent,  LPCTSTR szValue)
// DESCRIPT : 적재되어 있는 이벤트중에 원하는 이벤트가 있는지 확인한다.
// 매개변수 : 
// 반 환 값 : 있을 경우 TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQCheckDeviceDataExist(char* szType,  char* szName,  char* szEvent,  char* szValue)
{
	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(GetDevIDByDevName(szName));

	if (pEventQ == NULL)
	{
		return FALSE;
	}

	return pEventQ->CheckDeviceDataExist(szType,szName,szEvent,szValue);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BOOL EvtQPumpingEventToError(int nDevID)
// DESCRIPT  : 장치디바이스의 이벤트큐를 에러이벤트를 만날때까지 펌핑한다(개별처리)
// PARAMETER : 장치아디
// RETURN    : TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQPumpingEventToError(int nDevID)
{
	CEventQue*	pEventQ = NULL;
	pEventQ = GetEventQByDevID(nDevID);

	if (pEventQ == NULL)
	{
		return FALSE;
	}

	return pEventQ->PumpingEventToError();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BOOL EvtQPumpingCheck(int nDevID, int nCheckStatus)
// DESCRIPT  : 장치디바이스의 이벤트큐를 Pumping처리한다.(일괄처리)
// PARAMETER : 장치ID, 확인하고자 하는 상태값
// RETURN    : TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::EvtQPumpingCheck(int nDevID, int nCheckStatus /* = 0 */)
{
	BOOL	bReturn = TRUE;

	CEventQue*	pEventQ = NULL;

	// 1. 화면관련처리 : 화면에서의 입력데이타를 담고있는 큐를 클리어한다.
	if (nDevID & DEV_SCR)
	{
		EvtQReset(DEV_SCR);
	}

	// 2. 카드부관련처리 : 장애이벤트(DeviceError/FatalError)를 검사
	if (nDevID & DEV_MCU)		
		EvtQPumpingEventToError(DEV_MCU);

	// [#2325] NH KSK 2015.01.28
	// 3. RFID 관련처리 : 장애이벤트(DeviceError/FatalError)를 검사
	if (nDevID & DEV_RFID)		
		EvtQPumpingEventToError(DEV_RFID);
	// end of [#2325]

	// 4. 명세표부관련처리 : 장애이벤트(DeviceError/FatalError)를 검사
	if (nDevID & DEV_SPR)
		EvtQPumpingEventToError(DEV_SPR);

	// 5. 저널부관련처리 : 장애이벤트(DeviceError/FatalError)를 검사
	if (nDevID & DEV_JPR)
		EvtQPumpingEventToError(DEV_JPR);

	// 6. 현금부관련처리 : 장애이벤트(DeviceError/FatalError)를 검사
	//                     2004.01.16_1.2_2003
	if (nDevID & DEV_CDU)
		EvtQPumpingEventToError(DEV_CDU);

	// 7. PINPAD관련처리 : 장애이벤트(DeviceError/FatalError)를 검사
	//                     2004.01.16_1.2_2003
	if (nDevID & DEV_PIN)
		EvtQPumpingEventToError(DEV_PIN);

	// [#GLDV-3005] US Kook 2021.12.30 Side Car
	if (nDevID & DEV_BCR)
		EvtQPumpingEventToError(DEV_BCR);
	if (nDevID & DEV_BNA)
		EvtQPumpingEventToError(DEV_BNA);
	// end of [#GLDV-3005]

	return bReturn;
}