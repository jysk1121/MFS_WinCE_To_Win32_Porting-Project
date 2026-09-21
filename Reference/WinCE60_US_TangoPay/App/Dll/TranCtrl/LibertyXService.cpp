#include "stdafx.h"
#include ".\Tran\LibertyXService.h"
#include ".\Common\NHDbgApi.h"
#include ".\Common\Crypto.h"
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define CLIENT_TYPE_HEADER		L"X-Client-Type: NHATM"
#define CLIENT_VERION_HEADER	L"X-Client-Version: v0.5"
// NB: This value is hard-coded per request of LibertyX
#define API_KEY_HEADER			L"X-Api-Key: 2219be6986986277edc899e05935f68ab106d47db03d18330243e020a72a1821"
#define API_TID_HEADER_NAME		L"X-LibertyX-Tid"

// NB: At this time, LTX does not support Spanish translations. Once, they request that feature, 
//     we should move this into the APText.
#define RECEIPT_STRING			L"\n" \
								L"Thanks for using LibertyX, America's\n" \
								L"oldest and largest network of bitcoin\n" \
								L"ATMs and cashiers.\n\n" \
								L"You can view your receipt including\n" \
								L"transaction details under the\n" \
								L"<Transactions> menu.\n\n" \
								L"For all LibertyX and bitcoin purchase\n" \
								L"questions please contact LibertyX\n" \
								L"directly.\n\n" \
								L"Email: support@libertyx.com\n" \
								L"Call: (800) 511-8940\n" \
								L"Live-chat: Use chat icon at the bottom\n"\
								L"right corner of the screen on\n" \
								L"libertyx.com or under the <Support> menu\n" \
								L"in the LibertyX app.\n"


#define DISPENSE_RECEPT_STRING	L"        LibertyX Sell Bitcoin\n" \
								L"         for Cash Withdrawal\n\n" \
								L"TERMINAL #: %s\n" \
								L"DATE/TIME: %s\n" \
								L"TRANSACTION: %s\n" \
								L"ORDER NUMBER: %s\n" \
								L"REQUESTED %s AMOUNT: $%d\n" \
								L"DISPENSED %s AMOUNT: $%d\n" \
								L"\n" \
								L"JNL #: %d\n" \
								L"\n"
#define DISPENSE_RECEIPT_SUCC	L"TRANSACTION SUCCESSFUL\n"

#define DISPENSE_RECEIPT_ERR	L"TRANSACTION ERROR\n" \
								L"Error Code: %s\n" \
								L"Error Message: %s\n"



#define AUTH_ROUTE		L"/realtime/api/v1/payment/authorize"
#define CANCEL_ROUTE	L"/realtime/api/v1/payment/cancel"
#define EXECUTE_ROUTE	L"/realtime/api/v1/payment/execute"
#define PING_ROUTE		L"/realtime/api/v1/heartbeat"

#define CASHOUT_AUTH_ROUTE		L"/realtime/api/v1/dispense/authorize"
#define CASHOUT_EXEC_ROUTE		L"/realtime/api/v1/dispense/execute"
#define CASHOUT_CANCEL_ROUTE	L"/realtime/api/v1/dispense/cancel"
#define CASHOUT_TXNREPORT_ROUTE	L"/realtime/api/v1/dispense/transaction-report"

#define VERIF_ERROR_TO_LX_ERROR(verif_err, chain, error) \
{ \
	switch(verif_err) \
	{ \
	case VERIFERR_SIGNATURE_ERR: \
		error = LXERR_CHAIN ##chain ##FAIL; \
		break; \
	case VERIFERR_VALIDATION_ERR: \
		error = LXERR_PROCERR; \
		break; \
	case VERIFERR_CHAIN_ERR: \
		error = LXERR_CHAIN ##chain ##SUB; \
		break; \
	default: \
		error = LXERR_PROCERR; \
		break; \
	} \
}

typedef int t_verif_err;
const t_verif_err VERIFERR_CHAIN_ERR		= 1;
const t_verif_err VERIFERR_VALIDATION_ERR	= 2;
const t_verif_err VERIFERR_SIGNATURE_ERR	= 3;
const t_verif_err VERIFERR_UNKNOWN_ERR		= 4;

// The signing certificates for the A and B chain may change at any time. The root certifications will not and thus should be pinned as trusted in code.
/*
 * Development Certs
 */
#define CERT_CHAIN_A_ROOT_TEST	"-----BEGIN CERTIFICATE-----\n" \
								"MIIBfzCCASWgAwIBAgIUWXBH43TmlehQeO2eQAIsdeCxaD4wCgYIKoZIzj0EAwIw\n" \
								"FDESMBAGA1UEAwwJQV9URVNUX0NBMCAXDTIwMTExMjE2MDM0NloYDzIxMjAxMDE5\n" \
								"MTYwMzQ2WjAUMRIwEAYDVQQDDAlBX1RFU1RfQ0EwVjAQBgcqhkjOPQIBBgUrgQQA\n" \
								"CgNCAASYJHdDMGgDc1Aq+kSmBzAswpzlJekU5/j8oSVA/24yiq+ADHsC6zQ/nwc0\n" \
								"nhNe5P9gRLL6EdNEVZtxsinVFxQTo1YwVDASBgNVHRMBAf8ECDAGAQH/AgEAMB0G\n" \
								"A1UdDgQWBBQMMB5XYlRvVIU9eMi7pRD4CA9ELjAfBgNVHSMEGDAWgBQMMB5XYlRv\n" \
								"VIU9eMi7pRD4CA9ELjAKBggqhkjOPQQDAgNIADBFAiBCRQhGh0Z3T3dcJINJbrRg\n" \
								"ox6af4RcVk5qiI3AMqtzjgIhAI27IcpnZy8bd0OZlVLb4HuBYPDY31paIc0EQk3W\n" \
								"LU+N\n" \
								"-----END CERTIFICATE-----"

