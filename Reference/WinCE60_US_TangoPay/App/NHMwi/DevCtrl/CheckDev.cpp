// CheckDev.cpp : implementation file
//

#include "stdafx.h"
#include "DevCtrl.h"
#include "..\nhmwi.h"
#include "..\nhmwiCtl.h"

#include ".\Common\CommDef.h"
#include ".\Common\ConstDef.h"
#include "..\Matrix\Matrix.h"

//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4819) // [#2032] NH KJW 2011.03.15 한글주석으로 생긴문제라 warning disable함.

///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : void CDevCtrl::ProcSetDeviceEvent(LPCTSTR szDeviceName, LPCTSTR szEventName, LPCTSTR szEventValue) 
// DESCRIPT  : 디바이스(혹은SP)에서 발생된 이벤트를 처리한다.(일단 큐에 저장)
// PARAMETER : 
// RETURN    : 없음
// FROM		: SP/MWI
// ----------------------------------------------------------------------------
void CDevCtrl::ProcSetDeviceEvent(char* szDeviceName, char* szEventName, char* szEventValue)
{
	EvtQPutData("MWI_EVENT_DEVICE", szDeviceName,szEventName, szEventValue);
																// 2004.07.05
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_MCU,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_MCU;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_RFID,		strlen((LPCSTR)szDeviceName)) == 0)	// [#2325] NH KSK 2015.01.20
		m_pOwner->m_nEventDevice |= DEV_RFID;														// end of [#2325]
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_SPR,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_SPR;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_JPR,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_JPR;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_CDU,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_CDU;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_PIN,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_PIN;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_DOR,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_DOR;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_SIU,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_SIU;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_SCR,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_SCR;								
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_SNS,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_SNS;
	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_BCR,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_BCR;
	if (strncmp((LPCSTR)szDeviceName, (LPCSTR)DEVNM_BNA,		strlen((LPCSTR)szDeviceName)) == 0)
		m_pOwner->m_nEventDevice |= DEV_BNA;
	// end of [#GLDV-3005]
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::CheckDeviceAction(int nDevID, int nWaitSec)
// DESCRIPT : 디바이스의 동작완료를 검사한다.
// 파라미터 : 장치ID, 대기시간
// 기타사항 : 1안) 이벤트를 큐에서 가져와서 이를 설정한다음 동기상태를 확인
//				=> 이전동기이후 그냥 CheckDeviceAction시 다음 이벤트가 비워지는 현상 가능
//			  2안) 먼저 체크를 하여 이전 동작이 정상이면 큐에서 꺼내지 말고 동기처리(2003.05.17)
// ----------------------------------------------------------------------------
int CDevCtrl::CheckDeviceAction(int nDevID, int nWaitSec)
{
	if (nDevID == DEV_NONE)	
	{
		//NHDEBUG(1, (_T("DEV_NONE: R_NORMAL\n")));
		return R_NORMAL;										// 장치ID가 DEV_NONE이면 정상(R_NORMAL)을 리턴한다.
	}

	DWORD curTickCount = GetTickCount();
	// ------------------------------------------------------------------------
//	const int	MAX_CHECK_DEVICE_CNT = 5;						// 2007.07.19
//	const int	MAX_CHECK_DEVICE_CNT = 6;						// [#2325] NH KSK 2015.01.28
	const int	MAX_CHECK_DEVICE_CNT = 8;						// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	// ------------------------------------------------------------------------

	int		nMethodIndex, nEventIndex, nIndex;
	CString	strEvent;
	int		nReturn = R_DOING;
	int		nRetOfCheck = R_DOING;
	int		nFirstCheck = FALSE;								// 2004.04.16 : Delay Time조정
	nMethodIndex = nEventIndex = nIndex = 0;

	// ------------------------------------------------------------------------
//	nDevID &= (DEV_SPR | DEV_JPR | DEV_MCU | DEV_CDU | DEV_PIN);
//	nDevID &= (DEV_SPR | DEV_JPR | DEV_MCU | DEV_CDU | DEV_PIN | DEV_RFID);							// [#2325] NH KSK 2015.01.20
	nDevID &= (DEV_SPR | DEV_JPR | DEV_MCU | DEV_CDU | DEV_PIN | DEV_RFID | DEV_BCR | DEV_BNA);		// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
																// 2005.03.30
	if (nWaitSec <= 0)
	{
		nWaitSec = 5;											// "-1"등의 값을 이용한 무한Check는 없다.
																// 최소값을 5로 설정함 : 2004.11.05
	}

	m_pOwner->m_nErrorDevice = DEV_NONE;						// 초기화
	m_pOwner->m_nTimeoutDevice = DEV_NONE;						// 초기화

	int		CheckDevice = nDevID;								// 확인할전체장치
	int		nChkID		= DEV_NONE;								// 확인할단위장치

	// ------------------------------------------------------------------------
	ULONG uTime = TimerSet(nWaitSec);							// 장치 Check
	while(true)
	{
		if (CheckDevice == DEV_NONE)		
		{
			//NHDEBUG(1, (_T("There are no remaining devices to be checked.\n")));
			nReturn = R_NORMAL;									// 정상완료로 설정
			break;												// 장치확인 완료시 확인작업 종료함
		}
		
		if ((nReturn == R_ERROR)	|| 
			(nReturn == R_TIMEOUT)	||
			(nReturn == R_TIMEOVER)	)							// 타임아웃이나 에러시 확인작업 종료함
			break;
			
		// UPS Power Check 필요

		switch(nIndex)											// Select Checking DeviceID
		{
		case  0:	nChkID = DEV_SPR;		break;				// 명세표부
		case  1:	nChkID = DEV_JPR;		break;				// 저널부
		case  2:	nChkID = DEV_MCU;		break;				// 카드부
		case  3:	nChkID = DEV_CDU;		break;				// 현금부
		case  4:	nChkID = DEV_PIN;		break;				// PINPAD부
		case  5:	nChkID = DEV_RFID;		break;				// RFID부		[#2325] NH KSK 2015.01.20
		case  6:	nChkID = DEV_BCR;		break;				// [#GLDV-3005]
		case  7:	nChkID = DEV_BNA;		break;				// [#GLDV-3005]
		default:	nChkID = DEV_NONE;		break;
		}
		
		nIndex++;
		if (nIndex >= MAX_CHECK_DEVICE_CNT)						// Checking Loop
			nIndex = 0;

		// 장치 시간내 무응답 확인(97XXX)
		if ((nWaitSec > 0)  && (CheckTimer(uTime)))	
		{
			if (CheckDevice & DEV_SPR)							// 2005.03.30
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_SPR_TIMEOUT");					// [#2133] NH KSK 2012.05.07

				ProcSetDeviceEvent(DEVNM_SPR, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_SPR);
			}
			if (CheckDevice & DEV_JPR)
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_JPR_TIMEOUT");					// [#2133] NH KSK 2012.05.07

				ProcSetDeviceEvent(DEVNM_JPR, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_JPR);
			}
			if (CheckDevice & DEV_MCU)
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_MCU_TIMEOUT");					// [#2133] NH KSK 2012.05.07

				ProcSetDeviceEvent(DEVNM_MCU, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_MCU);
			}
			if (CheckDevice & DEV_CDU)
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_CDU_TIMEOUT");					// [#2133] NH KSK 2012.05.07

				ProcSetDeviceEvent(DEVNM_CDU, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_CDU);
			}
			if (CheckDevice & DEV_PIN)
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_PIN_TIMEOUT");					// [#2133] NH KSK 2012.05.07

				ProcSetDeviceEvent(DEVNM_PIN, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_PIN);
			}
			// [#2325] NH KSK 2015.01.20
			if (CheckDevice & DEV_RFID)
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_RFID_TIMEOUT");

				ProcSetDeviceEvent(DEVNM_RFID, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_RFID);
			}																			
			// end of [#2325]
			// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
			if (CheckDevice & DEV_BCR)
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_BCR_TIMEOUT");

				ProcSetDeviceEvent(DEVNM_BCR, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_BCR);
			}
			if (CheckDevice & DEV_BNA)
			{
				NVDump('F', '1', "99", L"9999999", L"MWI_BNA_TIMEOUT");

				ProcSetDeviceEvent(DEVNM_BNA, "FatalError", "");
				nReturn = CheckDeviceMatrix(DEV_BNA);
			}
			// end of [#GLDV-3005]

			m_pOwner->m_nErrorDevice |= (nDevID & CheckDevice);	// 2003.05.17
			nReturn = R_TIMEOVER;
			break;												// 2005.05.26
		}

		if (CheckDevice & nChkID)
		{
			nReturn = CheckDeviceMatrix(nChkID);
			if (nReturn != R_DOING)
				CheckDevice &= ~nChkID;
			if ((nReturn == R_ERROR) || (nReturn == R_TIMEOUT))
				break;
		}
		else
			continue;											// 2003.11.21 : 장치확인하지 않으면 Delay하지 않음

		// KSK 2014.12.19 Event 감지 속도를 높이기 위해 Delay Time 조정 (Latch / UnLatch 속도개선을 위해
