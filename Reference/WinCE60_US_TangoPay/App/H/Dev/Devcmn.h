#ifndef __DEV_CMN_H__
#define __DEV_CMN_H__

#ifndef _DEVCTRL_OWNER_
#pragma comment(lib, "DevCtrl.lib")
#endif

#include ".\DevDefine.h"
#include ".\JnlMgr.h"
#include ".\ErrorCodeInfo.h"
#include ".\MultiString.h"

#include "..\Common\CommDef.h"
#include "..\Common\CmnLib.h"

#include "..\Net\Network.h"
#include "..\Scr\ScrCtrl.h"

#include "..\Dll\NhMwi.h"
#include "..\Dll\Profiledef.h"

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
#include "..\Dev\BinMgr.h"			// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함 
#endif

//#include ".\Dev\EmvL2KernelDef.h"
#include ".\Dev\SCSp_ATM_EMV.h"		// [#2188] NH KSK 2013.04.24 Kernel 구버전/신버전 동시 사용을 위해 header file 통일 및 Dll 동적 Loading하도록 로직 변경

// [#RWC6-59] US William 2019.10.08 LibertyX
// Determines which host configuration to use
typedef enum _host_config
{
	HC_ATM					= 0,
	HC_DUALHOST				= 1,
	HC_LIBERTYX				= 2,
	HC_LTX_HOST_DEP_MODE			= 3,

	HC_MAX
} HOST_CONFIG;

/////////////////////////////////////////////////////////////////////////////
// CDevCmn window
/////////////////////////////////////////////////////////////////////////////
class AFX_EXT_CLASS CDevCmn : public CWnd
{
#include ".\DevCmn_Mcu.h"
#include ".\DevCmn_Cam.h"
// [#GLDV-3005] US Kook 2021.11.08 Support Side Car
#include ".\DevCmn_Bcr.h"
#include ".\DevCmn_Bna.h"
// end of [#GLDV-3005]

/////////////////////////////////////////////////////////////////////////////
// Attributes
/////////////////////////////////////////////////////////////////////////////
public:
	CNetWork*		m_pNetWork;			// NetWork Pointer
	CNHMWI*			m_pMwi;				// Mwi Pointer

	//char			m_szTemp[2048];		// WideToMulti 사용 2006.04.19 // [#2012] NH KJW 2011.02.09 미사용 삭제
//	char*			m_pRecvData;		// [#2012] NH KJW 2011.02.09 fnNET 함수에서 사용하는 버퍼 동적할당용 [###2] 
	CString			m_strNVLog;

	// Healthy Check Timer Variables PJH 2005.12.23
	COleDateTime		m_oletimeNow;
	COleDateTimeSpan	m_oletimeGap;
	COleDateTime		m_oletimeStart;
	CTime				m_AntiSkiimingRecoveryTime;		// [#2085] NH KSK 2011.07.22
	UINT				m_DelayTime;					// Health Check Message Time Interval

	BOOL	bAntiSkimmingFlag;			// [#2085] NH KSK 2011.07.22
	int		AtmDoing;					// Atm Doing
	int		AtmStatus;					// Atm Status
	int		AtmStatusSave;				// Atm Status Save
	int		TranStatus;					// Transaction Status
	int		TranResult;					// Transaction Result

	int		DeviceDefine;				// Device Define Set
	int		DeviceNoDevice;				// Device No Device	Set
	int		DeviceDownDevice;			// Device Down Device Set
	int		DeviceStatus;				// Device Status Set
	int		DeviceAutoOff;				// Device Auto Off Set(축퇴)
//	int		DeviceTran;					// Device Transaction Set	[#2325] NH KSK 2015.01.22 불필요 변수 삭제
	int		DeviceEvent;				// Device Event Set

	int		DeviceOperationFlag;		// Device Operation Flag

    int		OpenKey;					// Open Key

	int		CSTCnt;						// CST Count
	int		FirstCSTValue;				// First CST Value
	int		SecondCSTValue;				// Second CST Value
	int		ThirdCSTValue;				// Third CST Value
	int		FourthCSTValue;				// Fourth CST Value
	int		FirstCSTInitCash;				// First CST Initial Cash
	int		SecondCSTInitCash;				// Second CST Initial Cash
	int		ThirdCSTInitCash;				// Third CST Initial Cash
	int		FourthCSTInitCash;				// Fourth CST Initial Cash
	int		FirstCSTSetCash;				// First CST Set Cash
	int		SecondCSTSetCash;				// Second CST Set Cash
	int		ThirdCSTSetCash;				// Third CST Set Cash
	int		FourthCSTSetCash;				// Fourth CST Set Cash

	// [#3] AIREAT NH 2008.03.19
	int		nAdditionCST1;				// Cassette 1 Addition Cash
	int		nAdditionCST2;				// Cassette 2 Addition Cash
	int		nAdditionCST3;				// Cassette 3 Addition Cash
	int		nAdditionCST4;				// Cassette 4 Addition Cash
	// end of [#3]

	CString	m_strCurrencyID;			// [#146] KSK 2008.04.20

	int		RejectCSTCnt;				// Reject CST Count(ClerkInformation)
	int		FirstCSTCnt;				// First CST Count(ClerkInformation)
	int		SecondCSTCnt;				// Second CST Count(ClerkInformation)
	int		ThirdCSTCnt;				// Third CST Count(ClerkInformation)
	int		FourthCSTCnt;				// Fourth CST Count(ClerkInformation)

	int		CduChangeLoc;				// Cash Change Location
	int		CduLoc;						// Cash Location
	int		RejectCSTLoc;				// Reject CST Location
	int		FirstCSTLoc;				// First CST Location(Cd)
	int		SecondCSTLoc;				// Second CST Location(Cd)
	int		ThirdCSTLoc;				// Third CST Location(Cd)
	int		FourthCSTLoc;				// Fourth CST Location(Cd)
	int		CSTLoc;						// All CST Location(Cd)			// [#51] NH PSC 2008.04.03

