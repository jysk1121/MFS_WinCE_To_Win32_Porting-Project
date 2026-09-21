#include "stdafx.h"

#include <atlstr.h>
#include <vector>

#include "CommCdm.h"
#include <stdio.h>
//#include "Log.h"

//#include "IniFile.h"
#include "../../../EagleCE_Framework/EagleCE_Framework/IniFile.h"

//#include "EagleLogger.h"
#include "../../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

//#include "DES.h"
#include "../../../EagleCE_Framework/EagleCE_Framework/DES.h"

//CLog m_Trace;

// timeout second
#define TO_SEC_NORMAL		(10)
#define TO_SEC_LONG			(120)
#define TO_SEC_60			(60)

#define DLL_VER_MAJOR		2
//#define DLL_VER_MINOR		10
//#define DLL_VER_MINOR		11 // Test Dispense (Dispense into a Reject bin)
//#define DLL_VER_MINOR		12 // FW download 기능 추가 (CheckFirmware)
//#define DLL_VER_MINOR		13 // 2017.03.30 GetStatus Error 추가(CheckFirmware)
//#define DLL_VER_MINOR		14 // 2017.04.07 Warning Screen 추가(CheckFirmware)
//#define DLL_VER_MINOR		15 // 2017.06.09 Command Log 위치 수정(App Path + \\Trace)
//#define DLL_VER_MINOR		16 // 2018.03.05 암호화 추가, RejectLog 추가
//#define DLL_VER_MINOR		17 // 2018.03.12 Dispense Log 추가
//#define DLL_VER_MINOR		18 // 2018.03.15 WDM Firmware Download 추가
//#define DLL_VER_MINOR		19 // 2018.03.22 WDM CE부 인증 추가
//#define DLL_VER_MINOR		20 // 2018.03.27 방출 중 통신장애 시 처리 추가
#define DLL_VER_MINOR		21 // 2019.01.18 인증 키 불일치일 경우, AP Open시 통신장애로 보고 되는 버그 대응

// control character(CC) definition
const BYTE CCommCdm::CC_STX = 0x02;
const BYTE CCommCdm::CC_ETX = 0x03;
const BYTE CCommCdm::CC_ENQ = 0x05;
const BYTE CCommCdm::CC_ACK = 0x06;
const BYTE CCommCdm::CC_NAK = 0x15;

// command(CMD) defintion
const BYTE CCommCdm::CMD_RESET = 0x30;
const BYTE CCommCdm::CMD_STATUS = 0x31;
const BYTE CCommCdm::CMD_DIAGNOSTIC = 0x32;
const BYTE CCommCdm::CMD_DISPENSE = 0x33;
const BYTE CCommCdm::CMD_LASTSTATUS = 0x34;
const BYTE CCommCdm::CMD_CONFIG = 0x35;
const BYTE CCommCdm::CMD_SET_THICK = 0x36;
const BYTE CCommCdm::CMD_GET_THICK = 0x37;
const BYTE CCommCdm::CMD_SET_SIZE = 0x38;
const BYTE CCommCdm::CMD_GET_SIZE = 0x39;
const BYTE CCommCdm::CMD_MULTI_DISPENSE = 0x3A;
const BYTE CCommCdm::CMD_TEST_DISPENSE = 0x3C;
const BYTE CCommCdm::CMD_LEARN = 0x40;
const BYTE CCommCdm::CMD_GET_REJECTLOG = 0x43;
const BYTE CCommCdm::CMD_GET_RANDOM_NUMBER = 0x60;
const BYTE CCommCdm::CMD_CE_CERTIFICATION = 0x61;
const BYTE CCommCdm::CMD_EXIST_CE_INFO = 0x62;
const BYTE CCommCdm::CMD_GET_DISPENSELOG = 0x78;

// V02.12
#define	CMD_DOWNLOADSTART			0x70
#define	CMD_DOWNLOADHEADERSEND		0x71
#define	CMD_DOWNLOADDATASEND		0x72
#define	CMD_DOWNLOADEND				0x73
//

/** **********************************************************
*	@brief	ERROR CODE
************************************************************/
#define CERTIFICATION_ERROR				0x0B
#define COMMUNICATION_ERROR				0xF0
#define SEND_RECV_CMD_MISMATCH_ERROR	0xF1
#define COUNT_MISMATCH_ERROR			0xF2
#define FIRMWARE_DOWNLOAD_FAIL			0xF3


#define DOWNLOAD_HEADER_SIZE	32
#define DOWNLOAD_SEND_SIZE		240

#define DATA_PADING_SIZE	8

#define COMM_BUFF_SIZE		4096

// * time check interval
const UINT PTM_INTERVAL = 30;

BYTE CDM_KEY_TABLE[] = {
	0x72, 0x35, 0x6B, 0x43, 0x16, 0x0D, 0x75, 0x0A,
	0x18, 0x67, 0x56, 0x3F, 0x39, 0x28, 0x2A, 0x34,
	0x05, 0x78, 0x3D, 0x24, 0x63, 0x4A, 0x50, 0x15,
	0x27, 0x20, 0x44, 0x51, 0x0E, 0x2B, 0x31, 0x69,
	0x7A, 0x14, 0x3C, 0x52, 0x71, 0x58, 0x6E, 0x4D,
	0x73, 0x7D, 0x77, 0x19, 0x60, 0x0B, 0x07, 0x7B,
	0x48, 0x5A, 0x47, 0x46, 0x2D, 0x22, 0x61, 0x53,
	0x5B, 0x1C, 0x3E, 0x55, 0x41, 0x42, 0x29, 0x57,
	0x5E, 0x06, 0x54, 0x1E, 0x64, 0x1B, 0x36, 0x02,
	0x1A, 0x03, 0x6D, 0x5C, 0x32, 0x3A, 0x04, 0x45,
	0x40, 0x2F, 0x1D, 0x4F, 0x38, 0x23, 0x5F, 0x37,
	0x66, 0x25, 0x33, 0x7F, 0x21, 0x6F, 0x4C, 0x12,
	0x6A, 0x09, 0x11, 0x30, 0x79, 0x0F, 0x49, 0x7C,
	0x74, 0x7E, 0x17, 0x70, 0x5D, 0x4E, 0x3B, 0x65,
	0x10, 0x08, 0x62, 0x76, 0x59, 0x13, 0x26, 0x68,
	0x2C, 0x6C, 0x4B, 0x01, 0x1F, 0x2E, 0x0C
};

int g_nKeyTableSize = sizeof(CDM_KEY_TABLE);

typedef enum
{
	WRITE_LEVEL_0 = 0,
	WRITE_LEVEL_1,		// 송/수신 전문 로그
	WRITE_LEVEL_2,		// 추후 사용
	WRITE_LEVEL_3,		// 추후 사용
	WRITE_LEVEL_4,		// 추후 사용
} WRITE_LOG_LEVEL;


CCommCdm* CCommCdm::m_pInstance = NULL;

CCommCdm::CCommCdm()
{
	BYTE byTempKey[16] = { 0x36, 0x44, 0x45, 0x38, 0x42, 0x34, 0x43, 0x32, 0x41, 0x42, 0x31, 0x46, 0x35, 0x39, 0x30, 0x44 };

	m_bOpen = FALSE;
	m_bDownloadMode = FALSE;
	m_nPortNum = 0;
	m_hWarning = NULL;
	memset(m_byPreStatus, 0, sizeof(m_byPreStatus));
	memset(m_byRandomNumber, 0x00, sizeof(m_byRandomNumber));
	m_byOrgCommand = 0;
	m_bSendDispense = FALSE;
	m_nWriteLogLevel = FALSE;

	// 로거 생성
	CEagleLogger::CreateInstance();
	CEagleLogger::GetInstance()->Initialize(_T("EagleCE_CDM"), 30, 1024 * 1024);

	memset(m_byUniqueKey, 0x00, sizeof(m_byUniqueKey));
	memset(m_TempDesKey, 0x00, sizeof(m_TempDesKey));
	memcpy(m_TempDesKey, byTempKey, sizeof(byTempKey));

	m_bCEInfoError = FALSE;
	m_byRecvCommand = 0;
}

CCommCdm::~CCommCdm()
{
	CEagleLogger::GetInstance()->Finalize();
	CEagleLogger::ReleaseInstance();
}

/************************************************************
*	@brief		인스턴스 생성
*	@retval		없음
************************************************************/
void CCommCdm::CreateInstance()
{
	m_pInstance = new CCommCdm();
}


/************************************************************
*	@brief		인스턴스 취득
*	@retval		없음
************************************************************/
CCommCdm* CCommCdm::GetInstance()
{
	return m_pInstance;
}


/************************************************************
*	@brief		인스턴스 해제
*	@retval		없음
************************************************************/
void CCommCdm::ReleaseInstance()
{
	delete m_pInstance;
}


