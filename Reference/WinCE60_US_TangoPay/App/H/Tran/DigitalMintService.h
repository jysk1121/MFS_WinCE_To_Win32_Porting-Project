#pragma once

#include "stdafx.h"
#include ".\Net\HTTP.h"
#include ".\Dev\JnlMgr.h"

#include ".\Tran\DigitalMintTypes.h"
#include ".\Common\DigitalMintConfigurationManager.h"

#include <parson/parson.h>
#pragma comment(lib, "parson.lib")

class AFX_CLASS_EXPORT CDigitalMintService
{
public:
	explicit CDigitalMintService(DMConfiguration config, CDigitalMintConfigurationManager configManager, HTTP *http);

	/**
	 * "To initiate a transaction with a customer, the customer must first be identified by
	 * and enrolled into the system if they have not yet transacted with DigitalMint."
	 * @param greeting[in] the greeting request data
	 * @param response[out] The result of the greeting
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool Greet(const DMGreeting &greeting, DMGreetingResponse &response);

	/**
	 * "To initiate a transaction with a customer, the customer must first be identified by
	 * and enrolled into the system if they have not yet transacted with DigitalMint."
	 * @param greeting[in] the greeting request data
	 * @param response[out] The result of the greeting
	 * @param put[in] Whether to use PUT for the request or POST
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GreetCode(const DMGreeting &greeting, DMGreetingResponse &response, bool put = false);

	/**
	 * "Sometimes our customers forget their PIN code when standing at a kiosk. We allow a customer
	 * to re-confirm their phone number to create a new PIN and continue transacting. To request a
	 * PIN reset, send a request containing the customer's phone number via PUT to /greet/pin/reset:"
	 * @param greeting[in] the greeting request data
	 * @param response[out] The result of the greeting
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GreetReconfirm(const DMGreeting &greeting, DMGreetingResponse &response);

	/**
	 * "Sometimes our customers forget their PIN code when standing at a kiosk. We allow a customer
	 * to re-confirm their phone number to create a new PIN and continue transacting. To request a
	 * PIN reset, send a request containing the customer's phone number via PUT to /greet/pin/reset:"
	 * @param greeting[in] the greeting request data
	 * @param response[out] The result of the greeting
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GreetReset(const DMGreeting &greeting, DMGreetingResponse &response);

	/**
	 * "To initiate a transaction with a customer, the customer must first be identified by
	 * and enrolled into the system if they have not yet transacted with DigitalMint."
	 * @param enrollment[in] the greeting request data
	 * @param response[out] The result of the greeting
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GreetEnroll(const DMEnrollmentRequest &enrollment, DMEnrollmentResponse &response);

	/**
	 * "DigitalMint provides an api call that provides detailed view of the transacting customer's limits"
	 * @param accountHash[in] The account hash for which to retrieve the limit
	 * @param response[out] The account's limits
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GetAccountLimits(CString accountHash, DMAccountLimitsResponse &response);

	/**
	 * Returns a list of products available for purchase for the platform.
	 * @param response[out] List of products available for purchase
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GetProducts(DMProductsResponse &response);

	/**
	 * "The teller must use the customer's account hash to initiate a new transaction"
	 * @param request[in] Information about the transaction about to start
	 * @param response[out] Transaction information to use in subsequent requests
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool InitiateTransaction(const DMInitiateTransactionRequest &request, DMTransactionResponse &response);

	/**
	 * "Once a transaction has been initiated, proceed to scanning a customer's wallet
	 * address by submitting the following"
	 * @param transactionHash[in] The transaction session identifier
	 * @param request[in] The scanned wallet address information
	 * @param response[out] The transaction state information
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool SetWalletAddress(const CString transactionHash, const DMScanWalletRequest &request, DMTransactionResponse &response);

	/**
	 * "Obtaining price quotes requires authentication and may optionally provide a customer's
	 * account context to determine the best quote for the customer. To obtain a DigitalMint
	 * Bitcoin Buy & Sell quote, GET to /quotes/selectedCoin:"
	 * @param selectedCoin[in] The coin selected by the customer
	 * @param response[out] The quote information
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GetQuote(const CString selectedCoin, DMQuoteResponse &response);

	/**
	 * "To obtain a quote which is associated with the account, pass the account context to the
	 * quote api call with a GET to /quotes/selectedCoin?account=accountId. This quote will hold
	 * the rate throughout the transaction workflow."
	 * @param selectedCoin[in] The coin selected by the customer
	 * @param accountId[in] The account id of the customer
	 * @param response[out] The quote information
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GetQuote(const CString selectedCoin, const CString accountId, DMQuoteResponse &response);

	/**
	 * "For compliance purposes DigitalMint has provided a route to text a customer the terms of
	 * service to their phone number on file."
	 * @param accountId[in] The account id of the customer
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool SendTOS(const CString accountId);

	/**
	 * "After the system has accepted a scanned wallet address, the teller may begin collecting cash.
	 * Collecting cash may be done in multiple cash collection steps (e.g. a self-service kiosk with
	 * a bill acceptor) or in a single step with the Execute flag set to true to combine payment
	 * collection and transaction completion into one step. If the optional Receipt flag is set to
	 * true, the customer will receive a text message receipt for their transaction."
	 * @param transactionHash[in] The transaction session identifier
	 * @param request[in] The payment information collected
	 * @param response[out] Confirmation that the payment was recorded
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool CollectPayment(const CString transactionHash, const DMCollectPaymentRequest &request, DMTransactionResponse &response);

	/**
	 * "If at any time the transaction has collected payment, the teller may complete the transaction"
	 * @param transactionHash[in] The transaction session identifier
	 * @param request[in] Information describing how the transaction was completed
	 * @param response[out] Transaction state
	 * @returns  true when the operation is successful in retrieving an API response
	 */
	bool CompleteTransaction(const CString transactionHash, const DMCompleteTransactionRequest &request, DMTransactionResponse &response);