	int		SlipStatus;					// Slip Status

	int		RejectCSTStatus;			// Reject CST Status
	int		RecycleBoxStatus;			// Recycle Box Status
	int		FirstCSTStatus;				// First CST Status
	int		SecondCSTStatus;			// Second CST Status
	int		ThirdCSTStatus;				// Third CST Status
	int		FourthCSTStatus;			// Fourth CST Status
	int		WithAvail;					// Withdraw Avail

	int		m_nCstAvail;				// CST Set Data for MDB

	// [#GLDV-3005] US Kook 2022.04.06 support side car
	int		m_nBnaNumberOfItem;
	int		m_nBnaItemCount[16];
	int		m_nBnaItemValue[16];
	// end of [#GLDV-3005]

	// [#RWC6-448] US Wallace 2022.09.20 side car
	int		m_nBnaCSTStatus;

	CString	m_strHotKey;				// Hot-Key string to check
	int		m_nHotKeyStatus;
	
	CString m_strStoredHotKey;			// [#2283] Justin Support Cutomized Hot Key
	bool	m_bExecuteResetMP;			// [#RWC6-94] NH Kook 2020.01.31 Easing MPW Clear

	int		HostOpenFlag;				// Host Open Flag
	int		HostConfigCmd;				// Host Command Set
	int		HostOpenRetryTime;			// Host Open Retry Time

	int		nKindOfMedia;				// [#397] [NH] psc 2008.08.01 카드종류(IC, MS)
	int		nIsEmvChipPowerOn;			// for EMV(IC): Chip PowerOn flag - 1(yes), 0(no)

	int		m_nCurrentLangMode;

	CStringArray m_arrBinRestrictionList;	// [#522] US KSK 2009.05.19 NZ에서만 사용
	CStringArray m_arrExtendedBinList;		// [#522] US KSK 2009.05.19 Extended Bin List 지원
	CStringArray m_arrAUDomesticBinList;	// [#2379] AU KSK 2015.11.17 AU Domestic Bin List 지원

//	int		WorkingLoadFlag;			// [#2010] SOOK 2011.01.14 Working Load 결과 값 
	int		nPINErrorFlag;				// [#2135] NH KSK 2012.05.07
	
	int		m_nSurchTable[2][10];		// [#2136] US/CA PCS 2012.05.30
	CString	m_strTableSurcharge;		// [#2136] US/CA PCS 2012.05.30 "출금 확정 후 정해지는 수수료."
	int		m_nRemoteStatusCmd;			// [#2277] US Justin 2014.06.13 Remote Status
	int		m_nPrevLangMode;			// [#2339] NH Justin 2015.04.07 Support Asian Language. Fix font corruption on Out of Service Screen
	int		m_nDCCLocalAvail;			// [#2472] NH Justin 2017.02.07 DCC Withdrawal Option

	bool	bIsModemAvailable;			// [#GLDV-2683] NH Kook 2019.11.06 Detects Modem Removal

/////////////////////////////////////////////////////////////////////////////
// Operations
/////////////////////////////////////////////////////////////////////////////
private:
	CScrCtrl*		m_arScrCtrl[SCR_LOCATION_MAX];
	CScrCtrl*		m_pCurScrCtrl;
	CMultiString	m_MultiString;
	CNHEvent		m_eWaitObject;

public:
/////////////////////////////////////////////////////////////////////////////
//	Construction/Destruction
/////////////////////////////////////////////////////////////////////////////
	CDevCmn();
	virtual ~CDevCmn();
	
	CJnlMgr			m_JNLMgr;			// [#3] NH AIREAT 2008.3.10
	CString			strJnlKindCode;		// [#591] NH KSK 2009.12.07
	CErrorCodeInfo	m_ErrorCodeInfo;
	CNHConfig		*m_pConfig;

	// [#2342] US Justin 2015.05.06 AID Selection
#if (US_VERSION)
	CStringArray	m_arrUSCommonAID;
#endif
	// End of [#2342]
	BOOL			m_bUnKnownAID_SkipPE;		// [#2436] US Justin 2016.07.22 
	BOOL			m_bShownMultiAIDSelection;	// [#2440] US Justin 2016.08.10 

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	CBinMgr			m_BINMgr;		// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함 
#endif

//[#610] SOOK 2009.12.21 Parameter Change Journal 저장 
#if SUPPORT_CHANGE_PARAMETER_JNL   

	BOOL			bAllocSucceed;

	LPNVRAM2_ADDR	pCurrentNVRam2;
	LPNVRAM3_ADDR	pCurrentNVRam3;
	LPNVRAM6_ADDR	pCurrentNVRam6;
	LPNVRAM8_ADDR	pCurrentNVRam8;

	APPATMINFO			pPreviousATMInfo;
	ADVINFO				pPreviousAdvInfo;
	TCPIPINFO			pPreviousTCPIPInfo;
	OPTINALSETTING		pPreviousOptionalSetting;	
	OPTINALSETTING2		pPreviousOptionalSetting2;
	NETWORKINFO			pPreviousNetworkInfo;
	WORD				wPreviousSpeakerVolume;

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	WORD				wPreviousTotalBinCount;
	WORD				wPreviousAllowOnlyBinEnable;		
	BOOL				m_bDomesticFallbackTrans;	// [#2379] AU KSK 2015.11.17
#endif

#endif
//end of [#610]

/////////////////////////////////////////////////////////////////////////////
//	FUNCTION
/////////////////////////////////////////////////////////////////////////////
//	RETURN RULE1:NORMAL=TRUE, ABNORMAL=FALSE
//			   2:RETURN INT ONLY(TRUE OR FALSE OR VALUE)
//			   3:TRUE!=0, FALSE=0
//			   4:CSTRING APPEND									2003.08.23
/////////////////////////////////////////////////////////////////////////////
	void	SetOwner(CNHMWI* pMwi);					// Set Owner
	void	Set_NetworkInformation();				// [#105] KSK 2008.04.14
	CString	Convert_IpAddressInfo(CString strParm_Ip);				// [#465] [NH] KSK 2008.12.22

