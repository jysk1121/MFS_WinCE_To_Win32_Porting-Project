#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

#ifndef NO_REMOTE_UPDATE

#define UDPATES_PER_PAGE 6 

#define USB_CONFIG_PATH		L"\\USB\\{{terminal_id}}\\update-sources.up"

//
// Screen Data
//

CString CMainFrame::ClerkRemoteUpdatesScreenDataMake(int screenNumber)
{
	switch (screenNumber)
	{
	case 1013: return ClerkRemoteUpdatesGetGeneralOptions();
	case 1014: return ClerkRemoteUpdatesGetAdHocOptions();
	}

	return CString(L"");
}

CString CMainFrame::ClerkRemoteUpdatesGetGeneralOptions()
{
	CString data;
	
	bool schedulingIsEnabled = m_remoteUpdatesAvailable && MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE) == ENABLE;
	int dow = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_DOW);
	CString dowString = m_pDevCmn->fstrSCR_GetStringFromTextID(GET_STRING_FOR_DOW(dow));
	CString configuration = m_remoteUpdatesAvailable ? m_updateConfiguration->ToHumanReadableString() : L"";

	data.Format(L"%s%s%s%s%s%s%s",
		schedulingIsEnabled ? L"Enabled" : L"Disabled", SCR_CMD_DELIMITER,
		schedulingIsEnabled ? dowString : L"Disabled", SCR_CMD_DELIMITER,
		MemGetStr(_MEM_FLD_APP_VERSIONINFO, _MEM_VAR_APP_AP_VERSION), SCR_CMD_DELIMITER,
		configuration);
		
	return data;
}

CString CMainFrame::ClerkRemoteUpdatesGetAdHocOptions()
{
	CString data;

	int pageCount = GetCountForPageIndex(m_currentRemotePage);
	for (int i = 0; i < pageCount; i++)
	{
		int index = GetCacheIndexForPageIndex(m_currentRemotePage, i);
		POSITION pos = m_updateCache->Packages.FindIndex(index);
		Package p = m_updateCache->Packages.GetAt(pos);

		data.AppendFormat(L"%s%s", p.GetDisplayName(), SCR_CMD_DELIMITER);
	}
	
	return data;
}

//
// Enable Buttons
//

CString CMainFrame::ClerkRemoteUpdatesEnableButton(int screenNumber)
{
	switch (screenNumber)
	{
	case 1013: return ClerkRemoteUpdatesGeneralButtons();
	case 1014: return ClerkRemoteUpdatesAdHocEnableButtons();
	}

	return CString(L"");
}

CString CMainFrame::ClerkRemoteUpdatesGeneralButtons()
{
	CString data;
	bool showEnableBtn			= m_remoteUpdatesAvailable && m_OpInfo.nLoginMode == LOGIN_MASTER;
	bool showAdHocBtn			= m_remoteUpdatesAvailable;
	bool showScheduleBtn		= m_remoteUpdatesAvailable && m_OpInfo.nLoginMode == LOGIN_MASTER;
	bool showLoadConfigBtn		= m_OpInfo.nLoginMode == LOGIN_MASTER;
	bool showUsbBtn				= true;
	bool showConnectionTestBtn	= m_remoteUpdatesAvailable;

	data.Append(showEnableBtn ? L"on \r" : L"off\r");
	data.Append(showAdHocBtn ? L"on \r" : L"off\r");
	data.Append(showScheduleBtn ? L"on \r" : L"off\r");
	data.Append(showLoadConfigBtn ? L"on \r" : L"off\r");
	data.Append(showUsbBtn ? L"on \r" : L"off\r");
	data.Append(showConnectionTestBtn ? L"on \r" : L"off\r");

	return data;
}

CString CMainFrame::ClerkRemoteUpdatesAdHocEnableButtons()
{
	CString data;

	int pageCount = GetCountForPageIndex(m_currentRemotePage);
	for (int i = 0; i < UDPATES_PER_PAGE; i++)
	{
		if (i < pageCount)
		{
			data.Append(L"on \r");
		}
		else
		{
			data.Append(L"off\r");
		}
	}

	data.Append(HasPreviousPage() ? L"on \r" : L"off\r");
	data.Append(HasNextPage() ? L"on \r" : L"off\r");

	return data;
}

//
// Screen controls
//


