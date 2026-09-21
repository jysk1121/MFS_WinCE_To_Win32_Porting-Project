// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__CE7435D0_0A78_4B12_ACCB_590B6C846944__INCLUDED_)
#define AFX_MAINFRM_H__CE7435D0_0A78_4B12_ACCB_590B6C846944__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afx.h"
#include "..\H\Common\CmnLib.h"
#include "..\H\Dll\nhmwi.h"
#include "..\H\Rms\RmsCtrl.h"

#include "..\H\Dev\DiagCard.h"
#include "..\H\Dev\DiagCdu.h"
#include "..\H\Dev\DiagSpr.h"
#include "..\H\Dev\DiagRfid.h"		// [#2325] NH KSK 2015.01.25
#include "..\H\Dev\DiagBcr.h"		// [#GLDV-3005] US Kook 2021.10.18 Support Side Car
#include "..\H\Dev\DiagBna.h"		// [#GLDV-3005] US Kook 2021.10.18 Support Side Car

#ifdef UNDER_CE
#  include "..\H\Dll\KeyMgr.h"		// [#541] AIREAT 2009.07.09 KEY MGR 통합.
#endif

#include "..\H\Dev\DevCmn.h"
#include "..\H\Ada\AdaCtrl.h"
#include "..\H\Scr\ScrCtrl.h"
#include "..\H\Tran\TranCmn.h"
#include "..\H\Net\Network.h"

#include "..\H\Tran\LibertyXService.h"
#include "..\H\Tran\DigitalMintService.h"

#ifndef NO_REMOTE_UPDATE
#  ifndef _LIBUPDATE_OWNER_
#    pragma comment(lib, "LibUpdate.lib")
#  endif
#  include "LibUpdate\LibUpdate.h"
#endif

typedef enum OPERATION_RESULT
{
	OPERATION_OK = 1,			// Operation OK
	OPERATION_OK_SPRNG,			// Operation OK, but SPR error
	OPERATION_NG,				// Operation NG
	OPERATION_NG_CDUNG,			// Operation NG, Because CDU error
	OPERATION_NG_DATANG,		// Operation NG, Because Invalid Data
	OPERATION_NG_INSERTOVER,	// Operation NG, Bacause Inserted value over
	OPERATION_NG_DENOM
};

typedef struct
{
 	WORD	LastNo1;
 	WORD	LastNo2;
	WORD	Valid;				// if not 9114 , then initialize NVRAM Log Area
	WORD	Rsvd2[5];
	BYTE	Buff[64*1024 - 16];
}	sLog, *pLog;

#define MAX_USER_INPUT	10

#define MAX_PIN_KEYINDEX	15		// [#2172] NH KSK 2012.12.27
#define MAX_AID_PER_PAGE	10		// [#2342] US Justin 2015.05.06 AID Selection

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

/////////////////////////////////////////////////////////////////////////////
// Attributes
/////////////////////////////////////////////////////////////////////////////
public:
	CDevCmn*		m_pDevCmn;
	CAdaCtrl*		m_pAdaCtrl;
	CTranCmn*		m_pTranCmn;
	CRmsCtrl*		m_pRmsCtrl;
	CNHMWI			Mwi;								// Mwi Object
	CScrCtrl*		m_arScrCtrl[SCR_LOCATION_MAX];

#ifndef NO_REMOTE_UPDATE
	bool				m_remoteUpdatesAvailable;
	LPUpdateSourceInfo	m_updateConfiguration;
	CUpdateRepoService* m_updateRepo;
	// System-wide/global cache for remote update data
	CRepoLocalCache*	m_updateCache;

	HANDLE				m_hScheduledUpdateThread;
	HANDLE				m_hLibertyXThread;