BOOL CCommCdm::OpenPort(HWND hWnd, UINT nPortNum, int nWriteLogLevel)
{
	TCHAR szPortNum[10];

	DCB dcb;
	COMMTIMEOUTS timeouts;

	m_nWriteLogLevel = nWriteLogLevel;

	// --------------------------------
	//  * SERIAL PORT SETTING PARAMS *
	//
	//   - BAUD RATE = 9600
	//   - BYTE SIZE = 8
	//   - STOP BIT = ONESTOPBIT
	//   - PARITY = NOPARITY
	// --------------------------------
	memset(szPortNum, 0x00, sizeof(szPortNum));
	wsprintf(szPortNum, _T("COM%d:"), nPortNum);

	// keep window handle
	m_hWnd = hWnd;
	m_nPortNum = nPortNum;

	// overlapped structure 변수 초기화.
	m_osRead.Offset = 0;
	m_osRead.OffsetHigh = 0;

	//--> Read 이벤트 생성에 실패..
	if ( !(m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) ) 	
	{
		LOG(Error, _T("Read Event Create Failed"));
		return FALSE;
	}


	m_osWrite.Offset = 0;
	m_osWrite.OffsetHigh = 0;
	//--> Write 이벤트 생성에 실패..
	if (! (m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
	{
		LOG(Error, _T("Write Event Create Failed"));
		return FALSE;
	}

	// open serial port
	m_hPort = CreateFile(szPortNum, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(m_hPort == INVALID_HANDLE_VALUE) {
		LOG(Error, _T("Serial Port CreateFile Failed"));
		return FALSE;
	}

	// get comm port status 
	if(!GetCommState(m_hPort, &dcb)){
		LOG(Error, _T("GetCommState Failed"));
		return FALSE;
	}

	// change dcb
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;

	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;

	// set comm port status
	if(!SetCommState(m_hPort, &dcb)){
		LOG(Error, _T("GetCommState Failed"));
		return FALSE;
	}

	// set timeout
	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;
//	timeouts.ReadTotalTimeoutConstant = 30 * 1000;	//Timeout = 100ms
	timeouts.ReadTotalTimeoutConstant = 5 * 1000;	//Timeout = 5000ms
	timeouts.WriteTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	SetCommTimeouts(m_hPort, &timeouts);
	
	// set buffer size
	SetupComm(m_hPort, COMM_BUFF_SIZE, COMM_BUFF_SIZE);	

	// clears in/out buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	//! essential : starting by communicating with CDM
	EscapeCommFunction(m_hPort, CLRRTS);

	EscapeCommFunction(m_hPort, CLRDTR);

	// 수신 Thread 처리
	m_bOpen = TRUE;

	return TRUE;
}

BOOL CCommCdm::ClosePort()
{
	m_bOpen = FALSE;

	SetCommMask(m_hPort, 0);

	// 통신 큐 초기화
	PurgeComm(m_hPort, PURGE_TXABORT | PURGE_RXABORT | 
					   PURGE_TXCLEAR | PURGE_RXCLEAR );

	CloseHandle(m_hPort);

	return TRUE;
}

void CCommCdm::SetTimeout(UINT nSecond)
{
	m_dwTimeout = nSecond * 1000; //milli* 1000
}

BOOL CCommCdm::ReadPort(BYTE *lpBuf, DWORD nByteToRead)
{
	BYTE* ptr;
	DWORD nLeft, nRead;

	INT time_cnt = 0;
	DWORD dwStartTime = GetTickCount();

	ptr = (BYTE*)lpBuf;
	nLeft = nByteToRead;
	nRead = 0;

	while(nLeft > 0) {
		// read port
		if(!ReadFile(m_hPort, ptr, nLeft, &nRead, 0))
			return FALSE;

		if(nRead > 0)
		{
			// Data 수신
			ptr += nRead;
			nLeft -= nRead;
		}

		// Check Timeout
		if( (GetTickCount() - dwStartTime) > m_dwTimeout )
		{
			return FALSE;
		}

		Sleep(10);
		
	}

	return TRUE;
}

BOOL CCommCdm::WritePort(LPCSTR lpBuf, DWORD nByteToWrite)
{
	LPSTR ptr;
	DWORD nLeft, nWrite;

	ptr = (LPSTR)lpBuf;
	nLeft = nByteToWrite;
	nWrite = 0;

	if(!WriteFile(m_hPort, ptr, nLeft, &nWrite, 0))
	{
		return FALSE;
	}

	return TRUE;
}

BYTE CCommCdm::GetCheckSum(BYTE* lpText, UINT nStartPos, UINT nEndPos)
{
	BYTE result;
	BYTE* ptr;
	UINT i;

	ptr = lpText;

	result = *(ptr + nStartPos);
	for(i = nStartPos + 1; i < nEndPos + 1 ; i++)
		result = result ^ *(ptr + i);

	return result;
}

// Dispense명령에서 WritePort장애 발생시를 구분하기 위해 return값 변경.
// FW에서 LastDispense 결과값을 clear하는 조건이 Dispense 명령을 받았을 경우라고 함.
// 때문에 Dispense CMD 전송이 실패한 경우에는 LastDispense 명령을 보내면 안됨.
//BOOL CCommCdm::HandShake()
int CCommCdm::HandShake()
{
	CDMRXTYPE rxType;

	BYTE rxBuf[1024] = {0,};
	INT nTry = 0;
	BYTE *ptrSend;
	UINT nBCCEndPos = 0;
	BYTE byRealKeyIndex = 0;
	int nSendDataLen = 0;
	CDMCMDTRACE cmdOrgTrace;
	int i = 0;
	BYTE byDesKey[16] = { 0, };
	CString strTempData;
	CString strTemp, strTemp2;

	memset(&cmdOrgTrace, 0x00, sizeof(CDMCMDTRACE));
	memcpy(&cmdOrgTrace, &m_cmdTrace, sizeof(CDMCMDTRACE));
	
	m_byOrgCommand = m_cmdTrace.txCommand[2];

	// 송신 전문 로그
	if(WRITE_LEVEL_1 == m_nWriteLogLevel)
	{
		if(CMD_STATUS != m_byOrgCommand)
		{
			for(int i=0; i<m_cmdTrace.lenCommand; i++)
			{
				strTemp.Format(_T("%02x "), m_cmdTrace.txCommand[i]);
				strTemp2 += strTemp;
			}

			LOG(Info, _T("CMD:[%02x], SendData : [%s]"), m_byOrgCommand, strTemp2);
		}
	}
	//////////////////////////////////////////

	// 암호화 전문으로 재구성
	{
		if ( (CMD_CE_CERTIFICATION == m_byOrgCommand) || (CMD_EXIST_CE_INFO == m_byOrgCommand) )
		{
			memcpy(byDesKey, m_TempDesKey, sizeof(byDesKey));
		}
		else
		{
			strTempData.Format(_T("%S"), m_byUniqueKey);
			ConvertStringToHex(strTempData, byDesKey);
		}

		EncryptKeyData(byDesKey);
	}

	// 송신 전문 로그
	if(WRITE_LEVEL_1 == m_nWriteLogLevel)
	{
		strTemp.Empty();
		strTemp2.Empty();
		if(CMD_STATUS != m_byOrgCommand)
		{
			for(int i=0; i<m_cmdTrace.lenCommand; i++)
			{
				strTemp.Format(_T("%02x "), m_cmdTrace.txCommand[i]);
				strTemp2 += strTemp;
			}

			LOG(Info, _T("CMD:[%02x], EncryptData : [%s]"), m_byOrgCommand, strTemp2);
		}
	}
	//////////////////////////////////////////

	// COMMAND 전송
	if(!WritePort((LPCSTR)m_cmdTrace.txCommand, m_cmdTrace.lenCommand))
	{
		LOG(Error, _T("[1st - Send] Command Send Error"))
		return FAILED_TO_SEND_COMMAND;
	}
	
	// ACK 대기
	rxType = GetAction(rxBuf);

	switch(rxType)
	{
	case RX_ACK:
		// 정상 수신
		break;

	case RX_NAK:
		// retry는 있으나 다시 Read를 하지 않는 Bug가 있음, Error 처리로 변경
		LOG(Error, _T("[1st - Response] NAK Condition - Error Return"));
		return FAILED_TO_RECEIVE_ACK;

	default:
		// 예외 상황 처리
		if (CMD_STATUS != m_byOrgCommand)
		{
			LOG(Error, _T("[1st - Response] Invalid Condition - Error Return (%d)"), rxType);
		}
		return FAILED_TO_RECEIVE_ACK;
	}

	if( (CMD_MULTI_DISPENSE == m_byOrgCommand) || (CMD_TEST_DISPENSE == m_byOrgCommand) )
	{
		m_bSendDispense = TRUE;
	}

	// 방출 중 power off 시, 정보 세팅
	if(CMD_MULTI_DISPENSE == m_byOrgCommand)
	{
		SaveLastTransInfoData(_T("IsSendDispense"), _T("1"));
	}

	// ENQ 전송
	if(!WritePort((LPCSTR)&CC_ENQ, 1)) 
	{
		LOG(Error, _T("ENQ Send Error"));	
		return FAILED_TO_SEND_ENQ;
	}

	// RESPONSE 대기
	rxType = GetAction(rxBuf);

	switch(rxType)
	{
	case RX_RESPONSE:
		// 정상 수신
		break;

	default:
		if (CMD_STATUS != m_byOrgCommand)
		{
			LOG(Error, _T("[2nd - Response] Invalid Condition (%d)"), rxType);
		}
		return FAILED_TO_RECEIVE_REPLY;
	}

	// Ack 송신
	if(!WritePort((LPCSTR)&CC_ACK, 1)) 
	{
		LOG(Error, _T("Ack Send Error"));
		return FAILED_TO_SEND_ACK;
	}

	return NO_ERROR;
}

// --------------------------------------------------
//	  수신된 데이터 저장 및 해당 데이터 타입 리턴
// --------------------------------------------------
CDMRXTYPE CCommCdm::GetAction(BYTE* lpBuf)
{
	BYTE* ptr;
	BYTE ch;
	INT lenData = 0;

	BOOL bAfterSTX = FALSE;
	INT lenText, lenResponse;
	BYTE byBCC = 0;
	CString strTempData;
	BYTE byDesKey[16] = { 0, };
	CString strTemp, strTemp2;
	BYTE byCheckSumData4Key[8] = { 0, };
	
	ptr = lpBuf;
	lenText = lenResponse = 0;
	
	// -----------------------
	//     첫 데이터 읽음
	// -----------------------
	if(!ReadPort(ptr, 1))
	{
		if(CMD_STATUS != m_byOrgCommand)
		{
			LOG(Error, _T("1st ReadPort failed. CMD : [0x%02x]"), m_byOrgCommand);
		}

		return RX_ERROR;
	}
	
	ch = *ptr;

	if(ch == CC_ACK){			// ACK 문자 확인
		// V02.12
		if (m_byOrgCommand == CMD_DOWNLOADSTART)	// m_cmdTrace.txCommand에는 암호화 된 data가 있음.
		{
			m_bDownloadMode = TRUE;
		}

		return RX_ACK;
	}
	else if(ch == CC_NAK){		// NAK 문자 확인
		return RX_NAK;
	}
	else if(ch == CC_STX){		// 시작 문자가 STX
		ptr++; lenResponse++;

		// Length를 읽음
		if(!ReadPort(ptr, 1))
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("2nd ReadPort failed."));
			}

			return RX_ERROR;
		}

		// 응답 전문의 최대 길이는 256 byte이기 때문에
		// Length는 max 252byte까지만 받아야 함. STX + LEN + Data(252byte) + ETX + BCC = 256byte
		if(*ptr > MAX_RECV_DATA_LENGTH)
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("Data Length is over MAX."));
			}

			return RX_ERROR;
		}

		lenData = *ptr;
		ptr++; lenResponse++;

		// Body Reading : Response Code ~ Data
		if(!ReadPort(ptr, lenData))
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("3rd ReadPort failed."));
			}

			return RX_ERROR;
		}

		ptr += lenData; lenResponse += lenData;

		// ETX + CSum Reading 
		if(!ReadPort(ptr, 2))
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("4th ReadPort failed."));
			}

			return RX_ERROR;
		}

		lenResponse += 2;

		// 수신 전문 로그
		if(WRITE_LEVEL_1 == m_nWriteLogLevel)
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				for(int i=0; i<lenResponse; i++)
				{
					strTemp.Format(_T("%02x "), lpBuf[i]);
					strTemp2 += strTemp;
				}

				LOG(Info, _T("CMD:[%02x], RecvData : [%s]"), m_byOrgCommand, strTemp2);
			}
		}

		// ATM Mode가 아닌 경우 대응 - 최소 T-Length 정보가 4BYTE 이상(Key(1) + Key CheckSum(2) + Data Len(1))이어야함. (아닌 경우 평문으로 판단하여 에러 처리 추가)
		// CE부 인증 키 불일치 장애가 발생할 경우, Data Length는 1이기 때문에 인증 장애는 제외함.
		if ( (lpBuf[2] != CERTIFICATION_ERROR) && (lpBuf[1] <= 4) )
		{
			if (CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("Data Length is a small. (%d)"), lenData);
			}

			return RX_ERROR;
		}

		//////////////////////////////////////////
		// Calculate BCC
		byBCC = GetCheckSum(lpBuf, 1, lenResponse-2);

		ch = *ptr;
		if(ch != CC_ETX)
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("ETX error"));
			}

			return RX_ERROR;
		}

		if(byBCC != *(ptr+1))
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("BCC error"));
			}

			return RX_ERROR;
		}
		
		// -------------------------------------------------
		//   Trace에 Copy함
		// -------------------------------------------------
		memcpy(m_cmdTrace.rxResponse, lpBuf, lenResponse);
		m_cmdTrace.lenResponse = lenResponse;

		if(CERTIFICATION_ERROR == lpBuf[2])	// T-DES 인증키 불일치시에는 0x0B를 받는다.
		{
			if(CMD_STATUS != m_byOrgCommand)
			{
				LOG(Error, _T("System Error1. lpBuf[2] : [0x%02X]"), lpBuf[2]);
			}

			m_bCEInfoError = TRUE;

			return RX_RESPONSE;
		}

		// 암호화 전문 복호화
	//	ExcuteDecryptData();
		if ((CMD_CE_CERTIFICATION == m_byOrgCommand) || (CMD_EXIST_CE_INFO == m_byOrgCommand))
		{
			memcpy(byDesKey, m_TempDesKey, sizeof(byDesKey));
		}
		else
		{
			strTempData.Format(_T("%S"), m_byUniqueKey);
			ConvertStringToHex(strTempData, byDesKey);
		}

		if ((CMD_CE_CERTIFICATION != m_byOrgCommand) && (CMD_EXIST_CE_INFO != m_byOrgCommand))
		{
			GetCheckSumForCryptoKey(byCheckSumData4Key, byDesKey);

			// Encrypt Send Data : STX + LI + KEY + CheckSum(2byte) + T-DES DATA(LI + CMD + DATA) + ETX + BCC
			if( (lpBuf[3] != byCheckSumData4Key[0]) || (lpBuf[4] != byCheckSumData4Key[1]) )
			{
				if(CMD_STATUS != m_byOrgCommand)
				{
					LOG(Error, _T("System Error2. lpBuf[3]:[0x%02X], lpBuf[4]:[0x%02X], data1:[0x%02X], data2:[0x%02X]"), lpBuf[3], lpBuf[4], byCheckSumData4Key[0], byCheckSumData4Key[1]);
				}
				m_bCEInfoError = TRUE;

				return RX_RESPONSE;
			}
		}

		DecryptKeyData(byDesKey);
		///////////////////////////////////

		// 수신 전문 로그
		if(WRITE_LEVEL_1 == m_nWriteLogLevel)
		{
			strTemp.Empty();
			strTemp2.Empty();
			if(CMD_STATUS != m_byOrgCommand)
			{
				for(int i=0; i<m_cmdTrace.lenResponse; i++)
				{
					strTemp.Format(_T("%02x "), m_cmdTrace.rxResponse[i]);
					strTemp2 += strTemp;
				}

				LOG(Info, _T("CMD:[%02x], DecryptData : [%s]"), m_byOrgCommand, strTemp2);
			}
		}
		//////////////////////////////////////////

		// V2.15
		if (m_byOrgCommand == CMD_STATUS)
		{
			if (memcmp(m_byPreStatus, &m_cmdTrace.rxResponse[3], 2) != 0)
			{
				memcpy(m_byPreStatus, &m_cmdTrace.rxResponse[3], 2);
			}
		}

		return RX_RESPONSE;
	}
	else
	{
		if(CMD_STATUS != m_byOrgCommand)
		{
			LOG(Info, _T("Receive is unknwon!!! ch : [%02X]"), ch);
		}
	}
	
	return RX_NONE;

}


