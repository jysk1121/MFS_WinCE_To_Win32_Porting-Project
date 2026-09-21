#include "StdAfx.h"
#include "EPPComm.h"

//#define MFS_EPP

//CEPPComm* CEPPComm::m_pInstance = NULL;

CEPPComm::CEPPComm(void)
{
	m_bConnected = FALSE;
	m_hComm = NULL;
	m_hMutex = NULL;
	m_nFrameLength = 0;
	m_nRecvFrameLength = 0;
	m_bySendFlag = SENDFLAG_IDLE;
	m_byRecvFlag = RECVFLAG_IDLE;
	m_byRecvKeyFlag = KEYFLAG_IDLE;
	m_hCommWatchThread = NULL;
	m_hStatusThread = NULL;

	memset(m_SendBuff, NULL, sizeof(m_SendBuff));
	memset(m_RecvBuff, NULL, sizeof(m_RecvBuff));
	m_bStartKey = FALSE;
	m_bLookStartKey = FALSE;
	m_bRecvCtrl = FALSE;
	
	m_byCount = 0;

	m_nDeviceStatus = 0;
	m_nOldDeviceStatus = 0;

	m_EventFunc = NULL;

	InitializeCriticalSection(&m_cs);
}

CEPPComm::~CEPPComm(void)
{
	if(m_bConnected)
		ClosePort();

	DeleteCriticalSection(&m_cs);
}

int CEPPComm::OpenDevice(int nPortNum, DWORD dwBaudRate)
{
	LOGMSG(1, TEXT("CEPPComm::OpenDevice() Start\r\n"));

	memset(m_szErrCode, NULL, sizeof(m_szErrCode));
	
	//Device Open시에 필요한 정보를 설정함 - 각 디바이스마다 다름
	int nRet = 0;

	TCHAR szPortNum[10];

	memset(szPortNum, 0x00, sizeof(szPortNum));
	wsprintf(szPortNum, _T("COM%d:"), nPortNum);

	nRet = OpenPort(szPortNum, dwBaudRate);
	if(nRet)
	{
		LOGMSG(1, (L"Failure in OpenPort\r\n"));
		return nRet;
	}

	// COM포트 감시 스레드 생성
	DWORD dwThreadID;
	m_hCommWatchThread = ::CreateThread( NULL, 0, 
										(LPTHREAD_START_ROUTINE)ThreadWatchComm, 
										this, 0, &dwThreadID);
	if(!m_hCommWatchThread)
	{
		ClosePort();
		LOGMSG(1, (L"Failure in CreateThread\r\n"));
		return -1;
	}

	m_bConnected = TRUE;
	m_byRecvKeyFlag = KEYFLAG_NORMAL;
	LOGMSG(1, (L"CEPPComm::OpenDevice() End\r\n"));
	return 0;
}

int CEPPComm::CloseDevice()
{
	LOGMSG(1, (L"CEPPComm::CloseDevice() Start\r\n"));

	if(!m_bConnected)
	{
		LOGMSG(1, (L"CEPPComm::CloseDevice() END-1t\r\n"));
		return 0;
	}

	int nRet = 0;

	if(m_hCommWatchThread)
	{
		CloseHandle(m_hCommWatchThread);
		m_hCommWatchThread = NULL;
	}

	if(m_hStatusThread)
	{
		CloseHandle(m_hStatusThread);
		m_hStatusThread = NULL;
	}

	nRet = ClosePort();
	if(nRet)
	{
		LOGMSG(1, (L"CEPPComm::CloseDevice() END-2t\r\n"));
		return nRet;
	}

	LOGMSG(1, (L"CEPPComm::CloseDevice() END-3t\r\n"));

	return 0;
}


