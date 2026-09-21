#include "stdafx.h"
#include "RMSFlowCtrl.h"

//#define NH_DEBUG

#include ".\Common\NHDbgApi.h"
#include ".\Common\CmnLib.h"
#include <Winsock2.h>

enum RMS_STATUS
{
	RMS_INIT = 0,
	RMS_DIAL,			// RMS로 전화 걸때.
	RMS_ENQ,			// RMS로 전화 걸때. DAIL만.
	RMS_ACCEPT,			// RMS에서 전화 올때.
	RMS_CONN,			// RMS에서 전화 올때. DIAL만.
	RMS_SEND,
	RMS_RECV,
	RMS_STX_IN,
	RMS_ETX_IN
};

#define DBG_INFO		1
#define DBG_ERR			1

#define		DATA_STX		0x02
#define		DATA_ETX		0x03
#define		DATA_EOT		0x04
#define		DATA_ENQ		0x05
#define		DATA_ACK		0x06
#define		DATA_NAK		0x15

#define	SEND_EVT	'S'
#define RECV_EVT	'R'

#define	END_OK					0
#define END_ERR					1

#define TIMEOUT_CLEAR		    0
#define TIMEOUT_CONNECT			(60)					// [#2234] NH KMK 2013.11.21 MoniView Timeout값 변수로 변경	// 2013.01.10 Modem은 기존 define 값 사용함
#define TIMEOUT_ENQ				(7)						// [#305] NH AIREAT 2009.08.18 - 10초->35초
#define TIMEOUT_NEGOTIATE		(10)					// [#305] NH AIREAT 2008.06.16 - 5 -> 10 변경
#define TIMEOUT_RECVDATA		(30)					// [#2234] NH KMK 2013.11.21 MoniView Timeout값 변수로 변경	// 2013.01.10 Modem은 기존 define 값 사용함

CRMSFlowCtrl::CRMSFlowCtrl(CLineCtrl* clientLine, CLineCtrl *serverLine) : CFlowCtrl(clientLine)
{
	m_bOpen = FALSE;

	m_serverLine = serverLine;
	m_clientLine = clientLine;

	m_LineType = clientLine->GetLineType();
}

CRMSFlowCtrl::~CRMSFlowCtrl()
{
}

// RMS에서 접속을 위해 네트웍을 Server 기능을 수행하게 한다.
// TCP일경우 port 번호를 주어야 한다.
// option은 나중에 SSL 사용 여부 이다.
int	CRMSFlowCtrl::RMSConnectOpen(LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option, LPCTSTR OpenType)
{
	int	nResult = RMS_ERR;

	if (!m_bOpen)
	{
		// Close both lines
		m_serverLine->LineClose();
		m_clientLine->LineClose();

		if (!wcscmp(L"LISTEN", OpenType))
		{
			// Set the active line to the server type
			m_pNetLine = m_serverLine;
			m_LineType = m_pNetLine->GetLineType();

			if (m_LineType == LINE_TCP)
			{
				nResult = m_pNetLine->LineOpen(pDestInfo, port, option, L"1");
			}
			else
			{
				m_bOpen = TRUE; // 실제로 Open이 Line 성공하였으나 실패로 넘어오는 경우가 있어서 위치 옮김 (TCP/IP는 위치 옮기면 접속 안됨)
				                // LineClose 시 m_bOpen이 FALSE여서 RMS랑 계속 연결 되어 있는 현상 발생함 (SOOK 2010.10.28)
				nResult = m_pNetLine->LineOpen(pDestInfo, port, option, L"1");//LineOpen은 성공하였으나 실패로 리턴되는 경우 있어 보완 처리함 (추가확인 필요)
			}
			
			if (!nResult)
			{
				ClearQueue();
				SetCheckTimer(TIMEOUT_CLEAR);
				m_bOpen = TRUE;
				return RMS_OK;
			}
			
		}
		else if (!wcscmp(L"CONNECT", OpenType))
		{
			// Set the active line to the client type
			m_pNetLine = m_clientLine;
			m_LineType = m_pNetLine->GetLineType();

			ClearQueue();
			SetCheckTimer(TIMEOUT_CLEAR);

			if (m_LineType == LINE_TCP || m_LineType == LINE_TLS)
			{
//				nResult = m_pNetLine->LineOpen(pDestInfo, port, option, L"0");
				nResult = m_pNetLine->LineOpen(pDestInfo, port, option, L"2");	// [#2234] NH KSK 2014.03.03 TCP Timeout 조정을 위해 Option 2 추가
			}
			else
			{
				m_bOpen = TRUE; // 실제로 Open이 Line 성공하였으나 실패로 넘어오는 경우가 있어서 위치 옮김 (TCP/IP는 위치 옮기면 접속 안됨)
				// LineClose 시 m_bOpen이 FALSE여서 RMS랑 계속 연결 되어 있는 현상 발생함 (SOOK 2010.10.28)
				nResult = m_pNetLine->LineOpen(pDestInfo, port, option, L"0");
				if (nResult == RMS_OK)
					nResult = Flow_Matrix(RMS_DIAL);
			}

			if (nResult == RMS_OK)
			{
				ClearQueue();
				SetCheckTimer(TIMEOUT_CLEAR);
				m_bOpen = TRUE;
				return RMS_OK;
			}
		}
		
	}

	return RMS_ERR;
}

