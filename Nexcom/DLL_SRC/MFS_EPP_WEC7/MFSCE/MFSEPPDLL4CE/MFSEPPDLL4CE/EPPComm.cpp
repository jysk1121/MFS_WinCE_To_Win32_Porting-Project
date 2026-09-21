#include "StdAfx.h"
#include "EPPComm.h"

#define MFS_EPP

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

	memset(m_RecvBuff, NULL, sizeof(m_RecvBuff));

	m_bStartKey = FALSE;
	m_bLookStartKey = FALSE;
	m_bRecvCtrl = FALSE;
}

CEPPComm::~CEPPComm(void)
{
	if(m_bConnected)
		ClosePort();
}

//int CEPPComm::OpenDevice(const TCHAR* szPortNum, DWORD dwBaudRate)
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
		return 0;

	int nRet = 0;
	nRet = ClosePort();
	if(nRet)
		return nRet;
	if(m_hCommWatchThread)
	{
		CloseHandle(m_hCommWatchThread);
		m_hCommWatchThread = NULL;
	}
	return 0;
}


DWORD ThreadWatchComm(CEPPComm* pCmd)
{
	DWORD dwEvent;
	BOOL bOk = TRUE;
	BYTE buff[2048];
	DWORD dwRead;
	int nRet = 0;

#ifdef WIN32
	OVERLAPPED os;

	memset( &os, 0, sizeof(OVERLAPPED));
	if (! (os.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL)))
		bOk = FALSE;
#endif
//	if (! SetCommMask( pCmd->m_hComm, EV_RXCHAR))
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
				nRet = pCmd->ReadComm(buff, 2048, dwRead);
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

#ifdef WIN32
	CloseHandle(os.hEvent);
