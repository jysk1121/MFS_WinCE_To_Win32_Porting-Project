#include "stdafx.h"
#include "DEV_Define.h"
#include "DeviceSimUtil.h"
#include "DevSimSensorDlg.h"
#include "DEV_DIO.h"
#include "DEV_DIO_Define.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "DEV_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_DIO::CDEV_DIO()
{
	m_hWnd = NULL;

	m_FlickerLedThread	= NULL;
	m_WatchSensorThread = NULL;

	memset(m_SetLed, 0, sizeof(m_SetLed));
	memset(m_FlickerLed, 0, sizeof(m_FlickerLed));
	memset(m_byTemp, 0, sizeof(m_byTemp));

	m_byAction = 0;
	m_bExecFlag = FALSE;

	m_bThreadExit			= FALSE;

	m_bAudioJackInserted	= FALSE;
	m_bSafeDoorOpened		= FALSE;
	m_bTopDoorOpened		= FALSE;
	m_szSensor				= NULL;
	m_szPrevSensor			= (char)0xFF;	//최초 1회 Message전송을 위해

	m_bSupportFlicking = FALSE;
}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_DIO::~CDEV_DIO()
{

}


/** **********************************************************
*	@brief		Is DLL Loaded - 미사용
*	@retval		없음
************************************************************/
BOOL CDEV_DIO::IsDllLoaded(void)
{
#if	EMULATION_SIU_DEVICE
	return TRUE;
#endif

	return TRUE;
}


/** **********************************************************
*	@brief		열기
*	@retval		없음
************************************************************/
BOOL CDEV_DIO::Open(HWND hWnd, int nPortNum)
{
#if	EMULATION_SIU_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	BOOL bSimOpened = CDevSim::Confirm(_T("DIO"), _T("DIO_Open"));

	if (bSimOpened)
	{
		m_hWnd        = hWnd;
		m_bThreadExit = FALSE;

		// The sensor panel stands in for the SIU hardware. StartDIOThread()
		// then reports whatever it holds through the normal WM_DIO_NOTIFY path,
		// so the ATM sees door changes exactly as it would on a real terminal.
		CDevSimSensorDlg::Show();

		StartDIOThread();
	}

	return bSimOpened;
#endif

	if(FALSE == CDEV_Manager::GetInstance()->m_DEV_MUB.Open(hWnd, nPortNum))
	{
		LOG(Error, _T("[CDEV_DIO] DEV_MUB Open failed."));
		return FALSE;
	}

	m_bThreadExit = FALSE;

	//Start LED/Sensor thread
	m_hWnd = hWnd;

	StartDIOThread();

	return TRUE;
}


/** **********************************************************
*	@brief		닫기
*	@retval		없음
************************************************************/
BOOL CDEV_DIO::Close()
{
#if	EMULATION_SIU_DEVICE
	CDevSimSensorDlg::Destroy();
#endif

#if	EMULATION_SIU_DEVICE
	return CDevSim::Confirm(_T("DIO"), _T("DIO_Close"));
#endif

	m_bThreadExit = TRUE;

	if (m_FlickerLedThread != NULL)
	{
		::TerminateThread(m_FlickerLedThread, 0);
		CloseHandle(m_FlickerLedThread);
		m_FlickerLedThread = NULL;
	}

	if (m_WatchSensorThread != NULL)
	{
		::TerminateThread(m_WatchSensorThread, 0);
		CloseHandle(m_WatchSensorThread);
		m_WatchSensorThread = NULL;
	}

//	return TRUE;
	return CDEV_Manager::GetInstance()->m_DEV_MUB.Close();
}


