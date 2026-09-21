// StandardCtrl.cpp: implementation of the CStandardCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#define NH_DEBUG

#include ".\Common\NHDbgApi.h"
#include "StandardCtrl.h"
#include ".\Dev\DevDefine.h"	// [#358] [NH] KSK 2008.7.10

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define		CONN_TIMEOUT	20
#define		DATA_TIMEOUT	120

// [#358] [NH] KSK 2008.7.10
/*
#define		INIT_STS		0
#define		CONN_STS		1
#define		HIGHLENIN_STS	2
#define		LOWLENIN_STS	3
*/
// end of [#358]

CStandardCtrl::CStandardCtrl(CLineCtrl *line) : CFlowCtrl(line)
{
	// [#2012] NH KJW 2011.02.09
	//memset(m_RecvData, 0, sizeof(m_RecvData));
	//memset(m_SendData, 0, sizeof(m_SendData));
	// end of [#2012]
}

CStandardCtrl::~CStandardCtrl()
{

}

int CStandardCtrl::SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	WORD	tempLen = (WORD)(Length);
	int		nreturn = 0;

	// queue clear
	ClearQueue();

	// receive data buffer clear
	// [#2012] NH KJW 2011.02.09
	//memset(m_RecvData, 0, sizeof(m_RecvData));
	memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	// end of [#2012]
	m_RecvLen = 0;

	// send data buffer clear
	// [#2012] NH KJW 2011.02.09
	//memset(m_SendData, 0, sizeof(m_SendData));
	memset(m_SendData, 0, NETBUF_SEND_SIZE);
	// end of [#2012]
	m_SendLen = 0;

	m_SendData[0] = (tempLen&0xFF00) >> 8;
	m_SendData[1] = tempLen & 0x00FF;
	memcpy(&m_SendData[2], pSendBuf, Length);
	m_SendLen = Length+2;

	// make complete
	NHDEBUG(1, (_T("HIGH[%x]\n"), m_SendData[0]));
	NHDEBUG(1, (_T("LOW[%x]\n"), m_SendData[1]));	
	NHDEBUG(1, (_T("SET SEND[%S][LEN:%d]\n"), &m_SendData[2], m_SendLen));

	// [#2075] NH KSK 2011.06.27
	if (option == Int2Asc(RKT_MODE_R0) || option == Int2Asc(RKT_MODE_R1) || option == Int2Asc(RKT_MODE_R2))
	{
		// Flow Matrix Start
		nreturn = Flow_Matrix(RKT_SEND_STS, Asc2Int(option));
		return nreturn;
	}
	// end of [#2075]

	nreturn = m_pNetLine->LineOpen(pDestInfo, port, option);
	if (!nreturn)
	{
		// success
		// timer set(connected)

NHDEBUG(1, (_T("STANDARD FLOW MATRIX START\n")));
		SetCheckTimer(CONN_TIMEOUT);
		// Flow Matrix Start
		nreturn = Flow_Matrix();
	}

	// line open fail
	m_pNetLine->LineClose();
	return nreturn;
	
}

