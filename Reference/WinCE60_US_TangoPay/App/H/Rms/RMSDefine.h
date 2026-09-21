#ifndef __RMS_RETURN_CODE_H__
#define __RMS_RETURN_CODE_H__

#include ".\Common\NHDbgApi.h"
#include ".\Dev\LoginManager.h"
#include ".\Common\LibertyXConfigurationManager.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define RMS_CODEPAGE		1252		// Latin I Code Page

#define RMS_SETUP_DATA_FRAME_SIZE		512

/////////////////////////////////////////////////////
// Line Type
#define RMS_LINE_NONE	0
#define RMS_LINE_DIALUP	1
#define RMS_LINE_TCP	2

#define RMS_TLS_DEFAULT_PORT_NUM 9998

// ActType
#define RMS_LISTEN		1			// LISTEN - rms enable     ATM <- RMS		// RMSFlowCtrl.h에 중복 정의 되어 있음.
#define RMS_CONNECT		2			// CONNECT - status Send   ATM -> RMS		// RMSFlowCtrl.h에 중복 정의 되어 있음.

/////////////////////////////////////////////////////
//	Return Code
#define	RMS_OK			0			// 요청한 기능이 정상 완료 되었다.			// RMSFlowCtrl.h에 중복 정의 되어 있음.
#define RMS_ERR			1			// 요청한 기능이 에러가 발생하였다.			// RMSFlowCtrl.h에 중복 정의 되어 있음.

typedef DWORD	NUINT32;
typedef WORD	NUINT16;
typedef BYTE	NUINT8;

/////////////////////////////////////////////////////
//	Data Structure

//////////////////////////////////////////
//	RMS MESSAGE DEFINE
//////////////////////////////////////////

#define	RMS_REQ_SETUP			'A'
#define RMS_SET_SETUP			'B'
#define RMS_REQ_REBOOT_FORCE	'C'
#define RMS_REQ_X_JNL			'D'
#define RMS_REQ_FILE_DOWN		'E'
#define RMS_REQ_DATE_JNL		'F'				// [#524] US KSK 2009.05.19
#define RMS_SET_GO				'G'
#define	RMS_REQ_STATUS			'H'
#define RMS_REQ_INITALLDEV		'I'
#define RMS_REQ_INITERRDEV		'J'
#define RMS_EXEC_CSTTOTAL		'K'				// [#531] NH KSK 2009.06.03
#define RMS_SET_ADDCASH			'L'				// [#530] NH KSK 2009.06.03

#define RMS_REQ_REBBOT_NONTX	'N'
#define RMS_REQ_JNL				'O'
#define RMS_REQ_FILE_UPLOAD		'P'				// [#523] US KSK 2009.05.19
#define	RMS_REQ_ERR_TOTAL		'Q'				// [#424] [NH] KSK 2008.9.18
#define	RMS_REQ_ERR_TOTALNCLR	'R'				// [#424] [NH] KSK 2008.9.18

#define RMS_EXEC_DAYTOTAL		'X'				// [#532] NH KSK 2009.06.03
#define RMS_EXEC_TRIALDAYTOTAL	'Y'				// [#532] NH KSK 2009.06.03
#define RMS_REQ_REJECTTOTAL		'Z'				// [#533] NH KSK 2009.06.03



#define RMS_REQ_REJECTTOTALNCLR			'a'		// [#533] NH KSK 2009.06.03
#define RMS_SET_CHANGE_PASSWORD			'b'		// [#309] NH AIREAT 2008.06.17 - Change Password CMD 추가.
#define RMS_SET_DATETIME				'c'		// [#611] SOOK 2010.01.16 호주 CHANGE_ATM TIME 추가 
#define RMS_SET_CHANGE_EXCHANGE_RATE	'd'		// [#514] [MX] KSK 2009.3.11
#define RMS_SET_BINLIST					'e'		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#define RMS_REQ_BINLIST					'f'		// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
//#define RMS_REQ_NEWSTICKER			'g'		// [#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 KSK 2010.06.10 Setup정보로 변경
//#define RMS_SET_NEWSTICKER			'h'		// [#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 KSK 2010.06.10 Setup정보로 변경
#define RMS_CHANGE_RETAIL_PASSWORDS		'g'		// [#2276] US Justin 2014.06.09 Add Changing all 4 passwords	
#define RMS_SET_ATM_STATUS				'h'		// [#2276] US Justin 2014.06.10 Support in/Out of service 
#define RMS_REQ_JNL_IMAGE				'i'		// [#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 
#define RMS_SET_WEATHERE_INFO			'j'		// [#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 
#define RMS_SCHEDULED_JOURNAL_UPLOAD	'k'		// [#613] AU_C KSK 2010.01.21
#define RMS_ENHANCED_COUPON_DOWNLOAD	'l'		// [#620] US KSK 2010.02.22
#define RMS_ENHANCED_COUPON_UPLOAD		'm'		// [#620] US KSK 2010.02.22
#define RMS_SET_HOT_KEY					'o'		// [#2283] NH Justin Customized Hot Key
#define RMS_SET_EMV_AID					'p'		// [#2342] NH Justin 2015.05.11 AID Selection
#define RMS_RESET_PASSWORD_HASHED		'q'		// [#RWC6-56] US William Hashed Password
#define RMS_INSTALL_REMOTE_UPDATE		'r'		// [#RWC6-134] US William 2020.03.24 RMS Remote Updates
#define RMS_CHANGE_LTX_PASSWORD			's'		// [#RWC6-208] US William 2020.08.05 Add LTX change password job
#define RMS_RESET_MCCP_SYMMETRICAL_KEY	't'		// [#RWC6-533] US ryan.payton 2023.05.30 [PAI] RMS MCCP Key Reset
#define RMS_JUSTCASH_REGISTRATION		'z'		// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes

#define RMS_REQ_OP_TEST					0x01	// [#2507] US Justin 2017.09.26 Add Test Button on RMS Send Screen

#define BIN_MAXIMUN_COUNT 8000	//[#546] BIN MAX COUNT 8000개로 변경 

#pragma pack(1)

// COMMON HEADER
typedef struct _RMS_COMM_
{
	NUINT16	Length;
	NUINT8	TableIndex;
	NUINT8	MsgID;	

} RMS_COMM, *PRMS_COMM;

// RMS COMMAND COMM
typedef struct _RMS_CMD_COMM_
{
	RMS_COMM	Header;				//  4 byte

	NUINT16	FrameNumber;			//	6.
	NUINT8	TerminalNumber[16];		// 22.
	NUINT8	RMSPassword[7];			// 29.
//	NUINT8	dummy[3];				// 32. 
} RMS_CMD_COMM, *PRMS_CMD_COMM;

// sizeof(RMS_CMD_COMM) - Length
#define SIZE_RMS_CMD_COMM	(sizeof(RMS_CMD_COMM) - 2)

// JOURNAL BY DATE
typedef struct _RMS_CMD_JNL_DATE_
{
	RMS_COMM	Header;				//  4 byte

	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT16	Year;					//  2 byte
	NUINT16	Month;					//	2 byte
	NUINT16	Day;					//	2 byte

	NUINT8	RMSVersion;				//	1 byte

} RMS_CMD_JNL_DATE, *PRMS_CMD_JNL_DATE;

// LAST X JOURNAL
typedef struct _RMS_CMD_LAST_X_JNL_
{
	RMS_COMM	Header;				//  4 byte

	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT16	JournalCount;			//  2 byte

	NUINT8	RMSVersion;				//	1 byte

} RMS_CMD_LAST_X_JNR, *PRMS_CMD_LAST_X_JNR;

// [#524] US KSK 2009.05.19
// DATE BY JOURNAL
typedef struct _RMS_CMD_DATE_JNL_
{
	RMS_COMM	Header;				//  4 byte
	
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1
	
	NUINT16	wYear;					//  2 byte
	NUINT16	wMonth;					//  2 byte
	NUINT16	wDate;					//  2 byte
	
	NUINT8	RMSVersion;				//	1 byte
	
} RMS_CMD_DATE_JNR, *PRMS_CMD_DATE_JNR;
// end of [#524]

// ADD CASH [#530] NH KSK 2009.06.03
typedef struct _RMS_CMD_ADD_CASH_
{
	RMS_COMM	Header;				//  4 byte

	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT16	CST1cnt;				//  2 byte
	NUINT16	CST2cnt;				//  2 byte
	NUINT16	CST3cnt;				//  2 byte
	NUINT16	CST4cnt;				//  2 byte

	NUINT8	RMSVersion;				//	1 byte

} RMS_CMD_ADD_CASH, *PRMS_CMD_ADD_CASH;
// end of [#530]

// File Download CMD
typedef struct _RMS_CMD_FILE_DOWN_
{
	RMS_COMM	Header;
	
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT8	FileType;
	NUINT8	Dir[286];

} RMS_CMD_FILE_DOWN, *PRMS_CMD_FILE_DOWN;
// sizeof(RMS_CMD_FILE_DOWN)-Length
//#define LEN_RMS_CMD_FILE_DOWN	(sizeof(RMS_CMD_FILE_DOWN)-2)

// [#523] US KSK 2009.05.19
// File Upload CMD
typedef struct _RMS_CMD_FILE_UPLOAD_
{
	RMS_COMM	Header;
	
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte

	NUINT8	UploadDir[256];
	
} RMS_CMD_FILE_UPLOAD, *PRMS_CMD_FILE_UPLOAD;
// sizeof(RMS_CMD_FILE_DOWN)-Length

// [#611] AU SOOK 2010.01.16
// Change ATM Date TIme
typedef struct _RMS_CMD_ATM_DATE_TIME_
{
	RMS_COMM	Header;
	
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte

	NUINT8	DateTime[15];			//YYYYMMDDHHmmSS
	
} RMS_CMD_ATM_DATE_TIME, *PRMS_CMD_ATM_DATE_TIME;

//end of [#611]

