#include "StdAfx.h"
#include "AMSCtrl.h"

#define TIMEOUT_CONNECT			(60)
#define TIMEOUT_ENQ				(7)
#define TIMEOUT_NEGOTIATE		(10)
#define TIMEOUT_RECVDATA		(30)

#define FS		(char)0x1C

enum AMS_STATUS
{
	AMS_INIT = 0,
	AMS_DIAL,			// AMS로 전화 걸때.
	AMS_ENQ,			// AMS로 전화 걸때. DAIL만.
	AMS_ACCEPT,			// AMS에서 전화 올때.
	AMS_CONN,			// AMS에서 전화 올때. DIAL만.
	AMS_SEND,
	AMS_RECV,
	AMS_STX_IN,
	AMS_ETX_IN

};

CAMSCtrl::CAMSCtrl(int LineType)
{
	m_bOpen = FALSE;
	m_LineType = LineType;

	m_pNetLine = CLineCtrl::Instance(LineType);
}

CAMSCtrl::~CAMSCtrl(void)
{
}

/** ***********************************************************************************
*	@brief AMS 접속\n
			TCP일경우 port 번호를 주어야 한다.\n
			option은 나중에 SSL 사용 여부이다.
*	@param LPCTSTR OpenType		Listen인지 Connect인지 
*	@param LPCTSTR pDestInfo	AMS 정보
*	@param LPCTSTR port			AMS port 번호
*	@param LPCTSTR option		SSL 사용 여부 
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
int	CAMSCtrl::AMSConnectOpen(LPCTSTR OpenType, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	int	nResult = 1;

	if (!m_bOpen)
	{
		m_pNetLine->LineClose();

		if (!wcscmp(_T("LISTEN"), OpenType))
		{
			LOG(Info, _T("AMS Open - Listen Mode ( %d)"), m_LineType);

			if (LINE_TCP == m_LineType)
			{
				nResult = m_pNetLine->LineOpen(pDestInfo, port, option, _T("1"));
			}
			else
			{
				m_bOpen = TRUE; // 실제로 Open이 Line 성공하였으나 실패로 넘어오는 경우가 있어서 위치 옮김 (TCP/IP는 위치 옮기면 접속 안됨)
				// LineClose 시 m_bOpen이 FALSE여서 AMS랑 계속 연결 되어 있는 현상 발생함
				//nResult = m_pNetLine->LineOpen(pDestInfo, port, option, _T("1"));//LineOpen은 성공하였으나 실패로 리턴되는 경우 있어 보완 처리함 (추가확인 필요)
				nResult = m_pNetLine->LineOpen(pDestInfo, port, _T("1"), _T("1"));//LineOpen은 성공하였으나 실패로 리턴되는 경우 있어 보완 처리함 (추가확인 필요)
			}

			if (!nResult)
			{
				ClearQueue();
				SetCheckTimer(TIMER_CLEAR);
				m_bOpen = TRUE;
				return AMS_OK;
			}
		}
		else if (!wcscmp(_T("CONNECT"), OpenType))
		{
			LOG(Info, _T("AMS Open - Connect Mode"));

			ClearQueue();
			SetCheckTimer(TIMER_CLEAR);

			if (LINE_TCP == m_LineType)
			{
				// nResult = m_pNetLine->LineOpen(pDestInfo, port, option, L"0");
				nResult = m_pNetLine->LineOpen(pDestInfo, port, option, _T("2"));	// TCP Timeout 조정을 위해 Option 2 추가
			}
			else
			{
				m_bOpen = TRUE; // 실제로 Open이 Line 성공하였으나 실패로 넘어오는 경우가 있어서 위치 옮김 (TCP/IP는 위치 옮기면 접속 안됨)
				
				// LineClose 시 m_bOpen이 FALSE여서 AMS랑 계속 연결 되어 있는 현상 발생함
				
				nResult = m_pNetLine->LineOpen(pDestInfo, port, _T("1"), _T("0"));

				if (nResult == AMS_OK)
					nResult = Flow_Matrix(AMS_DIAL);
			}

			if (nResult == AMS_OK)
			{
				m_bOpen = TRUE;

				//ClearQueue();
				//SetCheckTimer(TIMER_CLEAR);

				// ENQ가 올때까지 대기 후 AMS OK로 Return하도록 처리 필요
				nResult = Flow_Matrix(AMS_CONN);

				if (nResult == AMS_OK)
					return AMS_OK;
			}
		}

		return AMS_ERR;
	}

	return AMS_OK;
}


/** ***********************************************************************************
*	@brief AMS로부터 통신을 받기 위함.
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
int	CAMSCtrl::AMSConnectAccept()
{
	int nResult;

	nResult = m_pNetLine->LineAccept((BYTE)1);

	// TCP와 Dial의 리턴값이 다름..
	if (m_LineType == LINE_TCP)
		nResult = !nResult;

	if (!nResult)
	{
		// Dial 만 수행 한다.
		if (m_LineType == LINE_DIALUP)
		{	
			nResult = Flow_Matrix(AMS_ACCEPT);

			if (nResult != AMS_OK)
			{
				AMSConnectClose();
				return AMS_ERR;
			}
		}

		// ENQ송신 후 정상 처리
		BYTE ctrl_rsp = 0;
		ctrl_rsp = DATA_ENQ;
		m_pNetLine->LineSendData(&ctrl_rsp, 1);

		return AMS_OK;
	}

	AMSConnectClose();

	return AMS_ERR;
}

/** ***********************************************************************************
*	@brief AMS LineClose
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
int	CAMSCtrl::AMSConnectClose()
{
	if (m_bOpen)
	{
		if (!m_pNetLine->LineClose())
		{
			m_bOpen = FALSE;
			return AMS_OK;
		}

		m_bOpen = FALSE;	// 추가

		return AMS_ERR;
	}

	return AMS_OK;
}

/** ***********************************************************************************
*	@brief AMS와의 연결 상태 체크\n
		   Dialup : 전화 벨이 울리고 있는지\n
		   TCP : listen 된 소켓이 있는지
*	@retval TRUE	성공 
*	@retval FALSE	장애
***************************************************************************************/
BOOL CAMSCtrl::AMSConnectCheck()
{
	if (!m_bOpen)
		return FALSE;

	if (m_LineType == LINE_DIALUP)
		return (!m_pNetLine->LineCallState());

	return (m_pNetLine->LineCallState());
}