/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ClerkRemoteUpdatesOptions()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1013 PROC - General Landing for remote update options
-------------------------------------------------------------------*/
void CMainFrame::ClerkRemoteUpdatesOptions()
{
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
		if (m_OpInfo.strReturn == L"ENDISABLE")
		{
			bool schedulingIsEnabled = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE) == ENABLE;
			if (schedulingIsEnabled)
			{
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE, 0);
			}
			else
			{
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_ENABLE, ENABLE);
			}

			break;
		}
		// F2
		else if (m_OpInfo.strReturn == L"ADHOC")
		{
			NHINFO((L"Loading Ad Hoc update screen\r\n"));

			if (!m_updateCache->initialized)
			{
				ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
				Delay_Msg(100);

				// Update everything
				if (!PrimeUpdateCache())
				{
					ClerkNoticeToUser(T_OPERATE_NG);
					break;
				}

				ClerkNoticeToUser(T_OPERATE_OK);
			}

			m_OpInfo.nProcNextScrNum = 1014;
			break;
		}
		// F3
		else if (m_OpInfo.strReturn == L"SCHEDULING")
		{
			int dow = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_DOW);

			int newDow = ++dow % 7;
			MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULE_U2D_DOW, newDow);

			break;
		}
		// F4
		else if (m_OpInfo.strReturn == L"LOADCONFIG")
		{
			if (ClerkConfirm(L"1013F4S", m_OpInfo.strReturn) == TRUE)
			{
				CString sourceFile(USB_CONFIG_PATH);
				CString tid = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);

				// No spaces in paths
				tid.Trim();
				sourceFile.Replace(L"{{terminal_id}}", tid);

				if (!IsExistFile(sourceFile))
				{
					// No file to load
					ClerkNoticeToUser(T_OPERATE_NG);
					break;
				}

				ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);

				if (!CopyFile(sourceFile, WUPDATE_SOURCES, false))
				{
					// No file to load
					ClerkNoticeToUser(T_OPERATE_NG);
					NHERROR((L"Can not copy source file\r\n"));
					break;
				}

				CUpdateRepoConfigurationManager manager;
				CONFIGERR err = manager.GetConfiguration(m_updateConfiguration);
				if (err != ERR_OK)
				{
					ClerkNoticeToUser(T_OPERATE_NG);
					m_remoteUpdatesAvailable = false;
					NHERROR((L"Could not load configuration\r\n"));
					break;
				}
				else
				{
					TerminateATM();
				}

				ClerkNoticeToUser(T_OPERATE_OK);
			}
		}
		else if (m_OpInfo.strReturn == L"USBLOCAL")
		{
			if (ClerkConfirm(L"846F1S", m_OpInfo.strReturn) == TRUE)
			{
				WIN32_FIND_DATA	FileData;
				HANDLE			hSearch = INVALID_HANDLE_VALUE;		// [#2022] NH KSK 2011.02.22
				CString			strFindFolder;

				NH_SCR_CONFIG eUpdateConfig = m_pDevCmn->m_pConfig->GetScreenConfig(SCR_UPDATE);
				// SD
				strFindFolder = eUpdateConfig.strSDUpdateSymbol;
				hSearch = FindFirstFile(strFindFolder, &FileData);
				if (hSearch != INVALID_HANDLE_VALUE)
				{
					FindClose(hSearch);

					ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			
					if (ExecuteSWUpdate() == FALSE)
						m_OpInfo.m_strMsg = T_OPERATE_NG;
				}
				else
				{
					// USB
					strFindFolder = eUpdateConfig.strUSBUpdateSymbol;

					hSearch = FindFirstFile(strFindFolder, &FileData);
					if (hSearch != INVALID_HANDLE_VALUE)
					{
						FindClose(hSearch);

						ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
				
						if (ExecuteSWUpdate() == FALSE)
							m_OpInfo.m_strMsg = T_OPERATE_NG;
					}
					else
						m_OpInfo.m_strMsg = T_NO_USB_SLOT;
				}

				ClerkNoticeToUser(m_OpInfo.m_strMsg);
			}
		}
		// F6
		else if (m_OpInfo.strReturn == L"CONNTEST")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			if (!PrimeUpdateCache(/* force update */ true))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
			else
			{
				ClerkNoticeToUser(T_OPERATE_OK);
			}
		}
		
		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 846; // Back to System Control
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
 FUNCTION NAME: ClerkRemoteUpdatesAdHoc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR 1014 PROC - Version options for installing software