void CCommCdm::AnalyzeGeneralCommand(BYTE cCommand, LPVOID lpParam)
{
	// -----------------------------------------------------------------------------------------
	//   Response Command Format : 
	//
	//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERR_CD(3) + REJECT_CD(4) + ST1(5) ~ STATUS 7(11)
	//		+ Dispensed Count(12)
	// -----------------------------------------------------------------------------------------
	BYTE* ptr;
	UINT i;

	ptr = m_cmdTrace.rxResponse;

	// V2.15 delete
	//m_Trace.WriteHexTrace((unsigned char*)m_cmdTrace.rxResponse,m_cmdTrace.lenResponse,RECV_DUMP,  0 );	

	if(cCommand == CMD_STATUS){
		LPCDMSTATUS lpStatus = (LPCDMSTATUS)lpParam;
		
		lpStatus->error_cd = *(ptr + 3);
		lpStatus->reject_cd = *(ptr + 4);

//		for(i=0; i<7; i++){
		for(i=0; i<8; i++){  // for f/w 1.40
			lpStatus->sensor[i]  = *(ptr + 5 + i);
		}

	}
	else if((cCommand == CMD_MULTI_DISPENSE) || (cCommand == CMD_TEST_DISPENSE)) {
		LPCDMMULTIDISPENSE lpResult = (LPCDMMULTIDISPENSE)lpParam;
		
		lpResult->error_cd = *(ptr + 3);
		lpResult->reject_cd = *(ptr + 4);

//		for(i=0; i<7; i++){
		for(i=0; i<8; i++){  // for f/w 1.40
			lpResult->sensor[i] = *(ptr + 5 + i);
		}

		for(i=0; i<MAX_CASSETTE; i++){
//			lpResult->count[i] = *(ptr + 12 + i);
			lpResult->count[i] = *(ptr + 13 + i);
		}

	}
	else if(cCommand == CMD_DIAGNOSTIC){
		LPCDMDIAGNOSTIC lpResult = (LPCDMDIAGNOSTIC)lpParam;

		lpResult->error_cd = *(ptr + 3);
		lpResult->reject_cd = *(ptr + 4);

//		for(i=0; i<7; i++){
		for(i=0; i<8; i++){  // for f/w 1.40
			lpResult->sensor[i] = *(ptr + 5 + i);
		}

		memcpy(lpResult->result_msg, m_cmdTrace.rxText, m_cmdTrace.lenText);
		lpResult->result_msg[m_cmdTrace.lenText] = '\0';
	}
	else if(cCommand == CMD_LEARN){
		// -------------------------------------------------------------------------------------------
		//   Response of LEARN BILL PARAM Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERROR CODE(3) + REJECT CODE(4) + DATA(2 BYTES) + ETX + BCC
		//	   Data :
		//			DATA1(5) - BILL SIZE AVERAGE
		//			DATA2(6) - BILL THICKNESS AVERAGE
		// -------------------------------------------------------------------------------------------
		LPCDMLEARN lpResult = (LPCDMLEARN)lpParam;

		lpResult->error_cd = *(ptr + 3);
		lpResult->reject_cd = *(ptr + 4);
		lpResult->size_avg = *(ptr + 5);
		lpResult->thickness_avg = *(ptr + 6);
	}
	else if (cCommand == CMD_GET_RANDOM_NUMBER)
	{
		// -------------------------------------------------------------------------------------------
		//   Response of GET RANDOM NUMBER PARAM Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + DATA(2 BYTES) + ETX + BCC
		//	   Data :
		//			DATA1(3) - Random Number1
		//			DATA2(4) - Random Number2
		// -------------------------------------------------------------------------------------------

		m_byRandomNumber[0] = *(ptr + 3);
		m_byRandomNumber[1] = *(ptr + 4);
	}
	else if(cCommand == CMD_GET_REJECTLOG)
	{
		// -------------------------------------------------------------------------------------------
		//   Response of GET REJECTLOG PARAM Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + DATA(MAX 246 BYTES) + ETX + BCC
		// -------------------------------------------------------------------------------------------
		LPCDM_REJECT_INFO lpResult = (LPCDM_REJECT_INFO)lpParam;
		int nDataLen =  *(ptr + 1);

		memset(lpResult->byRejectInfo, 0x00, sizeof(lpResult->byRejectInfo));
		memcpy(lpResult->byRejectInfo, ptr+3, nDataLen - 1);	// nDataLen : CMD(1byte) + Data(n byte)

		lpResult->byLogLen = (BYTE)(nDataLen - 1);
	}
	else if( (cCommand == CMD_DOWNLOADSTART) ||
			 (cCommand == CMD_DOWNLOADHEADERSEND) || 
			 (cCommand == CMD_DOWNLOADDATASEND) ||
			 (cCommand == CMD_DOWNLOADEND) )
	{
		// -------------------------------------------------------------------------------------------
		//   Response of DOWNLOAD PARAM Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERRORCODE(3) + ETX + BCC
		// -------------------------------------------------------------------------------------------
		LPBYTE lpErrorCode = (LPBYTE)lpParam;

		*lpErrorCode = *(ptr + 3);
	}
	else if(cCommand == CMD_CE_CERTIFICATION)
	{
		// -------------------------------------------------------------------------------------------
		//   Response of CE CERTIFICATION PARAM Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERROR CODE(3) + ETX + BCC
		// -------------------------------------------------------------------------------------------
		LPBYTE lpbyErrorCd = (LPBYTE)lpParam;

		*lpbyErrorCd = *(ptr +3);
	}
	else if(cCommand == CMD_EXIST_CE_INFO)
	{
		// -------------------------------------------------------------------------------------------
		//   Response of GET RANDOM NUMBER PARAM Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + DATA(3) + ETX + BCC
		//	   Data :
		//			DATA1(3) - CE Certification Set(0x30)/No Set(0x31)
		// -------------------------------------------------------------------------------------------
		LPBYTE lpbyExist = (LPBYTE)lpParam;

		*lpbyExist = *(ptr + 3);
	}
}