int CStandardCtrl::Flow_Matrix(int initStatus, int nMode)
{
//	int		status		= INIT_STS;
	int		status		= initStatus;	// [#2075] NH KSK 2011.06.27
	int		snd_rty		= 0;
	int		nak_rty		= 0;
	WORD	data_len	= 0;
	BYTE	recv_code	= 0;
	int		nresult		= 0;
	BYTE	ctrl_rsp	= 0;
	// AP Holding ??? ?? max timeout? ????.
	DWORD	max_timeout = GetTickCount() + (180*1000);

	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	INIT_STS);			// [#343] KSK 2008.6.26 - SAVE NET FLOW

	while(GetTickCount() < max_timeout)
	{
		nresult = GetQueue(&recv_code);

		if (nresult == RECV_DATA)
		{
			switch(status)
			{
			case INIT_STS:
				break;
			case CONN_STS:
				NVDump('O', 'K', "01", L"", L"HIGHLENRECV" );
				NHDEBUG(1, (_T("FLOW HIGH LENGTH RECEIVE[%x]\n"), recv_code));
				data_len = (WORD)recv_code << 8;					// [CS#69] NH AIREAT 2008.3.14 cast (WORD)	
				status = HIGHLENIN_STS;
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	HIGHLENIN_STS);			// [#343] KSK 2008.6.26 - SAVE NET FLOW
				break;
			case HIGHLENIN_STS:
				{
					NVDump('O', 'K', "01", L"", L"LOWLENRECV" );
					NHDEBUG(1, (_T("FLOW LOWER LENGTH RECEIVE[%x]\n"), recv_code));
					data_len += (WORD)recv_code;						// [CS#69] NH AIREAT 2008.3.14 cast (WORD)

					CString expectData;
					expectData.Format(L"EXPECTS[%d]", data_len);
					NVDump('O', 'K', "01", L"", expectData);

					NHDEBUG(1, (_T("CONVERT HIGH LENGTH RECEIVE[%x]\n"), data_len));
					status = LOWLENIN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	LOWLENIN_STS);			// [#343] KSK 2008.6.26 - SAVE NET FLOW

					// check data length
					if (data_len >= QUEUE_SIZE)
					{
						// data size over
						// line close
						data_len = 0;
						ClearQueue();
						SetCheckTimer(TIMER_CLEAR);
						NVDump('F', 'K', "01", L"", L"LENERROR" );
						return 7;
					}
					break;
				}
			case LOWLENIN_STS:
				// check length
				//if (m_RecvLen >= NET_BUFF_SIZE)		// V01.02.29 BUFFER SIZE ?? REVIEW8 [#89] KSK 2008.04.14
				if (m_RecvLen >= NETBUF_RECV_SIZE)	// [#2012] NH KJW 2011.03.31
				{
					// logic error
					data_len = 0;
					ClearQueue();
					//TimerThreadStop();
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"LENERROR1" );
					return 7;
				}

				m_RecvData[m_RecvLen] = recv_code;
				m_RecvLen++;

				if (m_RecvLen == data_len)
				{
					// receive success
					NVDump('O', 'K', "01", L"", L"DATARECV" );
					NHDEBUG(1, (_T("FLOW DATA RECEIVE OK[%x]\n"), recv_code));
					//TimerThreadStop();
					SetCheckTimer(TIMER_CLEAR);
					// success
					return 0;
				}

				break;
			default:
				break;
			}

		}
		else if (nresult == RECV_EVENT)
		{			
			switch(status)
			{
			case INIT_STS:
				if (recv_code == CONNECT_EVT)
				{
					NVDump('O', 'K', "01", L"", L"CONNECT" );
					NHDEBUG(1, (_T("FLOW CONNECT[%x]\n"), recv_code));

					// [#2075] KSK 2011.06.27
					if (nMode == RKT_CONNECTION)
					{
						SetCheckTimer(TIMER_CLEAR);
						return 0;
					}
					// end of [#2075]

					SetCheckTimer(DATA_TIMEOUT);
					// DATA SEND
					status = CONN_STS;
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_NETFLOW_STATE,	CONN_STS);			// [#343] KSK 2008.6.26 - SAVE NET FLOW
					snd_rty++;
					//[#2207] US KSK 2013.07.04 Data Send Error? Retry? ?? D1500 ??? ???. (?? ?? ??)
//					m_pNetLine->LineSendData(m_SendData, m_SendLen);
					if (m_pNetLine->LineSendData(m_SendData, m_SendLen) != 0)	// ?? ???
					{
						Delay_Msg(3000);
						return 1;
					}
					// end of [#2207]

					NHDEBUG(1, (_T("DATA SEND[%S][LEN:%d]\n"), &m_SendData[2], m_SendLen));
					NVDump('O', 'K', "01", L"", L"DATASEND" );
				}
				else if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					     recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"DISRECV[I]" );
					NHDEBUG(1, (_T("INIT_DISC[%x]\n"), recv_code));
					return 1;
				}

				break;
			case CONN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					//TimerThreadStop();
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"DISRECV[C]" );
					return 7;
				}
				break;
			case HIGHLENIN_STS:
			case LOWLENIN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					NVDump('F', 'K', "01", L"", L"DISRECV[S]" );

					CString received;
					received.Format(L"RECVLEN[%d]", m_RecvLen);
					NVDump('O', 'K', "01", L"", received);

					// [#2012] NH KJW 2011.02.09
					//memset(m_RecvData, 0, sizeof(m_RecvData));
					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					// end of [#2012]
					m_RecvLen = 0;
					return 7;
				}
				break;
			default:
				break;
			}
		}
		else
		{
			// [#2075] NH KSK 2011.06.27
			if (status == RKT_SEND_STS)
			{
				SetCheckTimer(DATA_TIMEOUT);
				status = CONN_STS;
				snd_rty++;
				m_pNetLine->LineSendData(m_SendData, m_SendLen);
				NVDump('O', 'K', "01", L"", L"DATASEND" );	
			}
			// end of [#2075]
		}

		if(nresult == 0)	Delay_Msg(100);
	}
	NHDEBUG(1, (_T("MATRIX RETURN\n")));
	return 1;
}

// [#2075] NH KSK 2011.06.27
BOOL CStandardCtrl::RKTConnectCheck()
{
	return m_pNetLine->LineCallState(1);
}

int	CStandardCtrl::RKTConnectOpen(LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	int nReturn = 0;

	ClearQueue();

	nReturn = m_pNetLine->LineOpen(pDestInfo, port, option);

	if (!nReturn)
	{
		// Line Open Success
		nReturn = Flow_Matrix(INIT_STS, RKT_CONNECTION);

		if (nReturn)
		{
			m_pNetLine->LineClose();
		}
	}
	else
	{
		// LineOpen Error
		m_pNetLine->LineClose();
	}

	return nReturn;
}

int	CStandardCtrl::RKTConnectClose()
{
	m_pNetLine->LineClose();
	return FALSE;
}
// end of [#2075]
