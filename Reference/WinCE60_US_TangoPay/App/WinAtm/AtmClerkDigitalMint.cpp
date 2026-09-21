#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"
#include ".\Common\DigitalMintConfigurationManager.h"
#include ".\Common\NHDbgApi.h"

#if (APP_DIGITALMINT)

CString CMainFrame::ClerkDigitalMintScreenDataMake(int screenNumber)
{
	switch (screenNumber)
	{
		case 1020: return ClerkDigitalMintGetGeneralOptions();
	}

	return CString(L"");
}

CString CMainFrame::ClerkDigitalMintGetGeneralOptions()
{
	DMConfiguration config;

	if (!m_pTranCmn->m_DMConfigManager->GetConfiguration(config))
	{
		NHERROR((L"Could not get DM configuration\r\n"));
		return CString(L"");
	}

	DMVersionResponse response;

	#ifdef APP_LOCAL_MODE
		response.Version = L"9e19aa0a79b76e6aa4cc2e3a45a0adeac25bb027";
	#else
		if (m_pTranCmn->m_DMService)
		{
			if (!m_pTranCmn->m_DMService->VerifyService())
				m_pTranCmn->UpdateDigitalMintServices(config.AtmAPIUrl);

			m_pTranCmn->m_DMService->GetVersion(response);
		}
	#endif

	CString data(L"");

	data.Format(L"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		L"", SCR_CMD_DELIMITER,
		config.IsEnabled ? L"Enabled" : L"Disabled", SCR_CMD_DELIMITER,
		config.GetTransactionMode(), SCR_CMD_DELIMITER,
		ClerkDigitalMintTrimValue(m_pTranCmn->m_DMConfigManager->APIKey(config)), SCR_CMD_DELIMITER,
		ClerkDigitalMintTrimValue(m_pTranCmn->m_DMConfigManager->APISecret(config)), SCR_CMD_DELIMITER,
		m_pTranCmn->m_DMConfigManager->PlatformHash(config), SCR_CMD_DELIMITER,
		m_pTranCmn->m_DMConfigManager->APIUrl(config), SCR_CMD_DELIMITER,
		response.Version);

	return data;
}

CString CMainFrame::ClerkDigitalMintTrimValue(CString value)
{
	CString ret = value;

	if (ret.GetLength() > 7)
	{
		ret = ret.Left(7);

		for (int i = 0; i < 4; i++)
		{
			ret.Append(L"*");
		}
	}

	return ret;
}

CString CMainFrame::ClerkDigitalMintEnableButtons(int screenNumber)
{
	switch (screenNumber)
	{
		case 1020: return ClerkDigitalMintGetGeneralOptionButtons();
	}

	return CString(L"");
}