void CCommCdm::AnalyzeE2PROMCommand(BYTE cCommand, LPVOID lpParam)
{
	BYTE* ptr;

	ptr = m_cmdTrace.rxResponse;

	//m_Trace.WriteHexTrace((unsigned char*)m_cmdTrace.rxResponse,m_cmdTrace.lenResponse,RECV_DUMP,  0 );	

	if(cCommand == CMD_LASTSTATUS){
		// -------------------------------------------------------------------------------------------
		//   Response of LAST STATUS Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + DATA(15 BYTES) + ETX + BCC
		//     Data : 
		//			DATA 1(3) - Last Command, DATA 2(4) - Last Error, DATA 3(5) - Last Cassette
		//			DATA  4,  5,   6,  7,  8,  9 (6,  7,  8,   9, 10, 11) - Last Dispense Request Count of Cass 1 ~ 4
		//			DATA 10, 11,  12, 13, 14, 15 (12, 13, 14, 15, 16, 17) - Last Reject Count of Cass 1 ~ 4
		//			DATA 16, 17,  18, 19, 20, 21 (18, 19, 20, 21, 22, 23) - Last Dispensed Count of Cass 1 ~ 4
		// -------------------------------------------------------------------------------------------
		LPCDMLASTDISPENSE lpResult = (LPCDMLASTDISPENSE)lpParam;

		lpResult->last_cmd = *(ptr + 3);
		lpResult->last_error_cd = *(ptr + 4);
		lpResult->last_cbx = *(ptr + 5);

		for(UINT i=0; i<MAX_CASSETTE; i++){
			lpResult->last_dispense_count[i] = *(ptr + 6 + i);
			lpResult->last_divert_count[i] = *(ptr + 12 + i);
			lpResult->last_pick_count[i] = *(ptr + 18 + i);
		}

	

	}
	else if(cCommand == CMD_CONFIG){
		// -------------------------------------------------------------------------------------------
		//   Response of CONFIG Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + DATA(4 BYTES) + ETX + BCC
		//     Data : 
		//			DATA1(3) - HIGH Version, DATA2(4) - LOW Version
		//          DATA3(5) - _____xx CDM Type (단),	DATA4(6) - Reserve
		// -------------------------------------------------------------------------------------------
		LPCDMCFGSTATUS lpCfgStatus = (LPCDMCFGSTATUS)lpParam;

		lpCfgStatus->major_no = *(ptr + 3);
		lpCfgStatus->minor_no = *(ptr + 4);
		lpCfgStatus->cbx_type = *(ptr + 5);
	}
	else if(cCommand == CMD_GET_THICK){
		// -------------------------------------------------------------------------------------------
		//   Response of GETTHICK Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERROR CODE(3) + DATA(2 BYTES) + ETX + BCC
		//     Data : 
		//			DATA1(4) - Bill Thickness
		//          DATA2(5) - Margin
		// -------------------------------------------------------------------------------------------
		LPCDMGETBILLTHICKNESS lpGet = (LPCDMGETBILLTHICKNESS)lpParam;

		lpGet->error_cd = *(ptr + 3);
		lpGet->thickness = *(ptr + 4);
		lpGet->margin = *(ptr + 5);
	}
	else if(cCommand == CMD_SET_THICK){
		// -------------------------------------------------------------------------------------------
		//   Response of SETTHICK Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERROR CODE(3) + ETX + BCC
		// -------------------------------------------------------------------------------------------
		LPBYTE lpbyErrorCd = (LPBYTE)lpParam;

		*lpbyErrorCd = *(ptr + 3);
	}
	else if(cCommand == CMD_GET_SIZE){
		// -------------------------------------------------------------------------------------------
		//   Response of GETSIZE Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERROR CODE(3) + DATA(2 BYTES) + ETX + BCC
		//     Data : 
		//			DATA1(4) - Bill Size
		//          DATA2(5) - Margin
		// -------------------------------------------------------------------------------------------
		LPCDMGETBILLSIZE lpGet = (LPCDMGETBILLSIZE)lpParam;

		lpGet->error_cd = *(ptr +3);
		lpGet->size = *(ptr + 4);
		lpGet->margin = *(ptr + 5);
	}
	else if(cCommand == CMD_SET_SIZE){
		// -------------------------------------------------------------------------------------------
		//   Response of SETSIZE Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERROR CODE(3) + ETX + BCC
		// -------------------------------------------------------------------------------------------
		LPBYTE lpbyErrorCd = (LPBYTE)lpParam;

		*lpbyErrorCd = *(ptr +3);
	}
	else if(cCommand == CMD_LEARN){
		// -------------------------------------------------------------------------------------------
		//   Response of LEARN BILL PARAM Command Format : 
		//
		//     STX(0) + LEN(1) + RESPONSE_CD(2) + ERROR CODE(3) + REJECT CODE(4) + DATA(2 BYTES) + ETX + BCC
		//	   Data :
		//			DATA1(5) - BILL SIZE AVERAGE
		//			DATA2(6) - BILL THICKNESS AVERAGE
		// -------------------------------------------------------------------------------------------

		m_cmdLearnResult.error = *(ptr + 3);
		m_cmdLearnResult.reject = *(ptr + 4);
		m_cmdLearnResult.avg_size = *(ptr + 5);
		m_cmdLearnResult.avg_thickness = *(ptr + 6);
	}
}

BOOL CCommCdm::Reset()
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("Reset() System Error!!!"));
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// ------------------------------
	//  STX + SIZE + CMD + ETX + BCC	
	// ------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_RESET;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_60);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	nResult = HandShake();

	if(TRUE == m_bCEInfoError)
		return FALSE;

	if(NO_ERROR == nResult)
		return TRUE;
	else
		return FALSE;
}

BOOL CCommCdm::CfgStatus(LPCDMCFGSTATUS lpCfgStatus, LPBYTE lpbyErrorCd)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("CfgStatus() System Error!!!"));
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));	

	// -------------------------------
	// STX + SIZE + CMD + ETX + BCC
	// -------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_CONFIG;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeE2PROMCommand(CMD_CONFIG, (LPVOID)lpCfgStatus);

	return TRUE;
}

BOOL CCommCdm::Status(LPCDMSTATUS lpStatus)
{
	//BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		lpStatus->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// -------------------------------
	//  STX + SIZE + CMD + ETX + BCC
	// -------------------------------
	m_cmdTrace.lenCommand = 5;

	int nTxBufIdx = 0;
	m_cmdTrace.txCommand[nTxBufIdx++] = CC_STX;
	m_cmdTrace.txCommand[nTxBufIdx++] = 0x01;
	m_cmdTrace.txCommand[nTxBufIdx++] = CMD_STATUS;
	m_cmdTrace.txCommand[nTxBufIdx++] = CC_ETX;
	m_cmdTrace.txCommand[nTxBufIdx++] = GetCheckSum(m_cmdTrace.txCommand, 1, 3);


	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);
	
	// clears in/out buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	nResult = HandShake();

	if(NO_ERROR != nResult)
	{
		lpStatus->error_cd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		lpStatus->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		lpStatus->error_cd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_STATUS, lpStatus);
	if(lpStatus->error_cd != 0x30)
		return FALSE;
	else
		return TRUE;
}

// 
int CCommCdm::MultiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult, LPBYTE lpbyCEInfo)
{
	BYTE* ptr;
	INT i;
	int nResult = -1;
	BYTE byErrorCode = 0;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("MultiDispense() System Error!!!"));
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return WFS_HARDWARE_ERROR;
	}

	m_bSendDispense = FALSE;
	
	GetRandomNumber(lpbyCEInfo);		// RN 정보 취득

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// ---------------------------------------------------
	//  STX + SIZE + CMD + Data(6 Bytes) + ETX + BCC
	// ---------------------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 13;	// RN(2byte) 추가
		
	*ptr++ = CC_STX;
	*ptr++ = 0x09;
	*ptr++ = CMD_MULTI_DISPENSE;

	for(i=0; i<MAX_CASSETTE; i++){
		*ptr++ = lpCbxItem->count[i];
	}

	*ptr++ = m_byRandomNumber[0];
	*ptr++ = m_byRandomNumber[1];
	
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 11);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_LONG);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	SaveLastTransInfoData(_T("IsSendDispense"), _T("0"));	// 방출 중 power off 설정 초기화

	nResult = HandShake();

	SaveLastTransInfoData(_T("IsSendDispense"), _T("0"));	// 방출 중 power off 설정 초기화

	if(NO_ERROR != nResult)
	{
		lpResult->error_cd = COMMUNICATION_ERROR;	// Communication error
		return nResult;
	}

	if(TRUE == m_bCEInfoError)
	{
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return WFS_HARDWARE_ERROR;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		lpResult->error_cd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return WFS_HARDWARE_ERROR;
	}

	AnalyzeGeneralCommand(CMD_MULTI_DISPENSE, lpResult);
	
	if (lpResult->error_cd != 0x30)
		return WFS_HARDWARE_ERROR;

// Added by smlee
	for(i = 0; i < MAX_CASSETTE; i++)
	{
		if (lpCbxItem->count[i] != lpResult->count[i])
		{
			lpResult->error_cd = COUNT_MISMATCH_ERROR;	//Miss count error
			return WFS_HARDWARE_ERROR;
		}
	}
	
	return NO_ERROR;
}

BOOL CCommCdm::LastDispense(LPCDMLASTDISPENSE lpResult, LPBYTE lpbyErrorCd)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("LastDispense() System Error!!!"));
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	//-----------------------------------
	//   STX + SIZE + CMD + ETX + BCC
	//-----------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_LASTSTATUS;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	
//	if(!HandShake())
	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeE2PROMCommand(CMD_LASTSTATUS, lpResult);
	
	return TRUE;
}

BOOL CCommCdm::Diagnostic(LPCDMDIAGNOSTIC lpResult)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("Diagnostic() System Error!!!"));
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// -------------------------------------------
	//		STX + SIZE + CMD + ETX + BCC
	// -------------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_DIAGNOSTIC;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_LONG);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

//	if(!HandShake())
	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		lpResult->error_cd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		lpResult->error_cd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_DIAGNOSTIC, lpResult);

	if(lpResult->error_cd != 0x30)
		return FALSE;
	else
		return TRUE;
}

