#ifndef __DEV_CMN_MCU_H__
#define __DEV_CMN_MCU_H__

public:

	BOOL	m_bPartialAIDSupport;		// [#2188] NH KSK 2013.05.21
	BOOL	m_bDisplayDeclined;			// [#2188] NH KSK 2013.05.21

	int		fnMCU_CardEnDisable(int Action, int WaitFlag = FALSE);
	int		fnMCU_ClearErrorCode();					// Clear Error Code

	CString	fstrMCU_GetErrorCode();					// Get Error Code
	int		fnMCU_GetDeviceStatus();
	int		fnMCU_GetMaterialInfo();
	CString	fstrMCU_GetCardData();					// Get Card Data
	CString	fstrMCU_GetICData();					// Get IC Data
	int		fnMCU_Initialize();						// Initialize
	int		fnMCU_Deinitialize();					// Deinitialize
	int		fnMCU_EntryEnable(int nWaitSec = K_WAIT);	
	int		fnMCU_EntryDisable();					// Entry Disable

	int		fnMCU_ICChipInitialize();				// Initialize IC Chip:Move Point
	int		fnMCU_ICEntryEnable(int nWaitSec = K_WAIT);	
	// IC Entry Enable(Wait Time)
	int		fnMCU_ICEntryDisable();					// IC Entry Disable
	int		fnMCU_ICChipPower(int nActType);		// IC Chip Power
	BOOL	fnMCU_IsEmvEnable();					// [#397] [NH] psc 2008.08.01 is EMV option enabled?
	BOOL	fnMCU_IsEmvTransaction();				// [#397] [NH] psc 2008.08.01 is EMV transaction?
	BOOL	fnMCU_IsEmvAvailable();					// [#397] [NH] psc 2008.08.01 is EMV Available?
//	BOOL	fnMCU_IsEmvMSFirst();					// [#397] [NH] psc 2008.08.20 is EMV Ms transaction first?	// [#2082] CA KSK 2011.07.02 함수 제거
	BOOL	fnMCU_IsEmvChipPowerOn();				// EMV(IC) Chip PowerOn flag

	// [#2325] NH KSK 2015.01.20
	int		fnRFID_Initialize();						// Initialize
	int		fnRFID_Deinitialize();						// Deinitialize
	CString	fstrRFID_GetErrorCode();					// Get Error Code
	int		fnRFID_ClearErrorCode();					// Clear Error Code
	int		fnRFID_GetDeviceStatus();
	CString fstrRFID_GetCardData();
	int		fnRFID_EntryEnable(int nWaitSec = K_WAIT);	
	int		fnRFID_EntryDisable();						// Entry Disable
	// end of [#2325]

	/////////////////////////////////////////////////////////////////////////////
	//	EMV FUNCTION(EMV)
	/////////////////////////////////////////////////////////////////////////////

	int				fnEMV_Initialize();

	// EMV Application Selection
	int				fnEMV_Sel_ApplicationSelection(unsigned char SupportPSE, EMVst_LIST_OF_AID *AIDList, int *CandidateCnt);
	int				fnEMV_V5_Sel_ApplicationSelection(unsigned char SupportPSE, EMVst_LIST_OF_AID_V5 *AIDList, int *CandidateCnt, BOOL bPartialSupport = TRUE);							// [#2188] NH KSK 2013.04.26
	int				fnEMV_Sel_FinalAppSelection(unsigned short AidLen, unsigned char *AIDVal, unsigned char *SW);
	int				fnEMV_Sel_MakeDisplayAppList(unsigned char SupportUserConfirm, unsigned short CandidateCnt, EMV_ICC_APPS_INFORM *IccAppInform);
	int				fnEMV_V5_Sel_MakeDisplayAppList(unsigned char SupportUserConfirm, unsigned short CandidateCnt, EMV_ICC_APPS_INFORM_V5 *IccAppInform);	// [#2188] NH KSK 2013.04.26
	void			fvEMV_Sel_RemoveCandidateList(unsigned short CandidateCnt, unsigned short AidLen, unsigned char *AIDVal);

	// 현재 사용 하지 않음
	void			fnEMV_Sel_DescretionaryDataList(EMVst_ICC_DESCDATA_INFORM *EmvIccDescInform);


	// EMV Transaction
	int				fnEMV_Trans_InitApplication(void);
	int				fnEMV_Trans_ReadAppData(void);
	int				fnEMV_Trans_OfflineDataAuth(EMVst_CA_PUBKEY *stCAPKey);
	int				fnEMV_Trans_ProcessRestrict(void);
	int				fnEMV_Trans_CardholderVerify(unsigned short OnlinePinLen, unsigned char *EncipheredOnlinePIN);
	int				fnEMV_Trans_TerminalRiskMgmt(unsigned char SupportForceOnline);
	unsigned char	fcEMV_Trans_TerminalActionAnalysis(void);
	int				fnEMV_Trans_CardActionAnalysis(unsigned char *OnlineFlag);
	int				fnEMV_Trans_OnlineProcess(int OnlineMode, unsigned char Acquirer_CID);
	int				fnEMV_Trans_Completion(void);

	//-------------------------------------------------------------------
	// for Tlv Function
	void			fvEMV_Tlv_GetTidTag(int tid, unsigned char *tag);
	int				fnEMV_Tlv_GetTag(unsigned char *buf, unsigned char *tag);
	int				fstrEMV_Tlv_GetVal(int nID, int *len, unsigned char *pData);
	int				fnEMV_Tlv_GetTidLen(int tid);
	int				fnEMV_Tlv_StoreVal(int tid, int len, unsigned char *val);
	int				fnEMV_Tlv_StoreValFromRecord(unsigned char *record, int type, int inlen, int source);

	//-------------------------------------------------------------------
	// EMV Utility
	void			fvEMV_Util_BINtoN(int val, unsigned char *data, int len);
	void			fvEMV_Util_toYYYYMMDD(unsigned char *data, unsigned char *out, int format);
	unsigned char	fcEMV_Util_GetCNRealLen(unsigned char *data, int len);
	void			fvEMV_Util_RightJustify(unsigned char *data, int len, unsigned char *dest);
	void			fvEMV_Util_Decompress(unsigned char *bySource, unsigned char *byDestination, int byCount);
	void			fvEMV_Util_Compress(unsigned char *bySource, unsigned char *byDestination, int byCount);

	//-------------------------------------------------------------------

	// [#2188] NH KSK 2013.04.24
	int				fnEMV_SetKernelVersion(int nVersion);
	int				fnEMV_GetKernelVersion();
	void			fnEMV_Trans_Initial();
	// end of [#2188]

#endif __DEV_CMN_MCU_H__