#include "stdafx.h"
#include "CommThread.h"
#include <windows.h>
#include <stdlib.h>

#define ACTIVE_DEBUG				0
#include "../../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

typedef enum
{
	WRITE_LEVEL_0 = 0,
	WRITE_LEVEL_1,		// 송/수신 전문 로그
	WRITE_LEVEL_2,		// 추후 사용
	WRITE_LEVEL_3,		// 추후 사용
	WRITE_LEVEL_4,		// 추후 사용
} WRITE_LOG_LEVEL;

#define ICR			0x20
#define IMR			0x21
#define ESC			0x1b 	 
#define SOH			0x01
#define STX			0x02
#define ETX			0x03
#define ENQ			0x05
#define ACK			0x06
#define DLE			0x10
#define NAK			0x15
#define DELI		0x40


WORD Crc_Table[] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7, 
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876, 
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a, 
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70, 
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff, 
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134, 
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a, 
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9, 
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78 
};

//Global variable
CCommThread *g_pCPortCom;
BOOL		m_bConnected;			// 
OVERLAPPED	m_osRead, m_osWrite;	// 포트 파일 Overlapped structure
HANDLE		m_hComm;				// 통신 포트 파일 핸들
HANDLE		m_hThreadWatchComm;		// Watch함수 Thread 핸들.
HANDLE		WaitEventHandle;
RX_PACKET *RecvData;
//BYTE g_byRcvBuf[MAX_RECEIVE];
BYTE g_byRcvBuf[512];		// Buffer가 RX_PACKET보다 크게 잡아야 memory overflow 발생하지 않음
BYTE g_bySendBuf[MAX_SEND];
BYTE bSensor[2];
BYTE ReceivedSTX ;
BYTE ReceivedETX ;
BYTE ReceivedACK ;
BYTE ReceivedNAK ; 
BYTE ReceivedENQ ; 
BYTE ReceivedDLE ; 
BYTE ReceivedPacket ;
static int RxTail;

int nMUBVersion = 0;

#define SWAP16(s) (((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))


//--- 클래스 생성자
CCommThread::CCommThread()
{
	//--> 초기는 당연히..포트가 열리지 않은 상태다.
	m_bConnected = FALSE;
	RxTail = 0;
	RecvData = (RX_PACKET*)g_byRcvBuf ;
	
	memset(bSensor, 0, sizeof(bSensor));

	CEagleLogger::CreateInstance();
	CEagleLogger::GetInstance()->Initialize(_T("EagleCE_MUB"), 30, 1024 * 1024);
	
	m_nLogLevel = 0;
	m_bPollingCommand4CIM = FALSE;
}

CCommThread::~CCommThread()
{
	CEagleLogger::GetInstance()->Finalize();
	CEagleLogger::ReleaseInstance();
}

