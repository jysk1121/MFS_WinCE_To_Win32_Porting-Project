#pragma once

#include "stdafx.h"

// [RWC6-676] SKKim 2024.03.27 created header file for TangoPay

//Integral indicators for different message types. Used for identifying log files.
static const int RESET_APIKEY			=					0;

static const int GET_PUBLICKEY			=					1;
static const int GET_TERMINAL_JWT		=					2;
static const int GET_USER				=					3;
static const int PRESTAGING_VERIFY		=					4;
static const int PRESTAGING_VALIDATION	=					5;
static const int COMMIT					=					6;

typedef struct _tangopay_resultinfo
{
	bool	Result;

	//-- Error Object
	CString ErrCode;
	CString ErrMessage;
	//--

	// [RWC6-676] Start SKKim 2024.05.28 HTTP Error Setting
	CString strStatusCode;
	CString strStatusMsg;

}sResultInfo;

/**
 * Response from the Get public key
 */
typedef struct _tangopay_get_publickey_response
{
	sResultInfo resultinfo;

	CString PublicKey;
	CString CheckSum;
} TangoPayGetPublicKeyResponse, *LPTangoPayGetPublicKeyResponse;

/**
* Request & Response from the JWT
*/
typedef struct _tangopay_get_terminal_jwt_request
{
	CString Id;
	CString Key;
	CString Nonce;
	CString CheckSum;
} TangoPayGetTerminaljwtRequest;

typedef struct _tangopay_get_terminal_jwt_response
{
	sResultInfo resultinfo;

	CString AccessToken;
	CString Nonce;
	CString Signature;
} TangoPayGetTerminaljwtResponse;

/**
* Request & Response from the Get User
*/
typedef struct _tangopay_get_user_request
{
	// header
	CString phoneNumber;
	CString countryCode;
	CString eid;
} TangoPayGetUserRequest;

typedef struct _tangopay_get_user_response
{
	sResultInfo resultinfo;

	CString retCode;
	CString retMsg;
	CString userId;
} TangoPayGetUserResponse;

/**
* Request & Response from the Prestaging verify
*/
typedef struct _tangopay_prestaging_verify_request
{
	// header
	CString eid;

	// body
	CString PhoneNumber;
	CString CountryCode;
	CString OtpCode;
	CString OtpSeed;
} TangoPayPreStagingVerifyRequest;


typedef struct _reservations
{
	CString Key;
	CString Value;
} Reservation;

typedef CList<Reservation> Reservations;

typedef struct STRUCT_prestaingInfo
{
	CString preStagingId;
	CString serviceProviderCd;
	CString serviceTypeCd;
	CString userId;
	CString deliveryOption;
	CString deliveryOptionDisplayName;

	CString receiveCountry;
	CString receiveState;
	CString receiveCurrency;
	CString receiverFirstName;
	CString receiverLastName;

	//-- reservations array
	Reservations reservations_array;
	//--

	CString stateName;
	CString countryName;
	CString ctn;
	CString ctnCountryCode;
	CString otpCode;
	int		otpExpireTime;
	CString createDt;
	CString receiveAgentID;
	CString receiveAgentAbbreviation;
	CString confirmationNumber;
	CString transactionId;
	bool	expired;
	bool	isUsed;
	int		otpSeed;
	int		lat;
	int		lng;
	bool	isRestored;
}PreStaingInfo;


typedef struct _tangopay_prestaging_verify_response
{
	sResultInfo resultinfo;

	CString retCode;
	CString retMsg;
	bool	verify;

	//-- Prestaging Info Object
	PreStaingInfo prestaingInfo_obj;
	//--
} TangoPayPreStagingVerifyResponse;

/**
* Request & Response from the Staging validation
*/
typedef struct _tangopay_staging_validation_request
{
	// header
	CString eid;

	// body
	CString MachineId;
	CString PartnerId;
	CString TransactionId;
	CString ConfirmationNumber;
	CString PrestagingId;
} TangoPayStagingValidationRequest;

typedef struct _detail_sendamount
{
	CString amountType;
	CString amount;
	CString amountCurrency;
} DetailSendAmount, *LPDetailSendAmount;

typedef CList<DetailSendAmount> DetailSendAmounts;


typedef struct _SendAmountInfo
{
	CString sendAmount;
	CString sendCurrency;
	CString totalSendFees;
	CString totalDiscountAmount;
	CString totalSendTaxes;
	CString totalAmountToCollect;

	//-- detailSendAmounts array
	DetailSendAmounts detailSendAmounts_array;
	//--
} SendAmountInfo, *LPSendAmountInfo;


typedef struct _detail_EstimatedReceiveAmount
{
	CString amountType;
	CString amount;
	CString amountCurrency;
} DetailEstimatedReceiveAmount, *LPDetailEstimatedReceiveAmount;

typedef CList<DetailEstimatedReceiveAmount> DetailEstimatedReceiveAmounts;


typedef struct _ReceiveAmountInfo
{
	CString  receiveAmount;
	CString receiveCurrency;
	bool	validCurrencyIndicator;
	CString payoutCurrency;
	CString	totalReceiveFees;
	CString	totalReceiveTaxes;
	CString	totalReceiveAmount;
	bool	receiveFeesAreEstimated;
	bool	receiveTaxesAreEstimated;

	//-- detailSendAmounts array
	DetailEstimatedReceiveAmounts	detailEstimatedReceiveAmounts_array;
	//--
} ReceiveAmountInfo, *LPReceiveAmountInfo;


typedef struct _disclosure
{
	CString fraudWarning;
	CString disclosure;
} sDisclosure, *LPsDisclosure;

typedef CList<sDisclosure> sDisclosures;

typedef struct _tangopay_staging_validation_response
{
	sResultInfo resultinfo;

	CString transactionId;
	CString mgiTransactionSessionId;
	CString receiverFirstName;
	CString receiverMiddleName;
	CString receiverLastName;
	CString deliveryOption;
	CString deliverOptionDisplayName;


	//-- SendAmount Info Object
	SendAmountInfo SendAmountInfo_obj;
	//--

	//-- ReceiveAmountInfo object
	ReceiveAmountInfo ReceiveAmountInfo_obj;
	//--

	//-- disclosures
	sDisclosures disclosuresFirst_array;
	sDisclosures disclosuresSecond_array;
	//--

} TangoPayStagingValidationResponse;

/**
* Request & Response from the Send Money Commit
*/
typedef struct _tangopay_send_monmey_commit_request
{
	// header
	CString eid;

	CString TransactionId;
	CString MgiTransactionSessionId;
} TangoPaySendMoneyCommitRequest;


typedef struct _agent_receipt
{
	CString contents;
} AgentReceipt, *LPAgentReceipt;

typedef CList<AgentReceipt> sAgentReceipts;

typedef struct _consumer_receipt
{
	CString contents;
} ConsumerReceipt, *LPConsumerReceipt;

typedef CList<ConsumerReceipt> sConsumerReceipts;

typedef struct _tangopay_send_monmey_commit_response
{
	sResultInfo resultinfo;

	CString referenceNumber;

	//-- AgentReceipts
	sAgentReceipts agentreceipts_array;

	//-- ConsumerReceipts
	sConsumerReceipts consumerreceipts_array;

	CString referenceNumberTextCode;
	CString referenceNumberText;
	CString referenceNumberConsumerText;

} TangoPaySendMoneyCommitResponse;