typedef struct _RMS_FILE_UPLOAD_DATA_
{
	RMS_COMM	Header;				//   4 byte

	NUINT16		FrameNumber;		//	 6 byte
	
	NUINT16		dwYear;				//   8 byte
	NUINT16		dwMonth;			//  10 byte
	NUINT16		dwDay;				//  12 byte
	NUINT16		dwHour;				//  14 byte
	NUINT16		dwMinute;			//  16 byte
	NUINT16		dwSecond;			//  18 byte
	NUINT8		MachineType;		//	19 byte comm1 header
	
	NUINT8		SerialNumber[11];	//  30 byte
	
	NUINT16		CST1Denomination;	//	32 byte
	NUINT16		CST1BillCount;		//	34 byte
	NUINT16		CST2Denomination;	//	36 byte
	NUINT16		CST2BillCount;		//	38 byte
	NUINT16		CST3Denomination;	//	40 byte
	NUINT16		CST3BillCount;		//	42 byte
	NUINT16		CST4Denomination;	//	44 byte
	NUINT16		CST4BillCount;		//	46 byte
	
	NUINT16		JournalCount;		//  48 byte
	NUINT8		ATMStatus;			//  49 byte
	
	NUINT8		ErrorCode[8];		//  57 byte

	NUINT16		UploadDataSize;		//  59 byte
	NUINT8		FileData[2048];		// 2104 byte
	
} RMS_FILE_UPLOAD_DATA, *PRMS_FILE_UPLOAD_DATA;
// end of [#523]

// [#309] NH AIREAT 2008.06.17 - Change Password CMD 추가.
// Change Password CMD
typedef struct _RMS_CMD_CHANGE_PW_
{
	RMS_COMM	Header;				//  4 byte

	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT8	MasterPW[6];			//  2 byte
	NUINT8	ManagerPW[6];			//  2 byte
	NUINT8	ServicePW[6];			//  2 byte

	NUINT8	RMSVersion;				//	1 byte

} RMS_CMD_CHANGE_PASSWORD, *PRMS_CMD_CHANGE_PASSWORD;
// end of [#309]

// [#2276] NH Justin 2014.06.10 Change Retail Passwords.
// Change Retail Password CMD
typedef struct _RMS_CMD_CHANGE_RETAIL_PW_
{
	RMS_COMM	Header;				//  4 byte
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT8	MasterPW[6];			//  2 byte
	NUINT8	ServicePW[6];			//  2 byte
	NUINT8	OperatorPW[6];			//  2 byte
	NUINT8	RMSPW[6];				//  2 byte

} RMS_CMD_CHANGE_RETAIL_PASSWORD, *PRMS_CMD_CHANGE_RETAIL_PASSWORD;
// end of [#2276]

// [#2277] NH Justin 2014.06.10 ATM Status
// ATM Status
typedef struct _RMS_CMD_SET_ATM_STATUS_
{
	RMS_COMM	Header;				//  4 byte
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT8	ATMStatus;				// 1 Byte

} RMS_CMD_SET_ATM_STATUS, *PRMS_CMD_SET_ATM_STATUS;
// end of [#2277]

// [#2283] NH Justin Customized Hot Key
typedef struct _RMS_CMD_SET_HOT_KEY_
{
	RMS_COMM	Header;				//  4 byte
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1

	NUINT8	HotKeyString[42];		// 42 Byte

} RMS_CMD_SET_HOT_KEY, *PRMS_CMD_SET_HOT_KEY;
// end of [#2283]

// [#2342] NH Justin 2015.05.11 AID SELECTION
typedef struct _RMS_CMD_SET_EMV_AID_
{
	RMS_COMM	Header;				//  4 byte
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1
} RMS_CMD_SET_EMV_AID, *PRMS_CMD_SET_EMV_AID;
// end of [#2342]

// [#424] [NH] KSK 2008.9.18
typedef	struct _RMS_ERR_SUM {
	NUINT8		ErrCode[8];
	NUINT16		ErrCount;	
}RMS_ERR_SUM;

typedef	struct _RMS_CMD_ERROR_TOTAL {
	
	RMS_COMM	Header;				//   4 byte

	NUINT16		FrameNumber;		//	 6 byte

	NUINT16		dwYear;				//   8 byte
	NUINT16		dwMonth;			//  10 byte
	NUINT16		dwDay;				//  12 byte
	NUINT16		dwHour;				//  14 byte
	NUINT16		dwMinute;			//  16 byte
	NUINT16		dwSecond;			//  18 byte
	NUINT8		MachineType;		//	19 byte comm1 header

	NUINT8		SerialNumber[11];	//  30 byte

	NUINT16		CST1Denomination;	//	32 byte
	NUINT16		CST1BillCount;		//	34 byte
	NUINT16		CST2Denomination;	//	36 byte
	NUINT16		CST2BillCount;		//	38 byte
	NUINT16		CST3Denomination;	//	40 byte
	NUINT16		CST3BillCount;		//	42 byte
	NUINT16		CST4Denomination;	//	44 byte
	NUINT16		CST4BillCount;		//	46 byte

	NUINT16		JournalCount;		//  48 byte
	NUINT8		ATMStatus;			//  49 byte

	NUINT8		ErrorCode[8];		//  57 byte

	NUINT16		StartDateTime[6];	//	69 byte	Clear한 시간
	NUINT16		EndDateTime[6];		//	81 byte	ATM 현재 시간

	NUINT8		ErrorCodeCnt;		//	82 byte
	RMS_ERR_SUM	ErrSum[50];
}RMS_CMD_ERROR_TOTAL, *PRMS_CMD_ERROR_TOTAL;
// end of [#424]

// [#533] NH KSK 2009
typedef	struct _RMS_CMD_REJECT_TOTAL {
	
	RMS_COMM	Header;				//   4 byte
	
	NUINT16		FrameNumber;		//	 6 byte
	
	NUINT16		dwYear;				//   8 byte
	NUINT16		dwMonth;			//  10 byte
	NUINT16		dwDay;				//  12 byte
	NUINT16		dwHour;				//  14 byte
	NUINT16		dwMinute;			//  16 byte
	NUINT16		dwSecond;			//  18 byte
	NUINT8		MachineType;		//	19 byte comm1 header
	
	NUINT8		SerialNumber[11];	//  30 byte
	
	NUINT16		CST1Denomination;	//	32 byte
	NUINT16		CST1BillCount;		//	34 byte
	NUINT16		CST2Denomination;	//	38 byte
	NUINT16		CST2BillCount;		//	40 byte
	NUINT16		CST3Denomination;	//	44 byte
	NUINT16		CST3BillCount;		//	46 byte
	NUINT16		CST4Denomination;	//	50 byte
	NUINT16		CST4BillCount;		//	52 byte
	
	NUINT16		JournalCount;		//  56 byte
	NUINT8		ATMStatus;			//  57 byte
	
	NUINT8		ErrorCode[8];		//  65 byte
	
	NUINT16		StartDateTime[6];	//	77 byte	Clear한 시간
	NUINT16		EndDateTime[6];		//	89 byte	ATM 현재 시간

	NUINT32		TotalDispensedCount;	// Total Dispensed Count
	NUINT32		CSTDispensedCount[4];	// Cassette별 Dispensed Count
	NUINT32		TotalRejectCount;		// Total Reject Count

	NUINT16		CST1SkewCount;
	NUINT16		CST1GapCount;
	NUINT16		CST1LongCount;
	NUINT16		CST1ShortCount;
	NUINT16		CST1DoubleCount;
	NUINT16		CST2SkewCount;
	NUINT16		CST2GapCount;
	NUINT16		CST2LongCount;
	NUINT16		CST2ShortCount;
	NUINT16		CST2DoubleCount;
	NUINT16		CST3SkewCount;
	NUINT16		CST3GapCount;
	NUINT16		CST3LongCount;
	NUINT16		CST3ShortCount;
	NUINT16		CST3DoubleCount;
	NUINT16		CST4SkewCount;
	NUINT16		CST4GapCount;
	NUINT16		CST4LongCount;
	NUINT16		CST4ShortCount;
	NUINT16		CST4DoubleCount;

}RMS_CMD_REJECT_TOTAL, *PRMS_CMD_REJECT_TOTAL;
// end of [#533]

// [#514] [MX] KSK 2009.3.11
// change exchange rate
typedef struct _RMS_CMD_CHANGE_EXRATE_
{
	RMS_COMM	Header;				//  4 byte
	
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte ;; +1
	
	NUINT8	ExchangeRate[12];		// 12 byte
	
	NUINT8	RMSVersion;				//	1 byte
	
} RMS_CMD_CHANGE_EXCHANGERATE, *PRMS_CMD_CHANGE_EXCHANGERATE;
// end of [#514]

// CST Status
typedef struct _CST_STATUS_
{
	NUINT16		CST1Denomination;
	NUINT16		CST1BillCount;
	NUINT16		CST1Status;
	NUINT16		CST2Denomination;
	NUINT16		CST2BillCount;
	NUINT16		CST2Status;
	NUINT16		CST3Denomination;
	NUINT16		CST3BillCount;
	NUINT16		CST3Status;
	NUINT16		CST4Denomination;
	NUINT16		CST4BillCount;
	NUINT16		CST4Status;
	NUINT16		dwCDUCSTstatus;			//save CST status for "Device Status" field

} CST_STATUS, *PCST_STATUS;