BOOL CCommThread::bInitComPort(CString strPortName)
{
	if( m_bConnected )	
	{
		g_pCPortCom->ClosePort();
	}

	COMMTIMEOUTS	timeouts;
	DCB				dcb;

	// overlapped structure 변수 초기화.
	m_osRead.Offset = 0;
	m_osRead.OffsetHigh = 0;
	if( !(m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) )	
	{
		LOG(Error, _T("bInitComPort() Read CreateEvent is failed."));
		return FALSE;
	}

	m_osWrite.Offset = 0;
	m_osWrite.OffsetHigh = 0;
	if( !(m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) )
	{
		LOG(Error, _T("bInitComPort() Write CreateEvent is failed."));
		return FALSE;
	}

	m_hComm = CreateFile( strPortName, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	
	if (m_hComm == (HANDLE) -1) 
	{
		LOG(Error, _T("bInitComPort() Commport CreateFile is failed."));
		return FALSE;
	}

	// get comm port status 
	if(!GetCommState(m_hComm, &dcb)){
		LOG(Error, _T("bInitComPort() GetCommState is failed."));
		return FALSE;
	}

	// change dcb
	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;

	// set comm port status
	if(!SetCommState(m_hComm, &dcb)){
		LOG(Error, _T("bInitComPort() SetCommState is failed."));
		return FALSE;
	}

	// set timeout
	timeouts.ReadIntervalTimeout = 0xFFFF/*MAXWORD*/;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	//timeouts.ReadTotalTimeoutConstant = 30 * 1000;
	timeouts.ReadTotalTimeoutConstant = 3 * 1000;	// Timeout을 30초에서 3초로 변경
	timeouts.WriteTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	SetCommTimeouts(m_hComm, &timeouts);

	// set buffer size
	SetupComm(m_hComm, 4096, 4096);	

	// clears inout buffer
	PurgeComm(m_hComm, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	//! essential : starting by communicating with CDM
	EscapeCommFunction(m_hComm, CLRRTS);	//<- for CE

	EscapeCommFunction(m_hComm, CLRDTR);

	m_bConnected = TRUE;

	return TRUE;
}

void CCommThread::ClosePort()
{
	m_bConnected = FALSE;

	SetCommMask(m_hComm, 0);
	
	PurgeComm(m_hComm,	PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
	
	CloseHandle(m_hComm);
}

// 실제로 쓰여진 Byte수를 리턴한다.
DWORD CCommThread::WriteComm(BYTE *pBuff, DWORD nToWrite)
{
	//DWORD	dwWritten, dwError, dwErrorFlags;
	//COMSTAT	comstat;
	DWORD	dwWritten = 0;

	if(!m_bConnected)		
	{
		return 0;
	}

	if( !WriteFile(m_hComm, pBuff, nToWrite, &dwWritten, &m_osWrite))
	{
		// Write File 실패시 처리
		return 0;
	}

	return dwWritten;
}

// 실제로 읽혀진 Byte수를 리턴한다.
DWORD CCommThread::ReadComm(BYTE *pBuff, DWORD nToRead)
{
	//DWORD	dwRead, dwError, dwErrorFlags;
	DWORD	dwRead, dwErrorFlags;
	COMSTAT	comstat;

	//--- system queue에 도착한 byte수만 미리 읽는다.
	ClearCommError( m_hComm, &dwErrorFlags, &comstat);

	//--> 시스템 큐에서 읽을 거리가 있으면..
	dwRead = comstat.cbInQue;
	
	if(dwRead > 0)
	{
		if(!ReadFile( m_hComm, pBuff, nToRead, &dwRead, &m_osRead))
		{
			return 0;
		}
	}

	return dwRead;
}

void CCommThread::ProcessRcvData( BYTE *byBuf, int nSize )
{
	BYTE CommData = 0;
	BYTE RespData = ACK;
	
	for(int j = 0; j<nSize; j++)
	{
		CommData = byBuf[j];
		if(!ReceivedSTX)
		{
			switch(CommData)
			{
				case STX :
    				ReceivedSTX = 1;
					ReceivedPacket = 0;
					ReceivedETX = 0;
					ReceivedACK = 0;
					ReceivedNAK = 0;
					ReceivedDLE = 0;
					CheckBCC = 0;
					DataLen = 0;
					RxTail = 0;
					break;
			
				case ACK :
				default  :
					break; 
			}
		}
		else
		{
			if(!ReceivedETX) 
			{
				if (CommData == ETX)
					ReceivedETX = 1;
			}
			else {
				ReceivedPacket = 1;
				ReceivedETX = 0;
				ReceivedSTX = 0;

				//WriteComm(&RespData, 1); // Send ACK	// 상위국에서 Ack 처리하므로 여기서는 제거함
			}

			CheckBCC ^= CommData;
			g_byRcvBuf[RxTail++] = CommData;
			RxTail %= MAX_RECEIVE;
		}
	}	 

	if(ReceivedPacket)
	{
		SetEvent(WaitEventHandle);
 
		ReceivedPacket = 0;	//V02.01
	}
	
	return;
}

DWORD	ThreadWatchComm(CCommThread* pComm)
{
   DWORD           dwEvent;
   OVERLAPPED      os;
   BOOL            bOk = TRUE;
   BYTE            buff[MAX_RECEIVE];      // Read buffer
   DWORD           dwRead = 0;			// Read Length
   int i = 0;

   memset( &os, 0, sizeof(OVERLAPPED));
   memset(buff, 0, sizeof(buff));
   
   if( !(os.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL)) )
   {
		bOk = FALSE;
   }

   if( !SetCommMask( m_hComm, EV_RXCHAR) )
   {
	   bOk = FALSE;
   }

   if( !bOk )
   {
		return FALSE;
   }

   // 포트를 감시하는 루프.
   while( m_bConnected )
   {
        dwEvent = 0;

        // 포트에 읽을 거리가 올때까지 기다린다.
        WaitCommEvent(m_hComm, &dwEvent, NULL);
		
		//--> 데이터가 수신되었다는 메세지가 발생하면..
        if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
        {
			// 포트에서 읽을 수 있는 만큼 읽는다.
            do
            {
				//--> buff 에 받아놓고..
				memset(buff, 0, sizeof(buff));
                dwRead = pComm->ReadComm( buff, sizeof(buff));
					
				//--> 버퍼를 분석하는 단계로 넘긴다.
				if (dwRead > 0)
					pComm->ProcessRcvData( buff, (int)dwRead );

            } while (dwRead);

        }//endif~

		Sleep(10);

   }//endwhile(..)... 포트가 ClosePort에 의해 닫히면 m_bConnected 가 FALSE가 되어 종료.

   CloseHandle( os.hEvent);

   //--> 쓰레드 종료가 되겠죠?
   m_hThreadWatchComm = NULL;

   return TRUE;

}

// Send Data to MUB
BOOL CCommThread::Send_Packet(BYTE byCommand, BYTE byID, BYTE* pData, int nLen)
{
	WORD wCRC = 0;
	int nIndex = 0;

	BYTE byControlChar = 0;

	if (nMUBVersion < SUPPORT_MULTIPORT_FUNC_VERSION)
	{
		if (byCommand == MBU_CMD_MULTI_COMPORT_SET)
		{
			LOG(Error, _T("Send_Packet() Command is MULTI COMPORT SET. This version is not supported. Command:[0x%02X]"), byCommand);
			return FALSE;
		}

		if (byID == ID_ASD || byID == ID_BA1 || byID == ID_BA2 || byID == ID_BA3 || byID == ID_BAR)
		{
			LOG(Error, _T("Send_Packet() This version is not supported. ID:[0x%02X]"), byID);
			return FALSE;
		}
	}

	// Init Buffer
	memset(g_bySendBuf, 0, sizeof(g_bySendBuf));

//////////////////////////////////////////////////////////////////////////
// Make Send Buffer
//////////////////////////////////////////////////////////////////////////

	g_bySendBuf[nIndex++] = STX;

	// LEN
	g_bySendBuf[nIndex++] = (BYTE)((nLen >> 8) & 0xFF);
	g_bySendBuf[nIndex++] = (BYTE)((nLen >> 0) & 0xFF);

	// MUB Command
	g_bySendBuf[nIndex++] = byCommand;

	if (byCommand == MBU_CMD_SEND || byCommand == MBU_CMD_MULTI_COMPORT_SET)
	{
		// Device ID
		g_bySendBuf[nIndex++] = byID;

		// Device Command & Data
		memcpy(&g_bySendBuf[nIndex], pData, nLen-2);
		nIndex += nLen-2;
	}

	g_bySendBuf[nIndex++] = ETX;

	// CRC16: LEN ~ ETX
	wCRC = Calc_CRC16(&g_bySendBuf[1], nIndex-1);
	g_bySendBuf[nIndex++] = (BYTE)((wCRC >> 8) & 0xFF);
	g_bySendBuf[nIndex++] = (BYTE)((wCRC >> 0) & 0xFF);
	//

	if(ID_BA1 == byID)	// CIM
	{
		if( (g_bySendBuf[7] & 0x10) && (g_bySendBuf[8] == 0x00) && (g_bySendBuf[9] == 0x1C) )	// polling command
			m_bPollingCommand4CIM = TRUE;
		else
			m_bPollingCommand4CIM = FALSE;
	}
	else
	{
		m_bPollingCommand4CIM = FALSE;
	}

//////////////////////////////////////////////////////////////////////////
// Send to MUB
//////////////////////////////////////////////////////////////////////////
	CString strTemp, strTemp2;

	// 송신 로그
	if(ID_BA1 == byID)	// CIM만 로그 남기도록
	{
		if(WRITE_LEVEL_1 == m_nLogLevel)
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			{
				for(int i=0; i<nIndex; i++)
				{
					strTemp.Format(_T("%02x "), g_bySendBuf[i]);
					strTemp2 += strTemp;
				}

				LOG(Info, _T("Send_Packet() byCommand : [0x%02X], byID : [0x%02X] SendData : [%s]"), byCommand, byID, strTemp2);	
			}
		}
	}
	///////////////////////////////////////
	
	//1. Send Command
	if(!WritePort(g_bySendBuf, nIndex))
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("Send_Packet() WritePort is failed_1."));
		return FALSE;
	}

	//2. Wait ACK
	RXTYPE rxType;
	BYTE rxBuf[MAX_RECEIVE];
	INT nTry = 0;

	memset(rxBuf, 0, sizeof(rxBuf));
	rxType = GetAction(rxBuf);

	if (rxType != RX_ACK)
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("Send_Packet() Wait ACK is failed. rxType:[%d]"), rxType);
		goto send_error;
	}

	//3. Wait Response
	rxType = GetAction(rxBuf);

	if(rxType == RX_RESPONSE || rxType == RX_RESPONSE_WITH_TEXT)
	{
//		LOG(Info, _T("Send_Packet() Send ACK. rxType:[%d]"), rxType);

		// 정상적으로 받았을때의 version을 저장함
		if (byCommand == MBU_CMD_VERSION)
		{
			LOG(Info, _T("Send_Packet() MUB Version Info :[%c][%c][%c]"), RecvData->Data[0], RecvData->Data[1], RecvData->Data[2]);
			nMUBVersion = (int)RecvData->Data[2];
		}

		//4. Send ACK
		byControlChar = ACK;
		WritePort(&byControlChar, 1);
	}
	else 
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("Send_Packet() Wait Response is failed. rxType:[%d]"), rxType);
		// clears inout buffer