#endif

	CString			m_strNVLog;							// LOG Buffer
	int				nScrMaxTimeOver;					// Screen Max Time over
	int				nOperatorSwitchStatus;				// OP Switch Status
	int				nBinPage;							//	[#23] NH KGS 2008.03.17 BinList
	int				nInitFlag;
	int				m_nPageOfList;						// [#81] US KGS 2008.04.07 AID List
	int				m_nTotalNumList;
	CString			m_strListOfPage;					// [#2342] US Justin 2015.05.08 AID Selection
	int				nErrSumPage;						// [#421] [NH] KSK 2008.9.17
	int				nLastUploadedJNLCount;				// [#613] AU_C KSK 2010.01.18

	BOOL			bNeedRebootOnInit;					// [#2186] US KMK 2013.05.06 Reboot Flag

	COleDateTime	m_tNextReboot;						// [#2558] NH Justin 2018.06.14 Add Scheduled Reboot Option

	// [iTM] US AIREAT 2009.12.02 : OP Wizard
	typedef struct tagQuickReplenish
	{
		CString		m_strStartTime;
		CString		m_strEndTime;
		CString		m_strResultOfDayTotal;
		CString		m_strResultOfCstTotal;
		CString		m_strResultOfAddCash;
	} QuickReplenish;
	QuickReplenish	m_QuickReplenishInfo;
	// end of [iTM] Op Wizard

	// [iTM] US KSK 2010.01.12 : Quick Configuration
	typedef struct tagQuickConfiguration
	{
		CString		m_strStartTime;
		CString		m_strResultMasterPassword;	// MasterPassword 설정
		CString		m_strEndTime;
	} QuickConfiguration;
	QuickConfiguration m_QuickConfigInfo;
	// end of [iTM] Quick Configuration

	typedef struct tagNetworkInformation
	{
		int		nLineType;

		int		nTCPIPFlowControl;		// TCP/IP
		CString szEOTCheckMode;			// DIALUP
		CString szMessageType;

		int 	nATM_DHCP;
		CString	szATM_IpAddr;
		CString	szATM_Subnet;
		CString	szATM_Gateway;
		CString	szATM_DNS;
		CString	szATM_DNS_2nd;		// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
		CString	szTerminalID;		// [#398] [NH] KSK 2008.8.6

		// US, CA, MX 추가
		CString	szHostPhoneNo1;
		CString	szHostAddress1;
		int		nHostPortNo1;
		int		nStandard3StatusMonitoringFlag;
		int		nStandard3CommunicationFlag;
		int		nStandard3CRCFlag;
		int		nSSLFlag;
		CString	szStandard3CommunicationID;
		CString szStandard1RoutingID;
		int		nKeyMode;
		int		nKeyIndex;
		CString	strKeyCheckSum;
		CString strMacKeyCheckSum;

		int		nEMVKernelVer;		// [#2226] US Justin 2013.10.23 Kernel Selection
		CString szEMVAIDStatus;		// [#2226] US Justin 2013.10.23 AID Selection

		int		nCDUBindingOption;	// [#2891] NH Justin 2016.01.27 Reboot if CDU Binding Option is changed.

	} NetworkInformation;
	NetworkInformation m_netInfo;
	
	CStringArray	Chk_Key;							// [#29] NH PSC 2008.03.18 memory leak 대책으로 전역변수->멤버변수로 수정.

	BOOL			bSendErrorAfterRecover;			// [#115] KSK 2008.04.16 bSendHealthAfterRecover 변수 용도 변경
	int				Mode_CSTStatus;					// [#149] KSK 2008.04.21

	struct tagOpInfo
	{
		tagOpInfo()
		{
			nNoteCount = 1;
		};

		int				Doing;
		
		int				nLoginMode;						// Op Login Mode (권한)
		int				nMoveProcReason;				// Move Procedure reason						// [#236] NZ AIREAT 2008.05.27
		int				nProcNextScrNum;				// Next Procedure Number
		int				nProcPrevScrNum;				// Previous Procedure Number
		int				nLibertyXScrNum;				// Next Procedure Number for LibertyX and PAI	// [#RWC6-512] US ryan.payton 2023.04.25 [PAI] Software Enhancement

		int				nCurScrNum;						// 화면에 표시되고 있는 화면 번호
		int				nTimeout;						// 화면 Timeout 값
		int				nProcLoofCnt;					// ProcLoofCnt

		int				bUpdateFlag;					// 화면 갱신 Flag, TRUE - 화면을 업데이트 한다. FALSE 화면을 업데이트 하지 않는다.

		CString			strTempButtonInformation;
		CString			strSaveButtonInformation;
		CString			strTempClerkInformation;
		CString			strSaveClerkInformation;		// read only
		int				nMsgClearCount;
		CString			strTempClerkMessage;
		CString			strSaveClerkMessage;			// read only
		CString			strReturn;
		CStringArray	strTempArray;
		CString			strSelectedMachineType;			// [#476] [NH] KSK 2009.1.5
		int				nMultiCurrencyType;				// [#2115] [MX] KSK 2012.02.05 Multi Currency 지원을 위해 추가 (MX only)

		CString			strFromToDateScreen;			// JNL Search Date string for screen	// [#508] AIREAT 2009.02.27 : Search by date
		CString			strFromToDateFile;				// JNL Search Date String for File		// [#508] AIREAT 2009.02.27 : Search by date
		CStringArray	strJNLSearchResult;				// JNL Search Result					// [#508] AIREAT 2009.02.27 : Search by date
		CString			strTestPhoneNumber;
		CString			strISO1Track;
		CString			strISO2Track;
		CString			strISO3Track;
		int				nISO1Length;
		int				nISO2Length;
		int				nISO3Length;
		CString			strATRData;

		int				nTestCount;
		int				nNoteCount;
		int				nProcessingCount;
		CString			strDiagExitMessage;
		CString			strAllFlicker;
		CString			strEPPFlicker;
		CString			strCDUFlicker;
		CString			strSPRFlicker;
		CString			strMCUFlicker;
		CString			strSideCarFlicker;			// [#GLDV-3005] US Kook 2022.01.18 Support Side Car
		BYTE			FlickingDevice;
		BYTE			OnOffDevice;

		USHORT			m_pusCstNum;
		ULONG			m_ulCstIndex[CDU_MAX_CST_COUNT];
		TCHAR			m_szCstCountry[4];
		USHORT			m_mbCDUType;

		CString			m_strnetworkDestination;	// [#585] NH KSK 2009.12.03
		CString			m_strPort;					// [#585] NH KSK 2009.12.03
		int				m_nSSLOption;				// [#585] NH KSK 2009.12.03
		int				m_nSSLVersion;
		int				m_nSSLCert;					// [#2093] NH PCS 2011.10.31

		CString			m_strMsg;
		CString			m_UserInputValue[MAX_USER_INPUT];

		int				nSelectedCountryInfo;			// [#2042] AU KSK 2011.04.02
		int				nSelectedNVRamClearType;		// [#2042] AU KSK 2011.04.02

		// [#2074] NH KSK 2011.07.09
		int				nAntiSkimmingEnable;
		int				nAntiSkimmingShutter;
		int				nAntiSkimmingTimeThreshold;
		int				nAntiSkimmingAutoRecovery;		// [#2085] NH KSK 2011.07.22
		CString			strAntiSkimmingVoltage;			// Diagnostics에서 사용
		CString			strAntiSkimmingTemperature;		// Diagnostics에서 사용
		CString			strAntiSkimmingStatus;			// Diagnostics에서 사용
		// end of [#2074]

		int				nTempHaloLedATM_Mode;			// [#2205] US KSK 2013.06.28
		int				nHaloLedMode;					// [#2205] US KSK 2013.06.28
		int				nHaloLedColor;					// [#2205] US KSK 2013.06.28

		int				nTempSelectedAID;				// [#2226] US Justin 2013.10.23 En/Disable AID...
	} m_OpInfo;

	// [#GLDV-3005] US Kook 2021.10.20 Support Side Car
	struct tagVdmInfo
	{
		/*
		typedef enum VdmVariables
		{
			BNAAcceptValue,
			BNAAcceptCount,
			BNARejectCount
		} VDMVAR;
		*/

		// TODO: Would it be better than various variables?
		//CMap<VDMVAR, VDMVAR, CString, LPCTSTR>		tempStrings;
		//CMap<VDMVAR, VDMVAR, int, int>				tempInts;
		CString			strBarcodeData;
		int				nAcceptCount;
		int				nAcceptValue;
		int				nRejectCount;

		bool			bBillAccepted;
		bool			bBarcodeAccepted;
	} m_VdmInfo;
	// end of [#GLDV-3005]

	
	COleDateTime	m_RMSSendTime;						// for RMS // [#] AIREAT 2009.06.05
	BOOL			m_bRMSStatusInitSend;				// [#2113] NH KSK 2012.01.03 최초 1번 RMS Status 송신을 위한 Flag
	BOOL			m_bLastRMSSendResult;				// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	int		m_nPINInputCount;							// input count of PIN 
#endif

#if (APP_DIGITALMINT)
private:
	bool			m_bAcceptorStatusChanged;
	COleDateTime	m_tLastAcceptorStatusCheck;
#endif

/////////////////////////////////////////////////////////////////////////////
// Operations
/////////////////////////////////////////////////////////////////////////////
public:
	/****************************************************************************/
	/* MainFrm.cpp																*/
	/****************************************************************************/
	void	SetOwner(CMainFrame *pOwner);						// Set Owner

	/****************************************************************************/
	/* AtmModeCtrl.cpp															*/
	/****************************************************************************/
	void	AtmStart();
	int		AtmModeCtrl();

	// [#23] KSK 2008.03.19 ModeCtrl 단순하기 위해 함수 생성
	BOOL	Check_CSTSensor();
	BOOL	Check_CSTRjtFull();
	BOOL	Check_DoorSensor();
	BOOL	Check_AntiSkimming();
	BOOL	Check_BrokenNVRAM();
	BOOL	UpgradePasswords();						// [#RWC6-56] US William 2019.12.26 Hash passwords
	BOOL	Check_Camera();							// [#2518] US Kook 2018.02.26
	BOOL	Check_EPPInitState();					// [#2580] AU Kook 2019.08.06
	// end of [#23]

	void	SetNextRebootTime();					// [#2558] NH Justin 2018.06.14 Add Scheduled Reboot Option

	BOOL	Mode_CheckErrorElements();
	BOOL	Mode_CheckAdditionalATMStatus();		// [#2277] NH Justin 2014.06.10 Additional ATM Status

	// for RMS
