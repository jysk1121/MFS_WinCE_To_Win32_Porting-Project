/////////////////////////////////////////////////////////////////////////////
// AtmModeCtrl.cpp : Implementation of the CWinAtmCtrl ActiveX Control class.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <winioctl.h>
#include "WinAtm.h"
#include ".\Dev\LoginManager.h"
#include "MainFrm.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

static	BOOL	g_bOutofService = TRUE;
static	CString	strSaveErrCode;	// [#419] [NH] 2008.9.12

/////////////////////////////////////////////////////////////////////////////
//	Atm Status
/////////////////////////////////////////////////////////////////////////////
//	Mode :	ATM_INIT			: Initial		Mode
//			ATM_ERROR			: Error			Mode
//			ATM_CLERK			: Clerk			Mode
//			ATM_READY			: Ready			Mode
//			ATM_CUSTOM			: Custom		Mode 
//			ATM_TRAN			: Transaction	Mode
//			ATM_REBOOT			: Reboot		Mode
/////////////////////////////////////////////////////////////////////////////
// Atm Mode Ctrl
int CMainFrame::AtmModeCtrl()
{
	int nConfigOption = 0;	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
	int	i = 0;				// [#2253] NH KSK 2014.02.21
	CString strKCV, strTemp;			// [#2253] NH KSK 2014.02.21

	Mode_CSTStatus = m_pDevCmn->fnCDU_GetAllCSTStatus();	// [#194] KSK 2008.05.16

	// [#2011] NH KJW 2011.01.20
	BOOL bWelcomeProcOK = FALSE;
	// end of [#2011]

	switch (m_pDevCmn->AtmStatus)
	{
//////////////////////////////////////////////////////////////////////////
		case ATM_INIT:										// Initial Mode
//////////////////////////////////////////////////////////////////////////
// Mode Procedure
			NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_INIT\n"));
			// Mode variant init
			Main_SetAtmStatus(ATM_INIT);		// [#93] KSK 2008.04.08
			bSendErrorAfterRecover = FALSE;		// [#8] KSK 2008.03.19

			// [#2113] NH KSK 2012.01.03 변수초기화
			m_RMSSendTime = COleDateTime::GetCurrentTime();
			m_bRMSStatusInitSend = FALSE;
			// end of [#2113]

			SetNextRebootTime();				// [#2558] NH Justin 2018.06.14 Add Scheduled Reboot Option
			m_bLastRMSSendResult = TRUE;		// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed

			// [#2205] US KSK 2013.06.28
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
				SetHaloLedControl(HALOLED_COLOR_CYAN, OFF_HALOLED_MODE, OFF_MCULED);
			// end of [#2205]

			// [v01.03.03] AU AIREAT 2009.05.08 : ATM Init으로 이동.
			m_pAdaCtrl->fnExp_SetVolume(MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL));

			m_pDevCmn->fnAPL_ClearError();

			// [#2283] NH Justin Support Customized Hot Key
			{
				CString strStoredLocalHotKey = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HOT_KEY_STRING);
				strStoredLocalHotKey.TrimLeft();
				strStoredLocalHotKey.TrimRight();
				strStoredLocalHotKey.MakeUpper();
				NHDEBUG(1, (_T("HOTKEY CHECK - Stored Local Hot Key = [%s]\n"), strStoredLocalHotKey  ) );
				if( m_pDevCmn->IsValidHotKey(strStoredLocalHotKey) != TRUE)
				{
					NHDEBUG(1, (_T("HOTKEY CHECK - stored hot key is not valid => Restore Factory default\n") ) );
					m_pDevCmn->m_strStoredHotKey = DEFAULT_OPMODE_HOTKEY;
					MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HOT_KEY_STRING, m_pDevCmn->m_strStoredHotKey);
				}
				else
					m_pDevCmn->m_strStoredHotKey = strStoredLocalHotKey;
			}
			// End of [#2283]

			// [#2056] NH KSK 2011.05.04 Master Password Clear 기능 추가
			if (m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_OPENED)
			{
				CTime CurTime = CTime::GetCurrentTime();

				// PIN ENABLE
				m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, ENABLE, FALSE, PIN_MENU_MODE);
				NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_INIT PIN ENABLE\n"));

				while(CurTime + 5 >= CTime::GetCurrentTime())
				{
					// PIN SP / Door Sensor 활성화 후 Special Key Check (5초동안)
					if ( ((m_pDevCmn->m_strHotKey.Find(RESET_MP_SPECIAL_KEY) != -1) || m_pDevCmn->m_bExecuteResetMP)	// [#RWC6-94] NH Kook 2020.01.31 Easing MPW Clear
						&& (m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_OPENED) )	// Special Key Value 사양 협의 필요
					{
						// [#2304] NHA Justin 2014.11.07 Clearing Master Password is available only when NVRAM is broken (US, CA, MX)
						#if (US_VERSION || CA_VERSION || MX_VERSION)
						if (Check_BrokenNVRAM() == TRUE)
						#endif
						// End of [#2304]
						{
							NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_INIT DETECT SPECIAL KEY\n"));
							m_pDevCmn->m_bExecuteResetMP = false;	// [#RWC6-94] NH Kook 2020.01.31 Easing MPW Clear

							// Screen Display
							m_pDevCmn->fnSCR_DisplayScreen(200, K_NO_WAIT, PIN_MENU_MODE);

							CString GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString();

							m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
							m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);

							Delay_Msg(1000);	// 1초 후에 수행한다.

							if (GetKeyStr == L"YES")
							{
								NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_INIT RESET MASTER PASSWORD\n"));

								// Clear EPP
								if (m_pDevCmn->fnPIN_ClearAllKeys() == R_NORMAL)
								{
									int i;
									// [#2253] NH KSK 2014.02.21 Reset Master Key시에는 NVRAM Master Key Clear는 공통으로 적용
									// 512K인 경우에 PIN Clear 후 해당 NVRAM을 Reset한다.
									if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
									{
										// [#2230] AU KMK 2013.11.12 변경 전 KEY MODE / KEY INDEX값 저장
										int nCurrKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);
										int nCurrKeyIndex = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX);
										// end of [#2230]

										NHDEBUG(1, (_T("***ModeCtrl***CMainFrame EP VERSION 512K DETECTED\n")));
										#if (AU_VERSION)	// [#2253] NH KSK 2014.02.21
											MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE, KEYMODE_TDES_TMACING);	// 호주는 T-DES / T-MAC
										#else
											MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE, KEYMODE_NON_UNIQ_TDES);			// 그 외 국가는 T-DES
										#endif				// end of [#2253]
											MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX, 0);			// 임의로 KEYINDEX값을 0으로 SET함

										MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECK, L"");			// [#2314]
										
										// [#2253] NH KSK 2014.02.21 모든 Key KCV값을 Clear하도록 로직 보완
										for(i=0; i<= MAX_PIN_KEYINDEX; i++)
											MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i, L"");
										// end of [#2253]

										// [#2230] AU KMK 2013.11.12 TDES/TMAC으로 기본값이 변경될 경우 저널에 기록함
										#if SUPPORT_CHANGE_PARAMETER_JNL
											if ( nCurrKeyMode != KEYMODE_TDES_TMACING )
												m_pDevCmn->m_JNLMgr.SaveChangeParameterJnl(m_OpInfo.nLoginMode, L"KEY MODE", Int2Asc(nCurrKeyMode), Int2Asc(KEYMODE_TDES_TMACING));
											if ( nCurrKeyIndex != 0 )		// Key Index 생략해도 되는지 확인 필요
												m_pDevCmn->m_JNLMgr.SaveChangeParameterJnl(m_OpInfo.nLoginMode, L"KEY INDEX", Int2Asc(nCurrKeyIndex), Int2Asc(0));
										#endif
										// end of [#2230]
									}
									// [#2314] NH Justin 2014.12.09 Clear Checksum when EPP is cleared.
									else
									{
										MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE, KEYMODE_NON_UNIQ_TDES);
										MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX, 0);
										MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECK, L"");
										for(i=0; i<= MAX_PIN_KEYINDEX; i++)
											MemSetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i, L"");
									}
									// End of [#2314]

									// [#RWC6-107] US William Hash passwords
									CLoginManager manager;
									manager.ResetMasterPassword(m_pDevCmn);
									// End of [#RWC6-107]

									// [#2283] NH Justin Support Customized Hot Key, Reset Hot Key Sequence As well.
									MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HOT_KEY_STRING, DEFAULT_OPMODE_HOTKEY);
								}
							}

							TerminateATM(L"ATM will be restarted");
							return 0;	// AP 종료
						}
					}

					Delay_Msg(100);
				}
				m_pDevCmn->fnAPL_DeviceEnDisable(DEV_PIN, DISABLE);
			}

			if (Check_BrokenNVRAM() == TRUE)
			{
				m_pDevCmn->fnAPL_StackError(_T("FFFFFFF"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003043), DEV_SYS);	// [#257] [MX] KSK 2008.6.5	// [#419] [NH] KSK 2008.9.18
				NVDump('F', 'A', "11", L"", L"2:FFFFFFF" );
				Main_SetAtmStatus(ATM_ERROR);	// [#93] KSK 2008.04.08
				break;
			}
			// end of [#23]

			// [#RWC6-322] US Kook 2021.10.27
			if (Check_EPPInitState() == FALSE)
			{
				Main_SetAtmStatus(ATM_ERROR);
				break;
			}
			// end of [#RWC6-322]

			SynchronizeKCV();	// [#2580] AU Kook 2019.08.06 Refactoring duplicated codes.

			// [#2499] US Justin 2017.08.21 Disable Dual Host DCC for non-512K EPP or Dial Up Mode
			if( (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) < 0) || 
				(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE) == NETWORK_DIALUP) )
			{
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE) == DYNAMICFLOW_DUALHOST)	
				{
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_ENABLE, DYNAMICFLOW_DISABLE);			// Disable STD1 DF

					if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC) != 0 )
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_DCC, 0);						// Disable STD1 DF DCC

					if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE) != 0)	
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_SERVICE_PINCHANGE, 0);				// DIsable PIN Change

					if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION) != DYNAMICFLOW_1STCALL_OFF)
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DF_1STCALL_OPTION, DYNAMICFLOW_1STCALL_OFF);// DF 1st Call OFF
				}

				if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE) != 0)
					MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);						// Disable Dual Host DCC
			}
			// End of [#2499]
		
			m_pTranCmn->LoadDCCDomesticBin();		// [#2317] US Justin 2015.01.05 Load Domestic BIN

			//////////////////////////////////////////////////////////////////////////
			// Card Transaction Menu Initialization to IC
			if(m_pDevCmn->fnMCU_IsEmvEnable())		// [#397] [NH] psc 2008.08.01 EMV(IC) 거래일때
			{
				if(MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_INTERCHANGE_PRO_TLV).GetLength() > 0 ||
					MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APPLICATION_ID_TLV).GetLength() > 0)	// EMV or MS reversal 조건을 판단하기위해.
					m_pDevCmn->nKindOfMedia = MEDIA_IC;		// IC Init
				else
					m_pDevCmn->nKindOfMedia = MEDIA_MS;		// MS Init
			}
			// end of [#68]

			m_pDevCmn->Create_EMV_Termdata_From_POOL();		// [#2342]
			m_pDevCmn->m_JNLMgr.Save(POWER_ON_SYSTEM);		// [#3] NH AIREAT 2008.3.10

			// AP Version 정보 JNL에 남김.
			{
				CString strAPVersion;
				CString	strOSversion, strTempVer;

				strTempVer = RegGetStr(L"SOFTWARE\\ATM", L"OSVersion");
				strOSversion = strTempVer.Mid(1,2) + strTempVer.Mid(4,2) + strTempVer.Mid(7,2);
				strAPVersion.Format(L"CE6 S(%d)A(%d%02d%02d%02d)R(0x%X)O(%s)",
														(int)m_pDevCmn->m_pConfig->GetOSVersion(),
														AP_DETAIL_VERSION, 
														RMS_VERSION, 
														strOSversion);
				m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, strAPVersion);
			}

			// [#423] [NH] KSK 2008.9.18
			// Power Off 대책 LOGIC Mode_ExecPowerOffProc() 함수로 위치 이동
			m_pTranCmn->BIZ_PowerOffReversal();		// [iTM] KSK 2010.01.26

			m_pDevCmn->DeviceStatus = m_pDevCmn->fnAPL_GetDefineDevice(DEV_MAIN);
			
			// [#2220] AU KMK 2014.01.23 현재 무슨 장치를 Initialize 하는지 표시함
			// PIN은 앞단에서 이미 수행함 (fnAPL_LoadDevice())
