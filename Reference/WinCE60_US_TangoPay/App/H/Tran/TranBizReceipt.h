#ifndef __TRAN_BIZ_RECEIPT_H__
#define __TRAN_BIZ_RECEIPT_H__

public:
//	BIZ_RETURN	LIB_PrintData(BOOL bTemp=TRUE);		// [#2147] KSK 2012.08.18 사용안하는 함수 정리
	void		LIB_GetDisplayData(CString &strPrintData);

	BOOL		LIB_AllSetupPrint();
	// [#2434] Kook AU 2016.07.19 Receipt Optimizing with refactoring (Print All Setup)
	void			LIB_GetSetupHeaderPrintData(CString &strPrintData);
	void			LIB_GetVersionInformationPrintData(CString &strPrintData);			// [#2326] Kook 2015.04.07 SW Version 출력부 공용화
	void			LIB_GetSystemSetupPrintData(CString &strPrintData);
	void				LIB_GetHaloLedPrintData(CString &strPrintData, int pRequiredLedState);
	void			LIB_GetCustomerSetupPrintData(CString &strPrintData);
	void			LIB_GetTransactionSetupPrintData(CString &strPrintData);
	void			LIB_GetHostSetupPrintData(CString &strPrintData);
	// end of [#2434]

	BOOL		LIB_ALLBINPrint();											// [#2000] SOOK 2010.10.12 호주 사양 적용 (Multiple BIN List)


protected:
	BIZ_RETURN	P_NH_NOR_TransactionPrint(BOOL bCut=TRUE);		// 추후 각 거래로 분리 예정..
//	void		P_NH_NOR_AmountAndBalancePrintProc(CString &strPrintData);
	void		P_NH_NOR_AmountAndBalancePrintProc(CString &strPrintData, int nDispScreen = FALSE, int nDualReceiptLang = FALSE);	// [#2147] NH KSK 2012.08.18 화면과 명세표 인자 관련 공통 로직을 수행하기 위해 Parameter 추가
	void		P_MX_NOR_BankNameandFeePrintProc(CString &strPrintData, int nDualReceiptLang = FALSE);	// [#2115] MX KSK 2012.02.04
	CString		GetAPTextInOtherLang(CString APText, int nDualReceiptLang = FALSE);
#endif