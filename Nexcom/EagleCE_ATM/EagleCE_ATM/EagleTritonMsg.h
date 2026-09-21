#ifndef __EAGLE_TRITON_MSG_H__
#define __EAGLE_TRITON_MSG_H__

/** **************************************************
*	@file		EagleTritonMsg.h
*	@brief		Triton Message Process Class, base on Triton Spec 5.22
*	@date		2017-08-23
*	@version	1.0.1
*****************************************************/

#include "../../EagleCE_Framework/EagleCE_Framework/EagleDefine.h"

#define MISC_REQ_TRAN		0
#define MISC_REQ_CONFIG		1
#define MISC_REQ_TOTAL		2
#define MISC_REQ_REVERSAL	3

#define TRITON_EMV_FALLBACK_CODE			_T("9F390192")

/** *********************************************************
*	@brief Triton response code table
************************************************************/
typedef	struct	__RSP_TBL
{
	BYTE	CODE[4];
}_RSP_TBL;

const _RSP_TBL	TRITON_RSP_TBL[40] = {
	"001", "002", "003", "004", "005",
	"006", "007", "008", "009", "010",
	"011", "012", "013", "014", "015",
	"016", "017", "018", "019", "020",
	"021", "022", "023", "024", "025",
	"026", "027", "028", "029", "030",
	"031", "032", "033", "034", "035",
	"036", "111", "222", "223", "037" };


// Define Miscellaneous Data Type
#define	FD_SRCH_AMT			'!'			/**<	Misc Data Type - Surcharge Amount	*/
#define FD_EXT_SEQ_NUM		'#'			/**<	Extended sequence number - not used	*/
#define FD_ACCNT_NUM		'&'	//tran response
#define FD_MAC_EN_DIS		'/'	//response
#define FD_CHG_PIN_BLOCK	'@'	//tran request
#define FD_ENCRYPT_MODE		'['	//Triple-DES capable
#define	FD_MAC_BLCK			'^'	//request/response
#define FD_PIN_COMM_KEY_2	'{'	//PIN Communication key part 2
#define FD_PIN_COMM_KEY_1	'~'	//PIN Communication key part 1
#define FD_MAC_COMM_KEY_2	'}'	//MAC Working Key part 2
#define	FD_SUPPLEMENT_BAL	'b'	//response
#define	FD_SCHDL_BAL		'c'	//configure the schedule balance time.(day close)
#define FD_MSG				'd'
  #define FD_MSG_LOCATION	's'//response-location(store) message (ds)
  #define FD_MSG_ATTRACT	'w'//response-attract(welcome) message (dw)
  #define FD_MSG_FAREWELL	'e'//response-farewell(exit) message (de)
  #define FD_MSG_MARKETING	'm'//response-marketing message (dm)
#define FD_EXTND_AMT		'e'	//response-Extended length amount 
#define FD_TRACK3_INFO		'g'	//request 
#define	FD_HRTBT_FRQNY		'h'	//response-configure the heartbeat frequency.
#define	FD_PAN_TRACK3		'i'	//response-Primary Account Number from track3.
//#define	FD_ISSUER_FEE_CREDIT'k'		//response-Display shurcharge screen.
//#define	FD_EXT_TRAN_CD		'l'		//request/response - money market & brokerage account 추가시 구현할 것.
#define	FD_MAC_COMM_KEY_1	'm'		//response-configure MAC Working Key part 1(tran/cfg)
#define	FD_RSN_RVRSL		'n'		//request maybe-reason for reversal
//	#define	FD_CRDT_ADVNCE		'o'		//quasicash credit tran???
#define	FD_PRT_TXT			'p'		//response-printing text field at the end of tran receipt,
//error receipt or host totals receipt.
#define	FD_LOCAL_TIME		't'		//response-configure local time.
#define	FD_EXT_FIELD_U		'u'
#define	FD_LOCAL_DATE		"ua"	//response-configure local date.
#define	FD_CRC_CHKSUM		"ub"	//req/response??
//	#define	FD_EMV_DT_BLCK		"ud"	//Encoded EMV data block
#define	FD_DT_WK_LEFT		"ue"	//response-DATA Working Key Left Block
#define	FD_DT_WK_RIGHT		"uf"	//response-DATA Working Key Right Block
#define	FD_HST_TRACE_NUM	"ug"	//request??response-Host transaction trace number