//		PurgeComm(m_hComm, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
//		return FALSE;
		goto send_error;

	}

	return TRUE;

send_error:

	if(FALSE == m_bPollingCommand4CIM)	// polling 제외
		LOG(Error, _T("Send_Packet() SendError. Send ACK"));
	//4. Send ACK
	byControlChar = ACK;
	WritePort(&byControlChar, 1);

	return FALSE;
}

// Receive Data From MUB
int CCommThread::Recv_Packet( BYTE byID, BYTE* pData )
{
	//BYTE byControlChar = 0;
	RXTYPE rxType;
	BYTE rxBuf[MAX_RECEIVE];

	memset(RecvData, 0, sizeof(RX_PACKET));
	memset(rxBuf, 0, sizeof(rxBuf));

	if (nMUBVersion < SUPPORT_MULTIPORT_FUNC_VERSION)
	{
		if (byID == ID_ASD || byID == ID_BA1 || byID == ID_BA2 || byID == ID_BA3 || byID == ID_BAR)
		{
			LOG(Error, _T("Recv_Packet() This version is not supported. ID:[0x%02X]"), byID);
			return FALSE;
		}
	}

	WORD wCRC = 0;
	int nIndex = 0;

	BYTE byControlChar = 0;

	// 상위 호환성을 위해 Version을 기준으로 Read Command 전송하도록 수정
	if (nMUBVersion >= SUPPORT_READPORT_FUNC_VERSION)
	{
		// Init Buffer
		memset(g_bySendBuf, 0, sizeof(g_bySendBuf));

		//////////////////////////////////////////////////////////////////////////
		// Make Send Buffer
		//////////////////////////////////////////////////////////////////////////

		g_bySendBuf[nIndex++] = STX;

		// LEN
		g_bySendBuf[nIndex++] = (BYTE)0x00;
		g_bySendBuf[nIndex++] = (BYTE)0x03;

		// MUB Command
		g_bySendBuf[nIndex++] = MBU_CMD_RECV;

		// Device ID
		g_bySendBuf[nIndex++] = byID;

		// Function
		g_bySendBuf[nIndex++] = MBU_CMD_RECVFUNC_READ;

		g_bySendBuf[nIndex++] = ETX;

		// CRC16: LEN ~ ETX
		wCRC = Calc_CRC16(&g_bySendBuf[1], nIndex-1);
		g_bySendBuf[nIndex++] = (BYTE)((wCRC >> 8) & 0xFF);
		g_bySendBuf[nIndex++] = (BYTE)((wCRC >> 0) & 0xFF);
		//

	//////////////////////////////////////////////////////////////////////////
	// Send to MUB
	//////////////////////////////////////////////////////////////////////////
		CString strTemp, strTemp2;
		if(ID_BA1 == byID)	// CIM만 로그 남기도록
		{
			if(WRITE_LEVEL_1 == m_nLogLevel)
			{
				if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				{
					for(int i=0; i<nIndex; i++)
					{
						strTemp.Format(_T("%02x "), g_bySendBuf[i]);
						strTemp2 += strTemp;
					}

					LOG(Info, _T("Recv_Packet() SendData : [%s]"), strTemp2);
				}
			}
		}

		////////////////////////////////////////////////////////
		//1. Send Command
		if(!WritePort(g_bySendBuf, nIndex))
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("Recv_Packet() WritePort is failed_1."));
			return FALSE;
		}

		// wait ack
		memset(rxBuf, 0, sizeof(rxBuf));
		rxType = GetAction(rxBuf);

		if (rxType != RX_ACK)
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("Recv_Packet() Wait ACK is failed. rxType:[%d]"), rxType);
			goto recv_error;
		}
	}

	//1. Wait Response
	memset(rxBuf, 0, sizeof(rxBuf));
	rxType = GetAction(rxBuf);

	if(rxType == RX_RESPONSE || rxType == RX_RESPONSE_WITH_TEXT) 
	{
//		LOG(Info, _T("Recv_Packet() Send ACK. rxType:[%d]"), rxType);

		if (nMUBVersion >= SUPPORT_READPORT_FUNC_VERSION)
		{
			//4. Send ACK
			byControlChar = ACK;
			WritePort(&byControlChar, 1);
		}
	}
	else
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("Recv_Packet() GetAction is failed."));
		// clears inout buffer
		//PurgeComm(m_hComm, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		//return FALSE;
		goto recv_error;
	}

	return RecvData->wLength;