BOOL CCommCdm::SetBillThickness(LPCDMSETBILLTHICKNESS lpSet, LPBYTE lpbyErrorCd)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("SetBillThickness() System Error!!!"));
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));
	
	// -------------------------------------------------------------
	// STX + SIZE + CMD + DATA(C# + ThickValue + Margin) + ETX + BCC
	// -------------------------------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 8;

	*ptr++ = CC_STX;
	*ptr++ = 0x04;
	*ptr++ = CMD_SET_THICK;
	*ptr++ = lpSet->cbx_no;
	*ptr++ = lpSet->thickness;
	*ptr++ = lpSet->margin;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 6);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

//	if(!HandShake())
	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeE2PROMCommand(CMD_SET_THICK, lpbyErrorCd);
	if(*lpbyErrorCd != 0x30)
		return FALSE;
	else
		return TRUE;
	
}


BOOL CCommCdm::GetBillThickness(BYTE byCbxNo, LPCDMGETBILLTHICKNESS lpGet)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("GetBillThickness() System Error!!!"));
		lpGet->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// ----------------------------------------
	// STX + SIZE + CMD + DATA(C#) + ETX + BCC
	// ----------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 6;

	*ptr++ = CC_STX;
	*ptr++ = 0x02;
	*ptr++ = CMD_GET_THICK;
	*ptr++ = byCbxNo;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 4);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		lpGet->error_cd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		lpGet->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		lpGet->error_cd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeE2PROMCommand(CMD_GET_THICK, lpGet);
	if(lpGet->error_cd != 0x30)
		return FALSE;
	else 
		return TRUE;
}

BOOL CCommCdm::SetBillSize(LPCDMSETBILLSIZE lpSet, LPBYTE lpbyErrorCd)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("SetBillSize() System Error!!!"));
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// -------------------------------------------------------
	// STX + SIZE + CMD + DATA(C# + Size + Margin) + ETX + BCC
	// -------------------------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 8;

	*ptr++ = CC_STX;
	*ptr++ = 0x04;
	*ptr++ = CMD_SET_SIZE;
	*ptr++ = lpSet->cbx_no;
	*ptr++ = lpSet->size;
	*ptr++ = lpSet->margin;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 6);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

//	if(!HandShake())
	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeE2PROMCommand(CMD_SET_SIZE, lpbyErrorCd);
	if(*lpbyErrorCd != 0x30)
		return FALSE;

	return TRUE;
}

BOOL CCommCdm::GetBillSize(BYTE byCbxNo, LPCDMGETBILLSIZE lpGet)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("GetBillSize() System Error!!!"));
		lpGet->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// -----------------------------------------
	//  STX + SIZE + CMD + DATA(C#) + ETX + BCC
	// -----------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 6;

	*ptr++ = CC_STX;
	*ptr++ = 0x02;
	*ptr++ = CMD_GET_SIZE;
	*ptr++ = byCbxNo;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 4);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		lpGet->error_cd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		lpGet->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		lpGet->error_cd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeE2PROMCommand(CMD_GET_SIZE, lpGet);

	if(lpGet->error_cd != 0x30)
		return FALSE;
	else 
		return TRUE;
}

BOOL CCommCdm::Learn(BYTE byCbxNo, BYTE byItemCount, LPCDMLEARN lpResult, LPBYTE lpbyCEInfo)
{
	BYTE* ptr;
	int nResult = -1;
	BYTE byErrorCode = 0;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("Learn() System Error!!!"));
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}
	
	GetRandomNumber(lpbyCEInfo);		// RN 정보 취득

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// --------------------------------------------------------
	//  STX + SIZE + CMD + Data(cassette# + bill#) + ETX + BCC
	// --------------------------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 9;	// RN(2byte) 추가
		
	*ptr++ = CC_STX;
	*ptr++ = 0x05;
	*ptr++ = CMD_LEARN;
	*ptr++ = byCbxNo;
	*ptr++ = byItemCount;
	*ptr++ = m_byRandomNumber[0];
	*ptr++ = m_byRandomNumber[1];
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 7);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_LONG);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		lpResult->error_cd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		lpResult->error_cd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_LEARN, (LPVOID)lpResult);

	if(lpResult->error_cd != 0x30)
		return FALSE;
	else
		return TRUE;
}

// 2016.05.24 shyoo : Dispense to Reject box
int CCommCdm::TestiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult, LPBYTE lpbyCEInfo)
{
	BYTE* ptr;
	INT i;
	int nResult = -1;
	BYTE byErrorCode = 0;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("TestiDispense() System Error!!!"));
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return WFS_HARDWARE_ERROR;
	}

	m_bSendDispense = FALSE;
	
	GetRandomNumber(lpbyCEInfo);		// RN 정보 취득

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// ---------------------------------------------------
	//  STX + SIZE + CMD + Data(6 Bytes) + ETX + BCC
	// ---------------------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 13;	// RN(2byte) 추가

	*ptr++ = CC_STX;
	*ptr++ = 0x09;
	*ptr++ = CMD_TEST_DISPENSE;

	for (i = 0; i<MAX_CASSETTE; i++) {
		*ptr++ = lpCbxItem->count[i];
	}

	*ptr++ = m_byRandomNumber[0];
	*ptr++ = m_byRandomNumber[1];

	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 11);


	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_LONG);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

//	if (!HandShake())
	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		lpResult->error_cd = COMMUNICATION_ERROR;	// Communication error
		return nResult;
	}

	if(TRUE == m_bCEInfoError)
	{
		lpResult->error_cd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return WFS_HARDWARE_ERROR;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		lpResult->error_cd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return WFS_HARDWARE_ERROR;
	}

	AnalyzeGeneralCommand(CMD_TEST_DISPENSE, lpResult);

	if (lpResult->error_cd != 0x30)
		return WFS_HARDWARE_ERROR;

	// Added by smlee
	for (i = 0; i < MAX_CASSETTE; i++)
	{
		if (lpCbxItem->count[i] != lpResult->count[i])
		{
			lpResult->error_cd = COUNT_MISMATCH_ERROR;	//Miss count error
			return WFS_HARDWARE_ERROR;
		}
	}

	return NO_ERROR;
}

BOOL CCommCdm::GetDLLversion(LPBYTE major, LPBYTE minor)
{
	*major = DLL_VER_MAJOR;
	*minor = DLL_VER_MINOR;
	return TRUE;
}

//
#define MFSCDM_FW_FILENAME	_T("WDMAP") //ex) WDMAP_V0033.BIN
#define FW_FILE_NAME_SIZE	5
#define FW_FILE_VER_LEN		4