	void	LoadBinFromFile(CString strFileName, CStringArray &strLoadedBinList);
	BOOL	IsBinExist(CString strTrackData, CStringArray &strLoadedBinList);

	int	LoadAUDomesticBinFromFile(CString strFileName, CStringArray &strLoadedBinList);	// [#2379] AU KSK 2015.11.17
	// [#2283] NH Justin 2014.07.01
	BOOL	IsValidHotKey(CString strKeyString);
	void	ReArrangeEnteredHotKey();
	// End of [#2283]

	// [#2342] US Justin 2015.05.12 AID Selection
	BOOL	Create_EMV_Termdata_From_POOL();		
	void	Get_AID_AP_VersionNumber(CString strAID, char* pVersion, int lenVersion);
	// End of [#2342]

//[#610] SOOK 2009.12.21 Parameter Change Journal 저장 
//#if SUPPORT_CHANGE_PARAMETER_JNL   
	void	SetPreviousParameter();
	void	SaveChangeParameter(int nUser);
	void	SaveNetworkInfoParameter(int nUser, LPNETWORKINFO pPreviousNetworkInfo, LPNETWORKINFO pCurrentNetworkInfo);
	void	SaveATMInfoParameter(int nUser, LPAPPATMINFO pPreviousATMInfo, LPAPPATMINFO pCurrentATMInfo);
	void	SaveAdvertiseInfoParameter(int nUser, LPADVINFO pPreviousAdvInfo, LPADVINFO pCurrentAdvInfo);
	void	SaveTCPIPInfoParameter(int nUser, LPTCPIPINFO pPreviousTCPIPInfo, LPTCPIPINFO pCurrentTCPIPInfo);
	void	SaveOptionalSettingParameter(int nUser, LPOPTIONALSETTING pPreviousOptionalSetting , LPOPTIONALSETTING pCurrentOptionalSetting );
	void	SaveOptionalSetting2Parameter(int nUser, LPOPTIONALSETTING2 pPreviousOptionalSetting2, LPOPTIONALSETTING2 pCurrentOptionalSetting2);
//#endif
//end of [#610]--

	// TAR Assets
	CAssetArchive*	m_pTarScreenAsset;
	void			SetTarAsset(void* asset);

/////////////////////////////////////////////////////////////////////////////
//	SLIP PRINTER FUNCTION(SPR)
/////////////////////////////////////////////////////////////////////////////
// SPR SET MODE
	int		fnSPR_ClearErrorCode();					// Clear Error Code
	int		fnSPR_ClearRetractCnt();				// Clear Retract Count

// SPR GET MODE
	CString	fstrSPR_GetErrorCode();					// Get Error Code
	int		fnSPR_GetDeviceStatus();				// Get Device Status

	int		fnSPR_GetPaperStatus();					// Get Paper Status
	int		fnSPR_GetMaterialInfo();				// Get Material Information

// SPR OPERATION MODE
	int		fnSPR_Initialize();						// Initialize
	int		fnSPR_Deinitialize();					// Deinitialize

	//BOOL	fnSPR_PrintReceipt(BOOL bPrintImage, CString strData, int nWaitSec);
	//BOOL	fnSPR_PrintReceipt(BOOL bPrintImage, CString strData, int nWaitSec, BOOL bEject=TRUE);								// [#2185] US Justin 2013.05.14 Add Eject(Cut) Option 
	BOOL	fnSPR_PrintReceipt(BOOL bPrintImage, CString strData, int nWaitSec, BOOL bEject=TRUE, BOOL bConvertBCD = FALSE );	// [#2373] US Justin Add Convert Print Barcode Option
	BOOL	fnSPR_PrintHeader();
	BOOL	fnSPR_PrintData(CString strData);
	BOOL	fnSPR_PrintCouponImage(int nType);		// [#2373] US Justin 2015.09.22 Print Coupon Image
	BOOL	fnSPR_EjectPaper(int nWaitSec = K_45_WAIT);
	BOOL	fnSPR_WaitTaken();
	BOOL	fnSPR_Retract();

/////////////////////////////////////////////////////////////////////////////
//	CDU FUNCTION(CDU)
/////////////////////////////////////////////////////////////////////////////

	int		m_MoneyMinOut;				// Money Min Out
	int		m_MoneyMaxOut;				// Money Max Out

	int		m_nFastCash[CDU_CST_MAXFASTCASH];
	int		m_FastCaseNo;				// Fast Cash Selected Case (Done on ATM_CUSTOM mode)
	int		m_nTranDispLimit;			// [#2350] US Justin 2015.06.18 Add Transaction Dispense Limit


// CDU SET MODE
	int		fnCDU_ClearErrorCode();					// Clear Error Code
	int		fnCDU_ClearDispenseInfo();				// Clear Dispense Information

	int		fnCDU_SetValueOfCash(LPCTSTR szCashValueData);		
													// Set Value Of Cash(Cash Value Data)
	int		fnCDU_SetMinMaxCST(LPCTSTR szMinCntData, LPCTSTR szMaxCntData);
													// Set MinMax Of CST(Min Count Data, Max Count Data)
	int		fnCDU_SetNumberOfCash(LPCTSTR szCashCntData);	// [#380] [NH] KSK 2008.7.22
	int		fnCDU_AddNumberOfCash(LPCTSTR szInitialCnt, LPCTSTR szAddCnt);	// [#380] [NH] KSK 2008.7.22

// CDU GET MODE
	CString	fstrCDU_GetErrorCode();					// Get Error Code
	CString	fstrCDU_GetSensorInfo();				// Get Sensor Information
	int		fnCDU_GetDeviceStatus();				// Get Device Status

