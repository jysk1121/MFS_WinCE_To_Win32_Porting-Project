#ifndef __TRAN_BIZ_HOST_HYOSUNG_H__
#define __TRAN_BIZ_HOST_HYOSUNG_H__

public:
	int						m_nUploadLastJnlCount;		// [#2076] NH KSK 2011.06.29
	CPin4Data				m_Pin4;						// [#2396] US Justin 2016.02.17 Add Pin4
	COfferAcceptance		m_OfferAcceptance;			// [#2540] NH Justin 2018.03.13 Add Pin4

#if(APP_POPMONEY) // (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Popmoney to All US Costomers
	CPopMoneyData			m_PopMoneyData;				// [#2442] US Justin 2016.08.25 Popmoney DayTotal
#endif

#if(APP_JUST_CASH)
	CJustCashData			m_JustCashData;				// [#2445] US Justin 2016.09.22 Just Cash
#endif

#if(APP_PAYDIANT_CCA)
	CPaypalCCAData			m_PayPalCCA;				// [#2446] US Justin 2016.09.29 Paypal CCA
#endif

#if(APP_GPAY_GIFTCARD_PURCHASE)
	CGivePayData			m_GivePayData;				// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
#endif

#if(APP_LIBERTYX)
	bool							m_LXIsConfigured;	// [#RWC6-59] US William 2019.10.08 LibertyX
	HTTP*							m_LXHttp;			// [#RWC6-59] US William 2019.11.11 LibertyX
	CHTTPBasicAuthenticator*		m_LXAuth;
	CLibertyXConfigurationManager*	m_LXConfigManager;	// [#RWC6-59] US William 2019.10.08 LibertyX
	LXConfiguration					m_LXConfig;			// [#RWC6-59] US William 2019.10.08 LibertyX
	CLibertyXService*				m_LXService;		// [#RWC6-59] US William 2019.10.08 LibertyX
#endif

#if (APP_DIGITALMINT)
	bool m_DMIsConfigured;
	HTTP *m_DMHttp;

	DMConfiguration						m_DMConfig;
	CDigitalMintConfigurationManager* 	m_DMConfigManager;
	CDigitalMintService*				m_DMService;
#endif

#if (APP_CUSTOM_PAI)
	bool m_PAIIsConfigured;

	PAIConfiguration			m_PAIConfig;
	CPAIConfigurationManager* 	m_PAIConfigManager;
#endif

#if (APP_CUSTOM_CASHDEPOT)
	HTTP *m_CDHttp;
	CCashDepotService *m_CDService;
#endif

// [RWC6-612] Start SKKim 2024.03.28
#if (APP_TANGOPAY)
	TangoPayConfiguration			m_TangoPayConfig;
	CTangoPayConfigurationManager	*m_pTangoPayConfigManager;
	HTTP							*m_pTangoPayHttp;
	TangoPayService 				*m_pTangoPayService;
#endif
// [RWC6-676] End SKKim 2024.03.28

protected:
	BIZ_RETURN	BIZ_STD1_SendHost();

	int			BIZ_STD1_MakeHostMsg();
	int			BIZ_STD1_MakeHeader();
	int			BIZ_STD1_MakeBody();

#if (APP_LIBERTYX)
	int			BIZ_STD1_MakeHeader_LibertyX();		// [#RWC6-59] US William 2019.10.08 LibertyX
#endif

	int			BIZ_STD1_MakeTransMsg();
	int			BIZ_STD1_MakeReversalMsg();
	int			BIZ_STD1_MakeTotalMsg();
	int			BIZ_STD1_MakeConfigMsg();
	int			BIZ_STD1_MakeExtendConfigMsg(int nSubType);
	int			BIZ_STD1_MakeHealthCheckMsg();
	int			BIZ_STD1_MakeDynamicFlowMsg();					// [#2150] US Justin 2012.09.27	Add Dynamic Flow

	int			BIZ_STD1_Make_AssortedField();
	CString		BIZ_STD1_Make_TerminalStatusField();
	CString		BIZ_STD1_Make_EJUploadData(CString strJnlData);	// [#2076] NH KSK 2011.06.29

	BIZ_RETURN	BIZ_STD1_RecvHost();
	BIZ_RETURN	BIZ_STD1_AnalHostData();

	BIZ_RETURN	BIZ_STD1_AnalCommHeader();

	BIZ_RETURN	BIZ_STD1_AnalConfigMsg(int nIndex);
	BIZ_RETURN	BIZ_STD1_AnalExtendConfigMsg(int nIndex);
	BIZ_RETURN	BIZ_STD1_AnalTranMsg(int nIndex);
	BIZ_RETURN	BIZ_STD1_AnalReversalMsg(int nIndex);
	BIZ_RETURN	BIZ_STD1_AnalTotalMsg(int nIndex);
	BIZ_RETURN	BIZ_STD1_AnalHealthCheckMsg(int nIndex);
	BIZ_RETURN	BIZ_STD1_AnalDynamicFlowMsg(int nIndex);		// [#2150] US Justin 2012.09.27	Add Dynamic Flow

	int			BIZ_STD1_AnalAssortedField(int nCurrentIndex);
	int			BIZ_STD1_CheckResponseCode(CString strResponseCode);
	int			BIZ_STD1_SetConfigRequestInitiator(CString strCmd);

	UINT		BIZ_STD1_GetFID(char fid2);

protected:
	STD1_CommonReqMsgHeader		m_sSTD1_CommReqHeader;
	STD1_CommonRespMsgHeader	m_sSTD1_CommRespHeader;

	STD1_ConfigRequest			m_sSTD1_ConfigReq;				// STD1 Configuration Request Msg
	STD1_ConfigResponse			m_sSTD1_ConfigResp;				// STD1 Configuration Download Response Msg

	STD1_ExtendConfigRequest	m_sSTD1_ExtendConfigReq;		// STD1 Extend Configuration Request Msg
	STD1_ExtendConfigResponse	m_sSTD1_ExtendConfigResp;		// STD1 Extend Configuration Response Msg

	STD1_TranRequest			m_sSTD1_TranReq;				// STD1 Transaction Request Msg Main
	STD1_TranResponse			m_sSTD1_TranResp;				// STD1 Transaction Response Msg Main

	STD1_ReversalRequest		m_sSTD1_ReversalReq;			// STD1 Reversal Request Msg
	STD1_ReversalResponse		m_sSTD1_ReversalResp;			// STD1 Reversal Response Msg

	STD1_TotalRequest			m_sSTD1_TotalReq;				// STD1 Total Request Msg
	STD1_TotalResponse			m_sSTD1_TotalResp;				// STD1 Total Response Msg

	STD1_HealthRequest			m_sSTD1_HealthReq;				// STD1 Healthy Check Request Msg
	STD1_HealthResponse			m_sSTD1_HealthResp;				// STD1 Healthy Check Response Msg

	// [#2150] US Justin 2012.09.27 Add Dynamic Flow
	STD1_DynamicFlowRequest		m_sSTD1_DynamicFlowReq;
	STD1_DynamicFlowResponse	m_sSTD1_DynamicFlowResp;
	// End of [#2150]

#endif 