// ATM Status Format.
typedef struct _RMS_RES_STATUS_
{
	RMS_COMM	Header;				//   4. byte

	NUINT16		FrameNumber;		//   6. byte
	NUINT16		dwYear;				//   8. byte
	NUINT16		dwMonth;			//  10. byte
	NUINT16		dwDay;				//  12. byte
	NUINT16		dwHour;				//  14. byte
	NUINT16		dwMinute;			//  16. byte
	NUINT16		dwSecond;			//  18. byte

	NUINT8		MachineType;		//  19. byte
	NUINT8		SerialNumber[11];	//  30. byte

	NUINT8		RMSPassword[7];		//  37. byte
	NUINT8		TerminalNumber[16];	//  53. byte
	NUINT8		RMSTel1[20];		//  73. byte
	NUINT8		RMSTel2[20];		//  93. byte
	
	NUINT16		CST1Denomination;	//  95. byte
	NUINT16		CST1BillCount;		//  97. byte
	NUINT16		CST1Status;			//  99. byte
	NUINT16		CST2Denomination;	// 101. byte	
	NUINT16		CST2BillCount;		// 103. byte
	NUINT16		CST2Status;			// 105. byte
	NUINT16		CST3Denomination;	// 107. byte
	NUINT16		CST3BillCount;		// 109. byte
	NUINT16		CST3Status;			// 111. byte
	NUINT16		CST4Denomination;	// 113. byte
	NUINT16		CST4BillCount;		// 115. byte
	NUINT16		CST4Status;			// 117. byte

	NUINT16		JournalCount;		// 119. byte
	NUINT8		ATMStatus;			// 120. byte

	NUINT8		ErrorCode[8];		// 128. byte
	NUINT8		APVersion[15];		// 143. byte
	NUINT8		RMSVersion[15];		// 158. byte
	NUINT16		LastTranDate[6];	// 170. byte
	NUINT32		DeviceStatus;		// 174. byte

	NUINT8		MWIVer[15];			// 189. byte
	NUINT8 		VDMVer[15];			// 204. byte
    NUINT8 		CDMSPVer[15];		// 219. byte
    NUINT8 		IDCSPVer[15];		// 234. byte
    NUINT8 		PINPADSPVer[15];	// 249. byte
    NUINT8		SIUSPVer[15];		// 264. byte
    NUINT8 		TTUSPVer[15];		// 279. byte
    NUINT8 		VFDSPVer[15];		// 294. byte
    NUINT8 		SPRSPVer[15];		// 309. byte
    NUINT8 		UPSSPVer[15];		// 324. byte
    NUINT8 		PTRSPVer[15];		// 339. byte
    NUINT8		JPRSPVer[15];		// 354. byte
    NUINT8 		CAMSPVer[15];		// 369. byte
    NUINT8 		PNCSPVer[15];		// 384. byte
    NUINT8 		COMSPVer[15];		// 399. byte
    NUINT8 		CDMEPVer[15];		// 414. byte
    NUINT8 		IDCEPVer[15];		// 429. byte
    NUINT8 		PINPADEPVer[15];	// 444. byte
    NUINT8 		SIUEPVer[15];		// 459. byte
    NUINT8 		TTUEPVer[15];		// 474. byte
    NUINT8 		VFDEPVer[15];		// 489. byte
    NUINT8 		SPREPVer[15];		// 504. byte
    NUINT8 		UPSEPVer[15];		// 519. byte
    NUINT8 		PTREPVer[15];		// 534. byte
    NUINT8 		JPREPVer[15];		// 549. byte
    NUINT8 		CAMEPVer[15];		// 564. byte
    NUINT8 		PNCEPVer[15];		// 579. byte
    NUINT8 		COMEPVer[15];		// 594. byte

	NUINT16		SC_RFIDStatus;		// 596. byte
	NUINT16		SC_BCRStatus;		// 598. byte
	NUINT16		SC_CIMStatus;		// 600. byte
	NUINT16		SC_CSTBillCount;	// 602. byte
	NUINT16		SC_CSTStatus;		// 604. byte

} RMS_RES_STATUS, *PRMS_RES_STATUS;

// sizeof(RMS_RES_STATUS) - length.
#define LEN_RMS_RES_STATUS	(sizeof(RMS_RES_STATUS) - 2)

typedef struct _RMS_SETUP_READ_DATA_
{
	RMS_COMM	Header;				//   4 byte

	NUINT16		FrameNumber;		//   6 byte

	NUINT16		dwYear;				//   8 byte
	NUINT16		dwMonth;			//  10 byte
	NUINT16		dwDay;				//  12 byte
	NUINT16		dwHour;				//  14 byte
	NUINT16		dwMinute;			//  16 byte
	NUINT16		dwSecond;			//  18 byte

	NUINT8		MachineType;		//  19 byte
	NUINT8		SerialNumber[11];	//  30 byte
	
	NUINT16		CST1Denomination;	//  32. byte
	NUINT16		CST1BillCount;		//  34. byte
	NUINT16		CST2Denomination;	//  36. byte
	NUINT16		CST2BillCount;		//  38. byte
	NUINT16		CST3Denomination;	//  40. byte
	NUINT16		CST3BillCount;		//  42. byte
	NUINT16		CST4Denomination;	//  44. byte
	NUINT16		CST4BillCount;		//  46. byte

	NUINT16		JournalCount;		//  48. byte
	NUINT8		ATMStatus;			//  49. byte

	NUINT8		ErrorCode[8];		//  57. byte
	NUINT8		SetupData[RMS_SETUP_DATA_FRAME_SIZE];		// 569. byte : 

} RMS_SETUP_READ_DATA, *PRMS_SETUP_READ_DATA;

// sizeof(RMS_SETUP_READ_DATA) - Length
#define LEN_RMS_SETUP_READ_DATA	(sizeof(RMS_SETUP_READ_DATA) - 2)

typedef struct _RMS_SETUP_WRITE_DATA_
{
	RMS_COMM	Header;				//   4 byte

	NUINT16		FrameNumber;		//   6 byte
	NUINT8		TerminalNumber[16];	//  22 byte
	NUINT8		RMSPassword[7];		//  29 byte

	NUINT8		SetupData[512];		// 541 byte

} RMS_SETUP_WRITE_DATA, *PRMS_SETUP_WRITE_DATA;

// sizeof(RMS_SETUP_WRITE_DATA) - Length
#define LEN_RMS_SETUP_WRITE_DATA	(sizeof(RMS_SETUP_WRITE_DATA) - 2)

typedef struct _RMS_FILE_DLOAD_DATA_
{
	RMS_COMM	Header;				//   4 byte

	NUINT16		FrameNumber;		//   6 byte
	NUINT8		FileData[3072];		// 2054 byte

} RMS_FILE_DLOAD_DATA, *PRMS_FILE_DLOAD_DATA;

// sizeof(RMS_FILE_DLOAD_DATA) - Length
#define LEN_RMS_FILE_DLOAD_DATA		(sizeof(RMS_FILE_DLOAD_DATA) - 2)
#define LEN_RMS_FILE_DATA_TCP		(sizeof(RMS_FILE_DLOAD_DATA) - 2 - 4)
#define LEN_RMS_FILE_DATA_DIAL		(sizeof(RMS_FILE_DLOAD_DATA) - 2 - 4 - 48)

