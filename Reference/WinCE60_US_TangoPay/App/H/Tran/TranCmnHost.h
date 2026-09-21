/////////////////////////////////////////////////////////////////////////////
#if !defined(_TranCmnHost_H_)
#define _TranCmnHost_H_

/////////////////////////////////////////////////////////////////////////////
//	Host Normal Message Send
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// KSK 2010.02.02 Standard1 Message Structure
///////////////////////////////////////////////////////////////////////////////

typedef struct tagSTD1_CommonReqMsgHeader	// Host STD1 Request Message Header
{
	CString	RecordFormat_1;				// Record Format (H)
	CString	ApplicationType_1;			// Application Type (= '0')
	CString	MessageDelimiter_1;			// MessageDelimiter (= '.')
	CString	BankID_6;					// Bank ID
	CString	TerminalID_8;				// Terminal ID
} STD1_CommonReqMsgHeader;

// 1-2.Response Message Header
typedef struct tagSTD1_CommonRespMsgHeader	// Host STD1 Response Message Header
{
	CString	RecordFormat_1;				// Record Format (H)
	CString	ApplicationType_1;			// Application Type (= '0')
	CString	MessageDelimiter_1;			// MessageDelimiter (= '.')
	CString	BankID_6;					// Bank ID (6)
	CString	TerminalID_8;				// Terminal ID
} STD1_CommonRespMsgHeader;

typedef struct tagSTD1_ConfigRequest	// Host STD1 Configuration Request
{
	CString RequestType_2;				// Request Type
	CString	KeyMode_1;					// Key Mode (0 ~ 11)
	CString	UKSerialNoA_8;				// Unique Serial Number A
	CString	UKSerialNoB_8;				// Unique Serial Number B
	CString SubRequestType_2;			// [#560] NH KSK 2009.8.20
} STD1_ConfigRequest;

typedef struct tagSTD1_ConfigResponse		// Host STD1 Open Response
{
	CString	ResponseType_2;				// Response Type
	CString	LocalDate_8;				// Local Date
	CString	LocalTime_6;				// Local Time
	CString	HealthMsgTimerVal_6;		// Healthy Message Timer Value
	CString	WorkingKey_V;				// Working Key
	CString	SurchargeAmount_V;			// Surcharge Amount	 (ddcc)		** Variable
	CString	ConfigInitFlag_1;			// Configuration Initiator Flag
	CString WorkingKey2_V;				// 2번째 암호화키
	CString WorkingKey3_V;				// 3번째 암호화키
	CString WorkingKey4_V;				// 4번째 암호화키
	CString WorkingKey5_V;				// 5번째 암호화키 [#565] NH KSK 2009.8.20
	CString WorkingKey6_V;				// 6번째 암호화키 [#565] NH KSK 2009.8.20 TDES, TMAC Option2인 경우 - 사용안함
	CString WorkingKey7_V;				// 7번째 암호화키 [#565] NH KSK 2009.8.20 TDES, TMAC Option2인 경우 - 사용안함
	CString	SubResponseType_2;			// [#560] NH KSK 2009.8.20 EXTENDED CONFIGURATION 지원을 위해 추가
} STD1_ConfigResponse;

typedef struct tagSTD1_ExtendConfigRequest		// Host STD1 Open Request
{
	CString RequestType_2;				// Request Type
	CString SubRequestType_2;			// [#560] NH KSK 2009.8.20
} STD1_ExtendConfigRequest;

typedef struct tagSTD1_ExtendConfigResponse		// Host STD1 Open Response
{
	CString	ResponseType_2;				// Response Type
	CString	SubResponseType_2;			// [#560] NH KSK 2009.8.20 EXTENDED CONFIGURATION 지원을 위해 추가
} STD1_ExtendConfigResponse;

typedef struct tagSTD1_TranRequest		// Host STD1 Request Main
{
	CString RequestType_2;				// Request Type
	CString	RandomMac_8;				// Random Mac				*2
	CString	OperationCode_2;			// Operation Code
	CString	SourceAccount_2;			// Source Account
	CString	DestinationAccount_2;		// Destination Account
	CString	TransactionSequenceNo_4;	// Transaction Sequence Number
	CString	Track1Data_79;				// ISO Track 1 Data
	CString	Track2Data_40;				// ISO Track 2 Data
	CString	Track3Data_106;				// ISO Track 3 Data
	CString	PINBuffer_16;				// PIN Buffer 16
	CString	TransactionAmount_V;		// Variable Size	"ddddddcc"
	CString	SurchargeAmount_V;			// Surcharge Amount	"ddcc"
	CString	SurchargeEnableFlag_1;		// Surcharge Enable Flag
	CString	AuthrizationNumber_9;		// Authorization Number (Check Cashing Customer Number) [#369] [US] KSK 2008.7.17
	CString	PINNumber_6;				// PIN Number (Check Cashing Customer Number) [#369] [US] KSK 2008.7.17
	CString	MACData_8;					// MAC Data					*2
} STD1_TranRequest;

// 2-2. Transaction Response Message Main
typedef struct tagSTD1_TranResponse		// Host STD1 Transaction Response
{
	CString	ResponseType_2;				// Response Type
	CString	RandomMac_8;				// Random Mac				*2
	CString	TranSequenceNo_4;			// Transaction Sequence Number
	CString	ResponseCode_2;				// Response Code
	CString	LocalDate_8;				// Local Transaction Date
	CString	LocalTime_6;				// Local Transaction Time
	CString	RetrievalRefNo_12;			// Retrieval Reference Number
	CString	SystemTraceAuditNo_6;		// System Trace Audit Number
	CString	NetworkIDCode_2;			// Network ID Code
	CString	SettlementDate_8;			// Settlement Date
	CString	AccountBalance_V;			// Account Balance,	 (ddddddcc)	** Variable
	CString	AvailableBalance_V;			// Available Balance,(ddddddcc)	** Variable
	CString	SurchargeAmount_V;			// Surcharge Amount	 (ddcc)		** Variable
	CString	AuthResponseText_V;			// Authorization Response Text	** Variable
	CString	ConfigRequestInitiator_2;	// Configuration Request Initiator
	CString	MACData_8;					// MAC Data
} STD1_TranResponse;