//	#define	FD_EMV_UNTAG_DT_BLCK"uh"	//encoded EMV untagged-data block
//	#define	FD_MOTOR_CR_CMD		"ui"	//not need - dip type
//	#define	FD_VAS_TERM_ID		"va"	//Value Added Service Terminal ID
//	#define	FD_VAS_POST_CD		"vb"	//Postal code of Terminal for Value Added Service
//	#define	FD_VAS_LOYALTY_CRD_INFO		"vc"	//Loyalty card information for Value Added Service
//	#define	FD_VAS_RESP			"vd"	//Postal code of Terminal for Value Added Service
#define	FD_MAX_WITH_AMT		'w'		//response-configure the maximum withdrawal amount.
//	#define	FD_TELCO_SERIAL_NO	"xa" ~y~z : 추후에 추가할 것.
//	#define FD_XXX				"??"

#define FID2_a		0x10000000
#define FID2_b		0x20000000
#define FID2_c		0x40000000
#define FID2_d		0x80000000
#define FID2_e		0x01000000
#define FID2_f		0x02000000
#define FID2_g		0x04000000
#define FID2_h		0x08000000
#define FID2_i		0x00100000
#define FID2_j		0x00200000
#define FID2_k		0x00400000
#define FID2_l		0x00800000
#define FID2_m		0x00010000
#define FID2_n		0x00020000
#define FID2_21h	0x00040000
#define FID2_p		0x00080000
#define FID2_q		0x00001000
#define FID2_r		0x00002000
#define FID2_s		0x00004000
#define FID2_t		0x00008000
#define FID2_u		0x00000100
#define FID2_v		0x00000200
#define FID2_w		0x00000400
#define FID2_x		0x00000800

// TRITON MISCELLANEOUS FIELD ID TABLE
const UINT chMisc_Requests[4][34] = 
	{	{'#','&','@','^','e','g','i','l','v'|FID2_a|FID2_b|FID2_c|FID2_d
										,'x'|FID2_b|FID2_d|FID2_g|FID2_r|FID2_u
										,'y'|FID2_a|FID2_b|FID2_c|FID2_d
										,'z'|FID2_a|FID2_b|FID2_c
										,'u'|FID2_b|FID2_d|FID2_h
										},			// Transaction Message
		
		{'[','u'|FID2_b},							// Config. Message 

		{'[','^','o','v'|FID2_a
					,'x'|FID2_e|FID2_s|FID2_v
					,'y'|FID2_e|FID2_f
					,'z'|FID2_k|FID2_m
					,'u'|FID2_b},					// Host Total Message

		{'#','^','g','i','k','n'
					,'v'|FID2_a|FID2_b
					,'x'|FID2_u
					,'u'|FID2_b|FID2_d|FID2_h
					},								// Reversal Message
	};

#define MISC_RESP_TRAN		0
#define MISC_RESP_CONFIG	1
#define MISC_RESP_TOTAL		2
#define MISC_RESP_REVERSAL	3

