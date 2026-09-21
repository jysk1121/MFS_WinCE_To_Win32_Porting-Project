#include "stdafx.h"

#include ".\Common\CmnLib.h"
#include ".\Tran\TranBizHost_Hyosung_EJUP.h"

#define JNLTYPE_TRANSACTION			_T("0")
#define JNLTYPE_CASSETTETOTAL		_T("1")
#define JNLTYPE_DAYTOTAL			_T("2")
#define JNLTYPE_TEXT				_T("3")
#define JNLTYPE_ADDCASH				_T("4")
#define JNLTYPE_DENOMINATION		_T("5")
#define JNLTYPE_VATDAYTOTAL			_T("6")
#define JNLTYPE_LTX_PURCHASE		_T("8")
#define JNLTYPE_LTX_SELL			_T("9")
#define JNLTYPE_B4U					_T("A")
#define JNLTYPE_DIGITALMINT			_T("B")

#define TRANSACTIONRESULT_OK		_T("0")
#define TRANSACTIONRESULT_REVERSAL	_T("1")

#define ADDCASHTYPE_ADDCASH			_T("0")
#define ADDCASHTYPE_REMOTE			_T("1")

#define DAYTOTALTYPE_TRIALDAY			_T("0")
#define DAYTOTALTYPE_DAY				_T("1")
#define DAYTOTALTYPE_TRIALDAY_REMOTE	_T("2")
#define DAYTOTALTYPE_DAY_REMOTE			_T("3")
#define DAYTOTALTYPE_DAY_ERROR			_T("X")

#define CASSETTETOTALTYPE_CSTTRIAL		_T("0")
#define CASSETTETOTALTYPE_CST			_T("1")
#define CASSETTETOTALTYPE_REMOTECST		_T("2")

#define VATTOTALTYPE_PIN4				_T("02")
#define VATTOTALTYPE_PIN4_TRIAL			_T("52")
#define VATTOTALTYPE_POPMONEY			_T("04")
#define VATTOTALTYPE_POPMONEY_TRIAL		_T("54")
#define VATTOTALTYPE_PAYPAL				_T("05")
#define VATTOTALTYPE_PAYPAL_TRIAL		_T("55")
#define VATTOTALTYPE_JUSTCASH			_T("06")
#define VATTOTALTYPE_JUSTCASH_TRIAL		_T("56")
#define VATTOTALTYPE_DIGITALCUR			_T("08")
#define VATTOTALTYPE_DIGITALCUR_TRIAL	_T("58")
#define VATTOTALTYPE_B4U				_T("09")
#define VATTOTALTYPE_B4U_TRIAL			_T("59")
#define VATTOTALTYPE_UNKNOWN			_T("99")


