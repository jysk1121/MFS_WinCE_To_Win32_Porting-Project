#ifndef __TRAN_BIZ_HOST_H__
#define __TRAN_BIZ_HOST_H__

public:
	BIZ_RETURN	BIZ_SendHost();
	BIZ_RETURN	BIZ_RecvHost();
	int			BIZ_SendData();
	int			BIZ_RecvData();

	// [#2396] US Justin 2016.02.17 Pin4
	#if (APP_PIN4_CASHPICKUP)			// (APP_CUSTOM_PAI) [#2471] US Justin 2017.02.01 Enable Pin4 for all customers
	BIZ_RETURN	BIZ_Pin4_SendHost();	// [#2515] US Justin.. Remove Parameter
	BIZ_RETURN	BIZ_Pin4_RecvHost();	// [#2515] US Justin.. Remove Parameter
	#endif
	// end of [#2396]
	
	// [#2445] US Justin 2016.09.28 Just.Cash
	#if (APP_JUST_CASH)
	BIZ_RETURN	BIZ_JustCash_SendHost();
	BIZ_RETURN	BIZ_JustCash_RecvHost();
	#endif
	// end of [#2445]

	// [#2446] US Justin 2016.09.29 Paypal CCA
	#if (APP_PAYDIANT_CCA)
	BIZ_RETURN	BIZ_PaypalCCA_SendHost();
	BIZ_RETURN	BIZ_PaypalCCA_RecvHost();
	#endif
	// end of [#2446]

	// [#2513] US Justin 2017.11.02 GivePay Giftcard Purchase
	#if (APP_GPAY_GIFTCARD_PURCHASE)
	BIZ_RETURN BIZ_GivePay_SendHost();
	BIZ_RETURN BIZ_GivePay_RecvHost();
	#endif
	// End of [#2513]

	// [#560] NH KSK 2009.8.20
	STANDARD1_TERMINAL_AIDLIST			m_Standard1_Terminal_AIDList;	
	STANDARD1_HOST_AIDLIST				m_Standard1_Host_AIDList;
	int		nTerminalRemainAIDCnt;
	int		nHostRemainBlock;
	int		nCalcRecvTotalAIDCnt;
	// end of [#560]

	char	m_szDefaultResponseCd[2];	// KSK 2010.08.29 Default ARC값 추가 ("30 35")

protected:
	int		m_nSendLength;				// Send Length
	int		m_nRecvLength;				// Recv Length

	// [#2012] NH KJW 2011.01.19 메모리 영역을 heap으로 옮김. heap 할당은 AP구동시에 하여 AP 동작중에는 계속 사용하기만 하도록 함.
	//BYTE	m_arSendBuffer[NETBUFFSIZE];	// Send Buffer
	//BYTE	m_arRecvBuffer[NETBUFFSIZE];	// Recv Buffer
	BYTE*	m_arSendBuffer;	// Send Buffer
	BYTE*	m_arRecvBuffer;	// Recv Buffer
	// end of [#2012]
	
	CStringArray m_strArrRecvData;		// Received Data after split raw data
	CString	m_strRecvData;				// Received Raw Data converted to String
	CString	m_strSendData;				// Send Data String
	CString	m_strMacSendResult;			// [#2135] NH KSK 2012.05.07

#endif 