/** ***********************************************************************************
*	@brief 전문 파싱
*	@param int initStatus		전문 초기 상태
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
int	CAMSCtrl::Flow_Matrix(int initStatus)
{
	int		status		= initStatus;
	int		rty_cnt		= 0;
	BYTE	ctrl_rsp	= 0;
	BYTE	chk_bcc		= 0;
	BYTE	recv_code	= 0;
	int		nresult		= 0;
	int		nDataLen	= 0;
	int		max_rty_cnt = 3;
	BYTE	byLength[6] = {0,};
	CString strLength;

//*LEH 추후 고려	int nAMSTimeoutReceiveTcpIp = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP);
	int nAMSTimeoutReceiveTcpIp = TIMEOUT_AMS_RECEIVE_TCPIP;
//	CString	strTemp;

	// TCP/IP일 경우 Retry 없음.
	if (m_LineType == LINE_TCP)	//*LEH 고려사항
		max_rty_cnt = -1;

	// AP Holding 방지를 위해 max timeout을 설정한다. -- Max 3분.
	DWORD	max_timeout = GetTickCount() + (180*1000);

	if (status == AMS_DIAL || status == AMS_ACCEPT)
	{
		// Status Send일 경우에는 MODEM에서만 발생하므로 TCP/IP는 코딩 안함
		SetCheckTimer(TIMEOUT_CONNECT);
		LOG(Info, _T("AMS_DIAL || AMS_ACCEPT"));
	}
	else if (status == AMS_RECV)
	{
		switch (m_LineType)	{
			case LINE_TCP:
				SetCheckTimer(nAMSTimeoutReceiveTcpIp);	break;
			default:
				SetCheckTimer(TIMEOUT_RECVDATA);	break;
		}
	}
	else if (status == AMS_SEND)
	{
		switch (m_LineType)	{
			case LINE_TCP:
				SetCheckTimer(TIMEOUT_NEGOTIATE);	break;
			default:
				SetCheckTimer(TIMEOUT_RECVDATA);	break;
		}
	}
	else
	{
		SetCheckTimer(TIMEOUT_NEGOTIATE);
	}

	while(GetTickCount() < max_timeout)
	{
		nresult = GetQueue(&recv_code);

		if (nresult == RECV_DATA)
		{
			switch(status)
			{
			case AMS_ENQ:
				{
					if (recv_code == DATA_EOT)
					{
						LOG(Error, _T("AMS Received EOT : (AMS_ENQ)"));

						// Send EOT
						//ctrl_rsp = DATA_EOT;
						//m_pNetLine->LineSendData(&ctrl_rsp, 1);

						SetCheckTimer(TIMER_CLEAR);
						return AMS_ERR;
					}
					else if (recv_code == DATA_ACK)
					{
						SetCheckTimer(TIMER_CLEAR);
						return AMS_OK;
					}
/*					else if (recv_code == DATA_NAK)
					{
						LOG(Error, _T("AMS Received NAK : (AMS_ENQ)"));

						SetCheckTimer(TIMER_CLEAR);

						if (rty_cnt > max_rty_cnt)
						{
							LOG(Error, _T("AMS Received NAK - Retry Over - Error Return : (AMS_ENQ)"));

							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);

							SetCheckTimer(TIMER_CLEAR);
							return AMS_ERR;
						}

						// wait
						CUtil::Sleep_Wait(500);

						// Send ENQ
						ctrl_rsp = DATA_ENQ;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						// reset time out
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt++;
					}*/
				}
				break;

			case AMS_CONN:
				{
					if (recv_code == DATA_EOT)
					{
						LOG(Error, _T("AMS Received EOT : (AMS_CONN)"));

						// Send EOT
						//ctrl_rsp = DATA_EOT;
						//m_pNetLine->LineSendData(&ctrl_rsp, 1);

						SetCheckTimer(TIMER_CLEAR);
						return AMS_ERR;
					}			
					else if (recv_code == DATA_ENQ)
					{
						SetCheckTimer(TIMER_CLEAR);
						return AMS_OK;
					}
				}
				break;

			case AMS_SEND:
				{
					if (recv_code == DATA_EOT)
					{
						LOG(Error, _T("AMS Received EOT"));

						// Send EOT
						//ctrl_rsp = DATA_EOT;
						//m_pNetLine->LineSendData(&ctrl_rsp, 1);

						SetCheckTimer(TIMER_CLEAR);
						return AMS_ERR;
					}
					else if (recv_code == DATA_ACK)
					{
						SetCheckTimer(TIMER_CLEAR);
						return AMS_OK;
					}
/*					else if (recv_code == DATA_NAK)
					{
						LOG(Info, _T("AMS Received NAK : (AMS_SEND)"));

						SetCheckTimer(TIMER_CLEAR);

						if (rty_cnt > max_rty_cnt)
						{
							LOG(Error, _T("AMS Received NAK Retry Over - Error Return : (AMS_SEND)"));

							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);

							SetCheckTimer(TIMER_CLEAR);
							return AMS_ERR;
						}

						// wait
						CUtil::Sleep_Wait(500);

						// ReSend Data.
						m_pNetLine->LineSendData(m_SendData, m_SendLen);

						// reset time out
						switch (m_LineType)	
						{
						case LINE_TCP:
							SetCheckTimer(TIMEOUT_NEGOTIATE);	break;
						default:
							SetCheckTimer(TIMEOUT_RECVDATA);	break;
						}
						rty_cnt++;
					}*/
				}
				break;

			case AMS_RECV:
				{
					if (recv_code == DATA_STX)
					{
						// move next state
						status = AMS_STX_IN;

						switch (m_LineType)	
						{
						case LINE_TCP:
							SetCheckTimer(nAMSTimeoutReceiveTcpIp);
							break;

						default:
							SetCheckTimer(TIMEOUT_RECVDATA);		
							break;
						}

						chk_bcc = 0;
						nDataLen = 0;
					}
					else if (recv_code == DATA_EOT)
					{
						LOG(Error, _T("AMS Received EOT : (AMS_RECV)"));

						SetCheckTimer(TIMER_CLEAR);
						return AMS_ERR;
					}
				}
				break;
			case AMS_STX_IN:
				{
					// get length.
					//if ((nDataLen == 0) && (m_RecvLen == (LEN_LENGTH+1)))	// STX와 Length사이에 구분자(0x1C)가 있기 때문에
					if ((m_RecvLen == (LEN_LENGTH+1)))	// STX와 Length사이에 구분자(0x1C)가 있기 때문에
					{
						// Get Length;
						// XOR 하기때문에 데이터가 EOT, ETX 등등이 올수 있어 길이를 구해 길이 만큼 Recv.
						memcpy(byLength, &m_RecvData[1], LEN_LENGTH);	// m_RecvData[0] = 0x1C
						strLength.Format(_T("%S"), byLength);
						nDataLen = CUtil::StringToInt(strLength);

						//LOG(Info, _T("AMS Received : (AMS_STX_IN) Data Length = [%d]"), nDataLen);

						if ((nDataLen > NETBUF_RECV_SIZE) || (nDataLen <= 0))
						{
							SetCheckTimer(TIMER_CLEAR);

							//ctrl_rsp = DATA_EOT;
							//m_pNetLine->LineSendData(&ctrl_rsp, 1);

							return AMS_ERR;
						}
					}

					//if ((recv_code == DATA_ETX) && ((nDataLen + LEN_LENGTH + 1) <= m_RecvLen))	// +1 => 0x1C
					if (recv_code == DATA_ETX)
					{
						//LOG(Info, _T("AMS Received ETX : (AMS_STX_IN) m_RecvLen = [%d]"), m_RecvLen);

						// XOR ETX.
						chk_bcc ^= recv_code;

						// move next state
						status = AMS_ETX_IN;
					}
					else
					{
						if (m_RecvLen < NETBUF_RECV_SIZE)
						{
							m_RecvData[m_RecvLen++] = recv_code;
							chk_bcc ^= recv_code;
						}
						else
						{
							// buffer full.
							SetCheckTimer(TIMER_CLEAR);

							// Send EOT
							//ctrl_rsp = DATA_EOT;
							//m_pNetLine->LineSendData(&ctrl_rsp, 1);
							return AMS_ERR;
						}
					}
				}
				break;

			case AMS_ETX_IN:
				{
					SetCheckTimer(TIMER_CLEAR);

					// check bcc
					if (chk_bcc == recv_code)
					{
						if (m_RecvLen == 0)
							return AMS_ERR;

						// Data 검증 완료 후 ACK 송신 후 OK를 return하도록 로직 수정
						// Send ACK
						ctrl_rsp = DATA_ACK;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						return AMS_OK;
					}
					else
					{
						LOG(Error, _T("AMS Received bcc : (AMS_ETX_IN) bcc error"));

						// Send EOT
						//ctrl_rsp = DATA_EOT;
						//m_pNetLine->LineSendData(&ctrl_rsp, 1);

						return AMS_ERR;
					}
				}
				break;
			default:
				break;
			}

		}
		else if (nresult == RECV_EVENT)
		{	
			switch(recv_code)
			{
			case CONNECT_EVT:
				{
					LOG(Info, _T("AMS EVENT : CONNECT_EVT"));

					if (status == AMS_DIAL)
					{
						SetCheckTimer(TIMER_CLEAR);
					
						return AMS_OK;
					}
					else if (status == AMS_ACCEPT)
					{
						SetCheckTimer(TIMER_CLEAR);

						return AMS_OK;
					}
				}
				break;

			case DISCONNECT_EVT:
				{
					LOG(Info, _T("AMS Receive EVENT : DISCONNECT_EVT"));

					SetCheckTimer(TIMER_CLEAR);
					return AMS_ERR;
				}
				break;				

			case TIMEOUT_EVT:
				{
					// Data 송/수신 State에서 Timeout 발생시에는 Retry 없으므로 변경
					if (status == AMS_DIAL || status == AMS_ACCEPT)
					{
						LOG(Info, _T("[DIAL ACCEPT] TIMEOUT"));
						SetCheckTimer(TIMER_CLEAR);
						return AMS_ERR;
					}
					else if (status == AMS_ENQ)
					{
						SetCheckTimer(TIMER_CLEAR);

						if (rty_cnt > max_rty_cnt)
						{
							SetCheckTimer(TIMER_CLEAR);

							LOG(Error, _T("[AMS_ENQ] Retry Over - Error Return"));

							// Send EOT
//							ctrl_rsp = DATA_EOT;
//							m_pNetLine->LineSendData(&ctrl_rsp, 1);
							return AMS_ERR;
						}

						// wait
						CUtil::Sleep_Wait(500);

						LOG(Error, _T("[AMS_ENQ] ENQ Retry"));

						// Send ENQ
						ctrl_rsp = DATA_ENQ;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						// reset time out
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt++;
					}
					else if (status == AMS_CONN)
					{
						// Connect 상태에서 Timeout시 Error Return
						SetCheckTimer(TIMER_CLEAR);

						LOG(Error, _T("AMS Receive EVENT : TIMEOUT_EVT (AMS_CONN)"));

						// Send EOT
//						ctrl_rsp = DATA_EOT;
//						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						return AMS_ERR;
					}
					else if (status == AMS_SEND)
					{
						SetCheckTimer(TIMER_CLEAR);

						LOG(Error, _T("AMS Receive EVENT : TIMEOUT_EVT (AMS_SEND)"));

						// Send EOT
//						ctrl_rsp = DATA_EOT;
//						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						return AMS_ERR;
					}
					else if (status == AMS_RECV || status == AMS_STX_IN || status == AMS_ETX_IN)
					{
						SetCheckTimer(TIMER_CLEAR);

						LOG(Error, _T("AMS Receive EVENT : TIMEOUT_EVT (AMS_RECV)(AMS_STX_IN)(AMS_ETX_IN)"));
			
						// Send EOT
//						ctrl_rsp = DATA_EOT;
//						m_pNetLine->LineSendData(&ctrl_rsp, 1);
						return AMS_ERR;
					}
				}
				break;

			default:
				break;
			}
		}
		else
		{
			CUtil::Sleep_Wait(10);
		}
	}

	return AMS_ERR;
}