#define CERT_CHAIN_B_ROOT_TEST	"-----BEGIN CERTIFICATE-----\n" \
								"MIIBfzCCASWgAwIBAgIUa31rn/pOK3w50ENKlz/3R66HccgwCgYIKoZIzj0EAwIw\n" \
								"FDESMBAGA1UEAwwJQl9URVNUX0NBMCAXDTIwMTExMjE2MDM0NloYDzIxMjAxMDE5\n" \
								"MTYwMzQ2WjAUMRIwEAYDVQQDDAlCX1RFU1RfQ0EwVjAQBgcqhkjOPQIBBgUrgQQA\n" \
								"CgNCAASLEuxSVEJ6SU75LXIieK69ePEyXeyTqcd5cifsccD6PNUHeXfjzqwoKvN7\n" \
								"McTztjewt+kPrBYOSw9eXU1xM77lo1YwVDASBgNVHRMBAf8ECDAGAQH/AgEAMB0G\n" \
								"A1UdDgQWBBSINwlTc5If8z0DCI930czVUL7XtDAfBgNVHSMEGDAWgBSINwlTc5If\n" \
								"8z0DCI930czVUL7XtDAKBggqhkjOPQQDAgNIADBFAiAoGbqQ708zIdMVPG9zwv6j\n" \
								"AZw4I4y6G0H0aqKm4kAusgIhAMzmmJwGBc2S8zp85j/RUQK3z28SJMczBSTJ8maE\n" \
								"Hun/\n" \
								"-----END CERTIFICATE-----"

#define CERT_CHAIN_C_ROOT_TEST	"-----BEGIN CERTIFICATE-----\n" \
								"MIIBgDCCASWgAwIBAgIUZS6qr6eCG1oxjX+sFoqW9qNz+0EwCgYIKoZIzj0EAwIw\n" \
								"FDESMBAGA1UEAwwJQ19URVNUX0NBMCAXDTIwMTExMjE2MDM0NloYDzIxMjAxMDE5\n" \
								"MTYwMzQ2WjAUMRIwEAYDVQQDDAlDX1RFU1RfQ0EwVjAQBgcqhkjOPQIBBgUrgQQA\n" \
								"CgNCAAQIbbZPcAbSVFEO7KWunNa76QjliIN5sIaghdryp0XsNfRHaG1IA/fmc6qA\n" \
								"5Hr6uzEroFnk8CakoWEtVKc6L81ko1YwVDASBgNVHRMBAf8ECDAGAQH/AgEAMB0G\n" \
								"A1UdDgQWBBTn9feiL+R8C7ammcxvLvtHJ+11JzAfBgNVHSMEGDAWgBTn9feiL+R8\n" \
								"C7ammcxvLvtHJ+11JzAKBggqhkjOPQQDAgNJADBGAiEArFwgkLlZYHgzzw2oABgc\n" \
								"0H7DxZg/r7V3jScffzLggnsCIQCG/6MeWvAwI/cYiZNk+VHQ9XR7Ln1LxsAY5NMj\n" \
								"sIgy/g==\n" \
								"-----END CERTIFICATE-----"

/*
 * Production Certs
 */
#define CERT_CHAIN_A_ROOT	"-----BEGIN CERTIFICATE-----\n" \
							"MIIBdDCCARugAwIBAgIUH/YxZRPVMRdD846KQTSjGsp9NPYwCgYIKoZIzj0EAwIw\n" \
							"DzENMAsGA1UEAwwEQV9DQTAgFw0yMDExMTAyMTUwNTJaGA8yMTIwMTAxNzIxNTA1\n" \
							"MlowDzENMAsGA1UEAwwEQV9DQTBWMBAGByqGSM49AgEGBSuBBAAKA0IABKrWR58u\n" \
							"15H3DUHTeTKwSqY3+QPRx1gbdNSEtowzJfN3N+3FCLzB+w+RHW+im8jyz3kjyQBv\n" \
							"+FawXfNRhQUtbF6jVjBUMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFAKA\n" \
							"8h9Jnh7pcRF9RYF4DQG0XWY/MB8GA1UdIwQYMBaAFAKA8h9Jnh7pcRF9RYF4DQG0\n" \
							"XWY/MAoGCCqGSM49BAMCA0cAMEQCIANsUJgIgX9y8+Lk9fPWEndbOwmzP5DfGLzJ\n" \
							"lLHhY6Z2AiB+d/l0glUF5JZoL5fCPGJxZfn0xmUEsv7cxMNCrilJdg==\n" \
							"-----END CERTIFICATE-----"

#define CERT_CHAIN_B_ROOT	"-----BEGIN CERTIFICATE-----\n" \
							"MIIBdTCCARugAwIBAgIUNltseS7hKhGIA5+UY37MOO2hhM8wCgYIKoZIzj0EAwIw\n" \
							"DzENMAsGA1UEAwwEQl9DQTAgFw0yMDExMTIwMTQ4MzRaGA8yMTIwMTAxOTAxNDgz\n" \
							"NFowDzENMAsGA1UEAwwEQl9DQTBWMBAGByqGSM49AgEGBSuBBAAKA0IABCzcDCJi\n" \
							"rclEGGBpz6I9vNFa7CDWstyyzilVODJuoVPByzE1nTQgWJZJQPuxjs/3rl9JOleo\n" \
							"8YXYie2+aWFepImjVjBUMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFKep\n" \
							"acYhUWjon2QSrpX+VazJn74FMB8GA1UdIwQYMBaAFKepacYhUWjon2QSrpX+VazJ\n" \
							"n74FMAoGCCqGSM49BAMCA0gAMEUCIQCwZdFli7hsfkoBebrDPQprpTE/uDX3wrYA\n" \
							"4f0SO4TujgIgPlKUNFTRrWxf0BphHO/VtFAxlC380W8rfL9CZI86Efk=\n" \
							"-----END CERTIFICATE-----"