//		if (!nFirstCheck)										// 2004.04.16 : Delay Time조정
//		{
//			Delay_Msg();
//			if (nIndex == 0)									// Loop이후 다시 처음으로 설정시 Flag설정
//				nFirstCheck = TRUE;
//		}
//		else
//		{
//			Delay_Msg(100);										// 최초 Loof가 아닌경우 시간조정
//		}
		Delay_Msg();
		// end of KSK 2014.12.19
	}

	// ------------------------------------------------------------------------
	// MWI장애코드를 설정한다.
	// TIMEOUT : SP에서 발생된 이벤트를 근간으로하며, 미수취장애코드를 설정한다.
	// TIMEOVER : 무응답 혹은 CheckDeviceAction시 지정된 이벤트없음을 근간으로 하며 97XXX시리즈 장애코드를 설정한다.
	// DEVICEERROR : SP에서 장애코드를 설정하며, MWI에서는 별도의 장애코드를 설정하지 않는다.
	// FATALERROR : SP에서 발생한 이벤트를 근간으로 하며, SP에서 미기록시 979XX시리즈 장애코드를 설정한다.
	// 20030729_1.0_1055 : Set ErrorCode
	// ------------------------------------------------------------------------
	char szTemp[64] = {};
	if ((nReturn == R_ERROR)		||							// 2004.01.05 : &&=>|| 변경
		(nReturn == R_TIMEOVER)		)
	{
		CString strMethodIndex("");
		CString strErrorCode("");
		int nErrorDevice = m_pOwner->m_nErrorDevice;
		
		if (nReturn == R_TIMEOVER)								// TIMEOVER時 9791XXX장애 유도
		{
			// KSK 2009.9.9 Codesonar 지적사항 대책
			// JPR에 대한 Matrix Pointer가 NULL이므로 삭제함
//			if (nErrorDevice & DEV_JPR)
//			{
//				strErrorCode = "97911";							// MWI 장애코드 // 20030913_1.0_1072 : Timeover ErrorCode수정
//				strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_JPR)->GetCurrMethodIndex());
//				strErrorCode += strMethodIndex;
//
//				memset(szTemp, NULL, sizeof(szTemp));
//				WideToMulti(szTemp, strErrorCode, sizeof(szTemp));
//
//				WriteMwiErrorCode(DEV_JPR, "JPR", strErrorCode.GetLength(), szTemp);	// 2003.11.11
//			}
			// end of KSK 2009.9.9

			if (nErrorDevice & DEV_SPR)
			{
				if (GetMatrix(DEV_SPR) != NULL)		// [#2022] NH KSK 2011.02.22
				{
					strErrorCode = "97912";							// MWI 장애코드 // 20030913_1.0_1072 : Timeover ErrorCode수정
					strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_SPR)->GetCurrMethodIndex());
					strErrorCode += strMethodIndex;

					memset(szTemp, NULL, sizeof(szTemp));
					WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

					WriteMwiErrorCode(DEV_SPR, "SPR", strErrorCode.GetLength(), szTemp);	// 2003.11.11
				}
			}

			if (nErrorDevice & DEV_MCU)
			{
				if (GetMatrix(DEV_MCU) != NULL)		// [#2022] NH KSK 2011.02.22
				{
					strErrorCode = "97913";							// MWI 장애코드 // 20030913_1.0_1072 : Timeover ErrorCode수정
					strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_MCU)->GetCurrMethodIndex());
					strErrorCode += strMethodIndex;

					memset(szTemp, NULL, sizeof(szTemp));
					WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

					WriteMwiErrorCode(DEV_MCU, "IDC", strErrorCode.GetLength(), szTemp);	// 2003.11.11
				}
			}

			if (nErrorDevice & DEV_CDU)							// 2004.01.16_1.2_2003
			{
				if (GetMatrix(DEV_CDU) != NULL)		// [#2022] NH KSK 2011.02.22
				{
					strErrorCode = "97914";							
					strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_CDU)->GetCurrMethodIndex());
					strErrorCode += strMethodIndex;

					memset(szTemp, NULL, sizeof(szTemp));
					WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

					WriteMwiErrorCode(DEV_CDU, "CDM", strErrorCode.GetLength(), szTemp);
				}
			}

			if (nErrorDevice & DEV_PIN)				// 2004.01.16_1.2_2003
			{
				if (GetMatrix(DEV_PIN) != NULL)		// [#2022] NH KSK 2011.02.22
				{
					strErrorCode = "97918";							
					strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_PIN)->GetCurrMethodIndex());
					strErrorCode += strMethodIndex;

					memset(szTemp, NULL, sizeof(szTemp));
					WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

					WriteMwiErrorCode(DEV_PIN, "PINPAD", strErrorCode.GetLength(), szTemp);
				}
			}

			// [#2325] NH KSK 2015.01.20
			if (nErrorDevice & DEV_RFID)
			{
				if (GetMatrix(DEV_RFID) != NULL)
				{
					strErrorCode = "97919";							// MWI 장애코드 // 20030913_1.0_1072 : Timeover ErrorCode수정
					strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_RFID)->GetCurrMethodIndex());
					strErrorCode += strMethodIndex;

					memset(szTemp, NULL, sizeof(szTemp));
					WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

					WriteMwiErrorCode(DEV_RFID, "RFID", strErrorCode.GetLength(), szTemp);	// 2003.11.11
				}
			}
			// end of [#2325]

			// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
			if (nErrorDevice & DEV_BCR)
			{
				if (GetMatrix(DEV_BCR) != NULL)
				{
					strErrorCode = "97920";
					strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_BCR)->GetCurrMethodIndex());
					strErrorCode += strMethodIndex;

					memset(szTemp, NULL, sizeof(szTemp));
					WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

					WriteMwiErrorCode(DEV_BCR, DEVNM_BCR, strErrorCode.GetLength(), szTemp);	// 2003.11.11
				}
			}
			if (nErrorDevice & DEV_BNA)
			{
				if (GetMatrix(DEV_BNA) != NULL)
				{
					strErrorCode = "97921";
					strMethodIndex.Format(_T("%2.2d"), GetMatrix(DEV_BNA)->GetCurrMethodIndex());
					strErrorCode += strMethodIndex;

					memset(szTemp, NULL, sizeof(szTemp));
					WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

					WriteMwiErrorCode(DEV_BNA, DEVNM_BNA, strErrorCode.GetLength(), szTemp);	// 2003.11.11
				}
			}
			// end of [#GLDV-3005]
		}
		else if (nReturn == R_ERROR)							// FATALERROR時 9792XXX장애 유도
		{
			if (m_pOwner->m_nErrorType == DEVICE_ERROR)			// DeviceError는 장애코드 설정하지 않음
			{
				NHDEBUG(1, (_T("***CDevCtrl***CDevCtrl::CheckDeviceAction(int nDevID, int nWaitSec) ... DONE with DEVICE_ERROR\n")));
				return nReturn;
			}

			strErrorCode = "9792";								// MWI 장애코드
			
			strErrorCode += (nErrorDevice == DEV_JPR) ? "1" : 
							(nErrorDevice == DEV_SPR) ? "2" : 
							(nErrorDevice == DEV_MCU) ? "3" : 
							(nErrorDevice == DEV_CDU) ? "4" :
							(nErrorDevice == DEV_PIN) ? "8" :
							(nErrorDevice == DEV_RFID)? "9" :	// [#2325] NH KSK 2015.01.20
							(nErrorDevice == DEV_BCR)? "A" :	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
							(nErrorDevice == DEV_BNA)? "B" :	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
							"@";
			
			strMethodIndex.Format(_T("%2.2d"), GetMatrix(nErrorDevice)->GetCurrMethodIndex());
			strErrorCode += strMethodIndex;

			memset(szTemp, NULL, sizeof(szTemp));
			WideToMulti(szTemp, strErrorCode, sizeof(szTemp));

			if		(nErrorDevice & DEV_SPR)
				WriteMwiErrorCode(DEV_SPR, "SPR", strErrorCode.GetLength(), szTemp);// 2003.11.11
			else if (nErrorDevice & DEV_JPR)
				WriteMwiErrorCode(DEV_JPR, "JPR", strErrorCode.GetLength(), szTemp);// 2003.11.11
			else if (nErrorDevice & DEV_MCU)
				WriteMwiErrorCode(DEV_MCU, "IDC", strErrorCode.GetLength(), szTemp);// 2003.11.11
			else if (nErrorDevice & DEV_CDU)					
				WriteMwiErrorCode(DEV_CDU, "CDM", strErrorCode.GetLength(), szTemp);// 2004.01.16_1.2_2003
			else if (nErrorDevice & DEV_PIN)					
				WriteMwiErrorCode(DEV_PIN, "PINPAD", strErrorCode.GetLength(), szTemp);// 2004.01.16_1.2_2003
			else if (nErrorDevice & DEV_RFID)											// [#2325] NH KSK 2015.01.20
				WriteMwiErrorCode(DEV_RFID, "RFID", strErrorCode.GetLength(), szTemp);	// end of [#2325]
			// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
			else if (nErrorDevice & DEV_BCR)					
				WriteMwiErrorCode(DEV_BCR, DEVNM_BCR, strErrorCode.GetLength(), szTemp);
			else if (nErrorDevice & DEV_BNA)					
				WriteMwiErrorCode(DEV_BNA, DEVNM_BNA, strErrorCode.GetLength(), szTemp);
			// end of [#GLDV-3005]
		}
		
	}

	if (GetMatrix(nDevID) != NULL)
	{
		nMethodIndex = GetMatrix(nDevID)->GetCurrMethodIndex();
		nEventIndex = GetMatrix(nDevID)->GetEventIndex(nMethodIndex);

		NHDEBUG(1, (_T("[%S] '%S' | [%d:'%s()'] -> [%d:'%s'] (%lu ms)\n"),
			GET_DEVNAME_BY_ID(nDevID), GET_DVC_ACT_NAME(nReturn),
			nMethodIndex, GetMethodNameByIndex(nDevID, nMethodIndex),
			nEventIndex, GetEventNameByIndex(nDevID, nEventIndex),
			GetTickCount() - curTickCount));
	}
	else
	{
		NHDEBUG(1, (_T("FAILED TO FIND DEVICE MATRIX: [0x%08x:%S]\n"), nDevID, GET_DEVNAME_BY_ID(nDevID)));
	}

	return nReturn;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CheckDeviceMatrix(int nDevID)