#endif
//	pCmd->m_hCommWatchThread = NULL;
	
	return TRUE;
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
//		Transmit_char ( *packet++ );
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
**************************************************************************/
void CEPPComm::ReceiveComData()
{
	memset(m_RecvBuff, NULL, sizeof(m_RecvBuff));
	int nReadSize = m_QueueRead.LookByte(m_RecvBuff);
	if(nReadSize <= 0 )
		return;

	BYTE bQData;
	m_bCRCError = FALSE;
	DWORD dwWritten = 0;
//	static BOOL bRecvCtrl = FALSE;

	CString str;
	str.Format(L"ReceiveComData() START %d\r\n", nReadSize);

	LOGMSG(1, str);

	while(nReadSize > 0)
	{
		memset(m_RecvBuff, NULL, sizeof(m_RecvBuff));
		nReadSize = m_QueueRead.LookByte(m_RecvBuff);
		if(nReadSize <= 0)
		{
			LOGMSG(1, (L"ReceiveComData() nReadSize <= 0 - 1 \r\n"));
			return;
		}
{
	CString str, strT;
	str.Empty();
	str += L"LOOK2 : ";
	for(int i = 0; i < nReadSize; i++)
	{
		strT.Format(L"%02X", m_RecvBuff[i]&0xFF);
		str += strT;
	}
	str += L"\r\n";
	RETAILMSG(1, (str));
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
#ifdef MFS_EPP
			//0x10 
			if(m_RecvBuff[0] == 0x10)
			{
				m_QueueRead.GetByte(&bQData);	//ACK를 받았으면 Queue에서 꺼낸다.
//				m_bySendFlag = SENDFLAG_IDLE;	//ACK 대기 모드 해제
				m_bRecvCtrl = TRUE;
				continue;
			}
			//06 
			else if(m_RecvBuff[0] == ACK)
			{
				m_QueueRead.GetByte(&bQData);	//ACK를 받았으면 Queue에서 꺼낸다.
				if(m_bRecvCtrl)
				{
					m_bySendFlag = SENDFLAG_IDLE;	//ACK 대기 모드 해제
					BYTE ENGBuff[2] = {0x10, 0x05};
					WriteComm(ENGBuff, 2, dwWritten);
					m_bRecvCtrl = FALSE;
				}
				continue;
			}
			else if(m_RecvBuff[0] == NACK)
			{
				m_QueueRead.GetByte(&bQData);	//ACK를 받았으면 Queue에서 꺼낸다.
				m_bySendFlag = SENDFLAG_IDLE;	//ACK 대기 모드 해제
				m_bRecvCtrl = FALSE;
				continue;
			}

			else if(m_RecvBuff[0] == STX)	//STX MTYPE CMD PARAM LL LH RC+DATA ETX BCC
			{
				if(nReadSize < 5)
					return;

				BYTE DataLen = m_RecvBuff[5] * 0x100 + m_RecvBuff[4];
				int nFrameLength = DataLen + 8;
				if(nReadSize < nFrameLength)
					return;

				BYTE bQData;
				for(int i=0; i < nFrameLength; i++)
					m_QueueRead.GetByte(&bQData);	 //Queue에서 한 프레임 데이터를 꺼낸다.
				continue;
			}
#else
			//06 
			if(m_RecvBuff[0] == ACK)
			{
				m_QueueRead.GetByte(&bQData);	//ACK를 받았으면 Queue에서 꺼낸다.
				m_bySendFlag = SENDFLAG_IDLE;	//ACK 대기 모드 해제
				return;
			}
			else if(m_RecvBuff[0] == SOH)
			{
				if(nReadSize < 5)
				{
					LOGMSG(1, (L"ReceiveComData() nReadSize < 5 - 1 \r\n"));
					return;
				}

				BYTE DataLen = m_RecvBuff[2] * 0x100 + m_RecvBuff[3];
				int nFrameLength = 1 + 2 + DataLen + 2; // SOH(1) + CTL(1) + LI(2) + DATA + CRC(2)
				if(nReadSize < nFrameLength)
				{
					LOGMSG(1, (L"ReceiveComData() nReadSize < nFrameLength - 1 \r\n"));
					return;
				}

				BYTE bQData;
				for(int i=0; i < nFrameLength; i++)
					m_QueueRead.GetByte(&bQData);	 //Queue에서 한 프레임 데이터를 꺼낸다.
				continue;
			}
#endif
			else	//ACK 대기중인데 다른 데이터가 들어오면 Clear시킨다.
			{
//				BYTE byTemp;
//				m_QueueRead.GetByte(&byTemp);
//				continue;
				m_QueueRead.Clear();
//				LOGMSG(1, (L"ReceiveComData() m_RecvBuff[0] = %X - 1\r\n", m_RecvBuff[0]));
				m_bRecvCtrl = FALSE;
				return;
			}
		} //end of if - SENDFLAG_SEND_REQ
		else
		{
#ifdef MFS_EPP
			if(m_RecvBuff[0] != STX)
			{
				BYTE bQData;
				m_QueueRead.GetByte(&bQData);
				continue;
//				m_QueueRead.Clear();
//				return;
			}

			if(nReadSize < 5)
				return;

			BYTE DataLen = m_RecvBuff[5] * 0x100 + m_RecvBuff[4];
			int nFrameLength = DataLen + 8; //STX MTYPE CMD PARAM LL LH RC+DATA ETX BCC
#else
			if(m_RecvBuff[0] != SOH)
			{
				str.Format(L"ReceiveComData() m_RecvBuff[0] = %X - 2 \r\n", m_RecvBuff[0]);
				LOGMSG(1, str);
				m_QueueRead.Clear(); //Think over!! : 이런 경우 쓰레기 데이터가 Queue에 들어 있다고 가정함
				return;
			}

			BYTE DataLen = m_RecvBuff[2] * 0x100 + m_RecvBuff[3];
			int nFrameLength = 2 + 2 + DataLen + 2; // SOH(1) + CTL(1) + LI(2) + DATA + CRC(2)
#endif
			if(nReadSize < nFrameLength)
			{
				LOGMSG(1, (L"ReceiveComData() nReadSize < nFrameLength - 2 \r\n"));
				return;
			}

//2.Check receive flag : Command에 대한 응답 대기 모드 /////////////////////////////////////////
/*			if(m_byRecvFlag == RECVFLAG_RECV_REQ)
			{
				//command response 한 프레임 data 모두 받았음 -> 해석 함수 호출
				if(AnalysisCmdResponse(m_RecvBuff, nFrameLength)) 
				{
#ifndef MFS_EPP
					BYTE ACKBuff[2] = {ACK, 0x00};
					WriteComm(ACKBuff, 1, dwWritten);
#endif
					m_byRecvFlag = RECVFLAG_IDLE;
				}
				else	//BCC Error인 경우 NACK를 보낸다.
				{
					m_QueueRead.Clear();
					BYTE NACKBuff[2] = {NACK, 0x00};
					WriteComm(NACKBuff, 1, dwWritten);
				}
				continue;
			} //end of if - RECVFLAG_RECV_REQ

//3. Receive key code : Key input 받는 모드 ///////////////////////////////////////////////////
			else if(m_byRecvKeyFlag == KEYFLAG_ENC || 
					m_byRecvKeyFlag == KEYFLAG_NORMAL ||
					m_byRecvKeyFlag == KEYFLAG_INPUT_KEY || 
					m_byRecvKeyFlag == KEYFLAG_INPUT_KEY_TDES)
			{
				//Key code 한 프레임 data 모두 받았음 -> 해석 함수 호출
				AnalysisKeyResponse(m_RecvBuff, nFrameLength);
				continue;
			}
*/
			if(AnalysisCmdResponse(m_RecvBuff, nFrameLength)) 
			{
#ifndef MFS_EPP
				BYTE ACKBuff[2] = {ACK, 0x00};
				WriteComm(ACKBuff, 1, dwWritten);
#endif
//				m_byRecvFlag = RECVFLAG_IDLE;
			}
			continue;
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
#ifdef MFS_EPP
	if(!CheckBCC(pData, nFrameLength))
	{
		m_bCRCError = TRUE;	
//		return FALSE;
	}
#else
	//Check CRC
	short CRC = pData[nFrameLength-2] * 0x100 + pData[nFrameLength-1];
	if(!CheckCRC(pData, nFrameLength-4, CRC))
	{
		LOGMSG(1, (L"AnalysisCmdResponse() CRC ERROR\r\n"));
		m_bCRCError = TRUE;
	}
#endif
	else
	{
		m_bCRCError = FALSE;
	}

	//1. Command check - Event or Command Response
#ifdef MFS_EPP
	unsigned short usCmd = 0;
//	usCmd = pData[2] + pData[1] * 0x100;
	usCmd = pData[1];
	m_wCommand = pData[2];
#else
	//SOH+CTL+LH+LL+CMDH+CMDL+PARAM+....+CRCH+CRCL
	m_wCommand = ((pData[4] * 0x100) & 0x7FFF) + pData[5];
#endif

#ifdef MFS_EPP
/*	if((usCmd == 0x0230) ||
		(usCmd == 0x0231) ||
		(usCmd == 0x0232) ||
		(usCmd == 0x0233) ||
		(usCmd == 0x0234))*/
	if(usCmd == 0x02)
#else
	if((m_wCommand == EVENT_ENTRY) ||
		(m_wCommand == EVENT_FLOAT_FRAME) ||
		(m_wCommand == EVENT_STATUS))
#endif
	{
		AnalysisKeyResponse(pData, nFrameLength);
		return TRUE;
	}

	if(m_bCRCError)
		return FALSE;

#ifdef MFS_EPP
	int nDataLength = pData[5] * 0x100 + pData[4];//Exclude Command(2), Return Code Param (2+4)
	SetStatusInfo(&pData[6]);
	SetDataBuffer(&pData[6+1], nDataLength-1);
#else
	int nDataLength = pData[2] * 0x100 + pData[3] - 2 - 2 - 4;//Exclude Command(2), Return Code Param (2+4)
	SetStatusInfo(&pData[8]);
	SetDataBuffer(&pData[12], nDataLength);
#endif

	if(nFrameLength < 2)
	{
		str.Format(L"AnalysisCmdResponse() nFrameLength < 2 (%d)\r\n", nFrameLength);
		LOGMSG(1, str);
		return FALSE;
	}
	m_byRecvFlag = RECVFLAG_IDLE;

	BYTE bQData;
	for(int i = 0; i < nFrameLength; i++)
	{
		m_QueueRead.GetByte(&bQData);	 //Queue에서 한 프레임 데이터를 꺼낸다.
	}

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
//	LOGMSG(1, (L"AnalysisKeyResponse() START(%d)\r\n", nFrameLength));
	LOGMSG(1, (L"AnalysisKeyResponse() START\r\n"));

	//PIN 입력 완료 response check
/*	if(m_RecvBuff[3] == SET_ENC_MODE ||
		m_RecvBuff[3] == INPUT_KEY_BY_PINPAD ||
		m_RecvBuff[3] == INPUT_KEY_BY_PINPAD_TDES)
	{
		AnalysisCmdResponse(nFrameLength);

		m_QueueKeyCode.Clear();
		m_byRecvKeyFlag = KEYFLAG_NORMAL;
		return;
	}
*/

	unsigned short nDataLength = 0;
	unsigned short usEventID = 0;
	unsigned short usParam = 0;

#ifdef MFS_EPP
	nDataLength = pData[5] * 0x100 + pData[4];
	usEventID = pData[2];
	usParam = pData[3];
#else
	nDataLength = pData[2] * 0x100 + pData[3] - 2;//Exclude Command(2)
	usEventID = pData[4] * 0x100 + pData[5];
#endif

	if(m_EventFunc)
	{
#ifdef MFS_EPP
		if((pData[2] == 0x30) ||
			(pData[2] == 0x31) ||
			(pData[2] == 0x32) ||
			(pData[2] == 0x33) ||
			(pData[2] == 0x34))
		{
			if(m_bStartKey)
				m_EventFunc(usEventID, usParam, &pData[6], nDataLength);
		}
		else
			m_EventFunc(usEventID, usParam, &pData[6], nDataLength);
#else
		m_EventFunc(usEventID, &pData[6], nDataLength);
#endif
	}

	BYTE bQData;
	for(int i=0; i< nFrameLength; i++)
	{
		m_QueueRead.GetByte(&bQData);
	}
	LOGMSG(1, (L"AnalysisKeyResponse() END\r\n"));
}

void CEPPComm::SetStatusInfo(BYTE *pData)
{
#ifdef MFS_EPP
	m_ulStatusInfo = pData[0];
#else
	m_ulStatusInfo = pData[0] * 0x1000000 + pData[1] * 0x10000 +  pData[2] * 0x100 + pData[3];
#endif
}


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

//int CEPPComm::SendCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus, BOOL bWaitResponse)
//int CEPPComm::SendCommand(int nCommand, LPBYTE pData, DWORD dwLen)
int CEPPComm::ExecuteCommand(int nCommand, BYTE *pData, DWORD dwDataLen,
								BYTE *pResult, DWORD &dwResultLength,
								BOOL bCheckStatus, BOOL bWaitResponse)
{
	LOGMSG(1, (L"ExecuteCommand() START\r\n"));

	if(nCommand == GET_NORMAL_KEY)
	{
		m_byRecvKeyFlag = KEYFLAG_NORMAL;
		m_bStartKey = TRUE;
		return 0;
	}
	else if(nCommand == INPUT_CANCEL)
	{
		m_byRecvKeyFlag = KEYFLAG_IDLE;
		m_bStartKey = FALSE;
			return 0;
	}
	//Clear flags, buffer
	InitializeExecuteCommand();
	BYTE bySendData[128];

	memset(bySendData, NULL, sizeof(bySendData));
	if(dwDataLen >= sizeof(bySendData))
		return 0xEE31;

	memcpy(bySendData, pData, dwDataLen);

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
	int nRet = WriteComm(pData, dwDataLen, dwWritten);
	while((nRet != 0) || (dwWritten != dwDataLen))
	{
		if(nRetryCount >= MAX_RETRANSMISSION)
		{
			if(!nRet)
				nRet = 0xEEA0;
			m_byRecvKeyFlag = KEYFLAG_NORMAL;
			return nRet;
		}
		nRet = WriteComm(pData, dwDataLen, dwWritten);
		nRetryCount++;
	}

//	LOGMSG(1, (L"SendCommand() %d / %d SUCCESS\r\n", dwLen, dwWritten));

	DWORD dwStartTime = ::GetTickCount();
	//ACK 대기
	while(m_bySendFlag == SENDFLAG_SEND_REQ)
	{
		if(::GetTickCount() - dwStartTime > 5*1000)	//timeout
		{
			m_byRecvKeyFlag = KEYFLAG_NORMAL;
			return 0xEE01;
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
			return 0xEE02;
		}

		Sleep(1);
	}

	//Check BCC
	if(m_bCRCError)
	{
		m_byRecvKeyFlag = KEYFLAG_NORMAL;
		return 0xEE03;
	}

	//해당 command에 대한 응답인지 check
	if(bWaitResponse && (m_wCommand != nCommand))
	{
		return 0xEE04;
	}

	//상태 정보 check
	if(bCheckStatus && (m_ulStatusInfo != 0))
	{
		return m_ulStatusInfo;
	}

//	dwRet = m_ExtraBuff[2] * 0x100 + m_ExtraBuff[3] - 2;
//	if(dwRet > 0)

	memcpy(pResult, m_ExtraBuff, m_nRecvFrameLength);
	dwResultLength = m_nRecvFrameLength;
	m_byRecvKeyFlag = KEYFLAG_NORMAL;
	LOGMSG(1, (L"ExecuteCommand() END\r\n"));
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
	m_byRecvKeyFlag = KEYFLAG_IDLE;
	m_bRecvCtrl = FALSE;
	
	m_bySendFlag = SENDFLAG_IDLE;
	m_byRecvFlag = RECVFLAG_IDLE;

	m_QueueRead.Clear();
	m_QueueKeyCode.Clear();
	
	memset(m_RecvBuff, NULL, sizeof(m_RecvBuff));
	memset(m_ExtraBuff, NULL, sizeof(m_ExtraBuff));
//	memset(m_szErrCode, NULL, sizeof(m_szErrCode));
}


int CEPPComm::RegistEventCallBackFunction(EventCallBackFunction pFunc)
{
/*	BYTE bySample[10];
	unsigned short id = 0x4567;
	unsigned short len = 0;
	if(pFunc != NULL)
	{
		bySample[0] = 0x41;
		bySample[1] = 0x41;
		bySample[2] = 0x41;
		bySample[3] = 0x41;
		bySample[4] = 0x41;
		bySample[5] = 0x41;
		bySample[6] = 0x41;
		bySample[7] = 0x41;
		bySample[8] = 0x41;
		bySample[9] = 0x41;
		m_EventFunc = pFunc;

		len = sizeof(bySample);

		m_EventFunc(id, bySample, len);
	}
	else
		return 0x4444;*/
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