typedef struct tagSTD1_ReversalRequest	// Host STD1 Reversal Request
{
	CString RequestType_2;				// Request Type
	CString	RandomMac_8;				// Random Mac				*2
	CString	LocalTranDate_8;			// Local Transaction Date
	CString	LocalTranTime_6;			// Local Transaction Time
	CString	RetrievalRefNo_12;			// Retrieval Reference Number
	CString	RequestedDollarAmt_V;		// Requested Dollar Amount		Variable
	CString	DispensedDollarAmt_V;		// Dispensed Dollar Amount		Variable
	CString	SurchargeAmount_4;			// Surcharge Amount	"ddcc"
	CString	SurchargeEnableFlag_1;		// Surcharge Enable Flag
	CString	MACData_8;					// MAC Data					*2
} STD1_ReversalRequest;

typedef struct tagSTD1_ReversalResponse	// Host STD1 Reversal Response
{
	CString	ResponseType_2;				// Response Type
	CString	RandomMac_8;				// Random Mac				*2
	CString	ConfigRequestInitiator_2;	// Configuration Request Initiator
	CString	MACData_8;					// MAC Data
} STD1_ReversalResponse;

typedef struct tagSTD1_TotalRequest	// Host STD1 Total Request Main
{
	CString RequestType_2;				// Request Type
	CString	ResetHostTotalFlag_1;		// Reset Host Total Flag
} STD1_TotalRequest;

// 5-2. Total Response Message Main
typedef struct tagSTD1_TotalResponse	// Host STD1 Reversal Response
{
	CString	ResponseType_2;				// Response Type = '87'
	CString NoOfCashWiths_4;			// No of Cash Withdrawals
	CString NoOfTransfers_4;			// No of Transfers
	CString NoOfInquiries_4;			// No of Inquiries
	CString NoOfNonCashWiths_4;			// No of None Cash Withdrawals
	CString TotalDispenseAmt_V;			// Total Dispensed Amount
	CString TotalNonCashDispAmt_V;		// Total Non-Cash Dispensed Amount
	CString TotalSurchargeAmt_V;		// Total Surcharge Amount
	CString ConfigRequestInitiator_2;	// Configuration Initiator
} STD1_TotalResponse;

typedef struct tagSTD1_HealthRequest	// Host STD1 Healthy Request
{
	CString RequestType_2;				// Request Type
	CString	LocalDate_8;				// Local Transaction Date
	CString	LocalTime_6;				// Local Transaction Time
	CString	BillCount1_V;				// Bill Count 1
	CString	BillCount2_V;				// Bill Count 2
	CString	ModeType_1;					// Mode Type
	CString ErrorCode_V;				// Error Code
	CString NewJournalCount_V;			// New Journal Count
	CString	ProgramVersion;				// Extended HealthCheck Message (Tranax Format임)
	CString CDUVersion;					// Extended HealthCheck Message (Tranax Format임)
	CString RPUVersion;					// Extended HealthCheck Message (Tranax Format임)
	CString EPPVersion;					// Extended HealthCheck Message (Tranax Format임)
	CString TopDoorOpen;				// Extended HealthCheck Message (Tranax Format임)
	CString VaultDoorOpen;				// Extended HealthCheck Message (Tranax Format임)
	CString DispenserStatus;			// Extended HealthCheck Message (Tranax Format임)
	CString ReceiptPrinterStatus;		// Extended HealthCheck Message (Tranax Format임)
	CString ReceiptPrinterPaperStatus;	// Extended HealthCheck Message (Tranax Format임)
	CString CardReaderStatus;			// Extended HealthCheck Message (Tranax Format임)
	CString CSTDenomination[4];			// Extended HealthCheck Message (Tranax Format임)
	CString CSTLoaded[4];				// Extended HealthCheck Message (Tranax Format임)
	CString CSTDispensed[4];			// Extended HealthCheck Message (Tranax Format임)
	CString CSTRejectEvents[4];			// Extended HealthCheck Message (Tranax Format임)
	CString CSTBillCout[4];				// Extended HealthCheck Message (Tranax Format임)
} STD1_HealthRequest;

typedef struct tagSTD1_HealthResponse	// Host STD1 Healthy Response
{
	CString	ResponseType_2;				// Response Type = '90'
	CString	ConfigRequestInitiator_2;	// Configuration Request Initiator
} STD1_HealthResponse;

// [#2150] US Justin 2012.09.27 Add Dynamic Flow
typedef struct tagSTD1_DynamicFlowRequest	// Host STD1 Dynamic Flow Request
{
	CString TransactionType;			// Transaction Type
	CString RandomMac_8;				// [#2362] AU KSK 2015.09.03
	CString R1_TerminalCurrency;		// at
	CString R1_Track2Data;				// cb
	CString R1_AID;						// cd		// [#2182] NH Justin 2013.03.21 Implement NHD1.6
	CString	R2_Surcharge_Withdrawal;	// fc
	CString	R2_TransactionAmount;		// ta
	CString	R2_CurrencyCode;			// tc
	CString R2_AccountType;				// tt		// [#RWC6-223] US William 2020-10-20 Add account type to DF 2nd request
} STD1_DynamicFlowRequest;

typedef struct tagSTD1_DynamicFlowResponse	// Host STD1 Dynamic Flow Response
{
	CString RandomMac_8;					// [#2362] AU KSK 2015.09.03
	CString PartnerID;						// i
	CString	R1_TransactionCapacity;			// ac
	CString	R1_Surcharge_Withdrawal;		// fc
	CString	R1_Surcharge_BalanceInquiry;	// fi
	CString	R1_Surcharge_PinChange;			// fp
	CString	R1_Surcharge_MPT;				// ft
	CString R1_DCC_ExchangeRate;			// er		// [#2182] NH Justin 2013.03.21 Implement NHD1.6
	CString R1_DCC_BaseTargetCurrency;		// ec		// [#2182] NH Justin 2013.03.21 Implement NHD1.6
	CString R1_Surcharge_DCC;				// fd		// [#2182] NH Justin 2013.03.21 Implement NHD1.6
	CString R1_ConnectionResult;
	CString	R2_DCC_BaseTargetCurrency;		// ec	
	CString	R2_DCC_ExchangeRate;			// er
	CString	R2_DCC_ConvertedTrAmount;		// et
	CString	R2_DCC_ConvertedSurcharge;		// ef	
	CString	R2_Surcharge_Withdrawal;		// fc	
	CString	R2_Surcharge_DCC;				// fd	
	CString	R2_Surcharge_MPT;				// ft	
	CString R2_ConnectionResult;
	int		nService_PINChange;
	int		nService_DCC;
	int		nService_MPT;
	int		nDCCTransaction;
	CString	R2_DisplayedExchangeRate;		//	[#2292] US Justin 2014.10.06	
	CString	R1_DCC_ExchangeRateMarkUp;		// em		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message (STD1)
	CString	R2_DCC_ExchangeRateMarkUp;		// em		// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message (STD1)
} STD1_DynamicFlowResponse;
// End of [#2150]