#define CERT_CHAIN_C_ROOT	"-----BEGIN CERTIFICATE-----\n" \
							"MIIBazCCARCgAwIBAgIJAKiHtMKPdJ/6MAoGCCqGSM49BAMCMA8xDTALBgNVBAMM\n" \
							"BENfQ0EwIBcNMjAxMTExMTc0NTQ5WhgPMjEyMDEwMTgxNzQ1NDlaMA8xDTALBgNV\n" \
							"BAMMBENfQ0EwVjAQBgcqhkjOPQIBBgUrgQQACgNCAARpewytnTLAL5NYuwbdEsEA\n" \
							"XcPUNWCXY+cDPZBCHtN8qrt/QbDuCyMWddoybAsM/lINtz+nQzH9PxojdyOkqgYC\n" \
							"o1YwVDASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBQcbHMwFfa77pjBHhw8\n" \
							"D4tamnZ4BDAfBgNVHSMEGDAWgBQcbHMwFfa77pjBHhw8D4tamnZ4BDAKBggqhkjO\n" \
							"PQQDAgNJADBGAiEA5yAIVgeufgzR2B/YtTO3n109RWIgUwRYyCc3VsfzzPECIQCC\n" \
							"I1ea8K48iLUnQUcnPzQFi37WaaBgU6zFJeS/yPi+6Q==\n" \
							"-----END CERTIFICATE-----"


#define OPENSSL_PRINT_ERR {int e=ERR_get_error();NHERROR((L"Openssl_error: %S\r\n", ERR_error_string(e, NULL)));}
#define OPENSSL_PRINT_X509_ERR(ctx) {int e=X509_STORE_CTX_get_error(ctx);NHERROR((L"Verification error: %S\r\n", X509_verify_cert_error_string(e)));}

bool verify_data(const unsigned char *hash, size_t hash_len, const unsigned char *signature, X509 *cert);
bool verify_data(const unsigned char *hash, size_t hash_len, const unsigned char *signature, X509 *cert)
{
	bool result = false;
	EC_KEY *ecpkey = NULL;
	EVP_PKEY *pubkey = NULL;
	ECDSA_SIG *sig = ECDSA_SIG_new();
	BIGNUM *r = BN_secure_new();
	BIGNUM *s = BN_secure_new();

	pubkey = X509_get_pubkey(cert);
	if (pubkey == NULL) 
	{
		NHERROR((L"Cannot find pub key in X509 container\r\n"));
		goto cleanup;
	}

	ecpkey = EVP_PKEY_get1_EC_KEY(pubkey);
	if (ecpkey == NULL) 
	{
		NHERROR((L"Cannot parse an EC pub key from the key\r\n"));
		goto cleanup;
	}

	if (!EC_KEY_check_key(ecpkey))
	{
		NHERROR((L"ECC Public key failed validation\r\n"));
		goto cleanup;
	}

	// LTX formats the signature as R || S, where R and S are 32 bytes
	BN_bin2bn(signature     , 32, r);
	BN_bin2bn(signature + 32, 32, s);

	// Setup the signature algorithm
	if (ECDSA_SIG_set0(sig, r, s) == 0)
	{
		NHERROR((L"Failed to set parameters for signature verification\r\n"));
		goto cleanup;
	}

	// Verify the data!
	if (ECDSA_do_verify(hash, hash_len, sig, ecpkey) != 1)
	{
		NHERROR((L"Signature verification failed\r\n"));
		goto cleanup;
	}

	result = true;

cleanup:
	if (pubkey) EVP_PKEY_free(pubkey);
	if (ecpkey) EC_KEY_free(ecpkey);
	if (sig)	ECDSA_SIG_free(sig);
	
	BN_clear_free(r); // Safe for NULL pointers
	BN_clear_free(s);

	return result;
}

t_verif_err validate_chain_and_signature(CString b64signature, char* data, CList<CString, CString&> *chain, X509 *root);
t_verif_err validate_chain_and_signature(CString b64signature, char* data, CList<CString, CString&> *chain, X509 *root)
{
	t_verif_err result = VERIFERR_UNKNOWN_ERR;
	unsigned int hash_size = hash_data_sha256(NULL, 0, NULL);
	unsigned char *hash = new unsigned char[hash_size]();
	int dataSize = strlen(data);
	int signature_len = 0;
	unsigned char *signature = NULL;
	X509 *leaf = NULL;

	signature = alloc_base64_decoded_string(b64signature, &signature_len);

	// 1. Data validation
	if (signature_len / 2 != hash_size)
	{
		NHERROR((L"Signature is of an incorrect length\r\n"));
		result = VERIFERR_VALIDATION_ERR;
		goto cleanup;
	}

	if (chain->GetCount() == 0)
	{
		NHERROR((L"Cert chain has no leaf certificates\r\n"));
		result = VERIFERR_VALIDATION_ERR;
		goto cleanup;
	}

	// 2. Verify Chain
	if (!verify_cert_chain(chain, root))
	{
		NHERROR((L"Chain verification for chain A failed\r\n"));
		result = VERIFERR_CHAIN_ERR;
		goto cleanup;
	}

	// 3. Verify Message
	// Get has for input into the ds verification
	hash_data_sha256(data, dataSize, hash);
	leaf = alloc_x509_pem_cstring(chain->GetHead());
	if (!verify_data(hash, hash_size, signature, leaf))
	{
		NHERROR((L"Verification of data on chain has failed\r\n"));
		result = VERIFERR_SIGNATURE_ERR;
		goto cleanup;
	}

	// Everything passed!
	result = 0;

cleanup:
	if (leaf) X509_free(leaf);

	if (hash)
	{
		memset(hash, 0, hash_size);
		delete [] hash;
	}

	if (signature) 
	{
		memset(signature, 0, signature_len);
		delete [] signature;
	}

	return result;
}


CLibertyXService::CLibertyXService(LXConfiguration config)
{ 
	this->config = config;

	isMyHttp = true;

	auth = new CHTTPBasicAuthenticator(config.Username, config.Password);
	this->http = new HTTP(config.APIUrl, auth);
}