const UINT chMisc_Responses[4][34]= 
	{	{'!','#','&','/','^','{','~','}','b','c','e','k','l','m','p','t'
			,'d'|FID2_e|FID2_m|FID2_s|FID2_w
			,'u'|FID2_a|FID2_b|FID2_d|FID2_e|FID2_f|FID2_g|FID2_h|FID2_21h//|FID2_i
			,'v'|FID2_d
			,'w'
			,'x'|FID2_a|FID2_b|FID2_c|FID2_d|FID2_f|FID2_g|FID2_h|FID2_i|FID2_n||FID2_p|FID2_q|FID2_t
			,'z'|FID2_c|FID2_d|FID2_e|FID2_j|FID2_n
			},											// Transaction Response Message

		{'!','/','{','~','}','m'
			,'u'|FID2_b|FID2_e|FID2_f|FID2_21h
			,'x'|FID2_q|FID2_t
			,'z'|FID2_f|FID2_g|FID2_h|FID2_i|FID2_n},	// Configure Response Message

		{'^','o','p'
			,'u'|FID2_b|FID2_g
			,'x'|FID2_e|FID2_s|FID2_v
			,'y'|FID2_e|FID2_f
			,'z'|FID2_k|FID2_m},						// Host Total Response Message

		{'#','^' 
			,'u'|FID2_b|FID2_g},						// Reversal Response Message
	};

//--------------------------------------------------------
/**
  @brief Triton Message Common Header structure
*/
//--------------------------------------------------------
typedef struct tag_Triton_CommHeader {

	void Clear(void);

	CString m_strCommsHeaderID_8;
	CString m_strTerminalID_2;
	CString m_strSoftwareVersionNo_2;
	CString m_strEncryptionModeFlag_1;
	CString m_strInformationHeader_7;
	CString m_strTerminalID_15;
} TRITON_COMMHEADERS, *PTRITON_COMMHEADERS;

//--------------------------------------------------------
/**
 @brief Triton Message Transaction Request Structure
*/
//--------------------------------------------------------
typedef struct tag_Triton_TranRequestMsg {

	void Clear(void);

	CString m_strTransactionCode_2;
	CString m_strSequenceNumber_4;
	CString m_strTrack2_37;
	CString m_strAmount1_8;	   
	CString m_strAmount2_8;    
	CString m_strPINBlock_16;
	CString m_strMiscellaneous1;
	CString m_strMiscellaneous2;
	CString m_strStatusMonintoring;
	CString m_strMiscellaneous3;
}TRITON_TRANREQMSG, *PTRITON_TRANREQMSG;

//--------------------------------------------------------
/**
 @brief Triton Message Transaction Response Structure
*/
//--------------------------------------------------------
typedef struct tag_Triton_TranResponseMsg {
	
	void Clear(void);

	CString m_strInformation_Header_12;
	CString m_strMulti_Block_Indicator_1;
	CString m_strTerminalID_15;
	CString m_strTransactionCode_2;
	CString m_strSequenceNumber_4;
	CString m_strResponseCode_3;
	CString m_strAuthorizationNum_8;
	CString m_strTransactionDate_6;
	CString m_strTransactionTime_6;
	CString m_strBusinessDate_6;
	CString m_strAmount1_8;
	CString m_strAmount2_8;
	CString m_strAvailableBalance_8;
	CString m_strAmount1_Ext12;
	CString m_strEMVTag;
}TRITON_TRANRESPMSG, *PTRITON_TRANRESPMSG;

// Triton Reversal Request, Response Message
typedef struct tag_Triton_ReversalRequestMsg {
	
	void Clear(void);

	CString m_strTransactionCode_2;
	CString m_strSequenceNumber_4;
	CString m_strTrack2_37;
	CString m_strAmount1_8;
	CString m_strAmount2_8;
	CString m_strAmount3_8;
	CString m_strStatusMonintoring;
	CString m_strMiscellaneousX_V;
}TRITON_REVERSALREQMSG, *PTRITON_REVERSALREQMSG;

typedef struct tag_Triton_ReversalResponseMsg {
	
	void Clear(void);

	CString m_strInformation_Header_12;
	CString m_strTerminalID_15;
	CString m_strTransactionCode_2;
	CString m_strSequenceNumber_4;
	CString m_strResponseCode_3;	
}TRITON_REVERSALRESPMSG, *PTRITON_REVERSALRESPMSG;