	int		fnCDU_GetPosition();					// Get Position
	int		fnCDU_GetRejectCSTStatus();				// Get Reject CST Status
	int		fnCDU_GetRecycleBoxStatus();			// Get Recycle Box Status
	int		fnCDU_GetMaterialInfo();				// Get Material Information
	int		fnCDU_GetAvailWithdraw();				// Get Avail Withdraw

	int		fnCDU_GetNumberOfCST();					// Get Number Of CST
	int		fnCDU_GetCSTStatus(int nCSTNo);			// Get CST Status(CST No)
	int		fnCDU_GetValueOfCash(int nCSTNo);		// Get Value Of Cash(CST No)
	CString	fnCDU_GetCurrencyID(int nCSTNo);		// Get CurrencyID(CST No) 2007.11.23 V01.02.19
	int		fnCDU_SetCurrencyID(LPCTSTR szCurrencyID);// Set CurrencyID 2009.03.10 [#514] [MX] KSK 2009.3.10
	int		fnCDU_GetNumberOfCash(int nCSTNo);		// Get Number Of Cash(CST No)
	int		fnCDU_GetNumberOfSetCash(int nCSTNo);	// Get Number Of Set Cash(CST No)
	CString	fstrCDU_GetDispenseOfCST(int nCashAmt);	// Get Dispense Of CST(Cash Amount)
	int		fnCDU_GetNumberOfReject(int nCSTNo);	// Get Number Of Reject(CST No)
													// Get Count of Last Dispensed Cassette(CST No) from Cst or To User.
	int		fnCDU_GetLastDispensedCount(int nCstNo);
	int		fnCDU_GetLastDispensedAmount();			// Get Last Dispensed Amount

/////////////////////////////////////////////////////////////////////////////
//	CASSETTE FUNCTION ROUTING
/////////////////////////////////////////////////////////////////////////////
	int		fnCDU_GetAllCSTStatus();				// [#194] KSK 2008.05.16

// CDU OPERATION MODE
	int		fnCDU_Initialize(long InitialType);		// Initialize	// [#507] [NH] KSK 2009.2.23 함수 Parameter 추가
	int		fnCDU_Deinitialize();					// De-initialize

	int		fnCDU_WaitTaken();						// Wait Taken
	int		fnCDU_Dispense(int nCashAmt);			// Dispense(Cash Amount)

	int		fnCDU_DispenseAndPresentCount(int nCST1Cnt, int nCST2Cnt, int nCST3Cnt, int nCST4Cnt, int nWaitSec);	// for presenter type	// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
	int		fnCDU_DispenseCount(int nCST1Cnt, int nCST2Cnt, int nCST3Cnt, int nCST4Cnt);							// for spray type		// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA


/////////////////////////////////////////////////////////////////////////////
//	Calculate Money Function
/////////////////////////////////////////////////////////////////////////////
	int		fnCDU_CalcMoneyMaxOut();							// Calculate Money Max Out
	int		fnCDU_CalcMoneyMinOut();							// Calculate Money Min Out
	int		fnCDU_GetCashDispenseInfo(int nDispLimit = -1 /*Dollar*/);	// Calculate Min/Max Out Cash Amount		// [#2350] US Justin 2015.06.18 Add Parameter (Overwrite Dispense Limit)
	int		fnCDU_CalculateFastCash();							// Fast Cash Button Calculation
	int		fnCDU_GetMaxAmountOfAllCsts();						// Get Maximum Amount of All Cassette Current Count
	BOOL	fbCDU_IsDispensible(int nCashAmount);				// 출금금액이 방출가능한지 먼저 물어봄.

	int		fnCDU_SumProc(int InputID);							// Sum Procedure
	int		fnCDU_SumClear(int n3rdTranType = 0);				// Sum Clear
	
	int		fnCDU_TestDispense();								// [#2504] NH Justin 2017.09.13	

/////////////////////////////////////////////////////////////////////////////
//	DOOR FUNCTION(DOR)
/////////////////////////////////////////////////////////////////////////////
// DOOR SET MODE
	int		fnDOR_ClearErrorCode();					// Clear Error Code

// DOOR GET MODE
	CString	fstrDOR_GetErrorCode();					// Get Error Code
	int		fnDOR_GetDeviceStatus();				// Get Device Status

	int		fnDOR_GetDoorStatus();					// Get Door Status

// DOOR OPERATION MODE
	int		fnDOR_Initialize();						// Initialize
	int		fnDOR_Deinitialize();					// Deinitialize

/////////////////////////////////////////////////////////////////////////////
//	LIGHT FUNCTION(LGT)
/////////////////////////////////////////////////////////////////////////////
// LIGHT SET MODE
	int		fnLGT_ClearErrorCode();					// Clear Error Code

	int		fnLGT_SetFlicker(int nIndex, int nValue);					
													// Set Flicker(Index, Value)
// LIGHT GET MODE
	CString	fstrLGT_GetErrorCode();					// Get Error Code
	int		fnLGT_GetDeviceStatus();				// Get Device Status

// LIGHT OPERATION MODE
	int		fnLGT_Initialize();						// Initialize
	int		fnLGT_Deinitialize();					// Deinitialize


/////////////////////////////////////////////////////////////////////////////
//	PIN FUNCTION(PIN)
/////////////////////////////////////////////////////////////////////////////
// PIN SET MODE
	int		fnPIN_ClearErrorCode();					// Clear Error Code
	int		fnPIN_ClearAllKeys();					// Clear All Keys // [#2057] NH KSK 2011.05.04 InitializeEDM() 함수 추가

// PIN GET MODE
	CString	fstrPIN_GetErrorCode();					// Get Error Code
	int		fnPIN_GetDeviceStatus();				// Get Device Status