//	BOOL	ProcRMSCtrl(int OpenType);
	BOOL	ProcRMSCtrl(int OpenType, char SubCommand = 0);	// [#613] AU_C KSK 2010.01.18

	/****************************************************************************/
	/* AtmMainLib.cpp															*/
	/****************************************************************************/
	int		TerminateATM(CString strGuideText = "", BOOL bMsgDisplay = TRUE, BOOL bShutdown = FALSE);			// Terminate Process
	CString SaveLogData();							// [#31] NH AIREAT 2008.03.18  BackupLogData -> SaveLogData 변경.
	BOOL	BackupLogData(CString strDestPath);		// [#31] NH AIREAT 2008.03.18 
	int		ExecuteSWUpdate();
	int		KillProcByName(const char *szToTerminate);
	int		KillProcByGoma(CString sExeName);
	int		KillProcess(CString sExeName);
	BOOL	ResetDeviceOf(int);
	BOOL	Check_NVRamData();						// Check NVRAM Data KSK_2007-11-09오후 12:46:51
	int		Main_NMS_Proc();						// [#8] KSK 2008.03.19 RMS / HEALTH CHECK 일원화를 위해 Tran 부분을 Main쪽으로 옮김
	int		GetCurrnetRMSLineType();				// for RMS
	void	Main_SetAtmStatus(int nStatus);			// Status Set

	/****************************************************************************/
	/* AtmClerkCtrl.cpp															*/
	/****************************************************************************/
	void	ClerkClearAllNVRAM(CString strMachineType);			// [#2442] US Justin Clear All NVRAM AREA
	int		ClerkProc();										// Clerk Procedure

	// Op Screen Proc
	//----------------------------------------------------------------------------
//	void	ClerkResetMasterPassword();							// Clerk Reset Master Password				(706)	// [iTM] US AIREAT 2009.12.15	[#2070] NH KSK 2011.06.16 AP로 변경함에 따라 제거
	void	ClerkExitSupervisor();								// Clerk Op Exit Procedure					(711)	// [iTM] US AIREAT 2009.12.15
	void	ClerkOpInputPinProc();								// Clerk Op Input Pin Procedure				(902)
	void	ClerkOpModeProc();									// Clerk Op Mode Procedure					(901)
	void	ClerkOpErrorModeProc();								// Clerk Op Error Mode Procedure			(900) [#539] US KSK 2009.06.29
	void	ClerkOpInitializeProc();							// Clerk OP Initialize Procedure			(899) [#539] US KSK 2009.07.06
	void	ClerkSitemapProc();									// Clerk OP Sitemap Procedure				(898)
	//----------------------------------------------------------------------------

	// [1] (901) OPMAIN - SUB
	void	ClerkReplenishProc();								// Clerk Replenish Procedure				(903)
	void	ClerkClearNvramProc();								// Clerk Clear NVRAM Procedure				(908)
#if (US_VERSION || CA_VERSION || MX_VERSION)					// [#2180] NH Justin 2013.03.12  Add 5000SE to CA,MX as well
	void	ClerkSelectMachineTypeProc();						// Clerk Select Machine Type Procedure		(807)  [#476] [NH] KSK 2009.1.5 기종 선택 화면 추가 //[#2097] US PCS 2011.11.15
#endif
	void	ClerkJournalProc();									// Clerk View Journal Procedure				(921)
	void	ClerkReportProc();									// Clerk Report Procedure					(931)
	//----------------------------------------------------------------------------
	
	// [2] (903) REPLENISH - SUB
	//----------------------------------------------------------------------------
	void	ClerkAddCashProc();									// Clerk Add Cash Procedure					(912)
	void	ClerkQuickReplenish();								// Clerk Quick Replenish					(701)	// [iTM] US AIREAT 2009.11.27 OP Wizard
	void	ClerkQuickDayTotal();								// Clerk Quick Replenish : Day Total		(702)	// [iTM] US AIREAT 2009.11.27 OP Wizard
	void	ClerkQuickCassetteTotal();							// Clerk Quick Replenish : Cassette Total	(703)	// [iTM] US AIREAT 2009.11.27 OP Wizard
	void	ClerkQuickAddCash();								// Clerk Quick Replenish : Add Cash			(704)	// [iTM] US AIREAT 2009.11.27 OP Wizard
	void	ClerkQuickResult();									// Clerk Quick Replenish : Result			(705)	// [iTM] US AIREAT 2009.11.27 OP Wizard
	//----------------------------------------------------------------------------

	// [2] (904) CONFIGURE - SUB
	//----------------------------------------------------------------------------
	void	ClerkCustomerSetupProc();							// Clerk Customer Setup Procedure			(941)
	void	ClerkSystemSetupProc();								// Clerk System Setup Procedure				(951)
	void	ClerkHostSetupProc();								// Clerk Host Setup Procedure				(961)
	void	ClerkTransactionSetupProc();						// Clerk Transaction Setup Procedure		(981)

	// [iTM] US KSK 2010.01.12 : Quick Configuration
	void	ClerkQuickConfigurationProc();						// Clerk Quick Configuration Procedure		(712)
	void	ClerkQuickConfCancelConfirmProc();					// Clerk Quick Conf Cancel Confirm Procedure(713)
	void	ClerkQuickCommunicationProc();						// Clerk Quick Communication Procedure		(714)
	void	ClerkQuickTCPIPTypeProc();							// Clerk QuickTCPIPType Procedure			(715)
	void	ClerkQuickEotEnqOptionProc();						// Clerk Quick Eot/Enq Option Procedure		(716)
	void	ClerkQuickMessageFormatProc();						// Clerk Quick MessageFormat Procedure		(717)
	void	ClerkQuickTelephoneNumberProc();					// Clerk Quick Telephone Number Procedure	(718)
	void	ClerkQuickProcessorProc();							// Clerk Quick Processor Procedure			(719)
	void	ClerkQuickHostIpSetupProc();						// Clerk Quick Host IP Setup Procedure		(720)
	void	ClerkQuickRemoteMonitorProc();						// Clerk Quick Remote Monitor (RMS) Procedure(721)
	void	ClerkQuickChangeOperatorPasswordProc();				// Clerk Quick Change Operator Password Procedure(722)
	void	ClerkQuickOperatorPasswordProc();
	void	ClerkQuickATMDHCPSettingProc();						// Clerk Quick ATM DHCP Setting Procedure	(723)
	void	ClerkQuickAtmIpSetupProc();							// Clerk Quick ATM IP SETUP Procedure		(724)
	void	ClerkQuickChangeServicerPasswordProc();				// Clerk Quick Change Servicer Password Procedure(726)
	void	ClerkQuickServicerPasswordProc();
	void	ClerkQuickRevenueAddedFeatures1Proc();				// Clerk Quick Revenue Added Features 1 Procedure(728)
	void	ClerkQuickTerminalIDViewProc();						// Clerk Quick Terminal ID View Procedure	(729)
	void	ClerkQuickRevenueAddedFeatures2Proc();				// Clerk Quick Revenue Added Features 2 Procedure(730)
	void	ClerkQuickRoutingIDViewProc();						// Clerk Quick Routing ID View Procedure	(731)
	void	ClerkQuickAcceptConfigurationProc();				// Clerk Quick Accept Configuration Procedure(732)
	void	ClerkQuickStandard3OptionProc();					// Clerk Quick Standard3Option Procedure	(733)
	void	ClerkQuickChangeMasterPasswordProc();				// Clerk Quick Master Password Procedure	(735)
	void	ClerkQuickMasterPasswordProc();
	void	ClerkQuickConfigResultViewProc();					// Clerk Quick Config Result View Procedure	(736)
	// end of [iTM] : Quick Configuration