recv_error:

	if(FALSE == m_bPollingCommand4CIM)	// polling 제외
		LOG(Error, _T("Recv_Packet() error_return"));

	if (nMUBVersion >= SUPPORT_READPORT_FUNC_VERSION)
	{
		//4. Send ACK
		byControlChar = ACK;
		WritePort(&byControlChar, 1);
	}

	return 0;
}


BYTE CCommThread::Calculate_BCC(BYTE *buffer, WORD leng)
{
	BYTE bcc = 0 ;

	while( leng-- )
		bcc^= *buffer++ ;

	return(bcc) ;
}

WORD CCommThread::Calc_CRC16( BYTE *buffer, int wleng )
{
	int dt = 0;
	WORD crc16 = 0;

	for(dt = 0; dt < wleng; dt++) {
		crc16 = (crc16 >> 8)^Crc_Table[(crc16^*buffer++)&0x00FF];
	}

	return(crc16);
}

//BOOL CCommThread::WritePort( LPSTR lpBuf, DWORD nByteToWrite )
BOOL CCommThread::WritePort( LPBYTE lpBuf, DWORD nByteToWrite )
{
	DWORD nWrite = 0;

	if(!WriteFile(m_hComm, lpBuf, nByteToWrite, &nWrite, 0))
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("WritePort() WriteFile is failed."));
		return FALSE;
	}

	if (nWrite == 0)
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("WritePort() nWrite is zero."));
		return FALSE;
	}

	return TRUE;
}