	CString	fstrPIN_GetPinKeyData();				// Get Pin Key Data
	CString	fstrPIN_GetMacingData();				// Get Macing Data

// PIN OPERATION MODE
	int		fnPIN_Initialize();						// Initialize
	int		fnPIN_Deinitialize();					// Deinitialize

	int		fnPIN_EntryEnable(int nEnableMode, int nMinKeyCount, int nMaxKeyCount, int bAutoEnd, LPCTSTR szActiveKeys, LPCTSTR szTerminatorKeys, LPCTSTR szCardData, int nTimeout = K_WAIT);
													// Entry Enable(EnableMode, MinKeyCount, MaxKeyCount, AutoEnd, ActiveKeys, TerminatorKeys, CardData, Timeout)
	int		fnPIN_EntryDisable();					// Entry Disable

	int		fnPIN_LoadIV(LPCTSTR szKeyName);		// Load IV(Key Name)
	int		fnPIN_LoadKey(LPCTSTR szKeyName, LPCTSTR szKeyValue);		
													// Load Key(Key Name, Key Value)
	int		fnPIN_LoadEncryptedKey(LPCTSTR KeyUse, LPCTSTR szKeyName, LPCTSTR szKeyValue, LPCTSTR szEncKeyName);	// [#554] KSK 2009.08.10
	int		fnPIN_MacingData(LPCTSTR szMacData);	// Macing Data(Mac Data)

// [#554] KSK 2009.08.10 RKT Function 추가
/////////////////////////////////////////////////////////////////////////////
//	RKT FUNCTION (PIN)
/////////////////////////////////////////////////////////////////////////////
	int		fnPIN_RKT_ExportRSAIssuerSignedItem(LPCTSTR ExportItemType, LPCTSTR KeyName);
// [#GLDV-2797] AU Kook 2020.05.07 Support Host PK Removal
//	int		fnPIN_RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature);
	int		fnPIN_RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature, LPCTSTR Usage=L"RSAPUBLICVERIFY");
// end of [#GLDV-2797]
	int		fnPIN_RKT_StartKeyExchange();
	int		fnPIN_RKT_ImportRSASignedDESKey(int KeyType, LPCTSTR Recv_EPP_RSACrypt, LPCTSTR Recv_TMK_RSACrypt, LPCTSTR Recv_HSM_Signature);
	int		fnPIN_RKT_GetExtraCaps();	// [#2080] NH KSK 2011.07.13

	CString	fnPIN_RKT_GetExportedKey();				// EPP Serial Key or Public Key
	CString	fnPIN_RKT_GetExportedSignedValue();		// Signed Value
	CString	fnPIN_RKT_GetEPP_RandomNumber();		// Get EPP Random Number (Repp)		// [#2259] NH KSK 2014.03.13
	CString	fnPIN_RKT_GetEPP_KCV();					// Get Epp KCV
// end of [#554]

	CString	fnPIN_Get_KCV_512K(CString KeyName);		// [#2253] NH KSK 2014.02.21
	BOOL	fnPIN_RKT_DeleteHostPK();					// [#GLDV-2797] AU Kook 2020.05.07 Support Host PK Removal

	// [#RWC6-151] US Kook 2020.07.20 TR34
	int		fnPIN_TR34_GetCertificate(CString Certificate);
	int		fnPIN_TR34_InitializeEDMEx(CString Indent, CString Key);
	int		fnPIN_TR34_ImportKeyBlock(CString Key, CString EncKey, CString KeyBlock);
	int		fnPIN_TR34_LoadCertificateEx(CString LoadOption, CString Signer, CString CertificateData);
	int		fnPIN_TR34_ImportRSAEncipheredPKCS7KeyEx(CString RSAKeyIn, CString Key, CString Use, CString LoadOption);
	int		fnPIN_TR34_StartAuthenticateSync(CString MethodName, CString Parameters = L"");

	// properties
	CString fstrPIN_TR34_GetCertificateData();
	CString fstrPIN_TR34_GetCertificateState();

	// properties for unbind ' StartAuthenticateSync("InitializeEDMEx") '
	CString fstrPIN_TR34_GetAuthenticateDataToSign();
	CString fstrPIN_TR34_GetAuthenticateSigner();
	void	fnPIN_TR34_SetAuthenticateSigner(CString Signer);
	void	fnPIN_TR34_SetAuthenticateSigKey(CString SigKey);
	void	fnPIN_TR34_SetAuthenticateSignedData(CString SignedData);


/////////////////////////////////////////////////////////////////////////////
//	SENSOR FUNCTION(SNS)
/////////////////////////////////////////////////////////////////////////////
// SENSOR SET MODE
	int		fnSNS_ClearErrorCode();					// Clear Error Code
	int		fnSNS_ClearOperatorSwitch();			// Clear Operator Switch

// SENSOR GET MODE
	CString	fstrSNS_GetErrorCode();					// Get Error Code
	int		fnSNS_GetDeviceStatus();				// Get Device Status

	int		fnSNS_GetOperatorSwitch();				// Get Operator Switch
	int		fnSNS_GetEnhancedAudio();				// Get Enhanced Audio Status
	int		fnSNS_GetProximity();					// Get ProximityStatus

// SENSOR OPERATION MODE
	int		fnSNS_Initialize();						// Initialize
	int		fnSNS_Deinitialize();					// Deinitialize


/////////////////////////////////////////////////////////////////////////////
//	COMMON FUNCTION(CMN)
/////////////////////////////////////////////////////////////////////////////
// CMN SET MODE
	int		fnCMN_ClearEventDevice(int nDevId);		// Clear Event Device(Device Id)
	int		fnCMN_ClearStatusDevice(int nDevId);	// Clear Status Device(Device Id)

