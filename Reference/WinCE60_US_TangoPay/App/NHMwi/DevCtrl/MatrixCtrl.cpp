// MatrixCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "..\nhmwi.h"
#include "DevCtrl.h"

#include ".\Common\CommDef.h"
#include ".\Common\ConstDef.h"

#include "..\Matrix\Matrix.h"
#include "..\Matrix\McuMatrix.h"
#include "..\Matrix\SprMatrix.h"
#include "..\Matrix\JprMatrix.h"
#include "..\Matrix\CduMatrix.h"								// 2004.01.15_1.2_2003 : CDU지원
#include "..\Matrix\PinMatrix.h"
#include "..\Matrix\RFIDMatrix.h"			// [#2325] NH KSK 2015.01.28
#include "..\Matrix\BcrMatrix.h"			// [#GLDV-3005]
#include "..\Matrix\BnaMatrix.h"			// [#GLDV-3005]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : 
// DESCRIPT : 
// ----------------------------------------------------------------------------
CMatrix* CDevCtrl::GetMatrix(int nDevID)
{
	CMatrix*	pMatrix = NULL;

	// 장치별 구조체 포인터 설정
	switch(nDevID)
	{
		case DEV_MCU		:	pMatrix = m_pMcuMatrix;		break;
		case DEV_RFID		:	pMatrix = m_pRFIDMatrix;	break;	// [#2325] NH KSK 2015.01.28
		case DEV_SPR		:	pMatrix = m_pSprMatrix;		break;
//-NOUSE		case DEV_JPR		:	pMatrix = m_pJprMatrix;	break;			// AIREAT 2009.07.14
		case DEV_CDU		:	pMatrix = m_pCduMatrix;		break;	// 2004.01.15_1.2_2003 : CDU추가
		case DEV_PIN		:	pMatrix = m_pPinMatrix;		break;
		case DEV_DOR		:								break;
		case DEV_SIU		:								break;
		case DEV_BCR		:	pMatrix = m_pBcrMatrix;		break;	// [#GLDV-3005] US Kook 2021.10.28 Support Side Car
		case DEV_BNA		:	pMatrix = m_pBnaMatrix;		break;	// [#GLDV-3005] US Kook 2021.10.28 Support Side Car
		default				:								break;
	}

	// 메트릭스 포인터 확인
	if (pMatrix == NULL)
	{
		return NULL;
	}

	return pMatrix;
}
// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::GetDevIDByDevName(LPCTSTR szDeviceName)
// DESCRIPT : 디바이스명으로 디바이스의 ID를 구한다
// 매개변수 : 장치명
// 반 환 값 : 장치아디
// ----------------------------------------------------------------------------
int CDevCtrl::GetDevIDByDevName(char* szDeviceName)
{
	int nDevID = -1;

	if		(strncmp(szDeviceName, DEVNM_MCU,		strlen(szDeviceName)) == 0)		nDevID = DEV_MCU;
	else if	(strncmp(szDeviceName, DEVNM_RFID,		strlen(szDeviceName)) == 0)		nDevID = DEV_RFID;	// [#2325] NH KSK 2015.01.28
	else if (strncmp(szDeviceName, DEVNM_SPR,		strlen(szDeviceName)) == 0)		nDevID = DEV_SPR;
	else if (strncmp(szDeviceName, DEVNM_JPR,		strlen(szDeviceName)) == 0)		nDevID = DEV_JPR;
	else if (strncmp(szDeviceName, DEVNM_CDU,		strlen(szDeviceName)) == 0)		nDevID = DEV_CDU;	// 2004.01.15_1.2_2003 : CDU추가
	else if (strncmp(szDeviceName, DEVNM_PIN,		strlen(szDeviceName)) == 0)		nDevID = DEV_PIN;
	else if (strncmp(szDeviceName, DEVNM_DOR,		strlen(szDeviceName)) == 0)		nDevID = DEV_DOR;
	else if (strncmp(szDeviceName, DEVNM_SIU,		strlen(szDeviceName)) == 0)		nDevID = DEV_SIU;
	else if (strncmp(szDeviceName, DEVNM_SCR,		strlen(szDeviceName)) == 0)		nDevID = DEV_SCR;
	else if (strncmp(szDeviceName, DEVNM_SNS,		strlen(szDeviceName)) == 0)		nDevID = DEV_SNS;
	else if (strncmp(szDeviceName, DEVNM_BCR,		strlen(szDeviceName)) == 0)		nDevID = DEV_BCR;	// [#GLDV-3005]
	else if (strncmp(szDeviceName, DEVNM_BNA,		strlen(szDeviceName)) == 0)		nDevID = DEV_BNA;	// [#GLDV-3005]

	return nDevID;
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString CDevCtrl::GetDevNameByDevID(INT nDevID)
// DESCRIPT : 장치이디를 이용하여 장치명을 조회한다.
// 매개변수 : 장치아디
// 반 환 값 : 장치명
// ----------------------------------------------------------------------------
CString CDevCtrl::GetDevNameByDevID(INT nDevID)
{
	//CString	strName("");

	//char szName[128] = {};

	//switch(nDevID)
	//{
	//	case DEV_MCU		:	strcpy(szName, DEVNM_MCU);		break;
	//	case DEV_RFID		:	strcpy(szName, DEVNM_RFID);		break;	// [#2325] NH KSK 2015.01.28
	//	case DEV_SPR		:	strcpy(szName, DEVNM_SPR);		break;
	//	case DEV_JPR		:	strcpy(szName, DEVNM_JPR);		break;
	//	case DEV_CDU		:	strcpy(szName, DEVNM_CDU);		break;	// 2004.01.15_1.2_2003 : CDU추가
	//	case DEV_PIN		:	strcpy(szName, DEVNM_PIN);		break;
	//	case DEV_DOR		:	strcpy(szName, DEVNM_DOR);		break;
	//	case DEV_SIU		:	strcpy(szName, DEVNM_SIU);		break;
	//	case DEV_SCR		:	strcpy(szName, DEVNM_SCR);		break;
	//	case DEV_SNS		:	strcpy(szName, DEVNM_SNS);		break;
	//	case DEV_BCR		:	strcpy(szName, DEVNM_BCR);		break;	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	//	case DEV_BNA		:	strcpy(szName, DEVNM_BNA);		break;	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	//	default				:	break;
	//}

	//strName.Format(_T("%S"), szName);
	//return strName;
	return CString(GET_DEVNAME_BY_ID(nDevID));
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::GetMethodIndexByName(INT nDevID, LPCTSTR szMethodName)
// DESCRIPT : 장치별로 메소드의 명으로 메소드의 인덱스를 조회한다.
// 매개변수 : 장치아디, 메소드명
// 반 환 값 : 메소드인덱스
// ----------------------------------------------------------------------------
int CDevCtrl::GetMethodIndexByName(INT nDevID, LPCTSTR szMethodName)
{
	LPMETHODLIST	pList = NULL;
	int			nMethodCount = 0;

	// 장치별 구조체 포인터 설정
	switch(nDevID)
	{
		case DEV_MCU		:	
			pList = McuMethod;
			nMethodCount = McuMethodCount;
			break;
		case DEV_RFID		:				// [#2325] NH KSK 2015.01.28
			pList = RFIDMethod;
			nMethodCount = RFIDMethodCount;
			break;							// end of [#2325]
		case DEV_SPR		:	
			pList = SprMethod;		
			nMethodCount = SprMethodCount;
			break;
		case DEV_JPR		:	
			pList = JprMethod;
			nMethodCount = SprMethodCount;
			break;
		case DEV_CDU		:									// 2004.01.15_1.2_2003 : CDU추가
			pList = CduMethod;
			nMethodCount = CduMethodCount;
			break;
		case DEV_PIN		:
			pList = PinMethod;
			nMethodCount = PinMethodCount;
			break;
		case DEV_DOR		:	break;
		case DEV_SIU		:	break;
		case DEV_CMR		:	break;
			// [#GLDV-3005] US Kook 2021.10.28 Support Side Car
		case DEV_BCR		:
			pList = BcrMethod;
			nMethodCount = BcrMethodCount;
			break;
		case DEV_BNA		:
			pList = BnaMethod;
			nMethodCount = BnaMethodCount;
			break;
			// end of [#GLDV-3005]
		default				:	break;
	}

	// 메트릭스 포인터 확인
	if (pList == NULL)
	{
		return -1;
	}
	
	// 메소드를 비교한다
	CString strMethod;
	for (int i =0; i<nMethodCount;i++)
	{
		// if (strncmp(pList[i].szMethodName, szMethodName, strlen(pList[i].szMethodName)) == 0)
		strMethod.Format(_T("%S"), pList[i].szMethodName);			// 20030913_1.0_1072 : strncmp debug
		if (strMethod.CompareNoCase(szMethodName) == 0)
			return i;
	}

	return -1;
	
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString CDevCtrl::GetMethodNameByIndex(INT nDevID, int nMethodIndex)
// DESCRIPT : 메소드인덱스를 이용하여 메소드명을 조회한다.
// 매개변수 : 장치아디, 메소드인덱스
// 반 환 값 : 메소드명
// ----------------------------------------------------------------------------
CString CDevCtrl::GetMethodNameByIndex(INT nDevID, int nMethodIndex)
{
	LPMETHODLIST	pList = NULL;
	int			nMethodCount = 0;

	// 장치별 구조체 포인터 설정
	switch(nDevID)
	{
		case DEV_MCU		:	
			pList = McuMethod;
			nMethodCount = McuMethodCount;
			break;
		case DEV_RFID		:				// [#2325] NH KSK 2015.01.28	
			pList = RFIDMethod;
			nMethodCount = RFIDMethodCount;
			break;							// end of [#2325]
		case DEV_SPR		:	
			pList = SprMethod;		
			nMethodCount = SprMethodCount;
			break;
		case DEV_JPR		:	
			pList = JprMethod;
			nMethodCount = SprMethodCount;
			break;
		case DEV_CDU		:									// 2004.01.15_1.2_2003 : CDU추가
			pList = CduMethod;
			nMethodCount = CduMethodCount;
			break;
		case DEV_PIN		:									// 2004.01.15_1.2_2003 : CDU추가
			pList = PinMethod;
			nMethodCount = PinMethodCount;
			break;
		case DEV_DOR		:	break;
		case DEV_SIU		:	break;
		case DEV_CMR		:	break;
			// [#GLDV-3005] US Kook 2021.10.28 Support Side Car
		case DEV_BCR		:
			pList = BcrMethod;
			nMethodCount = BcrMethodCount;
			break;
		case DEV_BNA		:
			pList = BnaMethod;
			nMethodCount = BnaMethodCount;
			break;
			// end of [#GLDV-3005]
		default				:	break;
	}

	// 메트릭스 포인터 확인
	if (pList == NULL)
	{
		return L"";
	}

	// 메소드의 갯수와 인덱스를 비교한다.
	if (nMethodIndex < 0 || nMethodIndex >= nMethodCount)
	{
		return L"";
	}

	// 메소드를 비교한다
	CString strReturn("");

	strReturn = pList[nMethodIndex].szMethodName;
	return strReturn;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::GetEventIndexByName(INT nDevID, LPCTSTR szEventName)
// DESCRIPT : 이벤트명으로 이벤트의 인덱스를 조회한다.
// 매개변수 : 장치아디, 이벤트명
// 반 환 값 : 이벤트인덱스
// ----------------------------------------------------------------------------
int CDevCtrl::GetEventIndexByName(INT nDevID, char* szEventName)
{
	LPITEMLIST	pList = NULL;
	int			nEventCount = 0;

	// 장치별 구조체 포인터 설정
	switch(nDevID)
	{
		case DEV_MCU		:	
			pList = McuEvent;
			nEventCount = McuEventCount;
			break;
		case DEV_RFID		:				// [#2325] NH KSK 2015.01.28	
			pList = RFIDEvent;
			nEventCount = RFIDEventCount;
			break;							// end of [#2325]
		case DEV_SPR		:	
			pList = SprEvent;		
			nEventCount = SprEventCount;
			break;
		case DEV_JPR		:	
			pList = JprEvent;
			nEventCount = SprEventCount;
			break;
		case DEV_CDU		:									// 2004.01.15_1.2_2003 : CDU추가
			pList = CduEvent;
			nEventCount = CduEventCount;
			break;
		case DEV_PIN		:									// 2004.01.15_1.2_2003 : CDU추가
			pList = PinEvent;
			nEventCount = PinEventCount;
			break;
		case DEV_DOR		:	break;	
		case DEV_SIU		:	break;	
		case DEV_CMR		:	break;	
			// [#GLDV-3005] US Kook 2021.10.28 Support Side Car
		case DEV_BCR		:
			pList = BcrEvent;
			nEventCount = BcrEventCount;
			break;
		case DEV_BNA		:
			pList = BnaEvent;
			nEventCount = BnaEventCount;
			break;
			// end of [#GLDV-3005]
		default				:	break;
	}

	// 메트릭스 포인터 확인
	if (pList == NULL)
	{
		return -1;
	}
	
	// 이벤트를 비교한다
	CString strEvent;
	CString strEventName = CString(szEventName);

	for (int i =0; i<nEventCount;i++)
	{
		//if (strncmp(pList[i].szItem, szEventName, strlen(pList[i].szItem)) == 0)
		//	return i;
		// 2003.07.02 대소문자 구별이 안되어 동기화 처리에 문제가 생길수 있음..

		strEvent.Format(_T("%S"), pList[i].szItem);
		
		if (strEvent.CompareNoCase(strEventName) == 0)
		{
			return i;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString CDevCtrl::GetEventNameByIndex(INT nDevID, int nEventIndex)
// DESCRIPT : 이벤트인덱스로 이벤트명을 조회한다.
// 매개변수 : 장치아디, 이벤트인덱스
// 반 환 값 : 이벤트명
// ----------------------------------------------------------------------------
CString CDevCtrl::GetEventNameByIndex(INT nDevID, int nEventIndex)
{
	LPITEMLIST	pList = NULL;
	int			nEventCount = 0;

	// 장치별 구조체 포인터 설정
	switch(nDevID)
	{
		case DEV_MCU		:	
			pList = McuEvent;
			nEventCount = McuEventCount;
			break;
		case DEV_RFID		:				// [#2325] NH KSK 2015.01.28
			pList = RFIDEvent;
			nEventCount = RFIDEventCount;
			break;							// end of [#2325]
		case DEV_SPR		:	
			pList = SprEvent;		
			nEventCount = SprEventCount;
			break;
		case DEV_JPR		:	
			pList = JprEvent;
			nEventCount = SprEventCount;
			break;
		case DEV_CDU		:									// 2004.01.15_1.2_2003 : CDU추가
			pList = CduEvent;
			nEventCount = CduEventCount;
			break;
		case DEV_PIN		:									// 2004.01.15_1.2_2003 : CDU추가
			pList = PinEvent;
			nEventCount = PinEventCount;
			break;
		case DEV_DOR		:	break;
		case DEV_SIU		:	break;
		case DEV_CMR		:	break;
			// [#GLDV-3005] US Kook 2021.10.28 Support Side Car
		case DEV_BCR		:
			pList = BcrEvent;
			nEventCount = BcrEventCount;
			break;
		case DEV_BNA		:
			pList = BnaEvent;
			nEventCount = BnaEventCount;
			break;
			// end of [#GLDV-3005]
		default				:	break;
	}

	// 메트릭스 포인터 확인
	if (pList == NULL)
	{
		return L"";
	}

	// 이벤트의 갯수와 인덱스를 비교한다.
	if (nEventIndex < 0 || nEventIndex >= nEventCount)
	{
		return L"";
	}

	// 메소드를 비교한다
	CString strReturn("");

	strReturn = pList[nEventIndex].szItem;
	return strReturn;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int GetCurrMethodIndex(INT nDevID)
// DESCRIPT  : 직전구동 메소드의 인덱스를 구한다.
// PARAMETER : 장치 아디
// RETURN    : 메소드의 인덱스
// ----------------------------------------------------------------------------
int CDevCtrl::GetCurrMethodIndex(INT nDevID)
{
	CMatrix*	pMatrix = GetMatrix(nDevID);

	// 메트릭스 포인터 확인
	if (pMatrix == NULL)
	{
		return -1;
	}

	int nResult = pMatrix->GetCurrMethodIndex();
	return nResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::SetDeviceMatrix(INT nDeviceID)
// DESCRIPT : 장치별 메소드/이벤트의 메트릭스 정보를 설정한다.
// 매개변수 : 설정할 장치ID
// 반 환 값 : True if Success
// RAMARK	: FUNCTION CUSTOMIZING NEED!!!!(I WILL...)
// ----------------------------------------------------------------------------
BOOL CDevCtrl::SetDeviceMatrix(INT nDeviceID)
{
	BOOL	retVal = TRUE;

	// 정보를 설정할 장치가 없는 경우에는 아무런 처리를 하지 않는다.
	if (nDeviceID == DEV_NONE)	return FALSE;

	// ------------------------------------------------------------------------
	// 카드부 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_MCU)									
	{
		if (m_pMcuMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 기본정보를 설정한다.
		m_pMcuMatrix->SetDeviceInfo(DEV_MCU, DEVNM_MCU);		// 카드부 기본정보 설정
		m_pMcuMatrix->SetMethodCount(McuMethodCount);			// 카드부 Kal Method 갯수
		m_pMcuMatrix->SetEventCount(McuEventCount);				// 카드부 Kal Event 갯수

		// 카드부의 메트릭스 정보를 설정한다.
		retVal = m_pMcuMatrix->SetMasterMatrix(*McuMatrix);	// MasterMatrix설정

		if (!retVal)	return FALSE;
	}

	// [#2325] NH KSK 2015.01.28
	// ------------------------------------------------------------------------
	// RFID 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_RFID)									
	{
		if (m_pRFIDMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 기본정보를 설정한다.
		m_pRFIDMatrix->SetDeviceInfo(DEV_RFID, DEVNM_RFID);		// RFID 기본정보 설정
		m_pRFIDMatrix->SetMethodCount(RFIDMethodCount);			// RFID Kal Method 갯수
		m_pRFIDMatrix->SetEventCount(RFIDEventCount);			// RFID Kal Event 갯수

		// 카드부의 메트릭스 정보를 설정한다.
		retVal = m_pRFIDMatrix->SetMasterMatrix(*RFIDMatrix);	// Master Matrix설정

		if (!retVal)	return FALSE;
	}
	// end of [#2325]
	
	// ------------------------------------------------------------------------
	// 명세표부 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_SPR)									
	{
		if (m_pSprMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 기본정보를 설정한다.
		m_pSprMatrix->SetDeviceInfo(DEV_SPR, DEVNM_SPR);		// 명세표부 기본정보 설정
		m_pSprMatrix->SetMethodCount(SprMethodCount);			// 명세표부 Kal Method 갯수
		m_pSprMatrix->SetEventCount(SprEventCount);				// 명세표부 Kal Event 갯수

		// 명세표부의 메트릭스 정보를 설정한다.
		retVal = m_pSprMatrix->SetMasterMatrix(*SprMatrix);	// MasterMatrix설정

		if (!retVal)	return FALSE;
	}
	
#if 0	//-NOUSE			// AIREAT 2009.07.14
	// ------------------------------------------------------------------------
	// 저널부 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_JPR)									
	{
		if (m_pJprMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 기본정보를 설정한다.
		m_pJprMatrix->SetDeviceInfo(DEV_JPR, DEVNM_JPR);		// 저널부 기본정보 설정
		m_pJprMatrix->SetMethodCount(JprMethodCount);			// 저널부 Kal Method 갯수
		m_pJprMatrix->SetEventCount(JprEventCount);				// 저널부 Kal Event 갯수

		// 메트릭스 정보를 설정한다.
		retVal = m_pJprMatrix->SetMasterMatrix(*JprMatrix);	// MasterMatrix설정
//		if (retVal)
//		{
//		}
//		else
//		{
//			return FALSE;
//		}
		if (!retVal)	return FALSE;
	}
#endif	//-NOUSE			// AIREAT 2009.07.14
	
	// ------------------------------------------------------------------------
	// CDU 메트릭스 처리 : 2004.01.15_1.2_2003
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_CDU)									
	{
		if (m_pCduMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 기본정보를 설정한다.
		m_pCduMatrix->SetDeviceInfo(DEV_CDU, DEVNM_CDU);		// 기본정보 설정
		m_pCduMatrix->SetMethodCount(CduMethodCount);			// Kal Method 갯수
		m_pCduMatrix->SetEventCount(CduEventCount);				// Kal Event 갯수

		// 메트릭스 정보를 설정한다.
		retVal = m_pCduMatrix->SetMasterMatrix(*CduMatrix);		// MasterMatrix설정

		if (!retVal)	return FALSE;
	}

	// ------------------------------------------------------------------------
	// PINPAD 메트릭스 처리 : 2004.01.15_1.2_2003
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_PIN)									
	{
		if (m_pPinMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 기본정보를 설정한다.
		m_pPinMatrix->SetDeviceInfo(DEV_PIN, DEVNM_PIN);		// 기본정보 설정
		m_pPinMatrix->SetMethodCount(PinMethodCount);			// Kal Method 갯수
		m_pPinMatrix->SetEventCount(PinEventCount);				// Kal Event 갯수

		// 메트릭스 정보를 설정한다.
		retVal = m_pPinMatrix->SetMasterMatrix(*PinMatrix);		// MasterMatrix설정

		if (!retVal)	return FALSE;		
	}

	// [#GLDV-3005] US Kook 2021.10.28
	// ------------------------------------------------------------------------
	// BARCODE READER
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_BCR)									
	{
		if (m_pBcrMatrix == NULL)
			return FALSE;

		// 기본정보를 설정한다.
		m_pBcrMatrix->SetDeviceInfo(DEV_BCR, DEVNM_BCR);
		m_pBcrMatrix->SetMethodCount(BcrMethodCount);
		m_pBcrMatrix->SetEventCount(BcrEventCount);

		// 메트릭스 정보를 설정한다.
		retVal = m_pBcrMatrix->SetMasterMatrix(*BcrMatrix);

		if (!retVal)	return FALSE;		
	}
	// ------------------------------------------------------------------------
	// BILL ACCEPTOR
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_BNA)									
	{
		if (m_pBnaMatrix == NULL)
			return FALSE;

		// 기본정보를 설정한다.
		m_pBnaMatrix->SetDeviceInfo(DEV_BNA, DEVNM_BNA);
		m_pBnaMatrix->SetMethodCount(BnaMethodCount);
		m_pBnaMatrix->SetEventCount(BnaEventCount);

		// 메트릭스 정보를 설정한다.
		retVal = m_pBnaMatrix->SetMasterMatrix(*BnaMatrix);

		if (!retVal)	return FALSE;		
	}
	// end of [#GLDV-3005]

	return	TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::SetEventMatrix(INT nDeviceID, LPCTSTR szMethodName)
// DESCRIPT : 이벤트메트릭스를 설정한다.
// 매개변수 : 디바이스아뒤, 메소드명
// 반 환 값 : 정상처리시 TRUE
// ----------------------------------------------------------------------------
BOOL CDevCtrl::SetEventMatrix(INT nDeviceID, char* szMethodName)
{
	int nMethodIndex = -1;
	int mIndex = 0;

	CString		strMethod;										// 20030913_1.0_1072 : strncmp debug
	CString		strMethodSPUse;									// 20030913_1.0_1072 : strncmp debug

	CString strMethodName = CString(szMethodName);

	// 정보를 설정할 장치가 없는 경우에는 아무런 처리를 하지 않는다.
	if (nDeviceID == DEV_NONE)	return FALSE;

	// ------------------------------------------------------------------------
	// 카드부 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_MCU)									
	{
		if (m_pMcuMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 메소드명으로 메소드인덱스를 추출한다.
		for (mIndex=0;mIndex<McuMethodCount;mIndex++)
		{
			// if (strncmp(szMethodName, McuMethod[mIndex].szMethodName, strlen(szMethodName)) == 0)
			strMethod.Format(_T("%S"), McuMethod[mIndex].szMethodName);		// 20030913_1.0_1072 : strncmp debug
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// EventQ를 Pumping 처리한다.
		// USE Field가 "NO"인것은 대부분이 연속처리이므로 이때 버퍼를 비우면
		// 연속이벤트를 확인처리 하지 못하는 경우가 발생된다. 따라서 이때는 큐를 비우지 않는다
		strMethodSPUse.Format(_T("%S"), McuMethod[nMethodIndex].szMethodSPUse);	// 20030913_1.0_1072 : strncmp debug
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_MCU);

		// 이벤트 메트릭스를 설정한다
		m_pMcuMatrix->SetMethodEventMatrix(nMethodIndex);
	}

	// [#2325] NH KSK 2015.01.28
	// ------------------------------------------------------------------------
	// RFID 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_RFID)									
	{
		if (m_pRFIDMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 메소드명으로 메소드인덱스를 추출한다.
		for (mIndex=0;mIndex<RFIDMethodCount;mIndex++)
		{
			strMethod.Format(_T("%S"), RFIDMethod[mIndex].szMethodName);
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// EventQ를 Pumping 처리한다.
		// USE Field가 "NO"인것은 대부분이 연속처리이므로 이때 버퍼를 비우면
		// 연속이벤트를 확인처리 하지 못하는 경우가 발생된다. 따라서 이때는 큐를 비우지 않는다
		strMethodSPUse.Format(_T("%S"), RFIDMethod[nMethodIndex].szMethodSPUse);	// 20030913_1.0_1072 : strncmp debug
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_RFID);

		// 이벤트 메트릭스를 설정한다
		m_pRFIDMatrix->SetMethodEventMatrix(nMethodIndex);
	}
	// end of [#2325]
	
	// ------------------------------------------------------------------------
	// 명세표부 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_SPR)									
	{
		if (m_pSprMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 메소드명으로 메소드인덱스를 추출한다.
		for (mIndex=0;mIndex<SprMethodCount;mIndex++)
		{
			// if (strncmp(szMethodName, SprMethod[mIndex].szMethodName, strlen(szMethodName)) == 0)
			strMethod.Format(_T("%S"), SprMethod[mIndex].szMethodName);		// 20030913_1.0_1072 : strncmp debug
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// EventQ를 Pumping 처리한다.
		// USE Field가 "NO"인것은 대부분이 연속처리이므로 이때 버퍼를 비우면
		// 연속이벤트를 확인처리 하지 못하는 경우가 발생된다. 따라서 이때는 큐를 비우지 않는다
		// if (strncmp("YES", SprMethod[nMethodIndex].szMethodSPUse, 3) == 0)
		strMethodSPUse.Format(_T("%S"), SprMethod[nMethodIndex].szMethodSPUse);	// 20030913_1.0_1072 : strncmp debug
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_SPR);

		// 이벤트 메트릭스를 설정한다
		m_pSprMatrix->SetMethodEventMatrix(nMethodIndex);
	}
	
#if 0	//-NOUSE			// AIREAT 2009.07.14
	// ------------------------------------------------------------------------
	// 저널부 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_JPR)									
	{
		if (m_pJprMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 메소드명으로 메소드인덱스를 추출한다.
		for (mIndex=0;mIndex<JprMethodCount;mIndex++)
		{
			// if (strncmp(szMethodName, JprMethod[mIndex].szMethodName, strlen(szMethodName)) == 0)
			strMethod.Format(_T("%S"), JprMethod[mIndex].szMethodName);		// 20030913_1.0_1072 : strncmp debug
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// EventQ를 Pumping 처리한다.
		// USE Field가 "NO"인것은 대부분이 연속처리이므로 이때 버퍼를 비우면
		// 연속이벤트를 확인처리 하지 못하는 경우가 발생된다. 따라서 이때는 큐를 비우지 않는다
		// if (strncmp("YES", JprMethod[nMethodIndex].szMethodSPUse, 3) == 0)
		strMethodSPUse.Format(_T("%S"), JprMethod[nMethodIndex].szMethodSPUse);	// 20030913_1.0_1072 : strncmp debug
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_JPR);

		// 이벤트 메트릭스를 설정한다
		m_pJprMatrix->SetMethodEventMatrix(nMethodIndex);
	}
#endif	//-NOUSE			// AIREAT 2009.07.14
	
	// ------------------------------------------------------------------------
	// CDU 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_CDU)
	{
		if (m_pCduMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 메소드명으로 메소드인덱스를 추출한다.
		for (mIndex=0;mIndex<CduMethodCount;mIndex++)
		{
			strMethod.Format(_T("%S"), CduMethod[mIndex].szMethodName);		// 20030913_1.0_1072 : strncmp debug
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// EventQ를 Pumping 처리한다.
		// USE Field가 "NO"인것은 대부분이 연속처리이므로 이때 버퍼를 비우면
		// 연속이벤트를 확인처리 하지 못하는 경우가 발생된다. 따라서 이때는 큐를 비우지 않는다
		strMethodSPUse.Format(_T("%S"), CduMethod[nMethodIndex].szMethodSPUse);	// 20030913_1.0_1072 : strncmp debug
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_CDU);

		// 이벤트 메트릭스를 설정한다
		m_pCduMatrix->SetMethodEventMatrix(nMethodIndex);
	}

	// ------------------------------------------------------------------------
	// PINPAD 메트릭스 처리
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_PIN)
	{
		if (m_pPinMatrix == NULL)								// Object Pointer가 없는 경우
		{
			return FALSE;
		}

		// 메소드명으로 메소드인덱스를 추출한다.
		for (mIndex=0;mIndex<PinMethodCount;mIndex++)
		{
			strMethod.Format(_T("%S"), PinMethod[mIndex].szMethodName);		// 20030913_1.0_1072 : strncmp debug
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)
		{
			return FALSE;
		}

		// EventQ를 Pumping 처리한다.
		// USE Field가 "NO"인것은 대부분이 연속처리이므로 이때 버퍼를 비우면
		// 연속이벤트를 확인처리 하지 못하는 경우가 발생된다. 따라서 이때는 큐를 비우지 않는다
		strMethodSPUse.Format(_T("%S"), PinMethod[nMethodIndex].szMethodSPUse);	// 20030913_1.0_1072 : strncmp debug
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_PIN);

		// 이벤트 메트릭스를 설정한다
		m_pPinMatrix->SetMethodEventMatrix(nMethodIndex);
	}

	// [#GLDV-3005] US Kook 2021.10.28
	// ------------------------------------------------------------------------
	// BARCODE READER
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_BCR)
	{
		if (m_pBcrMatrix == NULL)
			return FALSE;

		for (mIndex=0;mIndex<BcrMethodCount;mIndex++)
		{
			strMethod.Format(_T("%S"), BcrMethod[mIndex].szMethodName);
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)
			return FALSE;

		strMethodSPUse.Format(_T("%S"), BcrMethod[nMethodIndex].szMethodSPUse);
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_BCR);

		m_pBcrMatrix->SetMethodEventMatrix(nMethodIndex);
	}
	// ------------------------------------------------------------------------
	// BILL ACCEPTOR
	// ------------------------------------------------------------------------
	if (nDeviceID & DEV_BNA)
	{
		if (m_pBnaMatrix == NULL)
			return FALSE;

		for (mIndex=0;mIndex<BnaMethodCount;mIndex++)
		{
			strMethod.Format(_T("%S"), BnaMethod[mIndex].szMethodName);
			if (strMethod.CompareNoCase(strMethodName) == 0)
			{
				nMethodIndex = mIndex;
				break;
			}
		}

		if (nMethodIndex == -1)
			return FALSE;

		strMethodSPUse.Format(_T("%S"), BnaMethod[nMethodIndex].szMethodSPUse);
		if (strMethodSPUse.CompareNoCase(S_YES) == 0)
			EvtQPumpingCheck(DEV_BNA);

		m_pBnaMatrix->SetMethodEventMatrix(nMethodIndex);
	}
	// end of [#GLDV-3005]

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::SetEventMatrixValue(LPCSTR szDeviceName, LPCTSTR szEventName)
// DESCRIPT : 장치로부터 수신된 이벤트를 이벤트 메트릭스에 저장
// 매개변수 : 
// 반 환 값 : 
// ----------------------------------------------------------------------------
BOOL CDevCtrl::SetEventMatrixValue(char* szDeviceName, char* szEventName)
{
	int nDevID = GetDevIDByDevName(szDeviceName);
	return SetEventMatrixValue(nDevID, GetEventIndexByName(nDevID, szEventName));
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::SetEventMatrixValue(INT nDeviceID, LPCSTR szEventName)
// DESCRIPT : 장치로부터 수신된 이벤트를 이벤트 메트릭스에 저장
// 매개변수 : 
// 반 환 값 : 
// ----------------------------------------------------------------------------
BOOL CDevCtrl::SetEventMatrixValue(INT nDeviceID, INT nEventIndex)
{
	LPITEMLIST	pList = NULL;
	CMatrix*	pMatrix = GetMatrix(nDeviceID);

	// 메트릭스 포인터 확인
	if (pMatrix == NULL)
	{
		return FALSE;
	}

	// 설정처리
	if (nEventIndex >= 0)
		return pMatrix->SetEventMatrixValue(nEventIndex);
	else return FALSE;
}