CString CreateTransactionJournalString(CStringArray &arJnlField)
{
	// Journal Type (Transaction)
	CString strResult = JNLTYPE_TRANSACTION;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// Transaction Sequence Number
	arJnlField[10].TrimLeft();
	arJnlField[10].TrimRight();
	strResult += arJnlField[10];

	// Operation Code
	arJnlField[11].TrimLeft();
	arJnlField[11].TrimRight();
	strResult.AppendFormat(L"%2.2s", arJnlField[11]);

	// Source Account
	arJnlField[12].TrimLeft();
	arJnlField[12].TrimRight();
	strResult.AppendFormat(L"%2.2s", arJnlField[12]);

	// Destination Account
	arJnlField[13].TrimLeft();
	arJnlField[13].TrimRight();
	strResult.AppendFormat(L"%2.2s", arJnlField[13]);

	// Account Number
	arJnlField[30].TrimLeft();
	arJnlField[30].TrimRight();
	strResult.AppendFormat(L"%16.16s", arJnlField[30]);

	// Local Transaction Date
	arJnlField[16].TrimLeft();
	strResult.AppendFormat(L"%s%s20%s", arJnlField[16].Left(2), arJnlField[16].Mid(2,2), arJnlField[16].Right(2));

	// Local Transaction Time
	arJnlField[17].TrimLeft();
	strResult.AppendFormat(L"%6.6s", arJnlField[17]);

	// Retrieval Reference Number
	arJnlField[19].TrimLeft();
	arJnlField[19].TrimRight();
	strResult.AppendFormat(L"%012d", Asc2Int(arJnlField[19]));

	// System Trace Audit Number
	arJnlField[20].TrimLeft();
	arJnlField[20].TrimRight();
	// [#2454] NH Justin 2016.11.30 EJ UPLOAD BUG FIX (Transaction Audit/Network ID) Specify a number of digit
	strResult.AppendFormat(L"%6.6s", arJnlField[20].Left(6));

	// Network ID
	// [#2454] NH Justin 2016.11.30 EJ UPLOAD BUG FIX (Transaction Audit/Network ID) Specify a number of digit
	strResult.AppendFormat(L"%2.2s", arJnlField[20].Right(2));;

	// Settlement Date
	arJnlField[22].TrimLeft();
	strResult.AppendFormat(L"%s%s20%s", arJnlField[22].Left(2), arJnlField[22].Mid(2,2), arJnlField[22].Right(2));

	// Surcharge Amount
	arJnlField[23].TrimLeft();
	arJnlField[23].TrimRight();
	strResult.AppendFormat(L"%08d", Asc2Int(arJnlField[23]));

	// Requested / Deposited Amount
	arJnlField[24].TrimLeft();
	arJnlField[24].TrimRight();
	
	if (arJnlField[11] != L"CC")
		strResult.AppendFormat(L"%08d", Asc2Int(arJnlField[24]));
	else
		strResult += L"00000000";

	// Dispensed Amount
	arJnlField[25].TrimLeft();
	arJnlField[25].TrimRight();
	strResult.AppendFormat(L"%08d", Asc2Int(arJnlField[25]));

	// Check Cashing Device가 있을 경우 송신하므로 CE는 무조건 0으로 송신함.
	// Entered Amount
	strResult += L"00000000";

	// Error Code
	strResult.AppendFormat(L"%7.7s", arJnlField[29]);

	// Reversal Status
	// Reversal Failure를 알기 위해서는 Error Code를 봐야하므로 2는 빼야할 것으로 보임
	if (arJnlField[1] != REVERSAL_TRX)
		strResult += TRANSACTIONRESULT_OK;	// Transaction
	else
		strResult += TRANSACTIONRESULT_REVERSAL;	// Normal Reversal

	return strResult;
}

CString CreateErrorModeJournalString(CStringArray &arJnlField)
{
	CString strResult, strTemp;

	// Journal Type (Text Data)
	strResult = JNLTYPE_TEXT;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	//strTemp2.Format(L"[%s] (ERROR CODE : %s)", L"INTO ERROR MODE", arJnlField[30]);
	strTemp.Format(L"[%s] (ERROR CODE : %s)", L"INTO ERROR MODE", arJnlField[29]);		// [#2355] NH Justin 2015.07.02 EJ Upload Bug Fix (Error Code:29)
	strResult.AppendFormat(L"%02d%s", strTemp.GetLength(), strTemp);

	return strResult;
}