#if (AU_VERSION)
			// DEV_MCU
			m_pDevCmn->fnSCR_DisplayPrevSet(1);	// 이전 화면에서 Reversal을 할 경우 화면이 009 화면이므로 화면 갱신 필요 Bug Fix
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringByTextID( _T("INITIALIZING MCU") ));
//			m_pDevCmn->fnSCR_DisplayUpdate(1);
			m_pDevCmn->fnSCR_DisplayScreen(1);
			m_pDevCmn->fnAPL_ResetDevice(DEV_MCU);

			// DEV_SPR
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringByTextID( _T("INITIALIZING SPR") ));
			m_pDevCmn->fnSCR_DisplayUpdate(1);
			m_pDevCmn->fnAPL_ResetDevice(DEV_SPR);

			// DEV_JPR (WinCE는 해당없음)

			// DEV_CDU
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringByTextID( _T("INITIALIZING CDU") ));
			m_pDevCmn->fnSCR_DisplayUpdate(1);
			m_pDevCmn->fnAPL_ResetDevice(DEV_CDU);

			// 2014.02.28 초기화 완료 후 표시문구 제거
			m_pDevCmn->fnSCR_DisplayString(3, L"");
			m_pDevCmn->fnSCR_DisplayUpdate(1);
#else
			// [#2325] NH KSK 2015.01.21 RFID 지원
//			m_pDevCmn->fnAPL_ResetDevice(DEV_MAIN);	// [#507] [NH] KSK 2009.2.23	(DEV_MCU | DEV_SPR | DEV_JPR | DEV_CDU)
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
				m_pDevCmn->fnAPL_ResetDevice(DEV_MAIN|DEV_RFID);	// [#507] [NH] KSK 2009.2.23	(DEV_MCU | DEV_SPR | DEV_JPR | DEV_CDU)
			else
				m_pDevCmn->fnAPL_ResetDevice(DEV_MAIN);	// [#507] [NH] KSK 2009.2.23	(DEV_MCU | DEV_SPR | DEV_JPR | DEV_CDU)
			// end of [#2325]
#endif
			// end of [#2220]

			// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
			if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) == ENABLE)
			{
				m_pDevCmn->fnAPL_ResetDevice(DEV_BCR | DEV_BNA);
			}
			// end of [#GLDV-3005]

			// Device Initialize 이후 최초 Device Status update를 한다
			//m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN);
			//m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN|DEV_RFID);				// [#2325] NH KSK 2015.01.30
			m_pDevCmn->fBAPL_IsDeviceStatusChanged(m_pDevCmn->fnCMN_GetActiveDevice());	// [#GLDV-3005] US Kook 2022.01.18 Side Car

#if (MX_VERSION)	// [#2137] MX KSK 2012.07.23 Multi Currency 보완 처리
			if (m_pDevCmn->m_strCurrencyID == CURRENCY_TYPE)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE, 0);
			else if (m_pDevCmn->m_strCurrencyID == MULTI_CURRENCY_TYPE)
				MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MULTI_CURRENCY_ENABLE, ENABLE);
			// 그 이외의 값은 NVRAM 설정값 유지
#endif				// end of [#2137]

#if (AU_VERSION)
			// 512K EPP는 SP에서 KEY MODE 값을 세팅해주지 않으므로 AP에서 보완처리
			if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) >= 0)
			{
				int nCurrKeyMode = MemGetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE);

				// Key Mode 값이 없을 때
				if (nCurrKeyMode == 0)
				{
					// 호주인 경우 TDES/TMAC으로 설정 후 저널에 기록					
					// Key Mode
					MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYMODE, KEYMODE_TDES_TMACING);
					// Key Index
					MemSetInt(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYINDEX, 0);
					// Checksum은 위에서 처리 함 ([#2253])

					m_pDevCmn->m_JNLMgr.SaveChangeParameterJnl(m_OpInfo.nLoginMode, L"KEY MODE", Int2Asc(0), Int2Asc(KEYMODE_TDES_TMACING));
				}
			}
#endif
			// end of [#2231]

			// 2007.11.23 V01.02.19
			// SP에서 주는 정보를 가지고 BACKUP LOG를 할 것인지를 결정해야함
			if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGCDULOGBACKUP) == 1)
			{
				MemSetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGCDULOGBACKUP, 0);
				SaveLogData();				// [#31] NH AIREAT 2008.03.18  BackupLogData -> SaveLogData 변경.
			}
			// [#68] HWANG 2008.04.02 EMV Level2
			// Media 종류 별로 Reversal Transaction 수행 후 Media 종류를 MS/IC setting값으로 변경
			if(m_pDevCmn->fnMCU_IsEmvEnable())		// [#397] [NH] psc 2008.08.01 EMV(IC) 거래일때
				m_pDevCmn->nKindOfMedia = MEDIA_IC;
			else
				m_pDevCmn->nKindOfMedia = MEDIA_MS;
			// end of [#68]

			// 내부 변수 CSTCnt 문제로 인해 ATM_INIT에서만 Check Device를 먼저 call한다
			m_pDevCmn->fnAPL_CheckDevice();
			m_pDevCmn->fnAPL_SetSensorInfo(TRUE);
			m_pDevCmn->fnAPL_GetAvailTrans();
			m_pDevCmn->fnAPL_CheckMaterial();

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	#if DEV_AUTO_OFF_SPR
			if (m_pDevCmn->fnSPR_GetDeviceStatus() != NORMAL)
				m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, L"Receipt Print is not available");			// [#408] AU AIREAT 2008.08.28
	#endif
#endif

			if (!m_pDevCmn->fnAPL_CheckError())
				Main_SetAtmStatus(ATM_ERROR);	// [#93] KSK 2008.04.08
			// [#423] [NH] KSK 2008.9.18
			// Mode_CheckErrorElements 함수로 위치 이동
			else
			if (Mode_CheckErrorElements() == TRUE)
				Main_SetAtmStatus(ATM_ERROR);
			// end of [#423]
			else												// Next Mode
			{
				Main_SetAtmStatus(ATM_CLERK);	// [#93] KSK 2008.04.08
			}
			// [#8] KSK 2008.03.19
			m_pDevCmn->fnAPL_SetHealthCheckTimer();
			// end of [#8]

			m_pDevCmn->m_nPrevLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();		// [#2339] NH Justin 2015.04.07 Prevent Font corruption on Out of Service Screen
			break;
		
//////////////////////////////////////////////////////////////////////////
		case ATM_ERROR: 										// Error Mode
