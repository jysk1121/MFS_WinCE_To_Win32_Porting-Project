#pragma once

#include "stdafx.h"

// NB: Enabling this option will dispense in test transactions
//#define LTX_TEST_MODE

#ifndef _DEBUG
#  ifdef LTX_TEST_MODE
#    error Cannot build LTX dispense with LTX_TEST_MODE defined!
#  endif
#endif

// The transaction status
typedef CString TXNSTATUS;
const TXNSTATUS	LXTXN_SUCCESS	= L"success";
const TXNSTATUS LXTXN_ERROR		= L"error";

// Currency Types
typedef CString LXCURRENCY;
const LXCURRENCY USD	= L"USD";

// Payment card types
typedef CString LXCARDTYPE;
const LXCARDTYPE DEBIT	= L"debit";

typedef CString LXERRORCODE;
const LXERRORCODE LXERR_AUTH			= L"AuthenticationError";
const LXERRORCODE LXERR_ARG				= L"ArgumentMissingError";
const LXERRORCODE LXERR_PARSE			= L"ParseError";
const LXERRORCODE LXERR_ARGINVALID		= L"InvalidArgumentError";
const LXERRORCODE LXERR_INTSERV			= L"InternalServerError";
const LXERRORCODE LXERR_PROCERR			= L"ProcessingError";
const LXERRORCODE LXERR_NOUSER			= L"UserNotFound";
const LXERRORCODE LXERR_USERINVALID		= L"UserNotValid";
const LXERRORCODE LXERR_USERPEND		= L"UserHasPending";
const LXERRORCODE LXERR_NOLOC			= L"LocationNotFound";
const LXERRORCODE LXERR_MULTTXN			= L"MultipleTxError";
const LXERRORCODE LXERR_NOPAY			= L"PaymentNotFound";
const LXERRORCODE LXERR_PAYACCEPT		= L"PaymentAlreadyAccepted";
const LXERRORCODE LXERR_PAYCANCEL		= L"PaymentCanceled";
const LXERRORCODE LXERR_AMTINVALID		= L"AmountInvalid";
const LXERRORCODE LXERR_DAILYTXNLMT		= L"TxLimDaily";
const LXERRORCODE LXERR_HOTWALLET		= L"RefillingHotWallet";  // Bitcoin purchases are temporarily unavailable while we refill our bitcoin wallet. It is usually resolved in less than 3 hours.
const LXERRORCODE LXERR_AMTLOW			= L"AmountBelowFeeMinimum";
const LXERRORCODE LXERR_CHAINAFAIL		= L"S001A";  // DS verification failed on chain A
const LXERRORCODE LXERR_CHAINBFAIL		= L"S001B";  // DS verification failed on chain B
const LXERRORCODE LXERR_CHAINCFAIL		= L"S001C";  // DS verification failed on chain C
const LXERRORCODE LXERR_CHAINASUB		= L"S002A";  // Chain A failed to verify to root A
const LXERRORCODE LXERR_CHAINBSUB		= L"S002B";  // Chain B failed to verify to root B
const LXERRORCODE LXERR_CHAINCSUB		= L"S002C";  // Chain C failed to verify to root C
const LXERRORCODE LXERR_NONCEMISMATCH	= L"S001M";  // An unexpected value was received for authorize_nonce
const LXERRORCODE LXERR_OVERLIMIT		= L"C0001";  // Amount exceeds dispense limit
const LXERRORCODE LXERR_NOTDISPENSABLE	= L"C0002";  // Not dispensable
const LXERRORCODE LXERR_CONNERR			= L"ConnectionError";

typedef CString LXSIGNATUREALGO;
const LXSIGNATUREALGO DISPENSE_EXEC_V1 = L"LTX_DISPENSE_EXEC_V1";

typedef CString LXSURCHARGEMODE;
const LXSURCHARGEMODE LXSURCHARGEMODE_FLAT				= L"flat_fee_only";
const LXSURCHARGEMODE LXSURCHARGEMODE_PERCENT			= L"percent_fee_only";
const LXSURCHARGEMODE LXSURCHARGEMODE_PERCENT_GREATER	= L"greater_percent_or_flat";
const LXSURCHARGEMODE LXSURCHARGEMODE_PERCENT_LESSER	= L"lesser_percent_or_flat";
const LXSURCHARGEMODE LXSURCHARGEMODE_PERCENT_TIERED	= L"tiered";