CString CreateAddCashJournalString(CStringArray &arJnlField)
{
	// Journal Type (Add Cash)
	CString strResult = JNLTYPE_ADDCASH;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	int nJnlField = arJnlField.GetSize();

	// Add Cash Type
	if (arJnlField[1] == ADDCASH_CST1 || arJnlField[1] == ADDCASH_CST2 || arJnlField[1] == ADDCASH_CST1_2 ||
		arJnlField[1] == ADDCASH_CST3 || arJnlField[1] == ADDCASH_CST1_3 || arJnlField[1] == ADDCASH_CST2_3 ||
		arJnlField[1] == ADDCASH_CST1_2_3 || arJnlField[1] == ADDCASH_CST4 || arJnlField[1] == ADDCASH_CST1_4 ||
		arJnlField[1] == ADDCASH_CST2_4 || arJnlField[1] == ADDCASH_CST1_2_4 || arJnlField[1] == ADDCASH_CST3_4 ||
		arJnlField[1] == ADDCASH_CST1_3_4 || arJnlField[1] == ADDCASH_CST2_3_4 || arJnlField[1] == ADDCASH_CST1_2_3_4)
	{
		strResult += ADDCASHTYPE_ADDCASH;	// ADD CASH
	}
	else
	{
		strResult += ADDCASHTYPE_REMOTE;	// REMOTE ADD CASH
	}

	if (nJnlField >= 11)
	{
		// 1 CST REMAINING COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[9]));

		// 1 CST ADDITION COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[10]));
	}
	else
	{
		strResult += L"0000";
		strResult += L"0000";
	}

	if (nJnlField >= 13)
	{
		// 2 CST REMAINING COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[11]));

		// 2 CST ADDTION COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[12]));
	}
	else
	{
		strResult += L"0000";
		strResult += L"0000";
	}

	if (nJnlField >= 15)
	{
		// 3 CST REMAINING COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[13]));

		// 3 CST ADDTION COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[14]));
	}
	else
	{
		strResult += L"0000";
		strResult += L"0000";
	}

	if (nJnlField >= 17)
	{
		// 4 CST REMAINING COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[15]));

		// 4 CST ADDTION COUNT
		strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[16]));
	}
	else
	{
		strResult += L"0000";
		strResult += L"0000";
	}

	return strResult;
}

CString CreateChangeDenomJournalString(CStringArray &arJnlField)
{
	// Journal Type (Denomination)
	CString strResult = JNLTYPE_DENOMINATION;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	int nJnlField = arJnlField.GetSize();

	if (nJnlField >= 11)
	{
		// 1 CST OLD DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[9]));

		// 1 CST NEW DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[10]));
	}
	else
	{
		strResult += L"000";
		strResult += L"000";
	}

	if (nJnlField >= 13)
	{
		// 2 CST OLD DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[11]));

		// 2 CST NEW DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[12]));
	}
	else
	{
		strResult += L"000";
		strResult += L"000";
	}

	if (nJnlField >= 15)
	{
		// 3 CST OLD DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[13]));

		// 3 CST NEW DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[14]));
	}
	else
	{
		strResult += L"000";
		strResult += L"000";
	}

	if (nJnlField >= 17)
	{
		// 4 CST OLD DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[15]));

		// 4 CST NEW DENOMINATION
		strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[16]));
	}
	else
	{
		strResult += L"000";
		strResult += L"000";
	}

	return strResult;
}

CString CreateDayTotalJournalString(CStringArray &arJnlField)
{
	// Journal Type (DAY TOTAL)
	CString strResult = JNLTYPE_DAYTOTAL;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// Day Total Type
	if (arJnlField[1] == TRIALDAY_TOTAL)
		strResult += DAYTOTALTYPE_TRIALDAY;
	else if (arJnlField[1] == DAY_TOTAL)
		strResult += DAYTOTALTYPE_DAY;
	else if (arJnlField[1] == REMOTE_TRIALDAY_TOTAL)
		strResult += DAYTOTALTYPE_TRIALDAY_REMOTE;
	else if (arJnlField[1] == REMOTE_DAY_TOTAL)
		strResult += DAYTOTALTYPE_DAY_REMOTE;
	else
		strResult += DAYTOTALTYPE_DAY_ERROR;	// JOURNAL ERROR

	// Host Number of Cash Withdrawals
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[12]));

	// Host Number of Inquires
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[13]));

	// Host Number of Transfers
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[14]));

	// Host Number of Check Cashing (STANDARD1 FORMAT일 경우에만 Host에서 수신됨)
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[15]));

	// Host Total Cash Dispense Amount
	strResult.AppendFormat(L"%012d", Asc2Int(arJnlField[16]));

	// Host Total Check Cashing Dispense Amount
	strResult.AppendFormat(L"%012d", Asc2Int(arJnlField[17]));

	// Terminal Number of Cash Withdrawals
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[19]));

	// Terminal Number of Inquires
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[20]));

	// Terminal Number of Transfers
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[21]));

	// Terminal Number of Check Cashing
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[23]));

	// Terminal Total Cash Dispense Amount
	strResult.AppendFormat(L"%012d", Asc2Int(arJnlField[25]));

	// Terminal Total Check Cashing Dispense Amount
	strResult.AppendFormat(L"%012d", Asc2Int(arJnlField[26]));

	// Terminal Surcharge Amount
	strResult.AppendFormat(L"%012d", Asc2Int(arJnlField[27]));

	return strResult;
}