	/**
	 * "A transaction may be canceled at any time. The CancelReason flag set to an integer code
	 * representing the reason the transaction was canceled. The following table enumerates
	 * acceptable CancelReason codes."
	 * @param transactionHash[in] The transaction session identifier
	 * @param request[in] The reason for canceling the transaction
	 * @param response[out] The transaction state
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool CancelTransaction(const CString transactionHash, const DMCancelTransactionRequest &request, DMTransactionResponse &response);

	/**
	 * "DigitalMint provides a route for the kiosk to notify DigitalMint if any of the hardware state changes."
	 * @param request[in] The hardware state of the kiosk
	 * @param txnMode[in] The desired transaction mode to use for the request
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool ReportKioskState(const DMKioskStateRequest &request, DMTRANSACTIONMODE txnMode);

	/**
	 * "DigitalMint provides a route for the kiosk to notify DigitalMint if any of the specific events occur at the kiosk.
	 * This is achieved by sending a POST to /kiosk/event. Note: the platform is derived from the headers."
	 * @param request[in] The hardware event of the kiosk
	 * @param txnMode[in] The desired transaction mode to use for the request
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool ReportKioskEvent(const DMKioskEventRequest &request, DMTRANSACTIONMODE txnMode);

	/**
	 * "To register a sidecar with the DigitalMint API. You will need to get credentials on the machine.
	 * The current workflow is this:"
	 * @param code[in] A DigitalMint technician will provide the Sidecar operator with a unique QR code
	 * @param response[out] the response from the API
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool RegisterSidecar(CString code, DMDeviceRegistrationResponse &response);

	/**
	 * "To register a atm with the DigitalMint API. You will need to get credentials on the machine.
	 * The current workflow is this:"
	 * @param code[in] A DigitalMint technician will provide the ATM operator with a unique shortcode
	 * @param response[out] the response from the API
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool RegisterAtm(CString code, DMDeviceRegistrationResponse &response);

	/**
	 * Make a call to the base endpoint to retrieve the version of the API
	 * @param response[out] the response from the API
	 * @returns true when the operation is successful in retrieving an API response
	 */
	bool GetVersion(DMVersionResponse &response);

