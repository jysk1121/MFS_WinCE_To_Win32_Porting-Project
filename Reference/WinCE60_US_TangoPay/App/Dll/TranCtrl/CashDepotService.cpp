#include "stdafx.h"
#include ".\Tran\CashDepotService.h"
#include ".\Common\NHDbgApi.h"
#include ".\Common\Crypto.h"

#ifndef UNDER_CE
#include <math.h>
#endif

//
// Standard Flows
//

#define TERMINALID_HEADER L"TerminalId"

#define CONFIGURATIONS_ROUTE L"/api/External/Offering/GetConfigurations"

//
/// Cryptocurrency Flows
//

#define SESSIONID_HEADER L"X-Session-Id"

#define CRYPTO_ROUTE        L"/api/Crypto"
#define COINSOURCE_ROUTE    L"/api/CoinSource"

#define GET_PROVIDERS_ROUTE         CRYPTO_ROUTE        L"/GetProviders"
#define CLIENT_TOKEN_ROUTE          CRYPTO_ROUTE        L"/GetClientToken"
#define SEND_CODE_ROUTE             CRYPTO_ROUTE        L"/SendCode"
#define VERIFY_CODE_ROUTE           CRYPTO_ROUTE        L"/VerifyCode"
#define DISCLOSURE_ROUTE            CRYPTO_ROUTE        L"/Disclosure"
#define DISCLOSURE_ACCEPT_ROUTE     CRYPTO_ROUTE        L"/DisclosureAccept"
#define DAILY_LIMIT_ROUTE           CRYPTO_ROUTE        L"/DailyLimit"
#define CRYPTO_CURRENCY_ROUTE       CRYPTO_ROUTE        L"/CryptoCurrency"
#define CRYPTO_CURRENCY_PRICE_ROUTE CRYPTO_ROUTE        L"/CryptoCurrencyPrice"
#define MANUAL_WALLET_ROUTE         CRYPTO_ROUTE        L"/SendManualWalletLink"
#define GET_MANUAL_WALLET_ROUTE     COINSOURCE_ROUTE    L"/GetManualWalletAdress"
#define CHECK_MANUAL_WALLET_ROUTE   CRYPTO_ROUTE        L"/CheckManualWalletAddress"
#define BUY_ROUTE                   CRYPTO_ROUTE        L"/Buy"
#define BUY_COMPLETE_ROUTE          CRYPTO_ROUTE        L"/BuyComplete"

CCashDepotService::CCashDepotService(HTTP *http)
{
    isMyHttp = false;
    this->http = http;
}

//
// Public Functions
//

//
// Standard Flows
//

CString CCashDepotService::GetTimestamp()
{
    SYSTEMTIME time = {};
    GetSystemTime(&time);

    CString nanos;
    nanos.Format(L"%.9f", (double)time.wMilliseconds / 1000);
    nanos = nanos.Mid(2);

    CString datestring;

    datestring.Format(
        L"%d-%02d-%02dT%02d:%02d:%02d.%sZ",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        nanos);

    return datestring;
}

bool CCashDepotService::GetConfigurations(const CDConfigurations &configs, CString terminalId, CDConfigurationsResponse &response)
{
    NHDBG((L"Performing configurations API call\r\n"));

    CString requestData = this->SerializeConfigurations(configs);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(CONFIGURATIONS_ROUTE, requestData, terminalId, L"", HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make configurations request\r\n"));
        return false;
    }

    if (!this->DeserializeConfigurationsResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize configurations result\r\n"));
        return false;
    }

    return true;
}

//
/// Cryptocurrency Flows
//