#if (AU_VERSION)
	int		ClerkAUCountrySetup();								// Clerk Setup Country Information			(740)	// [#2042] AU KSK 2011.04.02
#endif

	//----------------------------------------------------------------------------

	// [2] (914) RCOPY
	//----------------------------------------------------------------------------
//#if (AU_VERSION) // KSK 2011.06.13 하기 함수 사용 안함으로 인해 주석 처리함
//	int		ClerkBackupNVRAM();									// Clerk Backup NVRAM						(918)		// [#253] NZ AIREAT 2008.06.04
//	int		ClerkRestoreNVRAM();								// Clerk Restore NVRAM						(919)		// [#253] NZ AIREAT 2008.06.04
//#endif
	void	ClerkSearchJournalbyDateProc();						// Clerk Journal Search by Date				(816)		// [#508] NH AIREAT 2008.03.02 JNL SEARCH BY DATE
	//----------------------------------------------------------------------------

	// [2] (921) E-JOURNAL - SUB
	//----------------------------------------------------------------------------
	void	ClerkViewJournalProc();								// Clerk View Journal Procedure				(922)
	void	ClerkJournalUploadToHostProc();						// Clerk Journal Upload to Host Procedure	(744)	// [#2076] NH KSK 2011.06.29
	//----------------------------------------------------------------------------

	// [2] (931) REPORT - SUB
	//----------------------------------------------------------------------------
	void	ClerkSWVersionProc();								// Clerk SW Version Procedure				(932)
	void	ClerkErrorCodeView();								// Clerk ErrorCode View						(810)		// [#] NH AIREAT 2008.07.21 ERROR CODE VIEW
	void	ClerkErrorSumProc();								// Clerk Error Sum							(812)		// [#421] NH 2008.9.16
	void	ClerkOptOutProc();									// Clerk Opt-Out Report						(813)		// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report
	void	ClerkRejectAnalysisProc();							// Clerk RejectAnalysis Procedure			(818)		// [#526]
	//----------------------------------------------------------------------------

	// [3] (941) CUSTOM SETUP - SUB
	//----------------------------------------------------------------------------
	void	ClerkChangeMessageProc();							// Clerk Change Message Procedure			(942)
	void	ClerkAUSurchargeModeProc();							// Clerk Surcharge Mode Procedure			(945)	//[#2000] SOOK 2010.09.27 호주 적용
	void	ClerkSurchargeModeProc();							// Clerk Surcharge Mode Procedure			(946)
	void	ClerkPercentSurchargeProc();						// Clerk Percent Surcharge Procedure		(959)	// [#112] US KGS 2008.04.15 Percent Surcharge Menu
	//void	ClerkPercentOptionProc();							// Clerk Percent Surcharge OptionProcedure	(960)	// [#2092] US PCS 2011.10.24
	void	ClerkAddTionalSurchargeOptionProc();				// Additional Surcharge Proc				(960)   // [#2136] US/CA PCS 2012.05.30
	void	ClerkTableSurchargeOptionProc();					// Surcharge Table Option Added.			(963)	// [#2136] US/CA PCS 2012.05.30

	void	ClerkBinListProc();									// Clerk BinList Procedure					(947)
	void	ClerkEditBinListProc();
#if (AU_VERSION)	// [#622] NH KSK 2010.02.24	// [#2069] NH KSK 2011.06.13
	void	ClerkBinManagementProc();							// Clerk Bin Management Procedure			(858)	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함
	void	ClerkBinPropertiesProc();							// Clerk Bin Properties Procedure			(869)	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함 
	void	ClerkViewBinListProc();								// Clerk View BIN List Procedure			(862)	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함
	void	ClerkBinConfirmationProc();							// Clerk BIN Confirmation Procedue			(859)	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함