DWORD ThreadWatchComm(CEPPComm* pCmd)
{
	DWORD dwEvent;
	BOOL bOk = TRUE;
	BYTE buff[4096];
	DWORD dwRead;
	int nRet = 0;

	if (! SetCommMask(pCmd->m_hComm, EV_RXCHAR | EV_BREAK | EV_ERR))
		bOk = FALSE;

	if (! bOk)
	{
		return FALSE;
	}
	
	while (pCmd->m_bConnected)
	{
		dwEvent = 0;
		WaitCommEvent(pCmd->m_hComm, &dwEvent, NULL);
		
		if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
		{
			do
			{
				nRet = pCmd->ReadComm(buff, 4096, dwRead);
				if (pCmd->m_QueueRead.GetMaxQSize() - pCmd->m_QueueRead.GetSize() > (int)dwRead)
				{
					for(WORD i = 0; i < dwRead; i++)
						pCmd->m_QueueRead.PutByte(buff[i]);
				}
			} while (dwRead);

			pCmd->ReceiveComData();
		}

		if ((dwEvent & EV_BREAK) == EV_BREAK)
		{
			LOGMSG(1, (L"EV_BREAK\r\n"));
			break;
		}

		if ((dwEvent & EV_ERR) == EV_ERR)
		{
			LOGMSG(1, (L"EV_ERR\r\n"));
			break;
		}

		Sleep(10);
	}

	return TRUE;
}


DWORD ThreadPolling(CEPPComm* pCmd)
{
	BYTE pData[16];
	CString str;
	memset(pData, 0, sizeof(pData));

	DWORD dwRet = 0;
	BYTE byRet[32];
	int nRet = 0;
	int i = 0;
	while(pCmd->m_bConnected)
	{
		if(pCmd->m_bStartKey)
		{
			Sleep(100);
			continue;
		}

		for(i = 0; i < 3; i++)
		{
			nRet = pCmd->ExecuteCommand(CMD_READ_SW_VERSION, pData, 0, byRet, dwRet);
			if((nRet != ERR_SEND_DATA_FAIL) && (nRet != ERR_RECV_RESPONSE_TIMEOUT) &&
				(nRet != ERR_RECV_BCC_ERROR) && (nRet != ERR_CMD_MISSMATCH) &&
				(nRet != ERR_RECV_ACK_TIMEOUT))
			{
				pCmd->m_nDeviceStatus = 0;
				break;
			}
			else
			{
				str.Format(L"ERR3 = %X\r\n", nRet);
				LOGMSG(1, (str));
			}
			Sleep(500);
		}

		if(i >= 3)					//Error Event
		{
			pCmd->m_nDeviceStatus = 1;
			if(pCmd->m_nOldDeviceStatus != pCmd->m_nDeviceStatus)
			{
				if(pCmd->m_EventFunc)
					pCmd->m_EventFunc(0x00FF, 0xFF, NULL, 0);
				pCmd->m_nOldDeviceStatus = pCmd->m_nDeviceStatus;
			}
		}
		else
			pCmd->m_nOldDeviceStatus = pCmd->m_nDeviceStatus;

		if(pCmd->m_nDeviceStatus)
			Sleep(5000);
		else
			Sleep(1000);
	}

	return 0;
}


BOOL CEPPComm::SetKeyMask(LPBYTE pMaskBuff, BYTE byMaskCnt)
{
	memset(m_KeyMaskBuff, NULL, sizeof(m_KeyMaskBuff));
	memcpy((void*)m_KeyMaskBuff, (const void*)pMaskBuff, byMaskCnt);
	m_byKeyMaskCount = byMaskCnt;
	return TRUE;
}

BOOL CEPPComm::CheckBCC(LPBYTE pData, int nLength)
{
	BYTE BCC=0;
	
	for(int i=1; i<nLength - 1; i++)
		BCC ^= pData[i];
	
	if(BCC != pData[nLength - 1])
		return FALSE;

	return TRUE;
}


BOOL CEPPComm::CheckCRC(LPBYTE pData, int nLength, short RecvCRC)
{
	short CRC = 0;
	int i = 0;

	CRC = Calculate_CRC (pData[1], 0 );	/* remainder = 0 	*/
	for (i = 0 ; i < nLength; i++)
	{
		CRC = Calculate_CRC ( pData[i+2], CRC );
	}

	if(CRC != RecvCRC)
		return FALSE;

	return TRUE;
}

