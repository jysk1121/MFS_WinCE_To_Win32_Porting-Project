#ifndef __SP_INFORM__H
#define __SP_INFORM__H

#pragma pack(1)

typedef struct NVRAM_SP_ERROR
{
	char ErrCode_PIN[8];
	char ErrCode_SPR[8];
	char ErrCode_IDC[8];
	char ErrCode_CDU[8];
	char ErrCode_SIU[8];
	char ErrCode_TTU[8];
	char ErrCode_JPR[8];
	char ErrCode_EDU[8];
	char ErrCode_CAM[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
//	char ErrCode_CDU[8];
} SP_Error, *LPSPError;

typedef struct NVRAM_SP_MDATA
{
	char MData_SPR[128];
	char MData_IDC[128];
	char MData_CDU[128];
	char MData_JRP[128];
	char MData_EDU[128];
//	char MData_CDU[128];
} SP_MData, *LPSP_MData;

typedef struct NVRAM_SP_VERSION
{
	char Version_PIN[16];
	char Version_SPR[16];
	char Version_IDC[16];
	char Version_CDU[16];
	char Version_SIU[16];
	char Version_TTU[16];
	char Version_JPR[16];
	char ErrCode_EDU[16];
	char ErrCode_CAM[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
} SP_Version, *LPSP_Version;

typedef struct NVRAM_EP_VERSION
{
	char Version_PIN[16];
	char Version_SPR[16];
	char Version_IDC[16];
	char Version_CDU[16];
	char Version_SIU[16];
	char Version_TTU[16];
	char Version_JPR[16];
	char ErrCode_EDU[16];
	char ErrCode_CAM[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
//	char ErrCode_CDU[16];
} EP_Version, *LPEP_Version;

typedef struct NVRAM_SP_EXTEND
{
	char ErrCode[8];
	char SPVersion[16];
	char EPVersion[16];
} SP_DataEx, *LPSP_DataEx;

typedef struct SP_InfoEx
{
	SP_DataEx	RFD;
	SP_DataEx	BCR;
	SP_DataEx	BNA;
	SP_DataEx	COIN;
} SP_InfoEx, *LPSP_InfoEx;

typedef struct NVRAM_SP
{
	BYTE		byDummy[2];
	char		szCheck[8];
	SP_Error	SPErr;
	SP_MData	SPMData;
	SP_Version	SPVersion;
	EP_Version	EPVersion;
	SP_InfoEx	SPInfo;
} SPInform, *LPSPInform;

typedef struct _bs_cdm_iniinfo
{
	USHORT	usIndex;
	char	szPhysicalPositionName[8];// Name indicating the physical location
	USHORT	usType;						// Type of the cash unit
	char	cUnitID[5];					// Cash Unit ID
	char	cCurrencyID[3];				// Currency ID(ISO format) of cash unit
	USHORT	ulInitialCount;
	SHORT	sExponent;					// Currency exponent
	USHORT	ulValues;					// The value of a single coin or bill in the cash unit
	SHORT	ulCount;					// Actual count of coins/bills
	USHORT	ulRejectCount;				// Actual reject count of coins/bills
	USHORT	ulMaximum;
	USHORT	usPStatus;					// The state of the physical cash unit
	USHORT	ulLastDispCount;			// Counts of notes to be required from cassettes
	USHORT	ulUsrCount;					// Counts of notes supplied to user
	USHORT	ulRejCount;					// Counts of notes rejected
	USHORT	ulCstOutCount;				// Counts of notes supplied from cassettes
	USHORT	ulSkewNote;
	USHORT	ulGapNote;
	USHORT	ulLongNote;
	USHORT	ulShortNote;
	USHORT	ulDoubleNote;
	USHORT	ulTotRejNote;
} BSCDMINIINFO, *LPBSCDMINIINFO;

// 해당 거래 정보 저장. 2009.03.27
typedef struct _cdm_lastdispinfo
{
	USHORT	ulUsrCount;					// SP 해당 거래의 Counts of notes supplied to user
	USHORT	ulRejCount;					// SP 해당 거래의 Counts of notes rejected
	USHORT	ulCstOutCount;				// SP 해당 거래의 Counts of notes supplied from cassettes
	USHORT	ulSkewNote;					// EP SKEW   COUNT
	USHORT	ulGapNote;					// EP GAP    COUNT
	USHORT	ulLongNote;					// EP LONG   COUNT
	USHORT	ulShortNote;				// EP SHORT  COUNT
	USHORT	ulDoubleNote;				// EP DOUBLE COUNT
	USHORT	ulTotRejNote;				// EP REJECT COUNT
//	USHORT	usReserve1;
	USHORT	usCstPositionWarning;		// 카세트 정위치 이탈에 대한 Warning 표시 정보, V06.06.10(V05.06.10) 2013.10.31 LEH
	USHORT	usReserve2;					// 예비2
	USHORT	usReserve3;					// 예비3
}LASTDISPINFO, *LPLASTDISPINFO;

// [#GLDV-3005] US Kook 2022.01.05 Side Car (NVRAM structure for CIM)
#define MAX_NOTE_FOR_CST	10

////////////////////////////////////////////
// PCU 정보 저장
typedef struct _cst_remain_count
{
	char	szCurrencyID[4];
	DWORD	dwNoteValue;
	DWORD	ulCount;
} RCOUNT;

typedef struct _cst_noteinfo
{
	DWORD	ulCount;
	DWORD	ulCashInCount;
	DWORD	dwNumOfNote;
	RCOUNT	NoteCount[MAX_NOTE_FOR_CST];
} NVRAMPCU;
// end of [#GLDV-3005]

typedef struct _bs_ini_cashunit
{
	///////////////////////
	// CDU INFO [0~1023] //
	WORD			wNVRamCheck[5];				// (10 bytes)
	USHORT			usCUNum;					// (2 bytes) Number of PCU
	USHORT			ulRejTotCount;				// (2 bytes)
	BSCDMINIINFO	CashUnit[6];				// (324 bytes, 54*6)
	BOOL			bENQSend;					// (4 bytes) if Dispense ENQ Send then TRUE  
	BOOL			bCDULogBackup;				// (4 bytes) if SP need Log Backup then TRUE
	BOOL			bSetDenomination;			// (4 bytes) CDU Setup 기능 관련 추가 2008.10.14
	BYTE			bySensorStatus[13];			// (13 bytes) Sensor값 기록 추가. 2009.03.02
	LASTDISPINFO	LastDispInfo[6];			// (144 bytes, 24*6) 해당 거래 정보 저장. 2009.03.27
	BYTE			bySystemInfo[33];			// (33 bytes) @ADD: [V06.06.15(V05.06.15)] 2015.12.15 LEH - CE부 DATA
	BOOL			bC3AuthDone;				// (4 bytes) @ADD: [V06.06.15(V05.06.15)] 2015.12.15 LEH - CE부 인증 성공 여부
	BOOL			bSupportAR;					// (4 bytes) [#RWC6-82] NH Kook 2021.12.27 Support CDU Auto Recovery (Jam Clear)
		// [#GLDV-3005] US Kook 2022.01.05 Side Car
	char			szReservedForCDU[476];		// (476 bytes, 1024-548)

	//////////////////////////
	// CIM INFO [1024~2047] //
	char			szCurrencyID[4];			// (4 bytes) 국가코드
	char			szBarcodeData[64];			// (64 bytes) 바코드 정보
	
	DWORD			dwNumOfPCU;					// (4 bytes) 카세트 정보, current 1
	NVRAMPCU		PCUInfo[1];					// (132 bytes) 카세트 정보
	char			szReservedForCIM[820];		// (820 bytes, 1024-204)

	//////////////////////////
	// RESERVED [2048-4095] //
	char			szReservedForCOD[2048];		// (2048 bytes)
	// end of [#GLDV-3005]
}BSINICASHUNIT, *LPBSINICASHUNIT;


typedef struct NVRAM_KEY_INFORM
{
	char szReserv[9];
	BYTE KeyMode;
	BYTE KeyIndex;
	char KeyCheck[6];
	BYTE MacIndex;
	char MacCheck[6];
	BYTE PINFormat;			//Format (ISO0/DIEBOLD 구분)
	BYTE PartNumber;		//Part Key Number (0=2개, 1=3개)
	BYTE PadChar;			//Pad data
	BYTE CSUMType;			//Key Check sum 생성 방식 구분
	BYTE InjectionMethod;	//Injection type
	char KeyCheckSum0[6];
	char KeyCheckSum1[6];
	char KeyCheckSum2[6];
	char KeyCheckSum3[6];
	char KeyCheckSum4[6];
	char KeyCheckSum5[6];
	char KeyCheckSum6[6];
	char KeyCheckSum7[6];
	char KeyCheckSum8[6];
	char KeyCheckSum9[6];
	char KeyCheckSum10[6];
	char KeyCheckSum11[6];
	char KeyCheckSum12[6];
	char KeyCheckSum13[6];
	char KeyCheckSum14[6];
	char KeyCheckSum15[6];
	char SerialNumberA[11];
	char SerialNumberB[11];
	BYTE byImportFlag;		// [#554] KSK 2009.08.10 512K 추가
	char szKeyName[132];
	char szPINKeyName[132];
	char szPINKeyName3[132];
	char szMACKeyName[132];
	char szMACKeyName3[132];
	char szDATAKeyName[132];
	char szDATAKeyName3[132];
	char szSingleKeyName[132];
	char szTripleKeyName[132];	// end of [#554]
} KeyInform, *LPKeyInform;

#pragma pack()

#endif		//__SP_INFORM__H