//////////////////////////////////////////////////////////////////////////
// Mode Procedure
			// [#215] KSK 2008.5.20
			// ATM ERROR로 오는 CASE별 Journal 및 Error 처리
			// 장애코드가 다를 경우에 타도록 한다.
			if ((m_pDevCmn->AtmStatus != m_pDevCmn->AtmStatusSave) && (m_pDevCmn->fstrAPL_GetErrorCode() != strSaveErrCode))	// [#419] [NH] 2008.9.12
			{
				#if (APP_EVENT_NOTICE)
				m_pTranCmn->BIZ_EVENT_SendHost(_EVENTID_OUT_OF_SERVICE);		// [#2313] US Justin 2014.11.20 Notice Event
				#endif

				NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_ERROR\n"));
				g_bOutofService = TRUE;

				m_pDevCmn->fnAPL_UnUsed(ATM_ERROR);					// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
				m_pDevCmn->m_JNLMgr.Save(CHANGE_ERROR_MODE);		// [#135] KSK 2008.04.18
				m_pDevCmn->fnAPL_MakeMobileAppQRCode(MOBILEAPP_QR_TYPE_ERRORCODE);		// [#2382] US Justin 2015.12.14 Make MobileApp QR Code
				strSaveErrCode = m_pDevCmn->fstrAPL_GetErrorCode();	// [#419] [NH] 2008.9.12

				// [#2205] US KSK 2013.06.28 UnUsed에서 Default로 모두 OFF 시킨 후 OP에 설정된 OUT OF SERVICE MODE로 SET 한다.
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
					SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_OUTOFSERVICE), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_OUTOFSERVICE), SKIP_MCULED);
				// end of [#2205]

				// MODEM의 TAPI에서 RESOURCE UNAVAILABLE 장애시에는 복구가 불가능 하므로 REBOOT 한다
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "D0307")	// [#419] [NH] 2008.9.12
				{
					TerminateATM(L"MODEM DEVICE RECOVERY");
					return TRUE;
				}

				SaveLogData();							// [#31] NH AIREAT 2008.03.18  BackupLogData -> SaveLogData 변경.

				// [#2397] NH KSK 2016.02.17 SPR 장애 중 Fatal Error 등 MWI 장애코드가 Set할 경우 재부팅하도록 로직 추가
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "97922")
				{
					TerminateATM(L"PRINTER DEVICE RECOVERY");
					return TRUE;
				}
				// end of [#2397]

				// [###] KSK 2014.05.29 PIN이 장애가 발생하였으나 Device Status가 정상으로 올 경우 DeviceStatus 제거 PCI2.0 / PCI3.0 대응
				if (m_pDevCmn->fnPIN_GetDeviceStatus() == NORMAL)
				{
					NHDEBUG(DBG_INFO, (L"___fnPIN_GetDeviceStatus() : NORMAL\n"));
					m_pDevCmn->DeviceStatus &= ~DEV_PIN;
				}
				else
				{
					NHDEBUG(DBG_INFO, (L"___fnPIN_GetDeviceStatus() : NOT NORMAL\n"));
				}
				// end of [###]
				NHDEBUG(DBG_INFO, (L"___fstrAPL_GetErrorCode() : %s\n", m_pDevCmn->fstrAPL_GetErrorCode()));

				SynchronizeKCV();		// [#2580] AU Kook 2019.08.06

				// AP SYSTEM ERROR(Fxxxxx)는 RESET하지 않는다
				// RESET을 하지 않는 조건
				// 1. 971A6/971A7 -> CDU SETTING or CDU NVRAM ERROR
				// 3. SYSTEM ERROR (F0001포함)
				// 4. 방출중 POWER OFF (97455)
				// 6. 과방출일 경우 C004F00(SP / EP가 모두 ERROR가 발생하지 않아 AP가 장애 SET한 경우)
				// 7. AP에서 SET한 ERROR CODE (20001, 20002, 20004, 20004, 20005)
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "971A6" || m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "971A7")	// [#419] [NH] 2008.9.12
					;
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(1) == "F")
					;
				else 
				if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGENQSEND) == 1)
					;
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "C004F")	// [#419] [NH] 2008.9.12
					;
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "20001" ||
					m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "20002" ||
					m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "20003" ||
					m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "20004" ||
					m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "20005")
					;
				else		// [#2079] NH KSK 2011.07.15 Anti Skimming Detect시 Auto Recovery 안하도록 추가
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"31095" ||
					m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"31096")
					;		// end of [#2079]
				// [#2580] AU Kook 2019.07.12 EPP Tamper Switch Removal
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"E21FF"																	// PCI 3.0
					|| m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"EX1FF" || m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"9EA61")		// PCI 2.0
				{
					// do not attempt auto-recovery for the case that tamper switch is removed, or it is initialization state.
					NHDEBUG(DBG_INFO, (L"___SKIPPING AUTO RECOVERY FOR ECODE '%s' - EPP Tamper Switch Removed or init state.\n", m_pDevCmn->fstrAPL_GetErrorCode()));
				}
				// end of [#2580]
				else
				{
					// [#114] KSK 2008.04.15 CDU 축퇴처리
					if (m_pDevCmn->fnAPL_GetAvailErrorDevice(m_pDevCmn->DeviceStatus) & DEV_CDU)
					{
						// [#386] [NH] KSK 2008.7.28
						// [#419] [NH] 2008.9.12
						if ((m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40047")	||	// CST1 - EP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4005B")	||	// CST2 - EP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4009F")	||	// CST3 - EP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4007C")	||	// CST4 - EP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "97447")	||	// CST1 - SP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9745B")	||	// CST2 - SP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9749F")	||	// CST3 - SP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9747C")	||	// CST4 - SP
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(3) == "4DN")	||
							(m_pDevCmn->fstrAPL_GetErrorCode().Left(7) == "8216091")||
							MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_CDU_SUPPORTAR)	// [#RWC6-82] NH Kook 2021.12.27 Support CDU Auto Recovery (Jam Clear)
							)
						{
							// CDU MISSFEED 장애 시 RECOVERY LOGIC 수정
							// 축퇴 MODE가 아닌 경우 CST가 모두 불가능일 경우에는 ERROR 상태로 남겨놓는다
							if (Mode_CSTStatus == CST_NORMAL || 
								m_pDevCmn->fstrAPL_GetErrorCode().Left(3) == "4DN"		||		// CDU LINE OFF
								m_pDevCmn->fstrAPL_GetErrorCode().Left(7) == "8216091"	||		// Cash Exist
								MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_CDU_SUPPORTAR))		// [#RWC6-82] NH Kook 2021.12.27 Support CDU Auto Recovery (Jam Clear)
							{
								// [#106] KSK 2008.04.14
								// Device Auto Recovery 화면 추가
								if (m_pDevCmn->fnAPL_CheckHostOpen())
									m_pDevCmn->fnSCR_ShowScreen(1);			// Added Device Initializing Screen
								// end of [#106]
								m_strNVLog.Format(L"5:ERR(%x,%d)", m_pDevCmn->DeviceStatus, m_pDevCmn->fnAPL_GetDownErrorDevice(m_pDevCmn->DeviceStatus));	// [#128] KSK 2008.04.16
								NVDump('F', 'A', "01", L"",  m_strNVLog);
								m_pDevCmn->fnAPL_ResetDevice(m_pDevCmn->fnAPL_GetDownErrorDevice(m_pDevCmn->DeviceStatus) | m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_MCU), INIT_BY_RESET);	// [#128] KSK 2008.04.16	// [#507] [NH] KSK 2009.2.23
							}
						}
						// end of [#386]

						// 2007.11.23 V01.02.19
						// SP에서 주는 정보를 가지고 BACKUP LOG를 할 것인지를 결정해야함
						if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGCDULOGBACKUP) == 1)
						{
							MemSetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGCDULOGBACKUP, 0);
							SaveLogData();				// [#31] NH AIREAT 2008.03.18  BackupLogData -> SaveLogData 변경.
						}
					}
					else
					{
						// [#106] KSK 2008.04.14
						// Device Auto Recovery 화면 추가
						if ((m_pDevCmn->fnAPL_CheckHostOpen()) && (m_pDevCmn->fnAPL_GetDownErrorDevice(m_pDevCmn->DeviceStatus)))
							m_pDevCmn->fnSCR_ShowScreen(1);			// Added Device Initializing Screen
						// end of [#106]

						m_strNVLog.Format(L"5:ERR(%x,%d)", m_pDevCmn->DeviceStatus, m_pDevCmn->fnAPL_GetDownErrorDevice(m_pDevCmn->DeviceStatus));	// [#128] KSK 2008.04.16
						NVDump('F', 'A', "01", L"",  m_strNVLog);
						NHDEBUG(DBG_INFO, (L"%s\n", m_strNVLog));

						// [#GLDV-2800] NH Kook 2020.04.20 Update KCV when EPP gets normal state.
						// Since "DEV_PIN in DeviceStatus" could be changed to ONLINE occasionally when EPP status is checked in "ResetDevice",
						// set a flag instead of using DeviceStatus to make it sure "there was a problem in EPP before reset".
						bool bEppAbnormal = false;
						if (m_pDevCmn->DeviceStatus & DEV_PIN)
							bEppAbnormal = true;
						// end of [#GLDV-2800]

						m_pDevCmn->fnAPL_ResetDevice(m_pDevCmn->fnAPL_GetDownErrorDevice(m_pDevCmn->DeviceStatus) | m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_MCU));	// [#128] KSK 2008.04.16
						// end of [#135]

						// [#GLDV-2800] NH Kook 2020.04.20 Update KCV when EPP gets normal state.
						if (bEppAbnormal && m_pDevCmn->fnPIN_GetDeviceStatus() == NORMAL)
						{
							// "DEV_PIN in DeviceStatus" is not updated within "ResetDevice()" as expected, so do it manually.
							m_pDevCmn->DeviceStatus &= ~DEV_PIN;
							SynchronizeKCV();
						}
						// end of [#GLDV-2800]
					}
					// end of [#114]

					Check_DoorSensor();
					Check_CSTSensor();

#if ! DEV_AUTO_OFF_SPR
					// [#254] NZ AIREAT 2008.06.03 : SPR-AUTO OFF
					m_pDevCmn->fnAPL_SetSensorInfo();
					m_pDevCmn->fnAPL_CheckDevice();
					m_pDevCmn->fnAPL_CheckMaterial();
					// end of [#254]
