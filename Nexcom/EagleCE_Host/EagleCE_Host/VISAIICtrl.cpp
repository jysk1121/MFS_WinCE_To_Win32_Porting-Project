/** **********************************************************************************
*	@file VISAIICtrl.cpp
*	@date 2017/10/20	
*	@author MFS
*	@brief VISAII프로토콜을 구현한 소스파일입니다. 
**************************************************************************************/




// VISAIICtrl.cpp: implementation of the CVISAIICtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VISAIICtrl.h"

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

CVISAIICtrl::CVISAIICtrl(int LineType)
{
	m_bPreDialStart = FALSE;

	m_pNetLine = NULL;
	m_pNetLine = CLineCtrl::Instance(LineType);
}

CVISAIICtrl::~CVISAIICtrl()
{

}
/** ***********************************************************************************
*	@brief 데이터를 VISAII의 사양에 맞게 만들고 플로우 매트릭스를 호출합니다. 
*	@param BYTE *pSendBuf 버퍼의 주소
*	@param int Length 버퍼의 길이 
*	@param LPCTSTR pDestInfo 아이피 주소 
*	@param LPCTSTR port 포트 주소 
*	@param LPCTSTR option SSL 옵션 
*	@retval 1 에러 
*	@retval nreturn 플로우 매트릭스 에러코드
***************************************************************************************/
int CVISAIICtrl::SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	BYTE	bcc = 0;
	int		nreturn = 0;

	m_bNetworkTest = FALSE;	// 변수 초기화

	LOG(Info, _T("VISA Flow SendData Call"));

	// [PCI-SSF Fix] m_SendData(NETBUF_SEND_SIZE) 버퍼 오버플로우 방지를 위한 길이 사전 검증
	if (Length <= 0 || (Length + 3) > NETBUF_SEND_SIZE)
	{
		LOG(Error, _T("VISA Flow SendData Length invalid (%d)"), Length);
		return 1;
	}

	if (strcmp((char*)pSendBuf, "NETWORK_TEST") == 0)
	{
		m_bNetworkTest = TRUE;
		m_bPreDialStart = FALSE;
	}

	if (m_bPreDialStart != TRUE)
		ClearQueue();				// queue clear

	memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	m_RecvLen = 0;

	memset(m_SendData, 0, NETBUF_SEND_SIZE);
	m_SendLen = 0;

	if (m_bNetworkTest == FALSE)
	{
		// make send data and save send buffer
		m_SendData[0] = 0x02;

		for(int i=0; i<Length; i++)
		{
			m_SendData[i+1] = pSendBuf[i];
			bcc ^= pSendBuf[i];
		}
		m_SendData[Length+1] = 0x03;
		bcc ^= m_SendData[Length+1];
		m_SendData[Length+2] = bcc;
		m_SendLen = Length+3;
		// make complete
	}

	if (m_bPreDialStart != TRUE)
	{
		if (m_pNetLine != NULL)
		{
			SetCheckTimer(CONN_TIMEOUT);
			nreturn = m_pNetLine->LineOpen(pDestInfo, port, option);
		}
		else
		{
			return 1;
		}
	}

	if (!nreturn)
	{
		// success
		nreturn = Flow_Matrix();
	}

	m_bPreDialStart = FALSE;
	CloseLine();

	LOG(Info, _T("VISA Flow SendData Call end"));

	return nreturn;
	
}

BOOL CVISAIICtrl::CloseLine()
{
	LOG(Info, _T("VISA Flow CloseLine Call"));

	m_pNetLine->LineClose();

	LOG(Info, _T("VISA Flow CloseLine Call end"));

	return TRUE;
}

int	CVISAIICtrl::PreDialStart(CString strHostPhoneNumber)
{
	int	nRetrun = 0;

	LOG(Info, _T("VISA Flow PreDialStart Call"));

	if (m_bPreDialStart == FALSE)
	{
		ClearQueue();

		m_bPreDialStart = TRUE;
		nRetrun = m_pNetLine->LineOpen(strHostPhoneNumber);

		if (nRetrun)
		{
			// LineOpen Error
			m_bPreDialStart = FALSE;		// LineOpen Fail시 기존 FlowControl을 정상적으로 흐르도록 Flag를 Clear해준다.
			m_pNetLine->LineClose();
		}
	}

	LOG(Info, _T("VISA Flow PreDialStart End"));

	return TRUE;
}

