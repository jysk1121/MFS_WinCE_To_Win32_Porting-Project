#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"
#include ".\Common\LibertyXConfigurationManager.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

const int ENV_OPTS_COUNT = 3;
const int ENV_OPTIONS[ENV_OPTS_COUNT] = {
	LX_VISAII,
	LX_STANDARD,
	LX_ACK
};

CString CMainFrame::ClerkLibertyXScreenDataMake(int screenNumber)
{
	switch (screenNumber)
	{
	case 1010: return ClerkLibertyXGetGeneralOptions();
	case 1011: return ClerkLibertyXGetHostOptions();
	case 1012: return ClerkLibertyXGetApiOptions();
	}

	return CString(L"");
}

CString CMainFrame::ClerkLibertyXGetGeneralOptions()
{
	LXConfiguration config;

	if (!m_LXConfig->GetConfiguration(config))
	{
		NHERROR((L"Could not get LX configuration\r\n"));
		return CString(L"");
	}

	CString data(L"");
	data.Format(L"%s%s%s%s%s%s%s", 
		config.BuyBitcoinEnabled ? L"Enabled" : L"Disabled", SCR_CMD_DELIMITER,
		config.LocationID, SCR_CMD_DELIMITER,
		config.RoutingID, SCR_CMD_DELIMITER,
		config.SellBitcoinEnabled ? L"Enabled" : L"Disabled");

	return data;
}

CString CMainFrame::ClerkLibertyXGetHostOptions()
{
	LXConfiguration config;

	if (!m_LXConfig->GetConfiguration(config))
	{
		NHERROR((L"Could not get LX configuration\r\n"));
		return CString(L"");
	}

	CString data(L"");
	data.Format(L"%s%s%s%s%s%s%s%s%d%s%d%s%s",
		config.GetHostProtocolName(), SCR_CMD_DELIMITER,
		config.TlsEnabled ? L"Enabled" : L"Disabled", SCR_CMD_DELIMITER,
		config.Host1Url, SCR_CMD_DELIMITER,
		config.Host2Url, SCR_CMD_DELIMITER,
		config.Host1Port, SCR_CMD_DELIMITER,
		config.Host2Port, SCR_CMD_DELIMITER,
		config.GetHostEnvelopeName());

	return data;
}

CString CMainFrame::ClerkLibertyXGetApiOptions()
{
	LXConfiguration config;

	if (!m_LXConfig->GetConfiguration(config))
	{
		NHERROR((L"Could not get LX configuration\r\n"));
		return CString(L"");
	}

	CString data(L"");
	data.Format(L"%s%s%s%s%s%s%d min(s)",
		config.APIUrl, SCR_CMD_DELIMITER,
		config.Username, SCR_CMD_DELIMITER,
		config.Password, SCR_CMD_DELIMITER,
		config.HeartbeatIntervalMinutes);

	return data;
}

CString CMainFrame::ClerkLibertyXScreenEnableButton(int screenNumber)
{
	switch (screenNumber)
	{
		case 1010: return ClerkLibertyXGetGeneralOptionButtons();
	}

	return CString(L"");
}