#endif
	void	ClerkAdvertisementProc();							// Clerk Advertisement Procedure			(948)
	void	ClerkWelcomeAdvProc();
	void	ClerkTransactionAdvProc();
	void	ClerkTranAdvertisement1Proc();
	void	ClerkTranAdvertisement2Proc();
	void	ClerkTranAdvertisement3Proc();
	void	ClerkTranAdvertisement4Proc();
	void	ClerkTranAdvertisement5Proc();
	void	ClerkTranAdvertisement6Proc();
	void	ClerkSelectCouponOptionProc();						// Clerk CouponSetup Procedure				(949)
	void	ClerkBasicCouponSettingProc();						// Clerk Basic Coupon Setting Procedure		(803)
	void	ClerkBasicCoupon1Proc();							// Clerk Basic Coupon 1 Procedure			(831)
	void	ClerkBasicCoupon2Proc();							// Clerk Basic Coupon 2 Procedure			(832)
	void	ClerkBasicCoupon3Proc();							// Clerk Basic Coupon 3 Procedure			(833)
	void	ClerkBasicCoupon4Proc();							// Clerk Basic Coupon 4 Procedure			(834)
	void	ClerkBasicCoupon5Proc();							// Clerk Basic Coupon 5 Procedure			(835)
	void	ClerkBasicCoupon6Proc();							// Clerk Basic Coupon 6 Procedure			(836)

	void	ClerkAusBasicCoupon1Proc();							// Clerk Aus Basic Coupon 1 Procedure		(837)
	void	ClerkAusBasicCoupon2Proc();							// Clerk Aus Basic Coupon 2 Procedure		(838)
	void	ClerkAusBasicCoupon3Proc();							// Clerk Aus Basic Coupon 3 Procedure		(839)
	void	ClerkAusBasicCoupon4Proc();							// Clerk Aus Basic Coupon 4 Procedure		(840)
	void	ClerkAusBasicCoupon5Proc();							// Clerk Aus Basic Coupon 5 Procedure		(841)
	void	ClerkAusBasicCoupon6Proc();							// Clerk Aus Basic Coupon 6 Procedure		(842)

	void	ClerkEnhancedCouponSettingProc();					// Clerk Enhanced Coupon 1 Procedure		(874)
	void	ClerkEnhancedCoupon1Proc();							// Clerk Enhanced Coupon 1 Procedure		(984)
	void	ClerkEnhancedCoupon2Proc();							// Clerk Enhanced Coupon 2 Procedure		(985)
	void	ClerkEnhancedCoupon3Proc();							// Clerk Enhanced Coupon 3 Procedure		(986)
	void	ClerkEnhancedCoupon4Proc();							// Clerk Enhanced Coupon 4 Procedure		(987)
	void	ClerkEnhancedCoupon5Proc();							// Clerk Enhanced Coupon 5 Procedure		(988)
	void	ClerkEnhancedCoupon6Proc();							// Clerk Enhanced Coupon 6 Procedure		(989)

	void	ClerkSelectHostProc();								// Clerk Select Host Procedure				(971)
	void	ClerkOptionalSettingProc();							// Clerk Optional Setting Procedure			(996)	// [#54] NH KGS 2008.03.27 Optional Setting
	void	ClerkEMVProc();										// Clerk EMV Procedure						(950)	// [#81] US KGS 2008.04.07 AID List
	void	ClerkStandard1OptionProc();							// Clerk Standard1 Option Procedure			(870)	// [#563] CA KSK 2009.8.20
	void	ClerkStandard1ExtenedProc();						// Clerk Standard1 Extended Function Proc	(994)	// [#2150] US Justin 2012.10.08
	void	ClerkDCCCustomOptionProc();							// Clerk DCC Custom Option					(794)	// [#2187] US Justin 2013.04.18
	void	ClerkServiceProc();
	void	ClerkNoticeServiceProc();
	void	ClerkChangBackgroundProc();
	void	ClerkDefaultBackgroundProc();
	void	ClerkBackScreen1Proc();
	void	ClerkBackScreen2Proc();
	void	ClerkBackScreen3Proc();
	void	ClerkBackScreen4Proc();
	void	ClerkBackScreen5Proc();
	void	ClerkBackScreen6Proc();

	//----------------------------------------------------------------------------
	// [3] (961) HOST SETUP - SUB
	//----------------------------------------------------------------------------
	void	ClerkKeyManagementProc();							// Clerk Key Management Procedure			(742)	// [#2080] NH KSK 2011.06.27
	void	ClerkRKTSetupProc();								// Clerk Remote Keys Trasfer Setup Procedure(743)	// [#2080] NH KSK 2011.06.27
	void	ClerkTelephoneNumberProc();							// Clerk Telephone Number Procedure			(962)
	void	ClerkHealthCheckMsgProc();							// Clerk Health Check Message Procedure		(964)
	void	ClerkRemoteMonitorProc();							// Clerk Remote Monitor Procedure			(965)
	void	ClerkHostIpSetupProc();								// Clerk Host IP Setup Procedure			(980)
	void	ClerkAutoDayTotalProc();							// Clerk Auto Day Total Procedure			(804)	[#415] NZ AIREAT 2008.09.08 Auto dya total
	void	ClerkConfigAtStartProc();							// Clerk Configuration at start procedure	(966)
	//----------------------------------------------------------------------------
	
	// [#16] NH KGS 2008.3.12 REVERSAL RETRY COUNT
	//----------------------------------------------------------------------------
	void	ClerkReversalRetryProc();							// Clerk Reversal Retry Count Procedure		(990)
	//----------------------------------------------------------------------------
	// end of [#16]

	// [3] (981) TRANSACTION SETUP - SUB
	//----------------------------------------------------------------------------
	void	ClerkSetFastCashProc();								// Clerk Set Fast Cash Procedure			(982)
	void	ClerkSetDenominationProc();							// Clerk Set Denomination Procedure			(983)
	bool	ChangeDenominations(int newDenominations[4], CString &errorMessage);

	// [#2472] US Justin 2017.02.09 DCC Withdrawal Option
	BOOL	ValidateDispenseLimitInput(int nInputAmt, CString &strRtnMsg);	
	BOOL	ValidateFastCashInput(int nInputAmt, CString &strRtnMsg, int nDispLimitAmt /*dollar*/, BOOL bDCCFastCash=FALSE);	
	// End of [#2472]

#if (MX_VERSION)
	void	ClerkSetDenominationProc_MX();						// Clerk Set Denomination MX Procedure		(808)	// [#2115] MX KSK 2012.02.05
#endif

	int		ClerkExtendedBinList();								// Clerk ExtendedBinList Procedure			(809)	// [#522] US KSK 2009.05.18
	//----------------------------------------------------------------------------

	// [3] (951) SYSTEM SETUP - SUB
	//----------------------------------------------------------------------------
	void	ClerkClockSetupProc();								// Clerk Clock Setup Procedure				(952)
	void	ClerkAdjustVolumeProc();							// Clerk Adjust Volume Procedure			(953)
	void	ClerkLanguageProc();								// Clerk Language Procedure					(993)	// [#2166] NH KSK 2012.11.13
//	void	ClerkLanguageEnDisableProc();						// Clerk Language En/Disable Procedure		(954)
	void	ClerkTransLanguageProc();							// Clerk Language En/Disable Procedure		(954)	// [#2166] NH KSK 2012.11.13 Change Function Name
	void	ClerkSupervisorLanguageProc();						// Clerk Supervisor Language Procedure		(992)	// [#2166] NH KSK 2012.11.13
	void	ClerkChangePasswordProc();							// Clerk Change Password Procedure			(956)
	void	ClerkModemSetupProc();								// Clerk Modem Setup Procedure				(957)
	void	ClerkAtmIpSetupProc();								// Clerk ATM IP SETUP Procedure				(979)
	//----------------------------------------------------------------------------

	// [4] (948) ADVERTISEMENT - SUB
	//----------------------------------------------------------------------------
	void	ClerkAdvertisement1Proc();							// Clerk Advertisement #1 Procedure			(934)			// [#14] NH AIREAT 2008.03.26 FOR ADVERTISEMENT
	void	ClerkAdvertisement2Proc();							// Clerk Advertisement #2 Procedure			(935)			// [#14] NH AIREAT 2008.03.26 FOR ADVERTISEMENT
	void	ClerkAdvertisement3Proc();							// Clerk Advertisement #3 Procedure			(936)			// [#14] NH AIREAT 2008.03.26 FOR ADVERTISEMENT
	void	ClerkAdvertisement4Proc();							// Clerk Advertisement #4 Procedure			(937)			// [#14] NH AIREAT 2008.03.26 FOR ADVERTISEMENT
	void	ClerkAdvertisement5Proc();							// Clerk Advertisement #5 Procedure			(938)			// [#14] NH AIREAT 2008.03.26 FOR ADVERTISEMENT
	void	ClerkAdvertisement6Proc();							// Clerk Advertisement #6 Procedure			(939)			// [#14] NH AIREAT 2008.03.26 FOR ADVERTISEMENT
	//----------------------------------------------------------------------------

	// [4] (971) SELECT HOST - SUB
	//----------------------------------------------------------------------------
	void	ClerkStandard3OptionProc();							// Clerk Standard3Option Procedure			(972)
	void	ClerkModemTypeProc();								// Clerk ModemType Procedure				(973)
	void	ClerkCommunicationProc();							// Clerk Communication Procedure			(974)
	void	ClerkMessageFormatProc();							// Clerk MessageFormat Procedure			(975)
	void	ClerkTCPIPTypeProc();								// Clerk TCPIPType Procedure				(976)
	//----------------------------------------------------------------------------

	// [4] (956) CHANGE PASSWORD - SUB
	//----------------------------------------------------------------------------
	void	ClerkOperatorPasswordProc();						// Clerk Operator Password Procedure		(905)
	void	ClerkServicePasswordProc();							// Clerk Service Password Procedure			(906)
	void	ClerkMasterPasswordProc();							// Clerk Master Password Procedure			(907)
	//----------------------------------------------------------------------------

	// [4] (996) OPTIONAL SETTING - SUB
	//----------------------------------------------------------------------------
	void	ClerkMOD10Proc();									// Clerk MOD10 Procedure						(916)		// [#] NH KGS 2008.06.10 Op.dat 통합
	void	ClerkPreBalanceProc();								// Clerk Pre-Balance Procedure					(920)		// [#] NH KGS 2008.06.10 Op.dat 통합
	void	ClerkAccountsProc();								// Clerk Accounts Procedure						(997)		// [#57] UK KGS 2008.03.27 Accounts
	int		ClerkAIDListProc();									// Clerk AID List Procedure						(998)		// [#81] US KGS 2008.04.07 AID List
	void	ClerkEMVOtherOptionsProc();							// Clerk EMV Other Options						(767)		// [#2549] NH Justin 2018.05.08 Add Perto Rico Option	
	void	ClerkSelectReceiptOptionProc();						// Clerk Select Receipt Option Procedure		(915)		// [#354] CA PSC 2008.07.02 Receipt Select Option 처리

	// [#2365] US Justin 2015.07.24 Add DCC+ on Canadian Version
