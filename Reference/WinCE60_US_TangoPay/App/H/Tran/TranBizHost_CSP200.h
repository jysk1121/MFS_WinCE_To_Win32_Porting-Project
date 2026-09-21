#ifndef __TRAN_BIZ_HOST_CSP200_H__
#define __TRAN_BIZ_HOST_CSP200_H__

protected:
	BIZ_RETURN	BIZ_STD2_SendHost();

	int			BIZ_STD2_MakeHostMsg();
	int			BIZ_STD2_MakeHeader();
	int			BIZ_STD2_MakeBody();

	int			BIZ_STD2_MakeTransMsg();
	int			BIZ_STD2_MakeReversalMsg();
	int			BIZ_STD2_MakeTotalMsg();
	int			BIZ_STD2_MakeConfigMsg();
	int			BIZ_STD2_MakeExtendConfigMsg();
	int			BIZ_STD2_MakeHealthCheckMsg();

	int			BIZ_STD2_MakeDetailHealthCheckMsg();

	BIZ_RETURN	BIZ_STD2_RecvHost();

	BIZ_RETURN	BIZ_STD2_AnalHostData();
	BIZ_RETURN	BIZ_STD2_AnalCommHeader();

	BIZ_RETURN	BIZ_STD2_AnalConfigMsg(int nIndex);
	BIZ_RETURN	BIZ_STD2_AnalTranMsg(int nIndex);
	BIZ_RETURN	BIZ_STD2_AnalReversalMsg(int nIndex);
	BIZ_RETURN	BIZ_STD2_AnalTotalMsg(int nIndex);
	BIZ_RETURN	BIZ_STD2_AnalHealthCheckMsg(int nIndex);
	BIZ_RETURN	BIZ_STD2_AnalDetailHealthCheckMsg(int nIndex);

	int			BIZ_STD2_CheckResponseCode(CString strResponseCode);
	int			BIZ_STD2_SetConfigRequestInitiator(CString strCmd);


protected:
	STD2_CommonReqMsgHeader		m_sSTD2_CommReqHeader;
	STD2_CommonRespMsgHeader	m_sSTD2_CommRespHeader;

	STD2_ConfigRequest			m_sSTD2_ConfigReq;				// STD2 Configuration Request Msg
	STD2_ConfigResponse			m_sSTD2_ConfigResp;				// STD2 Configuration Download Response Msg

	STD2_TranRequest			m_sSTD2_TranReq;				// STD2 Transaction Request Msg Main
	STD2_TranResponse			m_sSTD2_TranResp;				// STD2 Transaction Response Msg Main

	STD2_ReversalRequest		m_sSTD2_ReversalReq;			// STD2 Reversal Request Msg
	STD2_ReversalResponse		m_sSTD2_ReversalResp;			// STD2 Reversal Response Msg

	STD2_TotalRequest			m_sSTD2_TotalReq;				// STD2 Total Request Msg
	STD2_TotalResponse			m_sSTD2_TotalResp;				// STD2 Total Response Msg

	STD2_HealthRequest			m_sSTD2_HealthReq;				// STD2 Healthy Check Request Msg
	STD2_HealthResponse			m_sSTD2_HealthResp;				// STD2 Healthy Check Response Msg

	STD2_DetailHealthRequest	m_sSTD2_DetailHealthReq;
	STD2_DetailHealthResponse	m_sSTD2_DetailHealthResp;

#endif 