//BOOL CCommThread::ReadPort( LPSTR lpBuf, DWORD nByteToRead )
BOOL CCommThread::ReadPort( LPBYTE lpBuf, DWORD nByteToRead )
{
	DWORD nRead = 0;

	INT time_cnt = 0;

	// read port
	if(!ReadFile(m_hComm, lpBuf, nByteToRead, &nRead, 0))
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("ReadPort() ReadFile is failed."));
		return FALSE;
	}

	if(nRead == 0)
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("ReadPort() nRead is zero."));
		return FALSE;
	}

	return TRUE;

}

//RXTYPE CCommThread::GetAction( LPSTR lpBuf )
RXTYPE CCommThread::GetAction( LPBYTE lpBuf )
{
//	LPSTR ptr;
	LPBYTE ptr;
	CHAR ch;
	INT lenData = 0;

	BOOL bAfterSTX = FALSE;
	INT lenText, lenResponse;

	ptr = lpBuf;
	lenText = lenResponse = 0;

	// -----------------------
	//     첫 데이터 읽음
	// -----------------------
	if(!ReadPort(ptr, 1))
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("GetAction() ReadPort failed_1"));
		return RX_ERROR;
	}


	ch = *ptr;

	if(ch == ACK){			// ACK 문자 확인
//		LOG(Info, _T("GetAction() ACK"));
		return RX_ACK;
	}
	else if(ch == NAK){		// NAK 문자 확인
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("GetAction() NAK"));
		return RX_NAK;
	}
	else if(ch == STX) {		// 시작 문자가 STX
		ptr++;
		lenResponse++;

		// Length를 읽음
		if(!ReadPort(ptr, 2))
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("GetAction() ReadPort failed_2"));
			return RX_ERROR;
		}

		lenData = *ptr++ * 0x100;
		lenData += *ptr++;

		//RETAILMSG(1, (_T("[GetAction] Read 2 - Len : %d\n"), lenData));

		// Read Data가 Buffer를 초과하는 경우 Error 처리 추가
		//if (lenData > (MAX_RECEIVE - 1))
		if ((lenData > (MAX_RECEIVE - 1)) || (lenData <= 0))	// 0보다 작은 경우에 대한 처리 추가
		{
			// length 오류인 경우 serial port로부터 모든 data 버림 처리
			BYTE chTemp[1024] = { 0, };
//			ReadPort((LPSTR)chTemp, sizeof(chTemp));
			ReadPort(chTemp, sizeof(chTemp));

			RETAILMSG(1, (_T("[MFS_DLL] GetAction - Dummy Data Detected - Len : %d\n"), lenData));
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("[MFS_DLL] GetAction - Dummy Data Detected - Len : %d"), lenData);

			return LEN_ERROR;
		}

		lenResponse +=2;

		// Body Reading : CMD+ID+Data
		if(!ReadPort(ptr, lenData))
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("GetAction() ReadPort failed_3"));
			return RX_ERROR;
		}

		ptr += lenData; lenResponse += lenData;

		// ETX + CRC16 Reading 
		if(!ReadPort(ptr, 3))
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("GetAction() ReadPort failed_4"));
			return RX_ERROR;
		}

		//RETAILMSG(1, (_T("[GetAction] Read 4 ETX CRC Received\n")));

		lenResponse += 3;

		// 수신 로그
		CString strTemp, strTemp2;
		if(ID_BA1 == lpBuf[4])	// CIM만 로그 남기도록
		{
			if(WRITE_LEVEL_1 == m_nLogLevel)
			{
				if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				{
					for(int i=0; i<lenResponse; i++)
					{
						strTemp.Format(_T("%02x "), lpBuf[i]);
						strTemp2 += strTemp;
					}

					LOG(Error, _T("GetAction() All RecvData : [%s]"), strTemp2);
				}
			}
		}
		////////////////////////////////////////////////////////

		RecvData->byCommand = lpBuf[3];
		RecvData->byDevID = lpBuf[4];
		memcpy(RecvData->Data, &lpBuf[5], lenResponse - 8);
		RecvData->wLength = lenResponse - 8; //<- Data len

		ch = *ptr;
		if(ch != ETX)
			return RX_ERROR;
		else
			return RX_RESPONSE;
	}

	return RX_NONE;
}


