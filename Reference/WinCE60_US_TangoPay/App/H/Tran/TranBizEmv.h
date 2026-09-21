#ifndef __TRAN_BIZ_EMV_H__
#define __TRAN_BIZ_EMV_H__

// ISO 639.2 참고로 언어 define
#define IC_ENGLISH			_T("en")
#define IC_SPANISH			_T("es")
#define IC_FRENCH			_T("fr")
// [#2434] AU Kook 2016.08.10 add asian languages for EMV transaction.
#define IC_CHINESE			_T("zh")
#define IC_KOREAN			_T("ko")
#define IC_JAPANESE			_T("ja")
// end of [#2434]

#define ISSUER_COUNTRY_CODE		"CAN"		// 2009.3.23
#define	PRIMARY_PROC			(unsigned char) 0x80
#define	SECONDARY_PROC			(unsigned char) 0x40

#define EMV_TR_SUCCESS			(0)
#define EMV_ERROR_ONLINE_DATA	(0x00800000)	
#define EMV_ERROR_DECLINED		(0x00040000)

#define	FALLBACK_INIT				0
#define	FALLBACK_POSENTRY_SEND		1

#define MAX_RECORD			256

typedef struct 
{
	unsigned char	AidName[16];
	int				nAidLen;
	unsigned char	szAid[16];

	int				nRecordLen;
	unsigned char	szRecord[MAX_RECORD];
}TERM_DATA;

public:
	EMVst_LIST_OF_AID		m_sAIDList;
	EMVst_LIST_OF_AID_V5	m_sAIDList_V5;			// [#2188] NH KSK 2013.04.24
	int						m_nCandidateList;
	EMV_ICC_APPS_INFORM		m_sIccAppInform;
	EMV_ICC_APPS_INFORM_V5	m_sIccAppInform_V5;		// [#2188] NH KSK 2013.04.24
	int						m_nSelectAID;
	unsigned char			m_chSW;
	int						m_nSelectedTermDataIndex;
	TERM_DATA				m_TerminalData[ICEMV_APP_CNT];
	BOOL					m_bOnlineProcessing;

public:
	// EMV Proc
	BIZ_RETURN	P_EMV_CheckTransMode();
//	BIZ_RETURN	P_EMV_CheckICCardData();	// [#2082] CA KSK 2011.07.02 함수 제거
	BIZ_RETURN	P_EMV_ICCardRead(int screenNumber = 125);
	BIZ_RETURN	P_EMV_MakeAIDList();
	BIZ_RETURN	P_EMV_SelectMultiAIDList();
	BIZ_RETURN	P_EMV_ICProcessing();
	BIZ_RETURN	P_EMV_FallBackProc();			// Not used on US Version
	BIZ_RETURN	P_EMV_ContinueFallBackProc(int screenNumber = 127);	// [#2208] US Justin 2013.07.08 Continue Fallback for initial Fallback retrial.
	BIZ_RETURN	P_EMV_RemoveCard(CString strDisplayMsg = L"", int MSDataWaitFlag = FALSE, int bForceErrDisp = FALSE, int screenNumber = 127);
	BIZ_RETURN	P_EMV_FinalSelection4MultiTransactionPerLatch();	// [#2391] US Justin 2016.01.26  Support Multi-Transactions per latch

	//EMV Proc For ADA [#2077] CA PCS 2011.07.03
	#if!(US_VESION || CA_VERSION)
		BIZ_RETURN	P_EMV_ADA_SelectMultiAIDList();
		BIZ_RETURN	P_EMV_ADA_FallBackProc();
	#endif

	// Update AID List
	BIZ_RETURN	BIZ_EMV_UpdateAIDListProc();		// [#2081] NH KSK 2011.06.27

	// EMV Function
	void		BIZ_EMV_InitValue();
	void		BIZ_EMV_LoadAIDList();
	int			BIZ_EMV_GetTerminalDataIndex(unsigned char *pAID, int nAID);
	void		BIZ_EMV_StoreDefaultTerminalValue();
	void		BIZ_EMV_StoreTransactionValue();
	BIZ_RETURN	BIZ_EMV_ICCardRead(CString strCardData);
	//void		BIZ_EMV_MakeICDataforTrans();
	void		BIZ_EMV_MakeICDataforTrans(CString sMsgFormat, CString* pstrEMV= NULL);		// [#2237] US Justin 2013.12.05 Dual Host DCC EMV field separator Bug fix
																							// [#2292] US Justin 2014.09.25 Support TDL. Add Parameter to get whole value (default = NULL)
	void		BIZ_EMV_MakeICDataforReversal(int nIssuerScriptLen = 0);
	int			BIZ_EMV_Anal_ICDataforTrans(CString strMicellaneous);
	int			BIZ_EMV_SaveAIDList();				// [#2081] NH KSK 2011.06.27

	//int		BIZ_EMV_CreateAIDList_From_POOL();	// [#2226] US Justin 2013.10.23			// [#2342] US Justin 2015.05.12 Move to CDevCmn
	//void		BIZ_EMV_Get_AID_VersionNumber(CString strAID, char*pVersion);				// [#2342] US Justin 2015.05.05 AID Selection.


#endif 