// V1 signature params
#define LTX_DISPENSE_SIG_V1_start "LTX_DISPENSE_EXEC_V1"
#define LTX_DISPENSE_SIG_V1_end   "LTX_DISPENSE_EXEC_V1_END"

typedef CMap<CString, LPCTSTR, int, int> CashoutMap;

/**
 * A payment card
 */
typedef struct _lx_card {
	// "debit" mandatory, must be "debit"
	LXCARDTYPE Type;
	CString Last4;
	CString Name;
	CString TID;
	CString BIN;

	// Optional parameters
	CString Authorization;
	CString AtmTID;
} LXCard, *LPLXCard;

/** 
 * Authorizes a transaction
 */
typedef struct _lx_authorization {
	// Mobile app generated token for the customer transaction
	CString Token;

	// Amount the customer will pay
	double SourceAmount;

	// Currency of the payment (USD)
	LXCURRENCY  Currency;
	LXCard		Card;
	bool		HasCard;
} LXAuthorization, *LPLXAuthorization;

/**
 * The response from an authorization
 */
typedef struct _lx_authorization_response {
	TXNSTATUS Status;

	// Unique identifier for this payment.
	CString PaymentId;

	// Optional developer message
	CString Message;

	// Optional message to the customer
	CString MessageCustomer;

	bool HasError()
	{
		return Status == LXTXN_ERROR;
	};
} LXAuthorizationResponse, *LPLXAuthorizationResponse;

/**
 * An error code *TO* LibertyX
 */
typedef struct _lx_error {
	CString Code;

	// a human readable description of why the sale was canceled
	CString Message;

	// Optional data for an under-/over-dispense error (only applicable to the dispense operation)
	CString DispensedAmount;
	bool IsDispenseError;

	_lx_error::_lx_error()
	{
		IsDispenseError = false;
	}

} LXError, *LPLXError;

/**
 * A request to cancel a payment
 */
typedef struct _lx_cancel {
	CString PaymentId;

	LXCard	Card;
	bool	HasCard;

	LXError	Error;
	bool	HasError;
} LXCancel, *LPLXCancel;

/**
 * The response to the cancellation request
 */
typedef struct _lx_cancel_response {
	TXNSTATUS Status;
	CString Message;

	bool HasError()
	{
		return Status == LXTXN_ERROR;
	};
} LXCancelResponse, *LPLXCancelResponse;

/**
 * Information for confirming a payment
 */
typedef struct _lx_execute {
	CString PaymentId;
	LXCard	Card;
	bool	HasCard;
} LXExecute, *LPLXExecute;

/**
 * A response from the payment execution
 */
typedef struct _lx_execute_response {
	TXNSTATUS	Status;
	CString		Message;
	CString		ErrorType;

	CString		MessageCustomer;

	bool HasError()
	{
		return Status == LXTXN_ERROR;
	};
} LXExecuteResponse, *LPLXExecuteResponse;

/**
 * Information regarding the surcharge amounts in the dispense ping
 */
typedef struct _lx_dispense_surcharge_info {
	CString			FeeFlat;
	int				FeePercentage;
	LXSURCHARGEMODE	SurchargeMode;

	_lx_dispense_surcharge_info::_lx_dispense_surcharge_info() 
	{
		FeePercentage = 0;
	}
} LXDispenseSurcharge, *LPLXDispenseSurcharge;

typedef CMap<CString, LPCTSTR, LXDispenseSurcharge, LXDispenseSurcharge> SurchargeMap;

/**
 * Information regarding dispense information for LibertyX
 */
typedef struct _lx_dispense_ping_info {
	CString MaxWithdrawalLimit;
	CString MaxDispense;
	CashoutMap CashDenominations;
	SurchargeMap SurchargeInfos;
} LXDispensePing, *LPLXDispensePing;

/**
 * Information for sending a Ping request to LTX API
 */
typedef struct _lx_ping {
	CString LocationID;
	CString TID;
	CString SoftwareVersion;
	CString AtmTID;

	bool HasDispenseInfo;
	LXDispensePing DispenseInfo;
} LXPing, *LPLXPing;

/**
 * A response for the ping endpoint
 */
typedef struct _lx_ping_response {
	CString Status;
	CString Message;
	CString MessageCustomer;
} LXPingResponse, *LPLXPingResponse;