BOOL CCommThread::Recv_Buffer_Reset(BYTE byID)
{
	//BYTE byControlChar = 0;
	RXTYPE rxType;
	BYTE rxBuf[MAX_RECEIVE];

	memset(RecvData, 0, sizeof(RX_PACKET));
	memset(rxBuf, 0, sizeof(rxBuf));

	if (nMUBVersion < SUPPORT_MULTIPORT_FUNC_VERSION)
	{
		if (byID == ID_ASD || byID == ID_BA1 || byID == ID_BA2 || byID == ID_BA3 || byID == ID_BAR)
		{
			LOG(Error, _T("Recv_Buffer_Reset() This version is not supported. ID:[0x%02X]"), byID);
			return FALSE;
		}
	}

	WORD wCRC = 0;
	int nIndex = 0;

	BYTE byControlChar = 0;

	// 상위 호환성을 위해 Version을 기준으로 Read Command 전송하도록 수정
	if (nMUBVersion >= SUPPORT_READPORT_FUNC_VERSION)
	{
		// Init Buffer
		memset(g_bySendBuf, 0, sizeof(g_bySendBuf));

		//////////////////////////////////////////////////////////////////////////
		// Make Send Buffer
		//////////////////////////////////////////////////////////////////////////

		g_bySendBuf[nIndex++] = STX;

		// LEN
		g_bySendBuf[nIndex++] = (BYTE)0x00;
		g_bySendBuf[nIndex++] = (BYTE)0x03;

		// MUB Command
		g_bySendBuf[nIndex++] = MBU_CMD_RECV;

		// Device ID
		g_bySendBuf[nIndex++] = byID;

		// Function
		g_bySendBuf[nIndex++] = MBU_CMD_RECVFUNC_BUFF_RESET;

		g_bySendBuf[nIndex++] = ETX;

		// CRC16: LEN ~ ETX
		wCRC = Calc_CRC16(&g_bySendBuf[1], nIndex-1);
		g_bySendBuf[nIndex++] = (BYTE)((wCRC >> 8) & 0xFF);
		g_bySendBuf[nIndex++] = (BYTE)((wCRC >> 0) & 0xFF);
		//

	//////////////////////////////////////////////////////////////////////////
	// Send to MUB
	//////////////////////////////////////////////////////////////////////////
		CString strTemp, strTemp2;
		if(ID_BA1 == byID)	// CIM만 로그 남기도록
		{
			if(WRITE_LEVEL_1 == m_nLogLevel)
			{
				if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				{
					for(int i=0; i<nIndex; i++)
					{
						strTemp.Format(_T("%02x "), g_bySendBuf[i]);
						strTemp2 += strTemp;
					}

					LOG(Info, _T("Recv_Buffer_Reset() SendData : [%s]"), strTemp2);
				}
			}
		}
		////////////////////////////////////////////////////////
		//1. Send Command
		if(!WritePort(g_bySendBuf, nIndex))
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("Recv_Buffer_Reset() WritePort is failed_1."));
			return FALSE;
		}

		// wait ack
		memset(rxBuf, 0, sizeof(rxBuf));
		rxType = GetAction(rxBuf);

		if (rxType != RX_ACK)
		{
			if(FALSE == m_bPollingCommand4CIM)	// polling 제외
				LOG(Error, _T("Recv_Buffer_Reset() Wait ACK is failed. rxType:[%d]"), rxType);
			goto recv_error;
		}
	}

	//1. Wait Response
	memset(rxBuf, 0, sizeof(rxBuf));
	rxType = GetAction(rxBuf);

	if(rxType == RX_RESPONSE || rxType == RX_RESPONSE_WITH_TEXT) 
	{
//		LOG(Info, _T("Recv_Buffer_Reset() Send ACK. rxType:[%d]"), rxType);

		if (nMUBVersion >= SUPPORT_READPORT_FUNC_VERSION)
		{
			//4. Send ACK
			byControlChar = ACK;
			WritePort(&byControlChar, 1);
		}
	}
	else
	{
		if(FALSE == m_bPollingCommand4CIM)	// polling 제외
			LOG(Error, _T("Recv_Buffer_Reset() GetAction is failed."));
		// clears inout buffer
		//PurgeComm(m_hComm, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		//return FALSE;
		goto recv_error;
	}

	return TRUE;

recv_error:

	if(FALSE == m_bPollingCommand4CIM)	// polling 제외
		LOG(Error, _T("Recv_Buffer_Reset() error_return"));

	if (nMUBVersion >= SUPPORT_READPORT_FUNC_VERSION)
	{
		//4. Send ACK
		byControlChar = ACK;
		WritePort(&byControlChar, 1);
	}

	return FALSE;
}


void CCommThread::WriteLogLevel(BYTE byID, int nLogLevel)
{
	m_nLogLevel = nLogLevel;
}