CString CreateCassetteTotalJournalString(CStringArray &arJnlField)
{
	// Journal Type (CASSETTE TOTAL)
	CString strResult = JNLTYPE_CASSETTETOTAL;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// Cassette Total Type
	if (arJnlField[1] == TRIALCST_TOTAL)
		strResult += CASSETTETOTALTYPE_CSTTRIAL;
	else if (arJnlField[1] == CST_TOTAL)
		strResult += CASSETTETOTALTYPE_CST;
	else if (arJnlField[1] == REMOTE_CST_TOTAL)
		strResult += CASSETTETOTALTYPE_REMOTECST;

	// 1st Cassette Loaded Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[12]));

	// 1st Cassette Dispensed Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[13]));

	// 1st Cassette Rejected Count
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[14]));

	// 1st Cassette Remained Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[16]));

	// 1st Cassette Denomination
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[17]));

	// 2st Cassette Loaded Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[18]));

	// 2st Cassette Dispensed Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[19]));

	// 2st Cassette Rejected Count
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[20]));

	// 2st Cassette Remained Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[22]));

	// 2st Cassette Denomination
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[23]));

	// 3st Cassette Loaded Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[24]));

	// 3st Cassette Dispensed Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[25]));

	// 3st Cassette Rejected Count
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[26]));

	// 3st Cassette Remained Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[28]));

	// 3st Cassette Denomination
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[29]));

	// 4st Cassette Loaded Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[30]));

	// 4st Cassette Dispensed Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[31]));

	// 4st Cassette Rejected Count
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[32]));

	// 4st Cassette Remained Count
	strResult.AppendFormat(L"%04d", Asc2Int(arJnlField[34]));

	// 4st Cassette Denomination
	strResult.AppendFormat(L"%03d", Asc2Int(arJnlField[35]));

	return strResult;
}

