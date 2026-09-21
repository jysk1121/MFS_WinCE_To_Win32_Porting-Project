#pragma once

#include "stdafx.h"
#include ".\Net\HTTP.h"

#include ".\Tran\CashDepotTypes.h"

#include <parson/parson.h>
#pragma comment(lib, "parson.lib")

class AFX_CLASS_EXPORT CCashDepotService
{
public:
    explicit CCashDepotService(HTTP *http);

    //
    // Standard Flows
    //

    CString GetTimestamp();

    /**
     * Get the configurations
     * @param configs[in] the request data
     * @param terminalId[in] the terminal id of the ATM
     * @param response[out] The result of the request
     * @returns true when the operation is successful in retrieving an API response
     */
    bool GetConfigurations(const CDConfigurations &configs, CString terminalId, CDConfigurationsResponse &response);

    //
    /// Cryptocurrency Flows
    //

    /**
     * Get the providers
     * @param response[out] The result of the request
     * @returns true when the operation is successful in retrieving an API response
     */
    bool GetProviders(CDProvidersResponse &response);

    /**
     * Get the client token
     * @param response[out] The result of the request
     * @returns true when the operation is successful in retrieving an API response
     */
    bool GetClientToken(const CDClientTokenRequest &request, CDClientTokenResponse &response);

    /**
     * Call the send code function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @returns true when the operation is successful in retrieving an API response
     */
    bool SendCode(const CDSendCodeRequest &request, CString sessionId, CDSendCodeResponse &response);

    /**
     * Call the verify code function
     * @param request[in] the verify code request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the verify code request
     * @returns true when the operation is successful in retrieving an API response
     */
    bool VerifyCode(const CDVerifyCodeRequest &request, CString sessionId, CDVerifyCodeResponse &response);

    /**
     * Call the disclosure function
     * @param request[in] the request data
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool Disclosure(const CDDisclosureRequest &request, CDDisclosureResponse &response);

    /**
     * Call the disclosure accept function
     * @param request[in] the request data
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool DisclosureAccept(const CDDisclosureAcceptRequest &request, CDDisclosureAcceptResponse &response);

    /**
     * Call the daily limit function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool DailyLimit(const CDDailyLimitRequest &request, CString sessionId, CDDailyLimitResponse &response);

    /**
     * Call the crypto currency function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool CryptoCurrency(const CDCryptoCurrencyRequest &request, CString sessionId, CDCryptoCurrencyResponse &response, CString currency);

    /**
     * Call the crypto currency price function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool CryptoCurrencyPrice(const CDCryptoCurrencyPriceRequest &request, CString sessionId, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency);

    /**
     * Call the send manual wallet link function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool SendManualWalletLink(const CDSendManualWalletLinkRequest &request, CString sessionId, CDSendManualWalletLinkResponse &response);

    /**
     * Call the get manual wallet address function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool GetManualWalletAddress(const CDGetManualWalletAddressRequest &request, CString sessionId, CDGetManualWalletAddressResponse &response);

    /**
     * Call the check manual wallet address function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool CheckManualWalletAddress(const CDCheckManualWalletAddressRequest &request, CString sessionId, CDCheckManualWalletAddressResponse &response);

    /**
     * Call the buy function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool Buy(const CDBuyRequest &request, CString sessionId, CDBuyResponse &response);

    /**
     * Call the buy complete function
     * @param request[in] the request data
     * @param sessionId[in] new GUID
     * @param response[out] The result of the request
     * @return true when the operation is successful in retrieving an API response
     */
    bool BuyComplete(const CDBuyCompleteRequest &request, CString sessionId, CDBuyCompleteResponse &response);

private:
    bool isMyHttp;
    HTTP *http;

    HttpResponse MakeRequest(CString route, CString data, CString terminalId, CString sessionId, HTTP_METHOD method, bool *success);

    /* Serialization - CashDepotService_Serialization.cpp */

