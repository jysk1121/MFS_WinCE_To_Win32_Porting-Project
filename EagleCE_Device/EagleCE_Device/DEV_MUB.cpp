#include "stdafx.h"
#include "DEV_Define.h"
#include "DeviceSimUtil.h"
#include "DEV_MUB.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "DEV_Manager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_MUB::CDEV_MUB()
{
	// DLL 경로 설정
	CString	strPath = _T("");

	// API 취득
	m_hDll			= LoadLibrary(EAGLE_ATM_MUB_DLL);

	Dll_Open		= (DLL_OPEN)		GetProcAddress(m_hDll, _T("MBU_Open"));
	Dll_Close		= (DLL_CLOSE)		GetProcAddress(m_hDll, _T("MBU_Close"));
	Dll_GetVersion	= (DLL_GETVERSION)	GetProcAddress(m_hDll, _T("MBU_GetVersion"));
	Dll_Reset		= (DLL_RESET)		GetProcAddress(m_hDll, _T("MBU_Reset"));
	Dll_Send_Data	= (DLL_SEND_DATA)	GetProcAddress(m_hDll, _T("MBU_SendData"));
	Dll_Read_Data	= (DLL_READ_DATA)	GetProcAddress(m_hDll, _T("MBU_ReadData"));

	m_bCommError = FALSE;
}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_MUB::~CDEV_MUB()
{
	if (m_hDll)
	{
		FreeLibrary(m_hDll);

		m_hDll = NULL;
	}
}


/** **********************************************************
*	@brief		Is DLL Loaded
*	@retval		없음
************************************************************/
BOOL CDEV_MUB::IsDllLoaded(void)
{
#if	EMULATION_MUB_DEVICE
	return TRUE;
#endif

	return (NULL != m_hDll);
}


/** **********************************************************
*	@brief		열기
*	@retval		없음
************************************************************/
BOOL CDEV_MUB::Open(HWND hWnd, int nPortNum)
{
#if	EMULATION_MUB_DEVICE
	// Control/action command: confirm via OK/NG dialog per the device simulator policy.
	return CDevSim::Confirm(_T("MUB"), _T("MUB_Open"));
#endif

	if (NULL == Dll_Open)
	{
		LOG(Error, _T("[CDEV_MUB] Dll_Open is NULL."));
		return FALSE;
	}

	if (FALSE == Dll_Open(nPortNum))
	{
		LOG(Error, _T("[CDEV_MUB] Dll_Open is failed."));
		return FALSE;
	}

	////Start LED/Sensor thread
	//CDEV_Manager::GetInstance()->m_DEV_DIO.m_hWnd = hWnd;

	//CDEV_Manager::GetInstance()->m_DEV_DIO.StartDIOThread();

	return TRUE;
}


/** **********************************************************
*	@brief		닫기
*	@retval		없음
************************************************************/
BOOL CDEV_MUB::Close()
{
#if	EMULATION_MUB_DEVICE
	return CDevSim::Confirm(_T("MUB"), _T("MUB_Close"));
#endif

	if (NULL == Dll_Close)
	{
		return FALSE;
	}

	return Dll_Close();
}