int	CRMSFlowCtrl::RMSConnectClose()
{
	if (m_bOpen)
	{
		if (!m_pNetLine->LineClose())
		{
			m_bOpen = FALSE;
			return RMS_OK;
		}
	}

	// Reset the current line to NULL
	m_pNetLine = NULL;

	return RMS_ERR;
}

// Dialup : 전화 벨이 울리고 있는지.
// TCP : listen 된 소켓이 있는지.
// 검사한다.
BOOL CRMSFlowCtrl::RMSConnectCheck()
{
	if (!m_bOpen)
		return FALSE;

	if (m_LineType == LINE_DIALUP)
		return (!m_pNetLine->LineCallState());

	return (m_pNetLine->LineCallState());
}

int CRMSFlowCtrl::RMSConnectAccept()
{
	int nResult;

	nResult = m_pNetLine->LineAccept((BYTE)1);
	// TCP와 Dial의 리턴값이 다름..
	if (m_LineType == LINE_TCP || m_LineType == LINE_TLS)
		nResult = !nResult;

	if (!nResult)
	{
		// Dial 만 수행 한다.
		if (m_LineType == LINE_DIALUP)
		{	
			nResult = Flow_Matrix(RMS_ACCEPT);

			if (nResult != RMS_OK)
			{
				RMSConnectClose();
			}

			return nResult;
		}
		else
			return RMS_OK;
	}

	RMSConnectClose();

	return RMS_ERR;
}

int	CRMSFlowCtrl::SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	BYTE	bcc = 0;
	int nResult;

	ClearQueue();
	SetCheckTimer(TIMEOUT_CLEAR);
	
	if (Length > 1)
	{
		int	i;
		
		// send data buffer clear
		// [#2012] NH KJW 2011.02.09
		//memset(m_SendData, 0, sizeof(m_SendData));
		memset(m_SendData, 0, NETBUF_SEND_SIZE);
		// end of [#2012]
		m_SendLen = 0;
		
		// make send data and save send buffer
		m_SendData[0] = DATA_STX;
		for (i = 0; i < Length; i++)
		{
			m_SendData[i+1] = pSendBuf[i];
			bcc ^= pSendBuf[i];
		}
		bcc ^= DATA_ETX;
		m_SendData[i+1] = DATA_ETX;
		m_SendData[i+2] = bcc;
		
		m_SendLen = Length + 3;
		
		nResult = m_pNetLine->LineSendData(m_SendData, m_SendLen);
		
		// TCP와 Dial의 리턴값이 다름..
		if (m_LineType == LINE_TCP && nResult)
			nResult = 0;

		if (!nResult)
		{
			return Flow_Matrix(RMS_SEND); // V01.02.25
		}
	}
	else
	{
		return m_pNetLine->LineSendData(pSendBuf, Length);
	}


	return RMS_ERR;
}