// CMN GET MODE
	int		fnCMN_GetErrorDevice(int nDevId);		// Get Error Device(Device Id)
	int		fnCMN_GetTimeoutDevice(int nDevId);		// Get Timeout Device(Device Id)
	int		fnCMN_GetEventDevice(int nDevId);		// Get Event Device(Device Id)
	int		fnCMN_GetStatusDevice(int nDevId);		// Get Status Device(Device Id)

	int		fnCMN_GetActiveDevice();
	void	fnCMN_AddActiveDevice(int nDevId);
	void	fnCMN_RemoveActiveDevice(int nDevId);

// CMN OPERATION MODE
	CString	fstrCMN_CheckDeviceAction(int nDevId, int nCheckTime);	
													// Check Device Action(Device Id, Check Time)
	int		fnCMN_ScanDeviceAction(int nDevId, int nScanTime, int nEventKind);	
													// Scan Device Action(Device Id, Check Time, Event Kind)
/////////////////////////////////////////////////////////////////////////////
//	NETWORK FUNCTION(NET)
/////////////////////////////////////////////////////////////////////////////
// NET SET MODE
	int		fnNET_ClearErrorCode();					// Clear Error Code

// NET GET MODE
	CString	fstrNET_GetErrorCode();					// Get Error Code
	CString	fstrNET_GetErrorMsg();					// Get Error Message

	// [#525] US KSK 2009.05.25
	int	 	fnNET_PreDialStart();
	int 	fnNET_PreDialCancelByUser();
	// end of [#525]


	//int 	fnNET_SendData(BYTE* szSendData, int nLen, int nSendSec = K_180_WAIT);		// Send Data(Send Data, Wait Time)
	//int	fnNET_RecvData(CString& strRecvData, int nRecvSec = K_180_WAIT);		// Recv Data(Recv Data, Wait Time)
	//int 	fnNET_SendData(BYTE* szSendData, int nLen, int nSendSec = K_180_WAIT, int n3rdTranType = TRANHOST_ATM);	// Send Data(Send Data, Wait Time)  // [#2185] NH Justin 2013.05.06	Host Type Definition.
	int 	fnNET_SendData(BYTE* szSendData, int nLen, int nSendSec = K_180_WAIT, int n3rdTranType = TRANHOST_ATM, LPCTSTR sHostInfo = NULL);	// [#2412] NH Justin 2016.04.05	Support Extra Host Cert

	// [#2054] NH KJW 2011.05.03
	//int		fnNET_RecvData(CString& strRecvData, int nRecvSec = K_180_WAIT, int n3rdTranType = 0);			// Recv Data(Recv Data, Wait Time)
	int		fnNET_RecvData(LPBYTE pRecvBuffer, int *pnRecvLen, int nRecvSec = K_180_WAIT, int n3rdTranType = TRANHOST_ATM);	// [#2185] NH Justin 2013.05.06	Host Type Definition.
	// end of [#2054]
	// end of [#2012]

	int		fnNET_CloseLine();

	int 	fnNET_ModemTest(LPCTSTR szTestNum, int nTestOption = 0);							// Modem Test
	int		fnNET_ModemTest2();																	// [#GLDV-2683] NH Kook 2019.11.19 Modem Removal Detection
	int 	fnNET_NetworkTest(LPCTSTR szTestAddress, LPCTSTR szPort, LPCTSTR szSSLOption);		// Network Test	[#585] NH KSK 2009.12.03
	int		fnNET_PingTest(LPCTSTR szTestAddress, int* pOutRTT);								// Ping Test	[#2358] US Kook 2015.07.14

	int		fnNET_RMSConnectOpen(int	OpenType);
	int		fnNET_RMSConnectClose(void);
	BOOL	fbNET_RMSConnectCheck(void);
	int		fnNET_RMSConnectAccept(void);
	int		fnNET_RMSSendData(BYTE *pSendData, int nLen, int nSendSec);
	int		fnNET_RMSRecvData(BYTE *pRecvData, int *pnLen, int nRecvSec);

	// [#2075] NH KSK 2011.06.27
	int		fnNET_RKTConnectOpen();
	int		fnNET_RKTConnectClose();
	BOOL	fbNET_RKTConnectCheck();
	int		fnNET_RKTSendData(BYTE *pSendData, int nLen, int nSendSec, LPCTSTR Option);
	int		fnNET_RKTRecvData(BYTE *pRecvData, int *pnLen, int nRecvSec);
	// end of [#2075]
													
/////////////////////////////////////////////////////////////////////////////
//	SCR FUNCTION(SCR)
/////////////////////////////////////////////////////////////////////////////
// SCR SET MODE
	CScrCtrl* GetCurrentScreen();								// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
	int		fnSCR_ClearUserInputData();							// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
	int		fnSCR_SetVariable(CString Command, CString Data);	// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
	int		fnSCR_SetKeyEvent(CString Key);						// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
	int		fnSCR_SetByPassData(CString Key);
	int		fnSCR_SetActiveMode(NH_SCR_TYPE type);				// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
	CScrCtrl* GetScreenByType(NH_SCR_TYPE type);
	CString	fstrSCR_GetStringFromTextID(CString strTextID);		// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
	CString	fstrSCR_GetStringFromTextID(CString strTextID, int dccCustomer, BOOL isVisa);		
	CString	fstrSCR_GetStringByTextID(CString strTextID);		// [#2220] NH KMK 2014.02.10
	int		fnSCR_SetScrOwner(NH_SCR_TYPE type, CScrCtrl *pScr);	// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
	int		fnSCR_ClearErrorCode();					// Clear Error Code
	int		fnSCR_ClearKeyData();					// Clear Key Data
	int		fnSCR_SetDisplayData(CString szSetDisplayDataName, CString szSetDisplayDataValue);
													// Set Display Data(Display Data Name, Display Data Value)
	int		fnSCR_SetCurrentLangMode(int nLangMode = ENG_MODE);				
													// Set Current Language Mode