/**************************************************************************
// FUNCTION NAME : 
// DESCRIPTION	 : 
// PARAMETERS 
//               : 
//               : 
// RETURN VALUE 
//               : 
// DATE          : 2003-07-21 오전 10:41:13
// SEE ALSO      : 
**************************************************************************/
void CEPPComm::GetKeyCheckValueBuff(LPBYTE pDest, int nLength)
{ 
	if(nLength < sizeof(m_KeyCheckValueBuff))
		memcpy((void*)pDest, (const void*)m_KeyCheckValueBuff, nLength);	
	else
		memcpy((void*)pDest, (const void*)m_KeyCheckValueBuff, sizeof(m_KeyCheckValueBuff));	
};

/**************************************************************************
// FUNCTION NAME : 
// DESCRIPTION	 : 
// PARAMETERS 
//               : 
//               : 
// RETURN VALUE 
//               : 
// DATE          : 2003-07-21 오전 10:42:19
// SEE ALSO      : 
**************************************************************************/
void CEPPComm::GetExtraBuff(LPBYTE pDest, int nLength)
{ 
	if(nLength < sizeof(m_ExtraBuff))
		memcpy((void*)pDest, (const void*)m_ExtraBuff, nLength);	
	else
		memcpy((void*)pDest, (const void*)m_ExtraBuff, sizeof(m_ExtraBuff));	
};

/**************************************************************************
// FUNCTION NAME : ReceiveComData
// DESCRIPTION	 : COM port에서 읽은 데이터를 한 프레임 단위로 처리한다.
//                 ThreadWatchComm에서 호출되는 함수이다.
// PARAMETERS 
//               : 
//               : 
// RETURN VALUE 
//               : 
// DATE          : 2003-04-19 오전 10:39:22
// SEE ALSO      : 

1. ACK 수신 시
ACK가 아닌 경우
데이터가 많은 경우



2. 데이터 수신 시


**************************************************************************/
void CEPPComm::ReceiveComData()
{
	BYTE byRecvBuff[MAX_RECV_BUFFER_SIZE];

	memset(byRecvBuff, NULL, sizeof(byRecvBuff));
	int nReadSize = m_QueueRead.LookByte(byRecvBuff);
	if(nReadSize <= 0 )
		return;

	BYTE bQData;
	m_bCRCError = FALSE;

	CString str;
	str.Format(L"ReceiveComData() START %d (%X %X) \r\n", nReadSize, byRecvBuff[0], byRecvBuff[1]);

	LOGMSG(1, str);

	while(nReadSize > 0)
	{
		memset(byRecvBuff, NULL, sizeof(byRecvBuff));
		nReadSize = m_QueueRead.LookByte(byRecvBuff);
		if(nReadSize <= 0)
		{
			LOGMSG(1, (L"ReceiveComData() nReadSize <= 0 - 1 \r\n"));
			return;
		}

		if(m_bySendFlag == SENDFLAG_IDLE && 
			m_byRecvFlag == RECVFLAG_IDLE && 
			m_byRecvKeyFlag == KEYFLAG_IDLE)
		{
			m_QueueRead.Clear();
			m_QueueKeyCode.Clear();
			LOGMSG(1, (L"ReceiveComData() ALL IDLE - 1 \r\n"));
			return;
		}

		//Wait Command Response
		if(m_bySendFlag == SENDFLAG_SEND_REQ)
		{
			//06 
			if(byRecvBuff[0] == ACK)
			{
				m_QueueRead.GetByte(&bQData);	//ACK를 받았으면 Queue에서 꺼낸다.
				m_bySendFlag = SENDFLAG_IDLE;	//ACK 대기 모드 해제
				if(nReadSize > 1)
					continue;
				else
					return;
			}
			else if(byRecvBuff[0] == SOH)
			{
				if(nReadSize < 5)
				{
					LOGMSG(1, (L"ReceiveComData() nReadSize < 5 - 1 \r\n"));
					return;
				}

				unsigned short DataLen = byRecvBuff[2] * 0x100 + byRecvBuff[3];
				int nFrameLength = 1 + 2 + DataLen + 2; // SOH(1) + CTL(1) + LI(2) + DATA + CRC(2)
				if(nReadSize < nFrameLength)
				{
					LOGMSG(1, (L"ReceiveComData() nReadSize < nFrameLength - 1 \r\n"));
					return;
				}
				AnalysisCmdResponse(byRecvBuff, nFrameLength);

				for(int i=0; i < nFrameLength; i++)
					m_QueueRead.GetByte(&bQData);	 //Queue에서 한 프레임 데이터를 꺼낸다.
				if(nReadSize > nFrameLength)
					continue;
				else
					return;
			}
			else	//ACK 대기중인데 다른 데이터가 들어오면 Clear시킨다.
			{
				BYTE byTemp;
				m_QueueRead.GetByte(&byTemp);
				m_bRecvCtrl = FALSE;
				if(nReadSize > 1)
					continue;
				else
					return;
			}
		} //end of if - SENDFLAG_SEND_REQ
		else
		{
			if(byRecvBuff[0] != SOH)
			{
				str.Format(L"ReceiveComData() m_RecvBuff[0] = %X - 2 \r\n", byRecvBuff[0]);
				LOGMSG(1, str);
				m_QueueRead.GetByte(&bQData);
				if(nReadSize > 1)
					continue;
				else
					return;
			}

			unsigned short DataLen = byRecvBuff[2] * 0x100 + byRecvBuff[3];
			int nFrameLength = 2 + 2 + DataLen + 2; // SOH(1) + CTL(1) + LI(2) + DATA + CRC(2)

			if(nReadSize < nFrameLength)
			{
				LOGMSG(1, (L"ReceiveComData() nReadSize < nFrameLength - 2 \r\n"));
				return;
			}

			if(nReadSize >=  MAX_RECV_BUFFER_SIZE)
			{
				m_QueueRead.Clear();
				m_QueueKeyCode.Clear();
				LOGMSG(1, (L"ReceiveComData() nReadSize >= MAX_RECV_BUFFER_SIZE\r\n"));
				return;
			}

			AnalysisCmdResponse(byRecvBuff, nFrameLength);

			BYTE bQData;
			for(int i=0; i < nFrameLength; i++)
				m_QueueRead.GetByte(&bQData);	 //Queue에서 한 프레임 데이터를 꺼낸다.
			if(nReadSize > nFrameLength)
				continue;
			else
				return;

		} //end of else
	} //end of while
}