#endif

					// [#215] KSK 2008.5.20 
					// 1. 이전에 save한 errorcode가 같으면 journal을 하지 않는다.
					// 2. Device Reset후 ErrorCode가 변경되면 한번 더 Journal을 한다.
					if ((m_pDevCmn->fstrAPL_GetErrorCode() != strSaveErrCode) && (!m_pDevCmn->fnAPL_CheckError()))	// [#419] [NH] 2008.9.12
					{
						m_pDevCmn->m_JNLMgr.Save(CHANGE_ERROR_MODE);		// [#135] KSK 2008.04.18
						m_pDevCmn->fnAPL_MakeMobileAppQRCode(MOBILEAPP_QR_TYPE_ERRORCODE);		// [#2382] US Justin 2015.12.14 Make MobileApp QR Code
						strSaveErrCode = m_pDevCmn->fstrAPL_GetErrorCode();	// [#419] [NH] 2008.9.12

					}
					// end of [#215]
				}
			}

			if (Main_NMS_Proc() != T_OK)	break;		// [#113] KSK 2008.04.15 ERRORMODE시 RMS / HEALTH CHECK 송신 위치 변경

			m_pDevCmn->AtmStatusSave = ATM_ERROR;

			// [#471] [NH] KSK 2008.12.23
			// ERROR모드에서 ADA Jack 꼽을 경우 AP Holding되는 현상 수정
			if(m_pDevCmn->fnSNS_GetEnhancedAudio())
			{
				if(m_pAdaCtrl->IsPlaying() == FALSE)
				{
					Delay_Msg(1000);
					if (m_pAdaCtrl->IsPlaying() == FALSE)
					{
						// [#2375] US Justin 2015.11.03
						//m_pAdaCtrl->fnExp_ResetScrWaveFile();
						m_pAdaCtrl->fnExp_StopAndResetWaveFile();
						#if(US_VERSION)
							m_pAdaCtrl->fnExp_AddWaveFile(3,L"003.wav");
						#endif
						// End of [#2375]
						m_pAdaCtrl->fnExp_PlayScreenWave(3);
					}
				}
			}
			// end of [#471]

// Before Procedure
			m_pDevCmn->fnAPL_SetSensorInfo();
			m_pDevCmn->fnAPL_CheckDevice();
			m_pDevCmn->fnAPL_CheckMaterial();

			if (m_pDevCmn->bAntiSkimmingFlag == TRUE)	// [#2085] NH KSK 2011.07.22
			{
				if (CTime::GetCurrentTime() >= m_pDevCmn->m_AntiSkiimingRecoveryTime)
				{

					m_pDevCmn->fnSCR_ShowScreen(1);			// Added Device Initializing Screen

					m_pDevCmn->bAntiSkimmingFlag = FALSE;

					// Anti Skimming Auto Recovery

// [#2220] AU KMK 2014.02.07 장치 초기화 문구 추가
#if (AU_VERSION)
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringByTextID( _T("INITIALIZING MCU") ));
					m_pDevCmn->fnSCR_DisplayUpdate(1);
#endif
// end of [#2220]

					m_pDevCmn->fnAPL_ResetDevice(DEV_MCU);

					// 자동 복구 후 무조건 Mode를 변경하여 Error Check를 재 시도 하도록 한다.
					Main_SetAtmStatus(ATM_READY);
				}
			}											// end of [#2085]
			else
			if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_DOWNLOAD_FAIL || m_pDevCmn->nPINErrorFlag == PIN_MAC_BLANK_ERROR)
			{
				// [#2134] NH KSK 2012.05.07 Working Key Download Fail시에 PIN SP 재기동 추가
				m_pDevCmn->nPINErrorFlag = WORKINGKEY_INIT;
				m_pDevCmn->fnSCR_ShowScreen(1);			// Added Device Initializing Screen

				// PIN은 현재 Reset Device에서 미지원하여 직접 Call하도록 함
				if (m_pDevCmn->fstrAPL_GetErrorCode() == L"9799904")
					m_pDevCmn->fnAPL_ClearError();	// 장애코드 Clear후에

				// [#2220] AU KMK 2014.02.07 장치 초기화 문구 추가
#if (AU_VERSION)
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringByTextID( _T("INITIALIZING PIN") ));
				m_pDevCmn->fnSCR_DisplayUpdate(1);
#endif
				// end of [#2220]

				m_pDevCmn->fnPIN_Initialize();

				// 자동 복구 후 무조건 Mode를 변경하여 Error Check를 재 시도 하도록 한다.
				Main_SetAtmStatus(ATM_READY);
				SaveLogData();	// 로그 저장
				// end of [#2134]
			}
			else
			if (!m_pDevCmn->fnAPL_CheckError())
			{
				// [#202] KSK 2008.5.16
				// CST 자동 복구를 세분화 한다.
				// Sensor 복구 여부 Check
				// [#419] [NH] KSK 2008.9.16
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"20001")
				{
					if (Mode_CSTStatus != CST_SET_NG)
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);
					}
				}
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"20002")
				{
					if ( (Mode_CSTStatus != CST_NEAR) || MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INSERVICE_WHENCSTSSHORT) == ENABLE )
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);
					}
				}
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"F0001")
				{
					if ( (Mode_CSTStatus != CST_EMPTY) || MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INSERVICE_WHENCSTSEMPTY) == ENABLE )
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);
					}

					if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)	// [#483] [NH] KSK 2009.2.6
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);	// [#93] KSK 2008.04.08
					}
				}	// end of [#202]
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"20004")
				{
					if (m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_CLOSED)	// [#483] [NH] KSK 2009.2.6
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);	// [#93] KSK 2008.04.08
					}
				}
				// [#GLDV-3005] US Kook 2022.03.23 Side Car Doors
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"20006")
				{
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SIDECAR_SUPPORT) != ENABLE || !IsSideCarDoorOpen())
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);
					}
				}
				// end of [#GLDV-3005]
				else
				if ((m_pDevCmn->fstrAPL_GetErrorCode().Left(5) != L"F0001" && m_pDevCmn->fstrAPL_GetErrorCode().Left(1) == L"F") ||
					m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"20005")
				{
					Check_NVRamData();
				}
				else	// [#204] KSK 2008.5.16
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"20003")
				{
					if (m_pDevCmn->RejectCSTStatus != CST_FULL)
					{
						m_pDevCmn->fnAPL_ClearError();
					}
				}		// end of [#204]
				// [#254] NZ AIREAT 2008.06.03 : SPR-AUTO OFF
#if ! DEV_AUTO_OFF_SPR
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "20012" ||		
					m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "20013")
				{
					if (m_pDevCmn->fnSPR_GetDeviceStatus() == NORMAL)
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);
					}
				}
#endif
				// end of [#254]
				// [#GLDV-2800] NH Kook 2020.04.20 Clear 9799904 error when EPP is normal state.
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode() == L"9799904")
				{
					if (m_pDevCmn->fnPIN_GetDeviceStatus() == NORMAL)
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);
					}
				}
				// end of [#GLDV-2800]
				else
				if (m_pDevCmn->fstrAPL_GetErrorCode() == L"9770195")
				{
					if (m_pDevCmn->fnBNA_GetDeviceStatus() == NORMAL)
					{
						m_pDevCmn->fnAPL_ClearError();
						Main_SetAtmStatus(ATM_READY);
					}
				}
			}
			// [#423] [NH] KSK 2008.9.18
			else
			if (Mode_CheckErrorElements() == TRUE)
			{
				Main_SetAtmStatus(ATM_ERROR);
			}
			// end of [#423]
			// [#2277] US Justin 2014.06.10 Support remote status change(US, CA, MX)
			else
			if (Mode_CheckAdditionalATMStatus() == TRUE)
			{
				Main_SetAtmStatus(ATM_ERROR);
			}
			// end of [#2277]
			else												// Next Mode
			{
				Main_SetAtmStatus(ATM_READY);	// [#93] KSK 2008.04.08
			}

			m_pDevCmn->fnAPL_DisplayPrintError();

			if (!ClerkProc())
				Main_SetAtmStatus(ATM_REBOOT);	// [#93] KSK 2008.04.08

			break;

//////////////////////////////////////////////////////////////////////////
		case ATM_CLERK: 										// Clerk Mode
//////////////////////////////////////////////////////////////////////////
		{
			if (Main_NMS_Proc() != T_OK)	break;		// [#113] AIREAT 2008.05.07 for rms connection close

// Mode Procedure
			int prevAtmStatusSave = m_pDevCmn->AtmStatusSave;

			if (m_pDevCmn->AtmStatus != m_pDevCmn->AtmStatusSave)
			{
				NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_CLERK\n"));

				// [#2205] US KSK 2013.06.28
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
					SetHaloLedControl(HALOLED_COLOR_CYAN, OFF_HALOLED_MODE, OFF_MCULED);
				// End of [#2205]

				if (m_pDevCmn->fnSNS_GetEnhancedAudio())
				{
					if (m_pAdaCtrl->IsPlaying() == TRUE)		// ADA에서 Clerk Mode 전환시 안내방송을 OFF하도록 수정
						m_pAdaCtrl->fnExp_StopPlay();
				}
			}

			m_pDevCmn->AtmStatusSave = ATM_CLERK;

			if (!ClerkProc())
			{
				Main_SetAtmStatus(ATM_REBOOT);	// [#93] KSK 2008.04.08
			}
			else
			{
				Main_SetAtmStatus(ATM_READY);	// [#93] KSK 2008.04.08

#if (APP_DIGITALMINT)
				if (prevAtmStatusSave != ATM_INIT)
				{
					m_pTranCmn->F_NH_DigitalMintReportDeviceEvent(DMEC_MaintenanceModeExit);
				}
#endif
			}

			m_pDevCmn->m_nPrevLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();		// [#2339] NH Justin 2015.04.07 Prevent Font corruption on Out of Service Screen
			break;
		}
//////////////////////////////////////////////////////////////////////////
		case ATM_READY:											// Ready Mode
//////////////////////////////////////////////////////////////////////////
// Mode Procedure
			if (m_pDevCmn->AtmStatus != m_pDevCmn->AtmStatusSave)
			{
				NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_READY\n"));

				SynchronizeKCV();						// [#2580] AU Kook 2019.08.07	update KCV once when entering ATM_READY.

// [#68] HWANG 2008.04.02 EMV Level2
				m_pDevCmn->fnAPL_UnUsed(ATM_READY);		// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
				m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);

				// [#220] NH KSK 2008.5.21
				// HOST와 Configuration Fail후 장애 발생 -> 복귀 시 화면 갱신안되는 문제
				// 해결을 위해 다시 Configuration을 수행함
				if (!m_pDevCmn->fnAPL_CheckHostOpen())
					m_pDevCmn->HostOpenRetryTime = 0;
				// end of [#220]
			}
			m_pDevCmn->AtmStatusSave = ATM_READY;
			