CString CMainFrame::ClerkDigitalMintGetGeneralOptionButtons()
{
	DMConfiguration config;

	if (!m_pTranCmn->m_DMConfigManager->GetConfiguration(config))
	{
		NHERROR((L"Could not get DM configuration\r\n"));
		return CString(L"");
	}

	CString data;

	bool showEnableButton = config.TransactionMode != DMTXNMODE_UNREGISTERED;
	bool showAtmRegisterButton = config.AtmPlatformHash.IsEmpty() || config.AtmAPISecret.IsEmpty() || config.AtmAPIKey.IsEmpty();
	bool showSidecarRegisterButton = ClerkDigitalMintIsSidecarModeAvailable() && (config.SidecarPlatformHash.IsEmpty() || config.SidecarAPISecret.IsEmpty() || config.SidecarAPIKey.IsEmpty());

	data.Append(showEnableButton ? L"on \r" : L"off\r");			// F1-En/Disable
	data.Append(showAtmRegisterButton ? L"on \r" : L"off\r");		// F3-Enable ATM Mode
	data.Append(showSidecarRegisterButton ? L"on \r" : L"off\r");	// F5-Enable Sidecar Mode
	data.Append(L"on \ron ");										// F7-Transaction Mode, F8-Reset Settings

	return data;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDigitalMintGeneralOptions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1020 PROC - General options for DigitalMint
-------------------------------------------------------------------*/
void CMainFrame::ClerkDigitalMintGeneralOptions()
{
	DMConfiguration config;

	if (!m_pTranCmn->m_DMConfigManager->GetConfiguration(config))
	{
		m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
		m_OpInfo.nProcNextScrNum = 795;
		ClerkNoticeToUser(T_OPERATE_NG);
		return;
	}

	while (TRUE)
	{
		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		// F1
		if (m_OpInfo.strReturn == L"ENABLE")
		{
			config.IsEnabled = !config.IsEnabled;

			if (m_pTranCmn->m_DMConfigManager->SaveConfiguration(config))
			{
				// Enablement changed
				NHDBG((L"Changed DigitalMint Enablement: %d\r\n", config.IsEnabled));
			}
			else
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F3
		else if (m_OpInfo.strReturn == L"ENABLEATM")
		{
			CString shortcode;
			CString message = m_pDevCmn->fstrSCR_GetStringByTextID(T_OPERATE_OK);
			BIZ_RETURN nRes = RES_NG;

			if (ClerkInputAlpha(L"1020DF3T", L"", 1, 8, shortcode))
			{
				ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);

				m_pTranCmn->UpdateDigitalMintServices(config.AtmAPIUrl);

				DMDeviceRegistrationResponse response;

#ifdef APP_LOCAL_MODE
				// From Reg Code: XVGQPXDM
				config.AtmAPIKey = L"767d6798-4d85-4513-a9e3-10973f37200e";
				config.AtmAPISecret = L"iYYXsA6bdvHqM/knLCcamK0TMuC1aUeRjGpSudgB09k=";
				config.AtmPlatformHash = L"f4a58b7c-2e61-4130-a0bc-924008aac023";

				// DMTODO: We don't have these but DM is returning them:
				// config.Environment = L"api.staging.digital-mint.com/v2";
				// config.DeviceID = L"atm-shortcode-test-3";
				// config.DataURIA = L"";
				// config.DataURIB = L"";
				// config.DataURIC = L"";
				// config.DataURID = L"";

				config.TransactionMode = DMTXNMODE_ATM;
				config.IsEnabled = true;

				if (!m_pTranCmn->m_DMConfigManager->SaveConfiguration(config))
				{
					message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E2");
				}
				else
				{
					nRes = RES_OK;
				}
#else
				if (m_pTranCmn->m_DMService->RegisterAtm(shortcode, response))
				{
					if (response.Code == 0)
					{
						config.AtmAPIKey = response.ApiKey;
						config.AtmAPISecret = response.ApiSecret;
						config.AtmPlatformHash = response.PlatformHash;
						config.TransactionMode = DMTXNMODE_ATM;
						config.IsEnabled = true;

						if (!m_pTranCmn->m_DMConfigManager->SaveConfiguration(config))
						{
							message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E2");
						}
						else
						{
							m_pTranCmn->UpdateDigitalMintServices(config.AtmAPIUrl);
							nRes = RES_OK;
						}
					}
					else
					{
						message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E2");
					}
				}
				else
				{
					message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E1");
				}
#endif

				if (nRes == RES_OK)
				{
					if (m_pTranCmn->H_NH_DM_ReportAtmState(config, true) != RES_OK)
					{
						message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E1");
					}
				}

				ClerkNoticeToUser(message);
			}

			break;
		}
		// F5
		else if (m_OpInfo.strReturn == L"ENABLESIDECAR")
		{
			CString code;
			CString message = m_pDevCmn->fstrSCR_GetStringByTextID(T_OPERATE_OK);
			BIZ_RETURN nRes = RES_NG;

			if (ClerkDigitalMintBarcodeReader(code) == RES_OK)
			{
				ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);

				m_pTranCmn->UpdateDigitalMintServices(config.SidecarAPIUrl);

				DMDeviceRegistrationResponse response;

#ifdef APP_LOCAL_MODE
				// From Reg Code: 337228ee-661f-4658-9d5e-06c3564012ad
				config.SidecarAPIKey = L"6544f762-f525-487a-9709-a31061af2aa4";
				config.SidecarAPISecret = L"fnMNZGIcN5dezhc9DjxUYq5kROIALJjOHVebAFJhWkU=";
				config.SidecarPlatformHash = L"2afcd515-ecec-4a13-89e9-14f47a13f540";

				// DMTODO: We don't have these but DM is returning them:
				// config.Environment = L"api.staging.digital-mint.com/v2";
				// config.DeviceID = L"sidecar-5";
				// config.DataURIA = L"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAQAAAAEAAQMAAABmvDolAAAABlBMVEX///8AAABVwtN+AAAChklEQVR42uyZMY7rMAxEx3ChUkfQTeyLGbCAXMy+iY7gUoXg+Rgq2Tj7u48PhIVVLDb2K1ZccjhkcJ/73OffzkryWAt3Pgo3LAkDWyLZHAELMBxI4cRa7EHVx/7CD5C468nMhnDGRyHF83AHLKgTRto7njF7BeYDqc7MqU6AO0D5sLLOooZjJAbmvxLmy4BV1pLC/vHjd+l9GehUCfsxFijU4dc7D8CCsLM9ozzFBkyR5Hn5X3wfGBnO2FBntgTEnMhjZCCzI8CKaklhw1gwsF8gVbuPG2AsdYosdeCDJk91Jkud3qH2ALDOfNB+gzTAIn8pfw+ARXllVXnxhGXuo4QtZk8AMMuBkCzhVHNH7EF3BXDTk02ossA0tF7bwfcBQPVULBXM0L0SuTkCTKMexYpe8mRBpzV8P4D1yAxu3XDaVTKu7cADYPWewk4y7LJJehr2A46AtdThWAummJMZprBFSvebJ4BSdt1CSdGtu3Lk4uW+D0jjmWFRlobSLvXRkr4PKLZd6BVv6+sKP2JzBNiQm9Mryot5uXLNagfA+qOc6vDS/b0LVXYEaBInWaG/XTbJVggfLckBkMLZvRKf1n3WpDG9O5YDYCzm2rsGmAMx/zn9OBAPgLycPgyH3vWPDWF7T2oOACTLUkySVN0n2J7jMlO6AGwFY0E3DcCE12DpBlhpWarB1xYHciCU/4Qj4NU8bRx62qSG8PZyHoC+h5F5Qwp8zbxhgyfANlojzWZK8ovttj7MngPAdoPpudNQ0g6f+eAGUCk9TXzqJn5+L9W9AEh1eC0ObFyrgy+g50P/UkKTRp2P9apRHoD+HQrNF0no1dyXZE7ZD3Cf+9znf58/AQAA//+frj9YltHJRQAAAABJRU5ErkJggg==";
				// config.DataURIB = L"";
				// config.DataURIC = L"";
				// config.DataURID = L"";

				config.TransactionMode = DMTXNMODE_SIDECAR;
				config.IsEnabled = true;

				if (!m_pTranCmn->m_DMConfigManager->SaveConfiguration(config))
				{
					message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E2");
				}
				else
				{
					nRes = RES_OK;
				}
#else
				if (m_pTranCmn->m_DMService->RegisterSidecar(code, response))
				{
					if (response.Code == 0)
					{
						config.SidecarAPIKey = response.ApiKey;
						config.SidecarAPISecret = response.ApiSecret;
						config.SidecarPlatformHash = response.PlatformHash;
						config.TransactionMode = DMTXNMODE_SIDECAR;
						config.IsEnabled = true;

						if (!m_pTranCmn->m_DMConfigManager->SaveConfiguration(config))
						{
							message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E2");
						}
						else
						{
							m_pTranCmn->UpdateDigitalMintServices(config.SidecarAPIUrl);
							nRes = RES_OK;
						}
					}
					else
					{
						message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E2");
					}
				}
				else
				{
					message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E1");
				}
#endif

				if (nRes == RES_OK)
				{
					if (m_pTranCmn->H_NH_DM_ReportSidecarState(config, true) != RES_OK)
					{
						message = m_pDevCmn->fstrSCR_GetStringByTextID(L"APTextID:1020DF3E1");
					}
				}

				ClerkNoticeToUser(message);
			}

			break;
		}
		// F7
		else if (m_OpInfo.strReturn == L"MODE")
		{
			int mode = config.TransactionMode;
			mode++;

			bool allowAtmMode = !config.AtmPlatformHash.IsEmpty() && !config.AtmAPISecret.IsEmpty() && !config.AtmAPIKey.IsEmpty();
			bool allowSidecarMode = ClerkDigitalMintIsSidecarModeAvailable() && (!config.SidecarPlatformHash.IsEmpty() && !config.SidecarAPISecret.IsEmpty() && !config.SidecarAPIKey.IsEmpty());

			if (mode == DMTXNMODE_ATM && !allowAtmMode)
			{
				mode++;
			}

			if (mode == DMTXNMODE_SIDECAR && !allowSidecarMode)
			{
				mode++;
			}

			if (mode >= DMTXNMODE_MAX || mode < DMTXNMODE_UNREGISTERED)
			{
				mode = DMTXNMODE_UNREGISTERED;
			}

			config.TransactionMode = mode;

			if (m_pTranCmn->m_DMConfigManager->SaveConfiguration(config))
			{
				// Enablement changed
				NHDBG((L"Changed DigitalMint Mode: %d\r\n", config.TransactionMode));
			}
			else
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}

			break;
		}
		// F8
		else if (m_OpInfo.strReturn == L"RESET")
		{
			bool bChanged = false;

			if (config.TransactionMode == DMTXNMODE_ATM)
			{
				bChanged = true;

				config.AtmAPISecret = L"";
				config.AtmAPIKey = L"";
				config.AtmPlatformHash = L"";

				config.TransactionMode = DMTXNMODE_UNREGISTERED;
			}
			else if (config.TransactionMode == DMTXNMODE_SIDECAR)
			{
				bChanged = true;

				config.SidecarAPISecret = L"";
				config.SidecarAPIKey = L"";
				config.SidecarPlatformHash = L"";

				config.TransactionMode = DMTXNMODE_UNREGISTERED;
			}

			if (bChanged)
			{
				if (m_pTranCmn->m_DMConfigManager->SaveConfiguration(config))
					ClerkNoticeToUser(T_OPERATE_OK);
				else
					ClerkNoticeToUser(T_OPERATE_NG);
			}

			break;
		}

		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 795; // Back to Other Transactions
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
			break;
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDigitalMintBarcodeReader()
 RETURN TYPE  : BIZ_RETURN
 PARAMETER    : code[out] DigitalMint Sidecar registration code
 DESCRIPTION  : SCR 1021 PROC - Scan QR code for Sidecar registration
-------------------------------------------------------------------*/
BIZ_RETURN CMainFrame::ClerkDigitalMintBarcodeReader(CString &code)
{
	BIZ_RETURN result = RES_USER_TIMEOUT;
	NHDBG((L"[CMainFrame::ClerkDigitalMintBarcodeReader]\n"));
	NVDump('O', 'C', "2R", L"Start", L"ClerkDigitalMintBarcodeReader");

	CString bcrData, GetKeyStr;

	m_OpInfo.nProcNextScrNum = 1021;
	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BCR, FLICKER_ON);

	ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
	m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

	if (m_pDevCmn->fnBCR_SendRawData(FALSE))
	{
		while (TRUE)
		{
			if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BCR))
			{
				// Check device with reduced time
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BCR, MIN_DEVRSP_TIME);
				m_pDevCmn->fnAPL_CheckDevice();

				CString errorCode = m_pDevCmn->fstrBCR_GetErrorCode();
				bcrData = m_pDevCmn->fstrBCR_GetBarcodeData();

				if (errorCode == L"0000000" && !bcrData.IsEmpty())
				{
					if (ClerkDigitalMintParseScan(code, bcrData) == RES_OK)
					{
						m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;
						m_OpInfo.nProcNextScrNum = 1020;
						result = RES_OK;
					}
					else
					{
						m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
						m_OpInfo.nProcNextScrNum = 1020;
						result = RES_USER_EXIT;
					}

					break;
				}
				else
				{
					NHDEBUG(DBG_INFO, (_T("Failed to scan. Device error %s\n"), errorCode));
					NVDump('F', 'C', "2R", L"Failed to scan", L"ClerkDigitalMintBarcodeReader");

					// 9792001 -> Timeover error
					if (errorCode == L"9792001")
					{
						m_pDevCmn->fnBCR_ClearErrorCode();
					}

					m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
					m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
					result = RES_USER_TIMEOUT;
					break;
				}
			}
			else if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_BCR, EVENT_OUT))
			{
				NHDEBUG(DBG_INFO, (_T("Failed to scan - fnAPL_GetDeviceEvent(DEV_BCR, EVENT_OUT)\n")));
				NVDump('F', 'C', "2R", L"Failed to scan", L"ClerkDigitalMintBarcodeReader");

				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_BCR, MIN_DEVRSP_TIME);
				m_pDevCmn->fnAPL_CheckDevice();

				result = RES_NG;
			}
			else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					NHDEBUG(DBG_INFO, (_T("CANCEL OR EXIT BUTTON PRESSED\n")));
					m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
					m_OpInfo.nProcNextScrNum = 1020;
					result = RES_USER_EXIT;
					break;
				}
				else if (GetKeyStr == S_TIMEOVER)
				{
					NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
					m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
					m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
					result = RES_USER_TIMEOUT;
					break;
				}
			}

			Delay_Msg(PROC_LOOFDELAY);
		}

		if (result != RES_OK)
		{
			m_pDevCmn->fnBCR_CancelAccept();
		}
	}
	else
	{
		// DMTODO: Specific Error for BCR
		NVDump('F', 'C', "2R", L"Scan BCR Error", L"ClerkDigitalMintBarcodeReader");
		result = RES_NG;
	}