///////////////////////////////////////////////////////////////////////////////
// KSK 2010.02.02 Standard2 Message Structure
///////////////////////////////////////////////////////////////////////////////

typedef struct tagSTD2_CommonReqMsgHeader	// Host STD2 Request Message Header
{
	CString	RecordFormat_1;				// Record Format (I or 5)
	CString	ApplicationType_1;			// Application Type (= '0')
	CString	MessageDelimiter_1;			// MessageDelimiter (= '.')
	CString	BankID_6;					// Bank ID
	CString	TerminalID_8;				// Terminal ID
} STD2_CommonReqMsgHeader;

// 1-2.Response Message Header
typedef struct tagSTD2_CommonRespMsgHeader	// Host STD2 Response Message Header
{
	CString	RecordFormat_1;				// Record Format (I or 5)
	CString	ApplicationType_1;			// Application Type (= '0')
	CString	MessageDelimiter_1;			// MessageDelimiter (= '.')
	CString	BankID_6;					// Bank ID (6)
	CString	TerminalID_8;				// Terminal ID
} STD2_CommonRespMsgHeader;

typedef struct tagSTD2_ConfigRequest	// Host STD2 Configuration Request
{
	CString RequestType_2;				// Request Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	KeyMode_1;					// Key Mode (0 ~ 11)
	CString	UKSerialNoA_8;				// Unique Serial Number A
	CString	UKSerialNoB_8;				// Unique Serial Number B
	CString SubRequestType_2;			// [#560] NH KSK 2009.8.20
	CString	MACData_8;					// MAC Data
} STD2_ConfigRequest;

typedef struct tagSTD2_ConfigResponse		// Host STD2 Open Response
{
	CString	ResponseType_2;				// Response Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	LocalDate_8;				// Local Date
	CString	LocalTime_6;				// Local Time
	CString	HealthMsgTimerVal_6;		// Healthy Message Timer Value
	CString	WorkingKey_V;				// Working Key
	CString	SurchargeAmount_V;			// Surcharge Amount	 (ddcc)		** Variable
	CString	BINListReqFlag_1;			// BIN List Request Flag
	CString WorkingKey2_V;				// 2번째 암호화키
	CString WorkingKey3_V;				// 3번째 암호화키
	CString WorkingKey4_V;				// 4번째 암호화키
	CString WorkingKey5_V;				// 5번째 암호화키 [#565] NH KSK 2009.8.20
	CString	MACData_8;					// MAC Data
} STD2_ConfigResponse;

typedef struct tagSTD2_TranRequest		// Host STD2 Request Main
{
	CString RequestType_2;				// Request Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	OperationCode_2;			// Operation Code
	CString	SourceAccount_2;			// Source Account
	CString	DestinationAccount_2;		// Destination Account
	CString	TransactionSequenceNo_4;	// Transaction Sequence Number
	CString	Track1Data_79;				// ISO Track 1 Data
	CString	Track2Data_40;				// ISO Track 2 Data
	CString	Track3Data_106;				// ISO Track 3 Data
	CString	PINBuffer_16;				// PIN Buffer 16
	CString	TransactionAmount_V;		// Variable Size	"ddddddcc"
	CString	SurchargeAmount_V;			// Surcharge Amount	"ddcc"
	CString	SurchargeEnableFlag_1;		// Surcharge Enable Flag
	CString	AuthrizationNumber_9;		// Authorization Number (Check Cashing Customer Number) [#369] [US] KSK 2008.7.17
	CString	PINNumber_6;				// PIN Number (Check Cashing Customer Number) [#369] [US] KSK 2008.7.17
	CString	MACData_8;					// MAC Data					*2
} STD2_TranRequest;

// 2-2. Transaction Response Message Main
typedef struct tagSTD2_TranResponse		// Host STD2 Transaction Response
{
	CString	ResponseType_2;				// Response Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	TranSequenceNo_4;			// Transaction Sequence Number
	CString	ResponseCode_2;				// Response Code
	CString	LocalDate_8;				// Local Transaction Date
	CString	LocalTime_6;				// Local Transaction Time
	CString	RetrievalRefNo_12;			// Retrieval Reference Number
	CString	SystemTraceAuditNo_6;		// System Trace Audit Number
	CString	NetworkIDCode_2;			// Network ID Code
	CString	SettlementDate_8;			// Settlement Date
	CString	AccountBalance_V;			// Account Balance,	 (ddddddcc)	** Variable
	CString	AvailableBalance_V;			// Available Balance,(ddddddcc)	** Variable
	CString	SurchargeAmount_V;			// Surcharge Amount	 (ddcc)		** Variable
	CString	AuthResponseText_V;			// Authorization Response Text	** Variable
	CString	ConfigRequestInitiator_2;	// Configuration Request Initiator
	CString	MACData_8;					// MAC Data
} STD2_TranResponse;

typedef struct tagSTD2_ReversalRequest	// Host STD2 Reversal Request
{
	CString RequestType_2;				// Request Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	LocalTranDate_8;			// Local Transaction Date
	CString	LocalTranTime_6;			// Local Transaction Time
	CString	RetrievalRefNo_12;			// Retrieval Reference Number
	CString	RequestedDollarAmt_V;		// Requested Dollar Amount		Variable
	CString	DispensedDollarAmt_V;		// Dispensed Dollar Amount		Variable
	CString	SurchargeAmount_4;			// Surcharge Amount	"ddcc"
	CString	SurchargeEnableFlag_1;		// Surcharge Enable Flag
	CString	MACData_8;					// MAC Data					*2
} STD2_ReversalRequest;

typedef struct tagSTD2_ReversalResponse	// Host STD2 Reversal Response
{
	CString	ResponseType_2;				// Response Type
	CString	TimeVariant_8;				// Time Variant				*2
	CString	ConfigRequestInitiator_2;	// Configuration Request Initiator
	CString	MACData_8;					// MAC Data
} STD2_ReversalResponse;