CLibertyXService::CLibertyXService(LXConfiguration config, HTTP *http)
{
	this->config = config;

	isMyHttp = false;
	this->http = http;
}

CLibertyXService::~CLibertyXService(void)
{
	// Only delete the HTTP service, if we created it
	if (isMyHttp)
	{
		if (http != NULL) delete http;
		http = NULL;

		if (auth != NULL) delete auth;
		auth = NULL;
	}
}

/** 
 * "Call this when a customer attempts to deposit cash to LibertyX. You supply the customerï¿½s
 * information and payment amount. LibertyX returns a payment_id." - LibertyX
 * @param authorization[in] The data to authorize
 * @param result[out] The result of the authorization
 * @returns true when the operation is successful in retrieving a response from LibertyX
 */
bool CLibertyXService::AuthorizePayment(LXAuthorization authorization, LXAuthorizationResponse &result)
{
	NHDEBUG(DBG_CALL, (L"Authorizing LX payment\r\n"));

	CString requestData = this->SerializeAuthorization(authorization);

	bool success;
	CString response = this->MakeRequest(AUTH_ROUTE, requestData, &success);
	if (!success)
	{
		NHERROR((L"Failed to make authorization request\r\n"));
		return false;
	}

	if (!this->DeserializeAuthorizationResponse(response, result))
	{
		NHERROR((L"Failed to deserialize authorization result\r\n"));
		return false;
	}

	return true;
}

/**
 * "Call this to cancel an authorized transaction, and/or report an error with payment
 * processing." - LibertyX
 * @param cancel[in] The data containing the txn to cancel
 * @param result[out] The result of the cancellation
 * @returns true when the operation is successful in retrieving a response from LibertyX
 */
bool CLibertyXService::CancelPayment(LXCancel cancel, LXCancelResponse &result)
{
	NHDEBUG(DBG_CALL, (L"Cancelling LX payment\r\n"));

	CString requestData = this->SerializeCancellation(cancel);

	bool success;
	CString response = this->MakeRequest(CANCEL_ROUTE, requestData, &success);
	if (!success)
	{
		NHERROR((L"Failed to make cancellation request\r\n"));
		return false;
	}

	if (!this->DeserializeCancellationResponse(response, result))
	{
		NHERROR((L"Failed to deserialize cancellation result\r\n"));
		return false;
	}

	return true;
}

/**
 * "Call this once the cashier accepts payment from the customer. Immediately after 
 * the payment is successfully executed, we notify the customer that the payment is
 * available through our service." - LibertyX
 * @param execute[in] The data about the payment to confirm
 * @param result[out] The result of the payment confirmation
 * @returns true when the operation is successful in retrieving a response from LibertyX
 */
bool CLibertyXService::ExecutePayment(LXExecute execute, LXExecuteResponse &result)
{
	NHDEBUG(DBG_CALL, (L"Executing LX payment\r\n"));

	CString requestData = this->SerializeExecute(execute);

	bool success;
	CString response = this->MakeRequest(EXECUTE_ROUTE, requestData, &success);
	if (!success)
	{
		NHERROR((L"Failed to make execute request\r\n"));
		return false;
	}

	NHDEBUG(DBG_CALL, (L"LTX Response data: %s\r\n", response));

	if (!this->DeserializeExecuteResponse(response, result))
	{
		NHERROR((L"Failed to deserialize execute result\r\n"));
		return false;
	}

	return true;
}

/**
 * "Call this once per hour, or once per day if bandwidth is extremely limited. This call does require authorization, and can be used to check that the api configuration is correct. 
 * This is important to help us detect ATM technical issues and so we can keep our map of active locations accurate and up to date for our customers.
 * This call could also be used to confirm setup is valid without attempting a transaction or sale."

 * @param result[out] The response from the API call
 * @return true when the operation is successful
 */
bool CLibertyXService::Ping(LXPingResponse &pong)
{
	NHDEBUG(DBG_CALL, (L"Ping LTX!\r\n"));

	// Request
	LXPing ping;
	ping.LocationID = config.LocationID;
	ping.TID = config.LocationID;
	ping.AtmTID = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID).Trim();

#ifdef AP_VERSION
	ping.SoftwareVersion = AP_VERSION;
#endif

	CString request = this->SerializePing(ping);

	bool result;
	CString response = this->MakeRequest(PING_ROUTE, request, &result);
	if (!result)
	{
		NHERROR((L"Failed to ping LTX\r\n"));
		return false;
	}

	NHDEBUG(DBG_CALL, (L"LTX Response data: %s\r\n", response));

	if (!this->DeserializePingResponse(response, pong))
	{
		NHERROR((L"Failed to deserialize ping result\r\n"));
		return false;
	}

	return true;
}

/**
 * "Call this once per hour, or once per day if bandwidth is extremely limited. This call does require authorization, and can be used to check that the api configuration is correct. 
 * This is important to help us detect ATM technical issues and so we can keep our map of active locations accurate and up to date for our customers.
 * This call could also be used to confirm setup is valid without attempting a transaction or sale."
 * @param ping[in] Dispense information
 * @param pong[out] The response from the API call
 * @return true when the operation is successful
 */
bool CLibertyXService::PingDispense(LXPing &ping, LXPingResponse &pong)
{
	NHDEBUG(DBG_CALL, (L"Ping LTX!\r\n"));

	// Request
	ping.LocationID = config.LocationID;
	ping.TID = config.LocationID;
	ping.AtmTID = MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID).Trim();

#ifdef AP_VERSION
	ping.SoftwareVersion = AP_VERSION;