// DESCRIPT  : 개별장치의 동작을 확인한다.
// PARAMETER : 장치ID
// RETURN    : 동작완료가 확인된 장치의 ID
// ----------------------------------------------------------------------------
int CDevCtrl::CheckDeviceMatrix(int nDevID)
{
	int nReturn = R_DOING;
	int	CheckDevice = nDevID;

	nReturn = CheckDeviceEventMatrix(nDevID);
	if ((nReturn == R_ERROR) || (nReturn == R_TIMEOUT))
	{
		CheckDevice &= ~nDevID;
		if (nReturn == R_ERROR)
			m_pOwner->m_nErrorDevice = nDevID;			// 장애디바이스 설정
		else
		//if (nReturn == R_TIMEOUT)						//[CS#011] NH AIREAT 2008.03.14
			m_pOwner->m_nTimeoutDevice = nDevID;		// T/O디바이스 설정
	}
	else if (nReturn == R_NORMAL)
	{
		CheckDevice &= ~nDevID;
		m_pOwner->m_nErrorDevice &= ~nDevID;			// 2003.06.09
		m_pOwner->m_nTimeoutDevice &= ~nDevID;
	}

	// 확인이 되지 않으면 큐처리후 재검사
	if (CheckDevice & nDevID)
	{
		CheckDeviceEvent(nDevID);

		nReturn = CheckDeviceEventMatrix(nDevID);
		if ((nReturn == R_ERROR) || (nReturn == R_TIMEOUT))
		{
			CheckDevice &= ~nDevID;
			if (nReturn == R_ERROR)
				m_pOwner->m_nErrorDevice = nDevID;		// 장애디바이스 설정
			else
			if (nReturn == R_TIMEOUT)
				m_pOwner->m_nTimeoutDevice = nDevID;	// T/O디바이스 설정
		}
		else if (nReturn == R_NORMAL)
		{
			CheckDevice &= ~nDevID;
			m_pOwner->m_nErrorDevice &= ~nDevID;		// 2003.06.09
			m_pOwner->m_nTimeoutDevice &= ~nDevID;
		}
	}

	return nReturn;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CDevCtrl::CheckDeviceEvent(int nDevID)
// DESCRIPT : 장치별로 이벤트를 확인한다.
// 매개변수 : 장치아디
// 반 환 값 : TRUE
// 기타사항 : 먼저 장치별로 이벤트큐에서 이벤트를 가져와서 이를 이벤트메트릭스로 확인한다.
// ----------------------------------------------------------------------------
BOOL CDevCtrl::CheckDeviceEvent(int nDevID)
{
	BOOL	bReturn = FALSE;
	char	szType[128] = {}, szName[128] = {}, szEvent[128] = {}, szValue[128] = {};

	WideToMulti(szName, GetDevNameByDevID(nDevID), sizeof(szName));
	
	if (!EvtQIsEmpty(nDevID))									// 20030703_1.0_1046 
	{															// 큐가 비어있다면 아무런 처리를 하지않게한다.
		// 큐에서 이벤트를 하나 가져온다.
		bReturn = EvtQGetData(szType, szName, szEvent, szValue);
		if (bReturn)
		{
			// 디바이스명과 이벤트명으로 이벤트메트릭스를 설정한다.
			SetEventMatrixValue(szName, szEvent);
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::CheckDeviceEventMatrix(int nDevID)
// DESCRIPT : 이벤트를 가지고 장치별 이벤트매트릭스를 확인한다.
// ----------------------------------------------------------------------------
int CDevCtrl::CheckDeviceEventMatrix(int nDevID)
{
	int		nReturn = R_DOING;
	int		nMethodIndex, nIndex;
	CString	strEvent("");

	CMatrix*	pMatrix = GetMatrix(nDevID);

	m_pOwner->m_nErrorType = 0;									// 20030729_1.0_1055 : Set ErrorCode

	// 메트릭스 포인터 확인
	if (pMatrix == NULL)
	{
		return R_NORMAL;
	}

	nMethodIndex = pMatrix->GetCurrMethodIndex();		

	// 인덱스가 0미만이면 확인하지 않음 - 한번도 디바이스가 구동되지 않음
	// 인덱스로 0이 올수 있음 따라서 0미만으로 확인해야 함(2003.05.19)
	if (nMethodIndex < 0) return R_NORMAL;

	for (nIndex = 0; nIndex < pMatrix->GetEventCount(); nIndex++)
	{
		if ((pMatrix->GetMasterMatrixValue(nMethodIndex, nIndex) == MWI_EVENT_EXIST)	&&
			(pMatrix->GetEventMatrixValue(nMethodIndex, nIndex) == MWI_EVENT_OCCUR)	)
		{
			strEvent.Format(_T("%s"), GetEventNameByIndex(nDevID, nIndex));

			if (strEvent.IsEmpty())	continue;
			if (strEvent.CompareNoCase(L"DeviceError") == 0)
			{
				m_pOwner->m_nErrorType = DEVICE_ERROR;			// 20030729_1.0_1055 : Set ErrorCode
				nReturn = R_ERROR;
			}
			else 
			if (strEvent.CompareNoCase(L"FatalError") == 0)
			{
				m_pOwner->m_nErrorType = FATAL_ERROR;			// 20030729_1.0_1055 : Set ErrorCode
				nReturn = R_ERROR;
			}
			else 
			if (strEvent.CompareNoCase(L"Timeout") == 0)
			{
				nReturn = R_TIMEOUT;
			}
			else
			{
				nReturn = R_NORMAL;
			}
		}
	}

	return nReturn;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::ScanDeviceAction(int nDevID, int nWaitSec, int nEventKind)
// DESCRIPT : 관련이벤트가 발생되었는지 검색한다
// ----------------------------------------------------------------------------
int CDevCtrl::ScanDeviceAction(int nDevID, int nWaitSec, int nEventKind)
{
	int nResult = DEV_NONE;
	BOOL bInserted  = FALSE;
	BOOL bCheckOnce = FALSE;

	int		CheckDevice = nDevID;

	if (nWaitSec == 0)											// Checking시간이 0이면 한번만 처리
	{
		// 코드 처리 필요함
		bCheckOnce = TRUE;
	}

	ULONG uTime = TimerSet(nWaitSec);							// 장치 Check
	while(true)
	{
		// ------------------------------------------------------------------------
		if (CheckDevice == DEV_NONE)		break;				// 장치확인 완료시 확인작업 종료함
		
		// ------------------------------------------------------------------------
		if (CheckDevice & DEV_SCR)								// 화면입력 점검요청
		{	
			if (!EvtQIsEmpty(DEV_SCR))							// 데이타 있음
			{
				nResult = DEV_SCR;								// 화면큐가 비어있지 않으면 데이타있음으로 간주
				break;
			}
			Delay_Msg(5);
		}

		// ------------------------------------------------------------------------
		// 카드부 이벤트 검지 처리
		if ((CheckDevice & DEV_MCU) && (!EvtQIsEmpty(DEV_MCU)))
		{
			if (nEventKind == EVENT_IN)							// EVENTKIND : EVENT_IN
			{
				// [#259] NH JSW 2008.06.05 
				// MS Card: MediaInserted 이벤트 대기 -> ReadComplete 또는 InvalidMedia 대기로 변경
				// IC Card: ChipIOComplete 이벤트 대기
//				// 카드 들어옴
//				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "MediaInserted", "");
// 				if (bInserted)
// 				{
// 					nResult = DEV_MCU;
// 					break;
// 				}
				// 카드읽기 완료
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "ReadComplete", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
				// 카드읽기 실패
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "InvalidMedia", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
				// 카드읽기 실패
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "InvalidTrackData", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
				// IC카드읽기 완료(실패 포함)
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "ChipIOComplete", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
				// end of [#259]
			}
			else if (nEventKind == EVENT_OUT)					// EVENTKIND : EVENT_OUT
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "EjectComplete", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
																// 2005.11.11
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "CardTaken", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
			}
			else												// EVENT_ALL
			{
				if (!EvtQIsEmpty(DEV_MCU))	
				{
					nResult = DEV_MCU;							// 큐가 비어있지 않으면 데이타있음으로 간주
					break;
				}
			}

			// 20030711_1.0_1052 : SCANDEVICEACTION DEBUG
			// 장애이벤트/타임아웃이벤트가 존재하는지 확인한다.	
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "DeviceError", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "FatalError", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_MCU, "Timeout", "");
				if (bInserted)
				{
					nResult = DEV_MCU;
					break;
				}
			}
			Delay_Msg(5);
		}

		// ------------------------------------------------------------------------
		// 현금부 이벤트 검지 처리 : 2004.01.16_1.2_2003
		if ((CheckDevice & DEV_CDU)	&& (!EvtQIsEmpty(DEV_CDU)))
		{
			if (nEventKind == EVENT_IN)							// EVENTKIND : EVENT_IN
			{
			}
			else if (nEventKind == EVENT_OUT)					// EVENTKIND : EVENT_OUT
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_CDU, "PresentComplete", "");
				if (bInserted)
				{
					nResult = DEV_CDU;
					break;
				}
																// 2005.11.11
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_CDU, "CashLifted", "");
				if (bInserted)
				{
					nResult = DEV_CDU;
					break;
				}
																// 2005.11.11
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_CDU, "ItemsTaken", "");
				if (bInserted)
				{
					nResult = DEV_CDU;
					break;
				}
				
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_CDU, "RejectComplete", "");
				if (bInserted)
				{
					nResult = DEV_CDU;
					break;
				}
			}
			else												// EVENT_ALL
			{
				if (!EvtQIsEmpty(DEV_CDU))	
				{
					nResult = DEV_CDU;							// 큐가 비어있지 않으면 데이타있음으로 간주
					break;
				}
			}

			// 20030711_1.0_1052 : SCANDEVICEACTION DEBUG
			// 장애이벤트/타임아웃이벤트가 존재하는지 확인한다.	
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_CDU, "DeviceError", "");
				if (bInserted)
				{
					nResult = DEV_CDU;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_CDU, "FatalError", "");
				if (bInserted)
				{
					nResult = DEV_CDU;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_CDU, "Timeout", "");
				if (bInserted)
				{
					nResult = DEV_CDU;
					break;
				}
			}

			Delay_Msg(5);
		}

		// ------------------------------------------------------------------------
		// PINPAD부 이벤트 검지 처리
		if ((CheckDevice & DEV_PIN)		&&
			(!EvtQIsEmpty(DEV_PIN))		)
		{
			if (nEventKind == EVENT_IN)							// EVENTKIND : EVENT_IN
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_PIN, "KeyPressed", "");
				if (bInserted)
				{
					nResult = DEV_PIN;
					break;
				}
				
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_PIN, "ReadDataComplete", "");
				if (bInserted)
				{
					nResult = DEV_PIN;
					break;
				}
				
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_PIN, "ReadPinComplete", "");
				if (bInserted)
				{
					nResult = DEV_PIN;
					break;
				}
			}
			else if (nEventKind == EVENT_OUT)					// EVENTKIND : EVENT_OUT
			{
				;
			}
			else												// EVENT_ALL
			{
				if (!EvtQIsEmpty(DEV_PIN))	
				{
					nResult = DEV_PIN;
					break;
				}
			}

			// 20030711_1.0_1052 : SCANDEVICEACTION DEBUG
			// 장애이벤트/타임아웃이벤트가 존재하는지 확인한다.	
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_PIN, "DeviceError", "");
				if (bInserted)
				{
					nResult = DEV_PIN;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_PIN, "FatalError", "");
				if (bInserted)
				{
					nResult = DEV_PIN;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_PIN, "Timeout", "");
				if (bInserted)
				{
					nResult = DEV_PIN;
					break;
				}
			}
			Delay_Msg(5);
		}

		// [#2325] NH KSK 2015.01.20
		// RFID 이벤트 검지 처리
		if ((CheckDevice & DEV_RFID) && (!EvtQIsEmpty(DEV_RFID)))
		{
			if (nEventKind == EVENT_IN)							// EVENTKIND : EVENT_IN
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "ReadComplete", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
				// RFID 실패
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "InvalidMedia", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
				// RFID 실패
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "InvalidTrackData", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
			}
			else if (nEventKind == EVENT_OUT)					// EVENTKIND : EVENT_OUT
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "EjectComplete", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "CardTaken", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
			}