#if (US_VERSION || CA_VERSION || MX_VERSION)
	void	ClerkOtherTransactionProc();						// (795) [#2013] NH KJW 2011.01.23
	void	ClerkTDLOptionSetupProc();							// (749) [#2292] US Justin 2014.09.17 Cardtronics TDL Option
	CString GetEnabledOtherTransaction(int nOrder, CString &strServiceName);		// [#2419] US Justin 2016.04.20 Assign OP Menu Dymically
#endif
	// End of [#2365]

#if (US_VERSION)
	void	ClerkPOPMoneySetupProc();							// (753) [#2350] US Justin 2015.06.22 
	void	ClerkDualHostDCCProc();								// (791) [#2185] US Justin 2013.04.10
	void	ClerkDualHostDCCHostIpSetupProc();					// (792) [#2185] US Justin 2013.04.10
	void	ClerkHalCashSetupProc();							// (759) [#2396] US Justin 2016.02.17 Pin4
	void 	ClerkJustCashSetupProc();							// (761) [#2445] US Justin 2016.09.92 Add Just.Cash Setup
	void 	ClerkPaypalCCASetupProc();							// (762) [#2446] US Justin 2016.09.29 Add Paypal CCA Setup
#endif

// [RWC6-162] Add B4U to Canada Release	
#if (US_VERSION || CA_VERSION)
	void	ClerkB4USetupProc();								// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
#endif
// end of [RWC6-162]

#if (APP_GPAY_GIFTCARD_PURCHASE)
	void	ClerkGivePaySetupProc();							// (748) [#2535] US Justin 2018.03.20 GivePay Online
	void	ClerkGivePayGenerateQRCode();						// [#RWC6-44] GivePay AutoEnrollment QR Code Display

#endif

	void	ClerkPreDialOptionProc();							// Clerk PreDial Option	 Procedure				(817)		// [#525] US KSK 2009.05.21
	void	ClerkNeedMoreTime();								// Clerk Need More Time Procedure				(819)		// [#528] AU AIREAT 2009.06.02 NEED MORE TIME
	void	ClerkReceiptPaperCutSetupProc();					// (755) [#2371] US Justin 2015.09.08
	//----------------------------------------------------------------------------

	// [4] (965) REMOTE MONITOR - SUB
	//----------------------------------------------------------------------------
	// [#78] NH KGS 2008.04.02 RMS OP 메뉴 추가
	void	ClerkRMSListenningPortProc();						// Clerk Remote Monitor Procedure			(926)
	void	ClerkRMSStatusProc();								// Clerk Remote Monitor Procedure			(927)
	void	ClerkRMSPhoneNumberProc();							// Clerk Remote Monitor Procedure			(930)
	//----------------------------------------------------------------------------
	// end of [#78]
	void	ClerkRMSRingCountProc();							// Clerk RMS Ring Count Procedure	[#401] [NH] KSK 2008.8.14

	// [#2234] NH KMK 2014.01.13 MoniView Timeout
//#if (AU_VERSION)
	void	ClerkRMSTCPIPTimeoutProc();								// Clerk RMS Timeout Procedure				(978)
//#endif
	// end of [#2234]

// [#540] HWANG 2008.07.01
	// DIAGNOSTICS
	////////////////////////////////////////
	int		ClerkDiagnosticsDeviceHandler(int pOnOff);			// used to open/close VDM devices
	////////////////////////////////////////
	int		ClerkDiagonsticsMain();								// (821)
	int		ClerkDiagnosticsCardScan();							// (822)
	int		ClerkDiagnosticsMagtekCard();						// (856)
	int		ClerkDiagnosticsModem();							// (823)
	void	ClerkDiagnosticsSensor();							// (824)
	void	ClerkDiagnosticsCDUSensorView();					// (825)
	int		ClerkDiagnosticsSPRSensorView();					// (826)
	int		ClerkDiagnosticsAuxiliaryUnit();					// (827)
	int		ClerkDiagnosticsFlicker();							// (828)
	int		ClerkHaloLedNMCUFlickerProc();						// (888) [#2205] NH KSK 2013.06.28
	int		ClerkDiagnosticsCamera();							// (765) [#2518] US Kook 2017.12.21 Support MX-2800SE
	int		ClerkDiagnosticsBarcodeReader();					// (770) [#GLDV-3005] US Kook 2021.10.07 Support Side Car
	int		ClerkDiagnosticsBillAcceptor();						// (771) [#GLDV-3005] US Kook 2021.10.07 Support Side Car
	int		ClerkDiagnosticsExitScreen();						// (829)
#if (US_VERSION || CA_VERSION)	// [#2243] CA Justin 2013.12.17 Bug fix  on Canadian 4000W
	int		ClerkDiagnosticsWSPRSensorView();					// (876) [#2097] US PCS 2011.11.21 "SPR of WallMount Typed ATM"
	void	ClerkDiagnosticsWCDUSensorView();					// (877) [#2097] US PCS 2011.11.21 "SPR of WallMount Typed ATM"	
#endif
	int		ClerkDiagnosticsKeyMatrix();
	void	ClerkDiagnostics_LCDUSensorView();					// [#576] KSK 2009.11.06 L-CDU Image 추가 (871)
	void	ClerkDiagnostics_CDURSensorView();					// CDU Rear Image 추가
	void	ClerkDiagnostics_GBM10CHSensorView();				// (896) // [#2025] NH KJW 2011.02.28
	void	ClerkDiagnostics_TCPIPProc();						// [#585] KSK 2009.12.03 (872)
// end of [#540]

	void	ClerkOptionalFunction2Proc();						// Clerk Optional Function2 Procedure		(881)
	void	ClerkDeviceOptionProc();							// Clerk Device Option Procedure			(882)