// Before Procedure
			m_pDevCmn->fnAPL_SetSensorInfo();
			m_pDevCmn->fnAPL_GetAvailTrans();
			m_pDevCmn->fnAPL_CheckDevice();
			m_pDevCmn->fnAPL_CheckMaterial();
// Mode Setting(Condition Procedure)
			nConfigOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE);	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 

			if (!m_pDevCmn->fnAPL_CheckError())		// ERROR CODE 여부 CHECK
				Main_SetAtmStatus(ATM_ERROR);		// [#93] KSK 2008.04.08
			// [#423] [NH] KSK 2008.9.18
			else
			if (Mode_CheckErrorElements() == TRUE)
				Main_SetAtmStatus(ATM_ERROR);
			// end of [#423]
			else
//			if (m_pDevCmn->fnAPL_GetDownErrorDevice(~m_pDevCmn->fnAPL_GetAutoOffDevice(DEV_MAIN)))	// DOWN DEVICE FLAG 여부 CHECK	[#431] [NH] KSK 2008.10.02
//			if (m_pDevCmn->fnAPL_GetDownErrorDevice(~m_pDevCmn->fnAPL_GetAutoOffDevice(DEV_MAIN|DEV_RFID)))	// DOWN DEVICE FLAG 여부 CHECK	[#431] [NH] KSK 2008.10.02 [#2325] NH KSK 2015.01.29
			if (m_pDevCmn->fnAPL_GetDownErrorDevice(~m_pDevCmn->fnAPL_GetAutoOffDevice(DEV_MAIN|DEV_RFID|DEV_BCR|DEV_BNA)))	// [#GLDV-3005] US Kook 2022.01.03 Side Car
				Main_SetAtmStatus(ATM_ERROR);		// [#93] KSK 2008.04.08
			else
			if (ConnectHost(nConfigOption) == FALSE)// Open Procedure	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
				Main_SetAtmStatus(ATM_READY);		// [#93] KSK 2008.04.08
			else
			if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_DOWNLOAD_FAIL || m_pDevCmn->nPINErrorFlag == PIN_MAC_BLANK_ERROR)
			{
				// [#2134] NH KSK 2012.05.07 Working Key Download Fail시 ATM Error로 전환
				Main_SetAtmStatus(ATM_ERROR);
				// end of [#2134]
			}
			// [#2277] US Justin 2014.06.10 Support remote status change(US, CA, MX)
			else
			if (Mode_CheckAdditionalATMStatus() == TRUE)
			{
				Main_SetAtmStatus(ATM_ERROR);
			}
			// end of [#2277]
			else												// Next Mode
			{
				Main_SetAtmStatus(ATM_CUSTOM);		// [#93] KSK 2008.04.08
			}

			if (!ClerkProc())						// [#24] KSK 2008.03.11 Configuration Fail시 OP 진입 불가 BUG 수정
				Main_SetAtmStatus(ATM_REBOOT);		// [#93] KSK 2008.04.08

			if (Main_NMS_Proc() != T_OK)
				break;

			m_pDevCmn->bAntiSkimmingFlag = FALSE;	// [#2085] NH KSK 2011.07.22 AntiSkimming 변수 초기화 (에러인경우 Error에서 다시 설정하므로 무조건 초기화 한다)

			break;

//////////////////////////////////////////////////////////////////////////
		case ATM_CUSTOM:												// Custom Mode
//////////////////////////////////////////////////////////////////////////
			if (Main_NMS_Proc() != T_OK)	break;		// [#8] KSK 2008.03.19	test
// Mode Procedure

			if ((m_pDevCmn->AtmStatus != m_pDevCmn->AtmStatusSave))
			{
				NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_CUSTOM\n"));

				//m_pTranCmn->BIZ_EVENT_SendHost(_EVENTID_IN_SERVICE);		// [#2313] US Justin 2014.11.20 Notice Event

				// [#287] [NH] KSK 2008.6.12 ATM_CUSTOM으로 올 경우에는 ERROR가 없다고 가정하여 모두 CLEAR한다.
				// 통신장애시에 CUSTOM으로 올 경우에는 Error를 Clear한다
				m_pDevCmn->fnAPL_ClearError();
				strSaveErrCode.Empty();	// [#419] [NH] 2008.9.12
				// end of [#287]

				m_pDevCmn->nPINErrorFlag = WORKINGKEY_INIT;			// [#2134] NH KSK 2012.05.07

				#if (AU_VERSION)
					m_pDevCmn->m_bDomesticFallbackTrans = FALSE;	// [#2379] AU KSK 2015.11.17
				#endif

				m_pDevCmn->m_bUnKnownAID_SkipPE = FALSE;					// [#2436] US Justin 2016.07.22 Not sending POS ENTRY For Unknown AID
				m_pDevCmn->m_bShownMultiAIDSelection = FALSE;				// [#2440] US Justin 2016.08.10 

				// Media 종류 별로 Reversal Transaction 수행 후 Media 종류를 IC로 변경
				if(m_pDevCmn->fnMCU_IsEmvEnable())			// [#397] [NH] psc 2008.08.01 EMV(IC) 거래일때
					m_pDevCmn->nKindOfMedia = MEDIA_IC;		// [#2082] NH KSK 2011.07.11
				else
					m_pDevCmn->nKindOfMedia = MEDIA_MS;

				if (g_bOutofService)
				{
					// change flag to INSERVICE
					g_bOutofService = FALSE;

					m_strNVLog.Format(L"JNL_SAVE");
					NVDump('O', 'A', "11", L"", m_strNVLog); // [#2024] NH KSK 2011.02.24

					m_pDevCmn->m_JNLMgr.Save(CHANGE_SERVICE_MODE);	// [#3] NH AIREAT 2008.3.10
				}

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
				m_nPINInputCount = 0; //거래 대기 화면에서 PINInputCount 초기화 AU SOOk 2009.06.17
#endif
				// ADA시 Volume조정 적용 루틴
				m_pAdaCtrl->fnExp_SetVolume(MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL));

				// 속도개선
				if (m_pDevCmn->fnSCR_GetCurrentScreenNo() != 101)
					m_pDevCmn->fnCDU_GetCashDispenseInfo();
			
				m_pDevCmn->fnAPL_SetProcCount('0');
				m_pDevCmn->fnSCR_SetCurrentLangMode(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE));
				m_pDevCmn->m_nPrevLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();		// [#2339] NH Justin 2015.04.07 Prevent Font corruption on Out of Service Screen

#if !(AU_VERSION)	// [#2487] AU KS 2017.06.09 호주는 Weather Service 사용 안하므로 해당 영역을 수수료 표시 화면으로 사용하도록 변경(APCA 규정 대응)
				// Get WeaherInfo
				GetWeatherInfo();
#endif				// end of [#2487]

				// Get NoticeInfo
				GetNoticeInfo();

#if (US_VERSION)
				m_pTranCmn->BIZ_WelcomeProc(TRUE);
#elif (AU_VERSION)
				m_pTranCmn->BIZ_WelcomeProc(TRUE);
#elif (CA_VERSION)
				m_pTranCmn->BIZ_CA_WelcomeProc(TRUE);
#elif (MX_VERSION)
				m_pTranCmn->BIZ_MX_WelcomeProc(TRUE);	// [#2115] MX KSK 2012.01.16
#endif

				// [#2205] US KSK 2013.06.28
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
					SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_INSERVICE), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_INSERVICE), SKIP_MCULED);
				// end of [#2205]
			}

			m_pDevCmn->AtmStatusSave = ATM_CUSTOM;

			m_pDevCmn->fnAPL_SetSensorInfo();
			m_pDevCmn->fnAPL_GetAvailTrans();
			m_pDevCmn->fnAPL_CheckDevice();
			m_pDevCmn->fnAPL_CheckMaterial();

			// Mode Setting(Condition Procedure)
			if (!m_pDevCmn->fnAPL_CheckError())
			{
				Main_SetAtmStatus(ATM_ERROR);
			}
			else if (Mode_CheckErrorElements() == TRUE)
			{
				Main_SetAtmStatus(ATM_ERROR);
			}
			else if (m_pDevCmn->nPINErrorFlag == WORKINGKEY_DOWNLOAD_FAIL || m_pDevCmn->nPINErrorFlag == PIN_MAC_BLANK_ERROR)
			{
				// [#2134] NH KSK 2012.05.07 Working Key Download Fail시 ATM Error로 전환
				Main_SetAtmStatus(ATM_ERROR);
				// end of [#2134]
			}
			// [#2277] US Justin 2014.06.10 Support remote status change(US, CA, MX)
			else if (Mode_CheckAdditionalATMStatus() == TRUE)
			{
				Main_SetAtmStatus(ATM_ERROR);
			}
			// end of [#2277]
			// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화(IC 우선거래를 위해 필요한다.)
			else if((m_pTranCmn->DidMediaExist() == TRUE) && (m_pDevCmn->fnMCU_GetMaterialInfo() == 0))
			{
				m_strNVLog.Format(L"CARD_REMOVED");
				NVDump('F', 'A', "11", L"", m_strNVLog); // [#2024] NH KSK 2011.02.24

				//RETAILMSG(1, (L"GO CUSTOM -> READY 1 !!\n"));
				NHDEBUG(DBG_INFO, (L"GO CUSTOM -> READY 1 !!\n"));
				Main_SetAtmStatus(ATM_READY);
			}
			// SPR 축퇴 CHECK
//			else if (m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN))
			//else if (m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN|DEV_RFID))					// [#2325] NH KSK 2015.01.30
			else if (m_pDevCmn->fBAPL_IsDeviceStatusChanged(m_pDevCmn->fnCMN_GetActiveDevice()))	// [#GLDV-3005] US Kook 2022.04.26 side car
			{
				//RETAILMSG(1, (L"GO CUSTOM -> READY 2 !!\n"));
				NHDEBUG(DBG_INFO, (L"GO CUSTOM -> READY 2 !!\n"));
				NVDump('F', 'A', "11", L"", L"DEV_CHANGED");	// [#2024] NH KSK 2011.02.24
				Main_SetAtmStatus(ATM_READY);	// [#93] KSK 2008.04.08
			}

#if (US_VERSION)
			if( m_pTranCmn->BIZ_WelcomeProc() == TRUE )