// Triton Configuration Request, Response Message
typedef struct tag_Triton_ConfigRequestMsg {
	
	void Clear(void);

	CString m_strTransactionCode_2;
	CString m_strStatusMonitoringField_X;
	CString m_strMiscellaneousX_V;
} TRITON_CONFIGREQMSG, *PTRITON_CONFIGREQMSG;

typedef struct tag_Triton_ConfigResponseMsg {
	
	void Clear(void);

	CString m_strInformation_Header_12;
	CString m_strTerminalID_15;
	CString m_strTransactionCode_2;
	CString m_strFieldIDCode1_1;
	CString m_strEncryptedPINKey_16;
	CString m_strFieldIDCode2_1;
	CString m_strEncryptedPINKey2_16;
	CString m_strFieldIDCode3_1;
	CString m_strSurchargeAmount_8;
	CString m_strFieldIDCode4_1;
	CString m_strEncryptedPINKey4_16;
	CString m_strFieldIDCode5_1;
	CString m_strEncryptedPINKey5_16;
} TRITON_CONFIGRESPMSG,*PTRITON_CONFIGRESPMSG;

// Triton Host Total Request, Response Message
typedef struct tag_Triton_HostTotalsRequestMsg {
	
	void Clear(void);

	CString m_strTransactionCode_2;
	CString m_strStatusMonintoring;
	CString m_strTotalWithdrawals_4;
	CString m_strTotalInquiries_4;
	CString m_strTotalTransfers_4;
	CString m_strSettlement_12;
	CString m_strMiscellaneousX_V;
}TRITON_HOSTTOTALSREQMSG, *PTRITON_HOSTTOTALSREQMSG;

typedef struct tag_Triton_HostTotalResponseMsg {

	void Clear(void);

	CString m_strInformation_Header_12;
	CString m_strTerminalID_15;
	CString m_strTransactionCode_2;
	CString m_strBusinessDate_6;
	CString m_strNumberOfWithdrawals_4;
	CString m_strNumberOfInquiries_4;
	CString m_strNumberOfTransfers_4;
	CString m_strSettlement_8;
	CString m_strSettlement_Ext12;
}TRITON_HOSTTOTALSRESPMSG, *PTRITON_HOSTTOTALSRESPMSG;

// Triton Status Monitoring Message
typedef struct tag_Triton_StatusMonitoring {
	
	void Clear(void);

	CString m_strProgramVersionNo_10;
	CString m_strTableVersionNo_10;
	CString m_strFirmwareVersionNo_10;
	CString m_strAlarm_ChestDoorOpen_1;
	CString m_strAlarm_TopDoorOpen_1;
	CString m_strAlarm_SupervisonActive_1;
	CString m_strReceiptPrinter_PaperStatus_1;
	CString m_strReceiptPrinter_RibbonStatus_1;
	CString m_strJournalPrinter_PaperStatus_1;
	CString m_strJournalPrinter_RibbonStatus_1;
	CString m_strNoteStatus_Dispenser_1;
	CString m_strReceiptPrinter_1;
	CString m_strJournalPrinter_1;
	CString m_strDispenser_1;
	CString m_strCommunicationsSystem_1;
	CString m_strCardReader_1;
	CString m_strCardsRetained_3;
	CString m_strElectronicsSystem_2;
	CString m_strCurrentErrorCode_3;
	CString m_strCommunicatioinsFailures_3;
	CString m_strCassetteA_Denomination_3;
	CString m_strCassetteA_NotesLoaded_4;
	CString m_strCassetteA_NotesDispensed_4;
	CString m_strCassetteA_RejectEvents_3;
	CString m_strCassetteB_Denomination_3;
	CString m_strCassetteB_NotesLoaded_4;
	CString m_strCassetteB_NotesDispensed_4;
	CString m_strCassetteB_RejectEvents_3;
	CString m_strCassetteC_Denomination_3;
	CString m_strCassetteC_NotesLoaded_4;
	CString m_strCassetteC_NotesDispensed_4;
	CString m_strCassetteC_RejectEvents_3;
	CString m_strCassetteD_Denomination_3;
	CString m_strCassetteD_NotesLoaded_4;
	CString m_strCassetteD_NotesDispensed_4;
	CString m_strCassetteD_RejectEvents_3;
	CString m_strTotalNotesPurged_3;
} TRITON_STATUS_MONITOR, *PTRITON_STATUS_MONITOR;