CString CreateVatDayTotalJournalString(CStringArray &arJnlField)
{
	// Application Day Total
	CString strResult = JNLTYPE_VATDAYTOTAL;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// Application Day Total Type (2)
	if (	 arJnlField[1] == PIN4_DAY_TOTAL)				strResult += VATTOTALTYPE_PIN4;
	else if (arJnlField[1] == PIN4_TRIALDAY_TOTAL)			strResult += VATTOTALTYPE_PIN4_TRIAL;
	else if (arJnlField[1] == POPMONEY_DAY_TOTAL)			strResult += VATTOTALTYPE_POPMONEY;
	else if (arJnlField[1] == POPMONEY_TRIALDAY_TOTAL)		strResult += VATTOTALTYPE_POPMONEY_TRIAL;
	else if (arJnlField[1] == PAYPAL_DAY_TOTAL)				strResult += VATTOTALTYPE_PAYPAL;
	else if (arJnlField[1] == PAYPAL_TRIALDAY_TOTAL)		strResult += VATTOTALTYPE_PAYPAL_TRIAL;
	else if (arJnlField[1] == JUSTCASH_DAY_TOTAL)			strResult += VATTOTALTYPE_JUSTCASH;
	else if (arJnlField[1] == JUSTCASH_TRIALDAY_TOTAL)		strResult += VATTOTALTYPE_JUSTCASH_TRIAL;
	else if (arJnlField[1] == DIGITALCUR_DAY_TOTAL)			strResult += VATTOTALTYPE_DIGITALCUR;		// [#2496]
	else if (arJnlField[1] == DIGITALCUR_TRIALDAY_TOTAL)	strResult += VATTOTALTYPE_DIGITALCUR_TRIAL;		// [#2496]	
	else if (arJnlField[1] == B4U_DAY_TOTAL)				strResult += VATTOTALTYPE_B4U;
	else if (arJnlField[1] == B4U_TRIALDAY_TOTAL)			strResult += VATTOTALTYPE_B4U_TRIAL;
	else													strResult += VATTOTALTYPE_UNKNOWN;		// Undefined

	// Start Date Time (19)
	strResult.AppendFormat(L"%s %s",GetCmnLocalDate(arJnlField[9].Left(2), arJnlField[9].Mid(3,2), arJnlField[9].Mid(6,4)),	arJnlField[9].Right(8) );

	// End Date Time (19)
	strResult.AppendFormat(L"%s %s",GetCmnLocalDate(arJnlField[10].Left(2), arJnlField[10].Mid(3,2), arJnlField[10].Mid(6,4)),	arJnlField[10].Right(8) );

	// Employee ID (6) Available only for Local Employee Day Close
	strResult.Append(L"          ");

	// Transaction Count (6)
	strResult.AppendFormat(L"%06d", Asc2Int(arJnlField[12]) );

	// Transaction Amount (12)
	strResult.AppendFormat(L"%012ld", Asc2Int(arJnlField[13]) );

	return strResult;
}

CString CreateBasicTextJournalString(CStringArray &arJnlField)
{
	// Journal Type (TEXT DATA)
	CString strResult = JNLTYPE_TEXT;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// [#2454] US Justin 
	strResult.AppendFormat(L"%02d%s", arJnlField[9].GetLength(), arJnlField[9]);

	return strResult;
}

CString CreateChangeProcessorJournalString(CStringArray &arJnlField)
{
	CString strResult, strTemp, strTemp2;

	// Journal Type (TEXT DATA)
	strResult = JNLTYPE_TEXT;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	strTemp2.Format(L"Change MSG : %s => %s",  arJnlField[9].Trim(), arJnlField[10].Trim());
	strTemp.Format(L"%02d%s", strTemp2.GetLength(), strTemp2);

	strResult += strTemp;

	return strResult;
}

CString CreateOperatorActionJournalString(CStringArray &arJnlField)
{
	CString strResult, strTemp2;

	// Journal Type (TEXT DATA)
	strResult = JNLTYPE_TEXT;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// 그 외의 것들은 Header만 송신한다.
	if (arJnlField[1] == CHANGE_MASTER_PWD)
		strTemp2 = L"CHANGE MASTER PASSWORD";
	else if (arJnlField[1] == CHANGE_OPERATOR_PWD)
		strTemp2 = L"CHANGE OPERATOR PASSWORD";
	else if (arJnlField[1] == CHANGE_SUPERVISOR_PWD)
		strTemp2 = L"CHANGE SERVICE PASSWORD";
	else if (arJnlField[1] == POWER_ON_SYSTEM)
		strTemp2 = L"SYSTEM POWER ON";
	else if (arJnlField[1] == CHANGE_SERVICE_MODE)
		strTemp2 = L"INTO SERVICE MODE";
	else if (arJnlField[1] == CHANGE_OP_MODE)
		strTemp2 = L"INTO OPERATOR MODE";
	else if (arJnlField[1] == CHANGE_OP_RESET)
		strTemp2 = L"RESET OPERATOR";
	else if (arJnlField[1] == REBOOT_SYSTEM)
		strTemp2 = L"SYSTEM REBOOT";
	else if (arJnlField[1] == CHANGE_DIAG_MODE)
		strTemp2 = L"INTO DIAGNOSTIC MODE";
	else
		strTemp2 = L"UNDEFINED JOURNAL";

	strResult.AppendFormat(L"%02d%s", strTemp2.GetLength(), strTemp2);

	return strResult;
}