/** **********************************************************
*	@brief		버전 취득
*	@retval		버전 정보
************************************************************/
CString CDEV_DIO::GetVersion()
{
#if	EMULATION_SIU_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	return CDevSim::ReadString(_T("DIO"), _T("Version"), _T("01.00"));
#endif

	BOOL bResult = FALSE;
	char szFWVer[64] = { 0, };
	char szDLLVer[64] = { 0, };

//	bResult = CDEV_Manager::GetInstance()->m_DEV_MUB.MUB_DIO_GetVersion(szFWVer, szDLLVer);

	m_DIOLock.Lock();

	BYTE byRecvBuff[1024] = {0,};
	int nRecvLen = 0;

//	bResult = CDEV_Manager::GetInstance()->m_DEV_MUB.HandShakeDeviceCommand(ID_SIU, MakeSendFormat(SIU_VERSION), byRecvBuff, DIO_SEND_DATA_FIXED_LEN, &nRecvLen);
	
	bResult = CDEV_Manager::GetInstance()->m_DEV_MUB.GetVersion((LPSTR)byRecvBuff);

	if (bResult == TRUE)
	{
		sprintf(szFWVer, "V%c%c.%c%c", byRecvBuff[0], byRecvBuff[1], byRecvBuff[2], byRecvBuff[3]);	// DIO 버전 조회시에는 index 조정 필요
		m_strDIO_FW_Version = (CString)szFWVer;

		if (byRecvBuff[3] >= SIU_SUPPORTFLICKING_VERSION)		// 4버전이상부터 Flicking 기능 추가됨
		{
			m_bSupportFlicking = TRUE;
			LOG(Info, _T("[CDEV_DIO] MUB FW Version [%s], m_bSupportFlcikng : %d"), m_strDIO_FW_Version, m_bSupportFlicking);
		}
	}

	m_DIOLock.Unlock();

	return m_strDIO_FW_Version;
}


/** **********************************************************
*	@brief		LED 설정
*	@retval		없음
************************************************************/
BOOL CDEV_DIO::SetLed(BYTE byLED_ID, BYTE byAction)
{
#if	EMULATION_SIU_DEVICE
	// LED control is cosmetic and fires continuously while flickering, so it is
	// simulated silently - a dialog on every LED change would block the whole UI.
	return TRUE;
#endif

	m_byAction = byAction;

	if (byAction == DIO_CMD_OFF)
	{
		if (byLED_ID & DIO_1)
		{
			m_SetLed[0] &= ~(DIO_1);
			
			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_1);
		}

		if (byLED_ID & DIO_2)
		{
			m_SetLed[0] &= ~(DIO_2);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_2);
		}

		if (byLED_ID & DIO_3)
		{
			m_SetLed[0] &= ~(DIO_3);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_3);
		}

		if (byLED_ID & DIO_4)
		{
			m_SetLed[0] &= ~(DIO_4);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_4);
		}

		if (byLED_ID & DIO_5)
		{
			m_SetLed[1] &= ~((BYTE)0x01);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] &= ~((BYTE)0x01);
		}

		if (byLED_ID & DIO_6)
		{
			m_SetLed[1] &= ~((BYTE)0x02);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] &= ~((BYTE)0x02);
		}

		if (byLED_ID & DIO_7)
		{
			m_SetLed[1] &= ~((BYTE)0x04);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] &= ~((BYTE)0x04);
		}

		DIO_SetLED(m_SetLed[0], m_SetLed[1]);
	}
	else if (byAction == DIO_CMD_ON)
	{
		if (byLED_ID & DIO_1)
		{
			m_SetLed[0] |= DIO_1;

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_1);
		}

		if (byLED_ID & DIO_2)
		{
			m_SetLed[0] |= DIO_2;

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_2);
		}

		if (byLED_ID & DIO_3)
		{
			m_SetLed[0] |= DIO_3;

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_3);
		}

		if (byLED_ID & DIO_4)
		{
			m_SetLed[0] |= DIO_4;

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] &= ~(DIO_4);
		}

		if (byLED_ID & DIO_5)
		{
			m_SetLed[1] |= (BYTE)0x01;

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] &= ~((BYTE)0x01);
		}

		if (byLED_ID & DIO_6)
		{
			m_SetLed[1] |= (BYTE)0x02;

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] &= ~((BYTE)0x02);
		}

		if (byLED_ID & DIO_7)
		{
			m_SetLed[1] |= (BYTE)0x04;

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] &= ~((BYTE)0x04);
		}

		DIO_SetLED(m_SetLed[0], m_SetLed[1]);
	}
	else if (byAction = DIO_CMD_FLICKING)
	{
		if (byLED_ID & DIO_1)
		{
			m_SetLed[0] &= ~(DIO_1);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] |= DIO_1;
			else
				DIO_SetFlicking(SIU_PRTLED_FLICKING, 0x85);
		}

		if (byLED_ID & DIO_2)
		{
			m_SetLed[0] &= ~(DIO_2);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] |= DIO_2;
			else
				DIO_SetFlicking(SIU_CDRLED_FLICKING, 0x85);
		}

		if (byLED_ID & DIO_3)
		{
			m_SetLed[0] &= ~(DIO_3);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] |= DIO_3;
			else
				DIO_SetFlicking(SIU_CDMLED_FLICKING, 0x85);
		}

		if (byLED_ID & DIO_4)
		{
			m_SetLed[0] &= ~(DIO_4);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[0] |= DIO_4;
			else
				DIO_SetFlicking(SIU_EPPLED_FLICKING, 0x85);
		}

		if (byLED_ID & DIO_5)
		{
			m_SetLed[1] &= ~((BYTE)0x01);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] |= (BYTE)0x01;
			else
				DIO_SetFlicking(SIU_TOPLED_FLICKING, 0x85);
		}

		if (byLED_ID & DIO_6)
		{
			m_SetLed[1] &= ~((BYTE)0x02);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] |= (BYTE)0x02;
			else
				DIO_SetFlicking(SIU_EPPSIDELED_FLICKING, 0x85);
		}

		if (byLED_ID & DIO_7)
		{
			m_SetLed[1] &= ~((BYTE)0x04);

			if (m_bSupportFlicking == FALSE)
				m_FlickerLed[1] |= (BYTE)0x04;
			else
				DIO_SetFlicking(SIU_BOTTOMLED_FLICKING, 0x85);
		}

		if (m_bSupportFlicking == FALSE)
			m_bExecFlag = TRUE;
	}

	//m_bExecFlag = TRUE;

	return TRUE;
}


