#ifndef __TRAN_BIZ_HOST_TRITON_H__
#define __TRAN_BIZ_HOST_TRITON_H__

protected:
	BIZ_RETURN	BIZ_STD3_SendHost();

	int			BIZ_STD3_MakeHostMsg();
	int			BIZ_STD3_MakeHeader();
	int			BIZ_STD3_MakeHeader_LibertyX();		// [#RWC6-59] US William 2019.10.09 LibertyX
	int			BIZ_STD3_MakeBody();

	int			BIZ_STD3_MakeTransMsg();
	int			BIZ_STD3_MakeReversalMsg();
	int			BIZ_STD3_MakeTotalMsg();
	int			BIZ_STD3_MakeConfigMsg();
	int			BIZ_STD3_MakeTDLMsg();				// [#2292] US Justin 2014.09.24	Add TDL Msg


	int			BIZ_STD3_Make_MiscellaneousField();
	int			BIZ_STD3_Make_StatusMonitoringField();

	BIZ_RETURN	BIZ_STD3_RecvHost();
	BIZ_RETURN	BIZ_STD3_AnalHostData();

	BIZ_RETURN	BIZ_STD3_AnalConfigMsg(int nIndex);
	BIZ_RETURN	BIZ_STD3_AnalExtendConfigMsg(int nIndex);
	BIZ_RETURN	BIZ_STD3_AnalTranMsg(int nIndex);
	BIZ_RETURN	BIZ_STD3_AnalReversalMsg(int nIndex);
	BIZ_RETURN	BIZ_STD3_AnalTotalMsg(int nIndex);
	BIZ_RETURN	BIZ_STD3_AnalHealthCheckMsg(int nIndex);
	BIZ_RETURN	BIZ_STD3_AnalTDLMsg(int nIndex);	// [#2292] US Justin 2014.09.30 Add TDL Msg

	int			BIZ_STD3_AnalMiscellaneousField(int nCurrentIndex);
	int			BIZ_STD3_CheckResponseCode(CString strResponseCode);
	UINT		BIZ_STD3_GetFID(char fid2);


	BOOL			m_bFirstKeyReceived;
	BOOL			m_bSecondKeyReceived;

	CString			m_strMacResult_8;	// STANDARD3 MAC RESULT 
	CString			m_strCrcResult_16;	// STANDARD3 CRC RESULT
	CString			m_strEncryptedPINKey2_16;
	CString			m_strEncryptedPINKey3_16;

protected:
	STD3_COMMHEADERS			m_sSTD3_CommHeader;
	STD3_STATUS_MONITOR			m_sSTD3_StatusField;

	STD3_CONFIGREQMSG			m_sSTD3_ConfigReq;				// STD3 Configuration Request Msg
	STD3_CONFIGRESPMSG			m_sSTD3_ConfigResp;				// STD3 Configuration Download Response Msg

	STD3_TRANREQMSG				m_sSTD3_TranReq;
	STD3_TRANRESPMSG			m_sSTD3_TranResp;				// STD3 Transaction Response Msg Main

	STD3_REVERSALREQMSG			m_sSTD3_ReversalReq;
	STD3_REVERSALRESPMSG		m_sSTD3_ReversalResp;			// STD3 Reversal Response Msg

	STD3_HOSTTOTALSREQMSG		m_sSTD3_TotalReq;				// STD3 Total Request Msg
	STD3_HOSTTOTALSRESPMSG		m_sSTD3_TotalResp;				// STD3 Total Response Msg

	CSTD3_TDL_DATA				m_STD3_TDL_Data;				// STD3 TDL Data

#endif 