#pragma once;

#include ".\Net\HTTP.h"
#include ".\Dll\NHReadiniFile.h"
#include ".\Dev\JnlMgr.h"

#include <parson/parson.h>			// [RWC6-117] Update JSON implementation
#pragma comment(lib, "parson.lib")	// [RWC6-117] Update JSON implementation

#define B4U_INI_FILE				ATM_DATA_PATH L"\\B4UConfig.ini"
#define B4U_DISPENSERESULT_FILE		ATM2_DATA_PATH L"\\B4UDispenseResult.dat"

//
const int			B4U_DecKeyLen = 16;
const unsigned char B4U_DecKey[B4U_DecKeyLen] = {	0x35, 0x34, 0x67, 0x67, 0x34, 0x35, 0x35, 0x34, 
													0x67, 0x48, 0x55, 0x53, 0x42, 0x35, 0x36, 0x6a	};

// session level
typedef int B4USessionType;
const B4USessionType B4U_SESS_TYPE_NONE		= 0;
const B4USessionType B4U_SESS_TYPE_PRESTAGE	= 1;
const B4USessionType B4U_SESS_TYPE_STAGE		= 2;
const B4USessionType B4U_SESS_TYPE_REPORT		= 3;

// Opportunity statuses
typedef CString B4UStatus;
const B4UStatus B4U_PRE_WAITING		= _T("waiting");
const B4UStatus B4U_PRE_RECEVIED	= _T("received");
const B4UStatus B4U_CONFIRMED		= _T("confirmed");
const B4UStatus B4U_NOT_PAID		= _T("BTC/satoshi not paid");
const B4UStatus B4U_PAID			= _T("paid");

typedef int	B4UDispResult;
const B4UDispResult	B4U_DISP_RESULT_FULL	= 0;
const B4UDispResult	B4U_DISP_RESULT_PARTIAL	= 1;
const B4UDispResult	B4U_DISP_RESULT_NONE	= 2;

// An error provided by the V API
typedef struct _B4U_error {
	CString	Description;
	int		Code;
} B4UError, *LPB4UError;

// value from INI
typedef struct _B4U_ini {
	CString			Host;
	CString			MerchantId;
	CString			SecretId;
	CString			PostRoute;
	CString			PostRouteReport;	// [#RWC6-77] Bitload4U Updated API and Enhancement
	CString			GetRoute;
	CString			GetRoutePrestage;	// [#RWC6-77] Bitload4U Updated API and Enhancement

	CString			Currency;
	CString			FeeDisplay;
	CString			MinimumDisplay;
	CString			QRCodeTimeout;

}	B4UIni, *LPB4UIni;

// value for journal
typedef struct _B4U_Journal {
	CString			DayTotalDateTime;
	CString			DayTotalDispCount;
	CString			DayTotalDispAmt;
	B4UDispResult	DispenseResult;	// 0=dispensed fully, 1=partial dispensed, 2=not dispensed	

}	B4UJournal, *LPB4UJournal;

// The response from the opportunity registration route
typedef struct _B4U_session {
	B4UError		Error;	//LPB4UError Error;
	B4USessionType	Type;
	bool			IsCallbackNeeded;
	
	CString			SurchargeAmount;
	CString			Currency;

	CString			AmountDisplay;
	CString			ReqDateTime;
	CString			QrPath;
	CString			BitcoinAddr;
	CString			TransactionId;
	CString			EncTransactionId;
	CString			QrData;
	CString			BitcoinAmountDisplay;
	int				Amount;

	CString			SequenceNumber;
	CString			RequestedAmount;
	CString			DispensedAmount;
	CString			EquivalentAmount;
	CString			TerminalId;
	CString			Disbursed;


} B4USession, *LPB4USession;

// The response from the get trx status request
typedef struct _B4U_session_status {
	//LPB4UError	Error;
	B4UError	Error;
	bool		ErrorIsSet;

	B4UStatus		Status;
	CString			TransactionId;
	CString			Currency;
	CString			AmountDisplay;
	CString			Confirmation;
	CString			TransactionHash;
	CString			SatoshiValue;
	CString			EuroValue;
	CString			Usdvalue;
	CString			FeeFromServer;
	CString			CurrencyFromServer;
	CString			EquivalentAmount;
	int				Amount;

	/**
	 * Returns true if the trx has an error
	 */
	BOOL HasError()
	{
		return ErrorIsSet && Error.Code != 0;
	}
} B4USessionStatus, *LPB4USessionStatus;

class AFX_EXT_CLASS CB4UService {
public:
	CB4UService(CString host);
	~CB4UService(void);

	/**
	 * Registers a new opportunity and returns the session information
	 * @param[in] INI (configuration) information for B4U service 
	 * @param[out] session a reference to a B4U session information
	 * @returns LPB4USession the session information
	 */
	BOOL RequestNewSession(B4UIni ini, B4USession& session);

	/**
	 * Retrieves the trx's status from the API;
	 * @param[in] request information required for getting the trx status
	 * @param[in] INI (configuration) information for B4U service
	 * @param[out] sessionStatus a reference to a trx status object which will contain information about the trx if function returns true
	 * @returns LPB4USessionStatus the trx status
	 */
	BOOL GetSessionStatus(B4USession& session, B4USessionType sessionType, B4UIni ini, B4USessionStatus& sessionStatus);

	/**
	 * Saves the QR code data to a local file and returns the filename
	 * @param[in] base64Data the encoded PNG data from which the QR code should be derrived
	 * @param[out] filename the filename the QR code was saved as
	 * @returns whether or not the operation was successful
	 */
	static BOOL GenerateQRCode(CString qrdata, CString filename); 
	static BOOL SaveQrDataToFile(CString base64Data, CString* filename);

	BOOL LoadB4UConfigIni(B4UIni& ini);

	CString GetB4UDaytotalJournalData(B4UJournal& journal);
	BOOL ReadB4UDispenseHistory(B4UJournal& journal);
	BOOL SaveB4UDispenseResult(BOOL bReset, B4UJournal& journal);
	void CreateB4UDispenseResult(B4UJournal& journal);
	void RecordB4UDispenseResult(long nAmount, B4UJournal& journal);
	void ResetTotal();

	/**
	 * Prepares journal entry and sends it
	 * @param[in] journal object that will be handling the prepped entry
	 * @param[in] journal information from current session status
	 * @param[in] journal kind code that identifies journal kind
	 * @returns whether or not the operation was successful
	 */
	BOOL WriteB4UJournalEntry(CJnlMgr *mgr, B4USession& session, B4USessionStatus& status, CString sKindCode);

private:
	HTTP *httpClient;

	CString PostData(CString route, CString data, BOOL IsCallbackNeeded, CString terminalID, B4USessionType sessionType, B4UIni ini, BOOL *success);	// [#RWC6-116] B4U Registration QR
	CString GetData(CString route, CString data, B4USessionType sessionType, CString terminalID, B4UIni ini, B4USessionStatus& status, BOOL *success);

	static BOOL DeserializeB4USession(CString response, B4USession& session);
	static BOOL DeserializeB4USessionStatus(CString response, CString requestedCurrency, B4USessionType sessionType, B4USessionStatus& status);
	static CString SerializePrestageRequest();
	static CString SerializeSessionRequest(B4UIni ini, const B4USession& request);
	static CString FinalizeSessionStatus(const B4USession& request);
	static CString DecryptB4UValue(CString encryptedValue, BOOL *success);	// [#RWC6-77] Bitload4U Updated API and Enhancement
	static CString EncryptB4UValue(CString originalValue);
};