int	CRMSFlowCtrl::RecvData(BYTE *pRecvBuf, int *Length, int WaitSec)
{

	SetCheckTimer(TIMEOUT_CLEAR);

	// receive data buffer clear
	// [#2012] NH KJW 2011.02.09
	//memset(m_RecvData, 0, sizeof(m_RecvData));
	memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	// end of [#2012]
	m_RecvLen = 0;

	if (Flow_Matrix(RMS_RECV) == RMS_OK)
	{
		NHDEBUG(DBG_INFO, (L"RMS Data Received => Length = [%d] \n", m_RecvLen));

		// [#GLDV-2529] US Kook 2019.07.31 check buffer boundary to prevent buffer overflow attack.
		// currently, size of network buffer is 300K and RMS buffer must be written within its size, 10K.
		//memcpy(pRecvBuf, m_RecvData, m_RecvLen);
		//*Length = m_RecvLen;
		memcpy(pRecvBuf, m_RecvData,  __min(m_RecvLen, *Length));
		*Length = __min(m_RecvLen, *Length);
		// end of [#GLDV-2529]

		return RMS_OK;
	}

	NHDEBUG(DBG_INFO, (L"RMS_ERR\n"));

	return RMS_ERR;
}


// [#221] CSW 2008.5.21
// RMS STATUS 보완 처리
int	CRMSFlowCtrl::Flow_Matrix(int initStatus)
{
	int		statussave = RMS_INIT;
	int		status		= initStatus;
	int		rty_cnt		= 0;
	BYTE	ctrl_rsp	= 0;
	BYTE	chk_bcc		= 0;
	BYTE	recv_code	= 0;
	int		nresult		= 0;
	int		nMinRecv	= 0;
	int		max_rty_cnt = 5;

	int nRMSTimeoutReceiveTcpIp = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_RECEIVE_TCPIP);		// [#2234] NH KMK 2013.11.21 MoniView Timeout
	CString	strTemp;	// [#2257] NH KSK 2014.03.03

	// [#283] NH AIREAT 2008.06.11 - TCP/IP일 경우 Retry 없음.
	if (m_LineType == LINE_TCP)
		max_rty_cnt = -1;
	// end of [#283]

	// AP Holding 방지를 위해 max timeout을 설정한다. -- Max 3분.
	DWORD	max_timeout = GetTickCount() + (180*1000);

	// [#2234] NH KMK 2014.01.10 MoniView Timeout (TCP/IP)
	// 기존 로직 정리 (왜 호주만 CONNECT/RECV 구분없이 60초 대기하게 했는지?)
// 	//[#567] Symstream 장비 접속시 MoniView에 접속 하기 전에 Timeout 발행하여
// 	// 호주 Customers 만 변경함
// #if (AU_VERSION)
// 	SetCheckTimer(TIMEOUT_CONNECT);
// #else
	// end of [#2234]
	if (status == RMS_DIAL || status == RMS_ACCEPT)
	{
		// [#2234] NH KSK 2014.03.03 Status Send일 경우에는 MODEM에서만 발생하므로 TCP/IP는 코딩 안함
 		SetCheckTimer(TIMEOUT_CONNECT);
		NHDEBUG(DBG_INFO, (L"RMS_DIAL || RMS_ACCEPT\n"));
		NVDump('O', 'K', "01", L"", L"R_D R_A Call" );	// [#2257] NH KSK 2014.03.03
	}
	else if (status == RMS_RECV)
	{
		// [#2234] NH KMK 2014.01.10 MoniView Timeout (TCP/IP, Receive)
// 		SetCheckTimer(TIMEOUT_RECVDATA);
		switch (m_LineType)	{
			case LINE_TCP:
				SetCheckTimer(nRMSTimeoutReceiveTcpIp);	break;
			default:
				SetCheckTimer(TIMEOUT_RECVDATA);	break;
		}
		strTemp.Format(L"R_RECV:%d", nRMSTimeoutReceiveTcpIp);	// [#2257] NH KSK 2014.03.03
		NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03
		// end of [#2234]
	}
	else
		SetCheckTimer(TIMEOUT_NEGOTIATE);