BOOL CDEV_DIO::GetSensor(char* pStatus)
{
#if	EMULATION_SIU_DEVICE
	// Live value from the sensor panel (seeded from DeviceSim.ini [DIO] SensorStatus).
	if (pStatus != NULL)
		pStatus[0] = (char)CDevSimSensorDlg::GetSensorByte();

	return TRUE;
#endif

	memcpy(pStatus, (char*)m_szSensor, 1);

	return TRUE;
}


void CDEV_DIO::StartDIOThread()
{
	m_FlickerLedThread = NULL;
	m_FlickerLedThread = AfxBeginThread((AFX_THREADPROC)FlickerLed, this);

	CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);

	m_WatchSensorThread = NULL;
	m_WatchSensorThread = AfxBeginThread((AFX_THREADPROC)WatchSensor, this);

	return;
}

/** **********************************************************
*	@brief		LED 점멸
*	@retval		없음
************************************************************/
UINT CDEV_DIO::FlickerLed(LPVOID pParam)
{
#if	EMULATION_SIU_DEVICE
	return TRUE;
#endif

	CDEV_DIO* pDIO = (CDEV_DIO*)pParam;

	pDIO->FlickerLed();

	return 0;
}

/** **********************************************************
*	@brief		LED 점멸
*	@retval		없음
************************************************************/
void CDEV_DIO::FlickerLed(void)
{
	LOG(Info, _T("[CDEV_DIO] FlickerLed Thread Start"));

	BYTE	bySaveCondition = 0;

	memset(m_SetLed, 0, sizeof(m_SetLed));
	DIO_SetLED(m_SetLed[0], m_SetLed[1]);

	while(TRUE)
	{
		// Close시 Thread 종료
		if(m_bThreadExit)
		{
			memset(m_SetLed, 0, sizeof(m_SetLed));
			DIO_SetLED(m_SetLed[0], m_SetLed[1]);
			m_bExecFlag = FALSE;
			break;
		}

		//if (m_bExecFlag == TRUE)
		//{
		//	if (m_byAction == DIO_CMD_FLICKING)
		//	{
		//		bySaveCondition = 0;
		//	}
		//	else
		//	{
		//		DIO_SetLED(m_SetLed[0], m_SetLed[1]);
		//		//m_bExecFlag = FALSE;
		//	}

		//	m_bExecFlag = FALSE;
		//}

		if (m_bExecFlag == TRUE)
		{
			bySaveCondition = 0;
			m_bExecFlag = FALSE;
		}


		if (m_FlickerLed[0] != (BYTE)0x00 || m_FlickerLed[1] != (BYTE)0x00)
		{
			memset(m_byTemp, 0, sizeof(m_byTemp));

			if (bySaveCondition == 0)
			{
				// LED ON
				bySaveCondition = 1;
				m_byTemp[0] = m_FlickerLed[0];
				m_byTemp[0] |= m_SetLed[0];
				m_byTemp[1] = m_FlickerLed[1];
				m_byTemp[1] |= m_SetLed[1];
			}
			else
			{
				// LED OFF
				bySaveCondition = 0;
				m_byTemp[0] |= m_SetLed[0];
				m_byTemp[1] |= m_SetLed[1];
			}

			DIO_SetLED(m_byTemp[0], m_byTemp[1]);
			m_bExecFlag = FALSE;

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_500MS);
			continue;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}

	LOG(Info, _T("[CDEV_DIO] FlickerLed Thread Exit"));
}