/**************************************************************************
// FUNCTION NAME : AnalysisCmdResponse
// DESCRIPTION	 : COM port에서 읽은 데이터를 해석한다. 
//                 ThreadWatchComm에서 호출되는 함수이다.
// PARAMETERS 
//               : 
//               : 
// RETURN VALUE 
//               : 
// DATE          : 2003-03-29 오후 1:53:55
// SEE ALSO      : 
**************************************************************************/
BOOL CEPPComm::AnalysisCmdResponse(BYTE *pData, int nFrameLength)
{
	CString str;
	str.Format(L"AnalysisCmdResponse() START(%d)\r\n", nFrameLength);
	LOGMSG(1, str);

	//1. CRC Check
	short CRC = pData[nFrameLength-2] * 0x100 + pData[nFrameLength-1];

	if(!CheckCRC(pData, nFrameLength-4, CRC))
	{
		LOGMSG(1, (L"AnalysisCmdResponse() CRC ERROR\r\n"));
		m_bCRCError = TRUE;
	}
	else
	{
		DWORD dwWritten = 0;
		BYTE ACKBuff[2] = {ACK, 0x00};
		WriteComm(ACKBuff, 1, dwWritten);
		m_bCRCError = FALSE;
	}

	if(m_bCRCError)
	{
		return FALSE;
	}

	//1. Command check - Event or Command Response
	//SOH+CTL+LH+LL+CMDH+CMDL+PARAM+....+CRCH+CRCL
	m_wCommand = ((pData[4] * 0x100) & 0x7FFF) + pData[5];

	if((m_wCommand == EVENT_ENTRY) ||
		(m_wCommand == EVENT_FLOAT_FRAME) ||
		(m_wCommand == EVENT_STATUS))
	{
		AnalysisKeyResponse(pData, nFrameLength);
		return TRUE;
	}

	int nDataLength = pData[2] * 0x100 + pData[3] - 2 - 2 - 4;//Exclude Command(2), Return Code Param (2+4)
	SetStatusInfo(&pData[8]);
	SetDataBuffer(&pData[12], nDataLength);

	m_byRecvFlag = RECVFLAG_IDLE;

	LOGMSG(1, (L"AnalysisCmdResponse() END\r\n"));

	return TRUE;
}