typedef struct _RMS_SETUP_DATA_
{
	// No. 1 Termianl number
	NUINT8		TerminalNumber[16];			//   16.
	NUINT8		SerialNumber[11];			//   27.
	NUINT8		RoutingID[7];				//   34.
	NUINT8		CommunicationID[11];		//   45.
	NUINT8		HostTel1[21];				//   66.
	NUINT8		HostTel2[21];				//   87.
	NUINT8		HostTel3[21];				//	108.

	// No. 8 Modem Connect Timer
	NUINT16		ModemConnetTimer;			//  110.
	NUINT8		ModemSpeakerEnable;			//  111.
	NUINT8		ModemInitialString[51];		//  162.
	NUINT8		ModemVendor[2];				//  164.

	// No. 12 ENQ
	NUINT32		ENQTimeoutInterval;			//  166.
	NUINT8		EOTCheckMode;				//  167.
	NUINT8		ProcessorCode;				//  168.
	NUINT8		FormatOption;				//  169.
	NUINT8		HealthCheckSendEnable;		//  170.
	NUINT16		HealthCheckInterval;		//  172.
	
	// No. 18
	NUINT8		RMSAcceptEnable;			//  173.
	NUINT8		RMSStatusSendEnable;		//	174.
	NUINT8		RMSPassword[7];				//	181.
	NUINT8		RMSNewPassword[7];			//	188.
	NUINT8		RMSTel1[21];				//	209.
	NUINT8		RMSTel2[21];				//	230.
	NUINT8		RMSRingCount;				//	231.

	// No. 25 Speaker Vol.
	NUINT8		SpeakerVolume;				//	232.

	NUINT8		ISO1Enable;					//	233.
	NUINT8		ISO2Enable;					//	234.
	NUINT8		ISO3Enable;					//	235.
	NUINT8		SurchargeOwner[26];			//	261.
	NUINT8		SurchargeEnable;			//	262.

	NUINT16		SurchargeAmount;			//	264.
	NUINT16		FastCashAmount[6];			//	276.
	NUINT32		MaxDispenseAmount;			//	280.
	NUINT8		LowCurrencyCheckEnable;		//	281.
	NUINT8		EnglishMode;				//	282.

	NUINT8		SpanishMode;				//	283.
	NUINT8		JapaneseMode;				//	284.
	NUINT8		FrenchMode;					//	285.
	NUINT8		KoreanMode;					//	286.
	NUINT8		ChineseMode;				//	287.

	NUINT8		SelectReceipt;				//	288.
	NUINT8		ReceiptEnable;				//	289.
	NUINT8		ReceiptHeader[4][41];		//	453.
	NUINT8		ReceiptAddress[3][41];		//	576.
	NUINT8		ReceiptPhoneNo[41];			//	617.

	NUINT8		VFDDisplayOpt[5];			//	622.
	NUINT8		VFDFontOption[5];			//	627.
	NUINT8		VFDText[5][33];				//	792.
	NUINT8		MasterPassword[7];			//	799.
	NUINT8		ServicePassword[7];			//	806.

	NUINT8		OperatorPassword[7];		//	813.
	NUINT8		MachineType;				//	814.
	NUINT8		SPRType;					//	815.
	NUINT8		JPRType;					//	816.
	NUINT8		MCUType;					//	817.

	NUINT8		CDUType;					//	818.
	NUINT8		PBType;						//	819.
	NUINT8		KeyType;					//	820.
	NUINT8		SoundType;					//	821.
	NUINT8		CameraType;					//	822.

	NUINT8		ICCardType;					//	823.
	NUINT8		PowerType;					//	824.
	NUINT8		SPLDevice;					//	825.
	NUINT8		VFDDevice;					//	826.
	NUINT8		DualMonitor;				//	827.

	NUINT8		DVRDevice;					//	828.
	NUINT8		OPDevice;					//	829.
	NUINT8		MCUManufacturer;			//	830.
	NUINT8		CDUShutter;					//	831.
	NUINT8		AdaDevice;					//	832.

	NUINT8		dummy1;						//	833.
	NUINT8		dummy2;						//	834.
	NUINT16		BINListCount;				//	836.
	NUINT8		BINList[300][11];			//	4,136.
	NUINT8		RMSSendInterval;			//	4,137.

	NUINT8		RMSJournalUpload;			//	4,138.
	NUINT16		CDULowCount;				//	4,140.

	// No. 78 - RMS Callback
	NUINT8		RMSOnlyCallback;
	NUINT8		RMSOnlyCallbackRtyCnt;

	// No. 80 - ATM
	NUINT8		DHCPEnable;
	NUINT8		ATMIP[16];
	NUINT8		SubnetMask[16];
	NUINT8		GateWay[16];
	NUINT8		DNSServer[16];

	// No. 85 - HOST
	NUINT8		IsHostUseURL;
	NUINT8		HostName1[256];
	NUINT16		HostPort1;
	NUINT8		HostName2[256];
	NUINT16		HostPort2;

	// No. 90 - ATM Network option
	NUINT8		NetworkType;
	NUINT8		SSLEnable;
	NUINT8		TCPType;
	NUINT8		CRCEnable;
	
	// No. 94 - RMS
	NUINT8		IsRMSUseURL;
	NUINT8		RMSConnectType;
	NUINT8		RMSName[256];
	NUINT16		RMSInboundPort;		// ATM->RMS
	NUINT16		RMSOutboundPort;	// RMS->ATM

	// No. 99 - Advertisement
	NUINT16		AdvGuideTerm;
	NUINT8		AdvEnable;
	NUINT8		AdvTitle[6][41];

	// No. 102 - Adevertisement Coupon Msg
	NUINT8		AdvCouponEnable;
	NUINT8		AdvCouponMsg1[2][41];
	NUINT8		AdvCouponMsg2[2][41];
	NUINT8		AdvCouponMsg3[2][41];
	NUINT8		AdvCouponMsg4[2][41];
	NUINT8		AdvCouponMsg5[2][41];
	NUINT8		AdvCouponMsg6[2][41];

	// No. 109 - AID List(for EMV)
	NUINT8		AIDList[20][33];		// [#79] NH PSC 2008.04.15 AID List 추가

	// No. 110 - Encryption Key
	NUINT8		EncryptionKey;
	NUINT8		PercentSurchargeEnable;
	NUINT8		SurchargeWarningManner;
	NUINT8		PercentSurchargeAmount;
	
	// No. 114 - Pre-dial Enable
	NUINT8		PreDialEnable;
	NUINT8		PreDialOption;

	// No. 116 - Balance At First Enable
	NUINT8		BalanceAtFirstEnable;
	NUINT8		FastCashAtFirstEnable;
	NUINT8		Mode10Enable;
	NUINT8		StatusMonitoring;
	
	// No. 120 - Surcharge Owner Print Enable
	NUINT8		SurchargeOwnerPrintEnable;
	NUINT8		AutoDayTotalEnable;

	// No. 122 - Trial Day Total Time
	NUINT8		AutoDayTotalTime[4];
	NUINT8		_ignore; // US William - These fields were sized incorrectly, so we are skipping a byte on both to preserve compatibility with the RMS database
	NUINT8		LastDayTotalDate[8];
	NUINT8		_ignore2;

	// No. 124 - Welcome Message 1,2,3,4
	NUINT8		WelcomeMessage[4][31];
	NUINT8		StoreMessage[4][41];
	NUINT8		ProcessorMessage[4][41];
	NUINT8		MarketingMessage[4][41];

	// No. 128 - Checking Account
	NUINT8		CheckingAccount;
	NUINT8		SavingAccount;
	NUINT8		CreditAccount;

	// No. 131 - NUA (Network User Address)
	NUINT8		NUA[16];

#if (RMS_VERSION >= 0x010301)		// [#286] [NH] KSK 2008.6.12
	// [#284] [MX] KSK 2008.6.11 Added RMS Setup Information
	// No. 132 - Surcharge (cash, balance, PIN)
	NUINT32		CashWithdrawalSurcharge;
	NUINT32		BalanceSurcharge;		// 고객에게 보여지는 Surcharge 값.
	NUINT32		PINChangeSurcharge;

	// No. 135 - Default Language
	NUINT8		DefaultLanguage;

	// No. 136 - Continue After Pre-Balance
	NUINT8		ContinueAfterPreBalance;
	// end of [#284]
#endif

	///////////////////////
#if (RMS_VERSION >= 0x010303)	// [#337] [NH] AIREAT 2008.06.25

	// No. 137 - Exit Message
	NUINT8		ExitMessage[3][31];

	// No. 138 - Balance Surcharge
	NUINT16		nInquirySurchargeAmount;		// Host로 전송하는 Surcharge 값.

#endif

	// No. 139 - CheckCasing Option
#if (RMS_VERSION >= 0x010304)	// [#366] [NH] KSK 2008.07.15
	NUINT8		CheckCashing;
	NUINT8		PINOption;
	NUINT8		CheckCashingAvailable;	// [#378] [NH] KSK 2008.7.18
#endif							// end of [#366]

	//No. 142 - IC Available
#if (RMS_VERSION >= 0x010305)	// [#397] [NH] psc 2008.08.01 EMV 옵션저장
	NUINT8		EmvAvailable;
	NUINT8		EmvEnable;
	NUINT8		nSurchargeWarningLocation;		// 0: begining, 1: After amount
	NUINT8		EmvTransactionOption;			// 0: MS first, 1: IC first
#endif

	// No. 146 - Auto Day total type
#if (RMS_VERSION >= 0x010307)
	NUINT8		AutoDayTotalType;
	NUINT8		SurchargeOwnerContactInfo[3][26];
#endif

// [#518] [MX] KSK 2009.3.9
	// No. 148
#if (RMS_VERSION >= 0x010308)
	NUINT8		CurrencyExchangeEnable;
	NUINT8		CurrencyExchangeRate[12];
	NUINT8		CSTCurrencyID[4][3];
#endif
// end of [#518]

// [#528] AU AIREAT 2009.06.02
	// No. 151
#if (RMS_VERSION >= 0x010310)
	NUINT8		NeedMoreTime;
#endif
// end of [#528]

// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
	// No. 152
#if (RMS_VERSION >= 0x010311)
	NUINT8		AwardCouponEnable[6];
	NUINT8		MultipleBinAvailable;
#endif
// end of [#546]

#if (RMS_VERSION >= 0x010312)
	// No. 154
// [#558] KSK 2009.08.17 STANDARD1 OPTION 기능 추가
	NUINT8		TerminalStatusFieldEnable;
	NUINT8		ReversalAtHostError;
	NUINT8		Standard1OptionAvailable;
// end of [#558]
#endif

#if (RMS_VERSION >= 0x010313)
	// No. 157
	NUINT8		ADDisplayResolution;		// KSK 2010.01.19 XP만 사용
	NUINT8		SWFADVolume;				// KSK 2010.01.19 XP만 사용
	NUINT8		MainADEn_Disable[6];		// KSK 2010.01.19 XP만 사용
	NUINT8		MainADFileExtentionType[6];	// KSK 2010.01.19 XP만 사용
	NUINT8		MainADTitle[6][41];			// KSK 2010.01.19 XP만 사용
	NUINT8		MainADInterval[6];			// KSK 2010.01.19 XP만 사용
	NUINT8		SubADInterval;				// KSK 2010.01.19 XP만 사용
	NUINT8		SubADEn_Disable[6];			// KSK 2010.01.19 XP만 사용
	
	NUINT8		PaperLowSensorEnable;		//[#596] SOOK 2009.12.11 SPR PAPER LOW SENSOR EN/DISABLE 기능 추가
	NUINT8		EPPFlickerOption;			//[#604] NH KSK 2010.01.05
	NUINT8		CSTSoundOption;				//[#604] NH KSK 2010.01.05
	NUINT8		EmvLatchOption;				//[#605] NH KSK 2010.01.05
	NUINT8		ReasonForReversal;			//[#606] NH KSK 2010.01.06
#endif

#if (RMS_VERSION >=0x010314)
	// No. 170
	NUINT8		ConfigurationOption;		//[#615] AU_C KSK 2010.01.21
	NUINT8		ScheduledJournalEnable;		//[#613] AU_C KSK 2010.01.18
	NUINT8		ScheduledJournalOption;		//[#613] AU_C KSK 2010.01.18
	NUINT16		ScheduledJournalCount;		//[#613] AU_C KSK 2010.01.18
	NUINT8		ScheduledJournalDay;		//[#613] AU_C KSK 2010.01.18
	NUINT8		ScheduledJournalHour;		//[#613] AU_C KSK 2010.01.18
	NUINT8		ScheduledJournalAvailable;  //[#613] AU_C SOOK 2010.01.21
	NUINT8		ADBrandName[11];			//[#616] AU_C SOOK 2010.02.05 AD BRAND NAME
	NUINT8		TransactionAdver1Enable;	//[#616] AU_C SOOK 2010.02.05 AD BRAND NAME
	NUINT8		TransactionAdver2Enable;	//[#616] AU_C SOOK 2010.02.05 AD BRAND NAME
#endif

#if (RMS_VERSION >=0x010315)				// [#620] US KSK 2010.02.22
	NUINT8	EnhancedCouponAvailable;
#endif										// end of [#620]

#if (RMS_VERSION >= 0x010316)				// [#631] MX, 2010.03.29 JERRY
	NUINT8	BankNameFeePrint;
#endif										// end of [#631]

#if (RMS_VERSION >= 0x010317)
	NUINT8	SSLVersion;
	NUINT8	TransactionADDispTime;
	NUINT8	TransactionADEnable[6];
	NUINT8	ChangeBackgroundEnable;
	NUINT8	ChangeBackgroundScreenEnable[6];
	NUINT8	DefaultBackgroundScreen;
	NUINT8	TouchVibrationEnable;
	NUINT8	NoticeEnable;
	NUINT8	NoticeTitle[31];
	NUINT8	NoticeMsg[3][41];
#endif

// [#2106] NH KSK 2011.12.19 Setup 정보 추가
#if (RMS_VERSION >= 0x010318)
	NUINT8	ESUEnable;
	NUINT8	ESUShutterEnable;
	NUINT16	ESUTimeThreshold;

	/**
	 * Deprecated in favor of 16bit field in 01.03.47
	 */
	NUINT8	ESURecoveryTime;  // [#RWC6-168] US William 2020.06.09 Anti-skimming 2B RMS setup field
	NUINT8	RKTAvailable;
	NUINT8	RKTEnable;
	NUINT8	RKTRandomNumberEnable;
	NUINT8	EJUPLOADEnable;
#endif
	// end of [#2106]

// [#2206] NH Justin 2013.06.27 Add Setup Info - Dynamic Flow, Dual Host DCC
#if (RMS_VERSION >= 0x010319)
	NUINT8	SupervisorLanguage;		// [#2213] US Justin 2013.07.30

	NUINT8	DynamicFlowEnable;				
	NUINT8	DynamicFlowPINChange;
	NUINT8	DynamicFlowDCC;
	NUINT8	DynamicFlowDCCOption;

	NUINT8	DualHostDCCEnable;
	NUINT8	DualHostDCCSSLEnable;
	NUINT8	DualHostDCCSSLVersion;
	NUINT8	DualHostDCCIsHostUseURL;
	NUINT8	DualHostDCCHostName1[256];
	NUINT16	DualHostDCCHostPort1;
	NUINT8	DualHostDCCHostName2[256];
	NUINT16	DualHostDCCHostPort2;

	NUINT8	DCCDIsclaimer[6][41];

	NUINT8	ReceiptOnScreen;
#endif
	// end of [#2206]

// [#2242] NH Justin 2013.12.18 DF 1st Call Option
#if (RMS_VERSION >= 0x010320)
	NUINT8	DynamicFlow1stCallOption;
#endif
// End of [#2242]

// [#2255] NH KSK 2014.02.26 Added NOTE Counting Option
#if (RMS_VERSION >= 0x010321)
	NUINT8	NoteCountingOption;
#endif
// end of [#2255]

	// [#2309] US Justin 2014.11.17 US Balance No Fee Notice
#if (RMS_VERSION >= 0x010323)
	NUINT8	BalanceFeeNotice;
#endif
// end of [#2309]

// [#2316] NH Justin 2014.12.17 Decimal point percentabe
#if (RMS_VERSION >= 0x010324)
	NUINT16 DecimalPointPercentage;
#endif
// End of [#2316]

// [#2342] NH Justin 2015.05.14 Add Use Common AID Info
#if (RMS_VERSION >= 0x010326)
	NUINT8 UseCommonAID;
	NUINT8 EnableDonation;		// [#2347] US Justin 2015.05.28 Donation
#endif
// End of [#2342]

// [#2359] NH Justin 2015.07.15 Add MV Setup Data
#if (RMS_VERSION >= 0x010327)
	NUINT8 PercentageSurchargeNotice;
	NUINT8 PopmoneyEnable;		
	NUINT8 LoanAddEnable;		// Deprecated 2019-12-31
#endif
// End of [#2359]

// [#2405] NH Justin 2015.07.15 Add MV Setup Data
#if (RMS_VERSION >= 0x010328)
	NUINT8 Pin4Enable;
	NUINT8 MoniMobileQR;			// [#2409] US Justin 2016.03.28 Add Moniview MoniMobile Setup Information
	NUINT8 WalPay;					// Deprecated 2020-01-02
	NUINT8 WalPayMachineCode[6];	// Deprecated 2020-01-02
	NUINT16 WalPayDispLimit;		// Deprecated 2020-01-02
	NUINT8 WalPayAmountOption;		// Deprecated 2020-01-02
	NUINT8 EMVLanguageSelection;	// [#2425] US Justin 2016.05.23
	NUINT8 LocalLoanEnable;			// Deprecated 2019-12-31
	NUINT8 LocalLoanHealthCheck;	// Deprecated 2019-12-31
#endif
// End of [#2405]

// [#2435] NH Justin Add Kernel Version
#if (RMS_VERSION >= 0x010329)
	NUINT8 RMSEMVKernelVersion;
#endif
// End of [#2435]

// [#2442] NH Justin Add Donation Type
#if (RMS_VERSION >= 0x010330)
	NUINT8 NHDonationType;
	NUINT8 NHDualBalance;			// [#2444] US Jusin 2016.09.02
#endif
// End of [#2442]

// [#2449] NH Justin Add Paypal and Just.Cash Setup
#if (RMS_VERSION >= 0x010331)
	NUINT8 NHPaypalCCA;
	NUINT8 NHJustCash;
	NUINT8 LoanUseDualHost;				// Deprecated 2019-12-31
	NUINT8 LoanDualHostIP[41];			// Deprecated 2019-12-31
	NUINT16	LoanDualHostPort;			// Deprecated 2019-12-31
	NUINT8 LoanDualHostUpdateBalance;	// Deprecated 2019-12-31
	NUINT8 HostSSLTLSCertificate;		// [#2475] US Justin 2017.02.22
#endif
// End of [#2449]

// [#2508] NH Justin 2017.09.27 Upload Master Key checksums to Moniview
#if (RMS_VERSION >= 0x010332)
	NUINT8	MasterKeyCSum[4][9];
	NUINT8	MACKeyCSum[9];
#endif
// End of [#2508]

// [#2497] AU Kook 2017.10.20 ATS ALPHI
#if (RMS_VERSION >= 0x010333)
	NUINT8	AlphiEnDisable;
	NUINT8	AlphiAddress[40];
	NUINT16	AlphiPort;
	NUINT8	AlphiID[11];
	NUINT8	AlphiTermID[8];
	NUINT8	AlphiOprID[15];
	NUINT8	AlphiTDESKey[32];
	NUINT8	AlphiExtraInformation[40];
	//NUINT16	AlphiMsgAuditNum;
	NUINT8	AlphiReversalMaxRetry;
	NUINT8	AlphiCardDataToBeUsed;
	NUINT8	AlphiCurrencyCode[3];   // [#2497] NH woooZ 2017.10.25 ALPHI country code
#endif
// end of [#2497]

// [#2517] NH Justin 2017.11.30  EMV Fallback Enable/Diable Option
#if (RMS_VERSION >= 0x010334)
	NUINT8 EMV_Fallback4UnknownICCard;
	NUINT8 EMV_Fallback_EnDisable;
#endif
// End of [#2517]

// [#2538] NH Justin 2018.03.09 MoniMobile Cash Balance Option
#if (RMS_VERSION >= 0x010335)
	NUINT8 MoniMobileCashBalanceOption;
	NUINT8 MoniVisionEnable;					// [#2548] NH Justin 2018.04.27 Change Camera Option Name : MoniVision, Live-Feed, Journal Pic
	NUINT8 MoniVisionLiveFeedOption;			// [#2548] NH Justin 2018.04.27 Change Camera Option Name : MoniVision, Live-Feed, Journal Pic
	NUINT8 MoniVisionJournalPicOption;			// [#2548] NH Justin 2018.04.27 Change Camera Option Name : MoniVision, Live-Feed, Journal Pic
	NUINT8 USTerritoryOption;					// [#2549] NH Justin 2018.05.07 US Territory Option
#endif
// End of [#2538]

// [#2558] NH Justin 2018.06.13 Add Scheduled Reboot Optioin
#if (RMS_VERSION >= 0x010336)
	NUINT8 RebootOption;
	NUINT16 RebootTime;
	NUINT16 RebootInterval;
	NUINT8 GivePayEnable;						// [#2557] NH Justin 2018.06.12 GivePay Enhancement 
	NUINT8 RMSSendRetry;						// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
#endif
// End of [#2558]

	// [#2579] AU Kook 2018.10.12 Support ALPHI SSL Configuration via MoniView
#if (RMS_VERSION >= 0x010337)
	NUINT8	AlphiSSLEnDisable;
	NUINT8	AlphiSSLCertEnDisable;
#endif
	// end of [#2579]

#if (RMS_VERSION >= 0x010338)
	NUINT16 GivePayConfigTime;					// [#2574] US Justin GivePay Enhancement3, Add Download Config Time
	NUINT8  GivePayUpSell;						// [#J006] Upsell Option
	//NUINT8	GivePayEnrolled;					// [#RWC6-1, #2583] US Brandon 2018.12.17 GivePay DG911 fix, WinATM error fix, Enhancement 6
	NUINT8  DynamicFlowDCCType;					// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
	NUINT8  DualHostDCCType;					// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD1/STD3 message
#endif

#if (RMS_VERSION >= 0x010339)					// [#RWC6-10] Adding secondary DNS with default value of 8.8.8.8. or 8.8.4.4, for PAI
	NUINT8	DNSServer2nd[16];			
#endif

#if (RMS_VERSION >= 0x010340)
	NUINT8  RMSSendSilent;						// [#2580] AU Kook 2019.02.14 Support RMS Status Silent Send.
	NUINT8	RKTEPPSerialNumber[8];				// [#2580] AU Kook 2019.07.08 RKT EPP Serial Number
	NUINT8	AllPointSurcharge;
#endif

#if (RMS_VERSION >= 0x010341)
	NUINT8	RMSTLSEnable;						// [#RWC6-67] US William 2019.10.24 RMS TLS
	NUINT16	RMSTLSListeningPortNumber;			// [#RWC6-67] US William 2019.10.24 RMS TLS	
#endif

#if (RMS_VERSION >= 0x010342)
	NUINT8	PrintTotalAmountOption;				// [#GLDV-2681] AU Kook 2019.11.06 Print Total Amount
#endif

// [#RWC6-119] US William 2020.01.16 Add JustCash RMS changes
#if (RMS_VERSION >= 0x010343)
	NUINT8	JustCashMode;
	NUINT8	JustCashRegistrationState;
	NUINT8  B4UEnabled;							// [#RWC6-140] US William 2020.02.21 B4U RMS
#endif
// end of [#RWC6-119]

// [#GLDV-2792] AU Kook 2020.03.13 RMS Timeout configuration via MoniView
#if (RMS_VERSION >= 0x010344)
	NUINT8	RMSTimeoutConnect;
	NUINT8	RMSTimeoutReceive;
#endif
// end of [#GLDV-2792]

// [#RWC6-169] US William 2020.04.28 Add PAI Credit Segmentation RMS
#if (RMS_VERSION >= 0x010345)
	NUINT16 CreditAdditionalSurchargeAmount;

	// [#RWC6-170] US William 2020.04.30 Add Remote Update Config to RMS
	NUINT8  UpdateRepoConfigValid;
	NUINT8	UpdateRepoHostname[64];
	NUINT8	UpdateRepoHives[64];
	NUINT8  ScheduledUpdatesEnabled;
	NUINT8  ScheduledUpdateDayOfWeek;
	
	// [#RWC6-127] Add Disk Management to RMS Status
	NUINT32 DiskMgmtAtmFreeCapKiBytes;
	NUINT32 DiskMgmtAtmUsedCapKiBytes;
	NUINT32 DiskMgmtAtm2FreeCapKiBytes;
	NUINT32 DiskMgmtAtm2UsedCapKiBytes;
	NUINT32 DiskMgmtTotalFreeCapKiBytes;
	NUINT32 DiskMgmtTotalUsedCapKiBytes;
#endif

#if (RMS_VERSION >= 0x010346)
	NUINT8	Standard3CommHeaderEnabled;
	NUINT16 ESURecoveryTime2Byte;
#endif

#if (RMS_VERSION >= 0x010347)
	NUINT8	LibertyXBuyEnabled;
#endif

#if (RMS_VERSION >= 0x010348)
	NUINT8	CDUDispenseStyle;
#endif

#if (RMS_VERSION >= 0x010349)
	NUINT8	LibertyXSellEnabled;
#endif

#if (RMS_VERSION >= 0x010350)
	NUINT8	LibertyXUsername[32];
#endif

#if (RMS_VERSION >= 0x010351)
	NUINT8	ModelName[10];
	NUINT8	LocalDCCSurchargeEnabled;
	NUINT8	CDUBindingEnabled;
	NUINT8	CDUBindingActive;
#endif

#if (RMS_VERSION >= 0x010352)
	NUINT8	RetailRemoteUpdateComponent1[32];
    NUINT8	RetailRemoteUpdateComponent2[32];
    NUINT8	RetailRemoteUpdateComponent3[32];
    NUINT8	RetailRemoteUpdateComponent4[32];
    NUINT8	RetailRemoteUpdateComponent5[32];
    NUINT8	RetailRemoteUpdateComponent6[32];
    NUINT8	RetailRemoteUpdateComponent7[32];
    NUINT8	RetailRemoteUpdateComponent8[32];
#endif

#if (RMS_VERSION >= 0x010353)
	NUINT8	SurchargeSegmentation;
	NUINT8	SingleHostDCCEnabled;
	NUINT8	SingleHostDCCValue[32];
	NUINT8	SidecarEnabled;
	NUINT8	RFIDEnabled;
	NUINT8	AcceptorType;
#endif

} RMS_SETUP_DATA, *PRMS_SETUP_DATA;