/** **********************************************************
*	@brief		Sensor 감시 시작
*	@retval		없음
************************************************************/
UINT CDEV_DIO::WatchSensor( LPVOID pParam )
{
	CDEV_DIO* pDIO = (CDEV_DIO*)pParam;

	pDIO->WatchSensor();

	return 0;
}

/** **********************************************************
*	@brief		Sensor 감시
*	@retval		없음
************************************************************/
void CDEV_DIO::WatchSensor( void )
{
	m_szSensor = NULL;

	LOG(Info, _T("[CDEV_DIO] WatchSensor Thread Start"));

	while (1)
	{
		// Close시 Thread 종료
		if(m_bThreadExit)
		{
			break;
		}

		m_szSensor = 0;
		
	//	CDEV_Manager::GetInstance()->m_DEV_MUB.MUB_DIO_GetSensor(&m_szSensor);
#if	EMULATION_SIU_DEVICE
		// No SIU on the desktop - the panel supplies the byte.
		m_szSensor = (char)CDevSimSensorDlg::GetSensorByte();
#else
		DIO_GetSensor(&m_szSensor);
#endif

		if (m_szPrevSensor != m_szSensor)
		{
			LOG(Info, _T("[CDEV_DIO] Status Changed = 0x%02x"), (BYTE)m_szSensor);

			m_szPrevSensor = m_szSensor;

			// EarJack 변수 갱신
			if (m_szSensor & 0x10)
				m_bAudioJackInserted = TRUE;
			else
				m_bAudioJackInserted = FALSE;

			::PostMessage(m_hWnd, WM_DIO_NOTIFY, (WPARAM)m_szSensor, NULL);
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);
	}

	LOG(Info, _T("[CDEV_DIO] WatchSensor Thread Exit"));
}