/** **********************************************************
*	@brief		버전 취득
*	@retval		없음
************************************************************/
BOOL CDEV_MUB::GetVersion(LPSTR szVersion)
{
#if	EMULATION_MUB_DEVICE
	// Inquiry/status command: auto-answered from DeviceSim.ini, no dialog.
	// NOTE: the original stub assigned to the local pointer (szVersion = "0000"),
	// which never reached the caller's buffer. Copy into the caller buffer instead.
	if (szVersion != NULL)
	{
		CString strVer = CDevSim::ReadString(_T("MUB"), _T("Version"), _T("0000"));
		CT2A szVer(strVer);
		strcpy(szVersion, szVer.m_psz);
	}

	return TRUE;
#endif

	BOOL bResult = FALSE;

	if (NULL == Dll_GetVersion)
	{
		return FALSE;
	}

	m_MUBLock.Lock();

	bResult = Dll_GetVersion(szVersion);

	m_MUBLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		
*	@retval		없음
************************************************************/
BOOL CDEV_MUB::Reset()
{
#if	EMULATION_MUB_DEVICE
	return CDevSim::Confirm(_T("MUB"), _T("MUB_Reset"));
#endif

	if (NULL == Dll_Reset)
	{
		return FALSE;
	}

	BOOL bResult = FALSE;

	m_MUBLock.Lock();

	bResult = Dll_Reset();

	m_MUBLock.Unlock();

	return bResult;
}


/** **********************************************************
*	@brief		
*	@retval		없음
************************************************************/
BOOL CDEV_MUB::SendDeviceData(BYTE byID, BYTE* pData, int nLen)
{
#if	EMULATION_MUB_DEVICE
	return TRUE;
#endif

	if (NULL == Dll_Send_Data)
	{
		return FALSE;
	}

	return Dll_Send_Data(byID, pData, nLen);
}

/** **********************************************************
*	@brief		
*	@retval		없음
************************************************************/
int CDEV_MUB::ReadDeviceData(BYTE byID, BYTE* pData)
{
#if	EMULATION_MUB_DEVICE
	return TRUE;
#endif

	if (NULL == Dll_Read_Data)
	{
		return FALSE;
	}

	return Dll_Read_Data(byID, pData);
}

//////////////////////////////////////////////////////////////////////////
// Device in MUB Control START
//////////////////////////////////////////////////////////////////////////
// for SIU Command & LED Define
//#define	SIU_VERSION				0x56 //'V'
//#define	SIU_SENSOR				0x53 //'S'
//#define	SIU_FLICKER				0x4C //'L'
//
//#define SIU_NUM_PTR				1
//#define SIU_NUM_CDR				2
//#define SIU_NUM_CDM				3
//#define SIU_NUM_EPP				4
// 
///** **********************************************************
//*	@brief	    
//*	@retval		TRUE = Success, FALSE = Fail
//************************************************************/
//BOOL CDEV_MUB::MUB_DIO_GetVersion( LPSTR lpFWVer, LPSTR lpDLLVer )
//{
//	int nRet = 0, nRecvLen = 0;
//	char temp[32] = {0,};
//
//	m_MUBLock.Lock();
//
//	memset(m_MubSendBuff, 0, sizeof(m_MubSendBuff));
//	memset(m_MubRecvBuff, 0, sizeof(m_MubRecvBuff));
//
//	m_MubSendBuff[0] = 0x02;		//STX
//	m_MubSendBuff[1] = 0x01;		//LEN
//	m_MubSendBuff[2] = SIU_VERSION; //COMMAND
//	m_MubSendBuff[3] = 0x03;		//ETX
//	m_MubSendBuff[4] = GetCheckSum((LPSTR)m_MubSendBuff, 1, 3); //BCC
//
//	nRet = HandShakeDeviceCommand(ID_SIU, m_MubSendBuff, m_MubRecvBuff, 5, &nRecvLen);
//
//	sprintf(temp, "V%c%c.%c%c", m_MubRecvBuff[3], m_MubRecvBuff[4], m_MubRecvBuff[5], m_MubRecvBuff[6]); 
//	memcpy(lpFWVer, temp, 7);
//	memcpy(lpDLLVer, "V01.01", 7);
//
//	m_MUBLock.Unlock();
//
//	return nRet;
//}
//
///** **********************************************************
//*	@brief	    
//*	@retval		TRUE = Success, FALSE = Fail
//************************************************************/
//BOOL CDEV_MUB::MUB_DIO_SetLed( BYTE byLED_1, BYTE byLED_2 )
//{
//	int nRet = 0, nRecvLen = 0;
//
//	m_MUBLock.Lock();
//
//	memset(m_MubSendBuff, 0, sizeof(m_MubSendBuff));
//	memset(m_MubRecvBuff, 0, sizeof(m_MubRecvBuff));
//
//	m_MubSendBuff[0] = 0x02;			//STX
//	m_MubSendBuff[1] = 0x03;			//LEN
//	m_MubSendBuff[2] = SIU_FLICKER;		//COMMAND
//	m_MubSendBuff[3] = byLED_1;			//LED 1
//	m_MubSendBuff[4] = byLED_2;			//LED 2
//	m_MubSendBuff[5] = 0x03;			//ETX
//	m_MubSendBuff[6] = GetCheckSum((LPSTR)m_MubSendBuff, 1, 5); //BCC
//
//	nRet = HandShakeDeviceCommand(ID_SIU, m_MubSendBuff, m_MubRecvBuff, 7, &nRecvLen);
//
//	m_MUBLock.Unlock();
//
//	return nRet;
//}
//
///** **********************************************************
//*	@brief	    
//*	@retval		TRUE = Success, FALSE = Fail
//************************************************************/
//BOOL CDEV_MUB::MUB_DIO_GetSensor( char* pStatus )
//{
//	int nRet = 0, nRecvLen = 0;
//
//	m_MUBLock.Lock();
//
//	memset(m_MubSendBuff, 0, sizeof(m_MubSendBuff));
//	memset(m_MubRecvBuff, 0, sizeof(m_MubRecvBuff));
//
//	m_MubSendBuff[0] = 0x02;		//STX
//	m_MubSendBuff[1] = 0x01;		//LEN
//	m_MubSendBuff[2] = SIU_SENSOR; //COMMAND
//	m_MubSendBuff[3] = 0x03;		//ETX
//	m_MubSendBuff[4] = GetCheckSum((LPSTR)m_MubSendBuff, 1, 3); //BCC
//
//	nRet = HandShakeDeviceCommand(ID_SIU, m_MubSendBuff, m_MubRecvBuff, 5, &nRecvLen);
//
//	memcpy(pStatus, &m_MubRecvBuff[3], 1);
//
//	m_MUBLock.Unlock();
//
//	return nRet;
//}


//////////////////////////////////////////////////////////////////////////
// Device in MUB Control END
//////////////////////////////////////////////////////////////////////////

/** **********************************************************
*	@brief	    Communicate with MUB
*	@retval		TRUE = Success, FALSE = Fail
************************************************************/
BOOL CDEV_MUB::HandShakeDeviceCommand( BYTE byID, BYTE* pSendData, BYTE* pRecvData, int nLen, int* nOutLen )
{
	CString strText, strDisp;
	int nRet = 0;

	m_MUBLock.Lock();

	m_bCommError = FALSE;

	nRet = SendDeviceData(byID, pSendData, nLen);

	if (!nRet)
	{
//		m_bCommError = TRUE;
//		return FALSE;
		goto ErrorReturn;
	}

	//////////////////////////////////////////////////////////////////////////
	//2. Recv ACK from Device
	BYTE RecvBuff[256] = {0,};

	nRet = ReadDeviceData(byID, RecvBuff);

	// Data를 확인하지 않고 무조건 Ack를 받았다고 인식하네... 쩝...
	if (!nRet)
	{
//		m_bCommError = TRUE;
//		return FALSE;
		goto ErrorReturn;
	}

	//////////////////////////////////////////////////////////////////////////
	//3. Send ENQ to Device
	BYTE byENQ[2] = {0x05, 0x00};

	nRet = SendDeviceData(byID, (BYTE*)byENQ, 1);

	if (!nRet)
	{
//		m_bCommError = TRUE;
//		return FALSE;
		goto ErrorReturn;
	}

	//////////////////////////////////////////////////////////////////////////
	//4. Recv Response from Device

	memset(RecvBuff, 0, sizeof(RecvBuff));
	nRet = ReadDeviceData(byID, RecvBuff);

	if (nRet)
	{
		// Save response data
		memcpy(pRecvData, RecvBuff, __min(nRet, 256));
	}
	else
	{
//		m_bCommError = TRUE;
//		return FALSE;
		goto ErrorReturn;
	}

	//////////////////////////////////////////////////////////////////////////
	//5. Send ACK to Device
	BYTE byACK[2] = {0x06, 0x00};

	nRet = SendDeviceData(byID, (BYTE*)byACK, 1);

	if (!nRet)
	{
//		m_bCommError = TRUE;
//		return FALSE;
		goto ErrorReturn;
	}

	m_MUBLock.Unlock();

	return TRUE;

ErrorReturn:

	m_bCommError = TRUE;
	m_MUBLock.Unlock();

	return FALSE;
}

///** **********************************************************
//*	@brief		
//*	@retval		없음
//************************************************************/
//BYTE CDEV_MUB::GetCheckSum( LPSTR lpText, UINT nStartPos, UINT nEndPos )
//{
//	BYTE result;
//	LPSTR ptr;
//	UINT i;
//
//	ptr = lpText;
//
//	result = *(ptr + nStartPos);
//	for(i = nStartPos + 1; i < nEndPos + 1 ; i++)
//		result = result ^ *(ptr + i);
//
//	return result;
//}