#define LEN_RMS_SETUP_DATA	(sizeof(RMS_SETUP_DATA))	// header가 없어서 2를 빼지 않고 계산한다.

//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 

// Weahter Info
typedef struct _WEATHER_INFO_
{
	NUINT16	Year;			//	2 byte
	NUINT16	Month;			//	2 byte
	NUINT16	Day;			//	2 byte
	NUINT8	DayOfWeek[4];
	NUINT8	WeatherCode;	
	NUINT16	HighestTemp;
	NUINT16 LowestTemp;

} WEATHER_INFO, *PWEATHER_INFO;


typedef struct _RMS_WEATHER_DATA_
{
	NUINT16	ContentsCount;			//	2 byte
	NUINT8	TemperatureType;		

	WEATHER_INFO WeaterInfo[7];	//Text 6자 씩 10줄 

} RMS_WEATHER_DATA, *PRMS_WEATHER_DATA;

//Journal Image upload Command수신 전문 
typedef struct _RMS_CMD_IMAGE_UPLOAD_
{
	RMS_COMM	Header;
	
	NUINT16	FrameNumber;			//	2 byte
	NUINT8	TerminalNumber[16];		// 16 byte
	NUINT8	RMSPassword[7];			//  7 byte

	NUINT16	StackNumber;
	NUINT16	StackYear;
	NUINT16	StackMonth;
	NUINT16	StackDay;
	
} RMS_CMD_IMAGE_UPLOAD, *PRMS_CMD_IMAGE_UPLOAD;