/** **********************************************************
*	@brief		DIO_GetSensor
*	@param	char* pStatus	센서 정보
*	@retval	TRUE 성공
*	@retval	FALSE 실패
************************************************************/
BOOL CDEV_DIO::DIO_GetSensor(char* pStatus)
{
	BOOL bResult = FALSE;
	BYTE byRecvBuff[1024] = {0,};
	int nRecvLen = 0;	

	m_DIOLock.Lock();

	bResult = CDEV_Manager::GetInstance()->m_DEV_MUB.HandShakeDeviceCommand(ID_SIU, MakeSendFormat(SIU_SENSOR), byRecvBuff, DIO_SEND_DATA_FIXED_LEN, &nRecvLen);

	if (bResult == TRUE)
		memcpy(pStatus, &byRecvBuff[3], 1);

	m_DIOLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		DIO_SetLED
*	@param	BYTE byLED_1	PTR LED, CDR LED, WDM LED, EPP LED, CIM LED 정보
*	@param	BYTE byLED_2	Top LED, EPP Side LED, Bottom LED, ATM LED 정보
*	@retval	TRUE 성공
*	@retval	FALSE 실패
************************************************************/
BOOL CDEV_DIO::DIO_SetLED(BYTE byLED_1, BYTE byLED_2)
{
	BOOL bResult = FALSE;
	BYTE byRecvBuff[1024] = {0,};
	BYTE byData[2] = {0,};
	int nRecvLen = 0;

	m_DIOLock.Lock();

	if (m_bSupportFlicking == TRUE)
		LOG(Info, _T("[CDEV_DIO] DIO_SetLED - BYTE_1[0x%02X], BYTE_2[0x%02X]"), byLED_1, byLED_2);

	byData[0] = byLED_1;
	byData[1] = byLED_2;

	bResult = CDEV_Manager::GetInstance()->m_DEV_MUB.HandShakeDeviceCommand(ID_SIU, MakeSendFormat(SIU_FLICKER, byData, sizeof(byData)), byRecvBuff, (DIO_SEND_DATA_FIXED_LEN+sizeof(byData)), &nRecvLen);

	m_DIOLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		DIO_SetLED
*	@param	BYTE byDev	PTR LED(x030), CDR LED(0x31), WDM LED(0x32), EPP LED(0x33), TOP LED(0x34), EPP SIDE LED(0x35), BOTTOM LED(0x36), ATM LED(0x37), CIM LED(0x38) 정보
*	@param	BYTE byLED_2	Top LED, EPP Side LED, Bottom LED, ATM LED 정보
*	@retval	TRUE 성공
*	@retval	FALSE 실패
************************************************************/
BOOL CDEV_DIO::DIO_SetFlicking(BYTE byDev, BYTE byTime)
{
	BOOL bResult = FALSE;
	BYTE byRecvBuff[1024] = {0,};
	BYTE byData[2] = {0,};
	int nRecvLen = 0;

	m_DIOLock.Lock();

		LOG(Info, _T("[CDEV_DIO] DIO_SetFlicking - BYTE_1[0x%02X], BYTE_2[0x%02X]"), byDev, byTime);

	byData[0] = byDev;
	byData[1] = byTime;

	bResult = CDEV_Manager::GetInstance()->m_DEV_MUB.HandShakeDeviceCommand(ID_SIU, MakeSendFormat(SIU_FLICKING, byData, sizeof(byData)), byRecvBuff, (DIO_SEND_DATA_FIXED_LEN+sizeof(byData)), &nRecvLen);

	m_DIOLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		GetCheckSum
*	@param	LPSTR lpText	data 정보
*	@param	UINT nStartPos	CheckSum 산출 시작 위치
*	@param	UINT nEndPos	CheckSum 산출 종료 위치
*	@retval	CheckSum 결과 값
************************************************************/
BYTE CDEV_DIO::GetCheckSum( LPSTR lpText, UINT nStartPos, UINT nEndPos )
{
	BYTE result;
	LPSTR ptr;
	UINT i;

	ptr = lpText;

	result = *(ptr + nStartPos);
	for(i = nStartPos + 1; i < nEndPos + 1 ; i++)
		result = result ^ *(ptr + i);

	return result;
}


/** **********************************************************
*	@brief		MakeSendFormat
*	@param	BYTE byCommand		
*	@param	BYTE* pbyOutBuff	SendData Format 결과
*	@param	BYTE* pbyData		Data 정보(LED 정보 등)
*	@param	int nDataLen	Data 길이(전문 전체 길이 - 전문 Header(5(STX+LEN+Command+ETX+BCC)))
*	@retval	없음.
************************************************************/
BYTE* CDEV_DIO::MakeSendFormat(BYTE byCommand, BYTE* pbyData, int nDataLen)
{
	int nIndex = 0;
	static BYTE byOutputBuff[1024] = {0,};

	memset(byOutputBuff, 0, sizeof(byOutputBuff));

	byOutputBuff[nIndex++] = 0x02;							// STX
	byOutputBuff[nIndex++] = (BYTE)(nDataLen+1);			// LEN(Command ~ Data)
	byOutputBuff[nIndex++] = byCommand;					// COMMAND
	if(nDataLen > 0)
	{
		memcpy(byOutputBuff+nIndex, pbyData, nDataLen);	// Data
		nIndex += nDataLen;
	}
	byOutputBuff[nIndex++] = 0x03;							// ETX
	byOutputBuff[nIndex++] = GetCheckSum((LPSTR)byOutputBuff, 1, nDataLen+3); //BCC(LEN ~ ETX)

	return byOutputBuff;
}