bool CCashDepotService::GetProviders(CDProvidersResponse &response)
{
    NHDBG((L"Performing get providers API call\r\n"));

    bool success;
    HttpResponse httpResponse = this->MakeRequest(GET_PROVIDERS_ROUTE, L" ", L"", L"", HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make get providers request\r\n"));
        return false;
    }

    if (!this->DeserializeProvidersResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize get providers result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::GetClientToken(const CDClientTokenRequest &request, CDClientTokenResponse &response)
{
    NHDBG((L"Performing get client token API call\r\n"));

    CString requestData = this->SerializeClientToken(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(CLIENT_TOKEN_ROUTE, requestData, L"", L"", HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make get client token request\r\n"));
        return false;
    }

    if (!this->DeserializeClientTokenResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize get client token result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::SendCode(const CDSendCodeRequest &request, CString sessionId, CDSendCodeResponse &response)
{
    NHDBG((L"Performing send code API call\r\n"));

    CString requestData = this->SerializeSendCode(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(SEND_CODE_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make send code request\r\n"));
        return false;
    }

    if (!this->DeserializeSendCodeResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize send code result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::VerifyCode(const CDVerifyCodeRequest &request, CString sessionId, CDVerifyCodeResponse &response)
{
    NHDBG((L"Performing verify code API call\r\n"));

    CString requestData = this->SerializeVerifyCode(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(VERIFY_CODE_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make verify code request\r\n"));
        return false;
    }

    if (!this->DeserializeVerifyCodeResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize verify code result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::Disclosure(const CDDisclosureRequest &request, CDDisclosureResponse &response)
{
    NHDBG((L"Performing disclosure API call\r\n"));

    CString requestData = this->SerializeDisclosure(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(DISCLOSURE_ROUTE, requestData, L"", L"", HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make disclosure request\r\n"));
        return false;
    }

    if (!this->DeserializeDisclosureResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize disclosure result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::DisclosureAccept(const CDDisclosureAcceptRequest &request, CDDisclosureAcceptResponse &response)
{
    NHDBG((L"Performing disclosure accept API call\r\n"));

    CString requestData = this->SerializeDisclosureAccept(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(DISCLOSURE_ACCEPT_ROUTE, requestData, L"", L"", HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make disclosure accept request\r\n"));
        return false;
    }

    if (!this->DeserializeDisclosureAcceptResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize disclosure accept result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::DailyLimit(const CDDailyLimitRequest &request, CString sessionId, CDDailyLimitResponse &response)
{
    NHDBG((L"Performing daily limit API call\r\n"));

    CString requestData = this->SerializeDailyLimit(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(DAILY_LIMIT_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make daily limit request\r\n"));
        return false;
    }

    if (!this->DeserializeDailyLimitResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize daily limit result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::CryptoCurrency(const CDCryptoCurrencyRequest &request, CString sessionId, CDCryptoCurrencyResponse &response, CString currency)
{
    NHDBG((L"Performing cryptocurrency API call\r\n"));

    CString requestData = this->SerializeCryptoCurrency(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(CRYPTO_CURRENCY_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make cryptocurrency request\r\n"));
        return false;
    }

    if (!this->DeserializeCryptoCurrencyResponse(httpResponse.ResponseData, response, currency))
    {
        NHERROR((L"Failed to deserialize cryptocurrency result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::CryptoCurrencyPrice(const CDCryptoCurrencyPriceRequest &request, CString sessionId, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency)
{
    NHDBG((L"Performing cryptocurrency price API call\r\n"));

    CString requestData = this->SerializeCryptoCurrencyPrice(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(CRYPTO_CURRENCY_PRICE_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make cryptocurrency price request\r\n"));
        return false;
    }

    if (!this->DeserializeCryptoCurrencyPriceResponse(httpResponse.ResponseData, response, selectedCurrency))
    {
        NHERROR((L"Failed to deserialize cryptocurrency price result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::SendManualWalletLink(const CDSendManualWalletLinkRequest &request, CString sessionId, CDSendManualWalletLinkResponse &response)
{
    NHDBG((L"Performing send manual wallet link API call\r\n"));

    CString requestData = this->SerializeSendManualWalletLink(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(MANUAL_WALLET_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make send manual wallet link request\r\n"));
        return false;
    }

    if (!this->DeserializeSendManualWalletLinkResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize send manual wallet link result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::GetManualWalletAddress(const CDGetManualWalletAddressRequest &request, CString sessionId, CDGetManualWalletAddressResponse &response)
{
    NHDBG((L"Performing get manual wallet address API call\r\n"));

    CString requestData = this->SerializeGetManualWalletAddress(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(GET_MANUAL_WALLET_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make get manual wallet address request\r\n"));
        return false;
    }

    if (!this->DeserializeGetManualWalletAddressResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize get manual wallet address result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::CheckManualWalletAddress(const CDCheckManualWalletAddressRequest &request, CString sessionId, CDCheckManualWalletAddressResponse &response)
{
    NHDBG((L"Performing check manual wallet address API call\r\n"));

    CString requestData = this->SerializeCheckManualWalletAddress(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(CHECK_MANUAL_WALLET_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make check manual wallet address request\r\n"));
        return false;
    }

    if (!this->DeserializeCheckManualWalletAddressResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize check manual wallet address result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::Buy(const CDBuyRequest &request, CString sessionId, CDBuyResponse &response)
{
    NHDBG((L"Performing buy API call\r\n"));

    CString requestData = this->SerializeBuy(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(BUY_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make buy request\r\n"));
        return false;
    }

    if (!this->DeserializeBuyResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize buy result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

bool CCashDepotService::BuyComplete(const CDBuyCompleteRequest &request, CString sessionId, CDBuyCompleteResponse &response)
{
    NHDBG((L"Performing buy complete API call\r\n"));

    CString requestData = this->SerializeBuyComplete(request);

    bool success;
    HttpResponse httpResponse = this->MakeRequest(BUY_COMPLETE_ROUTE, requestData, L"", sessionId, HTTP_POST, &success);

    if (!success)
    {
        NHERROR((L"Failed to make buy complete request\r\n"));
        return false;
    }

    if (!this->DeserializeBuyCompleteResponse(httpResponse.ResponseData, response))
    {
        NHERROR((L"Failed to deserialize buy complete result\r\n"));
        return false;
    }

    response.StatusCode = Asc2Int(httpResponse.StatusCode);
    response.StatusMessage = httpResponse.StatusMessage;

    return true;
}

//
// Private Functions
//

HttpResponse CCashDepotService::MakeRequest(CString route, CString data, CString terminalId, CString sessionId, HTTP_METHOD method, bool *success)
{
    HttpResponse response;
    NHDBG((L"Making request to %s\r\n", route));

    char *buffer = NULL;
    *success = false;

#if (APP_CUSTOM_DYNAMICDCC)      // RWC6-645  Disable API call for CASHDEPOT if DYNAMICDCC is set
	return response;
#else

    // Encode the data, if present
    if (!data.IsEmpty())
    {
        int dataSize = data.GetLength();
        buffer = new char[dataSize + 1]();

        WideToMulti(buffer, data, dataSize);
    }

    // Add additional request headers
    CString headers(L"Content-Type: application/json\r\n");

    if (!terminalId.IsEmpty())
        headers.AppendFormat(L"%s: %s\r\n", TERMINALID_HEADER, terminalId);

    if (!sessionId.IsEmpty())
        headers.AppendFormat(L"%s: %s\r\n", SESSIONID_HEADER, sessionId);

    NHTRACE((L"%s\r\n", data));

    HttpRequest request;
    request.Url = route;
    request.Headers = headers;
    request.Data = (unsigned char *)buffer;

    HTTPERR result = this->http->SendRequest(method, &response, request);

    // Delete the buffer, if necessary.
    if (buffer != NULL)
        delete[] buffer;

    if (result != HTTP_NO_ERR)
    {
        NHERROR((L"Error in web request. Error code: %d\r\n", result));

        return response;
    }

    NHTRACE((L"%s\r\n", response.ResponseData));

    *success = true;

    return response;
#endif
}