CString CMainFrame::ClerkLibertyXGetGeneralOptionButtons()
{
	LXConfiguration config;

	if (!m_LXConfig->GetConfiguration(config))
	{
		NHERROR((L"Could not get LX configuration\r\n"));
		return CString(L"");
	}

	CString data;
	bool showHostOptionBtn = config.HostDepModeEnabled;

	data.Append(L"on \r");									// F1-ENABLE
	data.Append(showHostOptionBtn ? L"off\r" : L"on \r");	// F2-HOSTOPT (enabled = button off, disabled = button on)
	data.Append(L"on \ron \ron \ron \r");					// F3-CASHOUTENABLE, F4-APIOPT, F6-LOCATIONID, F8-ROUTINGID

	return data;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkLibertyXOptions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1010 PROC - General Landing for LibertyX Options
-------------------------------------------------------------------*/
void CMainFrame::ClerkLibertyXOptions()
{
	LXConfiguration config;

	if (!m_LXConfig->GetConfiguration(config))
	{
		NHERROR((L"Failed to get LibertyX configuration\r\n"));

		m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
		m_OpInfo.nProcNextScrNum = m_OpInfo.nLibertyXScrNum; // Back to previous screen
		ClerkNoticeToUser(T_OPERATE_NG);
		return;
	}

	while(TRUE)
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
			config.BuyBitcoinEnabled = !config.BuyBitcoinEnabled;
			if (m_LXConfig->SaveConfiguration(config))
			{
				// Enablement changed
				NHDEBUG(DBG_CALL, (L"Changed LibertyX buy btc enablement: %d\r\n", config.BuyBitcoinEnabled));
			}
			else 
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F2
		else if (m_OpInfo.strReturn == L"HOSTOPT")
		{
			m_OpInfo.nProcNextScrNum = 1011;
			break;
		}
		// F3
		if (m_OpInfo.strReturn == L"CASHOUTENABLE")
		{
			config.SellBitcoinEnabled = !config.SellBitcoinEnabled;
			if (m_LXConfig->SaveConfiguration(config))
			{
				// Dispense changed

				NHDEBUG(DBG_CALL, (L"Changed LibertyX cashout enablement: %d\r\n", config.SellBitcoinEnabled));
			}
			else 
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F4
		else if (m_OpInfo.strReturn == L"APIOPT")
		{
			m_OpInfo.nProcNextScrNum = 1012;
			break;
		}
		// F6
		else if (m_OpInfo.strReturn == L"LOCATIONID")
		{
			CString newLocationId;
			if (ClerkInputAlpha(L"1010DF6T", config.LocationID, 1, 15, newLocationId))
			{
				// Location ID change confirmed
				config.LocationID = newLocationId;
				m_LXConfig->SaveConfiguration(config);
				NHDEBUG(DBG_CALL, (L"Changed LibertyX Location ID: %s\r\n", config.LocationID));
				
				ClerkNoticeToUser(T_OPERATE_OK);
			}
			else 
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F8
		else if (m_OpInfo.strReturn == L"ROUTINGID")
		{
			CString newRoutingId;
			if (ClerkInputAlpha(L"1010DF7T", config.RoutingID, 1, 6, newRoutingId))
			{
				// Routing ID change confirmed
				config.RoutingID = newRoutingId;
				m_LXConfig->SaveConfiguration(config);
				NHDEBUG(DBG_CALL, (L"Changed LibertyX Routing ID: %s\r\n", config.RoutingID));
				
				ClerkNoticeToUser(T_OPERATE_OK);
			}
			else 
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		
		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = m_OpInfo.nLibertyXScrNum; // Back to previous screen
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
 FUNCTION NAME: ClerkLibertyXHostOptions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1011 PROC - Host Processor Options for LibertyX
-------------------------------------------------------------------*/
void CMainFrame::ClerkLibertyXHostOptions()
{
	LXConfiguration config;

	if (!m_LXConfig->GetConfiguration(config))
	{
		NHERROR((L"Failed to get LibertyX configuration\r\n"));

		m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
		m_OpInfo.nProcNextScrNum = 1010; // Back to LibertyX Main
		ClerkNoticeToUser(T_OPERATE_NG);
		return;
	}

	while(TRUE)
	{
		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		// F1 - Host protocol
		if (m_OpInfo.strReturn == L"HOSTPROTO")
		{
			// Toggle host protocol
			config.HostProtocol = (config.HostProtocol + 1) % 4;

			NHDEBUG(DBG_CALL, (L"Changed LibertyX Host Protocol. Now it is: %d\r\n", config.HostProtocol));

			if (!m_LXConfig->SaveConfiguration(config))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
				continue;
			}
		}
		// F2
		else if (m_OpInfo.strReturn == L"HOSTADDRESS1")
		{
			if (ClerkInputAlpha(L"1011DF2T", config.Host1Url, 1, 40, config.Host1Url) 
				&& m_LXConfig->SaveConfiguration(config))
			{
				// Host 1 URL change confirmed

				NHDEBUG(DBG_CALL, (L"Changed LibertyX Host 1 URL: %s\r\n", config.Host1Url));
				
				ClerkNoticeToUser(T_OPERATE_OK);
			}
			else 
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F3 - TLS Enable
		else if (m_OpInfo.strReturn == L"SSLENABLE")
		{
			// Toggle host protocol
			config.TlsEnabled = ! config.TlsEnabled;

			NHDEBUG(DBG_CALL, (L"Changed LibertyX TLS Enablement. Now it is: %d\r\n", config.TlsEnabled));

			if (!m_LXConfig->SaveConfiguration(config))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
				continue;
			}
		}
		// F4
		else if (m_OpInfo.strReturn == L"HOSTADDRESS2")
		{
			if (ClerkInputAlpha(L"1011DF4T", config.Host2Url, 1, 40, config.Host2Url) 
				&& m_LXConfig->SaveConfiguration(config))
			{
				// Host 2 URL change confirmed

				NHDEBUG(DBG_CALL, (L"Changed LibertyX Host 2 URL: %s\r\n", config.Host2Url));
				
				ClerkNoticeToUser(T_OPERATE_OK);
			}
			else 
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F6
		else if (m_OpInfo.strReturn == L"HOSTPORT1")
		{
			CString portTmp(Int2Asc(config.Host1Port));
			if (!ClerkInputNumber(L"1011DF6T", L"", portTmp, EDIT_NORMAL, 5, portTmp))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
				continue;
			}

			config.Host1Port = Asc2Int(portTmp);
			
			if (!m_LXConfig->SaveConfiguration(config))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
				continue;
			}

			// Host 1 URL change confirmed
			NHDEBUG(DBG_CALL, (L"Changed LibertyX Host 1 Port: %d\r\n", config.Host1Port));
			
			ClerkNoticeToUser(T_OPERATE_OK);
		}
		// F7 - Envelope type
		else if (m_OpInfo.strReturn == L"ENVTYPE")
		{
			// Find the current value in the array of options
			int currIndex = 0;
			for (int i = 0; i < ENV_OPTS_COUNT; i++)
			{
				if (ENV_OPTIONS[i] == config.EnvelopeType)
				{
					currIndex = i;
					break;
				}
			}

			// Get the next option in the list
			int nextOption = ENV_OPTIONS[(currIndex + 1) % ENV_OPTS_COUNT];

			config.EnvelopeType = nextOption;
			
			// Save option
			if (!m_LXConfig->SaveConfiguration(config))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
				continue;
			}

			NHDEBUG(DBG_CALL, (L"Changed LibertyX envelope type: %d\r\n", config.EnvelopeType));
		}
		// F8
		else if (m_OpInfo.strReturn == L"HOSTPORT2")
		{
			CString portTmp(Int2Asc(config.Host2Port));
			if (!ClerkInputNumber(L"1011DF8T", L"", portTmp, EDIT_NORMAL, 5, portTmp))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
				continue;
			}

			config.Host2Port = Asc2Int(portTmp);
			
			if (!m_LXConfig->SaveConfiguration(config))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
				continue;
			}

			// Host 1 URL change confirmed
			NHDEBUG(DBG_CALL, (L"Changed LibertyX Host 2 Port: %d\r\n", config.Host2Port));
			
			ClerkNoticeToUser(T_OPERATE_OK);
		}
		// Blank Key
		else if (m_OpInfo.strReturn == L"CONNTEST")
		{
			// Display OP MSG
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);

			m_pDevCmn->HostOpenFlag = FALSE;
			m_pDevCmn->HostOpenRetryTime = 0;
			if (config.HostDepModeEnabled)
			{
				m_pTranCmn->m_HostConfig = HC_LTX_HOST_DEP_MODE;
			}
			else
			{
				m_pTranCmn->m_HostConfig = HC_LIBERTYX;
			}
			
			m_pDevCmn->nPINErrorFlag = WORKINGKEY_INIT;

			if (m_pTranCmn->BIZ_HostConfigProc(FALSE, /* configOption */ 0, /* clearNetworkErrors */ FALSE) == TRUE)
			{
				if ( m_pDevCmn->nPINErrorFlag == WORKINGKEY_DOWNLOAD_SUCCESS)
				{
					m_OpInfo.m_strMsg = T_OPERATE_OK;
				}
				else
				{
					if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_DOWNLOAD_FAIL)
					{
						m_pDevCmn->fnPIN_Initialize();
					}

					m_OpInfo.m_strMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_OPERATE_NG);
				}
			}
			else
			{
				m_OpInfo.m_strMsg = m_pDevCmn->fstrSCR_GetStringFromTextID(T_OPERATE_NG);
			}

			if (m_pDevCmn->fnAPL_CheckError() == FALSE)  // Add error code when one is registered
			{
				m_OpInfo.m_strMsg.AppendFormat(L" (%s)", m_pDevCmn->fstrAPL_GetErrorCode()); 
				m_pDevCmn->fnAPL_ClearError(CLEAR_ERROR_NETWORK);
			}

			m_pDevCmn->nPINErrorFlag = WORKINGKEY_INIT;

			// Set back to default after running this transaction
			m_pTranCmn->m_HostConfig = HC_ATM;

			ClerkNoticeToUser(m_OpInfo.m_strMsg);
		}
		
		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 1010; // Back to LibertyX Main
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
 FUNCTION NAME: ClerkLibertyXAPIOptions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1012 PROC - API options for LibertyX