typedef struct tagSTD2_TotalRequest		// Host STD2 Total Request Main
{
	CString RequestType_2;				// Request Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	ResetHostTotalFlag_1;		// Reset Host Total Flag
	CString	MACData_8;					// MAC Data					*2
} STD2_TotalRequest;

// 5-2. Total Response Message Main
typedef struct tagSTD2_TotalResponse	// Host STD2 Reversal Response
{
	CString	ResponseType_2;				// Response Type = '87'
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString NoOfCashWiths_4;			// No of Cash Withdrawals
	CString NoOfTransfers_4;			// No of Transfers
	CString NoOfInquiries_4;			// No of Inquiries
	CString NoOfNonCashWiths_4;			// No of None Cash Withdrawals
	CString TotalDispenseAmt_V;			// Total Dispensed Amount
	CString TotalNonCashDispAmt_V;		// Total Non-Cash Dispensed Amount
	CString TotalSurchargeAmt_V;		// Total Surcharge Amount
	CString ConfigRequestInitiator_2;	// Configuration Initiator
	CString	MACData_8;					// MAC Data					*2
} STD2_TotalResponse;

typedef struct tagSTD2_HealthRequest	// Host STD2 Healthy Request
{
	CString RequestType_2;				// Request Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	LocalDate_8;				// Local Transaction Date
	CString	LocalTime_6;				// Local Transaction Time
	CString TerminalStatus_4;			// Terminal Status
	CString	MACData_8;					// MAC Data					*2
} STD2_HealthRequest;

typedef struct tagSTD2_HealthResponse	// Host STD2 Healthy Response
{
	CString	ResponseType_2;				// Response Type = '90'
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	ConfigRequestInitiator_2;	// Configuration Request Initiator
	CString	MACData_8;					// MAC Data					*2
} STD2_HealthResponse;

typedef struct tagSTD2_DetailHealthRequest // Host STD2 Detail Health Request by "15" Result from host
{
	CString RequestType_2;				// Request Type
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	LocalDate_8;				// Local Transaction Date
	CString	LocalTime_6;				// Local Transaction Time
	CString	TerminalStatus_16;			// Terminal Status
	CString	ApplicationVersion_7;		// Application Version #
	CString	ApplicationCRC_4;			// Application CRC #
	CString	MACData_8;					// MAC Data					*2
} STD2_DetailHealthRequest;

typedef struct tagSTD2_DetailHealthResponse	// Host STD2 Healthy Response
{
	CString	ResponseType_2;				// Response Type = '90'
	CString	TimeVariantNo_8;			// Time Variant				*2
	CString	ConfigRequestInitiator_2;	// Configuration Request Initiator
	CString	MACData_8;					// MAC Data					*2
} STD2_DetailHealthResponse;


//////////////////////////////////////////////////////////////////////////
//				TRITON MESSAGE FORMATE STRUCTURE							
//////////////////////////////////////////////////////////////////////////
// TRITON Communication Header Format
typedef struct tagSTD3_CommHeader {
	CString m_strCommsHeaderID_8;
	CString m_strTerminalID_2;			// "td" fixed data
	CString m_strSoftwareVersionNo_2;
	CString m_strEncryptionModeFlag_1;
	CString m_strInformationHeader_7;	// Unused Must be space filled
	CString m_strTerminalID_15;
} STD3_COMMHEADERS, *PSTD3_COMMHEADERS;

// TRITON TRANSACTION REQUEST, RESPONSE MESSAGE
typedef struct tagSTD3_TranRequestMsg {
	CString m_strTransactionCode_2;
	CString m_strSequenceNumber_4;
	CString m_strTrack2_37;
	CString m_strAmount1_8;	   // Accept.Money
	CString m_strAmount2_8;    // ATMINFO.INI [TRANINFO]
	CString m_strPINBlock_16;	 // Accept.PassWord
	CString m_strMiscellaneous1;
	CString m_strMiscellaneous2;
	CString m_strStatusMonintoring;
	CString m_strMiscellaneous3;
}STD3_TRANREQMSG, *PSTD3_TRANREQMSG;

typedef struct tagSTD3_TranResponseMsg {
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
	
	CString m_strAvailableBalance_8;	// CITI07-087 KJK_086 2007.04.30
	CString m_strAmount1_Ext12;			// CITI07-087 KJK_087 2007.04.30
	CString m_strEMVTag;				// [#2291] MX Justin 2014.09.02
}STD3_TRANRESPMSG, *PSTD3_TRANRESPMSG;

// TRITON REVERSAL REQUEST, RESPONSE MESSAGE
typedef struct tagSTD3_ReversalRequestMsg {
     CString m_strTransactionCode_2;
     CString m_strSequenceNumber_4;
     CString m_strTrack2_37;
     CString m_strAmount1_8;
     CString m_strAmount2_8;
     CString m_strAmount3_8;
     CString m_strStatusMonintoring;
     CString m_strMiscellaneousX_V;
}STD3_REVERSALREQMSG, *PSTD3_REVERSALREQMSG;

typedef struct tagSTD3_ReversalResponseMsg {
	CString m_strInformation_Header_12;
	CString m_strTerminalID_15;
	CString m_strTransactionCode_2;
	CString m_strSequenceNumber_4;
	CString m_strResponseCode_3;	
}STD3_REVERSALRESPMSG, *PSTD3_REVERSALRESPMSG;

// TRITON CONFIGURATION REQUEST, RESPONSE MESSAGE
typedef struct tagSTD3_ConfigRequestMsg {
	CString m_strTransactionCode_2;
	CString m_strStatusMonitoringField_X;
	CString m_strMiscellaneousX_V;
} STD3_CONFIGREQMSG, *PSTD3_CONFIGREQMSG;

typedef struct tagSTD3_ConfigResponseMsg {
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
	CString m_strFieldIDCode6_1;
	CString m_strTR31PINKey6_V;
} STD3_CONFIGRESPMSG,*PSTD3_CONFIGRESPMSG;

// TRITON HOST TOTAL REQUEST, RESPONSE MESSAGE
typedef struct STD3_HostTotalsRequestMsg {
     CString m_strTransactionCode_2;
     CString m_strStatusMonintoring;
     CString m_strTotalWithdrawals_4;
     CString m_strTotalInquiries_4;
     CString m_strTotalTransfers_4;
     CString m_strSettlement_12;
     CString m_strMiscellaneousX_V;
}STD3_HOSTTOTALSREQMSG, *PSTD3_HOSTTOTALSREQMSG;