//	int		fnSCR_SetTextLocale(int nLangMode = ENG_MODE);		// [#2186] US KMK 2013.04.10 Asian Language Support
// SCR GET MODE
	CString	fstrSCR_GetErrorCode();					// Get Error Code
	int		fnSCR_GetDeviceStatus();				// Get Device Status

	CString	fstrSCR_GetKeyString(DWORD dwWaitSec = INFINITE);

	void	fstrSCR_WaitTime(DWORD dwWaitSec = INFINITE);
	void	fstrSCR_WaitMiliTime(DWORD dwWaitMili = INFINITE);
													// Get Key String(Init Flag)
	CString	fstrSCR_GetCheckScreen(int nScreenNo, int nLangMode = ENG_MODE);	
													// Get Check Screen(Screen No, Language Mode)
	int		fnSCR_GetCurrentScreenNo();				// Get Current Screen No
	int		fnSCR_GetCurrentLangMode();				// Get Current Language Mode

	int		fnSCR_IsFontLoaded(int nLanguage);		// [#2186] US KSK 2013.04.30

// SCR OPERATION MODE
	int		fnSCR_Initialize();						// Initialize
	int		fnSCR_Deinitialize();					// Deinitialize

	void	fnSCR_ShowScreen(int nScreenNo);
	int		fnSCR_DisplayPrevSet(int nScreenNo);	// Prepare Screen
	int 	fnSCR_DisplayScreen(int nScreenNo, int nDisplaySec = K_NO_WAIT, int nPinInputMode = PIN_DISABLE_MODE, LPCTSTR szCardData = _T(""), int nPinPassWordMin = 4, int nPinPassWordMax = 6, int nPinPassWordAuto = FALSE, LPCTSTR szPinPassWordTerm = _T("ENTER,CANCEL"), LPCTSTR szEnabledKey =_T(""));
	int		fnSCR_DisplayAdaPrevSet(int nScreenNo);	// Prepare Screen
	int 	fnSCR_DisplayAdaScreen(int nScreenNo, int nDisplaySec = K_NO_WAIT, int nPinInputMode = PIN_DISABLE_MODE, LPCTSTR szCardData = _T(""), int nPinPassWordMin = 4, int nPinPassWordMax = 6, int nPinPassWordAuto = FALSE, LPCTSTR szPinPassWordTerm = _T("ENTER,CANCEL"), LPCTSTR szEnabledKey =_T(""));
	int 	fnSCR_DisplayUpdate(int nScreenNo);
	int 	fnSCR_DisplayMessage();					// Display Message
	int		fnSCR_DisplayImage(int nImageNo, int nOnOffFlag);	// Display Image(Image No, OnOff Flag)
	int		fnSCR_DisplayImage(int nImageNo, CString szString);	// Display Image(Image No, OnOff Flag)			// [#65] NH AIREAT 2008.4.1 파라미터 CString으로 변경.
	int		fnSCR_DisplayStringArray(int nStringNo, LPCTSTR szString);
													// Display String Array(String No, String)
	int		fnSCR_DisplayString(int nStringNo, CString szString);												// [#65] NH AIREAT 2008.4.1 파라미터 CString으로 변경.
	int		fnSCR_DisplayFormat(int nStringNo, LPCWSTR szFmt, ...);
	int		fnSCR_DisplayHistory(int nCount);
	int		fnSCR_DisplayHistory(int nStringNo, CString szString);
													// Display String(String No, String)	
	int		fnSCR_ScanDeviceAction(int nDevId, int nScanTime, int nEventKind);
													// Scan Device Action(Device Id, Check Time, Event Kind)
	int		fnSCR_SetDebugMessage(CString strDbgMsg);	// [#2326] US Kook 2015.09.03 Support MoniAir

	/**
	 * Saves a screenshot of the screen to the SD card in the \screenshots directory
	 */
	void	fnSCR_SaveScreenshotToSD();
/////////////////////////////////////////////////////////////////////////////
//	BASIC FUNCTION(APL)
/////////////////////////////////////////////////////////////////////////////
// APL SET MODE
// APL GET MODE
// APL OPERATION MODE
	int		fnAPL_InitializeDS();					// Initialize DS(Variable)
	int		fnAPL_LoadDevice();						// Load Device
	int		fnAPL_UnLoadDevice();					// Unload Device

	int		fnAPL_GetDefineDevice(int nDevId);		// Get Define Device
	int		fnAPL_GetYesDevice(int nDevId);			// Get Yes Device
	int		fnAPL_GetNoDevice(int nDevId);			// Get No Device
	int		fnAPL_GetDownDevice(int nDevId);		// Get Down Device
	int		fnAPL_GetAvailDevice(int nDevId);		// Get Avail Device
	int		fnAPL_GetAvailErrorDevice(int nDevId);	// Get Avail Error Device
	int		fnAPL_GetErrorDevice(int nDevId);		// Get Error Device
	BOOL	fBAPL_IsDeviceStatusChanged(int nDevId);// Check Device Status Changed
	int		fnAPL_GetDownErrorDevice(int nDevId);	// Get Down Error Device
	int		fnAPL_GetAutoOffDevice(int nDevId);		// Get Auto Off Device

	int 	fnAPL_GetAvailTrans();					// Get Avail Transaction
	int 	fnAPL_GetAvailWithdraw();				// Get Avail Withdraw

	int		fnAPL_SetSensorInfo(BOOL bUpdateCSTCnt = FALSE); // Set Sensor Information
	int 	fnAPL_AddSerialNo(HOST_CONFIG hostConfig, int nSerialLength = 4);
	CString fnAPL_GetSerialNo(HOST_CONFIG hostConfig);

													// Add Serial No
	int 	fnAPL_SetProcCount(char chProcCount);	// Set Process Count

	int		fnAPL_SetHealthCheckTimer();			// [#36] KSK 2008.03.17
	UINT	fnAPL_GetHealthGapTime();				// Get Health Check Timer 2005.12.23

	int		fnAPL_ClearError(int nClearOption = CLEAR_ERROR_ALL);									// [#2492] NN Justin 2017.06.30 Add Parameter
													// Clear Error				
	int 	fnAPL_CheckError();						// Check Error
	int 	fnAPL_StackError(CString szErrorCode, CString szErrorString, int nDevId = DEV_NONE);	// [#419] [NH] KSK 2008.9.16
	CString fstrAPL_GetErrorProCount();				// [#419] [NH] KSK 2008.9.11
	CString fstrAPL_GetErrorCode(int nBracket = 0); // Get Current Error Code	[#419] [NH] KSK 2008.9.11
	CString fstrAPL_GetErrorMessage();				// Get Current Error Message

	int		fnAPL_CheckHostOpen();					// Check Host Open

	int		fnAPL_InitializeDevice(int nDevId, int CDUInitialType = 0);	// [#507] [NH] KSK 2009.2.23
													// Initialize Device
	int		fnAPL_CheckDevice();					// Check Device
	int		fnAPL_CheckDeviceAction(int nDevId, int nCheckTime = MID_DEVRSP_TIME);	// Check Device Action
	int		fnAPL_CheckMaterial(int nDevId = DEV_MAIN);		
													// Check Material
	int		fnAPL_RetractMaterial(int nDevId);		// Retract Material
	
	int		fnAPL_DisplayPrintError(int nInitFlag = FALSE);
													// Display & Print Error
	int		fnAPL_ResetDevice(int nDevId, int CDUInitialType = INIT_BY_OPENSESSION);	// [#507] [NH] KSK 2009.2.23 (0: OpenSessionSync, 1: Reset)
													// Reset Device