#endif
	ping.HasDispenseInfo = true;

	CString request = this->SerializePing(ping);

	bool result;
	CString response = this->MakeRequest(PING_ROUTE, request, &result);
	if (!result)
	{
		NHERROR((L"Failed to ping LTX\r\n"));
		return false;
	}

	NHDEBUG(DBG_CALL, (L"LTX Response data: %s\r\n", response));

	if (!this->DeserializePingResponse(response, pong))
	{
		NHERROR((L"Failed to deserialize ping result\r\n"));
		return false;
	}

	return true;
}

/**
 * Writes an entry describing the transaction to the journal.
 * @param mgr[in] an instance of the journal manager
 * @param authResult[in] the result of the LTX authorization
 * @param execute[in] the execution request of the payment execution
 * @param execResult[in] the result of the execution of the payment
 * @param errorCode[in] the application error code
 * @returns true if the operation was successful
 */
bool CLibertyXService::WriteJournalEntry(CJnlMgr *mgr, LXAuthorization auth, LXAuthorizationResponse authResult, LXExecute execute, LXExecuteResponse execResult, CString errorCode)
{
	CString journalData;

	// 1. Terminal ID
	journalData.AppendFormat(L"%s", config.LocationID);

	// 2. Routing ID
	journalData.AppendFormat(L"^%s", config.RoutingID);

	// 3. API Host
	journalData.AppendFormat(L"^%s", config.APIUrl);

	// 4. TLS y/n
	journalData.AppendFormat(L"^%s", config.TlsEnabled ? L"TLS" : L"NOTLS");

	// 5. Payment ID
	journalData.AppendFormat(L"^%s", execute.PaymentId);

	// 6. Order ID
	journalData.AppendFormat(L"^%s", auth.Token);

	// 7. Amount
	journalData.AppendFormat(L"^%.2f", auth.SourceAmount);

	// 8-9. Authorization Result
	journalData.AppendFormat(L"^%s", authResult.Status);
	journalData.AppendFormat(L"^%s", authResult.Message);

	// 10. Debit Auth Code
	if ( !authResult.Status.IsEmpty() && !authResult.HasError())
	{
		journalData.AppendFormat(L"^%s/Seq.# %04d", execute.Card.Authorization, MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_LIBERTYX_SEQUENCE, 1));
	}
	else
	{
		journalData.AppendFormat(L"^%s/Seq.#", execute.Card.Authorization);
	}

	// 11-12. Card Data (Last 4, Name)
	journalData.AppendFormat(L"^%s", execute.Card.Last4);
	journalData.AppendFormat(L"^%s", execute.Card.Name);

	// 13. Error Code
	if (!errorCode.IsEmpty())
	{
		journalData.AppendFormat(L"^%7.7s", errorCode);
	}
	else
	{
		journalData.Append(L"^0000000");
	}

	NHDEBUG(DBG_INFO, (L"Writing Journal info: %s\r\n", journalData));

	CString txnKindCode = authResult.HasError() || execResult.HasError() ? LIBERTYX_TXN_ERROR : LIBERTYX_TXN;
	return !!mgr->Save(txnKindCode, journalData);
}

/**
 * Writes an entry describing the transaction to the journal.
 * @param mgr[in] an instance of the journal manager
 * @param auth[in] the authorization request to LTX
 * @param execResponse[in] the result of the execution of the dispense
 * @param error[in] the error code for the transaction
 * @returns the journal index
 */