//			else												// EVENT_ALL
//			{
//				if (!EvtQIsEmpty(DEV_RFID))	
//				{
//					nResult = DEV_RFID;							// 큐가 비어있지 않으면 데이타있음으로 간주
//					break;
//				}
//			}

			// 20030711_1.0_1052 : SCANDEVICEACTION DEBUG
			// 장애이벤트/타임아웃이벤트가 존재하는지 확인한다.	
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "DeviceError", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "FatalError", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_RFID, "Timeout", "");
				if (bInserted)
				{
					nResult = DEV_RFID;
					break;
				}
			}
			Delay_Msg(5);
		}
		// end of [#2325]

		// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
		// TODO: implement BCR/BNA devices (added stubs for now)
		if ((CheckDevice & DEV_BCR) && (!EvtQIsEmpty(DEV_BCR)))
		{
			if (nEventKind == EVENT_IN)							// EVENTKIND : EVENT_IN
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BCR, "SendRawDataComplete", "");
				if (bInserted)
				{
					nResult = DEV_BCR;
					break;
				}

				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BCR, "AcceptCancelled", "");
				if (bInserted)
				{
					nResult = DEV_BCR;
					break;
				}

				//// BCR 실패
				//bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BCR, "InvalidMedia", "");
				//if (bInserted)
				//{
				//	nResult = DEV_BCR;
				//	break;
				//}
			}
			else if (nEventKind == EVENT_OUT)					// EVENTKIND : EVENT_OUT
			{
				;
			}

			// 20030711_1.0_1052 : SCANDEVICEACTION DEBUG
			// 장애이벤트/타임아웃이벤트가 존재하는지 확인한다.	
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BCR, "DeviceError", "");
				if (bInserted)
				{
					nResult = DEV_BCR;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BCR, "FatalError", "");
				if (bInserted)
				{
					nResult = DEV_BCR;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BCR, "Timeout", "");
				if (bInserted)
				{
					nResult = DEV_BCR;
					break;
				}
			}
			Delay_Msg(5);
		}

		if ((CheckDevice & DEV_BNA) && (!EvtQIsEmpty(DEV_BNA)))
		{
			if (nEventKind == EVENT_IN)							// EVENTKIND : EVENT_IN
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "StartCashInComplete", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}

				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "AcceptCashComplete", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}

				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "RollbackCashComplete", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}

				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "AcceptCancelled", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}
			}
			else if (nEventKind == EVENT_OUT)					// EVENTKIND : EVENT_OUT
			{
				// BNA 실패
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "ItemsRefused", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}

				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "EjectComplete", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}
			}

			// 20030711_1.0_1052 : SCANDEVICEACTION DEBUG
			// 장애이벤트/타임아웃이벤트가 존재하는지 확인한다.	
			{
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "DeviceError", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "FatalError", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}
				bInserted = EvtQCheckDeviceDataExist(MWI_EVENT_DEVICE, DEVNM_BNA, "Timeout", "");
				if (bInserted)
				{
					nResult = DEV_BNA;
					break;
				}
			}
			Delay_Msg(5);
		}
		// end of [#GLDV-3005]
		
		// ------------------------------------------------------------------------
		if ((nWaitSec > 0) && (CheckTimer(uTime)))				// 20030711_1.0_1052 : TIMEOUT DEBUG
		{														// - nWaitSec가 0이면 타이아웃을 발생하지 않음
			nResult = R_TIMEOUT;
			break;
		}
		if (bCheckOnce == TRUE)				break;
		Delay_Msg(5);
		
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::CheckDeviceOpend(int nDevID, int nWaitSec)
// DESCRIPT : 장치디바이스 오픈 체크
// ----------------------------------------------------------------------------
int CDevCtrl::CheckDeviceOpend(int nDevID, int nWaitSec)		// 2005.03.30
{
	ULONG uTime = TimerSet(nWaitSec);
	while(true)
	{
		if (nDevID == DEV_DOR)
		{
			if (m_pOwner->m_bDoorOpenedEvent)
				break;
		}

		if (nDevID == DEV_LGT)
		{
			if ((m_pOwner->m_bLightOpenedEvent) &&
				(m_pOwner->m_bIndOpenedEvent))
				break;
		}

		if (nDevID == DEV_SNS)
		{
			if (m_pOwner->m_bSensorOpenedEvent)
				break;
		}

		Delay_Msg(50);

		if ((nWaitSec > 0) && (CheckTimer(uTime)))
			return R_TIMEOVER;
	}

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CDevCtrl::CheckDeviceClosed(int nDevID, int nWaitSec)
// DESCRIPT : 장치디바이스 클로우즈 체크
// ----------------------------------------------------------------------------
int CDevCtrl::CheckDeviceClosed(int nDevID, int nWaitSec)		// 2005.03.30
{
	ULONG uTime = TimerSet(nWaitSec);
	while(true)
	{
		if (nDevID == DEV_DOR)
		{
			if (m_pOwner->m_bDoorClosedEvent)
				break;
		}

		if (nDevID == DEV_LGT)
		{
			if ((m_pOwner->m_bLightClosedEvent) &&
				(m_pOwner->m_bIndClosedEvent))
				break;
		}

		if (nDevID == DEV_SNS)
		{
			if (m_pOwner->m_bSensorClosedEvent)
				break;
		}

		Delay_Msg(50);

		if ((nWaitSec > 0) && (CheckTimer(uTime)))
			return R_TIMEOVER;
	}

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString CDevCtrl::GetScrKeyString(int nWaitSec)
// DESCRIPT : 화면데이타를 가져온다.
// ----------------------------------------------------------------------------
CString CDevCtrl::GetScrKeyString(int nWaitSec)
{
	char	szType[1024] = {}, szName[1024] = {}, szEvent[1024] = {}, szValue[1024] = {};
	BOOL	bReturn = FALSE;

	CString strResult("");
	ULONG uTime = TimerSet(nWaitSec);							// 20030913_1.0_1072 : 화면값조회 수정
																// 15초 보정로직을 사용하였으나 제거함

	while(true)
	{
		if (!EvtQIsEmpty(DEV_SCR))	
		{
			strcpy(szName, DEVNM_SCR);
			bReturn = EvtQGetData(szType, szName, szEvent, szValue);
			if (bReturn)
			{
				strResult.Format(_T("%S"), szValue);
				break;
			}
		}

		Delay_Msg(50);

		if ((nWaitSec > 0) && (CheckTimer(uTime)))				// 20030913_1.0_1072 : 화면값조회 수정
		{														// 시간값이 0이상인경우에만 TIMEOVER값을 만듦
			strResult.Format(_T("%S"), "TIMEOVER");
			break;
		}

		Delay_Msg(50);
	}

	return strResult;
}


// ----------------------------------------------------------------------------
// DESCRIPT  : 979장애를 로긴한다
// PARAMETER : 키, 코드길이, 코드
// RETURN    : 0
// REMARK	 : 레지스트리의 해당키에 장애코드가 있는지 확인하고, 장애코드가 있을
//			   경우에는 장애코드를 기록하지 않고, 장애코드가 없을 경우에만 기록
//			   2003.11.11
// ----------------------------------------------------------------------------
int CDevCtrl::WriteMwiErrorCode(int nDevID, char* szEntry, int nCodeLen, char* szErrorCode)
{
	CString strErrorCode("");
	switch(nDevID)
	{
	case DEV_SPR:
		strErrorCode.Format(_T("%s"), m_pOwner->DevSprGetErrorCode());
		break;
	case DEV_JPR:
		strErrorCode.Format(_T("%s"), m_pOwner->DevJprGetErrorCode());
		break;
	case DEV_MCU:
		strErrorCode.Format(_T("%s"), m_pOwner->DevMcuGetErrorCode());
		break;
	case DEV_CDU:
		strErrorCode.Format(_T("%s"), m_pOwner->DevCduGetErrorCode());	// 2004.01.16_1.2_2003
		break;
	case DEV_PIN:
		strErrorCode.Format(_T("%s"), m_pOwner->DevPinGetErrorCode());	// 2004.01.16_1.2_2003
		break;
	case DEV_RFID:														// [#2325] NH KSK 2015.01.20
		strErrorCode.Format(_T("%s"), m_pOwner->DevRFIDGetErrorCode());	// end of [#2325]
		break;
		// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	case DEV_BCR:
		strErrorCode.Format(_T("%s"), m_pOwner->DevBcrGetErrorCode());
		break;
		// TODO
	case DEV_BNA:
		strErrorCode.Format(_T("%s"), m_pOwner->DevBnaGetErrorCode());
		break;
		// end of [#GLDV-3005]
	default:
		strErrorCode.Format(_T("%s"), L"0000000");	// 2008-01-14 V01.02.25 SRC-8 구문에러 수정
		break;
	}

	if (Asc2Int(strErrorCode) != 0)	return R_NORMAL;

	CString strEntry = CString(szEntry);
	m_pOwner->WriteMwiErrorCode_NVRam(strEntry, szErrorCode);	// 2008-01-14 V01.02.25 SRC-8

	return R_NORMAL;
}