CString CreateLibertyXPurchaseJournalString(CStringArray &arJnlField)
{
	// Journal Type (LibertyX Purchase Transaction)
	CString strResult = JNLTYPE_LTX_PURCHASE;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// Location ID
	strResult.AppendFormat(L"%15s", arJnlField[9]);

	// Routing ID
	strResult.AppendFormat(L"%8s", arJnlField[10]);

	// API Host
	strResult.AppendFormat(L"%32s", arJnlField[11]);

	// TLS
	strResult.AppendFormat(L"%6s", arJnlField[12]);

	// Payment ID
	strResult.AppendFormat(L"%16s", arJnlField[13]);

	// Token
	strResult.AppendFormat(L"%16s", arJnlField[14]);

	// Source Amount
	strResult.AppendFormat(L"%8s", arJnlField[15]);

	// Auth Status
	strResult.AppendFormat(L"%16s", arJnlField[16]);

	// Auth Message
	strResult.AppendFormat(L"%16s", arJnlField[17]);

	// Auth Code / Sequence
	CString authCode = arJnlField[18].Left(arJnlField[18].Find('/'));
	if (authCode != arJnlField[18])
	{
		// Format: "XXXXXX/Seq.# SSSS"
		// There is a sequence number, so we need to reformat it for MoniView
		CString sequence = arJnlField[18].Right(4);
		strResult.AppendFormat(L"%8s", authCode);
		strResult.AppendFormat(L"%4s", sequence);
	}
	else 
	{
		strResult.AppendFormat(L"%8s", authCode);
		strResult.AppendFormat(L"    ");  // Sequence
	}

	// Last 4
	strResult.AppendFormat(L"%4s", arJnlField[19]);

	// Name on Card
	strResult.AppendFormat(L"%32s", arJnlField[20]);

	// Error
	if (!arJnlField[21].IsEmpty())
	{
		strResult.AppendFormat(L"%16s", arJnlField[21]);
	}
	else
	{
		strResult.Append(L"0000000");
	}

	return strResult;
}

CString CreateLibertyXSellJournalString(CStringArray &arJnlField)
{
	// Journal Type (LibertyX Sell Transaction)
	CString strResult = JNLTYPE_LTX_SELL;

	// Stack Date
	strResult.AppendFormat(L"%s%s%s", arJnlField[4], arJnlField[5], arJnlField[3]);

	// Stack Time
	strResult.AppendFormat(L"%s%s%s", arJnlField[6], arJnlField[7], arJnlField[8]);

	// Location ID
	strResult.AppendFormat(L"%15s", arJnlField[9]);

	// Routing ID
	strResult.AppendFormat(L"%8s", arJnlField[10]);

	// API Host
	strResult.AppendFormat(L"%32s", arJnlField[11]);

	// TLS
	strResult.AppendFormat(L"%6s", arJnlField[12]);

	// Token
	strResult.AppendFormat(L"%16s", arJnlField[13]);

	// Nonce
	strResult.AppendFormat(L"%36s", arJnlField[14]);

	// Amount
	strResult.AppendFormat(L"%8s", arJnlField[15]);

	// Auth Result
	strResult.AppendFormat(L"%16s", arJnlField[16]);

	// Auth Message
	strResult.AppendFormat(L"%32s", arJnlField[17]);

	// Execute Result
	strResult.AppendFormat(L"%16s", arJnlField[18]);

	// Execute Result Message
	strResult.AppendFormat(L"%32s", arJnlField[19]);

	// Execute Result Customer Message
	strResult.AppendFormat(L"%64s", arJnlField[20]);

	// Signature A
	strResult.AppendFormat(L"%128s", arJnlField[21]);

	// Signature B
	strResult.AppendFormat(L"%128s", arJnlField[22]);

	// Signature C
	strResult.AppendFormat(L"%128s", arJnlField[23]);

	// Cert A
	strResult.AppendFormat(L"%128s", arJnlField[24]);

	// Cert B
	strResult.AppendFormat(L"%128s", arJnlField[25]);

	// Cert C
	strResult.AppendFormat(L"%128s", arJnlField[26]);

	// Requested Amount
	strResult.AppendFormat(L"%8s", arJnlField[27]);

	// Dispensed Amount
	strResult.AppendFormat(L"%8s", arJnlField[28]);

	// Errors
	if (!arJnlField[29].IsEmpty())
	{
		strResult.AppendFormat(L"%8s", arJnlField[29]);
		strResult.AppendFormat(L"%32s", arJnlField[30]);
		strResult.AppendFormat(L"%128s", arJnlField[31]);
	}
	else
	{
		strResult.Append(L"00000000");
		strResult.AppendFormat(L"%32s", arJnlField[30]);
		strResult.AppendFormat(L"%128s", arJnlField[31]);
	}

	return strResult;
}

