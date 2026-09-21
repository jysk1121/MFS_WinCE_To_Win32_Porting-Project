#pragma once

#include "stdafx.h"
#include ".\Net\HTTP.h"
#include ".\Common\LibertyXConfigurationManager.h"
#include ".\Tran\LibertyXTypes.h"
#include ".\Dev\JnlMgr.h"
#include ".\Dev\Devcmn.h"

#include <parson/parson.h>
#pragma comment(lib, "parson.lib")

class AFX_CLASS_EXPORT CLibertyXService
{
public:
	CLibertyXService(LXConfiguration config);
	CLibertyXService(LXConfiguration config, HTTP* http);
	~CLibertyXService(void);

	/** 
	 * "Call this when a customer attempts to deposit cash to LibertyX. You supply the customer’s
	 * information and payment amount. LibertyX returns a payment_id." - LibertyX
	 * @param authorization[in] The data to authorize
	 * @param result[out] The result of the authorization
	 * @returns true when the operation is successful in retrieving a response from LibertyX
	 */
	bool AuthorizePayment(LXAuthorization authorization, LXAuthorizationResponse &result);

	/**
	 * "Call this to cancel an authorized transaction, and/or report an error with payment
	 * processing." - LibertyX
	 * @param cancel[in] The data containing the txn to cancel
	 * @param result[out] The result of the cancellation
	 * @returns true when the operation is successful in retrieving a response from LibertyX
	 */
	bool CancelPayment(LXCancel cancel, LXCancelResponse &result);

	/**
	 * "Call this once the cashier accepts payment from the customer. Immediately after 
	 * the payment is successfully executed, we notify the customer that the payment is
	 * available through our service." - LibertyX
	 * @param execute[in] The data about the payment to confirm
	 * @param result[out] The result of the payment confirmation
	 * @returns true when the operation is successful in retrieving a response from LibertyX
	 */
	bool ExecutePayment(LXExecute execute, LXExecuteResponse &result);

	/**
	 * "Call this once per hour, or once per day if bandwidth is extremely limited. This call does require authorization, and can be used to check that the api configuration is correct. 
	 * This is important to help us detect ATM technical issues and so we can keep our map of active locations accurate and up to date for our customers.
	 * This call could also be used to confirm setup is valid without attempting a transaction or sale."
	 * @param pong[out] The response from the API call
	 * @return true when the operation is successful
	 */
	bool Ping(LXPingResponse &pong);

	/**
	 * "Call this once per hour, or once per day if bandwidth is extremely limited. This call does require authorization, and can be used to check that the api configuration is correct. 
	 * This is important to help us detect ATM technical issues and so we can keep our map of active locations accurate and up to date for our customers.
	 * This call could also be used to confirm setup is valid without attempting a transaction or sale."
	 * @param ping[in] Dispense information
	 * @param pong[out] The response from the API call
	 * @return true when the operation is successful
	 */
	bool PingDispense(LXPing &ping, LXPingResponse &pong);

	/**
	 * Writes an entry describing the transaction to the journal.
	 * @param mgr[in] an instance of the journal manager
	 * @param auth[in] the authorization request to LTX
	 * @param authResult[in] the result of the LTX authorization
	 * @param execute[in] the execution request of the payment execution
	 * @param execResult[in] the result of the execution of the payment
	 * @param errorCode[in] the application error code
	 * @returns true if the operation was successful
	 */
	bool WriteJournalEntry(CJnlMgr *mgr, LXAuthorization auth, LXAuthorizationResponse authResult, LXExecute execute, LXExecuteResponse execResult, CString errorCode);

	/**
	 * Writes an entry describing the transaction to the journal.
	 * @param mgr[in] an instance of the journal manager
	 * @param auth[in] the authorization request to LTX
	 * @param execResponse[in] the result of the execution of the dispense
	 * @param error[in] the error code from the transaction. If NULL, no error is recorded
	 * @returns the journal index
	 */
	int WriteDispenseJournalEntry(CJnlMgr *mgr, LXDispenseAuthorization &auth, LXDispenseAuthorizationResponse &authResponse, LXDispenseExecuteResponse &execResponse, LXError *error);