    //
    // Standard Flows
    //
    CString SerializeConfigurations(const CDConfigurations &configs);
    bool DeserializeConfigurationsResponse(CString data, CDConfigurationsResponse &response);

    //
    /// Cryptocurrency Flows
    //

    bool DeserializeProvidersArray(JSON_Array *providers, CDProviders &providersList);
    bool DeserializeProvider(JSON_Object *object, CDProvider &provider);
    bool DeserializeErrorsArray(JSON_Array *errors, CDErrors &errorsList);
    bool DeserializeError(JSON_Object *object, CDError &error);
    bool DeserializeDisclosuresArray(JSON_Array *disclosures, CDDisclosures &disclosuresList);
    bool DeserializeDisclosure(JSON_Object *object, CDDisclosure &disclosure);
    bool DeserializeCryptoCurrencyArray(JSON_Array *cryptoCurrencies, CDCryptoCurrencies &cryptoCurrenciesList, CString currency);
    bool DeserializeCryptoCurrency(JSON_Object *object, CDCryptoCurrency &cryptoCurrency, CString currency);
    bool DeserializeCryptoCurrencyPricesArray(JSON_Array *dataArray, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency);
    bool DeserializeCryptoCurrencyPrice(JSON_Object *object, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency);
    bool DeserializeCryptoCurrencyPriceRates(JSON_Object *object, CDCryptoCurrencyPriceResponse &response);

    //CString SerializeProviders(const CDProvidersRequest &request);
    bool    DeserializeProvidersResponse(CString data, CDProvidersResponse &response);
    CString SerializeClientToken(const CDClientTokenRequest &request);
    bool    DeserializeClientTokenResponse(CString data, CDClientTokenResponse &response);
    CString SerializeSendCode(const CDSendCodeRequest &request);
    bool    DeserializeSendCodeResponse(CString data, CDSendCodeResponse &response);
    CString SerializeVerifyCode(const CDVerifyCodeRequest &request);
    bool    DeserializeVerifyCodeResponse(CString data, CDVerifyCodeResponse &response);
    CString SerializeDisclosure(const CDDisclosureRequest &request);
    bool    DeserializeDisclosureResponse(CString data, CDDisclosureResponse &response);
    CString SerializeDisclosureAccept(const CDDisclosureAcceptRequest &request);
    bool    DeserializeDisclosureAcceptResponse(CString data, CDDisclosureAcceptResponse &response);
    CString SerializeDailyLimit(const CDDailyLimitRequest &request);
    bool    DeserializeDailyLimitResponse(CString data, CDDailyLimitResponse &response);
    CString SerializeCryptoCurrency(const CDCryptoCurrencyRequest &request);
    bool    DeserializeCryptoCurrencyResponse(CString data, CDCryptoCurrencyResponse &response, CString currency);
    CString SerializeCryptoCurrencyPrice(const CDCryptoCurrencyPriceRequest &request);
    bool    DeserializeCryptoCurrencyPriceResponse(CString data, CDCryptoCurrencyPriceResponse &response, CString selectedCurrency);
    CString SerializeSendManualWalletLink(const CDSendManualWalletLinkRequest &request);
    bool    DeserializeSendManualWalletLinkResponse(CString data, CDSendManualWalletLinkResponse &response);
    CString SerializeGetManualWalletAddress(const CDGetManualWalletAddressRequest &request);
    bool    DeserializeGetManualWalletAddressResponse(CString data, CDGetManualWalletAddressResponse &response);
    CString SerializeCheckManualWalletAddress(const CDCheckManualWalletAddressRequest &request);
    bool    DeserializeCheckManualWalletAddressResponse(CString data, CDCheckManualWalletAddressResponse &response);
    CString SerializeBuy(const CDBuyRequest &request);
    bool    DeserializeBuyResponse(CString data, CDBuyResponse &response);
    CString SerializeBuyComplete(const CDBuyCompleteRequest &request);
    bool    DeserializeBuyCompleteResponse(CString data, CDBuyCompleteResponse &response);
};