//Journal Image Upload 응답 전문
typedef struct _RMS_IMAGE_UPLOAD_DATA_
{
	RMS_COMM	Header;				//   4 byte

	NUINT16		FrameNumber;		//	 6 byte
	
	NUINT16		dwYear;				//   8 byte
	NUINT16		dwMonth;			//  10 byte
	NUINT16		dwDay;			//  12 byte
	NUINT16		dwHour;				//  14 byte
	NUINT16		dwMinute;			//  16 byte
	NUINT16		dwSecond;			//  18 byte
	NUINT8		MachineType;		//	19 byte comm1 header

	NUINT8		SerialNumber[11];	//  30 byte
	
	NUINT16		CST1Denomination;	//	32 byte
	NUINT16		CST1BillCount;		//	34 byte
	NUINT16		CST2Denomination;	//	36 byte
	NUINT16		CST2BillCount;		//	38 byte
	NUINT16		CST3Denomination;	//	40 byte
	NUINT16		CST3BillCount;		//	42 byte
	NUINT16		CST4Denomination;	//	44 byte
	NUINT16		CST4BillCount;		//	46 byte
	
	NUINT16		JournalCount;		//  48 byte
	NUINT8		ATMStatus;			//  49 byte
	
	NUINT8		ErrorCode[8];		//  57 byte

	NUINT8		TotalImageCount;	//  58 byte
	NUINT8		CurrentImageSeq;	//	59 byte
	NUINT16		UploadDataSize;		//  61 byte
	NUINT8		FileData[2048];		// 2109 byte
	
} RMS_IMAGE_UPLOAD_DATA, *PRMS_IMAGE_UPLOAD_DATA;

// [#RWC6-56] US William Hashed Password
typedef struct _RMS_HASHED_PASSWORD_CHANGE_	
{
	RMS_CMD_COMM	Header;				//	4  byte

	char*		OldMasterPassword;	
	char*		NewMasterPassword;	
	char*		NewOperatorPassword;
	char*		NewServicePassword;

	char* PasswordForIdentity(Identity i) const
	{
		switch(i)
		{
		case Master: return NewMasterPassword;
		case Operator: return NewOperatorPassword;
		case Service: return NewServicePassword;
		default: return NULL;
		}
	};
} RMS_HASHEDPASSWORDCHANGE, *PRMS_HASHEDPASSWORDCHANGE;
// end of [#RWC6-56]

//end of [#586]

// [#RWC6-134] US William 2020.03.24 RMS Remote Updates
typedef struct _RMS_INSTALL_REMOTE_UPDATE_
{
	RMS_CMD_COMM    Header;
	char            VersionName[32];

	/**
	 * Gets the CString representation of the version number
	 */
	CString GetVersionName() const 
	{
		CString version;
		version.Format(L"%S", VersionName);

		return version;
	};
} RMS_CMD_INSTALLREMOTEUPDATE, *PRMS_CMD_INSTALLREMOTEUPDATE;

// [#RWC6-208] US William 2020.08.05 Add LTX change password job
typedef struct _RMS_CHANGE_LTX_PASSWORD_
{
	RMS_CMD_COMM    Header;
	char			Username[64];
	char			Password[64];

	CString GetValue(const char data[64]) const
	{
		CString output;
		char buffer[65] = {};
		memcpy_s(buffer, 65, data, 64);

		output.Format(L"%S", buffer);

		memset(buffer, 0, 65);

		return output;
	};

	bool ChangePassword() const 
	{
		LXConfiguration config;
		CLibertyXConfigurationManager manager;
		
		if (!manager.GetConfiguration(config))
		{
			NHERROR((L"Failed to get LTX configuration\r\n"));
			return false;
		}

		config.Username = GetValue(Username);
		config.Password = GetValue(Password);

		return manager.SaveConfiguration(config);
	};

} RMS_CMD_CHANGELTXPASSWORD, *PRMS_CMD_CHANGELTXPASSWORD;

#pragma warning(push)
#pragma warning(disable:4351)	// disable unimportant warning below
// warning C4351: new behavior: elements of array '_RMS_INSTALL_REMOTE_UPDATE_RESPONSE_::ResultMessage'
// will be default initialized
typedef struct _RMS_INSTALL_REMOTE_UPDATE_RESPONSE_
{
	RMS_COMM	Header;				// 4 Bytes
	NUINT16		FrameNumber;		// 2 Bytes

	NUINT8	StatusCode;
	char	ResultMessage[32];

	_RMS_INSTALL_REMOTE_UPDATE_RESPONSE_() : ResultMessage()
	{
		memset(ResultMessage, 0, sizeof(ResultMessage));
		StatusCode = 0;
	}

} RMS_RES_INSTALLREMOTEUPDATE, *PRMS_RES_INSTALLREMOTEUDPATE;
#pragma warning(pop)

#define LEN_RMS_RES_REMOTEUPDATE		(sizeof(RMS_RES_INSTALLREMOTEUPDATE) - 2)
// End of [#RWC6-134]

// [#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
typedef struct _RMS_BINPROPERTIES_
{
	NUINT8		BinNumber[10];
	NUINT16		BinAction;
	NUINT32		BinValue;
} RMS_BINPROPERTIES, *PRMS_BINPROPERTIES;

typedef struct _RMS_BINLIST_INFO_
{
	NUINT8		AllowOnlyListedBinEnable;	
	NUINT16		TotalBinCount;
	RMS_BINPROPERTIES BinList[BIN_MAXIMUN_COUNT];
}RMS_BINLIST_INFO, *PRMS_BINLIST_INFO;
// end of [#546]

typedef struct _EJ_UPLOAD_MARK_
{
	BYTE		bUsingBlock;
	WORD		XAddressB0;
	WORD		XAddressB1;
} EJ_UPLOAD_MARK, *PEJ_UPLOAD_MARK;

typedef struct _ATM_JNL_INFO_
{
	NUINT16		DataSize;
	NUINT8		kindcode[2];
	NUINT16		stacknumber;
	NUINT16		year;
	NUINT8		month;
	NUINT8		day;
	NUINT8		hour;
	NUINT8		minute;
	NUINT8		second;
	NUINT8		data[4096];

} ATM_JNL_INFO, *PATM_JNL_INFO;

typedef struct _RMS_RES_JNL_
{
	RMS_COMM	Header;				//   4 byte

	NUINT16		FrameNumber;

	NUINT16		dwYear;				//   8 byte
	NUINT16		dwMonth;			//  10 byte
	NUINT16		dwDay;				//  12 byte
	NUINT16		dwHour;				//  14 byte
	NUINT16		dwMinute;			//  16 byte
	NUINT16		dwSecond;			//  18 byte

	NUINT8		MachineType;
	NUINT8		SerialNumber[11];

	NUINT16		CST1Denomination;	//  32. byte
	NUINT16		CST1BillCount;		//  34. byte
	NUINT16		CST2Denomination;	//  36. byte
	NUINT16		CST2BillCount;		//  38. byte
	NUINT16		CST3Denomination;	//  40. byte
	NUINT16		CST3BillCount;		//  42. byte
	NUINT16		CST4Denomination;	//  44. byte
	NUINT16		CST4BillCount;		//  46. byte

	NUINT16		JournalCount;		//  48. byte
	NUINT8		ATMStatus;			//  49. byte

	NUINT8		ErrorCode[8];		//  57. byte

	NUINT16		JNL_DataSize;
	NUINT8		JNL_KindCode[2];
	NUINT16		JNL_StackNum;
	NUINT8		JNL_StackMonth;
	NUINT8		JNL_StackDay;
	NUINT16		JNL_StackYear;
	NUINT8		JNL_StackHour;
	NUINT8		JNL_StackMinute;
	NUINT8		JNL_StackSecond;
	NUINT8		JNL_Data[1024];

} RMS_RES_JNL, *PRMS_RES_JNL;