/**
 * Information for authorizing a cashout operation at LibertyX
 */
typedef struct _lx_cashout_auth_request {
	CString Currency;
	CString Token;
	CString LocationId;
	CString AuthorizationNonce;
	CString MaxDispense;
	CashoutMap CashDenominations;
	bool Test;

	_lx_cashout_auth_request() 
	{
#ifdef LTX_TEST_MODE
		Test = true;
#else
		Test = false;
#endif
	}
} LXDispenseAuthorization;

/**
 * A response from the API in response to a cashout authorization request
 */
typedef struct _lx_cashout_auth_response {
	CString Status;
	CString MessageCustomer;
	CString Message;
	CString AuthorizationNonce;
	CString Amount;
	bool Test;

	bool IsSuccessful() const 
	{
		return Status == L"success";
	}
} LXDispenseAuthorizationResponse;

/**
 * A requst to confirm the success of a cashout operation
 */
typedef struct _lx_cashout_execute {
	CString AuthorizationNonce;
	bool Test;

	_lx_cashout_execute() 
	{
#ifdef LTX_TEST_MODE
		Test = true;
#else
		Test = false;
#endif
	}
} LXDispenseExecute;

/**
 * The response from an execution response
 */
typedef struct _lx_cashout_execute_response {
	CString AuthorizationNonce;
	CString Status;
	CString MessageCustomer;
	CString Message;
	bool Test;

	LXSIGNATUREALGO SignatureAlgorithm;
	CList<CString, CString&> CertChainA;
	CList<CString, CString&> CertChainB;
	CList<CString, CString&> CertChainC;
	CString SignatureA;
	CString SignatureB;
	CString SignatureC;

	int GetVerificationData(char *buffer, char *token, char *locationId, double amount) const {
		/*

		Signature validation
		form the message as a string catenation of the following utf-8 string values:
		"LTX_DISPENSE_EXEC_V1"            -- fixed width
		authorize_request.token           -- must be at least 6 bytes long, and not not more than 36 bytes
		authorize_request.location_id     -- must be at least 6 bytes long, and not not more than 36 bytes
		authorize_request.authorize_nonce -- must be 36 bytes exactly
		authorize_response.amount         -- must be 10 bytes, formated as NNNNNNN.NN padded with zeroes at the start as needed
		dispense_type                     -- must be "cash"
		"LTX_DISPENSE_EXEC_V1_END"        -- fixed width
		*/

		int authNonceLen = AuthorizationNonce.GetLength();
		int len = strlen(LTX_DISPENSE_SIG_V1_start) + strlen(token) + strlen(locationId) + authNonceLen + 10 + 4 + strlen(LTX_DISPENSE_SIG_V1_end) + 1;
		if (buffer != NULL) 
		{
			char *authNonce = new char[authNonceLen + 1]();
			WideToMulti(authNonce, AuthorizationNonce, authNonceLen);

			sprintf_s(buffer, len, "%s%s%s%s%010.2fcash%s",
				LTX_DISPENSE_SIG_V1_start,
				token,
				locationId,
				authNonce,
				amount,
				LTX_DISPENSE_SIG_V1_end
			);

			memset(authNonce, 0, authNonceLen);
			delete [] authNonce;
		}

		return len;
	}

	bool IsSuccessful() const 
	{
		return Status == L"success";
	}

} LXDispenseExecuteResponse;

/**
 * The report to the LX API reporting the transaction status
 */
typedef struct _lx_cashout_transaction_report {
	CString AuthorizationNonce;
	CString Status;

	bool	HasError;
	LXError	Error;
} LXDispenseTransactionReport;

/**
 * The response from the server acknowledging the dispense
 */
typedef struct _lx_cashout_transaction_report_response {
	CString Status;

	bool IsSuccessful() const 
	{
		return Status == L"success";
	}
} LXDispenseReportResponse;

/**
 * The report to the LX API canceling the transaction
 */
typedef struct _lx_cashout_cancel {
	CString AuthorizationNonce;

	bool	HasError;
	LXError	Error;
} LXDispenseCancel;

/**
 * A response from the dispense cancellation endpoint
 */
typedef struct _lx_cashout_cancel_response {
	CString Status;
	CString Message;
	CString MessageCustomer;

	bool IsSuccessful() const 
	{
		return Status == L"success";
	}
} LXDispenseCancelResponse;