typedef struct tagSTD3_HostTotalResponseMsg {
	CString m_strInformation_Header_12;
	CString m_strTerminalID_15;
	CString m_strTransactionCode_2;
	CString m_strBusinessDate_6;
	CString m_strNumberOfWithdrawals_4;
	CString m_strNumberOfInquiries_4;
	CString m_strNumberOfTransfers_4;
	CString m_strSettlement_8;
	
	// KSK_2007-07-24오후 3:36:21
	CString m_strSettlement_Ext12;			// CITI07-087 KJK_087 2007.04.30
	// end of KSK_2007-07-24오후 3:36:21
}STD3_HOSTTOTALSRESPMSG, *PSTD3_HOSTTOTALSRESPMSG;

// TRITON STATUS MONITORING MESSAGE
typedef struct tagSTD3_StatusMonitoring {
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
} STD3_STATUS_MONITOR, *PSTD3_STATUS_MONITOR;

// [#2292] US Justin 2014.09.18 Add TDL and Dual Balance
#define	_TDL_TRAN_NONE				0
#define _TDL_TRAN_DYNAMICSURCHARGE	1
#define _TDL_TRAN_DCC				2

#define	_TDL_DCCPROC_NONE			0
#define	_TDL_DCCPROC_LOOKUP			1
#define	_TDL_DCCPROC_TRANSACTION	2
#define _TDL_DCCPROC_REVERSAL		3

#define	_TDL_REQUEST_BUFFER_SIZE	2048
#define	_TDL_RESPONSE_BUFFER_SIZE	2048

#define	_TDL_TAGTYPE_PRIMITIVE		0
#define	_TDL_TAGTYPE_CONSTRUCTIVE	1

class CSTD3_TDL_DCC_LookUp 
{
public:
	CSTD3_TDL_DCC_LookUp();
	virtual ~CSTD3_TDL_DCC_LookUp();

	CString m_strReq_PAN;
	CString m_strReq_LocalCurrency;
	CString m_strReq_RequestAmount;
	CString m_strReq_Language;

	CString m_strRes_DCCEligible;
	CString m_strRes_SurchargeType;
	CString m_strRes_SurchargeAmount;
	CString m_strRes_DCCOfferType;
	CString m_strRes_DCCOfferConversionRate;
	CString m_strRes_DCCOfferHomeCurrencyCode;
	CString m_strRes_DCCOfferHomeCurrencyISO;
	CString m_strRes_DCCOfferQuotationType;
	//CString m_strRes_DCCDisclaimerType;			//[#2302] US Justin 2014.11.05 Remove DCC Disclaimer Type
	CString m_strRes_DCCDisclaimerScheme;			//[#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
	CString m_strRes_DCCConvertedSurcharge;			//[#2297] US Justin 2014.10.20 Add Converted Amount
	CString m_strRes_DCCConvertedTranAmount;		//[#2297] US Justin 2014.10.20 Add Converted Amount
	CString m_strRes_DCCMarkupRate;					//[#RWC6-2, #2585] US Brandon Support VISA DCC for STD3 message T96.01.36.00
	CString m_strRes_WorkingKey;
	CString m_strDisplayedExchangeRate;
	CString m_ivaTaxAmount;
	CString m_convertedIvaTaxAmount;
	CString m_surchargePlusIvaTaxAmount;
	CString m_convertedSurchargePlusIvaTaxAmount;

private:
	void	ClearResponseData();
	void	FillCurrencyISOWithCurrencyCode(LPCTSTR sCode);
	BOOL	ParseDCCOfferMessage(BYTE* pTargetBuffer, int nBufSize);

public:
	void	ClearData();
	int		MakeRequestMessage(BYTE* pTargetBuffer);
	BOOL	ParseResponseMessage(BYTE* pTargetBuffer, int nBufSize);
	int		GetCurrencyDecimalNumber(LPCTSTR sCode);	// [#2348] US Justin 2015.06.02 Fix Decimal Point
};

class CSTD3_TDL_DCC_Transaction 
{
public:
	CSTD3_TDL_DCC_Transaction();
	virtual ~CSTD3_TDL_DCC_Transaction();

	CString m_strReq_Account;
	CString m_strReq_OfferAcceptance;
	CString m_strReq_RequestAmount;
	CString m_strReq_LocalCurrency;
	CString m_strReq_SurchargeAmount;
	CString m_strReq_ConvertedAmount;
	CString m_strReq_HomeCurrencyCode;
	CString m_strReq_ConvertedSurcharge;
	CString m_strReq_PresentedAmount;
	CString m_strReq_Track2;
	CString m_strReq_PINBlock;
	CString m_strReq_EMVData;
	CString m_strReq_ivaTaxAmount;
	CString m_strReq_convertedIvaTaxAmount;
	CString m_strReq_surchargePlusIvaTaxAmount;
	CString m_strReq_convertedSurchargePlusIvaTaxAmount;

	CString m_strRes_TranDateTime;
	CString m_strRes_AuthCode;
	CString m_strRes_AuthNumber;
	CString m_strRes_BusDate;
	CString m_strRes_EMVData;
	CString m_strRes_WorkingKey;

private:
	void	ClearResponseData();

public:
	void	ClearData();
	int		MakeRequestMessage(BYTE* pTargetBuffer);
	BOOL	ParseResponseMessage(BYTE* pTargetBuffer, int nBufSize);
};

class CSTD3_TDL_DCC_Reversal 
{
public:
	CSTD3_TDL_DCC_Reversal();
	virtual ~CSTD3_TDL_DCC_Reversal();

	CString m_strReq_ReversalReason;
	CString m_strReq_OfferAcceptance;
	CString m_strReq_RequestAmount;
	CString m_strReq_DispensedAmount;
	CString m_strReq_LocalCurrency;
	CString m_strReq_SurchargeAmount;
	CString m_strReq_PresentedAmount;
	CString m_strReq_Track2;
	CString m_strReq_EMVData;
	CString m_strReq_ivaTaxAmount;

	CString m_strRes_AuthCode;
	CString m_strRes_WorkingKey;

private:
	void	ClearResponseData();

public:
	void	ClearData();
	int		MakeRequestMessage(BYTE* pTargetBuffer);
	BOOL	ParseResponseMessage(BYTE* pTargetBuffer, int nBufSize);
};