BOOL CCommCdm::UpdateFirmware(LPCTSTR szPath, LPBYTE lpbyErrorCd, LPBYTE lpbyCEInfo)
{
	BYTE* ptr;
	CFile binFile;
	unsigned char filereaddata[1] = { 0, }, filedata[256] = { 0, };
	int sendlength = 0;
	HANDLE	hFile;

	CDMCFGSTATUS CfgVer;
	CString strVer = _T("");
	CString strHeaderVer = _T("");

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("UpdateFirmware() System Error!!!"));
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&CfgVer, 0, sizeof(CDMCFGSTATUS));
	if (!CfgStatus(&CfgVer, lpbyErrorCd))
	{
		LOG(Error, _T("read version is failed."));	// read version fail
		return FALSE;
	}

	strVer.Format(_T("%02d%02d"), CfgVer.major_no, CfgVer.minor_no);
	LOG(Info, _T("Current Version : [%s]"), strVer);

	//0. File(Firmware Bin) check
	if (szPath == NULL)
	{
		LOG(Error, _T("szPath is NULL"));	// Path error
		return TRUE;
	}

	// Find firmware files
	WIN32_FIND_DATA FindFile;
	CString strPath = _T("");

	strPath.Format(_T("%s\\*.bin"), szPath);
	LOG(Info, _T("strPath : [%s]"), strPath);

	memset(FindFile.cFileName, 0, sizeof(FindFile.cFileName));
	HANDLE hFileFind = FindFirstFile(strPath, &FindFile);
	if (INVALID_HANDLE_VALUE == hFileFind)
	{
		LOG(Error, _T("FindFirstFile is failed."));	// invalid path
		return TRUE;
	}

	CString strFile;
	BOOL bRet = TRUE;
	int nNameLeng = 0;
	DWORD dwFileAttr = 0;
	std::vector<CString> strFWFile;	//디렉토리에 있는 Firmware 파일 이름들

	while (bRet)
	{
		nNameLeng = wcslen(FindFile.cFileName);
		dwFileAttr = FindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		if ((dwFileAttr == 0) && (nNameLeng >= (FW_FILE_NAME_SIZE + 10)))
		{
			if (wcsncmp(FindFile.cFileName, MFSCDM_FW_FILENAME, FW_FILE_NAME_SIZE) == 0)
			{
				strFile.Format(_T("%s"), FindFile.cFileName);
				strFWFile.push_back(strFile);
			}
		}
		memset(FindFile.cFileName, 0, sizeof(FindFile.cFileName));
		bRet = FindNextFile(hFileFind, &FindFile);
	}
	FindClose(hFileFind);

	if (strFWFile.size() < 1)
	{
		LOG(Info, _T("Firmware file not exist."));	// No FW File
			return TRUE;
	}

	int i = 0;
	CString strVer1, strVer2;			//Firmware 파일들 중 가장 높은 버전을 찾기 위해 변수
	CString strLast = strFWFile[0];		//처음 파일을 가장 높은 버전으로 설정
	if (strFWFile.size() >= 2)
	{
		for (i = 1; i < (int)strFWFile.size(); i++)	//버전 비교하여 최고 버전을 찾는다.
		{
			strVer1 = strLast.Mid(FW_FILE_NAME_SIZE + 2, FW_FILE_VER_LEN);
			strVer2 = strFWFile[i].Mid(FW_FILE_NAME_SIZE + 2, FW_FILE_VER_LEN);
			if (strVer1.CompareNoCase(strVer2) < 0)
				strLast = strFWFile[i];
		}
	}

	//1. Version Check
	CString strFilename = strLast;
	strFilename = strLast.Mid(FW_FILE_NAME_SIZE + 2, FW_FILE_VER_LEN);	//="0033"

	if ((strFilename.CompareNoCase(strVer)) <= 0)
	{
		LOG(Info, _T("firamware version is higher or same than bin version. FW ver:[%s], Bin ver:[%s]"), strVer, strFilename);	// Same Version
		return TRUE;
	}

	strLast.Format(_T("%s\\%s"), szPath, strLast);	// Debug: 16.10.27
	LOG(Info, _T("BinFile path : [%s]"), strLast);

	if(TRUE == GetFWVersionInHeader(strLast, strHeaderVer))
	{
		if ((strFilename.CompareNoCase(strHeaderVer)) != 0)
		{
			LOG(Info, _T("Version is different Bin Version between Header Version. Bin ver:[%s], Header ver:[%s]"), strFilename, strHeaderVer);
			return TRUE;
		}
	}
	else
	{
		LOG(Error, _T("GetFWVersionInHeader is failed."));
		*lpbyErrorCd = FIRMWARE_DOWNLOAD_FAIL;
		return FALSE;
	}

	LOG(Info, _T("FW version:[%s], Bin Version:[%s], Header Version:[%s]"), strVer, strFilename, strHeaderVer);

	// File open check
	if (FALSE == binFile.Open(strLast, CFile::modeRead | CFile::typeBinary))
	{
		LOG(Error, _T("file open is failed."));	// File open error
		*lpbyErrorCd = FIRMWARE_DOWNLOAD_FAIL;
		return FALSE;
	}

	DWORD dwLength = binFile.GetLength();
	DWORD dwFileIndex = 0;
	BYTE* lpData = NULL;
	DWORD dwRead = 0;
	BYTE byErrorCode = 0;

	lpData = new BYTE[dwLength+1];

	binFile.Close();

	hFile = ::CreateFile(strLast, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		LOG(Error, _T("CreateFile is failed."));	// File open error

		if(lpData != NULL)
			delete[] lpData;

		*lpbyErrorCd = FIRMWARE_DOWNLOAD_FAIL;
		return FALSE;
	}

	if (FALSE == ReadFile(hFile, lpData, dwLength, &dwRead, NULL))
	{
		LOG(Error, _T("ReadFile is failed."));	// File open error

		if(lpData != NULL)
			delete[] lpData;

		::CloseHandle(hFile);

		*lpbyErrorCd = FIRMWARE_DOWNLOAD_FAIL;
		return FALSE;
	}

	::CloseHandle(hFile);

	//2. DOWNLOADSTART
	// ---------------------------------------------------
	//  STX + SIZE + CMD + ETX + BCC
	// ---------------------------------------------------
	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_DOWNLOADSTART;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	SetTimeout(TO_SEC_NORMAL);
	PurgeComm(m_hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	int nResult = -1;
	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		LOG(Error, _T("HandShake is failed."));

		if(lpData != NULL)
			delete[] lpData;

		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
	
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		if(lpData != NULL)
			delete[] lpData;

		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		if(lpData != NULL)
			delete[] lpData;

		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_DOWNLOADSTART, lpbyErrorCd);
	if(*lpbyErrorCd != 0x30)
	{
		if(lpData != NULL)
			delete[] lpData;

		return FALSE;
	}

	LOG(Info, _T("CMD_DOWNLOADSTART success."));

	//3. DOWNLOADHEADERSEND
	memset(&filedata, 0x00, sizeof(filedata));

	filedata[sendlength++] = CMD_DOWNLOADHEADERSEND;

	for(i=0; i<DOWNLOAD_HEADER_SIZE; i++)
		filedata[sendlength++] = lpData[i];

	dwFileIndex += DOWNLOAD_HEADER_SIZE;

	if (FALSE == CommandSend(filedata, sendlength))
	{
		LOG(Error, _T("DOWNLOADHEADERSEND is failed."));

		if(lpData != NULL)
			delete[] lpData;

		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error

		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		if(lpData != NULL)
			delete[] lpData;

		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		if(lpData != NULL)
			delete[] lpData;

		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_DOWNLOADHEADERSEND, lpbyErrorCd);
	if(*lpbyErrorCd != 0x30)
	{
		if(lpData != NULL)
			delete[] lpData;

		return FALSE;
	}

	LOG(Info, _T("CMD_DOWNLOADHEADERSEND success."));

	//4. DOWNLOADDATASEND
	memset(&filedata, 0x00, sizeof(filedata));

	sendlength = 0;
	filedata[sendlength++] = CMD_DOWNLOADDATASEND;

	while(dwFileIndex < dwLength)
	{
		// 240byte단위로 전송
		for(i=0; i<DOWNLOAD_SEND_SIZE; i++)
		{
			if((dwLength - dwFileIndex) < DOWNLOAD_SEND_SIZE)
			{
				if(i < (dwLength - dwFileIndex))
					filedata[sendlength++] = lpData[i+dwFileIndex];
				else
					break;
			}
			else
				filedata[sendlength++] = lpData[i+dwFileIndex];
		}

		if (FALSE == CommandSend(filedata, sendlength))
		{
			LOG(Error, _T("DOWNLOADDATASEND is failed."));	// CMD Error(DownloadDataSend)
			
			if(lpData != NULL)
				delete[] lpData;

			*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error

			return FALSE;
		}
		else
		{
			if(TRUE == m_bCEInfoError)
			{
				if(lpData != NULL)
					delete[] lpData;

				*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
				return FALSE;
			}

			if(m_byOrgCommand != m_byRecvCommand)
			{
				if(lpData != NULL)
					delete[] lpData;

				*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
				return FALSE;
			}

			sendlength = 0;
			memset(&filedata, 0x00, sizeof(filedata));
			filedata[sendlength++] = CMD_DOWNLOADDATASEND;
			dwFileIndex += DOWNLOAD_SEND_SIZE;

			AnalyzeGeneralCommand(CMD_DOWNLOADDATASEND, lpbyErrorCd);
			if(*lpbyErrorCd != 0x30)
			{
				if(lpData != NULL)
					delete[] lpData;

				return FALSE;
			}
		}
	}

	LOG(Info, _T("CMD_DOWNLOADDATASEND success."));

	if(lpData != NULL)
		delete[] lpData;

	//5. DOWNLOADEND
	SetTimeout(TO_SEC_60);
	sendlength = 0;
	filedata[sendlength++] = CMD_DOWNLOADEND;
	if (FALSE == CommandSend(filedata, sendlength))
	{
		LOG(Error, _T("DOWNLOADEND is failed."));	// CMD Error(DownloadEnd)
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error

		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_DOWNLOADEND, lpbyErrorCd);
	if(*lpbyErrorCd != 0x30)
	{
		return FALSE;
	}

	LOG(Info, _T("CMD_DOWNLOADEND success"));

	m_bDownloadMode = FALSE;	// restore download mode

	LOG(Info, _T("Wait FW Reset Time.(30 second)"));
	Sleep(30 * 1000);	// Wait reboot

	//7. Resualt Check
	memset(&CfgVer, 0, sizeof(CDMCFGSTATUS));

	if (!CfgStatus(&CfgVer, lpbyErrorCd))
	{
		LOG(Error, _T("read version is failed after download."));

		return FALSE;
	}

	strVer.Format(_T("%02d%02d"), CfgVer.major_no, CfgVer.minor_no);
	LOG(Info, _T("Changed Version : [%s]"), strVer);

	if ((strFilename.CompareNoCase(strVer)) != 0)	// Version
	{
		LOG(Error, _T("Download success. but version is different."));
		*lpbyErrorCd = FIRMWARE_DOWNLOAD_FAIL;

		return FALSE;
	}

	// Success !!!
	return TRUE;
}

// Reserved
BOOL CCommCdm::CheckFirmware(LPSTR szPath, LPBYTE lpbyErrorCd)
{
	*lpbyErrorCd = 0x00;
	return TRUE;
}

BOOL CCommCdm::CommandSend(BYTE * ptr, BYTE length)
{
	BYTE sendbuf[256] = { 0, }, bcc = 0x00;
	BYTE byOrgBuf[256] = {0,};
	DWORD	x = 0, i;
	BYTE *ptrSend;
	UINT nBCCEndPos = 0;
	BYTE byRealKeyIndex = 0;
	int nSendDataLen = 0;
	BYTE byDesKey[16] = { 0, };
	CString strTempData;
	CString strTemp, strTemp2;

	// ptr = cmd + data

	bcc = CC_ETX;
	sendbuf[x++] = CC_STX;
	sendbuf[x++] = length;
	bcc ^= length;
	for (i = 0; i < length; i++) {
		sendbuf[x] = *ptr;
		bcc ^= sendbuf[x++];
		ptr++;
	}
	sendbuf[x++] = CC_ETX;
	sendbuf[x++] = bcc;

	m_byOrgCommand = sendbuf[2];
	
///////////////////////////////////////////////////////////////////////////////
// Send Data
///////////////////////////////////////////////////////////////////////////////
	CDMRXTYPE rxType;

	BYTE rxBuf[1000] = {0,};
	INT nTry = 0;

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));
	memcpy(m_cmdTrace.txCommand, sendbuf, x);
	m_cmdTrace.lenCommand = x;

	// 암호화 전문으로 재구성
	{
		strTempData.Format(_T("%S"), m_byUniqueKey);
		ConvertStringToHex(strTempData, byDesKey);
		
		EncryptKeyData(byDesKey);
	}

	// COMMAND 전송
	if (!WritePort((LPCSTR)m_cmdTrace.txCommand, m_cmdTrace.lenCommand))
	{
		return FALSE;
	}

	// ACK 대기
	rxType = GetAction(rxBuf);

	switch(rxType)
	{
	case RX_ACK:
		// 정상 수신
		break;

	case RX_NAK:
		LOG(Error, _T("NAK Condition - Error return"));
		return FALSE;

	default:
		LOG(Error, _T("Invalid Condition - Error return"));
		return FALSE;
	}

	// ENQ 전송
	if (!WritePort((LPCSTR)&CC_ENQ, 1))
		return FALSE;
	
	// RESPONSE 대기
	rxType = GetAction(rxBuf);

	switch(rxType)
	{
	case RX_RESPONSE:
		// 정상 수신
		break;

	default:
		LOG(Error, _T("[2nd] Invalid Condition - Error Return"));
		return FALSE;
	}

	// Ack 송신
	if (!WritePort((LPCSTR)&CC_ACK, 1))
		return FALSE;

	return TRUE;
}