#define LEN_RMS_RES_JNL		(sizeof(RMS_RES_JNL) - 2 - 1024)

typedef struct _RMS_JNL_TRANSACTION_
{
	NUINT8		TerminalNumber[15];				//  1
	NUINT8		SequenceNumber[4];				//  2
	NUINT8		TranType[2];					//  3
	NUINT8		FromAccount[2];					//  4
	NUINT8		ToAccount[2];					//  5
	NUINT8		BankCode[4];					//  6
	NUINT8		BankAccount[12];				//  7
	NUINT8		HostDate_Month;					//  8
	NUINT8		HostDate_Day;					//  9
	NUINT16		HostDate_Year;					// 10 
	NUINT8		HostTime_Hour;					// 11
	NUINT8		HostTime_Minute;				// 12
	NUINT8		HostTime_Second;				// 13
	NUINT8		HostAvailBalance[8];			// 14
	NUINT8		HostRetrievalNumber[12];		// 15
	NUINT8		HostAuditNetID[8];				// 16, 17
	NUINT8		HostSettleDate_Month;			// 18
	NUINT8		HostSettleDate_Day;				// 19
	NUINT16		HostSettleDate_Year;			// 20
	NUINT8		Surcharge[8];					// 21
	NUINT8		RequestAmount[8];				// 22
	NUINT8		DispensedAmount[8];				// 23
	NUINT8		RemainAmount[10];				// 24
	NUINT8		ProcCount;						// 25
	NUINT8		Result[5];						// 26
	NUINT8		ErrorCode[7];					// 27
	NUINT8		CardData[16];					// 28
	NUINT8		NonCashValue[4];				// 29
	NUINT8		NonCashType[12];				// 30
//	NUINT8		OtherMessage[77];				// 31
	NUINT8		OtherMessage[200];				// 31		// make it same as WINCE50 (needs to manage DCC information)
	
} RMS_JNL_TRAN, *PRMS_JNL_TRAN;

typedef struct _RMS_JNL_REVER_NG_
{
	NUINT8		TerminalNumber[15];				//  1
	NUINT8		SequenceNumber[4];				//  2
	NUINT8		TranDate_Month;					//  3
	NUINT8		TranDate_Day;					//  4
	NUINT16		TranDate_Year;					//  5 
	NUINT8		TranTime_Hour;					//  6
	NUINT8		TranTime_Minute;				//  7
	NUINT8		TranTime_Second;				//  8
	NUINT8		RetrievalNumber[12];			//  9
	NUINT8		RequestAmount[8];				// 10
	NUINT8		DispensedAmount[8];				// 11
	NUINT8		SurchargeAmount[8];				// 12
	NUINT8		SurchargeFlag;					// 13
	NUINT8		CardData[37];					// 14
	NUINT8		OtherMessage[77];				// 15

} RMS_JNL_REVER_NG, *PRMS_JNL_REVER_NG;

typedef struct _RMS_JNL_ADDCASH_
{
	NUINT16		CST1_Remain;					//  1
	NUINT16		CST1_Additon;					//  2
	NUINT16		CST2_Remain;					//  3
	NUINT16		CST2_Additon;					//  4
	NUINT16		CST3_Remain;					//  5
	NUINT16		CST3_Additon;					//  6
	NUINT16		CST4_Remain;					//  7
	NUINT16		CST4_Additon;					//  8

} RMS_JNL_ADDCASH, *PRMS_JNL_ADDCASH;

typedef struct _RMS_JNL_DENO_
{
	NUINT16		CST1_Old;						//  1
	NUINT16		CST1_New;						//  2
	NUINT16		CST2_Old;						//  3
	NUINT16		CST2_New;						//  4
	NUINT16		CST3_Old;						//  5
	NUINT16		CST3_New;						//  6
	NUINT16		CST4_Old;						//  7
	NUINT16		CST4_New;						//  8

} RMS_JNL_DENO, *PRMS_JNL_DENO;

typedef struct _RMS_JNL_DAYTOTAL_
{
	NUINT16		StartDate_Month;				//  1.1
	NUINT16		StartDate_Day;					//  1.2
	NUINT16		StartDate_Year;					//  1.3
	NUINT16		StartDate_Hour;					//  1.4
	NUINT16		StartDate_Minute;				//  1.5
	NUINT16		StartDate_Second;				//  1.6
	NUINT16		EndDate_Month;					//  2.1
	NUINT16		EndDate_Day;					//  2.2
	NUINT16		EndDate_Year;					//  2.3
	NUINT16		EndDate_Hour;					//  2.4
	NUINT16		EndDate_Minute;					//  2.5
	NUINT16		EndDate_Second;					//  2.6
	NUINT8		TerminalNumber[15];				//  3
	NUINT8		Result[8];						//  4
	NUINT8		Host_Count_Withdrawals[6];		//  5
	NUINT8		Host_Count_Balance[6];			//  6
	NUINT8		Host_Count_Transfer[6];			//  7
	NUINT8		Host_Count_NonCash[6];			//  8
	NUINT8		Host_Amt_Dispense[12];			//  9
	NUINT8		Host_Amt_NonCash[12];			// 10
	NUINT8		Host_Amt_Surcharge[12];			// 11
	NUINT8		Atm_Count_Withdrawals[6];		// 12
	NUINT8		Atm_Count_Balance[6];			// 13
	NUINT8		Atm_Count_Transfer[6];			// 14
	NUINT8		Atm_Count_Dispensed[6];			// 15
	NUINT8		Atm_Count_NonCash[6];			// 16
	NUINT8		Atm_Count_Reversal[6];			// 17
	NUINT8		Atm_Amt_Withdrawals[12];		// 18
	NUINT8		Atm_Amt_NonCash[12];			// 19
	NUINT8		Atm_Amt_Surcharge[12];			// 20
	NUINT8		Atm_Amt_Reversal[12];			// 21
	NUINT8		Atm_Amt_Transfer[12];			// 22
		

} RMS_JNL_DAYTOTAL, *PRMS_JNL_DAYTOTAL;

typedef struct _RMS_JNL_CSTTOTAL_
{
	NUINT16		StartDate_Month;				//  1.1
	NUINT16		StartDate_Day;					//  1.2
	NUINT16		StartDate_Year;					//  1.3
	NUINT16		StartDate_Hour;					//  1.4
	NUINT16		StartDate_Minute;				//  1.5
	NUINT16		StartDate_Second;				//  1.6
	NUINT16		EndDate_Month;					//  2.1
	NUINT16		EndDate_Day;					//  2.2
	NUINT16		EndDate_Year;					//  2.3
	NUINT16		EndDate_Hour;					//  2.4
	NUINT16		EndDate_Minute;					//  2.5
	NUINT16		EndDate_Second;					//  2.6
	NUINT8		TerminalNumber[15];				//  3
	NUINT16		CST1_Initial;					//  4
	NUINT16		CST1_Dispensed;					//  5
	NUINT16		CST1_Rejected;					//  6
	NUINT16		CST1_Jam;						//  7
	NUINT16		CST1_Remain;					//  8
	NUINT16		CST1_Denomi;					//  9
	NUINT16		CST2_Initial;					// 10
	NUINT16		CST2_Dispensed;					// 11
	NUINT16		CST2_Rejected;					// 12
	NUINT16		CST2_Jam;						// 13
	NUINT16		CST2_Remain;					// 14
	NUINT16		CST2_Denomi;					// 15
	NUINT16		CST3_Initial;					// 16
	NUINT16		CST3_Dispensed;					// 17
	NUINT16		CST3_Rejected;					// 18
	NUINT16		CST3_Jam;						// 19
	NUINT16		CST3_Remain;					// 20
	NUINT16		CST3_Denomi;					// 21
	NUINT16		CST4_Initial;					// 22
	NUINT16		CST4_Dispensed;					// 23
	NUINT16		CST4_Rejected;					// 24
	NUINT16		CST4_Jam;						// 25
	NUINT16		CST4_Remain;					// 26
	NUINT16		CST4_Denomi;					// 27
	NUINT16		NonCashValue;					// 28
	NUINT8		NonCashType;					// 29

} RMS_JNL_CSTTOTAL, *PRMS_JNL_CSTTOTAL;

typedef struct _RMS_JNL_BRIEF_DAYTOTAL_
{
	NUINT16		StartDate_Month;				//  1.1
	NUINT16		StartDate_Day;					//  1.2
	NUINT16		StartDate_Year;					//  1.3
	NUINT16		StartDate_Hour;					//  1.4
	NUINT16		StartDate_Minute;				//  1.5
	NUINT16		StartDate_Second;				//  1.6
	NUINT16		EndDate_Month;					//  2.1
	NUINT16		EndDate_Day;					//  2.2
	NUINT16		EndDate_Year;					//  2.3
	NUINT16		EndDate_Hour;					//  2.4
	NUINT16		EndDate_Minute;					//  2.5
	NUINT16		EndDate_Second;					//  2.6
	NUINT8		TerminalNumber[15];				//  3
	NUINT8		Count_Withdrawals[6];			//  4
	NUINT8		Amt_Dispense[12];				//  5

} RMS_JNL_BRIEF_DAYTOTAL, *PRMS_JNL_BRIEF_DAYTOTAL;

typedef struct _RMS_JNL_BRIEF_CSTTOTAL_
{
	NUINT16		StartDate_Month;				//  1.1
	NUINT16		StartDate_Day;					//  1.2
	NUINT16		StartDate_Year;					//  1.3
	NUINT16		StartDate_Hour;					//  1.4
	NUINT16		StartDate_Minute;				//  1.5
	NUINT16		StartDate_Second;				//  1.6
	NUINT16		EndDate_Month;					//  2.1
	NUINT16		EndDate_Day;					//  2.2
	NUINT16		EndDate_Year;					//  2.3
	NUINT16		EndDate_Hour;					//  2.4
	NUINT16		EndDate_Minute;					//  2.5
	NUINT16		EndDate_Second;					//  2.6
	NUINT8		TerminalNumber[15];				//  3
	NUINT16		CST1_Dispensed;					//  4
	NUINT16		CST2_Dispensed;					//  5
	NUINT16		CST3_Dispensed;					//  6
	NUINT16		CST4_Dispensed;					//  7

} RMS_JNL_BRIEF_CSTTOTAL, *PRMS_JNL_BRIEF_CSTTOTAL;
// End of [#2385]