class CSTD3_TDL_DATA 
{
public:
	CSTD3_TDL_DATA();
	virtual ~CSTD3_TDL_DATA();
	
	int	m_nTranType;
	int	m_nDCCProc;
	BOOL m_bTranResult;
	BOOL m_bProceedDCC;

	CString m_strWorkingKey;
	CString m_strSurchargeAmount;

	CString m_strTID;
	CString m_strSeqNum;
	CString m_strTimeStamp;

	CSTD3_TDL_DCC_LookUp		m_DCC_Lookup;
	CSTD3_TDL_DCC_Transaction	m_DCC_Transaction;
	CSTD3_TDL_DCC_Reversal		m_DCC_Reversal;

private:
	long	m_nHexSeqNum;		// [#2577] US Justin 2018.09.12 Fix TDL Sequence Number : String => HexDecimal

public:
	void	ClearAllData();
	CString	MakeTDLTag(int nTranCode, CString strTID, CString strSeqNum);
	BOOL	ParseTDLTag(CString strResponse);
	CString	UpdateSurchargeFromDCCLookup();

private:
	CString	MakeDCCRequestData(CString strTID, CString strSeqNum, int nProcStep);
	BOOL	ParseCorrelationInformation(BYTE* pTargetBuffer, int nBufSize);
	BOOL	ParseSetDataResponseCode(BYTE* pTargetBuffer, int nBufSize);
	BOOL	ParseDCCResponseData(CString strResponse);
};

// Dual Balance Inquiry
#define	_DUAL_BALANCE_DISPLAY_NOTSET	0
#define	_DUAL_BALANCE_DISPLAY_PRINTER	1
#define	_DUAL_BALANCE_DISPLAY_SCREEN	2

class CDualBalanceInquiry 
{
public:
	CDualBalanceInquiry();
	virtual ~CDualBalanceInquiry();

	BOOL	m_bDualBalance;
	int		m_nDisplaceDevice;
	BOOL	m_bTranResult;

	CString m_strAuthorizationNumber;
	CString	m_strSequenceNumber;
	CString	m_strTranDate;
	CString	m_strTranTime;
	CString m_strBusinessDate;

	BOOL	m_bCheckingBalance;
	CString m_strCheckingLedgerBalance;
	CString m_strCheckingAvailBalance;
	CString m_strCheckingError;

	BOOL	m_bSavingBalance;
	CString	m_strSavingLedgerBalance;
	CString m_strSavingAvailBalance;
	CString m_strSavingError;

private:


public:
	void	ClearSetting();
	void	ClearTranData();
};
// End of [#2292]

//[#2313] US Justin 2014.11.24 Event Notice
#define _EVENTID_IN_SERVICE		0
#define _EVENTID_START_TRAN		1
#define _EVENTID_READ_CARD		2
#define _EVENTID_DISPENSED		3
#define _EVENTID_PRINT_RECEIPT	4
#define _EVENTID_FINISH_TRAN	5
#define _EVENTID_TRAN_RESULT	9
#define _EVENTID_CANCEL_TRAN	10
#define _EVENTID_OUT_OF_SERVICE	11
// End of [#2313]

// HOST RESPONSE CODE, ERROR MESSAGE
typedef	struct	__RSP_TBL
{
	BYTE	CODE[4];
}_RSP_TBL;

typedef	struct	__RSP_ERR_TBL
{
	BYTE	MSG[44];
}_RSP_ERR_TBL;

// AIREAT
// V01.02.46
// V01.02.24 SRC-12
// [#559] KSK 2009.8.19 STANDARD1 최종 Response Code로 update 해야 하므로 STANDARD2 Cod도 호환을 위해 분기처리함
CONST _RSP_TBL	STD1_RSP_TBL[58] = {
/* 0 ~ 9	*/	"01", "02", "03", "04", "05", "06", "07", "09", "11", "12",
/* 10 ~ 19	*/	"13", "14", "15", "19", "20", "21", "22", "23", "24", "30",
/* 20 ~ 29	*/	"31", "32", "39", "40", "42", "44", "50", "51", "52", "53",
/* 30 ~ 39	*/	"54", "55", "56", "57", "58", "59", "60", "61", "62", "63",
/* 40 ~ 49	*/	"65", "70", "71", "75", "76", "78", "80", "83", "86", "90",
/* 50 ~ 57	*/	"91", "92", "93", "94", "95", "96", "97", "98"
 };

CONST _RSP_TBL	STD2_RSP_TBL[21] = {
/* 0 ~ 9	*/	"12", "13", "14", "20", "24", "39", "51", "52", "53", "54",
/* 10 ~ 19	*/	"55", "57", "58", "61", "75", "78", "80", "83", "86", "91",
/* 20		*/	"92"
};
// end of [#559]
// end of KSK 2009.4.25

#if 0	// Text File을 사용함
CONST _RSP_ERR_TBL STD12_RSP_ERR_TBL[21] = {
	{"Invalid transaction                        "},	// 12
	{"Invalid amount                             "},	// 13
	{"Invalid card                               "},	// 14
	{"Surcharge error                            "},	// 20
	{"Exceeds limit                              "},	// 24

	{"Invalid account                            "},	// 39
	{"Not enough funds                           "},	// 51
	{"Invalid account                            "},	// 52
	{"Invalid account                            "},	// 53
	{"Invalid card                               "},	// 54

	{"Bad PIN                                    "},	// 55
	{"Invalid card                               "},	// 57
	{"Invalid transaction                        "},	// 58
	{"Exceeds limit                              "},	// 61
	{"Bad PIN                                    "},	// 75

	{"Invalid account                            "},	// 78
	{"Invalid date                               "},	// 80
	{"Bad PIN                                    "},	// 83
	{"Bad PIN                                    "},	// 86
	{"No bank response                           "},	// 91

	{"No network response                        "}		// 92

	};
// end of AIREAT
#endif	// end of [#310]


// AIREAT_2007-12-13오후 4:55:43
// V01.02.46 
// Insert item for full authorization codes table
//CONST _RSP_TBL	TRITON_RSP_TBL[39] = {					/* RESPONSE AUTHORIZATION CODE TABLE */	// [#514] [MX] KSK 2009.3.10
CONST _RSP_TBL	TRITON_RSP_TBL[40] = {					/* RESPONSE AUTHORIZATION CODE TABLE */	// [#514] [MX] KSK 2009.3.10
		"001", "002", "003", "004", "005",
		"006", "007", "008", "009", "010",
		"011", "012", "013", "014", "015",
		"016", "017", "018", "019", "020",
		"021", "022", "023", "024", "025",
		"026", "027", "028", "029", "030",
		"031", "032", "033", "034", "035",
		"036", "111", "222", "223", "037" };		// ADD "037"