#elif (AU_VERSION)
			if (m_pTranCmn->BIZ_WelcomeProc() == TRUE)
#elif (CA_VERSION)
			if (m_pTranCmn->BIZ_CA_WelcomeProc() == TRUE)
#elif (MX_VERSION)
			if (m_pTranCmn->BIZ_MX_WelcomeProc() == TRUE)	// [#2115] MX KSK 2012.02.05
#endif
			{
				m_pAdaCtrl->fnExp_SetVolume(MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL));
//				m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN);
				m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN|DEV_RFID);	// [#2325] NH KSK 2015.01.30
				Main_SetAtmStatus(ATM_READY);
				m_pDevCmn->m_nPrevLangMode = m_pDevCmn->fnSCR_GetCurrentLangMode();		// [#2339] NH Justin 2015.04.07 Prevent Font corruption on Out of Service Screen
			}
			else 
			{
				// Mode Setting(Non Condition Procedure)
				if (!ClerkProc())
					Main_SetAtmStatus(ATM_REBOOT);	// [#93] KSK 2008.04.08
			}
			break;

//////////////////////////////////////////////////////////////////////////
		case ATM_REBOOT:											// Reboot Mode
//////////////////////////////////////////////////////////////////////////
// Mode Procedure
			NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_REBOOT\n"));

// [#68] HWANG 2008.04.02 EMV Level2
			m_pDevCmn->fnAPL_UnUsed(ATM_REBOOT);		// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
// end of [#68]

			if(m_pDevCmn->AtmStatusSave == ATM_RMSACTIVE)
				TerminateATM(L"ATM will be rebooted by RMS");				
			else
				TerminateATM();
			break;

//////////////////////////////////////////////////////////////////////////
		case ATM_RMSACTIVE:											// Ready Mode
//////////////////////////////////////////////////////////////////////////

// Mode Procedure
			if (m_pDevCmn->AtmStatus != m_pDevCmn->AtmStatusSave)	//RMS ACTIVE mode로 처음 진입시
			{
				NHDEBUG(DBG_INFO, (L"ATM MODE : ATM_ACTIVE\n"));

//				MemSetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_SPRSP_PAPERSTAT, m_pDevCmn->fnSPR_GetPaperStatus());

				// [#2205] US KSK 2013.06.28
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
					SetHaloLedControl(HALOLED_COLOR_CYAN, OFF_HALOLED_MODE, SKIP_MCULED);
				// end of [#2205]

				m_pDevCmn->AtmStatusSave = ATM_RMSACTIVE;

				m_pDevCmn->Disable_CardReader_RFID();		// [#2492] US Justin 

				if (!ProcRMSCtrl(RMS_LISTEN))
				{
					// if ATMStatus not changed in ProceRMSCtrl, ATMStatus will be ATM_CUSTOM
					Main_SetAtmStatus(ATM_READY);	// [#93] KSK 2008.04.08
				}
			}
			break;

		case ATM_REMOTEUDPATE:
			NHDBG((L"Set to remote update mode\r\n"));

			if (!ExecuteSWUpdate())
			{
				NHERROR((L"Could not launch update process\r\n"));
				m_pDevCmn->AtmStatus = ATM_READY;
			}

			break;

		default:
			NHDEBUG(DBG_INFO, (L"ATM MODE : UNKNOWN(%d)\n", m_pDevCmn->AtmStatus));

			m_strNVLog.Format(L"UNKNOWN_MODE");
			NVDump('F', 'A', "11", L"", m_strNVLog); // [#2024] NH KSK 2011.02.24

			break;
	}

	return TRUE;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CMainFrame
 FUNCTION NAME: ProcRMSCtrl()
 RETURN TYPE  : TRUE  : AtmStatus가 변경 되었다.
				FALSE : AtmStatus가 변경 되지 않았다.
 PARAMETER    : OpenType : RMS_LISTEN	- RMS로 부터 접속을 받는 모드 (ATM <- RMS)
						 : RMS_CONNECT  - RMS로 접속을 하는 모드      (ATM -> RMS)
 DESCRIPTION  : RMS로 상태전송과 연속 CMD의 일부를 수행한다.
