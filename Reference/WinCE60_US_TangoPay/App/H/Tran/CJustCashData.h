#ifndef __CJUSTCASHDATA_H__
#define __CJUSTCASHDATA_H__

// [#2445] US Justin 2016.09.21 Implement Just.Cash
#if(APP_JUST_CASH)

//#define JUSTCASH_REQ_TRANSACTION		0
//#define JUSTCASH_REQ_REVERSAL			1

#define JUSTCASH_CARDLESS_TRAN_NOTSTART			0
#define JUSTCASH_CARDLESS_TRAN_SUBMITCODE		1
#define JUSTCASH_CARDLESS_TRAN_HOSTAPPROVAL		2
#define JUSTCASH_CARDLESS_TRAN_UPDATETRAN		3
#define JUSTCASH_CARDLESS_TRAN_HOSTREVERSAL		4

#define JUSTCASH_DIGITAL_CUR_PURCHASE			5
#define JUSTCASH_DIGITAL_CUR_HOSTAPPROVAL		6
#define JUSTCASH_DIGITAL_CUR_COMMIT				7
#define JUSTCASH_DIGITAL_CUR_PROC_REVERSAL		8
#define JUSTCASH_DIGITAL_CUR_INFO				9

#define JUSTCASH_SETUP_LOGIN					10

// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash
#define JUSTCASH_DIGITAL_CUR_CASH_PHONE			11
#define JUSTCASH_DIGITAL_CUR_CASH_CODE			12
#define JUSTCASH_DIGITAL_CUR_CASH_ADD			13
#define JUSTCASH_DIGITAL_CUR_CASH_COMMIT		14
#define JUSTCASH_DIGITAL_CUR_VAULT_BALANCE		15
// End of [#RWC6-477]

#define JUSTCASH_FILE_LOGO_BUTTON				0
#define JUSTCASH_FILE_LOGO_GENERAL				1

#define JUSTCASH_SETTING_FILE					_T("JustCashConfig.xml")
#define JUSTCASH_DISPENSERESULT_FILE			_T("JustCashDispenseResult.dat")
#define JUSTCASH_DGTL_CUR_RESULT_FILE			_T("JustCashDgtlCurResult.dat")				// [#2496] US Justin 2017.08.14 Purchasing BitCoin

#define	JUSTCASH_SERVICE_CARDLESS				0x0001
#define	JUSTCASH_SERVICE_DIGITAL_CURRENCY		0x0002
#define JUSTCASH_SERVICE_ALL					0x0003

#define	JUSTCASH_CURRENCY_BITCOIN				_T("BITCOIN")								// [#2503]
#define	JUSTCASH_CURRENCY_ETHEREUM				_T("ETHEREUM")								// [#2503]

#define JUSTCASH_ETHEREUM_WALLETQRIMAGE			ATM_DATA_PATH _T("\\EthereumWalletQR.bmp")	// [#2503]

/**
 * The JC transaction mode of operation
 */
typedef int JCTRANSACTIONMODE;
const JCTRANSACTIONMODE JCTXNMODE_ATM = 0;
const JCTRANSACTIONMODE JCTXNMODE_SIDECAR = 1;
const JCTRANSACTIONMODE JCTXNMODE_BOTH = 2;
const JCTRANSACTIONMODE JCTXNMODE_MAX = 3;

class AFX_EXT_CLASS CJustCashData
{
public:
	CJustCashData();
	virtual ~CJustCashData();

	////////////////////////////////////////////////////////////
	// Setting Data
	BOOL				m_bJustCashAvailable, m_bJustCashRegistered;
	CString				m_strHostIP, m_strHostPort, m_strHostSSL, m_strHostProcess;
	CString				m_strJCLogo, m_strJCLogo_Button;
	CString				m_strUserName, m_strEncryptPWD, m_strDevKey, m_strSessionKey; // [#2528] 2018.01.29 Remove m_strEncrpytPIN
	JCTRANSACTIONMODE	m_transactionMode;

	void	ResetSetting();
	BOOL	ReadSettingAndDispenseHistory();
	BOOL	SaveSetting();
	CString	GetJustCashFileName(int nType);
	CString	GetHashCode();
	BOOL	IsSidecarConfigured();
	BOOL	IsAtmConfigured();
	CString	GetTransactionMode();

	////////////////////////////////////////////////////////////
	// Cardless Transaction Data
	int		m_nTransactionStep;
	BOOL 	m_bIsOnUsTransaction;
	CString	m_strCashCode, m_strTID, m_strTranAmount, m_strTrackData, m_strApprovalCode, m_strTranDispensed;	
	CString m_strErrorCode, m_strErrorDesc;

	// Digital Currency Transaction Data
	CString m_strCurrencyType, m_strDCPrice, m_strDCQuantity, m_strDCUnitPrice, m_strDCCommit;										 	// [#2496] US Justin 2017.08.14 Purchasing BitCoin
	CString m_strBTPublicKey, m_strBTPrivateKey;																					 	// [#2503] Bit coin information
	CString m_strETWalletFile, m_strETPassPhase, m_strETAddress, m_strWalletImage;													 	// [#2503] Ethereum information
	CString m_strPhoneNum, m_strTxId, m_strCode, m_strAmount, m_strMinAmount, m_strMaxAmount, m_strServerMessage, m_strVaultBalance;	// [#RWC6-477] US ryan.payton 2022.11.21 Purchase Bitcoin by Cash

	void	ResetTransactionData();
	CString	MakeRequestMessage(CString strTerminalID, CString strErrorCode,  CString strLast6Digit);
	CString	MakeDemoResponseMessage();
	BOOL	ParseReceivedData(CString strReceived);
	void	SetErrorCodeAndDesc(CString sResCode, LPCTSTR sErrText=NULL);	

	////////////////////////////////////////////////////////////
	// Day Total Data - Cardless Transaction
	CString m_strJCDayTotalDateTime, m_strJCDayTotalDispCount, m_strJCDayTotalDispAmt; // DOLLAR;
	CString m_strDCDayTotalDateTime, m_strDCDayTotalDispCount, m_strDCDayTotalDispAmt; // CENT !!!;						// [#2496] US Justin 2017.08.14 Purchasing BitCoin

	void	ResetTotal(int nService);
	BOOL	ReadDispenseHistory();
	BOOL	SaveDispenseResult(BOOL bReset=FALSE);
	void	RecodeDispenseResult(long nAmount /* Cent */, int nType);
	CString GetDaytotalJournalData(int nType);	
};
#endif
// End of [#2445]

#endif //__CJUSTCASHDATA_H__