/** ********************************************************************
* @brief Reject Log 취득
* @param LPCDM_REJECT_INFO lpRejectLog	 RejectLog 구조체
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CCommCdm::GetRejectLog(LPCDM_REJECT_INFO lpRejectLog, LPBYTE lpbyErrorCd)
{
	BYTE* ptr;
	int nResult = -1;

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		LOG(Error, _T("GetRejectLog() System Error!!!"));
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// ------------------------------
	//  STX + SIZE + CMD + ETX + BCC	
	// ------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_GET_REJECTLOG;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_LONG); //V1.40: TO_SEC_NORMAL(10) -> TO_SEC_LONG(120)

	// clears inout buffer
	PurgeComm(m_hPort,
		PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_GET_REJECTLOG, lpRejectLog);


	return TRUE;
}


/** ********************************************************************
* @brief Random Number 취득
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CCommCdm::GetRandomNumber(LPBYTE lpbyCEInfo)
{
	BYTE* ptr;
	int nResult = -1;
	BYTE byErrorCode = 0;

	if(TRUE == m_bCEInfoError)
	{
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));
	memset(m_byRandomNumber, 0x00, sizeof(m_byRandomNumber));

	// ------------------------------
	//  STX + SIZE + CMD + ETX + BCC	
	// ------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_GET_RANDOM_NUMBER;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_60);

	// clears inout buffer
	PurgeComm(m_hPort,
		PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
		return FALSE;

	if(TRUE == m_bCEInfoError)
	{
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_GET_RANDOM_NUMBER, NULL);


	return TRUE;
}


// /** ********************************************************************
// * @brief 생성된 Key와 전문으로 암호화 수행
// * @param BYTE byData	 전문 data
// * @param int nKeyIndex	 생성된 Key Index
// * @retval BYTE	암호화 된 data
// ************************************************************************/
// BYTE CCommCdm::ExcuteKeyTable(BYTE byData, int nKeyIndex)
// {
// 	BYTE byKeyValue = 0;
// 
// 	byKeyValue = byData ^ CDM_KEY_TABLE[nKeyIndex-1];	// KeyIndex가 1 ~ 127이기 때문에 -1을 해줘야 함.
// 	byKeyValue = byKeyValue ^ 0xAA;
// 
// 	return byKeyValue;
// }
// 
// 
// /** ********************************************************************
// * @brief 암호화 key index 생성
// * @param BYTE &byRealKeyIndex	 암호화에 사용될 key index
// * @retval BYTE	암호화 된 key index
// ************************************************************************/
// BYTE CCommCdm::GetEncryptKeyIndex(BYTE &byRealKeyIndex)
// {
// 	BYTE byIndex = 0;
// 
// 	srand((unsigned)GetTickCount());
// 
// 	byIndex = (BYTE)((rand() % g_nKeyTableSize) + 1);	// KeyIndex는 1 ~ 127
// 	byRealKeyIndex = byIndex;
// 
// 	byIndex = byIndex << 1;
// 	byIndex += 1;
// 
// 	Sleep(10);
// 
// 	return byIndex;
// }
// 
// 
// /** ********************************************************************
// * @brief 수신받은 암호화된 key index를 복호화 함.
// * @param BYTE byEncrytKeyIndex	 firmware로부터 받은 암호화 된 key index
// * @retval BYTE	data를 복호화 할 key index
// ************************************************************************/
// BYTE CCommCdm::GetDecryptKeyIndex(BYTE byEncrytKeyIndex)
// {
// 	BYTE byIndex = 0;
// 
// 	byIndex = byEncrytKeyIndex - 1;
// 	byIndex = byIndex >> 1;
// 
// 	return byIndex;
// }
// 
// 
// /** ********************************************************************
// * @brief 암호화 된 data 복호화
// * @retval 없음.
// ************************************************************************/
// void CCommCdm::ExcuteDecryptData()
// {
// 	CDMCMDTRACE cmdOrgTrace;
// 	BYTE byRealKeyIndex = 0;
// 	int nSendDataLen = 0;
// 	int nRecvDataLen = 0;
// 
// 	memset(&cmdOrgTrace, 0x00, sizeof(CDMCMDTRACE));
// 
// 
// 	memcpy(&cmdOrgTrace, &m_cmdTrace, sizeof(CDMCMDTRACE));
// 	memset(m_cmdTrace.rxResponse, 0x00, sizeof(m_cmdTrace.rxResponse));
// 
// 	m_cmdTrace.lenResponse -= 1;	// -1 : ENKey
// 	byRealKeyIndex = GetDecryptKeyIndex(cmdOrgTrace.rxResponse[2]);
// 	nRecvDataLen = cmdOrgTrace.lenResponse - 6;
// 
// 	m_cmdTrace.rxResponse[0] = cmdOrgTrace.rxResponse[0];	// STX
// 	m_cmdTrace.rxResponse[1] = cmdOrgTrace.rxResponse[1]-1;	// Length, -1: ENKey 제외
// 	m_cmdTrace.rxResponse[2] = ExcuteKeyTable(cmdOrgTrace.rxResponse[3], byRealKeyIndex);
// 	if (nRecvDataLen > 0)
// 	{
// 		for (int i = 0; i < nRecvDataLen; i++)
// 			m_cmdTrace.rxResponse[3 + i] = ExcuteKeyTable(cmdOrgTrace.rxResponse[4 + i], byRealKeyIndex);
// 	}
// 	m_cmdTrace.rxResponse[3 + nRecvDataLen] = cmdOrgTrace.rxResponse[4 + nRecvDataLen];	// ETX
// }


/** ********************************************************************
* @brief Dispense Log 취득
* @param LPCDM_REJECT_INFO lpRejectLog	 RejectLog 구조체
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CCommCdm::GetDispenseLog(LPCDMRECVDATA lpRecvData, LPBYTE lpbyErrorCd)
{
	BYTE* ptr;
	int nResult = FAILED_TO_SEND_COMMAND;

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		LOG(Error, _T("GetDispenseLog() System Error!!!"));
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// ------------------------------
	//  STX + SIZE + CMD + ETX + BCC	
	// ------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_GET_DISPENSELOG;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_60);

	// clears inout buffer
	PurgeComm(m_hPort,
		PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	Sleep(500);	// F/W 요구사항, 200ms 대기 후 data read하도록...

	GetDispenseLogData(lpRecvData);

	return TRUE;
}


/** ********************************************************************
* @brief Dispense Log 취득
* @param LPCDM_REJECT_INFO lpRejectLog	 RejectLog 구조체
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CCommCdm::GetDispenseLogData(LPCDMRECVDATA lpRecvData)
{
	DWORD dwLogDataLen = 0;
	int i=0;

	for (i = 3; i >= 0; i--)
		dwLogDataLen |= (m_cmdTrace.rxResponse[6-i] << (i * 8));	// m_cmdTrace.rxResponse[3] ~ [6] : LogData Len

	if(dwLogDataLen <= 0)
	{
		LOG(Error, _T("Dispense Log size is Zero"));
		return FALSE;
	}

	lpRecvData->hData = GlobalAlloc(GHND, dwLogDataLen);
	if (NULL == lpRecvData->hData)
	{
		LOG(Error, _T("GlobalAlloc is failed."));
		return FALSE;
	}

	lpRecvData->dwSize = dwLogDataLen;

	lpRecvData->lpbBody = (BYTE*)GlobalLock(lpRecvData->hData);

	if (NULL == lpRecvData->lpbBody)
	{
		GlobalFree(lpRecvData->hData);

		lpRecvData->dwSize = 0;
		lpRecvData->hData = NULL;

		LOG(Error, _T("GlobalLock is failed."));

		return FALSE;
	}

	// Read Log Data
	if(!ReadPort(lpRecvData->lpbBody, dwLogDataLen))
	{
		LOG(Error, _T("Dispense Log ReadPort failed."));
		return FALSE;
	}

	return TRUE;
}


/** ********************************************************************
* @brief Firmware file에 있는 버전 조회
* @param CString strFilePath		bin file path
* @param CString& strHeaderVersion	firmware version
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CCommCdm::GetFWVersionInHeader(CString strFilePath, CString& strHeaderVersion)
{
	CString strVer;
	CFile binFile;
	char szTemp[256] = {0,};

	if (FALSE == binFile.Open(strFilePath, CFile::modeRead | CFile::typeBinary))
	{
		LOG(Error, _T("file open is failed."));
		return FALSE;
	}

	DWORD dwLength = binFile.GetLength();
	BYTE* lpData = NULL;
	DWORD dwRead = 0;

	lpData = new BYTE[dwLength];

	binFile.Close();

	HANDLE	hFile;

	hFile = ::CreateFile(strFilePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		LOG(Error, _T("CreateFile is failed"));

		if(lpData != NULL)
			delete[] lpData;

		return FALSE;
	}

	if (FALSE == ReadFile(hFile, lpData, dwLength, &dwRead, NULL))
	{
		LOG(Error, _T("ReadFile is failed."));

		if(lpData != NULL)
			delete[] lpData;
		::CloseHandle(hFile);

		return FALSE;
	}

	strHeaderVersion.Format(_T("%c%c%c%c"), *(lpData+13), *(lpData+14), *(lpData+16), *(lpData+17));

	if(lpData != NULL)
		delete[] lpData;
	::CloseHandle(hFile);

	return TRUE;
}

/** ********************************************************************
* @brief CE부 인증 수행 명령
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CCommCdm::CEInfoCertification(LPBYTE lpbyCEInfo, LPBYTE lpbyErrorCd)
{
	BYTE* ptr;
	INT i;
	int nResult = FAILED_TO_SEND_COMMAND;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("CEInfoCertification() System Error!!!"));
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// ---------------------------------------------------
	//  STX + SIZE + CMD + Data(32 Bytes) + ETX + BCC
	// ---------------------------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 37;

	*ptr++ = CC_STX;
	*ptr++ = 0x21;
	*ptr++ = CMD_CE_CERTIFICATION;
	for (i = 0; i<32; i++)
	{
		*ptr++ = lpbyCEInfo[i];
	}
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 35);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_CE_CERTIFICATION, lpbyErrorCd);

	if(*lpbyErrorCd != 0x30)
		return FALSE;
	else
		return TRUE;
}

/** ********************************************************************
* @brief 방출 동작 중인지 판단
* @retval TRUE	방출 동작 중
* @retval FALSE	방출 미동작
************************************************************************/
BOOL CCommCdm::IsSendDispense()
{
	return m_bSendDispense;
}