#if (AU_A_VERSION)	// [#2459] AU KSK 2016.12.23
	void	ClerkDispenseMixOptionProc();						// Clerk Dispense Mix Option Procedure		(773)

	void	ClerkAlphiSetupProc();									// [#2494] AU Kook 2017.08.07 ATS Demo		(774)
	void	ClerkAlphiServerConfProc();								// [#2497] AU Kook 2017.09.05 ATS ALPHI		(775)
	void	ClerkAlphiIDConfProc();									// [#2497] AU Kook 2017.09.05 ATS ALPHI		(776)
	void	ClerkAlphiTranConfProc();								// [#2497] AU Kook 2017.09.05 ATS ALPHI		(777)
#endif				// end of [#2459]

	void	ClerkScheduledJournalUploadProc();					// Clerk ScheduledJournalUpload Procedure	(883)
	//----------------------------------------------------------------------------
// [#540] HWANG 2008.07.01
// DIAGNOSTICS
	BOOL	ClerkWaitScreenInput(BOOL bDIAGNOSTICS = FALSE, BOOL bKeyControl = TRUE);								// Clerk 화면을 표시, Update, Input을 기다린다.
	BOOL	ClerkWaitScreenInput_byCha(BOOL bDIAGNOSTICS = FALSE, BOOL bKeyControl = TRUE);	// Clerk 화면을 표시, Update, Input을 기다린다.
// end of [#540]

// [#160] NH KGS 2008.04.22 Password 변경 후 메뉴 Exit-> 패스워드 재입력하도록 변경
// [#236] NZ AIREAT 2008.05.27 TimeoutMode 삭제 (fnSCR_DisplayScreen에서 처리)
	int		ClerkScreenDisplay(int nScrNum, int nTimeout, BOOL bForce= FALSE, int nPinMode = 0);
	int		ClerkScreenUpdate(int nScrNum, BOOL bForce = FALSE);	// 현재 화면의 정보를 갱신한다.
	int		ClerkOpMsgUpdate(CString &OpMsg);					// APValue100으로 OP MSG를 출력한다.
	int		ClerkCheckDeviceStatus();							// Clerk 모드에서 장치의 상태를 check 한다.
	int		ClerkIsChangedCSTLoc();								// 카세트 장/탈착시 알람을 발생시킨다.

	CString	ClerkInformationMake(int nIndex);					// Clerk Information Make
	CString	ClerkEnableButton(int nIndex);						// Clerk ENABLE Button
	void	ClerkInitValue();
	
	BOOL ClerkInputAlpha(CString strTitleID, CString strValue, int nRow, int nCol, CString &strNewValue);
	BOOL ClerkConfirm(CString strGuideID, CString &strNewValue, BOOL bUseInputText = FALSE);				// [#2428] JUSTIN Add Option (UseInputText)
	BOOL ClerkInputNumber(CString strTitleID, CString strGuideID, CString strValue, CString strInputType, int MaxInChar, CString &strNewValue);
	//void ClerkNoticeToUser(CString strMessage, BOOL bWait=TRUE); // [#RWC6-10]
	void ClerkNoticeToUser(CString strMessage, BOOL bWait=TRUE, int nWaitTime=1); // [#RWC6-10]
	void ClerkStopToUser(CString strMessage);	// [#2063] NH KSK 2011.05.16 Journal Print시 Stop되지 않는 Bug Fix
	
	void	ClerkViewCamImages(	CString strTitle1, CString strImg1Path,
								CString strTitle2, CString strImg2Path);	// [#2518] US Kook 2018.01.15 Support Camera
	

	//----------------------------------------------------------------------------

	BOOL	StoreNetworkInformation();
	int		NetworkInformationHasChanged();

	void	GetWeatherInfo();
	void	GetNoticeInfo();
	void	SynchronizeKCV();									// [#2580] AU Kook 2019.08.06 Refactoring

	void	ClerkEnterNLeaveKeyMgr();							// [#2080] NH KSK 2011.06.25 Keymgr 진입 & Exit시 구현이 많아 함수화
	void	ClerkEnterDiagnostics();							// [#2078] NH KSK 2011.07.14 Diagnostics 진입시 구현이 많아 함수화

	// NVRAM Backup												
	BOOL	NVRAMBackUp();										// [#253] NZ AIREAT 2008.06.01
	// NVRAM Restore
	BOOL	NVRAMRestore();										// [#253] NZ AIREAT 2008.06.01

	void	ClerkDeviceSetupProc();								// (844)
	void	ClerkCDUSetupProc();								// (845)
	void	ClerkSystemControlProc();							// (846)
	void	ClerkCardReaderSetupProc();							// (741)	[#2073] NH KSK 2011.06.24
	void	ClerkAntiSkimmingSetupProc();						// (745)	[#2074] NH KSK 2011.07.09
	void	ClerkDiagAntiSkimmingTestProc();					// (746)	[#2078] NH KSK 2011.07.15
	void	ClerkHaloLEDSetupProc();							// (747)	[#2205] US KSK 2013.06.28
	void	ClerkCameraSetupProc();								// (764)	[#2518] US Kook 2017.12.22 Support MX-2800SE
	void	ClerkSideCarSetupProc();							// (772)	[#GLDV-3005] US Kook 2021.10.22 Support Side Car
	void	ClerkPowerControlProc();							// (766)	[#2536] NH Kook 2018.03.09 ATM Shutdown

	void	ClerkDigitalReceiptOptionProc();					// (793)	[#2263] CA Justin 2014.05.05 Digital Receipt Option - Canada 2700(BMO) ONLY
	int		ClerkDiagnosticsRFIDCardScan();						// (751)	[#2325] NH KSK 2015.01.28
	void	ClerkMoniMobileSetupProc();							// (757)	[#2382] US Justin 2015.12.16 MoniMobile Setup

	//
	// LibertyX
	//

	bool m_LTXAvailable;										// [#RWC6-59] US William 2019.11.11 LibertyX
	CLibertyXConfigurationManager* m_LXConfig;					// [#RWC6-59] US William 2019.10.06 LibertyX

	CString ClerkLibertyXScreenDataMake(int screenNumber);		// [#RWC6-59] US William 2019.10.04 LibertyX
	CString ClerkLibertyXGetGeneralOptions();					// [#RWC6-59] US William 2019.10.04 LibertyX
	CString ClerkLibertyXGetHostOptions();						// [#RWC6-59] US William 2019.10.06 LibertyX
	CString ClerkLibertyXGetApiOptions();						// [#RWC6-59] US William 2019.10.06 LibertyX

	CString ClerkLibertyXScreenEnableButton(int screenNumber);
	CString	ClerkLibertyXGetGeneralOptionButtons();

	void	ClerkLibertyXOptions();								// (1010)	[#RWC6-59] US William 2019.10.03 LibertyX
	void	ClerkLibertyXHostOptions();							// (1011)	[#RWC6-59] US William 2019.10.03 LibertyX
	void	ClerkLibertyXAPIOptions();							// (1012)	[#RWC6-59] US William 2019.10.03 LibertyX

	//
	// DigitalMint
	//
	bool m_DMAvailable;

	//
	// Cash Depot
	//
	bool m_CDAvailable;

	bool m_TangPayAvailable;	// [RWC6-676] SKKim 2024.03.28

	/* Disk Management */
	
	CString	ClerkDiskManagementScreenDataMake(int screenNumber);

	CString ClerkGetDiskManagementData();
	void	ClerkRunCleanupOperation(int operationFlags, int &count);
	void	ClerkDiskManagementMainProc();						// (1000)

	/* End Disk Managment */

