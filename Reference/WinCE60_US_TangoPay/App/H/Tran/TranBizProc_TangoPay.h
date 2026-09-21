#pragma once

// [RWC6-676] SKKim 2024.03.28 added new files

#if (APP_TANGOPAY)
	typedef struct STRUCT_TangoPayTransData
	{
		CString strID;
		CString strTerminalID;
		CString strPartnerID;
		CString strAccessToken;
		CString strPhoneNumber;
		CString strPhoneCountryCode;
		CString strUserId;
		CString strOTPCode;
		CString strOTPSeed;

		// PreStaging Verify
		CString strTransactionId;
		CString strConfirmationNumber;
		CString strPrestagingId;

		// PreStaging Validation
		CString strMgiTransactionSessionId;
		CString strReceiveCountry;
		CString strReceiveName;
		CString strTotalAmount;
		CString strSendFees;
		CString strSendTaxes;
		CString strSendAmount;
		CString strSendCurrency;
		CString strReceiveMethod;
		CString strReceiveAmount;
		CString strReceiveCurrency;
		CStringArray strArr_FraudWarning[2];
		CStringArray strArr_Disclosures[2];

		// Commit
		CString strReferenceNumber;
		CStringArray strArr_AgentReceipts;
		CStringArray strArr_ConsumerReceipts;

		CString strDepositedAmount;
		CString strErrorReason;

		BOOL	bNeedSaveToEJNL;

		STRUCT_TangoPayTransData::STRUCT_TangoPayTransData()
		{
			Reset();
		};

		void Reset()
		{
			strID.Empty();
			strTerminalID.Empty();
			strPartnerID.Empty();
			strAccessToken.Empty();
			strPhoneNumber.Empty();
			strPhoneCountryCode.Empty();
			strUserId.Empty();
			strOTPCode.Empty();
			strOTPSeed.Empty();
			strTransactionId.Empty();
			strConfirmationNumber.Empty();
			strPrestagingId.Empty();
			strMgiTransactionSessionId.Empty();
			strReceiveCountry.Empty();
			strReceiveName.Empty();
			strTotalAmount.Empty();
			strSendFees.Empty();
			strSendTaxes.Empty();
			strSendAmount.Empty();
			strSendCurrency.Empty();
			strReceiveMethod.Empty();
			strReceiveAmount.Empty();
			strReceiveCurrency.Empty();

			strArr_FraudWarning[0].RemoveAll();
			strArr_FraudWarning[1].RemoveAll();

			strArr_Disclosures[0].RemoveAll();
			strArr_Disclosures[1].RemoveAll();

			strReferenceNumber.Empty();
			strArr_AgentReceipts.RemoveAll();
			strArr_ConsumerReceipts.RemoveAll();

			strDepositedAmount.Empty();
			strErrorReason.Empty();

			bNeedSaveToEJNL = FALSE;
		};

	} TangoPayTransData;
  

	//
	// TangoPay Processes
	//

	public:
		TangoPayTransData m_sTangoPayTransData;

		void Load_TangoPayDefaultTransData();

		BOOL P_NH_IsTangoPayAvailable();

		BOOL P_NH_WaitforFrontBillRemoval(bool bDispkayGuideScreen = true, DWORD nTimeout = 10000);	// 입금 거래시 투입구 앞 지폐 유무 처리 (default 30sec)

		void P_NH_TangoPay_DisplayHostError(CString strErrorCode, CString strErrorMsg);		// [RWC6-676] SKKim 2024.05.28

		// Terminal Identify Process
		BIZ_RETURN P_NH_TangoPay_ResetApiKey();	// Test Only


		BIZ_RETURN P_NH_TangoPay_GetPublicKey();

		// Transaction Process
		BIZ_RETURN P_NH_TangoPay_GetTerminalJWT(bool bDisplayProcessingScreen = false);

		BIZ_RETURN P_NH_TangoPay_SelectLanguage();

		BIZ_RETURN P_NH_TangoPay_Enter_PhoneNumber();

		BIZ_RETURN P_NH_TangoPay_Enter_OTP();

		BIZ_RETURN P_NH_TangoPay_StagingValidation(bool bDisplayProcessingScreen = false);

		BIZ_RETURN P_NH_TangoPay_PreTransactionSummary();

		BIZ_RETURN P_NH_TangoPay_CustomerNoticeMessage(int nMsgType);

		BIZ_RETURN P_NH_TangoPay_AcceptCashProcess();

		BIZ_RETURN P_NH_TangoPay_TransactionSummary();

		BIZ_RETURN P_NH_TangoPay_Commit();

		BIZ_RETURN P_NH_TangoPay_TransactionCompleteData();

		BIZ_RETURN P_NH_TangoPay_PrintTransactionReceipt();

		BIZ_RETURN P_NH_TangoPay_CancelConfirmation();

		BIZ_RETURN P_NH_TangoPay_ErrorPrintReceipt();

		BIZ_RETURN P_NH_TangoPay_DownloadAppGuideScreen();	// [RWC6-676] SKKim 2024.05.29

		BOOL	TangoPay_WriteJournalEntry(CString strJnlCode);

#endif