/** ********************************************************************
* @brief Firmware에 CE부 인증 정보가 있는지 여부
* @retval TRUE	성공
* @retval FALSE	실패
************************************************************************/
BOOL CCommCdm::GetExistCEInfo(LPBYTE lpbyCEInfo, LPBYTE lpbyErrorCd, BOOL& bExist)
{
	BYTE* ptr;
	int nResult = -1;
	BYTE byRecvData = 0;

	if(TRUE == m_bCEInfoError)
	{
		LOG(Error, _T("GetExistCEInfo() System Error!!!"));
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	memset(&m_cmdTrace, 0x00, sizeof(m_cmdTrace));

	// T-DES 알고리즘 적용
	memset(m_byUniqueKey, 0x00, sizeof(m_byUniqueKey));
	memcpy(m_byUniqueKey, lpbyCEInfo, CE_INFO_LENGTH);

	// -------------------------------
	// STX + SIZE + CMD + ETX + BCC
	// -------------------------------
	ptr = m_cmdTrace.txCommand;
	m_cmdTrace.lenCommand = 5;

	*ptr++ = CC_STX;
	*ptr++ = 0x01;
	*ptr++ = CMD_EXIST_CE_INFO;
	*ptr++ = CC_ETX;
	*ptr = GetCheckSum(m_cmdTrace.txCommand, 1, 3);

	// ---------------------
	//  start communication  
	// ---------------------
	SetTimeout(TO_SEC_NORMAL);

	// clears inout buffer
	PurgeComm(m_hPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	//	if(!HandShake())
	nResult = HandShake();
	if(NO_ERROR != nResult)
	{
		*lpbyErrorCd = COMMUNICATION_ERROR;	// Communication error
		return FALSE;
	}

	if(TRUE == m_bCEInfoError)
	{
		*lpbyErrorCd = CERTIFICATION_ERROR;	// Unique Key mismatch
		return FALSE;
	}

	if(m_byOrgCommand != m_byRecvCommand)
	{
		*lpbyErrorCd = SEND_RECV_CMD_MISMATCH_ERROR;	// 송/수신 CMD mismatch
		return FALSE;
	}

	AnalyzeGeneralCommand(CMD_EXIST_CE_INFO, &byRecvData);

	// 0x31 : CE Certification No Set, 0x30 : CE Certification Set
	if (0x31 == byRecvData)
		bExist = FALSE;
	else
		bExist = TRUE;

	return TRUE;
}


/** ********************************************************************
* @brief 방출 중 Power Off 인지 판단
* @retval 없음.
************************************************************************/
BOOL CCommCdm::SaveLastTransInfoData(CString strKey, CString strValue)
{
	// 파일 경로 취득
	CString strPath = _T("");

	TCHAR path[_MAX_PATH] = {0,};

	// 3rd argument is a character count, not a byte count.
	GetModuleFileName(NULL, path, _countof(path));

	CString strTempPath = path;

	strPath.Format(_T("%s%s"), strTempPath.Left(strTempPath.ReverseFind('\\')), _T("\\Config\\DISPENSE.CFG"));

	// INI 파일 열기
	CIniFile iniFile(strPath);

	iniFile.WriteString(_T("DISPENSEINFO"), strKey, strValue);

	return TRUE;
}


// T-DES 암호화
void CCommCdm::EncryptKeyData(BYTE *pbyKey)
{
	CDMCMDTRACE cmdOrgTrace;
	int i = 0;
	CDES pDes;
	int nScrSize = 0;
	BYTE byTmp1[8] = { 0, }, byTmp2[8] = { 0, }, byTmp3[8] = { 0, };
	BYTE byScrData[256] = { 0, };
	BYTE byDestData[256] = { 0, };
	int nIndex = 0;
	int nDesDataLen = 0;
	BYTE byCheckSumData4Key[8] = { 0, };

	memset(&cmdOrgTrace, 0x00, sizeof(CDMCMDTRACE));
	memcpy(&cmdOrgTrace, &m_cmdTrace, sizeof(CDMCMDTRACE));
	memset(m_cmdTrace.txCommand, 0x00, sizeof(m_cmdTrace.txCommand));

	// T-DES로 암호화 할 DATA는 평문 DATA(STX + LI + CMD + DATA + ETX + BCC) 중에 (LI + CMD + DATA) 이다.
	nDesDataLen = cmdOrgTrace.lenCommand - 3;	// -3 => STX, ETX, BCC	

	memcpy(byScrData, &cmdOrgTrace.txCommand[1], nDesDataLen);

	if (0 == (nDesDataLen % DATA_PADING_SIZE))
		nScrSize = nDesDataLen;
	else
		nScrSize = (nDesDataLen / DATA_PADING_SIZE + 1) * DATA_PADING_SIZE;

	for (i = 0; i < (nScrSize / DATA_PADING_SIZE); i++)
	{
		memcpy(byTmp1, &byScrData[i*DATA_PADING_SIZE], DATA_PADING_SIZE);

		pDes.ENCRYPT(byTmp1, byTmp2, pbyKey);
		pDes.DECRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.ENCRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&byDestData[i*DATA_PADING_SIZE], byTmp1, DATA_PADING_SIZE);
	}

	GetCheckSumForCryptoKey(byCheckSumData4Key, pbyKey);

	m_cmdTrace.lenCommand = nScrSize + 7;	// +7 => STX, T-DES LEN, KEY, CheckSum(2byte), ETX, BCC

	// Encrypt Send Data : STX + LI + KEY + CheckSum(2byte) + T-DES DATA(LI + CMD + DATA) + ETX + BCC
	// KEY : CE부 인증 CMD인지, 일반 CMD인지 구분자
	// CheckSum : T-DES Key에 대한 CheckSum
	m_cmdTrace.txCommand[nIndex++] = CC_STX;
	m_cmdTrace.txCommand[nIndex++] = (BYTE)nScrSize + 3;	// +3 => KEY(CE부 인증 CMD인지, 일반 CMD인지 구분자), CheckSum(2byte)
	if ((CMD_CE_CERTIFICATION == m_byOrgCommand) || (CMD_EXIST_CE_INFO == m_byOrgCommand))
		m_cmdTrace.txCommand[nIndex++] = 0xD0;
	else
		m_cmdTrace.txCommand[nIndex++] = 0xD1;
	if ((CMD_CE_CERTIFICATION == m_byOrgCommand) || (CMD_EXIST_CE_INFO == m_byOrgCommand))
	{
		m_cmdTrace.txCommand[nIndex++] = GetDummyRandomNumber();
		m_cmdTrace.txCommand[nIndex++] = GetDummyRandomNumber();
	}
	else
	{
		m_cmdTrace.txCommand[nIndex++] = byCheckSumData4Key[0];
		m_cmdTrace.txCommand[nIndex++] = byCheckSumData4Key[1];
	}
	memcpy(&m_cmdTrace.txCommand[nIndex], byDestData, nScrSize);

	nIndex += nScrSize;

	m_cmdTrace.txCommand[nIndex++] = CC_ETX;
	m_cmdTrace.txCommand[nIndex] = GetCheckSum(m_cmdTrace.txCommand, 1, m_cmdTrace.lenCommand-2);
}


// T-DES 복호화
void CCommCdm::DecryptKeyData(BYTE *pbyKey)
{
	CDMCMDTRACE cmdOrgTrace;
	int i = 0;
	CDES pDes;
	int nScrSize = 0;
	BYTE byTmp1[8] = { 0, }, byTmp2[8] = { 0, }, byTmp3[8] = { 0, };
	BYTE byScrData[256] = { 0, };
	BYTE byDestData[256] = { 0, };
	int nIndex = 0;

	memset(&cmdOrgTrace, 0x00, sizeof(CDMCMDTRACE));
	memcpy(&cmdOrgTrace, &m_cmdTrace, sizeof(CDMCMDTRACE));
	memset(m_cmdTrace.rxResponse, 0x00, sizeof(m_cmdTrace.rxResponse));

	// Encrypt Send Data : STX + LI + KEY + CheckSum(2byte) + T-DES DATA(LI + CMD + DATA) + ETX + BCC
	// KEY : CE부 인증 CMD인지, 일반 CMD인지 구분자
	// CheckSum : T-DES Key에 대한 CheckSum
	nScrSize = cmdOrgTrace.rxResponse[1] - 3;	// -3 => KEY + CheckSum(2byte)
	memcpy(byScrData, &cmdOrgTrace.rxResponse[5], nScrSize);

	for (i = 0; i < (nScrSize / DATA_PADING_SIZE); i++)
	{
		memcpy(byTmp1, &byScrData[i*DATA_PADING_SIZE], DATA_PADING_SIZE);

		pDes.DECRYPT(byTmp1, byTmp2, pbyKey);
		pDes.ENCRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.DECRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&byDestData[i*DATA_PADING_SIZE], byTmp1, DATA_PADING_SIZE);
	}

	// 평문 포맷인 STX + LI + CMD + DATA + ETX + BCC 로 맞춰야 함.
	m_cmdTrace.lenResponse = byDestData[0] + 4;	// +4 => STX, LI, ETX, BCC

	m_cmdTrace.rxResponse[nIndex++] = CC_STX;
	m_cmdTrace.rxResponse[nIndex++] = byDestData[0];
	memcpy(&m_cmdTrace.rxResponse[nIndex], &byDestData[1], byDestData[0]);

	nIndex += byDestData[0];

	m_cmdTrace.rxResponse[nIndex++] = CC_ETX;

	m_byRecvCommand = byDestData[1];	// Receive Command
}


// Make Pack
BYTE CCommCdm::MakePack(BYTE bySrc)
{
	BYTE byRet = 0;

	if ((bySrc >= 'a') && (bySrc <= 'f'))
		byRet = ((bySrc - 0x57) & 0x0f);
	else if ((bySrc >= 'A') && (bySrc <= 'F'))
		byRet = ((bySrc - 0x37) & 0x0f);
	else
		byRet = (bySrc & 0x0f);

	return byRet;
}


void CCommCdm::ConvertStringToHex(CString strSrc, BYTE *pByDest)
{
	int nSrcLength = strSrc.GetLength() / 2;

	for (int i = 0; i < nSrcLength; i++)
	{
		pByDest[i] = ((MakePack(strSrc.GetAt(2 * i)) << 4)) | ((MakePack(strSrc.GetAt((2 * i) + 1)) & 0x0F));
	}
}


BYTE CCommCdm::GetDummyRandomNumber()
{
	BYTE byRandomNumber = 0;

	srand((unsigned) GetTickCount());

	byRandomNumber	= (BYTE)(rand() % 256);

	Sleep(10);

	return byRandomNumber;
}


void CCommCdm::GetCheckSumForCryptoKey(BYTE *pbyOutData, BYTE *pbyKey)
{
	int i = 0;
	CDES pDes;
	int nScrSize = 0;
	BYTE byTmp1[8] = { 0, }, byTmp2[8] = { 0, }, byTmp3[8] = { 0, };
	BYTE byScrData[8] = { 0, };

	nScrSize = 8;

	for (i = 0; i < (nScrSize / 8); i++)
	{
		memcpy(byTmp1, &byScrData[i*8], 8);

		pDes.ENCRYPT(byTmp1, byTmp2, pbyKey);
		pDes.DECRYPT(byTmp2, byTmp3, &pbyKey[8]);
		pDes.ENCRYPT(byTmp3, byTmp1, pbyKey);

		memcpy(&pbyOutData[i*8], byTmp1, 8);
	}
}