int CVISAIICtrl::PreDialCancelByUser()
{
	LOG(Info, _T("VISA Flow PreDialCancelByUser Call"));

	if (m_bPreDialStart == TRUE)
	{
		ClearQueue();

		m_bPreDialStart = FALSE;

		CloseLine();
	}

	LOG(Info, _T("VISA Flow PreDialCancelByUser End"));

	return FALSE;
}


/** ********************************************************** 
*	@brief VISAII 프로토콜을 구현한 함수입니다. 
*	@param initStatus 플로우 플래그로 사용됨.
*	@param nMode 
*	@retval 0 성공적인 리턴값 
*	@retval 1~8 각 상황별 에러 
*************************************************************/

int CVISAIICtrl::Flow_Matrix(int initStatus, int nMode)
{
	int		status		= initStatus;
	int		snd_rty		= 0;
	int		nak_rty		= 0;
	char	chk_bcc		= 0;
	BYTE	recv_code	= 0;
	int		nresult		= 0;
	BYTE	ctrl_rsp	= 0;

	CString	m_strNVLog;

	LOG(Info, _T("VISA Flow Flow_Matrix Call"));

	// AP Holding 방지를 위해 max timeout을 설정한다.
	DWORD	max_timeout = GetTickCount() + (180*1000);		// Max 3 min timeout

	m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));

	if (Check_Queue(RECV_EVENT, DISCONNECT_EVT))
	{
		LOG(Info, _T("Check Queue - Disconnect Event Detected"));
		return 1;
	}

	while(GetTickCount() < max_timeout)
	{
		recv_code = 0;
		nresult = GetQueue(&recv_code);

		if (nresult == RECV_DATA)
		{
			// 7bit 처리를 software로 한다
			recv_code &= 0x7F;

			switch(status)
			{
			case INIT_STS:
				break;

			case CONN_STS:
					if (recv_code == DATA_ENQ)
					{
						LOG(Info, _T("VISA Flow Flow_Matrix ENQ Received"));
						
						// ENQ RECEIVED
						SetCheckTimer(ACK_TIMEOUT);
						// DATA SEND
						status = ENQIN_STS;
						m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));

						snd_rty++;
						LOG(Info, _T("VISA Flow Flow_Matrix Data Send"));
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
					}
					else
					{
					}
				break;
			case ENQIN_STS:
				if (recv_code == DATA_STX)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix STX Received"));
					SetCheckTimer(DATA_TIMEOUT);
					status = STXIN_STS;
					m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));
				}
				else if (recv_code == DATA_ACK)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix ACK Received"));
					SetCheckTimer(DATA_TIMEOUT);
					status = ACKIN_STS;
					m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));
				}
				else if (recv_code == DATA_NAK)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix NAK Received"));
					if (snd_rty < 4)
					{
						// 3초후 resend
						Sleep(3000);
						SetCheckTimer(ACK_TIMEOUT);
						
						// DATA SEND - 통신 Queue Clear 처리 보완
						ClearQueue();
						snd_rty++;
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
					}
					else
					{
						SetCheckTimer(TIMER_CLEAR);
						return 11;
					}
				}
				else
				{
				}
				break;
			case ACKIN_STS:
				if (recv_code == DATA_STX)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix STX Received"));
					SetCheckTimer(DATA_TIMEOUT);
					status = STXIN_STS;
					m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));
				}
				else
				{
				}
				break;
			case STXIN_STS:
				// calculate bcc
				// buffering
				if (recv_code != DATA_ETX)
				{
					if (m_RecvLen >= NETBUF_RECV_SIZE)
					{
						LOG(Info, _T("VISA Flow Flow_Matrix Recv Buffer Overflow"));
						// receive buffer error
						SetCheckTimer(TIMER_CLEAR);

						ClearQueue();

						// receive buffer clear
						memset(m_RecvData, 0, NETBUF_RECV_SIZE);
						m_RecvLen = 0;
						chk_bcc = 0;
						return 7;
					}

					m_RecvData[m_RecvLen] = recv_code;
					m_RecvLen++;
				}

				chk_bcc ^=recv_code;
				
				if (recv_code == DATA_ETX)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix ETX Received"));
					status = ETXIN_STS;
					m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));
				}
				break;
			case ETXIN_STS:
				// check bcc
				if (chk_bcc == recv_code)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix bcc OK"));
					status = EOTWAIT_STS;
					SetCheckTimer(EOT_TIMEOUT_OPTIONAL);

					ctrl_rsp = DATA_ACK;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);

					// NO USE EOT 적용
					if (m_EagleConfig.LoadHostData(_T("useEOT")) == _T("0"))
					{
						return 0;
					}
				}
				else if (nak_rty < 4)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix bcc NG NAK Send"));

					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					m_RecvLen = 0;
					chk_bcc = 0;

					// bcc error nak send
					nak_rty++;
					status = ACKIN_STS;
					m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));

					SetCheckTimer(DATA_TIMEOUT);
					ctrl_rsp = DATA_NAK;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);
				}
				else
				{
					LOG(Info, _T("VISA Flow Flow_Matrix bcc NG EOT Send"));

					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					m_RecvLen = 0;
					chk_bcc = 0;

					// bcc error & nak retry over
					ctrl_rsp = DATA_EOT;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);

					// error report
					return 10;
				}
				break;
			case EOTWAIT_STS:
				if (recv_code == DATA_EOT || recv_code == DATA_ENQ)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix EOT or ENQ Received (%02x)"), recv_code);
					SetCheckTimer(TIMER_CLEAR);
					ctrl_rsp = DATA_EOT;
					m_pNetLine->LineSendData(&ctrl_rsp, 1);

					// success
					return 0;
				}
				else
				{
				}
				break;
			default:
				break;
			}
		}
		else if (nresult == RECV_EVENT)
		{
			LOG(Info, _T("Event - Kind : %d, Status : %d"), recv_code, status);

			switch(status)
			{
			case INIT_STS:
				if (recv_code == CONNECT_EVT)
				{
					LOG(Info, _T("VISA Flow Flow_Matrix CONNECT OK"));
					if (m_bNetworkTest == TRUE)
					{
						m_bNetworkTest = FALSE;
						
						// success return
						return 0;
					}

					// ENQ is not required option
					if (m_EagleConfig.LoadHostData(_T("useENQ")) == _T("0"))
					{
						LOG(Info, _T("VISA Flow Flow_Matrix Option : not required ENQ"));

						SetCheckTimer(ACK_TIMEOUT);
						// DATA SEND
						status = ENQIN_STS;
						m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));

						snd_rty++;
						LOG(Info, _T("VISA Flow Flow_Matrix Data Send"));
						m_pNetLine->LineSendData(m_SendData, m_SendLen);
					}
					else
					{
						SetCheckTimer(ENQ_TIMEOUT);
						status = CONN_STS;
						m_EagleConfig.SaveLastTransInfoData(_T("Protocol_processing_state"), CUtil::IntToString(status));
					}
				}
				else if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					     recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);

					switch(recv_code)
					{
					case USER_NODIALTONE:
						return 2;
					case USER_NOANSWER:
						return 3;
					case USER_LINEBUSY:
						return 4;
					default:
						return 1;
					}
				}
				break;

			case CONN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					return 5;
				}
				break;

			case ENQIN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);
					return 6;
				}
				break;

			case ACKIN_STS:
			case STXIN_STS:
			case ETXIN_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY || recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);

					memset(m_RecvData, 0, NETBUF_RECV_SIZE);
					m_RecvLen = 0;
					chk_bcc = 0;
					return 7;
				}
				break;

			case EOTWAIT_STS:
				if (recv_code == DISCONNECT_EVT || recv_code == USER_NOANSWER ||
					recv_code == USER_NODIALTONE || recv_code == USER_LINEBUSY)
				{
					SetCheckTimer(TIMER_CLEAR);
					return 8;
				}
				else if (recv_code == TIMEOUT_EVT)
				{
					SetCheckTimer(TIMER_CLEAR);

					if (m_EagleConfig.LoadHostData(_T("useEOT")) == _T("2"))	// Optional
					{
						// 10초 이후에 TIMEOUT인 경우에는 정상처리 한다.
						return 0;
					}
					else
					{
						// EOT OPTIONAL이 아닌 경우에는 비정상 처리 한다.
						return 8;
					}
				}
				break;

			default:
				break;
			}
		}
		else
		{

		}

		if(nresult == 0)	CUtil::Sleep_Wait(100);

	}

	return 1;
}