/**************************************************************************
// FUNCTION NAME : AnalysisKeyResponse
// DESCRIPTION	 : COM port에서 읽은 데이터를 해석한다. 
//                 ThreadWatchComm에서 호출되는 함수이다.
// PARAMETERS 
//               : 
//               : 
// RETURN VALUE 
//               : 
// DATE          : 2003-03-29 오후 1:53:55
// SEE ALSO      : 
**************************************************************************/
void CEPPComm::AnalysisKeyResponse(BYTE *pData, int nFrameLength)
{
	LOGMSG(1, (L"AnalysisKeyResponse() START\r\n"));
	BYTE byData[4096];

	memset(byData, 0, sizeof(byData));

	memcpy(byData, pData, nFrameLength);

	unsigned short nDataLength = 0;
	unsigned short usEventID = 0;
	unsigned short usParam = 0;

	nDataLength = byData[2] * 0x100 + byData[3] - 2;//Exclude Command(2)
	usEventID = byData[4] * 0x100 + byData[5];

	if(m_EventFunc)
	{
		m_EventFunc(usEventID, usParam, &byData[6], nDataLength);
	}

	LOGMSG(1, (L"AnalysisKeyResponse() END\r\n"));
}

void CEPPComm::SetStatusInfo(BYTE *pData)
{
	m_ulStatusInfo = pData[0] * 0x1000000 + pData[1] * 0x10000 +  pData[2] * 0x100 + pData[3];
	LOGMSG(1, (L"SetStatusInfo() END\r\n"));
}


/**************************************************************************
// FUNCTION NAME : 
// DESCRIPTION	 : 
// PARAMETERS 
//               : 
//               : 
// RETURN VALUE 
//               : 
// DATE          : 2003-04-02 오후 8:45:31
// SEE ALSO      : 
**************************************************************************/
void CEPPComm::SetDataBuffer(BYTE *pData, int nLength)
{
	memcpy(m_ExtraBuff, pData, nLength);
	m_nRecvFrameLength = nLength;
}

short CEPPComm::Calculate_CRC (BYTE ch, short remainder )
{
	BYTE *ch_ptr;
	int i;

	ch_ptr = (BYTE*)&remainder;
	*(ch_ptr+1) = *(ch_ptr+1) ^ ch; /* exclusive or ch with msb of remainder */
	for (i=0; i<=7; i++)
		if (remainder < 0)
		{	/* msb of remainder = 1 */
			remainder = remainder << 1;
			remainder = remainder ^ CRC_POLYNOMIAL;
		}
		else
			remainder = remainder << 1;
	return (remainder);
}

BYTE CEPPComm::GetControlCount()
{
	m_byCount++;
	if(m_byCount > 0x3F)
		m_byCount = 1;
	return m_byCount;
}