	/**
	 * Writes an entry describing the transaction to the journal.
	 * @param mgr[in] An instance of the journal manager
	 * @param sessionId[in] The transaction session identifier
	 * @param transactionId[in] The transaction identifier
	 * @param accountId[in] The account ID of the customer
	 * @param selectedCoinSymbol[in] The symbol for the coin selected by the customer
	 * @param transactionAmount[in] The amount of the current transaction
	 * @returns true if the operation was successful
	 */
	bool WriteJournalEntry(CJnlMgr *mgr, CString sessionId, CString transactionId, CString accountId, CString selectedCoinSymbol, int transactionAmount);

	/**
	 * Writes an entry describing the transaction error to the journal.
	 * @param mgr[in] An instance of the journal manager
	 * @param sessionId[in] The transaction session identifier
	 * @param transactionId[in] The transaction identifier
	 * @param accountId[in] The account ID of the customer
	 * @param selectedCoinSymbol[in] The symbol for the coin selected by the customer
	 * @param transactionAmount[in] The amount of the current transaction
	 * @param errorCode[in] The application error code
	 * @returns true if the operation was successful
	 */
	bool WriteErrorJournalEntry(CJnlMgr *mgr, CString sessionId, CString transactionId, CString accountId, CString selectedCoinSymbol, int transactionAmount, CString errorCode);

	/**
	 * Set the Transaction Mode object
	 * @param transactionMode[in] The transaction mode
	 */
	void SetTransactionMode(DMTRANSACTIONMODE transactionMode);

	/**
	 * Get the Transaction Mode object
	 * @returns The transaction mode
	 */
	DMTRANSACTIONMODE GetTransactionMode();

	/**
	 * Checks if the HTTP service is valid and has a valid hostname
	 * @returns Whether the HTTP service is valid and has a valid hostname
	 */
	bool VerifyService();

private:
	bool isMyHttp;
	HTTP *http;

	DMConfiguration						config;
	CDigitalMintConfigurationManager*	configManager;
	DMTRANSACTIONMODE					transactionMode;

	HttpResponse	MakeRequest(CString route, CString data, HTTP_METHOD method, bool authenticate, bool *success);
	HttpResponse	MakeRequest(CString route, CString data, HTTP_METHOD method, bool authenticate, DMTRANSACTIONMODE txnMode, bool *success);
	CString			GetTimestamp();
	CString			CreateSignature(CString path, CString timestamp, CString apikey, CString apiSecret);
	CString			HMACSHA256(unsigned char *key, int keylen, CString data);

	/* Serialization - DigitalMintService_Serialization.cpp */

	JSON_Value*	SerializeStateMap(DMStateMap map);
	bool		DeserializeProduct(JSON_Object *object, DMProduct &product);
	bool		DeserializeProductArray(JSON_Array *products, CList<DMProduct> &productList);

	CString		SerializeGreet(const DMGreeting &greeting);
	CString		SerializeEnrollment(const DMEnrollmentRequest &enrollment);
	bool		DeserializeGreetResponse(CString data, DMGreetingResponse &response);
	bool		DeserializeEnrollmentResponse(CString data, DMEnrollmentResponse &response);
	bool		DeserializeAccountLimitsResponse(CString data, DMAccountLimitsResponse &response);
	bool		DeserializeQuote(CString data, DMQuoteResponse &response);
	bool		DeserializeProductsResponse(CString data, DMProductsResponse &response);
	CString		SerializeIntiateTransactionRequest(const DMInitiateTransactionRequest &request);
	bool		DeserializeTransactionStateResponse(CString data, DMTransactionResponse &response);
	bool		DeserializeQuoteResponse(CString data, DMQuoteResponse &response);
	CString		SerializeScanWalletRequest(const DMScanWalletRequest &request);
	CString		SerializeCollectPaymentRequest(const DMCollectPaymentRequest &request);
	CString		SerializeCompleteTransactionRequest(const DMCompleteTransactionRequest &request);
	CString		SerializeCancelTransactionRequest(const DMCancelTransactionRequest &request);
	CString		SerializeKioskStateRequest(const DMKioskStateRequest &request);
	CString		SerializeKioskEventRequest(const DMKioskEventRequest &request);
	bool		DeserializeDeviceRegistrationResponse(CString data, DMDeviceRegistrationResponse &response);
	bool		DeserializeVersionResponse(CString data, DMVersionResponse &response);
};