-------------------------------------------------------------------*/
void CMainFrame::ClerkRemoteUpdatesAdHoc()
{
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
		if (m_OpInfo.strReturn == L"OPT1")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			if (StartSWUpdateWithCachePackageIndex(GetCacheIndexForPageIndex(m_currentRemotePage, 0)))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F2
		else if (m_OpInfo.strReturn == L"OPT2")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			if (StartSWUpdateWithCachePackageIndex(GetCacheIndexForPageIndex(m_currentRemotePage, 1)))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F3
		else if (m_OpInfo.strReturn == L"OPT3")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			if (StartSWUpdateWithCachePackageIndex(GetCacheIndexForPageIndex(m_currentRemotePage, 2)))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F4
		if (m_OpInfo.strReturn == L"OPT4")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			if (StartSWUpdateWithCachePackageIndex(GetCacheIndexForPageIndex(m_currentRemotePage, 3)))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F5
		else if (m_OpInfo.strReturn == L"OPT5")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			if (StartSWUpdateWithCachePackageIndex(GetCacheIndexForPageIndex(m_currentRemotePage, 4)))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// F6
		else if (m_OpInfo.strReturn == L"OPT6")
		{
			ClerkNoticeToUser(T_PROCESS_WAIT, FALSE);
			if (StartSWUpdateWithCachePackageIndex(GetCacheIndexForPageIndex(m_currentRemotePage, 5)))
			{
				ClerkNoticeToUser(T_OPERATE_NG);
			}
		}
		// Previous
		else if (m_OpInfo.strReturn == L"PREV")
		{
			if (m_currentRemotePage > 0)
			{
				m_currentRemotePage--;
			}
			break;
		}
		// Next
		else if (m_OpInfo.strReturn == L"NEXT")
		{
			if (m_currentRemotePage < m_maxRemotePages)
			{
				m_currentRemotePage++;
			}
			break;
		}
		
		//-----------------------------------
		// COMMON PROCESS
		//-----------------------------------
		if ((m_OpInfo.strReturn == S_EXIT) || (m_OpInfo.strReturn == S_CANCEL))
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_EXIT;
			m_OpInfo.nProcNextScrNum = 1013; // Back to Other Transactions
			m_currentRemotePage = 0;
			break;
		}
		else if (m_OpInfo.strReturn == S_TIMEOVER)
		{
			m_OpInfo.nMoveProcReason = MOVE_PROC_TIMEOUT;
			m_OpInfo.nProcNextScrNum = OP_TIMEOUT_SCREEN_NUMBER;
			m_currentRemotePage = 0;
			break;
		}
	}
}

//
// Helper methods
//


bool CMainFrame::HasNextPage()
{
	return m_maxRemotePages > m_currentRemotePage + 1;
}

bool CMainFrame::HasPreviousPage()
{
	return m_currentRemotePage > 0;
}

int CMainFrame::GetCountForPageIndex(int pageIndex)
{
	int startCacheIndex = pageIndex * UDPATES_PER_PAGE;
	int count = m_updateCache->Packages.GetCount();

	int diff = count - startCacheIndex;
	return min(diff, UDPATES_PER_PAGE);
}

int CMainFrame::GetCacheIndexForPageIndex(int page, int position)
{
	int index = page * UDPATES_PER_PAGE + position;
	return index;
}

bool CMainFrame::PrimeUpdateCache(bool force)
{
	if (!force && m_updateCache->initialized)
	{
		NHINFO((L"Already cached\r\n"));
		return true;
	}

	m_updateCache->ClearCache();

	if (m_updateRepo->GetComponentContents(COMPONENT_AP, m_updateCache->Packages) != REPOERR_OK)
	{
		NHERROR((L"Error while downloading components\r\n"));
		return false;
	}

	// Update pagination information
	int updateCount = m_updateCache->Packages.GetCount();
	m_maxRemotePages = (updateCount / UDPATES_PER_PAGE);
	if (updateCount % UDPATES_PER_PAGE) 
	{
		m_maxRemotePages += 1;
	}
	m_currentRemotePage = 0;

	m_updateCache->initialized = true;
	return true;
}

bool CMainFrame::StartSWUpdateWithCachePackageIndex(int i)
{
	if (i > m_updateCache->Packages.GetCount())
	{
		NHINFO((L"Index beyond bounds of cache\r\n"));
		return false;
	}

	CFile file;
	char* swVersion = NULL;
	POSITION pos = m_updateCache->Packages.FindIndex(i);
	Package p = m_updateCache->Packages.GetAt(pos);

	if (!file.Open(REMOTE_UPDATE_INIT_FILE, CFile::modeReadWrite | CFile::modeCreate))
	{
		return false;
	}

	swVersion = new char[p.ReadableVersion.GetLength() + 1]();
	WideToMulti(swVersion, p.ReadableVersion, p.ReadableVersion.GetLength());

	file.Write(swVersion, p.ReadableVersion.GetLength());
	file.Close();

	delete [] swVersion;
	
	return TerminateATM() == TRUE;
}

#endif