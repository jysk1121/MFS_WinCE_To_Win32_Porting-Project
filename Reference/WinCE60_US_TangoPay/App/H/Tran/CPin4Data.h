#ifndef __CPIN4DATA_H__
#define __CPIN4DATA_H__

#define PIN4_TRAN_NONE			0
#define PIN4_TRAN_GETTOKEN		1
#define PIN4_TRAN_AUTH_PIN4		2
#define PIN4_TRAN_AUTH_PROC		3
#define PIN4_TRAN_REVERSAL_PIN4	4
#define PIN4_TRAN_REVERSAL_PROC	5

//#define PIN4_REQ_TRANSACTION	0
//#define PIN4_REQ_REVERSAL		1

#define PIN4_FILE_LOGO_BUTTON	0
#define PIN4_FILE_LOGO_TOPCONER	1
#define PIN4_FILE_SSLCERT		2

#define	PIN4_RESULT_OK			0
#define PIN4_RCV_ERROR			1
#define PIN4_HOST_DENY			2

#define PIN4_SETTING_FILE				_T("HalCash_Pin4.xml")
#define PIN4_DISPENSERESULT_FILE		_T("Pin4DispenseResult.dat")

#define PIN4_SYMMETRIC_KEY_FILE			_T("PIN4_SYMMETRIC_KEY.DAT")
#define PIN4_PRIVATE_KEY_FILE			_T("PIN4_PROC_PRVKEY.DER")
#define PIN4_CERTIFICATE_FILE			_T("PIN4_CERTIFICATE.PEM")	// [#2524]

#define PIN4_PUBLIC_KEY_FILE			_T("PIN4_PBKEY.PEM")

class AFX_EXT_CLASS CPin4Data
{
public:
	CPin4Data();
	virtual ~CPin4Data();

	// Pin4 Setting 
	BOOL	m_bPin4Available;
	CString m_strHostIP, m_strHostPort, m_strHostSSL, m_strHostProcess, m_strCommId;						//  [#2524] US Justin Download Certificate (remove m_strHostSSLCert) // [#2431] US Justin 2016.06.13 Add PAI CommId 
	CString m_strMaxDispenseLimit;																			// [#2433] US Justin 2016.06.23 Add MaxDispenseLimit
								
	CString m_strAuthHostIP, m_strAuthHostPort, m_strAuthHostSSL, m_strAuthHostProcess, m_strAuthClientID;	// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging
	char* m_pPublicKey;				// PIN4 PUBLIC KEY														// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging	
	unsigned char* m_pSymmetricKey;	// SYMMETRIC KEY														// [#2515] US Justin 2017.11.22 MasterCard Cash Pickup - Prestaging	
	unsigned char* m_pPrivateKey;	// PROCESSOR PRIVATE KEY												// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging	
	long  m_nLenSymmetricKey, m_nLenPrivateKey;			// Length of KEYs									// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging	

	CString m_strPin4Logo, m_strPin4Logo_Button;
	//CString m_strHostPage;

	// Day Total Data
	CString m_strDayTotalDateTime, m_strDayTotalDispCount, m_strDayTotalDispAmt;

	// Transaction Buffer
	int		m_nTransactionStep;
	CString m_strP4AccessToken, m_strVCN, m_strExpirationYYMM;												// [#2515] US Justin 2017.11.14 MasterCard Cash Pickup - Prestaging	
	CString m_strUSPhoneNumber, m_strIntPhoneCountryCode, m_strIntPhoneNumber;
	CString	m_strPin4Code, m_strSecretPin;
	CString m_strTranSeqNo, m_strTID;
	CString m_strTranAmount, m_strTranDispensed;

	CString m_strTranResCode, m_strTranResAuthNum, m_strTranResDate, m_strTranResTime, m_strTranResBizDate;
	CString m_strRevResCode;
	CString m_strErrorCode, m_strErrorDesc;
	int		m_nNeedReversal;

public:
	void	ResetSetting();
	void	SetTestTransactionSettings();								// [#RWC6-57] US William 2019.09.18 PAI MCCP Key presence
	void	ResetTotal();
	BOOL	ReadSettingAndDispenseHistory();
	void	MoveCredentialData2ATM2Folder();							// [#2568] US Justin 2018.07.30 Move PIN4 Credential from ATM/DATA to ATM2/DATA
	BOOL	SavePin4Setting()					;						// [#2515] [#2455] US Justin
	BOOL	AssignProcessorPrivateKey(unsigned char* key, int nLen);	// [#2515]
	BOOL	AssignSymmetricKey(unsigned char* key, int nLen);			// [#2515]
	BOOL	AssignX509Certificate(unsigned char* cert, int nLen);		// [#2524]
	void	RemoveDownloadCredentials();								// [#2563] US Justin 2018.07.12 Reset Cashed PIN4 and GivePay data for NVRAM CLEAR
	BOOL	HaveSymmetricKey();											// [#J003] US Justin 2018.11.12 Mastercard Cash Pickup - Add OP Menu for removing Sym Key, Add an indicator for Sym Key Existance
	BOOL	SaveDispenseResult(BOOL bReset=FALSE);
	void	RecodeDispenseResult(long nAmount);
	CString GetDaytotalJournalData();	
	CString	GetPin4FileName(int nType);

	void	ResetTransactionData();
	void	SetTransactionData();
	void	SetJournalRMSOtherMsg(bool bUserCancelMsg = false);	// [#2515]
	CString	MakeRequestMessage(LPCTSTR sStatus);
	CString	MakeDemoResponseMessage();
	BOOL	ParseHTTPHeader(CString strReceived, CString &strHeaderCode);	// [#2576] US Justin 2018.09.17 Add Pin4 HTTP Header Parsing Logic 
	int		ParseReceivedData(CString strReceived);		// [#2515]
	int		SetErrorCodeAndDesc(CString sResCode);	
};

#endif //__CPIN4DATA_H__