#ifndef NO_REMOTE_UPDATE
	/* Remote Updates [#RWC6-121] US William 2020.01.27 Remote Updates */

	int m_currentRemotePage;
	int m_maxRemotePages;

	CString ClerkRemoteUpdatesScreenDataMake(int screenNumber);
	CString ClerkRemoteUpdatesGetGeneralOptions();
	CString ClerkRemoteUpdatesGetAdHocOptions();

	CString ClerkRemoteUpdatesEnableButton(int screenNumber);
	CString ClerkRemoteUpdatesGeneralButtons();
	CString ClerkRemoteUpdatesAdHocEnableButtons();

	void ClerkRemoteUpdatesOptions();							// (1013)
	void ClerkRemoteUpdatesAdHoc();								// (1014)

	bool HasNextPage();
	bool HasPreviousPage();
	int GetCountForPageIndex(int pageIndex);
	int GetCacheIndexForPageIndex(int page, int position);
	bool PrimeUpdateCache(bool force = false);
	bool StartSWUpdateWithCachePackageIndex(int i);
	bool DoRemoteSoftwareUpdate();

	/* End Remote Updates */
#endif

	/* Just.Cash */
	CString ClerkJustCashScreenDataMake(int screenNumber);
	CString ClerkJustCashGetGeneralOptions();
	CString ClerkJustCashEnableButtons(int screenNumber);
	CString ClerkJustCashGetGeneralOptionButtons();

	void ClerkJustCashGeneralOptions();
	bool ClerkJustCashIsSidecarModeAvailable();

	/* DigitalMint */
	CString ClerkDigitalMintScreenDataMake(int screenNumber);
	CString ClerkDigitalMintGetGeneralOptions();
	CString ClerkDigitalMintTrimValue(CString value);
	CString ClerkDigitalMintEnableButtons(int screenNumber);
	CString ClerkDigitalMintGetGeneralOptionButtons();

	void		ClerkDigitalMintGeneralOptions();
	BIZ_RETURN	ClerkDigitalMintBarcodeReader(CString &code);
	BIZ_RETURN	ClerkDigitalMintParseScan(CString &retData, CString rawData);
	bool		ClerkDigitalMintIsSidecarModeAvailable();

	// JCTODO: Combine Just.Cash and DigitalMint checks once BCR issues are fixed
	/* Just.Cash + DigitalMint */
	// bool ClerkIsSidecarModeAvailable();

	/* Cash Depot */
	CString ClerkCashDepotScreenDataMake(int screenNumber);
	CString ClerkCashDepotGetGeneralOptions();
	CString ClerkCashDepotEnableButtons(int screenNumber);
	CString ClerkCashDepotGetGeneralOptionButtons();

#if (APP_TANGOPAY)	// [RWC6-676] Start SKKim 2024.03.29
	void ClerkTangoPaySettings();
#endif				// [RWC6-676] End SKKim 2024.03.29

	#if (MX_VERSION)
	void	ClerkMXPrintOptionProc();							// (814)	[#2115] MX KSK 2012.02.05
	#endif

	// [#2472] US Justin 2017.02.08 DCC Withdrawal Option
	#if (APP_DCC_WITHOPTION)	
	void	ClerkDCCWithdrawalOptionProc();						// (752)
	#endif
	// End of [#2474]

	// [iTM] US AIREAT 2009.12.02 : OP Wizard
	//OPERATION_RESULT ProcDayTotal();			//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
	OPERATION_RESULT ProcDayTotal(BOOL bReal);	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
	OPERATION_RESULT ProcCassetteTotal();
	OPERATION_RESULT ProcAddCash(CStringArray &arAddCashString, int nActUser);
	OPERATION_RESULT ProcCashInTotal_Demo(BOOL bClear = FALSE);
	// end of [iTM] OP Wizard

	OPERATION_RESULT ProcOPTestDispensing();	// [#2504] US Justin 2017.09.11 Test Dispensing on ADD CASH SCREEN

	// [#419] [NH] KSK 2008.9.23
	/* Convert Error Summary*/
	CString	ConvertErrSummaryDataToPrint();							// Convert Error Summary Data To Print
	CString	ConvertErrSummaryDataToDisplay(int nPage = 0);			// Convert Error Summary Data To Print
	// end of [#419]

	CString	ConvertOptOutSummaryDataToPrint();						// [#2540] NH Justin 2018.03.13 Surcharge and DCC Acceptance Report

	// [iTM] KSK 2010.02.08 IP Check 함수 추가
	int		CheckValidate_IPValue(CString strIP);
	// end of [iTM]

	//BOOL	ConnectHost();	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
	BOOL	ConnectHost(int nConfigOption = 0);	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 

// [#540] HWANG 2008.07.01
	CDiagCard*	m_pDiagMcu;
	CDiagCdu*	m_pDiagCdu;
	CDiagSpr*	m_pDiagSpr;
	CDiagRfid*	m_pDiagRfid;			// [#2325] NH KSK 2015.01.25
	CDiagBcr*	m_pDiagBcr;				// [#GLDV-3005] US Kook 2021.10.18 Support Side Car
	CDiagBna*	m_pDiagBna;				// [#GLDV-3005] US Kook 2021.10.18 Support Side Car
	BOOL		m_bDiagMCUOpen;
	BOOL		m_bDiagSPROpen;
	BOOL		m_bDiagCDUOpen;
	BOOL		m_bDiagRFIDOpen;		// [#2325] NH KSK 2015.01.25	
	BOOL		m_bDiagBCROpen;			// [#GLDV-3005] US Kook 2021.10.18 Support Side Car
	BOOL		m_bDiagBNAOpen;			// [#GLDV-3005] US Kook 2021.10.18 Support Side Car
// end of [#540]

#ifdef UNDER_CE
	CKeyMgr		m_KeyManagementProc;	// [#541] AIREAT 2009.07.09 KEY MGR 통합.
#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:  // control bar embedded members
//	CCeCommandBar	m_wndCommandBar;
	
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//-------------------------------------------------------------------------
	afx_msg void OnSafeDoorChanged(LPCTSTR status);							// [#RWC6-14] US William 2019.09.18 Add Safe Door Status To Journal
	afx_msg void OnDisplayData(LPCTSTR szDataName, LPCTSTR szDataValue);	// AXINAX
	afx_msg void OnDisplayPinpadData(LPCTSTR szDataValue);					// AXINAX 2004.04.07

	DECLARE_EVENTSINK_MAP()													// AXINAX
	//-------------------------------------------------------------------------
};

//AtmDoing Thread
//UINT AtmDoingThread(LPVOID pParam);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__CE7435D0_0A78_4B12_ACCB_590B6C846944__INCLUDED_)