int CEPPComm::ExecuteCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus, BOOL bWaitResponse)
{
	LOGMSG(1, (L"ExecuteCommand() START\r\n"));

	//BYTE pSendData[4096];
	DWORD dwLength = 0;
	int nPos = 0;

	if(dwLen > (sizeof(m_SendBuff) - 6))
	{
		LOGMSG(1, (L"ExecuteCommand() ERR_BUFFER_SIZE_ERROR\r\n"));
		return ERR_BUFFER_SIZE_ERROR;
	}

	memset(m_SendBuff, 0, sizeof(m_SendBuff));
	unsigned short wPkt_length = 0;
 
	///////////////////////////////////
	//SOH LH LL CMDH CMDL CRCH CRCL
	///////////////////////////////////

	m_SendBuff[nPos++] = SOH;									dwLength++;
	m_SendBuff[nPos++] = GetControlCount();						dwLength++;

	//Length
	nPos += 2;
	dwLength += 2;

	//Command
	m_SendBuff[nPos++] = (nCommand & 0xFF00) >> 8;				dwLength++;		wPkt_length++;
	m_SendBuff[nPos++] = (nCommand & 0x00FF);					dwLength++;		wPkt_length++;

	//Data
	memcpy(&m_SendBuff[nPos], pData, dwLen);

	nPos += dwLen;
	dwLength += dwLen;
	wPkt_length += (unsigned short)dwLen;

	//Length
	m_SendBuff[2] = (wPkt_length & 0xFF00) >> 8;
	m_SendBuff[3] = wPkt_length & 0x00FF;

	int i = 0;
	short CRC = Calculate_CRC ( m_SendBuff[1], 0 );	/* remainder = 0 	*/
	for (i = 0 ; i < (wPkt_length+2); i++)
		CRC = Calculate_CRC ( m_SendBuff[i+2], CRC );

	m_SendBuff[nPos++] = (char) (CRC >> 8 ) & 0xFF;		dwLength++;
	m_SendBuff[nPos++] = (char) CRC & 0xFF;				dwLength++;

	CString str;
	EnterCriticalSection(&m_cs);
	int nRet = 0;
	int nRetryCount = 0;
	for(nRetryCount = 0; nRetryCount < 4; nRetryCount++)
	{
		str.Format(L"CNT = %d\r\n", nRetryCount);
		LOGMSG(1, (str));
		nRet = SendCommand(nCommand, m_SendBuff, dwLength, pResp, dwRet, bCheckStatus, bWaitResponse);
		if((nRet == ERR_SEND_DATA_FAIL) || (nRet == ERR_RECV_RESPONSE_TIMEOUT) ||
			(nRet == ERR_RECV_BCC_ERROR) || (nRet == ERR_CMD_MISSMATCH))
		{
			str.Format(L"ERR = %X\r\n", nRet);
			LOGMSG(1, (str));
			Sleep(500);
			continue;
		}
		else if(nRet == ERR_RECV_ACK_TIMEOUT)
		{
			Sleep(500);
			m_SendBuff[1] |= 0x80;

			CRC = Calculate_CRC (m_SendBuff[1], 0 );
			for (i = 0 ; i < (wPkt_length+2); i++)
				CRC = Calculate_CRC ( m_SendBuff[i+2], CRC );

			m_SendBuff[nPos-1] = (char) (CRC >> 8 ) & 0xFF;
			m_SendBuff[nPos] = (char) CRC & 0xFF;
			Sleep(500);
			str.Format(L"ERR2 = %X\r\n", nRet);
			LOGMSG(1, (str));
			continue;
		}
		else
		{
			break;
		}
	}
	LeaveCriticalSection(&m_cs);

	LOGMSG(1, (L"ExecuteCommand() END\r\n"));
	return nRet;
}