// [#68] HWANG 2008.04.02 EMV Level2
// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 공통화
	int		fnAPL_UnUsed(int nMode);							// Un Used
// end of [#68]
//	int 	fnAPL_DeviceEnDisable(int nDevId, int nAction, int nPinInputMode = PIN_DISABLE_MODE, LPCTSTR szCardData = _T(""), int nPinPassWordMin = 4, int nPinPassWordMax = 6, int nPinPassWordAuto = FALSE, LPCTSTR szPinPassWordTerm = _T("ENTER,CANCEL"), int nWaitFlag = FALSE, LPCTSTR szEnabledKey=_T(""));
	int 	fnAPL_DeviceEnDisable(int nDevId, int nAction, int nWaitFlag = FALSE, int nPinInputMode = PIN_DISABLE_MODE, LPCTSTR szCardData = _T(""), int nPinPassWordMin = 4, int nPinPassWordMax = 6, int nPinPassWordAuto = FALSE, LPCTSTR szPinPassWordTerm = _T("ENTER,CANCEL"), LPCTSTR szEnabledKey=_T(""));
													// Device EnDisable	
	int		fnAPL_EnDisableHotKey(int nEnDisable);	//-- SJKWONNOTE 2006-05-01 --ADD//

	int 	fnAPL_DeviceSetFlicker(int nDevId, int nAction);
													// Device Set Flicker
	int 	fnAPL_GetDeviceEvent(int nDevId, int nEventKind = EVENT_IN);		
													// Get Device Event

	CString	fstrAPL_ClerkInformationMake(int nIndex);	
													// Clerk Information Make	
	//int	fnAPL_BuildPinBlockWithAccountNo(CString sAccountNo);		// [#2150] US Justin 2012.10.04 Making common Pin Block Function.
	int		fnAPL_BuildPinBlockWithAccountNo(CString sAccountNo, int nMasterKeyName = MASTERKEY_ATM);		// [#2150] US Justin 2012.10.04 Making common Pin Block Function. // [#2181] US Justin 2013.03.12 Add Parameter (which MasterKey)
																											// [#2185] US Justin 2013.05.06 MasterKeyName Definition		
													// Build Pin Block with Account Number 

	CString	GetSWVersionInfo();
	int		GetDeviceType(CString strDeviceName);	// [#25] NH PSC Mcu type 체크기능추가
	BOOL	IsValidSurchargeTable();				// [#2389] US Justin Change Function Name (Prev:"IsValidCheckTerminalID();")   [#2136] US PCS 2012.05.30 "Additional Surcharge Functions For Table."	

/////////////////////////////////////////////////////////////////////////////
//	CASH FUNCTION ROUTING(CASH)
/////////////////////////////////////////////////////////////////////////////
// CASH SET MODE
	int		fnCDU_SetCstInitialDispInfo(int nTargetCst);	// [#380] [NH] KSK 2008.7.22
													// Set Value for Cash Dispenser's Cassette Info (SP Handling Dispinfo.ini)
// CASH GET MODE
//	int		fnCDU_GetAmountOfCST();					// Get Amount Of CST
	int		fnCDU_GetNumberOfAllCSTs();					// Get Amount Of CST	[#507] [NH] KSK 2009.2.23 함수명 변경
	int		fnCDU_GetAmountOfCash(int nCSTNo);		// Get Amount Of Cash(CST No)
	int		fnCDU_GetNumberOfDispenseCash(int nCSTNo);
	CString	fstrCDU_GetLastDispensedAmount();		// Get Last Dispensed Amount
		
// AntiSkimming Function
	BOOL	fbESU_IsAntiSkimmingAvailable();

// Check Latch Option available
	BOOL	fbIDC_IsLatchOptionAvailable();			// [#2458] AU KSK 2016.12.26

	// [#2382] US Justin 2015.12.11 Generate Mobile APP QR Code
	BOOL	fnAPL_MakeMobileAppQRCode(int nType);
	void	fnAPL_DisplayQRCodeOnScreen(CString strQRFileName);
	// End of [#2382]


	void	Disable_CardReader_RFID();				// [#2449] US Justin Make Function


/////////////////////////////////////////////////////////////////////////////
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDevCmn)
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CDevCmn)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