/** ***********************************************************************************
*	@brief AMS로 DATA 전송
*	@param BYTE *pSendBuf		Send Data 
*	@param int Length			Data Length 
*	@param LPCTSTR pDestInfo	AMS 정보(IP or PhoneNumber) 
*	@param LPCTSTR port			AMS Port 번호 
*	@param LPCTSTR option		SSL 옵션 
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
int CAMSCtrl::SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	BYTE	bcc = 0;
	int		nResult;
	
	ClearQueue();
	SetCheckTimer(TIMER_CLEAR);

	if (Length > 1)
	{
		int	i;

		// [PCI-SSF Fix] m_SendData(NETBUF_SEND_SIZE) 버퍼 오버플로우 방지를 위한 길이 사전 검증
		if ((Length + 3) > NETBUF_SEND_SIZE)
		{
			LOG(Error, _T("AMS SendData Length invalid (%d)"), Length);
			return AMS_ERR;
		}

		// send data buffer clear
		memset(m_SendData, 0, NETBUF_SEND_SIZE);
		m_SendLen = 0;

		// make send data and save send buffer
		m_SendData[0] = DATA_STX;
		for (i = 0; i < Length; i++)
		{
			m_SendData[i+1] = pSendBuf[i];
			bcc ^= m_SendData[i+1];
		}
		m_SendData[i+1] = DATA_ETX;
		bcc ^= m_SendData[i+1];
		m_SendData[i+2] = bcc;

		m_SendLen = Length + 3;

#if (SEND_RECV_DEBUG_AMS)
		LOG(Info, _T("AMS Binary m_SendLen:[%d], BCC : [0x%02X]"), m_SendLen, bcc);
#endif

		//LOG(Info, _T("AMS Send Data - Start"))	// Debug

		nResult = m_pNetLine->LineSendData(m_SendData, m_SendLen);

		// For Debug
		//LOG(Info, _T("AMS Send Data - Done"))	// Debug

		if (!nResult)
		{
			return Flow_Matrix(AMS_SEND);
			//int nResult = Flow_Matrix(AMS_SEND);
			//LOG(Info, _T("AMS Send Data - Ack Received"))	// Debug
			//return nResult;
		}
	}
	else
	{
		return m_pNetLine->LineSendData(pSendBuf, Length);
	}


	return AMS_ERR;
}

/** ***********************************************************************************
*	@brief AMS로 DATA 전송
*	@param BYTE *pRecvBuf		Receive Data
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
int	CAMSCtrl::RecvData(BYTE *pRecvBuf, int nBufSize)
{
	int nResult = 0;

	SetCheckTimer(TIMER_CLEAR);

	// receive data buffer clear
	memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	m_RecvLen = 0;

	nResult = Flow_Matrix(AMS_RECV);

	if (nResult == AMS_OK)
	{
		//LOG(Info, _T("AMS Data Received => Length = [%d]"), m_RecvLen);

		// [PCI-SSF Fix] 목적지 버퍼(pRecvBuf) 크기를 넘어서는 복사를 방지
		int nCopyLen = m_RecvLen - 1;

		if (nCopyLen > nBufSize)
		{
			LOG(Error, _T("CAMSCtrl::RecvData copy length(%d) > nBufSize(%d) - truncated"), nCopyLen, nBufSize);
			nCopyLen = nBufSize;
		}

		if (nCopyLen > 0)
			memcpy(pRecvBuf, &m_RecvData[1], nCopyLen);	// m_RecvData[0] = 0x1C

		return nCopyLen;
	}
	else
	{
		LOG(Info, _T("Flow_Matrix Error. Return: [%d]"), nResult);
	}

	return (m_RecvLen-1);
}
