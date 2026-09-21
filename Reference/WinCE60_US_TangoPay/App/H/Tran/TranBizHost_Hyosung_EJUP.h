#pragma once

typedef CString (*JournalFormatter)(CStringArray&);
typedef CMapStringToPtr JournalFormatterMap;

// NH Standard 1 EJUP Journal Formatting
CString		CreateTransactionJournalString(CStringArray &arJnlField);
CString		CreateErrorModeJournalString(CStringArray &arJnlField);
CString		CreateAddCashJournalString(CStringArray &arJnlField);
CString		CreateChangeDenomJournalString(CStringArray &arJnlField);
CString		CreateDayTotalJournalString(CStringArray &arJnlField);
CString		CreateCassetteTotalJournalString(CStringArray &arJnlField);
CString		CreateVatDayTotalJournalString(CStringArray &arJnlField);
CString		CreateBasicTextJournalString(CStringArray &arJnlField);
CString		CreateChangeProcessorJournalString(CStringArray &arJnlField);
CString		CreateOperatorActionJournalString(CStringArray &arJnlField);
CString		CreateLibertyXPurchaseJournalString(CStringArray &arJnlField);
CString		CreateLibertyXSellJournalString(CStringArray &arJnlField);
CString		CreateBitload4UJournalString(CStringArray &arJnlField);
CString		CreateDigitalMintJournalString(CStringArray &arJnlField);