class CEagleTritonMsg
{
public:
	// 인스턴스
	static CEagleTritonMsg* m_pInstance;

	// 생성자, 소멸자.
	CEagleTritonMsg();
	virtual ~CEagleTritonMsg();

	// 인스턴스 생성
	static void CreateInstance();
	// 인스턴스 취득
	static CEagleTritonMsg* GetInstance();
	// 인스턴스 해제
	static void ReleaseInstance();


public:
	TRITON_COMMHEADERS			m_sTriton_CommHeader;			// Triton Common Header
	TRITON_STATUS_MONITOR		m_sTriton_Status_Field;			// Triton Status Monitoring

	TRITON_CONFIGREQMSG			m_sTriton_ConfigReq;			// Triton Configuration Request Msg
	TRITON_CONFIGRESPMSG		m_sTriton_ConfigResp;			// Triton Configuration Response Msg

	TRITON_TRANREQMSG			m_sTriton_TranReq;				// Triton Transaction Request Msg
	TRITON_TRANRESPMSG			m_sTriton_TranResp;				// Triton Transaction Response Msg

	TRITON_REVERSALREQMSG		m_sTriton_ReversalReq;			// Triton Reversal Request Msg
	TRITON_REVERSALRESPMSG		m_sTriton_ReversalResp;			// Triton Reversal Response Msg

	TRITON_HOSTTOTALSREQMSG		m_sTriton_TotalReq;				// Triton Total Request Msg
	TRITON_HOSTTOTALSRESPMSG	m_sTriton_TotalResp;			// Triton Total Response Msg

	BYTE		*m_pSendBuffer;
	BYTE		*m_pRecvBuffer;

	CString		m_strSendData;
	CString		m_strRecvData;

	int			m_nSendLen;
	int			m_nRecvLen;

	BOOL		m_bFirstKeyReceived;
	BOOL		m_bSecondKeyReceived;

	CString			m_strCrcResult_16;
	CStringArray	m_strarrPrintTextData;	// 'p' filed text print
	CStringArray	m_strArrRecvData;

public:
	void		InitializeVariable();
	int			Triton_MakeHostMsg(int nTranType);
	int			Triton_AnalHostData(int nTranType);
	BOOL		Triton_IsReversalCondition();

	int			Triton_Save_EMV_Data(int nIssuerScriptLen = 0);

protected:
	int			Triton_MakeHeader();
	int			Triton_MakeBody(int nTranType);

	int			Triton_MakeTransMsg(int nTranType);
	int			Triton_MakeReversalMsg();
	int			Triton_MakeTotalMsg(int nTranType);
	int			Triton_MakeConfigMsg();

	int			Triton_Make_MiscellaneousField(int nTranType);
	int			Triton_Make_StatusMonitoringField();

	int			Triton_Make_EMV_RequestData(int nTranType);

	int			Triton_AnalConfigMsg(int nIndex, int nTranType);
	int			Triton_AnalTranMsg(int nIndex, int nTranType);
	int			Triton_AnalReversalMsg(int nIndex);
	int			Triton_AnalTotalMsg(int nIndex, int nTranType);

	int			Triton_AnalMiscellaneousField(int nCurrentIndex, int nTranType);
	int			Triton_Anal_EMV_ResponseData(CString strEMVResponseData);

	int			Triton_CheckResponseCode(CString strResponseCode);
	UINT		Triton_GetFID(char fid2);

};

#endif __EAGLE_TRITON_MSG_H__