CString CreateBitload4UJournalString(CStringArray &arJnlField)
{
	CString strResult, strTmp;

	strResult = JNLTYPE_B4U;

	strTmp.Format(L"%s", arJnlField[3]);
	if(strTmp.GetLength() == 2) 
	{
		strTmp.Format(L"20%s", arJnlField[3]);
	}

	// Date/time
	strResult.AppendFormat(L"%4.4s%2.2s%2.2s%2.2s%2.2s%2.2s", strTmp, arJnlField[4], arJnlField[5], arJnlField[6], arJnlField[7], arJnlField[8]);

	// Terminal ID
	strResult.AppendFormat(L"%16s", arJnlField[9]);

	// Sequence Number
	strResult.AppendFormat(L"%4s", arJnlField[10]);

	// BTC Charged
	strResult.AppendFormat(L"%16s", arJnlField[22]);

	// Transaction Hash
	strResult.AppendFormat(L"%84s", arJnlField[23]);

	// Requested Amount
	strResult.AppendFormat(L"%8s", arJnlField[24]);

	// Dispensed Amount
	strResult.AppendFormat(L"%8s", arJnlField[25]);

	// Transaction Result
	strResult.AppendFormat(L"%2s", arJnlField[1]);

	return strResult;
}

CString CreateDigitalMintJournalString(CStringArray &arJnlField)
{
	// Journal Type (DigitalMint Transaction)
	CString strResult = JNLTYPE_DIGITALMINT;

	// Date/Time
	strResult.AppendFormat(L"%s %2.2s:%2.2s:%2.2s", GetCmnLocalDate(arJnlField[4], arJnlField[5], arJnlField[3]), arJnlField[6], arJnlField[7], arJnlField[8]);

	// Terminal ID
	strResult.AppendFormat(L"%16s", arJnlField[9]);

	// Session ID
	strResult.AppendFormat(L"%36s", arJnlField[10]);

	// Transaction ID
	strResult.AppendFormat(L"%36s", arJnlField[11]);

	// Account ID
	strResult.AppendFormat(L"%36s", arJnlField[12]);

	// API URL
	strResult.AppendFormat(L"%32s", arJnlField[13]);

	// Selected Coin
	strResult.AppendFormat(L"%10s", arJnlField[14]);

	// Transaction Mode
	strResult.AppendFormat(L"%1s", arJnlField[15]);

	// Transaction Amount
	strResult.AppendFormat(L"%8s", arJnlField[16]);

	// Error
	if (arJnlField[1] == DIGITALMINT_TXN)
	{
		strResult.Append(L"       ");
	}
	else if (arJnlField[17].IsEmpty())
	{
		strResult.Append(L"0000000");
	}
	else
	{
		strResult.AppendFormat(L"%7s", arJnlField[17]);
	}

	return strResult;
}