int CEPPComm::SendCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus, BOOL bWaitResponse)
{
	LOGMSG(1, (L"SendCommand() START\r\n"));

	//Clear flags, buffer
	InitializeExecuteCommand();

	if(dwLen >= 4096)
		return ERR_BUFFER_SIZE_ERROR;

	//Set flags
	m_bySendFlag = SENDFLAG_SEND_REQ;
	if(bWaitResponse)
		m_byRecvFlag = RECVFLAG_RECV_REQ;

	DWORD	dwErrorFlags = 0;
	COMSTAT	comstate;
	DWORD	dwEvent = 0;

	ClearCommError(m_hComm, &dwErrorFlags , &comstate);

	DWORD dwSize  = 0;
	BOOL bRet = FALSE;

	//Write command
	DWORD dwWritten = 0;
	int nRetryCount = 0;
	int nRet = WriteComm(pData, dwLen, dwWritten);
	if(nRet || (dwWritten != dwLen))
	{
		CString str;
		str.Format(L"WRITE ERR = %X\r\n", nRet);
		LOGMSG(1, (str));
		return ERR_SEND_DATA_FAIL;
	}

	DWORD dwStartTime = ::GetTickCount();
	//ACK 대기
	while(m_bySendFlag == SENDFLAG_SEND_REQ)
	{
		if(::GetTickCount() - dwStartTime > 5*1000)	//timeout
		{
			m_byRecvKeyFlag = KEYFLAG_NORMAL;
			LOGMSG(1, L"ERR_RECV_ACK_TIMEOUT\r\n");
			return ERR_RECV_ACK_TIMEOUT;
		}

		Sleep(1);
	}

	//Command 응답 대기
	dwStartTime = ::GetTickCount();
	while(m_byRecvFlag == RECVFLAG_RECV_REQ)
	{
		if(::GetTickCount() - dwStartTime > 20*1000)	//timeout
		{
			m_byRecvKeyFlag = KEYFLAG_NORMAL;
			LOGMSG(1, L"ERR_RECV_RESPONSE_TIMEOUT\r\n");
			return ERR_RECV_RESPONSE_TIMEOUT;
		}

		Sleep(1);
	}

	//Check BCC
	if(m_bCRCError)
	{
		m_byRecvKeyFlag = KEYFLAG_NORMAL;
		LOGMSG(1, L"ERR_RECV_BCC_ERROR\r\n");
		return ERR_RECV_BCC_ERROR;
	}

	//해당 command에 대한 응답인지 check
	if(bWaitResponse && (m_wCommand != nCommand))
	{
		m_byRecvKeyFlag = KEYFLAG_NORMAL;
		LOGMSG(1, L"ERR_CMD_MISSMATCH\r\n");
		return ERR_CMD_MISSMATCH;
	}

	//상태 정보 check
	if(bCheckStatus && (m_ulStatusInfo != 0))
	{
		m_byRecvKeyFlag = KEYFLAG_NORMAL;
		return m_ulStatusInfo;
	}

	memcpy(pResp, m_ExtraBuff, m_nRecvFrameLength);
	dwRet = m_nRecvFrameLength;
	m_byRecvKeyFlag = KEYFLAG_NORMAL;
	return 0;
}

void CEPPComm::InitializeExecuteCommand()
{
	m_bCRCError = FALSE;
	m_wCommand = 0;
	m_byPINCount = 0;
	m_nRecvFrameLength = 0;
	m_byStatusInfo = STATE_OK;
	m_ulStatusInfo = 0;
	m_bRecvCtrl = FALSE;
	
	m_byRecvKeyFlag = KEYFLAG_IDLE;
	m_bySendFlag = SENDFLAG_IDLE;
	m_byRecvFlag = RECVFLAG_IDLE;

	m_QueueRead.Clear();
	m_QueueKeyCode.Clear();
	
	memset(m_RecvBuff, NULL, sizeof(m_RecvBuff));
	memset(m_ExtraBuff, NULL, sizeof(m_ExtraBuff));
	memset(m_szErrCode, NULL, sizeof(m_szErrCode));
}


int CEPPComm::RegistEventCallBackFunction(EventCallBackFunction pFunc)
{
	if(pFunc != NULL)
		m_EventFunc = pFunc;
	return 0;
}

int CEPPComm::GetCommandResultData(LPBYTE lpData, int nBuffLeng, int* nLength)
{
	if(nBuffLeng <= m_nRecvFrameLength)
	{
		*nLength = m_nRecvFrameLength;
		return m_nRecvFrameLength;
	}

	memcpy(lpData, m_ExtraBuff, m_nRecvFrameLength);
	*nLength = m_nRecvFrameLength;
	return 0;
}