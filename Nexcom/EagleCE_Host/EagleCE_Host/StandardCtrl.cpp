/** **********************************************************************************
*	@file StandardCtrl.cpp
*	@date 2017/10/20	
*	@author MFS
*	@brief Standard프로토콜을 구현한 소스파일입니다. 
**************************************************************************************/

// StandardCtrl.cpp: implementation of the CStandardCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StandardCtrl.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CStandardCtrl::CStandardCtrl(int LineType)
{
	m_pNetLine = CLineCtrl::Instance(LineType);
}

CStandardCtrl::~CStandardCtrl()
{

}

int CStandardCtrl::SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	WORD	tempLen = (WORD)(Length);
	int		nreturn = 0;

	LOG(Info, _T("Standard Flow SendData Call"));

	// [PCI-SSF Fix] m_SendData(NETBUF_SEND_SIZE) 버퍼 오버플로우 방지를 위한 길이 사전 검증
	if (Length <= 0 || (Length + 2) > NETBUF_SEND_SIZE)
	{
		LOG(Error, _T("Standard Flow SendData Length invalid (%d)"), Length);
		return 7;
	}

	m_bNetworkTest = FALSE;	// 변수 초기화

	if (strcmp((char*)pSendBuf, "NETWORK_TEST") == 0)
		m_bNetworkTest = TRUE;

	// queue clear
	ClearQueue();

	// receive data buffer clear
	memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	m_RecvLen = 0;

	// send data buffer clear
	memset(m_SendData, 0, NETBUF_SEND_SIZE);
	m_SendLen = 0;

	if (m_bNetworkTest == FALSE)
	{
		m_SendData[0] = (tempLen&0xFF00) >> 8;
		m_SendData[1] = tempLen & 0x00FF;
		memcpy(&m_SendData[2], pSendBuf, Length);
		m_SendLen = Length+2;
		// make complete
	}

	nreturn = m_pNetLine->LineOpen(pDestInfo, port, option);

	if (!nreturn)
	{
		// success
		SetCheckTimer(CONN_TIMEOUT_TCPIP);

		// Flow Matrix Start
		nreturn = Flow_Matrix();
	}

	// line close
	CloseLine();

	LOG(Info, _T("Standard Flow SendData Call end"));
	return nreturn;
	
}


BOOL CStandardCtrl::CloseLine()
{
	LOG(Info, _T("STandard Flow CloseLine Call"));

	m_pNetLine->LineClose();

	LOG(Info, _T("STandard Flow CloseLine Call end"));

	return TRUE;
}


int CStandardCtrl::Flow_Matrix(int initStatus, int nMode)
{
	int		status		= initStatus;
	int		snd_rty		= 0;
	int		nak_rty		= 0;
	WORD	data_len	= 0;
	BYTE	recv_code	= 0;
	int		nresult		= 0;
	BYTE	ctrl_rsp	= 0;
	// AP Holding 방지를 위해 max timeout을 설정한다.
	DWORD	max_timeout = GetTickCount() + (180*1000);

	LOG(Info, _T("Standard Flow Flow_Matrix Call"));

	m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));

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
				data_len = (WORD)recv_code << 8;
				status = HIGHLENIN_STS;
				m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));
				break;

			case HIGHLENIN_STS:
				data_len += (WORD)recv_code;
				status = LOWLENIN_STS;
				m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));

				LOG(Info, _T("Standard Flow Flow_Matrix Data Len (%d)"), data_len);

				// check data length
				if (data_len >= QUEUE_SIZE)
				{
					// data size over
					// line close
					data_len = 0;
					ClearQueue();
					SetCheckTimer(TIMER_CLEAR);
					return 7;
				}
				break;

			case LOWLENIN_STS:
				// check length
				if (m_RecvLen >= NETBUF_RECV_SIZE)
				{
					LOG(Info, _T("Standard Flow Flow_Matrix Recv Buffer Over Flow"));

					// logic error
					data_len = 0;
					ClearQueue();
					SetCheckTimer(TIMER_CLEAR);
					return 7;
				}

				m_RecvData[m_RecvLen] = recv_code;
				m_RecvLen++;

				if (m_RecvLen == data_len)
				{
					LOG(Info, _T("Standard Flow Flow_Matrix Data Receive OK"));
					// receive success
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
					LOG(Info, _T("Standard Flow Flow_Matrix Data Connect OK"));

					SetCheckTimer(DATA_TIMEOUT_STD);

					if (m_bNetworkTest == TRUE)
						return 0;

					// DATA SEND
					status = CONN_STS;
					m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));

					snd_rty++;
					if (m_pNetLine->LineSendData(m_SendData, m_SendLen) != 0)	// 전송 실패시
					{
						Sleep(3000);
						return 1;
					}

				}
				else if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					     recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					return 1;
				}

				break;

			case CONN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					return 7;
				}
				break;

			case HIGHLENIN_STS:
			case LOWLENIN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
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

		}

		if(nresult == 0)	Sleep(100);
	}

	return 1;
}