	/**
	 * Verifies and initiates the dispense operation with the data supplied by the customer
	 * @param authorization[in] The authorization request
	 * @param result[out] The result of the operation
	 * @return true when the operation is successful
	 */
	bool AuthorizeDispense(LXDispenseAuthorization &authorization, LXDispenseAuthorizationResponse &result);

	/**
	 * Reports to LTX that the ATM will attempt to dispense the transaction.
	 * @param execute[in] The execute request
	 * @param result[out] The response from LTX with the authorization data
	 * @return true when the operation is successful
	 */
	bool ExecuteDispense(LXDispenseExecute &execute, LXDispenseExecuteResponse &result);

	/**
	 * Cancels the dispense transaction, and informs LTX that the dispense will not occur.
	 * @param cancel[in] The cancel request with information about the error
	 * @param result[out] The result of the cancel operation
	 * @return true when the operation is successful
	 */
	bool CancelDispense(LXDispenseCancel &cancel, LXDispenseCancelResponse &result);

	/**
	 * Informs LTX to the result of the dispense operation
	 * @param report[in] The result of the dispense operation
	 * @param result[out] The response from LTX
	 * @return true when the operation is successful
	 */
	bool CommitDispenseTransactionReport(LXDispenseTransactionReport &report, LXDispenseReportResponse &result);

	/**
	 * Gets the data, which should be printed on the receipt.
	 * @returns a CString representing the text, which should be printed on the receipt.
	 */
	CString GetReceiptData();

	/**
	 * Makes the receipt data for the Bitcoin sell transaction
	 */
	CString MakeReceiptDataForDispense(CString dispensedAmount, CString requestedAmount, CString token, int journalIdx, LXError *err);

	/**
	 * Verifies the signatures from the LTX response
	 * @param executeResponse[in] The original execute response from LTX
	 * @param token[in] The token from the customer
	 * @param amount[in] The amount of the transaction
	 * @param error[out] The error code if the function returns false
	 * @returns false when the response is invalid
	 */
	bool VerifyDispenseExecutionV1Response(LXDispenseExecuteResponse &executeResponse, CString token, CString amount, LXERRORCODE &error);

	/**
	 * Creates a new UUID
	 */
	static CString GetNewUUID();

	static long GetTotalTransactionAmount(const CashoutMap &totals, long withdrawalLimit);

	/**
	 * Populates a ping struct with information
	 */
	static void PopulateDispenseInformation(LXPing &ping, CDevCmn* devCmn);

	static void PopulateCashCounts(CashoutMap &map, CDevCmn *devCmn);

private:
	LXConfiguration config;

	bool isMyHttp;
	HTTP *http;
	CHTTPBasicAuthenticator *auth;

	static CString CreateBasicAuthHeader(CString username, CString password);
	CString MakeRequest(CString route, CString data, bool *success);

	/* Serialization - LibertyXService_Serialization.cpp */

	JSON_Value* SerializeCard(LXCard card);
	JSON_Value*	SerializeDispensePingInformation(LXDispensePing &data);

	/* Purchase Serialization */
	CString	SerializeAuthorization(LXAuthorization authorization);
	bool	DeserializeAuthorizationResponse(CString data, LXAuthorizationResponse &response);
	CString	SerializeCancellation(LXCancel cancel);
	bool	DeserializeCancellationResponse(CString data, LXCancelResponse &response);
	CString	SerializeExecute(LXExecute execute);
	bool	DeserializeExecuteResponse(CString data, LXExecuteResponse &response);
	CString	SerializePing(LXPing &ping);
	bool	DeserializePingResponse(CString data, LXPingResponse &pong);

	/* Dispense Serialization */
	CString SerializeDispenseAuthorization(LXDispenseAuthorization &authorization);
	bool	DeserializeDispenseAuthorizationResponse(CString data, LXDispenseAuthorizationResponse &response);
	CString	SerializeDispenseExecute(LXDispenseExecute &execute);
	bool	DeserializeDispenseExecuteResponse(CString data, LXDispenseExecuteResponse &response);
	CString SerializeDispenseCancel(LXDispenseCancel &cancel);
	bool	DeserializeDispenseCancelResponse(CString data, LXDispenseCancelResponse &response);
	CString SerializeDispenseTransactionReport(LXDispenseTransactionReport &report);
	bool	DeserializeDispenseTransactionReport(CString data, LXDispenseReportResponse &response);
};
