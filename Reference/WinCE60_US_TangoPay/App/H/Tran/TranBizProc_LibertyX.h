#pragma once
#if(APP_LIBERTYX)

//
// [#RWC6-224]	US William 2020.10.12 LibertyX Dispense
//

public:
	// Purchase Transaction
	BIZ_RETURN P_NH_LTXP_OrderID(CString &orderId);
	BIZ_RETURN P_NH_LTXP_Amount(double &amount);
	BIZ_RETURN P_NH_LTXP_InsertCard();
	BIZ_RETURN P_NH_LTXP_TransactionSelection();
	BIZ_RETURN P_NH_LTXP_Confirm();
	BIZ_RETURN P_NH_LTXP_Authorize(LXAuthorization auth, LXAuthorizationResponse &response);
	BIZ_RETURN P_NH_LTXP_Debit(CString &errorCode, CString &errorMessage);
	BIZ_RETURN P_NH_LTXP_Execute(LXExecute execute, LXExecuteResponse &executeResponse);
	BIZ_RETURN P_NH_LTXP_Cancel(LXCancel cancel, LXCancelResponse &response, bool reverseAtmDebit);
	BIZ_RETURN P_NH_LTXP_Success();
	BIZ_RETURN P_NH_LTXP_Receipt();
	BIZ_RETURN P_NH_LTXP_DigitalReceipt();
	BIZ_RETURN P_NH_LTXP_AuthFail(CString errorMessage);
	BIZ_RETURN P_NH_LTXP_ExecFail(CString errorMessage);
	BIZ_RETURN P_NH_LTXP_DebitFail(CString errorCode, CString errorMessage);
	BIZ_RETURN P_NH_LTXP_BillPay();

	// Purchase ADA Transaction
	BIZ_RETURN P_NH_LTXP_OrderID_ADA(CString &orderId);
	BIZ_RETURN P_NH_LTXP_Amount_ADA(double &amount);
	BIZ_RETURN P_NH_LTXP_InsertCard_ADA();
	BIZ_RETURN P_NH_LTXP_TransactionSelection_ADA();
	BIZ_RETURN P_NH_LTXP_Confirm_ADA();
	BIZ_RETURN P_NH_LTXP_Success_ADA();
	BIZ_RETURN P_NH_LTXP_BillPay_ADA();
	BIZ_RETURN P_NH_LTXP_AuthFail_ADA(CString errorMessage);
	BIZ_RETURN P_NH_LTXP_ExecFail_ADA(CString errorMessage);
	BIZ_RETURN P_NH_LTXP_DebitFail_ADA(CString errorCode, CString errorMessage);
	BIZ_RETURN P_NH_LTXP_Receipt_ADA();

	// Liquidate Transaction
	BIZ_RETURN P_NH_LTXD_EnterToken(CString &token);
	BIZ_RETURN P_NH_LTXD_ProcessTransaction(CString token);
	BIZ_RETURN P_NH_LTXD_Authorize(LXDispenseAuthorization &auth, LXDispenseAuthorizationResponse &response, LXERRORCODE &error);
	BIZ_RETURN P_NH_LTXD_Execute(LXDispenseExecute &execute, LXDispenseExecuteResponse &response, LXERRORCODE &error);
	BIZ_RETURN P_NH_LTXD_Dispense(CString amount, CString &dispensedAmount);
	BIZ_RETURN P_NH_LTXD_ReportTransaction(LXDispenseTransactionReport &report, LXDispenseAuthorizationResponse &authResponse);
	BIZ_RETURN P_NH_LTXD_PrintingReceipt(CString token, int journalIdx, LXError *err);
	BIZ_RETURN P_NH_LTXD_Cancel(LXDispenseCancel &cancel);
	BIZ_RETURN P_NH_LTXD_Error(CString error, CString message);
	BIZ_RETURN P_NH_LTXD_ThankYou();

	// Liquidate ADA Transaction
	BIZ_RETURN P_NH_LTXD_EnterToken_ADA(CString &token);
	BIZ_RETURN P_NH_LTXD_PrintingReceipt_ADA(CString token, int journalIdx, LXError *err);
	BIZ_RETURN P_NH_LTXD_Error_ADA(CString error, CString message);
	//BIZ_RETURN P_NH_LTXD_ThankYou_ADA();

	// Utility Functions
	int GetNumberOfLibertyXAds();	// [#RWC6-280] LibertyX Default Ads

private:
	BIZ_RETURN PrintReceipt();
	BIZ_RETURN PlayLibertyXADAMessage(int screenNumber);
#endif