-------------------------------------------------------------------*/
void CMainFrame::ClerkLibertyXAPIOptions()
{
	LXConfiguration config;

	if (!m_LXConfig->GetConfiguration(config))
	{
		NHERROR((L"Failed to get LibertyX configuration\r\n"));

		m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
		m_OpInfo.nProcNextScrNum = m_OpInfo.nLibertyXScrNum; // Back to previous screen
		ClerkNoticeToUser(T_OPERATE_NG);
		return;
	}

	while(TRUE)
	{
		///////////////////////////////////
		// WAIT FOR USER INPUT
		ClerkWaitScreenInput_byCha();
		m_OpInfo.strReturn = m_pDevCmn->fstrSCR_GetKeyString();

		NHDEBUG(DBG_INFO, (L"USER INPUT [%s]\n", m_OpInfo.strReturn));

		//-----------------------------------
		// PROCESS KEY STRING
		//-----------------------------------
		// F2
		if (m_OpInfo.strReturn == L"APIURL")
		{
			if (ClerkInputAlpha(L"1012DF2T", config.APIUrl, 1, 40, config.APIUrl) 
				&& m_LXConfig->SaveConfiguration(config))
			{
				NHDEBUG(DBG_CALL, (L"Changed LX API URL\r\n"));

				ClerkNoticeToUser(T_OPERATE_OK);
			}
		}
		// F4
		else if (m_OpInfo.strReturn == L"USERNAME")
		{
			if (ClerkInputAlpha(L"1012DF4T", config.Username, 1, 40, config.Username) 
				&& m_LXConfig->SaveConfiguration(config))
			{
				NHDEBUG(DBG_CALL, (L"Changed LX API Username\r\n"));

				ClerkNoticeToUser(T_OPERATE_OK);
			}
		}
		// F6
		else if (m_OpInfo.strReturn == L"PASSWORD")
		{
			if (ClerkInputAlpha(L"1012DF6T", config.Password, 1, 40, config.Password) 
				&& m_LXConfig->SaveConfiguration(config))
			{
				NHDEBUG(DBG_CALL, (L"Changed LX API Password\r\n"));

				ClerkNoticeToUser(T_OPERATE_OK);
			}
		}
		// F8
		else if (m_OpInfo.strReturn == L"HEARTBEAT")
		{
			CString stringValue;
			stringValue.Format(L"%d", config.HeartbeatIntervalMinutes);
			
			if (!ClerkInputNumber(L"1012DF8T", L"1012DF8S", stringValue, EDIT_NORMAL, 2, stringValue))
			{
				continue; // Try again
			}

			config.HeartbeatIntervalMinutes = Asc2Int(stringValue);

			if (m_LXConfig->SaveConfiguration(config))
			{
				NHDEBUG(DBG_CALL, (L"Changed LX API heartbeat interval\r\n"));

				ClerkNoticeToUser(T_OPERATE_OK);
			}
		}
		// Blank Key - Connection Test
		else if (m_OpInfo.strReturn == L"CONNTEST")
		{
			// Do connection test

			// Notify user of waiting
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			Delay_Msg(100); // Let graphics thread catch up

			LXConfiguration config;
			if (!m_LXConfig->GetConfiguration(config)) 
			{
				NHERROR((L"Failed to retrieve LX configuration\r\n"));
				ClerkNoticeToUser(T_OPERATE_NG);

				continue;
			}

#if (APP_LIBERTYX == 1)
			CLibertyXService service(config);
			LXPingResponse pong;
			bool result = false;

			if (config.SellBitcoinEnabled)
			{
				LXPing ping;
				CLibertyXService::PopulateDispenseInformation(ping, m_pDevCmn);
				result = service.PingDispense(ping, pong);
			}
			else 
			{
				result = service.Ping(pong);
			}

			if (result && pong.Status == L"success")
			{
				ClerkNoticeToUser(T_OPERATE_OK);
			}
			else 
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
#else 
			ClerkNoticeToUser(T_OPERATE_NG);
#endif
		}

		
		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 1010; // Back to LibertyX Main
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