-------------------------------------------------------------------*/
//BOOL CMainFrame::ProcRMSCtrl(int OpenType)
BOOL CMainFrame::ProcRMSCtrl(int OpenType, char SubCommand)	// [#613] AU_C KSK 2010.01.18
{
	#ifdef APP_LOCAL_MODE
		return FALSE;
	#endif

	// [#2507] US Justin 2017.09.26 Add Test Button on RMS Send Screen
	char nNewSubCommand = SubCommand;
	BOOL bIsOPTest = FALSE;
	if( SubCommand == RMS_REQ_OP_TEST)
	{
		bIsOPTest = TRUE;
		nNewSubCommand = 0;
	}
	// End of [#2507]

	BOOL	bRes = FALSE;
	int		nRMSCmd = 0;
	int		nTempAtmStatus, nTempAtmStatusSave;

	// if RMS Disable, not Check for RMS
	if (!MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG))
	{
		NHDEBUG(DBG_INFO, (_T("RMS: RMS DISABLED !!\n")));
		m_bLastRMSSendResult = TRUE;							// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
		return FALSE;
	}

	// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
	#if( US_VERSION || CA_VERSION || MX_VERSION )
	if( (OpenType == RMS_CONNECT) && (SubCommand==0) )		// Status Send....
		m_bLastRMSSendResult = FALSE;
	#endif
	// End of [#2559]

	// Check invalid setting.
	if (OpenType == RMS_CONNECT)
	{
		CString strTemp;

		m_RMSSendTime = COleDateTime::GetCurrentTime();		// [#] AIREAT 2009.06.05

		// Terminal ID
		strTemp = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
		if (!strTemp.GetLength() || strTemp == TERMINAL_ID_DEFAULT)	// [#398] [NH] KSK 2008.8.6
			return FALSE;
		
		if (GetCurrnetRMSLineType() == RMS_LINE_TCP)
		{
			// RMS IP
			strTemp = MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_RMSNAME);
			if (!strTemp.GetLength())
				return FALSE;
		}
		else
		{
			// RMS Phone number
			strTemp = MemGetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSPHONENO1);
			if(!strTemp.GetLength())
				return FALSE;
		}
	}
	
	// Disable Hot Key
	// [#2507] US Justin 2017.09.26 Add Test Button on RMS Send Screen
	//m_pDevCmn->fnAPL_UnUsed(ATM_RMSACTIVE);		// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
	if(bIsOPTest != TRUE)
		m_pDevCmn->fnAPL_UnUsed(ATM_RMSACTIVE);		// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
	// End of [#2507]

	// Save ATM Status
	nTempAtmStatus = m_pDevCmn->AtmStatus;
	nTempAtmStatusSave = m_pDevCmn->AtmStatusSave;
	m_pDevCmn->AtmStatus = ATM_RMSACTIVE;
	m_pDevCmn->AtmStatusSave = ATM_RMSACTIVE;

	// Display RMS Mode.
	m_pDevCmn->fnAPL_DisplayPrintError(TRUE);
	Delay_Msg(1000);

	while(1)
	{	
		// Clear previous RMS CMD.
		MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_IDLE);
		
		{	
			// after process RMS.

			// [#2507] NH Justin 2017.09.27
			//m_pRmsCtrl->RMS_Communication(GetCurrnetRMSLineType(), OpenType);
			//m_pRmsCtrl->RMS_Communication(GetCurrnetRMSLineType(), OpenType, SubCommand);		// [#613] AU_C KSK 2010.01.18
			BOOL bCommResult = m_pRmsCtrl->RMS_Communication(GetCurrnetRMSLineType(), OpenType, nNewSubCommand);	
			NHDEBUG(DBG_INFO, (L"RMS_Communication(Type=[%d], OpenType=[%d], subCmd=[%d]) ==> Return=[%d] \n", GetCurrnetRMSLineType(), OpenType, nNewSubCommand, bCommResult));

			if(bIsOPTest == TRUE)
				bRes = bCommResult;
			// End of [#2507]

			// [#2559] NH Justin 2018.06.18 Retry Status Send if "send" failed
			#if( US_VERSION || CA_VERSION || MX_VERSION )
			if( (OpenType == RMS_CONNECT) && (SubCommand==0) )
			{
				m_bLastRMSSendResult = bCommResult;			// Status Send Result
				NHDEBUG(DBG_INFO, (L"RMS Status Send Result (m_bLastRMSSendResult) = [%d] \n", m_bLastRMSSendResult));
			}
			#endif
			// End of [#2559]

			// if status send, update time to sent to rms
		}
		
		// Get RMS CMD and Clear
		nRMSCmd = MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND);
		NHDEBUG(DBG_INFO, (L"RMS COMMAND = [%d] \n", nRMSCmd));

		// [#532] NH KSK 2009.06.09 REMOTE AUTO DAY TOTAL or AUTO DAY TOTAL을 구분하기 위해 COMMAND 수행 후 CLEAR하도록 위치 수정
		//MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSCOMMAND, RMSCMD_IDLE);

		switch(nRMSCmd)
		{
		case RMSCMD_REBOOT:
			{
				Delay_Msg(5000);
				m_pDevCmn->AtmStatus = ATM_REBOOT;
				bRes = TRUE;
			}
			break;
			
		case RMSCMD_INITALLDEV:
			{
				ResetDeviceOf(ALLDEV);
				m_pDevCmn->AtmStatus = ATM_RMSACTIVE;
				// [#2557] US Justin 2018.06.12 Reset GivePay cashed data when RMS Initialiae All command is issued.
				#if (APP_GPAY_GIFTCARD_PURCHASE)
				m_pTranCmn->m_GivePayData.ResetGPTransactionData(TRUE);
				#endif
				// End of [#2557]
				Delay_Msg(5000);
			}
			break;
			
		case RMSCMD_INITERRDEV:
			{
				ResetDeviceOf(ERRDEV);
				m_pDevCmn->AtmStatus = ATM_RMSACTIVE;
				Delay_Msg(5000);
			}
			break;
			
		case RMSCMD_SWUPDATE:
			{
				RegSetInt(L"SOFTWARE\\ATM\\APP", L"RMSFileUpdate", 1);
				Delay_Msg(2000);
				if (ExecuteSWUpdate() == TRUE)
				{
					Delay_Msg(15000);
					bRes = TRUE;
				}
				else
					bRes = FALSE;
			}
			break;

		// [#RWC6-134] US William 2020.03.24 RMS Remote Updates
		case RMSCMD_SWUPDATE_REPO:
			{
				Delay_Msg(2000);
				bRes = TerminateATM();
			}
			break;

		// [#530] NH KSK 2009.06.03
		case RMSCMD_SETADDCASH:
			{
				// ADD CASH PROC EXECUTE
				CStringArray strArrAddCashInfo;
				CString		 strTemp;

				strArrAddCashInfo.RemoveAll();
				strTemp = L"ADDCASH";
				strArrAddCashInfo.Add(strTemp);

				strTemp.Format(L"%d", m_pRmsCtrl->m_AddCashInfo.CST1cnt);
				strArrAddCashInfo.Add(strTemp);

				strTemp.Format(L"%d", m_pRmsCtrl->m_AddCashInfo.CST2cnt);
				strArrAddCashInfo.Add(strTemp);

				// [#2520] NH Justin 2017.12.11 1500SE Remote Add Cash Bug Fix
				/*
				strTemp.Format(L"%d", m_pRmsCtrl->m_AddCashInfo.CST3cnt);
				strArrAddCashInfo.Add(strTemp);

				strTemp.Format(L"%d", m_pRmsCtrl->m_AddCashInfo.CST4cnt);
				strArrAddCashInfo.Add(strTemp);
				*/
				if (m_pDevCmn->m_pConfig->GetOSVersion() != NH_OS_NH1500SE)
				{
					strTemp.Format(L"%d", m_pRmsCtrl->m_AddCashInfo.CST3cnt);
					strArrAddCashInfo.Add(strTemp);

					strTemp.Format(L"%d", m_pRmsCtrl->m_AddCashInfo.CST4cnt);
					strArrAddCashInfo.Add(strTemp);
				}
				// End of [#2520]

				NHDEBUG(1, (L"REMOTE RMS TEST\n"));
				NHDEBUG(1, (L"ADD CASH [%s]\n", strArrAddCashInfo[0]));
				NHDEBUG(1, (L"ADD CASH [1ST] - [%s]\n", strArrAddCashInfo[1]));
				NHDEBUG(1, (L"ADD CASH [2ND] - [%s]\n", strArrAddCashInfo[2]));
				NHDEBUG(1, (L"ADD CASH [3RD] - [%s]\n", strArrAddCashInfo[3]));
				NHDEBUG(1, (L"ADD CASH [4TH] - [%s]\n", strArrAddCashInfo[4]));

				OPERATION_RESULT res = ProcAddCash(strArrAddCashInfo, 1);

				NHDEBUG(1, (L"REMOTE RMS TEST(%d)\n", (int)res));
				NHDEBUG(1, (L"ADD CASH [1ST] - [%s]\n", strArrAddCashInfo[1]));
				NHDEBUG(1, (L"ADD CASH [2ND] - [%s]\n", strArrAddCashInfo[2]));
				NHDEBUG(1, (L"ADD CASH [3RD] - [%s]\n", strArrAddCashInfo[3]));
				NHDEBUG(1, (L"ADD CASH [4TH] - [%s]\n", strArrAddCashInfo[4]));
				NHDEBUG(1, (L"------------------------\n"));
			}
			break;
		// end of [#530]

		// [#531] NH KSK 2009.06.03
		case RMSCMD_EXECCSTTOTAL:
			{
				m_pTranCmn->BIZ_CassetteTotalProc(TRUE, 1);
			}
			break;
		// end of [#531]

		// [#532] NH KSK 2009.06.03
		case RMSCMD_EXECTRIALDAYTOTAL:
		case RMSCMD_EXECDAYTOTAL:
			{
				CString strAutoDayTotal_ErrorCode;	//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 

				if (m_pDevCmn->HostOpenFlag == TRUE)
				{					
					if ( nRMSCmd == RMSCMD_EXECDAYTOTAL)
					{
						m_pTranCmn->BIZ_DayTotalProc(FALSE, TRUE);
					}
					else
					{
						m_pTranCmn->BIZ_DayTotalProc(FALSE, FALSE);
					}
					
					//[#2000] SOOK 2010.10.12 호주 사양 적용 (Configuration at start) 
					if (m_pDevCmn->TranResult == FALSE)
						strAutoDayTotal_ErrorCode = m_pDevCmn->m_pNetWork->GetErrorCode();
					
					// [#639] NH KSK 2010.05.03 DAY TOTAL MAC ERROR시 CONFIGURATION 수행하도록 수정
					// 현재는 STANDARD3일 경우에만 설정하도록 되어져 있으므로, MESSAGE FORMAT별 분기처리는 하지 않음
					if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_CONF)	// reversal flag가 2이면 configuration만 한다.
					{
						m_pDevCmn->HostOpenFlag = FALSE;
						m_pDevCmn->HostOpenRetryTime = 0;
						
						if (!m_pTranCmn->BIZ_HostConfigProc(FALSE))
						{
							if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CONFIGURATION_ENABLE) == 1)
							{
								// Option이 Disable인 경우 Success로 변수 설정
								m_pDevCmn->HostOpenFlag = TRUE;
								m_pDevCmn->HostOpenRetryTime = 0;
							}
						}
						// REVERSAL FLAG CLEAR
						MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
						
						// DAY TOTAL FAIL이므로 TranResult를 Fail로 설정
						m_pDevCmn->TranResult = FALSE;
					}
					// end of [#639]
					//end of [#2000]

					if (!m_pDevCmn->TranResult)
					{
						CString strJnlMsg;
						// when total failed, JNL log
						if (nRMSCmd == RMSCMD_EXECDAYTOTAL)
							strJnlMsg.Format(L"Remote Auto Day Total - Failed(%7.7s)", strAutoDayTotal_ErrorCode);
						else
							strJnlMsg.Format(L"Remote Auto Trial Day Total - Failed(%7.7s)", strAutoDayTotal_ErrorCode);
						
						#if SUPPORT_CHANGE_PARAMETER_JNL	// KSK Bug Fix
							m_pDevCmn->m_JNLMgr.Save(MONIVIEW_ACTION, strJnlMsg);
						#endif
					}
				}
			}
			break;
		// end of [#532]

		case RMSCMD_CHANGEDATETIME:
			{
				char DateTime[15];
				CString strDateTime;

				memset(DateTime, 0, sizeof(DateTime));
				strncpy(DateTime, (const char *)m_pRmsCtrl->m_DateTimeInfo.DateTime, 14);
				strDateTime = DateTime;

				// KSK 2010.01.20 Bug Fix System Time 변경 시 AUTO DAY TOTAL 및 Scheduled Journal Upload 하도록 수정
				SYSTEMTIME	ti, orgTi;

				GetLocalTime(&ti);
				GetLocalTime(&orgTi);

				//MMDDYYYYHHmmSS 형식임 
				ti.wMonth   = Asc2Int(strDateTime.Left(2));
				ti.wDay		= Asc2Int(strDateTime.Mid(2, 2));
				ti.wYear	= Asc2Int(strDateTime.Mid(4, 4));
				ti.wHour	= Asc2Int(strDateTime.Mid(8, 2));
				ti.wMinute	= Asc2Int(strDateTime.Mid(10, 2));
				ti.wSecond	= Asc2Int(strDateTime.Mid(12, 2));
				SetLocalTime(&ti);

				// [#613] AU_C KSK 2010.01.18 System Date 변경 시 무조건 Send하도록 함
				if (orgTi.wYear != ti.wYear || orgTi.wMonth != ti.wMonth || orgTi.wDay != ti.wDay)
				{
					MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_AUTODAYTOTAL_TIME_YEAR, 2000);
					// [#2037] AU KSK 2011.03.31	
					// [#2457] US Justin 2016.12.09 Support Moniview Journal Uploading
					// [#2500] CA Justin 2017.08.22 Support Moniview Journal Uploading
					#if (AU_VERSION || US_VERSION || CA_VERSION)
						MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SCHEDULED_JOURNAL_YEAR, 2000);
					#endif
				}
				// end of [#613]
				
				#if SUPPORT_CHANGE_PARAMETER_JNL	//[#610] SOOK 2010.01.16 Configuration Change 저널 추가 
					m_pDevCmn->m_JNLMgr.Save(MONIVIEW_ACTION, L"ATM Date & Time Changed"); 
				#endif //end of [#610]
			}
			
		case RMSCMD_IDLE:
		default:
			{
				// Recovery Atm Status
				if (nTempAtmStatus != ATM_RMSACTIVE)
				{
					m_pDevCmn->AtmStatus = nTempAtmStatus;
					m_pDevCmn->AtmStatusSave = nTempAtmStatusSave;

					NHDEBUG(DBG_INFO, (L"RMS COMMAND is RMSCMD_IDLE or DEFAULT.. Status = [%d] \n", nTempAtmStatus));
				}
				else
				{
					// 연속 CMD가 아닌 RMS 접속은 READY에서 다시 시작.
					Main_SetAtmStatus(ATM_READY);

					NHDEBUG(DBG_INFO, (L"RMS COMMAND is RMSCMD_IDLE or DEFAULT.. Make ATM_READY \n"));
				}

				// [#2507] US Justin 2017.09.26 Add Test Button on RMS Send Screen
				//bRes = FALSE;
				if(bIsOPTest != TRUE)
					bRes = FALSE;
				// End of [#2507]
			}
			break;
		}

		// [#71] KSK 2008.03.20
		// RMS에서 RMSCMD_INITALLDEV / RMSCMD_INITERRDEV Command가 올 경우 실행하고 나서
		// 다른 Error가 있을 경우 RMS로 ErrorCode를 올리지 못하는 문제 있음
		m_pDevCmn->fnAPL_SetSensorInfo();	// Sensor Check
		Check_DoorSensor();
		Check_CSTSensor();
		// end of [#71]

		// end of RMS Ctrl.
		if (m_pDevCmn->AtmStatus != ATM_RMSACTIVE)
			break;
		
		Delay_Msg(500);
	}

	// Hot Key Enable
	// [#2507] US Justin 2017.09.26 Add Test Button on RMS Send Screen
	//m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
	if(bIsOPTest != TRUE)
		m_pDevCmn->fnAPL_EnDisableHotKey(ENABLE);
	// End of [#2507]

	return bRes;
}