#if 0		// [#514] [MX] KSK 2009.3.10
//CONST _RSP_ERR_TBL TRITON_RSP_ERR_TBL[39] = {				/* RESPONSE AUTHORIZATION MESSAGE TABLE */
CONST _RSP_ERR_TBL TRITON_RSP_ERR_TBL[40] = {				/* RESPONSE AUTHORIZATION MESSAGE TABLE */	// [#514] [MX] KSK 2009.3.10
	{"Expired card                               "},	// 001
	{"Unauthorized usage                         "},	// 002
	{"PIN error                                  "},	// 003
	{"Invalid PIN                                "},	// 004
	{"Bank unavailable                           "},	// 005

	{"Card not supported                         "},	// 006
	{"Insufficient funds                         "},	// 007
	{"Ineligible transaction                     "},	// 008
	{"Ineligible account                         "},	// 009
	{"Daily withdrawal count over                "},	// 010
	
	{"Cannot process transaction                 "},	// 011
	{"Amount too large                           "},	// 012
	{"Account closed                             "},	// 013
	{"PIN tries exceeded                         "},	// 014
	{"Database problem                           "},	// 015
	
	{"Withdrawal limit already over              "},	// 016
	{"Invalid Amount                             "},	// 017
	{"External decline                           "},	// 018
	{"System error                               "},	// 019
	{"Contact card issuer                        "},	// 020
	
	{"Routing lookup problem                     "},	// 021
	{"Message edit error                         "},	// 022
	{"Transaction not supported                  "},	// 023
	{"Insufficient funds                         "},	// 024
	{"Western Union sender data error            "},	// 025

	{"Western Union receiver data err            "},	// 026
	{"CRC error                                  "},	// 027
	{"Pre-pay transaction failed                 "},	// 028
	{"Pre-pay transaction rejected               "},	// 029
	{"Invalid mobile phone number                "},	// 030

	{"Pre-pay account limit reached              "},	// 031
	{"Pre-pay system unavailable                 "},	// 032
	{"Response msg size limit over               "},	// 033
	{"required info lost to process              "},	// 034
	{"Second Invalid PIN                         "},	// 035
	{"Fallback Not Allowed                       "},	// 036

	{"Reversal Declined                          "},	// 111
	{"PIN Change Declined                        "},	// 222
	{"PIN Unlock Declined                        "}		// 223
	{"Invalid Exchange Rate                      "}		// 223
};
#endif	// end of [#310]

#define MISCELLANEOUS_REQ_TRAN		0
#define MISCELLANEOUS_REQ_CONFIG	1
#define MISCELLANEOUS_REQ_TOTAL		2
#define MISCELLANEOUS_REQ_REVERSAL	3

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
//#define FID2_o	0x00040000		//	[#438] AU AIREAT 2008.10.17	- 사용하지 않음.
#define FID2_21h	0x00040000		//	[#438] AU AIREAT 2008.10.17	- 'o' -> '!'(21h)로 변경.
#define FID2_p		0x00080000
#define FID2_q		0x00001000
#define FID2_r		0x00002000
#define FID2_s		0x00004000
#define FID2_t		0x00008000
#define FID2_u		0x00000100
#define FID2_v		0x00000200
#define FID2_w		0x00000400			// 사용하지 않음.
#define FID2_x		0x00000800

// TRITON MISCELLANEOUS FIELD ID TABLE
CONST UINT chMiscellaneous_Requests[4][34] = 
//	{	{'!','#','&','@','^','e','g','i','l','v'|FID2_a|FID2_b|FID2_c|FID2_d
	{	{'#','&','@','^','e','g','i','l','v'|FID2_a|FID2_b|FID2_c|FID2_d	// KSK 2008.10.29 '!' Field 제거
										,'x'|FID2_b|FID2_d|FID2_g|FID2_r|FID2_u
										,'y'|FID2_a|FID2_b|FID2_c|FID2_d
										,'z'|FID2_a|FID2_b|FID2_c
										,'u'|FID2_b|FID2_d|FID2_h
										},	// Transaction Message
		
		{'[','u'|FID2_b},						// Config. Message 
		{'[','^','o','v'|FID2_a
					,'x'|FID2_e|FID2_s|FID2_v
					,'y'|FID2_e|FID2_f
					,'z'|FID2_k|FID2_m
					,'u'|FID2_b},			// Host Total Message
		{'#','^','g','i','k','n'

#if (MX_VERSION) 
					,'q'|FID2_b            // [#2015] MX, 2012.02.02 KSK: qbA,qbB,qbC,qbD 필드 추가 (Reversal Request)     
#endif
					,'v'|FID2_a|FID2_b
					,'x'|FID2_u
					,'u'|FID2_b|FID2_d|FID2_h
					},							// Reversal Message
	};

#define MISCELLANEOUS_RESP_TRAN		0
#define MISCELLANEOUS_RESP_CONFIG	1
#define MISCELLANEOUS_RESP_TOTAL	2
#define MISCELLANEOUS_RESP_REVERSAL	3