// [#2234] NH KMK 2014.01.13 
// #endif //end of [#567]
// end of [#2234]
	while(GetTickCount() < max_timeout)
	{
		nresult = GetQueue(&recv_code);
		
		if (nresult == RECV_DATA)
		{
			switch(status)
			{
			case RMS_ENQ:
				{
					NHDEBUG(DBG_INFO, (L"RMS Received : RMS_ENQ\n"));

					if (recv_code == DATA_EOT)
					{
						// Send EOT
						ctrl_rsp = DATA_EOT;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_ERR;
					}
					else if (recv_code == DATA_ACK)
					{
						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_OK;
					}
					else if (recv_code == DATA_NAK)
					{
						strTemp.Format(L"R_RECV:NAK");	// [#2257] NH KSK 2014.03.03
						NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

						SetCheckTimer(TIMEOUT_CLEAR);

						if (rty_cnt > max_rty_cnt)
						{
							strTemp.Format(L"R_SND:EOT");	// [#2257] NH KSK 2014.03.03
							NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);

							SetCheckTimer(TIMEOUT_CLEAR);
							return RMS_ERR;
						}

						// wait
						Delay_Msg(500);
						
						strTemp.Format(L"R_SND:ENQ");	// [#2257] NH KSK 2014.03.03
						NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

						// Send ENQ
						ctrl_rsp = DATA_ENQ;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);
						
						// reset time out
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt++;
					}
				}
				break;

			case RMS_CONN:
				{
					NHDEBUG(DBG_INFO, (L"RMS Received : RMS_CONN\n"));

					if (recv_code == DATA_EOT)
					{
						strTemp.Format(L"R_RCV:EOT");	// [#2257] NH KSK 2014.03.03
						NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

						strTemp.Format(L"R_SND:EOT");	// [#2257] NH KSK 2014.03.03
						NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

						// Send EOT
						ctrl_rsp = DATA_EOT;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_ERR;
					}
					else if (recv_code == DATA_ENQ)
					{
						// Send ACK
						ctrl_rsp = DATA_ACK;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_OK;
					}
				}
				break;
				
			case RMS_SEND:
				{
					NHDEBUG(DBG_INFO, (L"RMS Received : RMS_SEND\n"));

					if (recv_code == DATA_EOT)
					{
						NHDEBUG(DBG_INFO, (L"RMS Received : recv_code == DATA_EOT\n"));

						strTemp.Format(L"R_RCV:EOT");	// [#2257] NH KSK 2014.03.03
						NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

						strTemp.Format(L"R_SND:EOT");	// [#2257] NH KSK 2014.03.03
						NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03
						// Send EOT
						ctrl_rsp = DATA_EOT;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_ERR;
					}
					else if (recv_code == DATA_ACK)
					{
						NHDEBUG(DBG_INFO, (L"RMS Received : recv_code == DATA_ACK, m_RecvLen = [%d]\n", m_RecvLen));
						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_OK;
					}
					else if (recv_code == DATA_NAK)
					{
						NHDEBUG(DBG_INFO, (L"RMS Received : recv_code == DATA_NAK\n"));

						strTemp.Format(L"R_RCV:NAK");	// [#2257] NH KSK 2014.03.03
						NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

						SetCheckTimer(TIMEOUT_CLEAR);

						if (rty_cnt > max_rty_cnt)
						{
							strTemp.Format(L"R_SND:EOT");	// [#2257] NH KSK 2014.03.03
							NVDump('O', 'K', "01", L"", strTemp );	// [#2257] NH KSK 2014.03.03

							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);

							SetCheckTimer(TIMEOUT_CLEAR);
							return RMS_ERR;
						}

						// wait
						Delay_Msg(500);

						// ReSend Data.
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
						
						// reset time out
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt++;
					}
				}
				break;

			case RMS_RECV:
				{
					NHDEBUG(DBG_INFO, (L"RMS Received : RMS_RECV\n"));

					if (recv_code == DATA_STX)
					{
						NHDEBUG(DBG_INFO, (L"RMS_RECV-->STX RECEIVE\n"));
						// move next state
						status = RMS_STX_IN;

						// [#2234] NH KMK 2014.01.15 Moniview Timeout
// 						SetCheckTimer(TIMEOUT_RECVDATA);
						switch (m_LineType)	{
							case LINE_TCP:
								SetCheckTimer(nRMSTimeoutReceiveTcpIp);		break;
							default:
								SetCheckTimer(TIMEOUT_RECVDATA);		break;
						}
						// end of [#2234]

						chk_bcc = 0;
						nMinRecv = 0;
					}
					else if (recv_code == DATA_EOT)
					{
						NHDEBUG(DBG_INFO, (L"RMS Received : recv_code == DATA_EOT\n"));

						m_RecvData[m_RecvLen++] = recv_code;

						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_OK;
					}
				}
				break;
			case RMS_STX_IN:
				{
					// get length.
					if ((nMinRecv == 0) && (m_RecvLen == 2))
					{
						// Get Length;
						// XOR 하기때문에 데이터가 EOT, ETX 등등이 올수 있어 길이를 구해 길이 만큼 Recv.
						nMinRecv = (((int)m_RecvData[1] << 8) | ((int)m_RecvData[0]));		// [CS#72] NH AIREAT 2008.3.14 cast (int)
						
						NHDEBUG(DBG_INFO, (L"RMS Received : RMS_STX_IN1 Data Length = [%d]\n", nMinRecv));

						// [#2012] NH KJW 2011.02.09
						//if ((nMinRecv > sizeof (m_RecvData)) || (nMinRecv <= 0))
						if ((nMinRecv > NETBUF_RECV_SIZE) || (nMinRecv <= 0))
						// end of [#2012]
						{
							SetCheckTimer(TIMEOUT_CLEAR);

							if (rty_cnt > max_rty_cnt)
							{
								// Send NAK
								ctrl_rsp = DATA_EOT;
								m_pNetLine->LineSendData(&ctrl_rsp, 1);
								
								return RMS_ERR;
							}

							// 2007.11.27 V01.02.19
							// NAK를 보낼 경우 queue clear 및 buffer를 clear해야한다
							Delay_Msg(500);
							// [#2012] NH KJW 2011.02.09
							//memset(m_RecvData, 0, sizeof(m_RecvData));
							memset(m_RecvData, 0, NETBUF_RECV_SIZE);
							// end of [#2012]
							m_RecvLen = 0;
							ClearQueue();
							//--

							// Send NAK
							ctrl_rsp = DATA_NAK;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);

							// move next
							status = RMS_RECV;
							SetCheckTimer(TIMEOUT_NEGOTIATE);
							rty_cnt++;
						}
					}
					
					if ((recv_code == DATA_ETX) && ((nMinRecv + 2) <= m_RecvLen))
					{
						NHDEBUG(DBG_INFO, (L"RMS Received : RMS_STX_IN2 : (recv_code == DATA_ETX) && ((nMinRecv + 2) <= m_RecvLen),  m_RecvLen = [%d]\n", m_RecvLen));

						// XOR ETX.
						chk_bcc ^= recv_code;

						// move next state
						status = RMS_ETX_IN;
					}
					else
					{
						// check max
						// [#2012] NH KJW 2011.02.09
						//if (m_RecvLen < sizeof (m_RecvData))
						if (m_RecvLen < NETBUF_RECV_SIZE)
						// end of [#2012]
						{
							m_RecvData[m_RecvLen++] = recv_code;
							chk_bcc ^= recv_code;
						}
						else
						{
							// buffer full.
							SetCheckTimer(TIMEOUT_CLEAR);
							
							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);
							return RMS_ERR;
						}
					}
				}
				break;

			case RMS_ETX_IN:
				{

					NHDEBUG(DBG_INFO, (L"RMS Received : RMS_ETX_IN :  m_RecvLen = [%d]\n", m_RecvLen ));

					SetCheckTimer(TIMEOUT_CLEAR);

					// check bcc
					if (chk_bcc == recv_code)
					{
						if (m_RecvLen == 0)
							return RMS_ERR;

						return RMS_OK;
					}

					if (rty_cnt > max_rty_cnt)
					{
						// Send NAK
						ctrl_rsp = DATA_EOT;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);
						
						return RMS_ERR;
					}

					// 2007.11.27 V01.02.19
					// NAK를 보낼 경우 queue clear 및 buffer를 clear해야한다
					Delay_Msg(500);
					// [#2012] NH KJW 2011.02.09
					//memset(m_RecvData, 0, sizeof(m_RecvData));
					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					// end of [#2012]
					m_RecvLen = 0;
					ClearQueue();
					//--

					// Send NAK
					ctrl_rsp = DATA_NAK;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);

					// move next
					status = RMS_RECV;
					SetCheckTimer(TIMEOUT_NEGOTIATE);
					rty_cnt++;
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
					NHDEBUG(DBG_INFO, (L"RMS Receive EVENT : CONNECT_EVT \n" ));

					if (status == RMS_DIAL)
					{
						SetCheckTimer(TIMEOUT_CLEAR);

						if (m_LineType == LINE_TCP)
							return RMS_OK;

						// wait
						Delay_Msg(1500);

						// Send ENQ
						ctrl_rsp = DATA_ENQ;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);
						
						// move Next State
						status = RMS_ENQ;
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt = 0;						
					}
					else if (status == RMS_ACCEPT)
					{
						SetCheckTimer(TIMEOUT_CLEAR);

						if (m_LineType == LINE_TCP)
							return RMS_OK;
						
						// move next state
						status = RMS_CONN;
						SetCheckTimer(TIMEOUT_ENQ);					// [#305] NH AIREAT 2008.06.16
						rty_cnt = 0;
					}
				}
				break;

			case DISCONNECT_EVT:
				{
					NHDEBUG(DBG_INFO, (L"RMS Receive EVENT : DISCONNECT_EVT \n" ));

					SetCheckTimer(TIMEOUT_CLEAR);
					RMSConnectClose(); // V01.02.25
					return RMS_ERR;
				}
				break;				

			case TIMEOUT_EVT:
				{
					NHDEBUG(DBG_INFO, (L"RMS Receive EVENT : TIMEOUT_EVT \n" ));

					if (status == RMS_DIAL || status == RMS_ACCEPT)
					{
						NHDEBUG(DBG_INFO, (L"[DIAL ACCEPT] TIMEOUT\n"));
						SetCheckTimer(TIMEOUT_CLEAR);
						return RMS_ERR;
					}
					else if (status == RMS_ENQ)
					{
						SetCheckTimer(TIMEOUT_CLEAR);				// [#305] NH AIREAT 2008.06.16

						if (rty_cnt > max_rty_cnt)
						{
							SetCheckTimer(TIMEOUT_CLEAR);
							
							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);
							return RMS_ERR;
						}

						// wait
						Delay_Msg(500);
						
						// Send ENQ
						ctrl_rsp = DATA_ENQ;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);
						
						// reset time out
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt++;
					}
					else if (status == RMS_CONN)
					{
						// RETRY 추가.
						if (rty_cnt < max_rty_cnt)
						{
							// wait
							Delay_Msg(500);
							
							// Send NAK
							ctrl_rsp = DATA_NAK;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);
							
							// reset time out
							SetCheckTimer(TIMEOUT_ENQ);
							rty_cnt++;
							break;
						}

						// [#305] NH AIREAT 2008.06.16
						// Retry 없앰. Timeout 5 -> 35로 변경.
						// EOT 보내기전 NAK 루틴 추가.

						SetCheckTimer(TIMEOUT_CLEAR);
						
						// Send NAK
						ctrl_rsp = DATA_NAK;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);
						
						// Send EOT
						ctrl_rsp = DATA_EOT;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);

						// end of [#305]

						NHDEBUG(DBG_INFO, (L"[RMS_CONN] TIMEOUT\n"));

						return RMS_ERR;
					}
					else if (status == RMS_SEND)
					{
						SetCheckTimer(TIMEOUT_CLEAR);				// [#305] NH AIREAT 2008.06.16

						if (rty_cnt > max_rty_cnt)
						{
							SetCheckTimer(TIMEOUT_CLEAR);
							
							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);
							return RMS_ERR;
						}

						// wait
						Delay_Msg(500);
						
						// Send Data.
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
						
						// reset time out
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt++;
					}
					else if (status == RMS_RECV || status == RMS_STX_IN || status == RMS_ETX_IN)
					{
						NHDEBUG(DBG_INFO, (L"TIMEOUT EVENT --> status(%d)\n", status));
						SetCheckTimer(TIMEOUT_CLEAR);				// [#305] NH AIREAT 2008.06.16

						if (rty_cnt > max_rty_cnt)
						{
							SetCheckTimer(TIMEOUT_CLEAR);
							
							NHDEBUG(DBG_ERR, (L"status(%d), retry over.. RMS_ERR\n", status));
							// Send EOT
							ctrl_rsp = DATA_EOT;
							m_pNetLine->LineSendData(&ctrl_rsp, 1);
							return RMS_ERR;
						}
									
						// NAK를 보낼 경우 queue clear 및 buffer를 clear해야한다
						Delay_Msg(500);
						// [#2012] NH KJW 2011.02.09
						//memset(m_RecvData, 0, sizeof(m_RecvData));
						memset(m_RecvData, 0, NETBUF_RECV_SIZE);
						// end of [#2012]
						m_RecvLen = 0;
						ClearQueue();

						// Send NAK
						ctrl_rsp = DATA_NAK;
						m_pNetLine->LineSendData(&ctrl_rsp, 1);
						
						// reset time out
						status = RMS_RECV;
						SetCheckTimer(TIMEOUT_NEGOTIATE);
						rty_cnt++;
					}
				}
				break;

			default:
				break;
			}
		}
		else
			Delay_Msg(5);					// [#305] NH AIREAT 2008.06.16
	}

	return RMS_ERR;
}