// [#51] NH PSC 2008.04.03 CDU 축퇴처리를 위해 Check_DoorSensor() 함수와 분리함
BOOL CMainFrame::Check_CSTSensor()
{
	BOOL nReturn = TRUE;

	if (Mode_CSTStatus == CST_SET_NG)
	{
		m_pDevCmn->fnAPL_StackError(_T("2000100"), L"", DEV_CDU);	// [#405] [NH] KSK 2008.8.25	// [#419] [NH] KSK 2008.9.16
		nReturn = FALSE;
	}
	else
	if (Mode_CSTStatus == CST_NEAR && MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INSERVICE_WHENCSTSSHORT) != ENABLE)
	{
		m_pDevCmn->fnAPL_StackError(_T("2000200"), L"", DEV_CDU);	// [#405] [NH] KSK 2008.8.25	// [#419] [NH] KSK 2008.9.16
		nReturn = FALSE;
	}
	else
	if (Mode_CSTStatus == CST_EMPTY && MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_INSERVICE_WHENCSTSEMPTY) != ENABLE)
	{
	#if (AU_VERSION) //[#599] 2010.01.15 Customers 예외 사항 추가	[#2041] AU KSK 2011.03.31
		m_pDevCmn->fnAPL_StackError(_T("F000100"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003358), DEV_CDU);	
		nReturn = FALSE;
	#else  //enf of [#599]
		m_pDevCmn->fnAPL_StackError(_T("F000100"), L"", DEV_CDU);	// [#405] [NH] KSK 2008.8.25	// [#419] [NH] KSK 2008.9.16
		nReturn = FALSE;
	#endif
	}

	return nReturn;
}

// [#51] NH PSC 2008.04.03 CDU 축퇴처리를 위해 Check_CSTSensor() 함수와 분리함
BOOL CMainFrame::Check_DoorSensor()
{
	return m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_CLOSED; // [#RWC6-14] US William 2019.09.23 Safe door journal
}

BOOL CMainFrame::Check_AntiSkimming()
{
#if 0
	if (IsAntiSkimmingSensorOn() == true)
	{
		CString strTemp = L"Anti";

		if (m_pDevCmn->m_ErrorCodeInfo.Find(L"CARDSKM"))
			strTemp = m_pDevCmn->m_ErrorCodeInfo.GetDescription(m_pDevCmn->fnSCR_GetCurrentLangMode());

		m_pDevCmn->fnAPL_StackError(_T("CARDSKM"), strTemp, DEV_SNS);
		return FALSE;
	}
#endif
	return TRUE;
}

// [#23] KSK 2008.03.19
BOOL CMainFrame::Check_BrokenNVRAM()
{
	CLoginManager manager;
	if (manager.ArePasswordsInInitialState()) // [#RWC6-107] US William Hash passwords
	{
		return TRUE;
	}

	if (CheckNVRam() == FALSE)
	{
		ClearAllNVRam();

#ifdef	APP_LOCAL_MODE
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MASTERPW, L"777777");
#else
		// [#RWC6-107] US William Hash passwords
		manager.InitializePasswords();
#endif
		// ERROR 모드에서 CLERK 모드로의 진입을 위해 Network 전송 안하게 설정.
		MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSFLAG,		0);
		MemSetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_HEALTHYFLAG,	0);

		CNHConfig *m_pConfig = GetConfigFuncPointer();
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND, m_pConfig->GetMachineType());	// 국가별 Default 기종을 Set한다.

		return TRUE;		
	}
	return FALSE;
}
// end of [#23]

// [#RWC6-56] US William 2019.12.26 Hash passwords
BOOL CMainFrame::UpgradePasswords()
{
	CLoginManager manager;
	if (manager.ArePasswordsInInitialState()) // [#RWC6-107] US William Hash passwords
	{
		return FALSE;
	}

	return (BOOL) manager.UpgradeOldPasswordsOrNoop();
}

// [#2326] US Kook 2015.09.24 refactoring
BOOL CMainFrame::Check_CSTRjtFull()
{
	if (m_pDevCmn->RejectCSTStatus == CST_FULL)
	{
		m_pDevCmn->fnAPL_StackError(_T("2000300"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003042), DEV_CDU); // [#419] [NH] KSK 2008.9.16
		return FALSE;
	}
	return TRUE;
}
// end of [#2326]

// [#2518] US Kook 2018.02.26 Support MX-2800SE, Camera
BOOL CMainFrame::Check_Camera()
{
#ifdef UNDER_CE
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
	{
		if (m_pDevCmn->fnCAM_GetDeviceStatus(CAM_MAIN) != NORMAL)
		{
			CString strCamErrorCode = m_pDevCmn->fstrCAM_GetErrorCode();

			NVDump('F', 'C', "93", strCamErrorCode, _T("Camera Error"));

			// [#2548] NH Justin 2018.04.27 Leave IN SERVICE even though the Camera is OUT OF SERVICE
			//m_pDevCmn->fnAPL_StackError(strCamErrorCode, _T("CAMERA ERROR DESCRIPTION"), DEV_CMR);
			// End of [#2548]
			
			return FALSE;
		}
	}
#endif

	return TRUE;
}
// end of [#2518]

// [#2580] AU Kook 2019.08.06 detect 'EPP INITIALIZATION STATE'.
BOOL CMainFrame::Check_EPPInitState()
{
	NHDEBUG(1, (_T("CMainFrame::Check_EPPInitState()(%s) \n"), _T(" ")));

	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_PCI20_EP_VERSION) >= 0)
	{
		if (m_pDevCmn->fnPIN_GetDeviceStatus() == NORMAL)
		{
			// 2021.10.27 Please note that EPP SP doesn't produce related error codes yet. ("E21FF03", "EX1FF03")
			CString strPINErrCode = m_pDevCmn->fstrPIN_GetErrorCode();
			if (strPINErrCode == L"E21FF03" || strPINErrCode == L"EX1FF03")
			{
				m_pDevCmn->fnAPL_StackError(strPINErrCode, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003006), DEV_PIN);
				m_strNVLog.Format(_T("EPPINITSTATE"));
				NVDump('F', 'A', "11", L"", m_strNVLog);

				return FALSE;
			}
		}
	}

	return TRUE;
}
// end of [#2580]

// [#2558] NH Justin 2018.06.14 Add Scheduled Reboot Option
void CMainFrame::SetNextRebootTime()
{
	NHDEBUG(DBG_INFO, (L"SetNextRebootTime()\n"));
	BOOL bSetTime = FALSE;
	
	m_tNextReboot = COleDateTime::GetCurrentTime();
	int nRebootOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_OPTION);
	if(nRebootOption == REBOOT_OPTION_ONTIME)
	{
		int nEnteredNum = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_TIME);
		int nEntHH = (int) (0.01 * nEnteredNum);
		int nEntMM = nEnteredNum % 100;
		if( (nEntHH>=0) && (nEntHH<=23) && (nEntMM>=0) && (nEntMM<=59) )		// Set next reboot time only when the entered value is valid
		{
			COleDateTime tNextRbt = COleDateTime(m_tNextReboot.GetYear(), m_tNextReboot.GetMonth(), m_tNextReboot.GetDay(), nEntHH, nEntMM, 0);			
			if( tNextRbt <= m_tNextReboot )
				tNextRbt += COleDateTimeSpan(1, 0, 0, 0);	// 1 day later

			m_tNextReboot = tNextRbt;
			bSetTime = TRUE;
		}
	}		
	else if(nRebootOption == REBOOT_OPTION_INTERVAL)
	{
		int nRebootIntval = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_REBOOT_INTERVAL);
		if( (nRebootIntval>0) && (nRebootIntval<=REBOOT_OPTION_MAX_INTERVAL) ) 		// Set next reboot time only when the entered value is valid
		{
			m_tNextReboot += COleDateTimeSpan(0, nRebootIntval, 0, 0);
			bSetTime = TRUE;
		}
	}

	if(bSetTime!=TRUE)
		m_tNextReboot += COleDateTimeSpan(365, 0, 0, 0);		// 1 year Later... ==> Disabling....

	NHDEBUG(DBG_INFO, (L"Next Reboot = [%04d.%02d.%02d %02d:%02d:%02d]\n", m_tNextReboot.GetYear(), m_tNextReboot.GetMonth(), m_tNextReboot.GetDay(),
		m_tNextReboot.GetHour(), m_tNextReboot.GetMinute(), m_tNextReboot.GetSecond() ) );
}
// End of [#2558]

BOOL CMainFrame::Mode_CheckErrorElements()
{
	if (m_pDevCmn->fnAPL_GetAvailErrorDevice(m_pDevCmn->DeviceStatus))		return TRUE;
	// SYSTEM ERROR가 없을 경우 장애코드가 삭제되므로 fnAPL_CheckError 함수 이후 Check해야함
	else if (!Check_NVRamData())											return TRUE;
	else if (!Check_CSTRjtFull())											return TRUE;
	else if (!Check_DoorSensor())											return TRUE;
	else if (!Check_AntiSkimming())											return TRUE;
	else if (!Check_CSTSensor())											return TRUE;		// [#51] UK PSC 2008.04.03 CDU 축퇴처리

	return FALSE;
}
// end of [#423]

// [#2277] NH Justin 2014.06.10 Additional ATM Status
BOOL CMainFrame::Mode_CheckAdditionalATMStatus()
{
#if (US_VERSION || CA_VERSION || MX_VERSION)		// US, CA, MX
	if( m_pDevCmn->m_nRemoteStatusCmd != 0 )
	{
		m_pDevCmn->fnAPL_StackError(_T("F002000"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003401), DEV_SYS);
		return TRUE;
	}
#endif
	return FALSE;
}
// End of [#2277]