CONST UINT chMiscellaneous_Responses[4][34]= 
	{	{'!','#','&','/','^','{','~','}','<','b','c','e','k','l','m','p','t'
			,'d'|FID2_e|FID2_m|FID2_s|FID2_w
			,'u'|FID2_a|FID2_b|FID2_d|FID2_e|FID2_f|FID2_g|FID2_h|FID2_21h//|FID2_i
#if (MX_VERSION) 
			,'q'|FID2_a|FID2_b|FID2_c					// [#2015] MX, 2012.02.02 KSK: qbA,qbB,qbC,qbD 필드 추가 (Transaction Response)
#elif(US_VERSION || CA_VERSION)			// [#2365] CA Justin 2015.07.27 Add DCC+ in Canadian Version.
			,'q'|FID2_a|FID2_c			// [#2292] US Justin 2014.09.30 Add TDL qa(dynamic surcharge) and qc(TDL) Field.
#endif
			,'v'|FID2_d
			,'w'
			,'x'|FID2_a|FID2_b|FID2_c|FID2_d|FID2_f|FID2_g|FID2_h|FID2_i|FID2_n||FID2_p|FID2_q|FID2_t
			,'z'|FID2_c|FID2_d|FID2_e|FID2_j|FID2_n
			}, // Transaction Response Message
		{'!','/','{','~','}','<','m'
			,'u'|FID2_b|FID2_e|FID2_f|FID2_21h
			,'x'|FID2_q|FID2_t
			,'z'|FID2_f|FID2_g|FID2_h|FID2_i|FID2_n}, // Configure Response Message
		{'^','o','p'
			,'u'|FID2_b|FID2_g
			,'x'|FID2_e|FID2_s|FID2_v
			,'y'|FID2_e|FID2_f
			,'z'|FID2_k|FID2_m}, // Host Total Response Message
		{'#','^'
#if(US_VERSION || CA_VERSION)			// [#2365] CA Justin 2015.07.27 Add DCC+ in Canadian Version.
			,'q'|FID2_c			// [#2292] US Justin 2014.09.30 Add TDL qc(TDL) Field (Reversal Response)
#endif
			,'u'|FID2_b|FID2_g}, // Reversal Response Message
	};


#define ASSORTED_REQUEST_TRAN			0
#define ASSORTED_REQUEST_REVERSAL		1
#define ASSORTED_REQUEST_TOTAL			2
#define ASSORTED_REQUEST_HEALTHCHECK	3
#define ASSORTED_REQUEST_EXTCONFIG		4	// [#560]

// [#2076] NH KSK 2011.06.28
// [#397] NH PSC 2008.12.15 Standard1 전문에 "uh" 필드 추가
//CONST UINT chAssorted_Requests[4][17] = 
CONST UINT chAssorted_Requests[5][17] =			// [#560] NH KSK 2009.8.20
	{
		{'e'|FID2_j
			, 's', 'u'|FID2_d|FID2_h
			, 'x'|FID2_c|FID2_f|FID2_r|FID2_t|FID2_x		//[#2150] US Justin 2012.10.01 Add DCC and Pin Change Assorted Fields.
			, 'y'|FID2_c|FID2_d|FID2_m},		// ASSORTED_REQUEST_TRAN 
		{'e'|FID2_j, 'f'|FID2_k, 'n', 'r', 's'
		, 'u'|FID2_d|FID2_h},					// ASSORTED_REQUEST_REVERSAL
		{'e'|FID2_j, 's', 'y'|FID2_e},			// ASSORTED_REQUEST_TOTAL
		{'e'|FID2_j, 's'},						// ASSORTED_REQUEST_HEALTHCHECK
		{'e'|FID2_t|FID2_j	 					// ASSORTED_REQUEST_EXTENDED CONFIGURATION
			, 'a'|FID2_e }						// [#2449] US Justin 2016.11.08
	};

#define ASSORTED_RESPONSE_TRAN			0
#define ASSORTED_RESPONSE_REVERSAL		1
#define ASSORTED_RESPONSE_TOTAL			2
#define ASSORTED_RESPONSE_HEALTHCHECK	3
#define ASSORTED_RESPONSE_EXTCONFIG		4

CONST UINT chAssorted_Response[5][17] =	
{
	{'~', '{', '(', '}', '%', 'c'
		, 'e'|FID2_j
		, 'f'|FID2_c|FID2_i|FID2_k
		, 'h', 'j', 'm', 'p', 't'
		, 'u'|FID2_a|FID2_d|FID2_e|FID2_f|FID2_h|FID2_p
		, 'x'|FID2_g				//[#2182] US Justin 2013.03.22 NHS2.6  => DCC Disclaimer
		, 'y'|FID2_q
		, 'w'},						// ASSORTED_RESPONSE_TRAN
	{'e'|FID2_j, 'h', 'j'},			// ASSORTED_RESPONSE_REVERSAL
	{'e'|FID2_j, 'h', 'j'
	,'p', 'y'|FID2_e},				// ASSORTED_RESPONSE_TOTAL
	{'e'|FID2_j, 'h', 'j'},			// ASSORTED_RESPONSE_HEALTHCHECK
	{'e'|FID2_f|FID2_s|FID2_j, 'j'	// ASSORTED_RESPONSE_EXTENDED CONFIGURATION		// [#2329] US Justin 2015.01.27 Add 'j' for extended config msg
		, 'a'|FID2_e|FID2_i|FID2_k|FID2_l|FID2_p|FID2_u }			// [#2449] US Justin 2016.11.08
};
// end of [#2076]

// [#560] NH KSK 2009.8.20
#define MAX_AIDLIST_CNT			60		// Kernel이 60개까지밖에 현재 지원하지 않음 (2009.8.24 Kernel Version : 2.1.7.0)

typedef struct _tagStandard1TerminalAIDList {
	int		nTotalAIDCount;
	char	chAID[MAX_AIDLIST_CNT][64];
}STANDARD1_TERMINAL_AIDLIST, *PSTANDARD1_TERMINAL_AIDLIST;

typedef struct _tagStandard1HostAIDList {
	int		nTotalAIDCount;
	char	chAID[MAX_AIDLIST_CNT][64];
	char	chTAC_Denial[MAX_AIDLIST_CNT][16];
	char	chTAC_Online[MAX_AIDLIST_CNT][16];
	char	chTAC_Default[MAX_AIDLIST_CNT][16];
}STANDARD1_HOST_AIDLIST, *PSTANDARD1_HOST_AIDLIST;
// end of [#560]

// [#2317] US Justin 2015.01.05 Support XML BIN Range
//#define _DCC_DOMESTIC_BINDATA_PATH	_T("\\ATM\\Data\\DCC_DomesticBIN.xml")	// Dual Host DCC Domestic BINS
#define _DCC_DOMESTIC_BINDATA_PATH	_T("\\ATM\\Data\\DCC_DomesticBIN.bin")	// Dual Host DCC Domestic BINS	// [#2332] US Justin 2015.02.12
#define _SIZE_ONE_BINRANGE			9

class CXMLBinRangeData 
{
public:
	CXMLBinRangeData();
	virtual ~CXMLBinRangeData();

	CString	 m_strUpdateDate;
	unsigned char *m_bufBIN;
	unsigned long m_nNumBIN;

	BOOL	IsDataLoaded();
	BOOL	LoadXMLBinRangeData();
	BOOL	IsCardListed(CString strCardNumber);
};
// End of [#2317]
#endif