#ifndef UNDER_CE
	while (TRUE)
	{
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NHDEBUG(DBG_INFO, (_T("CANCEL OR EXIT BUTTON PRESSED\n")));
				m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
				m_OpInfo.nProcNextScrNum = 1020;
				result = RES_USER_EXIT;
				break;
			}
			else if (GetKeyStr == S_TIMEOVER)
			{
				NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
				m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
				m_OpInfo.nProcNextScrNum = DIAG_TIMEOUT_SCREEN_NUMBER;
				result = RES_USER_TIMEOUT;
				break;
			}
			else if (GetKeyStr == S_ENTER)
			{
				if (ClerkDigitalMintParseScan(code, L"api.staging.digital-mint.com/v2/register/device/f5db0f48-50f8-46fb-bcae-f64a5c6d3e5e") == RES_OK)
				{
					m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;
					m_OpInfo.nProcNextScrNum = 1020;
					result = RES_OK;
				}
				else
				{
					m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
					m_OpInfo.nProcNextScrNum = 1020;
					result = RES_USER_EXIT;
				}

				break;
			}
		}

		Delay_Msg(PROC_LOOFDELAY);
	}
#endif

	m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_BCR, FLICKER_OFF);

	ClerkScreenDisplay(m_OpInfo.nProcNextScrNum, m_OpInfo.nTimeout);
	m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

	return result;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkDigitalMintParseScan()
 RETURN TYPE  : BIZ_RETURN
 PARAMETER    : retData[out] The parsed data
 PARAMETER    : rawData[in] The raw data from the BCR scan
 DESCRIPTION  : Parse scan from the DigitalMint QR to grab the registration code
-------------------------------------------------------------------*/
BIZ_RETURN CMainFrame::ClerkDigitalMintParseScan(CString &retData, CString rawData)
{
	int pos = 0;
	CString field;
	BIZ_RETURN nRes = RES_OK;

	if (!rawData.IsEmpty())
	{
		CString token = rawData.Tokenize(L"/", pos);

		while (!token.IsEmpty())
		{
			field = token;
			token = rawData.Tokenize(L"/", pos);
		}

		retData = field;
		nRes = RES_OK;
	}
	else
	{
		nRes = RES_NG;
	}

	return nRes;
}

bool CMainFrame::ClerkDigitalMintIsSidecarModeAvailable()
{
#ifdef UNDER_CE
	bool bAvailable = false;
#else
	bool bAvailable = true;
#endif

	// Ensure sidecar support is enabled
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
	{
		bAvailable = true;

		// Update device status
		m_pDevCmn->fnAPL_CheckDevice();

		// Check BNA
		bAvailable &= m_pDevCmn->fnBNA_GetAggregateDeviceStatus() == NORMAL;

		// Check BCR
		bAvailable &= m_pDevCmn->fnBCR_GetDeviceStatus() == NORMAL;
	}

	return bAvailable;
}

#endif