int CLibertyXService::WriteDispenseJournalEntry(CJnlMgr *mgr, LXDispenseAuthorization &auth, LXDispenseAuthorizationResponse &authResponse, LXDispenseExecuteResponse &execResponse, LXError *error)
{
	CString journalData;
	X509 *cert_a = NULL;
	X509 *cert_b = NULL;
	X509 *cert_c = NULL;
	CString certPemData;
	char nameBuffer[256] = {};
	X509_NAME *subjectName = NULL;  // Not mine

	// 1. Terminal ID
	journalData.AppendFormat(L"%s", config.LocationID);

	// 2. Routing ID
	journalData.AppendFormat(L"^%s", config.RoutingID);

	// 3. API Host
	journalData.AppendFormat(L"^%s", config.APIUrl);

	// 4. TLS y/n
	journalData.AppendFormat(L"^%s", config.TlsEnabled ? L"TLS" : L"NOTLS");

	// 5. Token
	journalData.AppendFormat(L"^%s", auth.Token);

	// 6. Nonce
	journalData.AppendFormat(L"^%s", auth.AuthorizationNonce);

	// 7. Amount
	journalData.AppendFormat(L"^%s", authResponse.Amount);

	// 8-9. Authorization Result
	journalData.AppendFormat(L"^%s", authResponse.Status);
	journalData.AppendFormat(L"^%s", authResponse.Message);

	// 10-12. Execute status
	journalData.AppendFormat(L"^%s", execResponse.Status);
	journalData.AppendFormat(L"^%s", execResponse.Message);
	journalData.AppendFormat(L"^%s", execResponse.MessageCustomer);

	// 13-15. Signatures
	journalData.AppendFormat(L"^%s", execResponse.SignatureA);
	journalData.AppendFormat(L"^%s", execResponse.SignatureB);
	journalData.AppendFormat(L"^%s", execResponse.SignatureC);

	// 16. Cert A CN
	if (execResponse.CertChainA.GetCount() >= 1)
	{
		certPemData = execResponse.CertChainA.GetHead();
		cert_a = alloc_x509_pem_cstring(certPemData);

		subjectName = X509_get_subject_name(cert_a); // Not mine
		X509_NAME_oneline(subjectName, nameBuffer, sizeof(nameBuffer));
	
		journalData.AppendFormat(L"^%S", nameBuffer);
	}
	else 
	{
		journalData.Append(L"^Not Present");
	}

	// 17. Cert B CN
	if (execResponse.CertChainB.GetCount() >= 1)
	{
		certPemData = execResponse.CertChainB.GetHead();
		cert_b = alloc_x509_pem_cstring(certPemData);

		subjectName = X509_get_subject_name(cert_b);  // Not mine
		X509_NAME_oneline(subjectName, nameBuffer, sizeof(nameBuffer));

		journalData.AppendFormat(L"^%S", nameBuffer);
	}
	else 
	{
		journalData.Append(L"^Not Present");
	}

	// 18. Cert C CN
	if (execResponse.CertChainC.GetCount() >= 1)
	{
		certPemData = execResponse.CertChainC.GetHead();
		cert_c = alloc_x509_pem_cstring(certPemData);

		subjectName = X509_get_subject_name(cert_c);  // Not mine
		X509_NAME_oneline(subjectName, nameBuffer, sizeof(nameBuffer));

		journalData.AppendFormat(L"^%S", nameBuffer);
	}
	else 
	{
		journalData.Append(L"^Not Present");
	}

	// 19-20. Dispense result
	journalData.AppendFormat(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
	journalData.AppendFormat(L"^%8.8s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

	// 21-22. Error
	if (error != NULL)
	{
		journalData.AppendFormat(L"^%s", error->Code);
		journalData.AppendFormat(L"^%s", error->Message);
		journalData.AppendFormat(L"^");
	}
	else 
	{
		// Empty error report
		journalData.Append(L"^0000000");
		journalData.Append(L"^");
		journalData.Append(L"^");
	}

	NHDEBUG(DBG_INFO, (L"Writing Journal info: %s\r\n", journalData));

	if (cert_a) X509_free(cert_a);
	if (cert_b) X509_free(cert_b);
	if (cert_c) X509_free(cert_c);
	
	mgr->Save(LIBERTYX_TXN_DISPENSE, journalData);
	return mgr->GetCurrentIndex();
}

/**
 * Verifies and initiates the cashout operation with the data supplied by the customer
 * @param authorization[in] The authorization request
 * @param result[out] The result of the operation
 * @return true when the operation is successful
 */
bool CLibertyXService::AuthorizeDispense(LXDispenseAuthorization &authorization, LXDispenseAuthorizationResponse &result)
{
	NHDEBUG(DBG_CALL, (L"Authorizing LX dispense\r\n"));

	CString requestData = this->SerializeDispenseAuthorization(authorization);

	bool success;
	CString response = this->MakeRequest(CASHOUT_AUTH_ROUTE, requestData, &success);
	if (!success)
	{
		NHERROR((L"Failed to make dispense authorization request\r\n"));
		return false;
	}

	NHDEBUG(DBG_CALL, (L"LTX Response data: %s\r\n", response));

	if (!this->DeserializeDispenseAuthorizationResponse(response, result))
	{
		NHERROR((L"Failed to deserialize dispense authorization result\r\n"));
		return false;
	}

	return true;
}

/**
 * Reports to LTX that the ATM will attempt to dispense the transaction.
 * @param execute[in] The execute request
 * @param result[out] The response from LTX with the authorization data
 * @return true when the operation is successful
 */
bool  CLibertyXService::ExecuteDispense(LXDispenseExecute &execute, LXDispenseExecuteResponse &result)
{
	NHDEBUG(DBG_CALL, (L"Executing LX dispense\r\n"));

	CString requestData = this->SerializeDispenseExecute(execute);

	bool success;
	CString response = this->MakeRequest(CASHOUT_EXEC_ROUTE, requestData, &success);
	if (!success)
	{
		NHERROR((L"Failed to make dispense execute request\r\n"));
		return false;
	}

	// Enabling this will cause an overflow on the console buffer
	//NHDEBUG(DBG_CALL, (L"LTX Response data: %s\r\n", response));

	if (!this->DeserializeDispenseExecuteResponse(response, result))
	{
		NHERROR((L"Failed to deserialize dispense execute result\r\n"));
		return false;
	}

	return true;
}

/**
 * Cancels the dispense transaction, and informs LTX that the dispense will not occur.
 * @param cancel[in] The cancel request with information about the error
 * @param result[out] The result of the cancel operation
 * @return true when the operation is successful
 */
bool  CLibertyXService::CancelDispense(LXDispenseCancel &cancel, LXDispenseCancelResponse &result)
{
	NHDEBUG(DBG_CALL, (L"Canceling LX dispense\r\n"));

	CString requestData = this->SerializeDispenseCancel(cancel);

	bool success;
	CString response = this->MakeRequest(CASHOUT_CANCEL_ROUTE, requestData, &success);
	if (!success)
	{
		NHERROR((L"Failed to make dispense cancel request\r\n"));
		return false;
	}

	NHDEBUG(DBG_CALL, (L"LTX Response data: %s\r\n", response));

	if (!this->DeserializeDispenseCancelResponse(response, result))
	{
		NHERROR((L"Failed to deserialize dispense cancellation result\r\n"));
		return false;
	}

	return true;
}

/**
 * Informs LTX to the result of the dispense operation
 * @param report[in] The result of the dispense operation
 * @param result[out] The response from LTX
 * @return true when the operation is successful
 */
bool  CLibertyXService::CommitDispenseTransactionReport(LXDispenseTransactionReport &report, LXDispenseReportResponse &result)
{
	NHDEBUG(DBG_CALL, (L"Executing LX dispense\r\n"));

	CString requestData = this->SerializeDispenseTransactionReport(report);

	bool success;
	CString response = this->MakeRequest(CASHOUT_TXNREPORT_ROUTE, requestData, &success);
	if (!success)
	{
		NHERROR((L"Failed to make dispense report request\r\n"));
		return false;
	}

	NHDEBUG(DBG_CALL, (L"LTX Response data: %s\r\n", response));

	if (!this->DeserializeDispenseTransactionReport(response, result))
	{
		NHERROR((L"Failed to deserialize dispense report result\r\n"));
		return false;
	}

	return true;
}

/**
 * Gets the data, which should be printed on the receipt.
 * @returns a CString representing the text, which should be printed on the receipt.
 */
CString CLibertyXService::GetReceiptData()
{
	CString data(RECEIPT_STRING);
	CString delimiter(FIELD_DELIMITER);
	data.Replace(L"\n", delimiter);
	return data;
}

/**
 * Makes the receipt data for the Bitcoin sell transaction
 */
CString CLibertyXService::MakeReceiptDataForDispense(CString dispensedAmount, CString requestedAmount, CString token, int journalIdx, LXError *err)
{
	CString data;
	CString dateString;
	CString delim(FIELD_DELIMITER);
	SYSTEMTIME localTime;
	int dispensed = Asc2Int(dispensedAmount) / 100;
	int requested = Asc2Int(requestedAmount) / 100;

	// Date Date
	::GetLocalTime(&localTime);
	dateString.Format(L"%s %2.2d:%2.2d:%2.2d",
		GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear),
		localTime.wHour, localTime.wMinute, localTime.wSecond);
	
	data.AppendFormat(DISPENSE_RECEPT_STRING, 
		config.LocationID,
		dateString,
		L"SELL BITCOIN FOR CASH",
		token,
		CURRENCY_TYPE,
		requested,
		CURRENCY_TYPE,
		dispensed,
		journalIdx
	);

	if (err != NULL)
	{
		data.AppendFormat(DISPENSE_RECEIPT_ERR,
			err->Code,
			err->Message
		);
	}
	else 
	{
		data.Append(DISPENSE_RECEIPT_SUCC);
	}

	data.Append(RECEIPT_STRING);

	data.Replace(L"\n", delim);
	return data;
}

//
// Private members
//

CString CLibertyXService::CreateBasicAuthHeader(CString username, CString password)
{
	CString digest(L"");
	CString header(L"Authorization: Basic ");
	char *output = NULL;
	char *input = NULL;
	int inputLen = 0;
	long lNewBufferLen = 0;

	digest.AppendFormat(L"%s:%s", username, password);

	inputLen = digest.GetLength() + 1;
	input = new char[inputLen]();
	WideToMulti(input, digest, inputLen);

	// Length of encoded data is (4 * (nChars / 3)) - nPaddingChars
	lNewBufferLen = ((4 * inputLen / 3) + 3) & ~3;
	output = new char[lNewBufferLen + 1]();

	// Len - 1 because it will pull a null byte otherwise
	base64_encode(input, inputLen - 1, output, lNewBufferLen);

	// Serialize into a header string
	header.AppendFormat(L"%S", output);

	// Clear the buffers for security
	memset(input, 0, inputLen);
	memset(output, 0, lNewBufferLen);
	delete [] input;
	delete [] output;

	return header;
}

CString CLibertyXService::MakeRequest(CString route, CString data, bool *success)
{
	HttpResponse response;
	NHDEBUG(DBG_CALL, (L"Making request to %s\r\n", route));

	char* buffer = NULL;
	*success = false;

	// Encode the data, if present
	if (!data.IsEmpty())
	{
		int dataSize = data.GetLength();
		buffer = new char[dataSize + 1]();
		WideToMulti(buffer, data, dataSize);
	}

	// Add additional request headers
	CString headers(L"Content-Type: application/json\r\n");
	headers.AppendFormat(L"%s\r\n", API_KEY_HEADER);
	headers.AppendFormat(L"%s\r\n", CLIENT_TYPE_HEADER);
	headers.AppendFormat(L"%s\r\n", CLIENT_VERION_HEADER); 
	headers.AppendFormat(L"%s: %s\r\n", API_TID_HEADER_NAME, config.LocationID);

	HttpRequest request;
	request.Url = route;
	request.Headers = headers;
	request.Data = (unsigned char *)buffer;
	HTTPERR result = this->http->SendPostRequest(&response, request);

	// Delete the buffer, if necessary
	if (buffer != NULL) delete [] buffer;

	if (result != HTTP_NO_ERR)
	{
		NHERROR((L"Error in web request. Error code: %d\r\n", result));
		return _T("");
	}

	// If the request failed
	if (!response.StatusCodeIsSuccessful())
	{
		NHERROR((L"Error in web request. Status code not successful\r\n"));
		return _T("");
	}

	*success = true;
	return response.ResponseData;
}

bool CLibertyXService::VerifyDispenseExecutionV1Response(LXDispenseExecuteResponse &executeResponse, CString token, CString amount, LXERRORCODE &error)
{
#define AMOUNT_BUFFER_LEN 65

	bool result = false;
	t_verif_err verif_err = VERIFERR_UNKNOWN_ERR;
	int err_count = 0;
	char *inputData = NULL;;
	int dataSize = 0;
	char *tokenBuffer = alloc_string_from_unicode(token);
	char *locationId = alloc_string_from_unicode(config.LocationID);
	X509 *root_a = alloc_x509_pem(executeResponse.Test ? CERT_CHAIN_A_ROOT_TEST : CERT_CHAIN_A_ROOT);
	X509 *root_b = alloc_x509_pem(executeResponse.Test ? CERT_CHAIN_B_ROOT_TEST : CERT_CHAIN_B_ROOT);
	X509 *root_c = alloc_x509_pem(executeResponse.Test ? CERT_CHAIN_C_ROOT_TEST : CERT_CHAIN_C_ROOT);
	char amountBuffer[AMOUNT_BUFFER_LEN] = {};

	if (amount.GetLength() >= AMOUNT_BUFFER_LEN)
	{
		NHERROR((L"Amount value is too long\r\n"));
		error = LXERR_PARSE;
		goto cleanup;
	}

	WideToMulti(amountBuffer, amount, min(AMOUNT_BUFFER_LEN - 1, amount.GetLength()));

	// Data validation
	if (executeResponse.SignatureAlgorithm != DISPENSE_EXEC_V1)
	{
		NHERROR((L"Unknown algorithm type\r\n"));
		error = LXERR_PROCERR;
		goto cleanup;
	}

	// Message to verify
	dataSize = executeResponse.GetVerificationData(NULL, tokenBuffer, locationId, atof(amountBuffer));  // passing NULL to the buffer will only return the size
	inputData = new char[dataSize]();
	executeResponse.GetVerificationData(inputData, tokenBuffer, locationId, atof(amountBuffer));

	// Chain and signature A
	verif_err = validate_chain_and_signature(executeResponse.SignatureA, inputData, &executeResponse.CertChainA, root_a);
	if (verif_err != 0)
	{
		NHERROR((L"Signature validation for Chain A failed\r\n"));
		err_count++;
		VERIF_ERROR_TO_LX_ERROR(verif_err, A, error);
	}

	// Chain and signature B
	verif_err = validate_chain_and_signature(executeResponse.SignatureB, inputData, &executeResponse.CertChainB, root_b);
	if (verif_err != 0)
	{
		NHERROR((L"Signature validation for Chain B failed\r\n"));
		err_count++;
		VERIF_ERROR_TO_LX_ERROR(verif_err, B, error);
	}

	// Chain and signature C
	verif_err = validate_chain_and_signature(executeResponse.SignatureC, inputData, &executeResponse.CertChainC, root_c);
	if (verif_err != 0)
	{
		NHERROR((L"Signature validation for Chain C failed\r\n"));
		err_count++;
		VERIF_ERROR_TO_LX_ERROR(verif_err, C, error);
	}

	if (err_count > 1)
	{
		NHERROR((L"More than 2 signatures failed to validate\r\n"));
		goto cleanup;
	}

	// Everything passed!
	result = true;

cleanup:
	if (root_a) X509_free(root_a);
	if (root_b) X509_free(root_b);
	if (root_c) X509_free(root_c);

	if (tokenBuffer) delete [] tokenBuffer;
	if (locationId)	 delete [] locationId;

	if (inputData) 
	{
		memset(inputData, 0, dataSize);
		delete [] inputData;
	}

	return result;
}

//
// Static Functions
//

CString CLibertyXService::GetNewUUID()
{
	CString strUUID = L"";
	CString strTemp;

	RAND_poll();

	for(int i=0; i<16; i++)
	{
		// add 4 "-" 
		if( (i==4)||(i==6)||(i==8)||(i==10) )
			strUUID += L"-";

		// Generate 16 Random Numbers
		unsigned char btRand = NULL;
		RAND_bytes(&btRand, 1);

		// Adjust Version and Variant
		if(i==6)		btRand = ((btRand & 0x0f) | 0x40);	// Version : Make Version "4" (Random)
		else if(i==8)	btRand = ((btRand & 0x3f) + 0x80);	// Variant : Range 0x80 ~ 0xbf

		// Combine UUID
		strTemp.Format(L"%02x", btRand);
		strUUID += strTemp;
	}

	return strUUID;
}

long CLibertyXService::GetTotalTransactionAmount(const CashoutMap &totals, long withdrawalLimit)
{
	long total = 0;
	int count = 0;
	CString denomination;
	int denomValue = 0;
	POSITION pos = totals.GetStartPosition();;

	while (pos != NULL)
	{
		totals.GetNextAssoc(pos, denomination, count);

		denomValue = Asc2Int(denomination);
		if (denomValue <= 0)
		{
			continue;
		}

		total += denomValue * count;
	}

	total = min(total, CDU_MCASH_MAXOUT);
	return min(withdrawalLimit, total);
}

/**
 * Populates a ping struct with information
 */
void CLibertyXService::PopulateDispenseInformation(LXPing &ping, CDevCmn* devCmn)
{
	CString withdrawalLimitCents;
	int maxNotesOut = 0;
	long totalOutAmount = 0;
	int withdrawalLimitDollars = 0;
	LXDispenseSurcharge surcharge;
	int surchargeMode = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_ENABLE);

	withdrawalLimitCents = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT);
	withdrawalLimitDollars = Asc2Int(withdrawalLimitCents) / 100;
	CLibertyXService::PopulateCashCounts(ping.DispenseInfo.CashDenominations, devCmn);
	totalOutAmount = GetTotalTransactionAmount(ping.DispenseInfo.CashDenominations, withdrawalLimitDollars);

	ping.HasDispenseInfo = true;
	ping.DispenseInfo.MaxDispense.Format(L"%d", totalOutAmount);
	ping.DispenseInfo.MaxWithdrawalLimit.Format(L"%d", withdrawalLimitDollars);

	// Set default currency surcharge
	surcharge.FeeFlat.Format(L"%.02f", Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)) / 100.0);
	surcharge.FeePercentage = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) / 100;

	if (surchargeMode == PERCENT_MODE) 
	{
		if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == GREATER)
		{
			surcharge.SurchargeMode = LXSURCHARGEMODE_PERCENT_GREATER;
		}
		else 
		{
			surcharge.SurchargeMode = LXSURCHARGEMODE_PERCENT_LESSER;
		}
	}
	else if (surchargeMode == TABLE_MODE)
	{
		surcharge.SurchargeMode = LXSURCHARGEMODE_PERCENT_TIERED;
	}
	else 
	{
		surcharge.SurchargeMode = LXSURCHARGEMODE_FLAT;
	}

	ping.DispenseInfo.SurchargeInfos.SetAt(CURRENCY_TYPE, surcharge);

#ifdef MULTI_CURRENCY_TYPE
	if (CURRENCY_TYPE != MULTI_CURRENCY_TYPE)
	{
		ping.DispenseInfo.SurchargeInfos.SetAt(MULTI_CURRENCY_TYPE, surcharge);
	}
#endif
}

void CLibertyXService::PopulateCashCounts(CashoutMap &map, CDevCmn* devCmn)
{
	for (int i = 0; i < CDU_MAX_CST_COUNT; i++)
	{
		int denomination = devCmn->fnCDU_GetValueOfCash(i+1);
		int count = devCmn->fnCDU_GetNumberOfCash(i+1);

		if (denomination == 0)
		{
			continue;
		}

		CString denomString;
		denomString.Format(L"%d", denomination);
		map.SetAt(denomString, count);
	}
}