typedef struct _RMS_JNL_CHANGE_PROCESSOR_
{
	NUINT8		OldProcessor[10];				//  1
	NUINT8		NewProcessor[10];				//  2

} RMS_JNL_PROCESSOR, *PRMS_JNL_PROCESSOR;

typedef struct _RMS_JNL_CHANGE_PASSWORD_
{
	NUINT8		OldPassword[8];					//  1
	NUINT8		NewPassword[8];					//  2

} RMS_JNL_PASSWORD, *PRMS_JNL_PASSWORD;

// [#408] AU AIREAT 2008.08.26
typedef struct _RMS_JNL_OPERATOR_ACTION_
{
	NUINT8		Action[40];
} RMS_JNL_OPERATOR_ACTION, *PRMS_JNL_OPERATOR_ACTION;
// end of [#408]

// KSK 2009.3.30
typedef struct _RMS_JNL_CHANGE_EXCHANGE_RATE_
{
	NUINT8		Action[40];
} RMS_JNL_CHANGE_EXCHANGE_RATE, *PRMS_JNL_CHANGE_EXCHANGE_RATE;
// end of KSK 2009.3.30

// [#529] AU AIREAT 2009.06.02
typedef struct _RMS_JNL_USER_CANCEL_
{
	NUINT8		Action[80];
} RMS_JNL_USER_CANCEL, *PRMS_JNL_USER_CANCEL;
// end of [#529]

//[#610] SOOK 2009.12.21 Parameter Change Journal 저장 
//#if SUPPORT_CHANGE_PARAMETER_JNL    

typedef struct _RMS_JNL_CHANGE_PARAMETER_
{
	NUINT8		User;
	NUINT8		Parameter[41];
	NUINT8		Value[512];  //OLD VALUE + NEW VALUE

} RMS_JNL_CHANGE_PARAMETER, *PRMS_JNL_CHANGE_PARAMETER;

//#endif
//end of [#610]

// [#2279] Justin Uploading EMV Data to Moniview
typedef struct _RMS_JNL_TRXEMVDATA_
{
	NUINT8		EMVData[300]; // Moniview Table Size (Big enough for EMV Journal Data)
} RMS_JNL_TRXEMVDATA, *PRMS_JNL_TRXEMVDATA;
// End of [#2279]

// LibertyX Journal
typedef struct _RMS_JNL_LIBERTYX_
{
	NUINT8		LocationId[16];
	NUINT8		RoutingId[8];
	NUINT8		APIUrl[32];
	NUINT8		TLSEnabled[6];
	NUINT8		PaymentId[16];
	NUINT8		Token[16];
	NUINT8		SourceAmount[8];
	NUINT8		AuthStatus[16];
	NUINT8		AuthMessage[16];
	NUINT8		AuthCode[16];
	NUINT8		Last4[5];
	NUINT8		NameOnCard[32];
	NUINT8		Error[16];
} RMS_JNL_LIBERTYX, *PRMS_JNL_LIBERTYX;

// LibertyX Dispense Journal
typedef struct _RMS_JNL_LIBERTYX_DISPENSE_
{
	NUINT8		LocationId[16];
	NUINT8		RoutingId[8];
	NUINT8		APIUrl[32];
	NUINT8		TLSEnabled[6];
	NUINT8		Token[16];
	NUINT8		Nonce[36];
	NUINT8		Amount[8];
	NUINT8		AuthResult[16];
	NUINT8		AuthMessage[32];
	NUINT8		ExecuteResult[16];
	NUINT8		ExecuteMessage[32];
	NUINT8		ExecuteMessageCustomer[64];
	NUINT8		SignatureA[128];
	NUINT8		SignatureB[128];
	NUINT8		SignatureC[128];
	NUINT8		CertACN[128];
	NUINT8		CertBCN[128];
	NUINT8		CertCCN[128];
	NUINT8		RequesetedAmount[8];
	NUINT8		DispensedAmount[8];
	NUINT8		Error[8];
	NUINT8		ErrorMessage[32];
	NUINT8		ErrorData[128];
} RMS_JNL_LIBERTYX_DISPENSE, *PRMS_JNL_LIBERTYX_DISPENSE;

// B4U Journal
typedef struct _RMS_JNL_B4U_
{
	NUINT8		TransactionTime[14];	// 20200821072156 (YYYYMMDDHHMMSS format)
	NUINT8		TerminalNumber[16];     // NHATM01
	NUINT8		SequenceNumber[4];		// 0007
	NUINT8		RequestAmount[8];		// 00002000 ($20)
	NUINT8		DispensedAmount[8];		// 00002000 ($20)
	NUINT8		TransactionResult[2];				// 01 (success), 02 (error-cancelled), 03 ...
	NUINT8		TransactionHash[84];	// 61b6smmosklsjef8kj4qjk9s92j3kj1901n1m23j45j1l
	NUINT8		BtcCharged[16];			// 0.00170000000000 (0.0017)
} RMS_JNL_B4U, *PRMS_JNL_B4U;

// DigitalMint Journal
typedef struct _RMS_JNL_DIGITALMINT_
{
    NUINT8      TransactionTime[19];    // 09/26/2022 15:09:46 (MM/DD/YYYY HH:mm:ss format)
    NUINT8      TerminalNumber[16];     // NHATM01
    NUINT8      SessionId[36];          // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX (GUID format)
    NUINT8      TransactionId[36];      // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX (GUID format)
    NUINT8      AccountId[36];          // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX (GUID format)
    NUINT8      APIUrl[32];             // api.staging.digital-mint.com
    NUINT8      SelectedCoin[10];       // TBTCUSD
    NUINT8      TransactionMode[1];     // 1 (DEBIT), 2 (CASH), any other value (UNKNOWN)
    NUINT8      RequestAmount[8];       // 00002000 ($20.00)
    NUINT8      Error[7];               // 0000000
} RMS_JNL_DIGITALMINT, *PRMS_JNL_DIGITALMINT;

// [#620] US KSK 2010.02.22
typedef	struct _RMS_READ_ENHANCED_COUPON_ 
{
	RMS_COMM	Header;				//   4 byte
	
	NUINT16		FrameNumber;		//	 6 byte
	
	NUINT16		dwYear;				//   8 byte
	NUINT16		dwMonth;			//  10 byte
	NUINT16		dwDay;				//  12 byte
	NUINT16		dwHour;				//  14 byte
	NUINT16		dwMinute;			//  16 byte
	NUINT16		dwSecond;			//  18 byte
	
	NUINT8		MachineType;
	NUINT8		SerialNumber[11];
	
	NUINT16		CST1Denomination;	//  32. byte
	NUINT16		CST1BillCount;		//  34. byte
	NUINT16		CST2Denomination;	//  36. byte
	NUINT16		CST2BillCount;		//  38. byte
	NUINT16		CST3Denomination;	//  40. byte
	NUINT16		CST3BillCount;		//  42. byte
	NUINT16		CST4Denomination;	//  44. byte
	NUINT16		CST4BillCount;		//  46. byte
	
	NUINT16		JournalCount;		//  48. byte
	NUINT8		ATMStatus;			//  49. byte
	
	NUINT8		ErrorCode[8];		//  57. byte
	
	NUINT8		EnhancedCouponData[512];	// 569. byte : 
} RMS_READ_ENHANCED_COUPON, *PRMS_READ_ENHANCED_COUPON;

// sizeof(RMS_READ_ENHANCED_COUPON) - Length
#define LEN_RMS_ENHANCEDCOUPON_READ_DATA	(sizeof(RMS_READ_ENHANCED_COUPON) - 2)

typedef	struct _RMS_WRITE_ENHANCED_COUPON_ 
{
	RMS_COMM	Header;				//   4 byte
	
	NUINT16		FrameNumber;		//   6 byte
	NUINT8		TerminalNumber[16];	//  22 byte
	NUINT8		RMSPassword[7];		//  29 byte
	
	NUINT8		EnhancedCouponData[512];		// 541 byte
	
} RMS_WRITE_ENHANCED_COUPON, *PRMS_WRITE_ENHANCED_COUPON;

// sizeof(RMS_WRITE_ENHANCED_COUPON) - Length
#define LEN_RMS_WRITE_ENHANCED_COUPON	(sizeof(RMS_WRITE_ENHANCED_COUPON) - 2)

typedef	struct _RMS_ENHANCED_COUPON_ 
{
	NUINT8		EnhancedCouponUse;
	NUINT8		AdvCouponEnable;		// 기존 Coupon 정보
	NUINT8		AdvCouponMsg1[2][41];
	NUINT8		AdvCouponMsg2[2][41];
	NUINT8		AdvCouponMsg3[2][41];
	NUINT8		AdvCouponMsg4[2][41];
	NUINT8		AdvCouponMsg5[2][41];
	NUINT8		AdvCouponMsg6[2][41];
	NUINT8		EnhancedCouponCutOption;
	NUINT8		EnhancedCouponOption[6];
	NUINT8		EnhancedCouponPrintStartTime[6];
	NUINT8		EnhancedCouponPrintEndTime[6];
	NUINT8		EnhancedCouponText[6][16][41];
} RMS_ENHANCED_COUPON, *PRMS_ENHANCED_COUPON;
// end of [#620]

#define LEN_RMS_ENHANCED_COUPON_DATA	(sizeof(RMS_ENHANCED_COUPON))	// header가 없어서 2를 빼지 않고 계산한다.


//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
#if (AU_VERSION)

typedef struct _RMS_GO_COMMAND_
{
	RMS_COMM	Header;				//   4 byte
	
	NUINT16		FrameNumber;		//   6 byte
	NUINT8		TerminalNumber[16];	//  22 byte
	NUINT8		RMSPassword[7];		//  29 byte
	
	NUINT16		FileCount;
	NUINT16		HashValueLength;
	NUINT8		HashValue[2048];		// 541 byte
} RMS_GO_COMMAND, *PRMS_GO_COMMAND;

typedef struct _RMS_CERTIFICATE_
{
	RMS_COMM	Header;				//   4 byte
	
	NUINT16		FrameNumber;		//   6 byte
	NUINT8		TerminalNumber[16];	//  22 byte
	NUINT8		RMSPassword[7];		//  29 byte
	
	NUINT16		CertifiateLength;
	NUINT8		Certifiate[2048];		// 541 byte
} RMS_CERTIFICATE, *PRMS_CERTIFICATE;

#endif
//end of [#635]

#pragma pack()

#endif //__RMS_RETURN_CODE_H__
