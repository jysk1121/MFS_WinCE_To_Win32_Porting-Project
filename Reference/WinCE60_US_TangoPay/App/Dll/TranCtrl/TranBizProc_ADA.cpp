#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\TimeCheck.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

//------------------------------------------------------------------
//	Define Symbol
//------------------------------------------------------------------

//------------------------------------------------------------------
//	Global Variable
//------------------------------------------------------------------
CTimeCheck	g_TimeCheck;

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_Welcome()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ADA_Welcome(int*nChosenFtn)
{
	NVDump('O', 'C', "00", L"P_NHADA", L"Welcome");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_Welcome]\n"));

	int	nCheckCount = 0;
	CString			GetKeyStr;
	
	m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);	// Card Enable 처리

	// [#2325] NH KSK 2015.02.27 ADA는 wave file 변경이 필요하여 일단 RFID는 미지원하기로 NHA와 협의함
	//if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RFID_DEVICE_ENABLE) == ENABLE)
	//{
	//	if (m_pDevCmn->fnRFID_GetDeviceStatus() == NORMAL)
	//	{
	//		m_pDevCmn->fnRFID_EntryEnable();
	//		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_RFID);
	//	}
	//}
	// end of [#2325]

	// [#2413] US Justin 2016.04.08 VG for Popmoney and Pin4
	if(nChosenFtn != NULL)	// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
		*nChosenFtn = (int) FID_END_OF_FLOW;
	int nNumEnabled, nEnabledServices;
	GetEnabledCardlessServices(&nNumEnabled, &nEnabledServices);
	// End of [#2413]

	while (1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (m_pAdaCtrl->IsPlaying() == FALSE)
		{
			if (nCheckCount != 0)
			{
				Delay_Msg(1000);
				// [#2375] US Justin 2015.11.02 Add VG ... Check Headphone Jack after Delay
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)	
					break;
				// End of [#2375]
			}

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();

			// [#2199] NH Justin 2013.05.29 Update VG -- Adding 4000W.

			// [#2368] US KSK 2015.10.25 US만 Script변경이 이루어져 국가 Define함
			#if (US_VERSION)	
				// Welcome  => 101_1.wav
				m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_1.wav"); 

				#if(APP_TDL_OPTION)
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"CertifiedVG.wav"); 
				#endif

				// Familiar...
				// [#2380] US Justin 2015.12.21 Cardtronics Additional VG
				m_pAdaCtrl->fnExp_AddWaveFile(101, L"Familiar_ATM.wav");
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")			m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Direction_V.wav");
				else																						m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Direction_H.wav");
				if (P_EMV_CheckTransMode() == RES_NOR_MS_TRANS)		m_pAdaCtrl->fnExp_AddWaveFile(101, L"Smooth_remove.wav");
				else												m_pAdaCtrl->fnExp_AddWaveFile(101, L"Donot_remove.wav");
				// End of [#2380]

				// Pin Location
				if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"PIN_Location_1500.wav"); 	
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX5200SE")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"PIN_Location_1800_5200.wav"); 	
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"PIN_Location_NH2600.wav"); 	
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"PIN_Location_MX2600.wav"); 	
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"PIN_Location_4000.wav");
				else
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"PIN_Location_2700_5000.wav");					// 5300SE...	

				// PIN_Arrangement
				m_pAdaCtrl->fnExp_AddWaveFile(101, L"PIN_Arrangement.wav");

				// Card Location
				if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Card_Location_1500.wav"); 	
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Card_Location_1800.wav");
				else
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Card_Location_Other.wav");

				// Card Insertion
				// [#2380] US Justin 2015.12.21 Cardtronics Additional VG
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")			
				{
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Vertical.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Direction_V.wav");
				}
				else
				{
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Horizontal.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Direction_H.wav");
				}
				if(P_EMV_CheckTransMode() == RES_EMV_IC_FIRST)		
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Donot_remove_yourCard.wav");
				// End of [#2380]

				// Receipt Location
				if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Receipt_Location_1500.wav"); 	
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Receipt_Location_1800.wav");
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Receipt_Location_4000.wav");
				else
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Receipt_Location_Other.wav");

				// Dispenser Location
				if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Dispenser_Location_1500.wav"); 	
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")		// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Dispenser_Location_1800.wav");
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Dispenser_Location_4000.wav");
				else
					m_pAdaCtrl->fnExp_AddWaveFile(101, L"Dispenser_Location_Other.wav");

				m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_2.wav"); 
				m_pAdaCtrl->fnExp_AddWaveFile(101, L"LocationRepeatKey.wav");

				// Begin Transaction
				// [#2380] US Justin 2015.12.21 Cardtronics Additional VG
				m_pAdaCtrl->fnExp_AddWaveFile(101, L"Please_Insert.wav");
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")			m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Direction_V.wav");
				else																						m_pAdaCtrl->fnExp_AddWaveFile(101, L"Insertion_Direction_H.wav");
				if (P_EMV_CheckTransMode() == RES_NOR_MS_TRANS)		m_pAdaCtrl->fnExp_AddWaveFile(101, L"Smooth_remove.wav");
				else												m_pAdaCtrl->fnExp_AddWaveFile(101, L"Donot_remove.wav");
				// End of [#2380]

				if (nNumEnabled > 1)														m_pAdaCtrl->fnExp_AddWaveFile(101, L"Press8_for_Cardless.wav");
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_POPMONEY))		m_pAdaCtrl->fnExp_AddWaveFile(101, L"Press8_for_Popmoney.wav");		// [#2413] US Justin 2016.04.08 VG for Popmoney and Pin4
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_PIN4))			m_pAdaCtrl->fnExp_AddWaveFile(101, L"Press8_for_Pin4.wav");			// [#2413] US Justin 2016.04.08 VG for Popmoney and Pin4
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_JUSTCASH))		m_pAdaCtrl->fnExp_AddWaveFile(101, L"Press8_for_JustCash.wav");		// [#2445] US Justin 2016.09.28 Just.Cash
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_PAYPALCCA))	m_pAdaCtrl->fnExp_AddWaveFile(101, L"Press8_for_Paypal.wav");		// [#2446] US Justin 2016.09.30 Paypal CCA
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_LIBERTYX))		m_pAdaCtrl->fnExp_AddWaveFile(101, L"Press8_for_LibertyX.wav");
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_DIGITALMINT))	m_pAdaCtrl->fnExp_AddWaveFile(101, L"Press8_for_DigitalMint.wav");	// [#RWC6-399] US ryan.payton 2022.09.28 DigitalMint
			#else
				// Welcome  => 101.wav

				// Receipt Slot Location
				if      (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")		m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_3.wav"); 
				else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")		m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_4.wav"); 
				else																							m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_2.wav"); 

				// Card Reader
				if (P_EMV_CheckTransMode() == RES_NOR_MS_TRANS)
				{
					// Non EMV Transaction
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")				m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_6.wav"); 
					else																							m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_5.wav"); 
				}
				else
				{
					// EMV Transaction
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")				m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_8.wav"); 
					else																							m_pAdaCtrl->fnExp_AddWaveFile(101, L"101_7.wav"); 
				}
				// End of [#2199]

			#endif	
			// End of [#2368]
			m_pAdaCtrl->fnExp_PlayScreenWave(101);
		}

		// Sensor Check Every 300 ms (50ms * 6)
		if (nCheckCount >= 6)
		{
			m_pDevCmn->fnAPL_SetSensorInfo();
			m_pDevCmn->fnAPL_GetAvailTrans();
			m_pDevCmn->fnAPL_CheckDevice();
			m_pDevCmn->fnAPL_CheckMaterial();

			if ((m_pDevCmn->fnAPL_CheckError() == FALSE) ||
				(m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN)) ||
//				(m_pDevCmn->fBAPL_IsDeviceStatusChanged(DEV_MAIN|DEV_RFID)) ||	// [#2325] NH KSK 2015.01.30
				(DidMediaExist() == TRUE && m_pDevCmn->fnMCU_GetMaterialInfo() == 0))
				break;

			nCheckCount = 1;
		}

		// Check Card Read
		// [#2325] NH KSK 2015.01.28 ADA인 경우 NFC 지원시 Guide 문구 추가가 필요함. (추후 검토 필요)
//		if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == T_OK) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))
		if ((DidMediaExist() == FALSE) && (LIB_MainMenuDeviceEvent() == DEV_MCU) && (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL))	// [#2325] NH KSK 2015.01.22 LIB_MainMenuDeviceEvent return 변경으로 인해 비교문 변경
		{
			// [#2205] US KSK 2013.06.28 ADA Stop전에 Flicker Off하도록 위치 이동
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
			else
				SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
			// end of [#2205]

			m_nEventKind = DEV_MCU;	// [#2325] NH KSK 2015.01.22

			m_pAdaCtrl->fnExp_StopPlay();
			//m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);	// [#2205]
			return RES_OK;
		}
		else if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			// [#2375] US Justin 2015.11.03 Implement repeat and Invalid Key on Welcome Screen.
			/*
			// [#2145] NH KSK 2012.08.20 US만 Repeat 기능 Support (추후 전체 국가 적용 예정) 
			// [#2199] CA Justin 2013.06.10 Add Repeat on Idle Screen	
			// [#2220] AU KMK 2014.02.12 호주 추가 (원래 사양임)
			#if (US_VERSION || CA_VERSION || AU_VERSION)	
				//Volume 조절
				if (GetKeyStr == L".") //Down
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeDown();
					continue;
				}
				else if (GetKeyStr == L"00") //Up
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeUp();
					continue;
				}
			#endif				
			// end of [#2145]
			*/

			int nValidADAInput = 0;
			if( (GetKeyStr == L".")||(GetKeyStr == L"00"))			// For
			{
				nValidADAInput = 1;					// (1) volume control.
				m_pAdaCtrl->fnExp_StopPlay();
				if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
				else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
			}
			else if(GetKeyStr == L"000" )
			{
				#if (US_VERSION)	
					nValidADAInput = 1;				// (1) repeat  for US ONLY
					m_pAdaCtrl->fnExp_StopPlay();
				#endif
			}
			// [#2413] US Justin 2016.04.08 VG for Popmoney and Pin4
			#if (US_VERSION)
			else if(GetKeyStr == L"8" )
			{
				CString strChosenTr = L"";
				if( nNumEnabled>1 )													
				{
					strChosenTr = L"CardlessTr.wav";
					if(nChosenFtn != NULL)	// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
						*nChosenFtn = (int) FID_NH_CARDLESS_FLOWS;
				}
				else if( (nNumEnabled==1)&&(nEnabledServices==CDLS_SVC_POPMONEY) )
				{
					strChosenTr = L"PopmoneyTr.wav";
					if(nChosenFtn != NULL)	// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
						*nChosenFtn = (int) FID_NH_POPMONEY_FLOWS;
				}
				else if( (nNumEnabled==1)&&(nEnabledServices==CDLS_SVC_PIN4) )
				{
					strChosenTr = L"Pin4Tr.wav";
					if(nChosenFtn != NULL)	// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
						*nChosenFtn = (int) FID_NH_PIN4_FLOWS;
				}
				// [#2445] US Justin 2016.09.27 Just.Cash
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_JUSTCASH))
				{
					strChosenTr = L"JustCashTr.wav";

					if (nChosenFtn != NULL) // [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
						*nChosenFtn = (int)FID_NH_JUSTCASH_START;
				}
				// End of [#2445]
				// [#2446] US Justin 2016.09.30 Paypal CCA
				else if( (nNumEnabled==1)&&(nEnabledServices==CDLS_SVC_PAYPALCCA) )	
				{
					strChosenTr = L"PayPalTr.wav";
					if(nChosenFtn != NULL)	// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
						*nChosenFtn = (int) FID_NH_PAYPAL_FLOWS;
				}
				// End of [#2446]
				else if( (nNumEnabled==1)&&(nEnabledServices==CDLS_SVC_LIBERTYX) )	
				{
					strChosenTr = L"LibertyXTr.wav";
					if(nChosenFtn != NULL)	// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
						*nChosenFtn = (int) FID_NH_LIBERTYX_INIT;
				}
				// [#RWC6-399] US ryan.payton 2022.09.28 DigitalMint
				else if ((nNumEnabled == 1) && (nEnabledServices == CDLS_SVC_DIGITALMINT))
				{
					strChosenTr = L"DigitalMintTr.wav";

					if (nChosenFtn != NULL) // [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA)
						*nChosenFtn = (int)FID_NH_DIGITALMINT_START;
				}
				// End of [#RWC6-188]

				if(strChosenTr.GetLength()>0)
				{
					if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
						m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
					else
						SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);

					m_pDevCmn->fnAPL_DeviceEnDisable(DEV_MCU, DISABLE, TRUE);
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"8.wav", TRUE, strChosenTr);
					return RES_OK; 
				}
			}
			#endif
			// End of [#2413]
								
			if	( nValidADAInput == 0 )				// Invalid
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
				#if (US_VERSION)
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
				#endif
			}
			else if( nValidADAInput == 1)			// Repeat or Volume Control
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			}
			// End of [#2375]
		}
		Delay_Msg(50);
		nCheckCount++;
	}
	m_pAdaCtrl->fnExp_StopPlay();
	return RES_USER_EXIT;
}

/*------------------------------------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_CA_ADA_Language()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
---------------------------------------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_CA_ADA_Language()
{
	NVDump('O', 'C', "00", L"P_CAADA", L"SEL_Language");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_CA_ADA_Language]\n"));

	// [#2205] US KSK 2013.06.28
	// [#604] NH KSK 2010.01.05 DEV_MCU는 fnAPD_CardEnDisable에서 ON하므로 DEV_MCU는 삭제함
	// [#2333] NH KSK 2015.03.06 Always인 경우 EPP가 On상태이므로 여기서 추가로 On 필요없음 주석처리함 (확인 필요)
//	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
//	{
//		if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_FLICKER_OPTION) == 0)	// ALWAY 인 경우에만 FLICKER ON
//			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_PIN, FLICKER_ON);
//	}
	// end of [#604]
	// end of [#2205]
	

	//[#2077] [CA] PCS 2011.06.23
	//*-------------------STATE 1----------------------------------------------------------------------
	//언어선택 후 거래진행을 위해서 언어선택 입력을 받는 화면
	//스크린 엔진에서 timeout을 주지 않기 때문에 AP에서 시간을 계산하는 로직이 필요
	//1번 안내 방송을 내보낸 후에 60초 동안 선택이 없으면 디폴트 값인 영어를 선택하고 다음 스테이트로 넘어간다. 
	{
		
		//오디오가 연결되어 있지 않으면 바로 거래 중지.
		if((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			return RES_USER_EXIT;
		}

		// Show Screen
		{
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
		}

		//언어선택 화면과 음성파일 재생
		NHDEBUG(DBG_INFO, (_T("PLAY ADA WAVE FILE FOR LANGUAGE SELECTIOM\n")));
		m_pAdaCtrl->fnExp_ResetScrWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(102,L"102_1.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(102);	//[#2077] CA PCS 2011.07.05 언어선택 wave 추가.

		g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_TIMEOUT);
		while(g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			//오디오가 연결되어 있지 않으면 바로 거래 중지.
			if((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				return RES_USER_EXIT;
			}

			//사용자 PIN 입력(2:영어 4:불어)값을 확인한다.
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				CString GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("Get Key String [%s]\n"), GetKeyStr));

				if (GetKeyStr == L"2")
				{
					m_pAdaCtrl->fnExp_SetLanguage(ENG_MODE);
					return RES_OK;
				}
				else if (GetKeyStr == L"4")
				{
					m_pAdaCtrl->fnExp_SetLanguage(FRN_MODE); 
					return RES_OK;
				}
				else if (GetKeyStr == S_CANCEL|| GetKeyStr == S_EXIT)
				{
					return RES_USER_EXIT;
				}

			}
			Delay_Msg(50);
		}
	}

	NHDEBUG(DBG_INFO, (_T("CHOOSE KEY AS ENGLISH BY TIME OUT\n")));
	m_pAdaCtrl->fnExp_SetLanguage(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE));	// Timeout시 Default Language로 진행하도록 사양협의함

	return RES_OK;

}//end of [#2077]

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_EnterPassword()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ADA_EnterPassword()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"EnterPassword");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_EnterPassword]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	int				nInputCount;
	BOOL			bStartToTimeout = FALSE; //[#2118] US PCS 2012.01.16

	m_bAdaPasswordMode = TRUE;

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			m_bAdaPasswordMode = FALSE;
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_75");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, KEYIN_TIME_OUT, PIN_PASSWORD_MODE, L"", PIN_PASSWORD_MIN, PIN_PASSWORD_MAX, PIN_PASSWORD_AUTO_TRUE, PIN_PASSWORD_TERM);//[#2077] CA PCS 2011.07.10				
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();

				//[#2104] US PCS 2012.01.17 "입력 키 위치에 대한 설명 추가"
#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(103,L"LocateEnterKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(103,L"LocateCancelKey.wav");
#endif
				//end of [#2104]

				m_pAdaCtrl->fnExp_PlayScreenWave(103);
			}

			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE;//[#2118] NH PCS 2012.01.16
			nInputCount = 0;
		}
		///////////////////////////////////
	
		//[#2118] NH PCS 2012 01.06 "타임아웃 시작점을 음성 종료 시로 설정함.
		if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				m_bAdaPasswordMode = FALSE;
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_76");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_CLEAR)
			{
				nInputCount = 0;
			}
			else if (GetKeyStr == S_ENTER)
			{
				m_bAdaPasswordMode = FALSE;
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_PIN);	// Read Complete가 올때까지 대기
				m_pAdaCtrl->fnExp_WaitUntilStop();				// [#2311] US Justin 2014.11.18 Wait until End of Playing

				// [#GLDV-2853] AU Kook 2021.03.23
				if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_ENABLE) == ENABLE)
				{
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_MEMORIZE_ENABLE) == ENABLE)
					{
#ifdef UNDER_CE
						// use journal index to be used in this case. (+1)
						m_pDevCmn->fnCAM_CaptureFace(((Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SEQUENCENO)) % 9999) + 1), TIME_PIN_ENTERED);
#endif
					}
				}
				// end of [#GLDV-2853]

				return RES_OK;
			}
			else if (GetKeyStr.GetLength() == 1)
			{
				nInputCount++;
				if (nInputCount == 12)
					m_pDevCmn->fnSCR_SetByPassData(S_ENTER);
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	m_bAdaPasswordMode = FALSE;
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_77");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}


// [#2375] US Justin 2015.10.30 Disable un used prc for US
#if !(US_VERSION || CA_VERSION)
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_ReadCard()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_ReadCard(int nEventKind)
{
	NVDump('O', 'C', "00", L"P_NHADA", L"ReadCard");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_ReadCard]\n"));

	BIZ_RETURN	nRes;

	// KSK 2010.04.11 Initialize variant
	m_sCardData.strISO1Data = L"";
	m_sCardData.strISO2Data = L"";
	m_sCardData.strISO3Data = L"";

	m_sCardData.strBankID = L"";
	m_sCardData.strAccountNo = L"";
	m_sCardData.strShowNumber = L"";
	// end of KSK
	m_nEventKind = nEventKind;

	// [#2325] NH KSK 2015.01.22
	// Parse Data
//	nRes = LIB_CheckCardRead(m_pDevCmn->fstrMCU_GetCardData());
	if (m_nEventKind == DEV_RFID)
		nRes = LIB_CheckCardRead(m_pDevCmn->fstrRFID_GetCardData());
	else
		nRes = LIB_CheckCardRead(m_pDevCmn->fstrMCU_GetCardData());
	// end of [#2325]

	if (nRes != RES_OK)
	{
		if(m_pDevCmn->fnSNS_GetEnhancedAudio())
		{
			m_pAdaCtrl->fnExp_ResetScrWaveFile();
			m_pAdaCtrl->fnExp_PlayScreenWave(5);
			m_pAdaCtrl->fnExp_WaitUntilStop();
		}
		
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_74");
			NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_ReadCard]\n")); // [#2024] NH KSK 2011.02.24
	}

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_SelectTransaction()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Transaction
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_SelectTransaction()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"SelectTran.");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_SelectTransaction]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2118] US PCS 2012.01.16

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();

				// Withdrawal
				if (m_pDevCmn->WithAvail != TRAN_WITH_NOT)
				{
					NHDEBUG(DBG_INFO, (_T("WITHDRAWAL NOT AVAILABLE\n")));
					m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press2ForWith.wav");
				}

				m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press4ForInq.wav");

// [#2047] AU KSK 2011.04.13
#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(105, L"Press6ForTrans.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(105, L"LocateCancelKey.wav");	// [#2104] US PCS 2012.01.09 " CardTronics 요청 사항 - Cancel Key위치 음성 추가 - "
				m_pAdaCtrl->fnExp_AddWaveFile(105, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
// end of [#2047]
				m_pAdaCtrl->fnExp_PlayScreenWave(105);
			}		
			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE;//[#2118] NH PCS 2012.01.16
		}

		//[#2118] US PCS 2012 01.06 "타임아웃 시작점을 음성 종료 시로 설정함.
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		//////////////////////////////////////////////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_79");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if ((GetKeyStr == L"2") && (m_pDevCmn->WithAvail != TRAN_WITH_NOT))
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				TranCode = TC_WITHDRAWAL;
				return RES_OK;
			}
			else if (GetKeyStr == L"4")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				TranCode = TC_INQUIRY;
				return RES_OK;
			}
#if (US_VERSION)	// [#2047] AU KSK 2011.04.13
			else if (GetKeyStr == L"6")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				TranCode = TC_TRANSFER;
				return RES_OK;
			}
#endif				// end of [#2047]

			////////////////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			///////////////////////////////////////////////////////////////////
			else
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
			}
		}
		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_80");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_DisplaySurcharge()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Display Surcharge
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_DisplaySurcharge(int nSurchargeOption)
{
	NVDump('O', 'C', "00", L"P_NHADA", L"ShowSurcharge");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_DisplaySurcharge]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2118] US PCS 2012.01.16

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_81");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Get Surcharge Amount
			NHDEBUG(DBG_INFO, (_T("nSurchargeOption[%d]\n"),nSurchargeOption));
			CString strChargeAmount = L"";
			switch (nSurchargeOption)
			{
				case SURCHARGE_DISP_NORMAL:
					strChargeAmount.Format(_T("%s"), MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)));
					break;
				case SURCHARGE_DISP_PRECENT:
					{
						//float fPercentSurcharge = (float)(Asc2Int(m_sUserSelection.strMoney) * 0.01) * (float)(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_PERCENT) * 0.01);
						float fPercentSurcharge = (float)(Asc2Int(m_sUserSelection.strMoney) * 0.01) * (float)(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DECIMAL_SURCHARGE) * 0.0001);	// [#2316] US Justin 2014.12.17 Support Decimal Percentage surcharge

						float fResultSurcharge = 0;
						if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_PERSURCHARGE_MANNER) == GREATER)
							fResultSurcharge = max(fPercentSurcharge, (float)(Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT))*0.01));
						else
							fResultSurcharge = min(fPercentSurcharge, (float)(Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT))*0.01));
						strChargeAmount.Format(L"%.2f", fResultSurcharge);
					}
					break;
				case SURCHARGE_DISP_TABLE:
					{
						int nRequestAmtTemp = Asc2Int(m_sUserSelection.strMoney, 12) / 100;
						int i=0;
						for (i=0; i < 10; i++)
						{
							if (nRequestAmtTemp <= m_pDevCmn->m_nSurchTable[0][i])
							{
								m_pDevCmn->m_strTableSurcharge.Format(L"%d", m_pDevCmn->m_nSurchTable[1][i]);
								break;
							}
						}
						if (i == 10)	// Maximum Value
							m_pDevCmn->m_strTableSurcharge.Format(L"%d", m_pDevCmn->m_nSurchTable[1][9]);

						strChargeAmount.Format(_T("%s"), MakeMoneyCent(m_pDevCmn->m_strTableSurcharge));		
					}
					break;
				case SURCHARGE_DISP_TDL_SEGMENTATION:
				case SURCHARGE_DISP_TDL_DCC:							// [#2365] US Justin 2015.07.24 Support DCC Surcharge on VG Mode
					strChargeAmount.Format( _T("%s"), MakeMoneyCent(m_STD3_TDL_Data.m_strSurchargeAmount) );
					break;
			}

			// [#2311] US Justin 2014.11.18 TDL Dynamic Surcharge
			#if(APP_TDL_OPTION)
				if( ((nSurchargeOption==SURCHARGE_DISP_NORMAL)||(nSurchargeOption==SURCHARGE_DISP_PRECENT)||(nSurchargeOption==SURCHARGE_DISP_TABLE)||(nSurchargeOption==SURCHARGE_DISP_TDL_SEGMENTATION))&&
					( m_SourceAccount == S_CREDITCARD ) )
				{
					int nNewSurcharge = Asc2Int(strChargeAmount) + Asc2Int( MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CUSTOMIZED_CREDITSURCHARGE) );
					strChargeAmount.Format(L"%0.2f", 0.01*nNewSurcharge);
				}
			#endif
			// End of [#2311]
			
			// Assign Shown Surcharge Amount
			m_nDisplayedSurchargeAmount = Asc2Int(strChargeAmount);

			// [#2311] US Justin ADA Surcharge VG... Remove BEFORE PERCENT. Make Common Routine
			// Play voice Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(201, strChargeAmount);
				m_pAdaCtrl->fnExp_AddWaveFile(201, L"201_1.wav");  //"This fee is in addition...

				#if(US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(201);
			}
			// End of [#2311]

			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEDISPLAY, 1);
			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
		}
		
		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_82");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == L"7")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				return RES_OK;
			}
			else if (GetKeyStr == L"8")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_83");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			//////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			/////////////////////////////////////////////////////////
			else
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_84");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_ReceiveReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Receive Receipt
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_ReceiveReceipt()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"ReceiveReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_ReceiveReceipt]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2118] US PCS 2012.01.16

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_85");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				// [#2104] US PCS 2012.01.09 " CardTronics 요청 사항 - Cancel Key위치 음성 추가 - "
#if(US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(106, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(106, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
				//end of [#2104]
				m_pAdaCtrl->fnExp_PlayScreenWave(106);
			}

			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_86");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == L"2")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_sUserSelection.nPrintReceipt = RCPT_PAPER;		// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
				return RES_OK;
			}
			else if (GetKeyStr == L"4")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_sUserSelection.nPrintReceipt = FALSE;				// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
				return RES_OK;
			}
			//////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			/////////////////////////////////////////////////////////
			else
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_87");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_ErrorReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Receive Receipt
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_ErrorReceipt()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"ErrorReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_ErrorReceipt]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2118] US PCS 2012.01.16


	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_88");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				// [#2104] US PCS 2012.01.09 " CardTronics 요청 사항 - Cancel Key위치 음성 추가 - "
#if(US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(116, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(116, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
				//end of [#2104]
				m_pAdaCtrl->fnExp_PlayScreenWave(116);
			}

			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_89");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == L"2")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_sUserSelection.nPrintReceipt = FALSE;					// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name
				return RES_OK;
			}
			else if (GetKeyStr == L"4")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_90");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			//////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			/////////////////////////////////////////////////////////
			else
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_91");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_SelectCWAccount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Cash Withdrawal Account
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_SelectCWAccount()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"SelectCWAccount");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_SelectCWAccount]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2118] US PCS 2012.01.16

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_92");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))
					m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press2ForChecking.wav");	// Checking

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))
					m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press4ForSavings.wav");	// Savings

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))
					m_pAdaCtrl->fnExp_AddWaveFile(202, L"Press6ForCredit.wav");		// CreditCard
#if(US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(202, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(202, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(202);
			}

			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_93");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if ((GetKeyStr == L"2") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING)))
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_SourceAccount = S_CHECKING;
				return RES_OK;
			}
			else if ((GetKeyStr == L"4") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS)))
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_SourceAccount = S_SAVINGS;
				return RES_OK;
			}
			else if ((GetKeyStr == L"6") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD)))
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_SourceAccount = S_CREDITCARD;
				return RES_OK;
			}
			//////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			/////////////////////////////////////////////////////////
			else
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_94");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_SelectBIAccount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Balance Inquiry Account
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_SelectBIAccount()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"SelectBIAccount");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_SelectBIAccount]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2118] US PCS 2012.01.16

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_95");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))
					m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press2ForChecking.wav");	// Checking

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))
					m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press4ForSavings.wav");	// Savings

				if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))
					m_pAdaCtrl->fnExp_AddWaveFile(301, L"Press6ForCredit.wav");		// CreditCard
#if(US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(301, L"LocateCancelKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(301, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(301);
			}
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
			bPlayGuidance = FALSE;
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_96");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if ((GetKeyStr == L"2") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING)))
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_SourceAccount = S_CHECKING;
				return RES_OK;
			}
			else if ((GetKeyStr == L"4") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS)))
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_SourceAccount = S_SAVINGS;
				return RES_OK;
			}
			else if ((GetKeyStr == L"6") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD)))
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				m_SourceAccount = S_CREDITCARD;
				return RES_OK;
			}
			//////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			/////////////////////////////////////////////////////////
			else
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_97");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_SelectTRAccount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Select Transfer Account
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_SelectTRAccount()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_SelectTRAccount]\n"));

	BOOL			bPlayGuidance = TRUE;
	BOOL			bInputToAccount = FALSE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2118] NH PCS 2012.01.16
	///////////////////////////////////
	// 1. From Account
	{
		NVDump('O', 'C', "00", L"P_NHADA", L"FromAccount");
		bPlayGuidance = TRUE;

		g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
		while(g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			///////////////////////////////////
			// CHECK STATUS
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_98");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}

			///////////////////////////////////
			// SHOW SCREEN & PLAY GUIDANCE
			if (bPlayGuidance == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

				// Show Screen
				{
					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				}

				// Play Guidance
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();

					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING))
						m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press2ForChecking.wav");	// Checking

					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS))
						m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press4ForSavings.wav");	// Savings

					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD))
						m_pAdaCtrl->fnExp_AddWaveFile(402, L"Press6ForCredit.wav");		// CreditCard
					//[#2104] US PCS 2012.01.09 
#if(US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(402, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(402, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
					//end of [#2104]
					m_pAdaCtrl->fnExp_PlayScreenWave(402);
				}

				bPlayGuidance = FALSE;
				bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
			}

			//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
			if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
			{
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			//end of [#2118]

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_99");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if ((GetKeyStr == L"2") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING)))
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					bInputToAccount = TRUE;
					m_SourceAccount = S_CHECKING;
					break;
				}
				else if ((GetKeyStr == L"4") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS)))
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					bInputToAccount = TRUE;
					m_SourceAccount = S_SAVINGS;
					break;
				}
				else if ((GetKeyStr == L"6") && (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD)))
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					bInputToAccount = TRUE;
					m_SourceAccount = S_CREDITCARD;
					break;
				}
				//////////////////////////////////////////////////////////
				//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
				else if (GetKeyStr == L".")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeDown();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
				else if (GetKeyStr == L"00")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeUp();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
#endif
				//end of [#2101],[#2103]
				/////////////////////////////////////////////////////////
				else
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
				}
			}
			Delay_Msg(50);
		}
		if (bInputToAccount == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_100");	// [#2024] NH KSK 2011.02.24
			return RES_USER_TIMEOUT;
		}
	}

	///////////////////////////////////
	// 2. To Accont
	{
		NVDump('O', 'C', "00", L"P_NHADA", L"ToAccount");
		bPlayGuidance = TRUE;

		BOOL	bEnableChecking = FALSE;
		BOOL	bEnableSaving = FALSE;
		BOOL	bEnableCredit = FALSE;
		bStartToTimeout = FALSE; //[#2118] NH PCS 2012.01.16

		g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
		while(g_TimeCheck.IsElapsedTimes() == FALSE)
		{
			///////////////////////////////////
			// CHECK STATUS
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_101");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}

			///////////////////////////////////
			// SHOW SCREEN & PLAY GUIDANCE
			if (bPlayGuidance == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

				// Show Screen
				{
					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				}

				// Play Guidance
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();

					if (m_SourceAccount == S_CHECKING)
					{
						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS) == ENABLE)
						{
							bEnableSaving = TRUE;
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press4ForSavings.wav");	// Savings
						}

						if ((MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD) == ENABLE) &&
							(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE))
						{
							bEnableCredit = TRUE;
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press6ForCredit.wav");		// CreditCard
						}
					}
					else if (m_SourceAccount == S_SAVINGS)
					{
						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING) == ENABLE)
						{
							bEnableChecking = TRUE;
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press2ForChecking.wav");	// Checking
						}

						if ((MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CREDITCARD) == ENABLE) &&
							(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE))
						{
							bEnableCredit = TRUE;
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press6ForCredit.wav");		// CreditCard
						}
					}
					else if (m_SourceAccount == S_CREDITCARD)
					{
						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_CHECKING) == ENABLE)
						{
							bEnableChecking = TRUE;
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press2ForChecking.wav");	// Checking
						}

						if ((MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ACCOUNT_SAVINGS) == ENABLE) &&
							(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) != MSG_TRITON_TYPE))
						{
							bEnableSaving = TRUE;
							m_pAdaCtrl->fnExp_AddWaveFile(403, L"Press4ForSavings.wav");	// Savings
						}
					}
					//[#2104] US PCS 2012.01.09 
#if(US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(403, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(402, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
					//end of [#2104]

					m_pAdaCtrl->fnExp_PlayScreenWave(403);
				}

				bPlayGuidance = FALSE;
				bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
			}

			//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
			if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
			{
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			//end of [#2118]

			///////////////////////////////////
			// GET KEY STRING

			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_102");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if ((GetKeyStr == L"2") && (bEnableChecking == TRUE))
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					m_DestAccount = S_CHECKING;
					return RES_OK;
				}
				else if ((GetKeyStr == L"4") && (bEnableSaving == TRUE))
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					m_DestAccount = S_SAVINGS;
					return RES_OK;
				}
				else if ((GetKeyStr == L"6") && (bEnableCredit == TRUE))
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					m_DestAccount = S_CREDITCARD;
					return RES_OK;
				}
				//////////////////////////////////////////////////////////
				//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
				else if (GetKeyStr == L".")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeDown();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
				else if (GetKeyStr == L"00")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeUp();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
#endif
				//end of [#2101],[#2103]
				/////////////////////////////////////////////////////////
				else
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
				}
			}

			Delay_Msg(50);
		}

		NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
		NVDump('O', 'C', "00", L"P_NHADA", L"NG_103");	// [#2024] NH KSK 2011.02.24
		return RES_USER_TIMEOUT;
	}

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_104");	// [#2024] NH KSK 2011.02.24
	return RES_USER_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_SelectCWAccount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Input Cash Withdrawal Amount
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_InputCWAmount(BOOL bUseFastCash)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_InputCWAmount]\n"));

	int				i;
	int				nInputAmount;
	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	CString			strFastCash, strAddWaveFile;
	BOOL			bOtherAmountRoof = FALSE;
	int				nRetryCount = 3;	// [#2006] NH KSK 2010.11.29
	BIZ_RETURN		nRes; //[#2114] NH PCS 2012.01.10
	BOOL			bStartToTimeout = FALSE; //[#2118] NH PCS 2012.01.16

	///////////////////////////////////
	// 1. Fast Cash
	{
		NVDump('O', 'C', "00", L"P_NHADA", L"FastCash");
		bPlayGuidance = TRUE;

		g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
		while((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))		// [#2006] NH KSK 2010.11.29
		{
			///////////////////////////////////
			// CHECK STATUS
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
				{
					NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_105");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}

				if (m_pDevCmn->fnAPL_GetAvailTrans() == TRAN_WITH_NOT)
				{
					NHDEBUG(DBG_INFO, (_T("WITHDRAWAL NOT AVAILABLE\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_106");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
			}

			///////////////////////////////////
			// SHOW SCREEN & PLAY GUIDANCE
			if (bPlayGuidance == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

				// m_nFastCaseNo
				// 0 : Fast Cash Available
				// 1 : Fast Cash Unavailable, Input Other Amount
				// -1 : Unable to dispense
				m_pDevCmn->fnCDU_GetCashDispenseInfo();	// [#485] [NH] KSK 2009.1.14

				// Show Screen
				{
					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				}

				// Play Guidance
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();

					for (i=0; i<CDU_CST_MAXFASTCASH; i++)		//Button On/Off 처리
					{
						if (m_pDevCmn->m_nFastCash[i])
						{
							strAddWaveFile.Format(L"Press%dFor.wav", i+1);
							strFastCash.Format(L"%d", m_pDevCmn->m_nFastCash[i]);
							m_pAdaCtrl->fnExp_AddWaveFile(203, strAddWaveFile);
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(203, strFastCash);
						}
					}

					m_pAdaCtrl->fnExp_AddWaveFile(203, L"OtherAmt.wav");
					//[#2104] US PCS 2012.01.09 
#if(US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(203, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(203, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
					//end of [#2104]
					m_pAdaCtrl->fnExp_PlayScreenWave(203);
				}

				bPlayGuidance = FALSE;
				bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
			}

			//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
			if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
			{
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			//end of [#2118]

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_107");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}

				else if (GetKeyStr == L"7")
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
					bOtherAmountRoof = TRUE;
					break;
				}
				else if (GetKeyStr == L"1" || GetKeyStr == L"2" || GetKeyStr == L"3" ||
						 GetKeyStr == L"4" || GetKeyStr == L"5" || GetKeyStr == L"6")
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////

					// [#2047] AU KJW 2011.04.20 "6" 눌렀을 때, 거래진행 안되는 현상 수정
					//if (m_pDevCmn->m_nFastCash[Asc2Int(GetKeyStr)])

					// [#2022][#2129] NH KSK 2011.02.21 / 2012.03.19 Code Sonar 지적사항 대책
					if (Asc2Int(GetKeyStr)-1 < 0)
					{
						NVDump('O', 'C', "00", L"P_NHADA", L"NG_108");	// [#2024] NH KSK 2011.02.24
						return RES_USER_EXIT;
					}
					// end of [#2022][#2129]

					if (m_pDevCmn->m_nFastCash[Asc2Int(GetKeyStr)-1]) 
					// end of [#2047]
					{
						nInputAmount = m_pDevCmn->m_nFastCash[Asc2Int(GetKeyStr)-1];
						strFastCash.Format(L"%d", nInputAmount);

						NHDEBUG(DBG_INFO, (_T("REMOVE SYMBOL CHAR - VALUE [%s] - [%d], [%s]\n"), GetKeyStr, nInputAmount, strFastCash));

						////////////////////////////////////////////////////////////////////////////////////////
						//[#2114] NH PCS 2012.01.18 "입력 금액 확인 전 입력 금액에 대한 유효성 검사."
						if (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
						{
							m_pAdaCtrl->fnExp_StopPlay();
							m_pAdaCtrl->fnExp_ResetScrWaveFile();
							m_pAdaCtrl->fnExp_AddWaveFile(1, L"Indispensable.wav");
							m_pAdaCtrl->fnExp_PlayScreenWave(1);
							m_pAdaCtrl->fnExp_WaitUntilStop();

							NVDump('O', 'C', "00", L"P_NHADA", L"NG_109");	// [#2024] NH KSK 2011.02.24
							return RES_CASH_NOT_AVAILABLE;

						}
						else if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
						{
							m_pAdaCtrl->fnExp_StopPlay();
							m_pAdaCtrl->fnExp_ResetScrWaveFile();
							m_pAdaCtrl->fnExp_AddWaveFile(1, L"MaxAmt.wav");
							m_pAdaCtrl->fnExp_PlayScreenWave(1);
							m_pAdaCtrl->fnExp_WaitUntilStop();

							NVDump('O', 'C', "00", L"P_NHADA", L"NG_110");	// [#2024] NH KSK 2011.02.24
							return RES_CASH_MAX_OVER;
						}
						else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
						{
							m_pAdaCtrl->fnExp_StopPlay();
							m_pAdaCtrl->fnExp_ResetScrWaveFile();
							m_pAdaCtrl->fnExp_AddWaveFile(1, L"LowAmt.wav");
							m_pAdaCtrl->fnExp_PlayScreenWave(1);
							m_pAdaCtrl->fnExp_WaitUntilStop();

							NVDump('O', 'C', "00", L"P_NHADA", L"NG_111");	// [#2024] NH KSK 2011.02.24
							return RES_CASH_MIN_OVER;
						}//end of [#2114]

						////////////////////////////////////////////////////////////////////////////////////////
						//[#2114] NH PCS 2012.01.10 "금액입력 받는 프록시저에 의한 리턴값을 따라서 분기 처리함.
						nRes = P_NH_ADA_ConfirmAmt(nInputAmount);

						if (nRes == RES_USER_PREV)
						{
							nRetryCount--;
							bPlayGuidance = TRUE;
						}
						else
						{
							if (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
							{
								m_pAdaCtrl->fnExp_StopPlay();
								m_pAdaCtrl->fnExp_ResetScrWaveFile();
								m_pAdaCtrl->fnExp_AddWaveFile(1, L"Indispensable.wav");
								m_pAdaCtrl->fnExp_PlayScreenWave(1);
								m_pAdaCtrl->fnExp_WaitUntilStop();

								NVDump('O', 'C', "00", L"P_NHADA", L"NG_109");	// [#2024] NH KSK 2011.02.24
								return RES_CASH_NOT_AVAILABLE;
							}
							else if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
							{
								m_pAdaCtrl->fnExp_StopPlay();
								m_pAdaCtrl->fnExp_ResetScrWaveFile();
								m_pAdaCtrl->fnExp_AddWaveFile(1, L"MaxAmt.wav");
								m_pAdaCtrl->fnExp_PlayScreenWave(1);
								m_pAdaCtrl->fnExp_WaitUntilStop();

								NVDump('O', 'C', "00", L"P_NHADA", L"NG_110");	// [#2024] NH KSK 2011.02.24
								return RES_CASH_MAX_OVER;
							}
							else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
							{
								m_pAdaCtrl->fnExp_StopPlay();
								m_pAdaCtrl->fnExp_ResetScrWaveFile();
								m_pAdaCtrl->fnExp_AddWaveFile(1, L"LowAmt.wav");
								m_pAdaCtrl->fnExp_PlayScreenWave(1);
								m_pAdaCtrl->fnExp_WaitUntilStop();

								NVDump('O', 'C', "00", L"P_NHADA", L"NG_111");	// [#2024] NH KSK 2011.02.24
								return RES_CASH_MIN_OVER;
							}
							else
								return nRes;
						}
						//end of [#2114]
						///////////////////////////////////////////////////////////////////////////////////////
					}
				}

				//////////////////////////////////////////////////////////
				//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
				else if (GetKeyStr == L".")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeDown();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
				else if (GetKeyStr == L"00")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeUp();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
#endif
				//end of [#2101],[#2103]
				/////////////////////////////////////////////////////////
				else
				{
					////////////////////////////////////////////////////////////////
					//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
					CString strTemp;
					strTemp.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
					//end of [#2117][#2121]
					/////////////////////////////////////////////////////////////////
				}
			}

			Delay_Msg(50);
		}

		if (nRetryCount <= 0)
		{
			NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_114");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}
		else if (bOtherAmountRoof == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_115");	// [#2024] NH KSK 2011.02.24
			return RES_USER_TIMEOUT;
		}
	}

	///////////////////////////////////
	// 2. Other Amount
	{
		NVDump('O', 'C', "00", L"P_NHADA", L"OtherAmount");

//		int				nRetryCount = 3;
		CStringArray	strtmp_ArrayAmount;
		CString			strtmp_Amount;
		CString			strInputAmount;
//		BOOL			bConfirmState = FALSE;
		int				nCashDenomination[8] = {CASH_DENOMINATION1, CASH_DENOMINATION2, CASH_DENOMINATION3, CASH_DENOMINATION4, 
												CASH_DENOMINATION5, CASH_DENOMINATION6, CASH_DENOMINATION7, CASH_DENOMINATION8};

		nRetryCount = 3;	// Retry Count Initialize
		bPlayGuidance = TRUE;
		bStartToTimeout = FALSE; //[#2118] NH PCS 2012.01.16

#if (US_VERSION)
		m_bAdaSuccessiveInput = TRUE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif

		g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
		while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
		{
			///////////////////////////////////
			// CHECK STATUS
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
				{
					NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_116");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}

				if (m_pDevCmn->fnAPL_GetAvailTrans() == TRAN_WITH_NOT)
				{
					NHDEBUG(DBG_INFO, (_T("WITHDRAWAL NOT AVAILABLE\n")));
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_117");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
			}

			///////////////////////////////////
			// SHOW SCREEN & PLAY GUIDANCE
			if (bPlayGuidance == TRUE)
			{
				NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

				// m_nFastCaseNo
				// 0 : Fast Cash Available
				// 1 : Fast Cash Unavailable, Input Other Amount
				// -1 : Unable to dispense
				m_pDevCmn->fnCDU_GetCashDispenseInfo();	// [#485] [NH] KSK 2009.1.14

				strtmp_ArrayAmount.RemoveAll();

				for(i=0; i<8; i++)
				{
					if (m_pDevCmn->WithAvail & (DENOMINATION_BIT1<<i))
					{
						strtmp_Amount.Format(L"%d", nCashDenomination[i]);

						strtmp_ArrayAmount.Add(strtmp_Amount);
					}
				}

				// Show Screen
				{
					m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
					m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
				}

				// Play Guidance
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMultipleOf.wav");

					if (strtmp_ArrayAmount.GetSize() > 0)	// KSK 2011.04.28 출금 불가일 경우에 대한 예외처리 추가
					{
						m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[0]);

						// KSK 2011.4.28 Multi Cassette일 경우 모든 권종을 표시하도록 추가 (호주 Customers요청에 의함) - 전체 적용 시 wave file 추가해야함
						#if (AU_VERSION)
						for(int i=1; i<m_pDevCmn->CSTCnt; i++)
						{
							if (strtmp_ArrayAmount.GetSize() > i)
							{
								m_pAdaCtrl->fnExp_AddWaveFile(204, L"and.wav");
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_ArrayAmount[i]);
							}
						}
						#endif
						// end of KSK 2011.04.28
					}										// end of KSK 2011.04.28
				
					m_pAdaCtrl->fnExp_AddWaveFile(204, L"AmtMax.wav");
					strtmp_Amount.Format(L"%d", m_pDevCmn->m_MoneyMaxOut);
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(204, strtmp_Amount);
					//[#2104] US PCS 2012.01.09 " CardTronics 요청 사항 - Enter/Cancel 키 위치 -"
#if (US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(204,L"LocateEnterKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(204,L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(204,L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
					//end of [#2104]
					m_pAdaCtrl->fnExp_PlayScreenWave(204);
				}

				strInputAmount = L"";
				bPlayGuidance = FALSE;
				bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
			}

			//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
			if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
			{
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
				bStartToTimeout = FALSE;
			}
			//end of [#2118]

			///////////////////////////////////
			// GET KEY STRING
			if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
			{
				GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
				GetKeyStr = GetKeyStr.Mid(6);

				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

				if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
				{
					NVDump('O', 'C', "00", L"P_NHADA", L"NG_118");	// [#2024] NH KSK 2011.02.24
					return RES_USER_EXIT;
				}
				else if (GetKeyStr == S_CLEAR)
				{
					strInputAmount = L"";
				}
				else if (GetKeyStr == S_ENTER)
				{
					NHDEBUG(DBG_INFO, (_T("INPUT AMOUNT - VALUE [%s]\n"), strInputAmount));

					if (strInputAmount.GetLength() > 0)
					{
						nInputAmount = Asc2Int(strInputAmount);
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2249] us	Justin Bug(Time out of Incorrect Amount) 2014.01.10

#if (US_VERSION)
						m_bAdaSuccessiveInput = FALSE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif
						////////////////////////////////////////////////////////////////////////////////////////
						//[#2114] NH PCS 2012.01.18 "입력 값에 대한 유효성 검사 추가."
						if (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
						{
							m_pAdaCtrl->fnExp_StopPlay();
							m_pAdaCtrl->fnExp_ResetScrWaveFile();
							m_pAdaCtrl->fnExp_AddWaveFile(1, L"Indispensable.wav");
							m_pAdaCtrl->fnExp_PlayScreenWave(1);
							m_pAdaCtrl->fnExp_WaitUntilStop();

							bPlayGuidance = TRUE;
							nRetryCount--;
						}
						else if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
						{
							m_pAdaCtrl->fnExp_StopPlay();
							m_pAdaCtrl->fnExp_ResetScrWaveFile();
							m_pAdaCtrl->fnExp_AddWaveFile(1, L"MaxAmt.wav");
							m_pAdaCtrl->fnExp_PlayScreenWave(1);
							m_pAdaCtrl->fnExp_WaitUntilStop();

							bPlayGuidance = TRUE;
							nRetryCount--;
						}
						else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
						{
							m_pAdaCtrl->fnExp_StopPlay();
							m_pAdaCtrl->fnExp_ResetScrWaveFile();
							m_pAdaCtrl->fnExp_AddWaveFile(1, L"LowAmt.wav");
							m_pAdaCtrl->fnExp_PlayScreenWave(1);
							m_pAdaCtrl->fnExp_WaitUntilStop();

							bPlayGuidance = TRUE;
							nRetryCount--;
						}
						////////////////////////////////////////////////////////////////////////////////////////
						//[#2114] NH PCS 2012.01.10 "금액입력 받는 프록시저에 의한 리턴값을 따라서 분기 처리함.
						else
						{
							nRes = P_NH_ADA_ConfirmAmt(nInputAmount);
							if (nRes == RES_USER_PREV)
							{
								nRetryCount--;
								bPlayGuidance = TRUE;
							}
							else
							{
								g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2249] us	Justin Bug(Time out of Incorrect Amount) 2014.01.10
								if (!m_pDevCmn->fbCDU_IsDispensible(nInputAmount))
								{
									m_pAdaCtrl->fnExp_StopPlay();
									m_pAdaCtrl->fnExp_ResetScrWaveFile();
									m_pAdaCtrl->fnExp_AddWaveFile(1, L"Indispensable.wav");
									m_pAdaCtrl->fnExp_PlayScreenWave(1);
									m_pAdaCtrl->fnExp_WaitUntilStop();

									bPlayGuidance = TRUE;
									nRetryCount--;
								}
								else if (nInputAmount > m_pDevCmn->m_MoneyMaxOut)
								{
									m_pAdaCtrl->fnExp_StopPlay();
									m_pAdaCtrl->fnExp_ResetScrWaveFile();
									m_pAdaCtrl->fnExp_AddWaveFile(1, L"MaxAmt.wav");
									m_pAdaCtrl->fnExp_PlayScreenWave(1);
									m_pAdaCtrl->fnExp_WaitUntilStop();

									bPlayGuidance = TRUE;
									nRetryCount--;
								}
								else if (nInputAmount < m_pDevCmn->m_MoneyMinOut)
								{
									m_pAdaCtrl->fnExp_StopPlay();
									m_pAdaCtrl->fnExp_ResetScrWaveFile();
									m_pAdaCtrl->fnExp_AddWaveFile(1, L"LowAmt.wav");
									m_pAdaCtrl->fnExp_PlayScreenWave(1);
									m_pAdaCtrl->fnExp_WaitUntilStop();

									bPlayGuidance = TRUE;
									nRetryCount--;
								}
								else
									return nRes;
							}
						}

						//end of [#2114]
						///////////////////////////////////////////////////////////////////////////////////////
#if (US_VERSION)
						m_bAdaSuccessiveInput = TRUE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif
					}
					else
					{
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2249] us	Justin Bug(Time out of Incorrect Amount) 2014.01.10
						m_pAdaCtrl->fnExp_StopPlay();
						m_pAdaCtrl->fnExp_ResetScrWaveFile();
						m_pAdaCtrl->fnExp_AddWaveFile(1, L"Indispensable.wav");
						m_pAdaCtrl->fnExp_PlayScreenWave(1);
						m_pAdaCtrl->fnExp_WaitUntilStop();
						bPlayGuidance = TRUE;
						nRetryCount--;
					}					
				}
				else if ((GetKeyStr.GetLength() == 1) && (GetKeyStr != L".")) //[#2114] NH PCS 2012.01.10 
				{
					strInputAmount += GetKeyStr;
				}
				/////////////////////////////////////////////////////////////////////////////////
				//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
				else if (GetKeyStr == L".")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeDown();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
				else if (GetKeyStr == L"00")
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_SetVolumeUp();
					bPlayGuidance = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
#endif
				//end of [#2101],[#2103]
				////////////////////////////////////////////////////////////////////////////////
			}

			Delay_Msg(50);
		}

		if (nRetryCount <= 0)
		{
			NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_119");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}
		else
		{
			NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_120");	// [#2024] NH KSK 2011.02.24
			return RES_USER_TIMEOUT;
		}
#if (US_VERSION)
		m_bAdaSuccessiveInput = FALSE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif
	}

#if (US_VERSION)
	m_bAdaSuccessiveInput = FALSE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif

	NHDEBUG(DBG_INFO, (_T("ABNORMAL PROCESSING\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_121");	// [#2024] NH KSK 2011.02.24
	return RES_USER_EXIT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_InputTRAmount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Input Transfer Amount
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_InputTRAmount()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"InputTRAmount");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_InputTRAmount]\n"));

	int			nRetryCount = 3;
	int			nInputAmount = 0;//[#2114] US PCS 2012.01.10
	CString		GetKeyStr;
	BOOL		bPlayGuidance = TRUE;
	CString		strInputAmount;
//	BOOL		bConfirmState = FALSE;
	BIZ_RETURN	nRes;
	BOOL		bStartToTimeout = FALSE; //[#2118] NH PCS 2012.01.16
#if (US_VERSION)
	m_bAdaSuccessiveInput = TRUE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while ((g_TimeCheck.IsElapsedTimes() == FALSE) && (nRetryCount > 0))
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_122");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				//[#2104] US PCS 2012.01.09 "CardTronics 요청 사항 -Enter Key 위치 표시-"
#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(401,L"LocateEnterKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(401, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
				//end of [#2104]
				m_pAdaCtrl->fnExp_PlayScreenWave(401);
			}

			bPlayGuidance = FALSE;
//			bConfirmState = FALSE;
			strInputAmount = L"";
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString();
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_123");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == S_CLEAR)
			{
				strInputAmount = L"";
			}
			else if (GetKeyStr == S_ENTER)
			{
#if (US_VERSION)
				m_bAdaSuccessiveInput = FALSE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif
				NHDEBUG(DBG_INFO, (_T("INPUT AMOUNT - VALUE [%s]\n"), strInputAmount));
				/////////////////////////////////////////////////////////////////////////////////////////////
				//[#2114] NH PCS 2012.01.10 " 입력받은 값에 대한 유효성 검사. 이체 거래는 8자리 까지 입력가능."
				if (strInputAmount.GetLength() > 8)
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"MaxAmt.wav");
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();

					nRetryCount--;
					bPlayGuidance = TRUE;
				}
				//end of [#2114]

				else if ((strInputAmount.GetLength() > 0) && (strInputAmount.GetLength() < 9))
				{
					////////////////////////////////////////////////////////////////////////////////////////
					//[#2114] NH PCS 2012.01.10 "금액입력 받는 프록시저에 의한 리턴값을 따라서 분기 처리함.
					nInputAmount = Asc2Int(strInputAmount);
					nRes = P_NH_ADA_ConfirmAmt(nInputAmount);

					if (nRes == RES_USER_PREV)
					{
						nRetryCount--;
						bPlayGuidance = TRUE;
					}
					else
					{
						return nRes;
					}
					//end of [#2114]
					///////////////////////////////////////////////////////////////////////////////////////

				}
				else
				{
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"LowAmt.wav");
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();

					bPlayGuidance = TRUE;
					nRetryCount--;
				}
#if (US_VERSION)
				m_bAdaSuccessiveInput = TRUE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif
			}
			else if ((GetKeyStr.GetLength() == 1)&&(GetKeyStr != L".")) //[#2114] NH PCS 2012.01.10  
			{
				strInputAmount += GetKeyStr;
			}

			//////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bPlayGuidance = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			/////////////////////////////////////////////////////////
		}

		Delay_Msg(50);
	}

	if (nRetryCount <= 0)
	{
		NHDEBUG(DBG_INFO, (_T("RETRY COUNT OVER\n")));
		NVDump('O', 'C', "00", L"P_NHADA", L"NG_124");	// [#2024] NH KSK 2011.02.24
		return RES_USER_EXIT;	
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_125");	// [#2024] NH KSK 2011.02.24

#if (US_VERSION)
	m_bAdaSuccessiveInput = FALSE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
#endif

	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_Transaction()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Transaction with HOST
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_Transaction()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"Transaction");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_Transaction]\n"));

	BIZ_RETURN	nRes;

	// KSK 2011.07.13 Host Connect시에 PIN Disable 처리 (PIN Block이 생성되지 않는 Bug Fix)
	// Show Screen
	{
		m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
		m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_DISABLE_MODE);
	}
	
	// Communication with Host
	while(1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE)
		{
			// PLAY GUIDANCE
			m_pAdaCtrl->fnExp_StopPlay();
			if (TranCode == TC_REVERSAL)
			{
				m_pAdaCtrl->fnExp_PlayScreenWave(10);
				//NHDEBUG(DBG_CALL, (L"fnExp_PlayScreenWave(10)\n"));
			}
			else
			{
				m_pAdaCtrl->fnExp_PlayScreenWave(9);
				//NHDEBUG(DBG_CALL, (L"fnExp_PlayScreenWave(9)\n"));
			}
			m_pAdaCtrl->fnExp_WaitUntilStop();
		}

		m_pDevCmn->fnAPL_SetProcCount('3');

		if ((nRes = BIZ_SendHost()) != RES_OK)
		{	
			//NHDEBUG(DBG_CALL, (L"BIZ_SendHost()\n"));
			break;
		}
		
		if ((nRes = BIZ_RecvHost()) != RES_OK)
		{
			//NHDEBUG(DBG_CALL, (L"BIZ_RecvHost()\n"));
			break;
		}

		// Host OK
		m_pDevCmn->fnAPL_SetProcCount('5');

		return RES_OK;
	}

	if ((nRes != RES_OK) && (m_pDevCmn->TranStatus == TRAN_TRAN))
	{
		m_pAdaCtrl->fnExp_ResetScrWaveFile();
		m_pAdaCtrl->fnExp_AddWaveFile(1, L"001ATMError.wav");
		m_pAdaCtrl->fnExp_PlayScreenWave(1);
		m_pAdaCtrl->fnExp_WaitUntilStop();

		// [#2006] NH KSK 2010.11.29 Bug Fix (Host Denial인 경우에는 Denial Description을 Play하도록 함 - WINCE5.0에 적용된 항목임)
		if (m_pDevCmn->fstrAPL_GetErrorCode().Left(3) == "DA0")
		{
			int nDiffScrWave = 0;
			CString strHostDenyMsg = m_pDevCmn->fstrAPL_GetErrorCode();

			// Host Deny Code 추출. DA0XXX0 -> XXX is Host error code
			nDiffScrWave = Asc2Int(strHostDenyMsg.Mid(3, 3));

			if (nDiffScrWave != 0)
			{
				// STD3의 ADA Host Deny 파일은 8XXX로 구성됨.
				if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
					nDiffScrWave += 8000;
				// STD1,2의 ADA Host Deny 파일은 7XX0로 구성됨. 
				else
					nDiffScrWave += 7000;

				if (!m_pAdaCtrl->fnExp_PlayScreenWave(nDiffScrWave))
				{
					nDiffScrWave = 8999;
					m_pAdaCtrl->fnExp_PlayScreenWave(nDiffScrWave);
				}
			}
			m_pAdaCtrl->fnExp_WaitUntilStop();	// [#167] KSK 2008.04.24		
		}
		// end of [#2006]

		NVDump('O', 'C', "00", L"P_NHADA", L"NG_126");	// [#2024] NH KSK 2011.02.24
	}
	return nRes;	
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_CashDispense()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : Cash Dispense
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_CashDispense()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"CashDispense");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_CashDispense]\n"));

	CString strMoney;

	///////////////////////////////////
	// PLAY GUIDANCE

	BOOL	bPlayGuidance = TRUE;

	while (1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (bPlayGuidance == TRUE)
		{
			m_pAdaCtrl->fnExp_StopPlay();
			m_pAdaCtrl->fnExp_ResetScrWaveFile();
			m_pAdaCtrl->fnExp_PlayScreenWave(205);
			bPlayGuidance = FALSE;
		}

		if (m_pAdaCtrl->IsPlaying() == FALSE)
			break;

		Delay_Msg(50);
	}

	///////////////////////////////////
	// DISPENSE MONEY

	MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 1);	// [#2058] NH KSK 2011.05.11 Dispense 전에 Reason For Reversal값 설정 추가

	// [#2205] US KSK 2013.06.28
//	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_ON);
	else
		SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DISPENSING), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_DISPENSING), SKIP_MCULED);
	// end of [#2205]

	// Dispense, CENT 제거.
	m_pDevCmn->fnCDU_Dispense(Asc2Int(m_sUserSelection.strMoney.Left(10)));

	// Wait
	m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);
	m_pDevCmn->fnAPL_CheckDevice();			// [#7] KSK 2008. 03.14

	// TOTAL AMOUNT OF LAST DISPENSED NOTES
	strMoney = m_pDevCmn->fstrCDU_GetLastDispensedAmount();
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, strMoney);	// [#169] [NH] KSK 2008.04.25

	int	i = 0;
	int	nDispensed[4] = { 0, }, nRejected[4] = { 0, };
	int nSkewNote[4] = { 0, }, nGapNote[4] = { 0, }, nLongNote[4] = { 0, }, nShortNote[4] = { 0, }, nDoubleNote[4] = { 0, };
	CString strErrorCode, strtemp;

	// Dispense Status
	{	
		strErrorCode = m_pDevCmn->fstrAPL_GetErrorCode();

		if ((strErrorCode == L"9792401") ||			// FATAL ERROR
			(strErrorCode.Left(5) == L"97400"))		// CDU 단선
		{
			// Dispense Command가 SP로 가지 않는 경우 Dispensed / Rejected Count를 임의로 0으로 만든다.
			strtemp.Format(L"00 00 00 00");
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT, strtemp);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT,	strtemp);				
		}
		else
		{
			// Get Last Dispense Info
			for (i = 0; i < 4; i++)
			{
				nDispensed[i]  = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i);
				nRejected[i]   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i);
				nSkewNote[i]   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSKEW_CST1 + i);
				nGapNote[i]	   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST1 + i);
				nLongNote[i]   = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTLONGNOTE_CST1 + i);
				nShortNote[i]  = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST1 + i);
				nDoubleNote[i] = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTDOUBLE_CST1 + i);

				NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CMD_DISPENSE CALL CST [%d] DISPENSED[%d] REJECTED[%d]\n"), i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i), MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i)));
			}

			// Add to Reject Total
			for (i = 0; i < 4; i++)
			{
				AddSumOfDispensed(i+1, nDispensed[i]);
				AddSumOfRejected(i+1, nRejected[i]);
				AddSumOfDetectedSkew(i+1, nSkewNote[i]);
				AddSumOfDetectedGap(i+1, nGapNote[i]);
				AddSumOfDetectedLong(i+1, nLongNote[i]);
				AddSumOfDetectedShort(i+1, nShortNote[i]);
				AddSumOfDetectedDouble(i+1, nDoubleNote[i]);
			}

			// save Last dispensed Info
			strtemp.Format(L"%02d %02d %02d %02d",	nDispensed[0], nDispensed[1], nDispensed[2], nDispensed[3]);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT, strtemp);

			// save Last rejected Info
			strtemp.Format(L"%02d %02d %02d %02d",  nRejected[0], nRejected[1], nRejected[2], nRejected[3]);
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT,	strtemp);
		}
	}

	// Check Error
	{
		int nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
		int nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

		// KSK 2010.04.30
		if (nDispensedAmount >= nRequestedAmount)
		{
			// 과방출 처리는 상위에서 한다.
			return RES_OK;
		}
		else
		{
// [#2047] AU KSK 2011.04.14
#if (AU_VERSION)
			if ((nDispensedAmount < nRequestedAmount) && m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_CDU))	// 요구매수보다 방출매수가 작고 CDU 장애인 경우
			{
				// 1. Partial ErrorCode Check
				if ((m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40047")	||	// CST1 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4005B")	||	// CST2 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4009F")	||	// CST3 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4007C")	||	// CST4 - EP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "97447")	||	// CST1 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9745B")	||	// CST2 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9749F")	||	// CST3 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "9747C")	||	// CST4 - SP
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40041")	||	// REJECT 과다 장애1
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40043")	||	// REJECT 과다 장애2
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "40044")	||	// REJECT 과다 장애3 (Bug Fix) KSK 2010.01.04
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4004B")	||	// REJECT 과다 장애4
					(m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == "4005D"))		// REJECT 과다 장애5
				{
					int nRetryRequestAmount = (nRequestedAmount - nDispensedAmount)/100;

					// Check Dispensable
					if (m_pDevCmn->fbCDU_IsDispensible(nRetryRequestAmount))
					{
						// 방출이 가능함
						// misfeed일 경우에는 방출이 가능한 경우 errorcode를 clear후에 retry를 시도해야만 한다.
						m_pDevCmn->fnAPL_ClearError();
						m_pDevCmn->DeviceStatus &= ~DEV_CDU;			// Device Status에서 CDU 장애를 Clear

						m_pDevCmn->fnCDU_Dispense(nRetryRequestAmount);
						m_pDevCmn->fnAPL_CheckDeviceAction(DEV_CDU);
						m_pDevCmn->fnAPL_CheckDevice();			// [#7] KSK 2008. 03.14

						strMoney = m_pDevCmn->fstrCDU_GetLastDispensedAmount();

						int nFinalDispenseResult = Asc2Int(strMoney) + Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT, Int2Asc(nFinalDispenseResult));

						// 9792401, 97400은 CDU에 Dispense Command가 전달되지 않았기 때문에 Skip함
						if (!(m_pDevCmn->fstrAPL_GetErrorCode() == L"9792401" || m_pDevCmn->fstrAPL_GetErrorCode().Left(5) == L"97400"))
						{
							// Get Last Dispense Info
							for (i = 0; i < 4; i++)
							{
								nDispensed[i]  += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i);
								nRejected[i]   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i);
								nSkewNote[i]   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSKEW_CST1 + i);
								nGapNote[i]	   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST1 + i);
								nLongNote[i]   += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTLONGNOTE_CST1 + i);
								nShortNote[i]  += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST1 + i);
								nDoubleNote[i] += MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTDOUBLE_CST1 + i);

								// Add to Reject Total
								AddSumOfDispensed(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i));
								AddSumOfRejected(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i));
								AddSumOfDetectedSkew(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSKEW_CST1 + i));
								AddSumOfDetectedGap(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTGAPTOCLOSE_CST1 + i));
								AddSumOfDetectedLong(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTLONGNOTE_CST1 + i));
								AddSumOfDetectedShort(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTSHORTNOTE_CST1 + i));
								AddSumOfDetectedDouble(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTDOUBLE_CST1 + i));
							}
						}

						// save Last dispensed Info
						strtemp.Format(L"%02d %02d %02d %02d",	nDispensed[0], nDispensed[1], nDispensed[2], nDispensed[3]);
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_DISP_COUNT, strtemp);

						// save Last rejected Info
						strtemp.Format(L"%02d %02d %02d %02d",  nRejected[0], nRejected[1], nRejected[2], nRejected[3]);
						MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANS_REJ_COUNT,	strtemp);
					}
				}
			}

			nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
			nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

			// 추가 방출 수행 후 장상 방출 되었는지 다시 확인함 
			if (nDispensedAmount >= nRequestedAmount)
			{
				// 과방출 처리는 위에서 처리함.
				return RES_OK;
			}
#endif
// end of [#2047]

			bPlayGuidance = TRUE;

			// [#2205] US KSK 2013.07.06
//			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
			if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
				m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_OFF);
			else
				SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION), SKIP_MCULED);
			// end of [#2205]

			while (1)
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
					break;

				if (bPlayGuidance == TRUE)
				{
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"001ATMError.wav");
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					bPlayGuidance = FALSE;
				}

				if (m_pAdaCtrl->IsPlaying() == FALSE)
					break;

				Delay_Msg(50);
			}
			

			if (nDispensedAmount > 0)
			{
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_127");	// [#2024] NH KSK 2011.02.24

				// [#2205] US KSK 2013.07.02
//				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
				if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
					m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_ON);
				else
					SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_DISPENSING), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_DISPENSING), SKIP_MCULED);
				// end of [#2205]

				return RES_DEV_CDU_ERR_PARTIAL;
			}

			NVDump('O', 'C', "00", L"P_NHADA", L"NG_128");	// [#2024] NH KSK 2011.02.24
			return RES_DEV_CDU_ERR;
		}
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_TakeCash()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_TakeCash()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"TakeCash");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_TakeCash]\n"));

	///////////////////////////////////
	
	BOOL	bPlayGuidance = TRUE;

	while (1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (bPlayGuidance == TRUE)
		{
			m_pAdaCtrl->fnExp_StopPlay();
			m_pAdaCtrl->fnExp_ResetScrWaveFile();

			//////////////////////////////////////////////////////////////////////////////////////////////
#if (US_VERSION)
			//[#2099] US PCS 2011.11.27
			if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_MACHINEKIND)==L"MX4000W")
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"US_4000_Takemoney.wav");
			else
			{
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_MACHINEKIND) == L"NH2700T")
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"US_2700T_Takemoney.wav");
				else
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takemoney.wav");
			}
#else
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takemoney.wav");
#endif
			//end of [#2099]
			//////////////////////////////////////////////////////////////////////////////////////////////

			m_pAdaCtrl->fnExp_PlayScreenWave(1);
			bPlayGuidance = FALSE;
		}

		if (m_pAdaCtrl->IsPlaying() == FALSE)
			break;

		Delay_Msg(50);
	}

	// [#2205] US KSK 2013.07.02
//	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_CDU, FLICKER_OFF);
	else
		SetHaloLedControl(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDCOLOR_TRANSACTION), MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_HALO_LEDMODE_TRANSACTION), SKIP_MCULED);
	// end of [#2205]

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_Reversal()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ADA_Reversal()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_Reversal]\n"));

	int		nReversalFlag = 0;
	CString	strTemp;		// KSK 2012.03.01 Code Sonar 대책

	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE)
		return RES_OK;

	NVDump('O', 'C', "00", L"P_NHADA", L"Reversal");

	// reversal flag가 2, 3 이면 configuration조건을 초기화한다.
	// reversal flag - 0 : no reversal
	// reversal flag - 1 : normal reversal
	// reversal flag - 2 : configuration only, host MAC result is all space("         ") or host MAC result error(조회, 이체거래)
	// reversal flag - 3 : configuration and reversal, host MAC result error(출금거래)
	nReversalFlag = MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG);

	if (nReversalFlag == REVERSAL_TRAN)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL FLAG IS REVERSAL_TRAN\n"));

		m_pDevCmn->TranResult = FALSE;
		m_pDevCmn->TranStatus = TRAN_REVERSAL;
		TranCode = TC_REVERSAL;

		// Communication with Host
		BIZ_RETURN nRes;
		nRes = P_NH_ADA_Transaction();
		if ((nRes == RES_HOST_DENIED) || (nRes == RES_OK))
		{
			// OK or NG Value를 받았으면 Reversal Flag 를 끈다.
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
			MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 0);

			// Sum Procedure(Reversal)
			m_pDevCmn->fnCDU_SumProc(SUM_OF_REVERSAL);
		}
	}
	else if (nReversalFlag == REVERSAL_CONF)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL FLAG IS REVERSAL_CONF\n"));

		m_pDevCmn->HostOpenRetryTime = 0;
		m_pDevCmn->HostOpenFlag = FALSE;

		if (BIZ_HostConfigProc(FALSE) == TRUE)
		{
			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
		}
	}
	else if (nReversalFlag == REVERSAL_CONF_TRAN)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL FLAG IS REVERSAL_CONF_TRAN\n"));

		m_pDevCmn->HostOpenRetryTime = 0;
		m_pDevCmn->HostOpenFlag = FALSE;

		if (BIZ_HostConfigProc(FALSE) == TRUE)
		{
//			MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_TRAN);	// KSK 2010.06.18 Bug Fix (Mac Error 처리 Bug)

			m_pDevCmn->TranResult = FALSE;
			m_pDevCmn->TranStatus = TRAN_REVERSAL;
			TranCode = TC_REVERSAL;

			// Communication with Host
			BIZ_RETURN nRes;
			nRes = P_NH_ADA_Transaction();
			if ((nRes == RES_HOST_DENIED) || (nRes == RES_OK))
			{
				// OK or NG Value를 받았으면 Reversal Flag 를 끈다.
				MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);
				MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 0);

				// Sum Procedure(Reversal)
				m_pDevCmn->fnCDU_SumProc(SUM_OF_REVERSAL);
			}
		}
	}
					
	// Reversal 결과가 DA0 장애시 정상 처리함.
	if (m_pDevCmn->fstrNET_GetErrorCode().Left(3).CompareNoCase(L"DA0") == 0)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL RESPONSE [%s]\n", m_pDevCmn->fstrNET_GetErrorCode()));

		// 만일 에러 코드가 다를시에는 JNL에 메시지 남김
		if (m_pDevCmn->fstrAPL_GetErrorCode().Left(7) != m_pDevCmn->fstrNET_GetErrorCode().Left(7))	
		{
			strTemp.Format(L"%1.1s%c%5.5s(%2.2s)%c%s", MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSPROCCOUNT), UNIT_DELIMITER,
							m_pDevCmn->fstrNET_GetErrorCode().Left(5),	m_pDevCmn->fstrNET_GetErrorCode().Right(2),	UNIT_DELIMITER,	m_pDevCmn->fstrNET_GetErrorMsg());
			MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG, strTemp+MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSOTHERMSG) );	// [#2292] JUSTIN 2014.10.07 Leave DCC INFO
			SetErrSum(m_pDevCmn->fstrNET_GetErrorCode(), DEV_NET);	// [#419] [NH] KSK 2008.9.18	
		}

		MemSetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG, REVERSAL_NONE);

		NVDump('O', 'C', "00", L"P_NHADA", L"NG_129");	// [#2024] NH KSK 2011.02.24
	}
	
	// REVERSAL SUCCESS
//	if (MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE)
	if ((MemGetInt(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_REVERSALFLAG) == REVERSAL_NONE) && (nReversalFlag != REVERSAL_CONF))	// KSK 2010.06.18 Bug Fix (Mac Error시 처리 Bug)
	{
		NHDEBUG(DBG_INFO, (L"REVERSAL TRANSACTION SUCCESSFUL\n"));

		m_pDevCmn->m_JNLMgr.Save(REVERSAL_TRX);

#if (US_VERSION || AU_VERSION || CA_VERSION)	// [#2353] AU KSK 2015.06.28 AU EMV Data 저장 기능 추가	(추가적으로 ADA시 EMV 미저장 Bug Fix) // [#2477] CA Justin 2017.04.10 Leave EMV Journal
		// [#2294] US JUSTIN 2014.10.07 Leave EMV Data in US VERSION
		if ( (m_pDevCmn->fnMCU_IsEmvEnable())&&(MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID).GetLength()>0) )
				m_pDevCmn->m_JNLMgr.Save(TRX_EMV_DATA);
		// End of [#2294]
#endif

		return RES_OK;
	}

	NHDEBUG(DBG_INFO, (L"REVERSAL TRANSACTION FAILED\n"));

	NVDump('O', 'C', "00", L"P_NHADA", L"NG_130");	// [#2024] NH KSK 2011.02.24
	return RES_NG;	
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_PrintReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ADA_PrintReceipt()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"PrintReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_PrintReceipt]\n"));

	BIZ_RETURN	nRes;

	///////////////////////////////////
	// PLAY GUIDANCE

	BOOL	bPlayGuidance = TRUE;

	while (1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (bPlayGuidance == TRUE)
		{
			m_pAdaCtrl->fnExp_StopPlay();
			m_pAdaCtrl->fnExp_ResetScrWaveFile();
			m_pAdaCtrl->fnExp_PlayScreenWave(107);
			bPlayGuidance = FALSE;
		}

		if (m_pAdaCtrl->IsPlaying() == FALSE)
			break;

		Delay_Msg(50);
	}

	///////////////////////////////////
	// PRINT RECEIPT
	nRes = P_NH_NOR_TransactionPrint();
	if (nRes != RES_OK)
	{
		bPlayGuidance = TRUE;

		while (1)
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
				break;

			if (bPlayGuidance == TRUE)
			{
				// PLAY ERROR GUIDANCE TO PRINT RECEIPT
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_PlayScreenWave(7);
				bPlayGuidance = FALSE;
			}

			if (m_pAdaCtrl->IsPlaying() == FALSE)
				break;

			Delay_Msg(50);
		}
	}

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_TakeReceipt()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ADA_TakeReceipt()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"TakeReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_TakeReceipt]\n"));

	///////////////////////////////////
	// PLAY GUIDANCE
	BOOL	bPlayGuidance = TRUE;

	while (1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (bPlayGuidance == TRUE)
		{
			m_pAdaCtrl->fnExp_StopPlay();
			m_pAdaCtrl->fnExp_ResetScrWaveFile();

			// [#2375] US Justin Remove US code.. US Codes are merged to Screen Flow.
			/*
			////////////////////////////////////////////////////////////////////////////////////////////
			// [#2104] NH PCS 2012.01.10 "영수증 수취 부분에 대한 CardTronics 요청 사항 반영."
			//기존의 부분은 주석 처리 함. 일단 미국 사양으로 개발함.
			#if !(US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takereceipt.wav");
			#else
				CString strMachineType = m_pDevCmn->m_pConfig->GetMachineType();
				//1. Machine Type : 1800SE
				if (strMachineType == L"NH1800SE")
				{
					if(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")
						m_pAdaCtrl->fnExp_AddWaveFile(1,L"TakeReceipt1800.wav");
					else//MX4000W 
						m_pAdaCtrl->fnExp_AddWaveFile(1,L"TakeReceipt4000.wav");
				}
				// 2. Machine Type :  NH2700, NH2700L, NH2700T, 1500SE, and 5200SE
				else if( strMachineType==L"NH2700" || strMachineType==L"NH2700L" || strMachineType==L"NH2700T" || strMachineType==L"NH1500SE" || strMachineType==L"NH5200SE" ) // [#2267] US KSK 2014.5.12 // [#2315] US Justin 2014.12.12 
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"TakeReceipt2700.wav");
				//3. Others (Unknown) => default (please take a receipt)
				else
				{
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takereceipt.wav");
				}
				//end of [#2104]
			#endif
			/////////////////////////////////////////////////////////////////////////////////////////////
			*/
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"Takereceipt.wav");
			// End of [#2375]

			m_pAdaCtrl->fnExp_PlayScreenWave(1);
			bPlayGuidance = FALSE;

		}

		if (m_pAdaCtrl->IsPlaying() == FALSE)
			break;

		Delay_Msg(50);
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_DisplayCancel()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_NH_ADA_DisplayCancel(BIZ_RETURN nReason)
{
	NVDump('O', 'C', "00", L"P_NHADA", L"DisplayCancel");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_DisplayCancel] nReason(%d)\n", nReason));

	CString strTemp;

	// [#2375] US Justin 2015.10.28... Disable for US VERSION... TO AVOID CONFUSION
	/*
	#if (US_VERSION)
		m_bAdaSuccessiveInput = FALSE; //[#2117] US PCS 2012.01.16 "연속 금액 입력 시 안내 문구 중단."
	#endif
	*/

	///////////////////////////////////
	// PLAY GUIDANCE
	BOOL	bPlayGuidance = TRUE;

	while (1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (bPlayGuidance == TRUE)
		{
			m_pAdaCtrl->fnExp_StopPlay();

			if (nReason == RES_USER_TIMEOUT)
			{
				m_pAdaCtrl->fnExp_ResetScrWaveFile();

#if !(AU_VERSION) // [#2047] AU KJW 2011.04.19 호주에서는 We are sorry가 빠짐.
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"Sorry.wav");
#endif
				m_pAdaCtrl->fnExp_AddWaveFile(1, L"TimeOver.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
			}
			else
			{
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_PlayScreenWave(4);
			}

			bPlayGuidance = FALSE;
		}

		if (m_pAdaCtrl->IsPlaying() == FALSE)
			break;

		Delay_Msg(50);
	}

	return RES_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_DisplayExit()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_DisplayExit()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"DisplayExit");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_TakeReceipt]\n"));

	///////////////////////////////////
	// PLAY GUIDANCE
	BOOL	bPlayGuidance = TRUE;

	while (1)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (bPlayGuidance == TRUE)
		{
			m_pAdaCtrl->fnExp_StopPlay();
			m_pAdaCtrl->fnExp_ResetScrWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(1, L"TRCompleted.wav");
			m_pAdaCtrl->fnExp_PlayScreenWave(1);
			bPlayGuidance = FALSE;
		}

		if (m_pAdaCtrl->IsPlaying() == FALSE)
			break;

		Delay_Msg(50);
	}

	return RES_OK;
}

// [#2047] AU KSK 2011.04.13
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_AU_ADA_ErrorReceipt()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Error Receopt
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_AU_ADA_ErrorReceipt()
{
	NVDump('O', 'C', "00", L"P_AUADA", L"ErrorReceipt");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_ADA_ErrorReceipt]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2145] NH KSK 2012.08.22 ADA 사양을 맞추기 위해 (Voice Guide 완료 후 30초)

//	g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_TIMEOUT);
	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2145] NH KSK 2012.08.22 "타임아웃 설정을 음성 재생 후로 변경"

	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_88");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_PlayScreenWave(116);
			}

			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE; //[#2145] NH KSK 2012.08.22 
			//g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_TIMEOUT);	//[#2145] NH KSK 2012.08.22 
		}

		//[#2145] NH KSK 2012.08.22 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2145]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT || GetKeyStr == L"4")
			{
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_89");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == L"2")
			{
				m_sUserSelection.nPrintReceipt = FALSE;			// [#2219] 2013.09.04 Justin Digital Receipt, Change Variable Name

				if (TranCode == TC_INQUIRY)
					m_sUserSelection.bScreenDisplay = TRUE;

				return RES_OK;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_AUADA", L"NG_91");	// [#2024] NH KSK 2011.02.24
	return RES_USER_TIMEOUT;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_AU_ADA_DisplaySurcharge()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : Display Surcharge
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_AU_ADA_DisplaySurcharge()
{
	NVDump('O', 'C', "00", L"P_AUADA", L"ShowSurcharge");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_AU_ADA_DisplaySurcharge]\n"));

	BOOL			bPlayGuidance = TRUE;
	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE; //[#2145] NH KSK 2012.08.22 ADA 사양을 맞추기 위해 (Voice Guide 완료 후 30초)

//	g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_TIMEOUT);
	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2145] NH KSK 2012.08.22 "타임아웃 설정을 음성 재생 후로 변경"

	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_81");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				CString strChargeAmount;

				// Normal Surcharge
				{
					strChargeAmount.Format(_T("%s"), MakeMoneyCent(Int2Asc(SurchargeAmount)));		// [#2020] US KSK 2011.02.14

					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(201, strChargeAmount);
					m_pAdaCtrl->fnExp_AddWaveFile(201, L"201_1.wav");  //"This fee is in addition...
					m_pAdaCtrl->fnExp_PlayScreenWave(201);
				}
			}
			m_nDisplayedSurchargeAmount = SurchargeAmount;		// [#2150] US Justin 2012.10.01 Add Displayed Surcharge Amount	(for future use...)
			MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHARGEDISPLAY, 1);
			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE; //[#2145] NH KSK 2012.08.22 
			//g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_TIMEOUT);
		}

		//[#2145] NH KSK 2012.08.22 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2145]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_82");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
			else if (GetKeyStr == L"7")
			{
				return RES_OK;
			}
			else if (GetKeyStr == L"8")
			{
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_83");	// [#2024] NH KSK 2011.02.24
				return RES_USER_EXIT;
			}
		}

		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_84");	// [#2024] NH KSK 2011.02.24

	return RES_USER_TIMEOUT;
}
#endif

// end of [#2047]
/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_NH_ADA_GuideBalanceAmount()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_GuideBalanceAmount()
{
	NVDump('O', 'C', "00", L"P_NHADA", L"GuideBalAmount");

	if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		return RES_OK;

	// BALANCE 처리
	BOOL	bPlayGuidance = TRUE;
	BOOL	bStartToTimeout = FALSE; //[#2118] NH PCS 2012.01.16

	// [#2375] US Justin Disable US CODES.. US VERSION Uses different Transactin result function
	/*
	//[#2109] US PCS 2012.01.09 "Receipt Voicing"
#if (US_VERSION)
	// 1. 변수 선언
	CString GetKeyStr;
	CString strRequestAmt;
	CString strDispenseAmt;
	CString strTerminalFee;
	CString strLedgerBalance;
	CString strAvailableBalance;
	CString	strTransType;
	//2.변수 할당.
	strRequestAmt.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
	strDispenseAmt.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
	strTerminalFee.Format(L"%s",MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
	strLedgerBalance = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT);
	strAvailableBalance = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE);
	strTransType = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE);
	//3.공백 제거
	strRequestAmt.Replace(_T(" "), _T(""));
	strDispenseAmt.Replace(_T(" "), _T(""));
	strTerminalFee.Replace(_T(" "), _T(""));
	strLedgerBalance.Replace(_T(" "), _T(""));
	strAvailableBalance.Replace(_T(" "), _T(""));
#endif
	//end of [#2109]
	*/
	// End of [#2375]

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
			break;

		if (bPlayGuidance == TRUE) 
		// [#2375] US Justin Disable US CODES.. US VERSION Uses different Transactin result function
		//#if (!US_VERSION)		
		{
			m_pAdaCtrl->fnExp_StopPlay();

			// BALANCE 처리 (Error가 없고, 통신장애 && SPR장애인 경우)
			if ((m_pDevCmn->fnAPL_CheckError() == TRUE) || 
				(m_pDevCmn->fstrAPL_GetErrorCode().Left(1) != L"D" && m_pDevCmn->fnAPL_GetDownErrorDevice(DEV_MAIN) == DEV_SPR))	// ERROR가 없을 경우 or SPR만 장애인 경우	// [#419] [NH] 2008.9.12
			{
				m_pAdaCtrl->fnExp_ResetScrWaveFile();

				CString strTemp = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT);
				strTemp.Replace(L" ", L"");

				m_pAdaCtrl->fnExp_AddWaveFile(1, L"BALANCE.wav");
				if (strTemp.IsEmpty())
					m_pAdaCtrl->fnExp_AddWaveFile(1, L"unavailable.wav");
				else
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(1, MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT)));
				// end of [#2005]

				m_pAdaCtrl->fnExp_PlayScreenWave(1);
			}
			else
			{
				break;
			}

			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		// 미국 이외의 국가는 Guide가 끝나면 바로 Next로 이동
		if (m_pAdaCtrl->IsPlaying() == FALSE)
			break;

		//#endif// [#2375] US Justin Disable US CODES.. US VERSION Uses different Transactin result function

		Delay_Msg(50);
	}

	return RES_OK;
}

// [#2114] NH PCS 2012.01.10 "금액 입력 시 입력 값을 확인 받는 화면"
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_NH_ADA_ConfirmAmt()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_NH_ADA_ConfirmAmt(int nInputAmount)
{
	NVDump('O', 'C', "00", L"P_NHADA", L"ConfirmAmt.");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_ConfirmAmt]\n"));

	CString			GetKeyStr;
	CString			strPlayAmount;
	strPlayAmount.Format(L"%d", nInputAmount);
	if(TranCode == TC_TRANSFER)			//이체 거래의 경우 소수점까지 포함하기 때문에 분기 처리함.
		strPlayAmount = MakeMoneyCent(strPlayAmount);

	BOOL			bShowScreen = TRUE;
	BOOL			bStartToTimeout = FALSE;	// [#2118] NH PCS 2012.01.16
	BOOL			bInvalidRepeat = FALSE;		// [#2375] US Justin 2015.10.30 US ADA
	CStringArray	arrVG;						// [#2375] US Justin 2015.10.30 US ADA	

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //[#2118] NH PCS 2012.01.16 "타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		// [#2375] US Justin 2015.10.30 US ADA
		if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
		{
			bInvalidRepeat = FALSE;
			bShowScreen = TRUE;
		}
		// End of [#2375]

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			
				// [#2375] US Justin 2015.10.30 US ADA
				/*
				// KSK 2011.04.28 호주 Customers 요청에 의해 변경
				#if (US_VERSION || CA_VERSION)	//[#2077] CA PCS 2011.06.29
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(110, strPlayAmount);
								//{#2104] US PCS 2012.01.09 "CardTronics 사항 - Cancel Key 위치"
					#if (US_VERSION)
								m_pAdaCtrl->fnExp_AddWaveFile(110,L"LocateCancelKey.wav");
								m_pAdaCtrl->fnExp_AddWaveFile(110, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
					#endif
								//end of [#2104]
								m_pAdaCtrl->fnExp_PlayScreenWave(110);
				#elif (AU_VERSION)
								m_pAdaCtrl->fnExp_AddWaveFile(110, L"110_1.wav");
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(110, strPlayAmount);
								m_pAdaCtrl->fnExp_AddWaveFile(110, L"110_2.wav");
								m_pAdaCtrl->fnExp_PlayScreenWave(110);
				#endif
								// end of KSK 2011.04.28
				*/
				#if (US_VERSION)
					//if (TranCode == TC_WITHDRAWAL)	m_pAdaCtrl->fnExp_AddWaveFile(110, L"110.wav");		// Withdrawal Notice
					if (TranCode != TC_TRANSFER)		m_pAdaCtrl->fnExp_AddWaveFile(110, L"110.wav");		// [#2413] US Justin 2016.04.13 VG for Pin4 or Popmoney
					else								m_pAdaCtrl->fnExp_AddWaveFile(110, L"110_2.wav");	// Transfer Notice

					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(110, strPlayAmount);

					if (TranCode == TC_TRANSFER)
					{
						// Added Account Information
						m_pAdaCtrl->fnExp_AddWaveFile(110, L"From.wav");
						if (m_SourceAccount == S_CHECKING)
							m_pAdaCtrl->fnExp_AddWaveFile(110, L"Checking.wav");
						else if (m_SourceAccount == S_SAVINGS)
							m_pAdaCtrl->fnExp_AddWaveFile(110, L"Savings.wav");
						else if (m_SourceAccount == S_CREDITCARD)
							m_pAdaCtrl->fnExp_AddWaveFile(110, L"Credit.wav");

						m_pAdaCtrl->fnExp_AddWaveFile(110, L"To.wav");
						if (m_DestAccount  == S_CHECKING)
							m_pAdaCtrl->fnExp_AddWaveFile(110, L"Checking.wav");
						else if (m_DestAccount  == S_SAVINGS)
							m_pAdaCtrl->fnExp_AddWaveFile(110, L"Savings.wav");
						else if (m_DestAccount  == S_CREDITCARD)
							m_pAdaCtrl->fnExp_AddWaveFile(110, L"Credit.wav");
					}
					m_pAdaCtrl->fnExp_AddWaveFile(110, L"110_1.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(110, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(110, L"LocationRepeatKey.wav");
				#elif(CA_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(110, strPlayAmount);
				#elif (AU_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(110, L"110_1.wav");
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(110, strPlayAmount);
					m_pAdaCtrl->fnExp_AddWaveFile(110, L"110_2.wav");
				#endif

				m_pAdaCtrl->fnExp_PlayScreenWave(110);
				// End of [#2375]

			}		
			bShowScreen = FALSE;
			bStartToTimeout = TRUE; //[#2118] NH PCS 2012.01.16 
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		////////////////////////////////////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));
			
			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
				#if (US_VERSION)
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
				#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_112");	
				return RES_USER_EXIT;
			}
			// [#2375] US Justin 2015.10.30 
			else
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
				arrVG.RemoveAll();

				int nValidADAInput = 0;
				if( GetKeyStr == L"2" )					
				{
					//if(TranCode == TC_WITHDRAWAL)	m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);		// WITHDRAWAL : 12 DIGIT (CENT)
					if(TranCode != TC_TRANSFER)		m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);		// WITHDRAWAL : 12 DIGIT (CENT) [#2413] US Justin 2016.04.13 VG for Pin4 and Popmoney
					else							m_sUserSelection.strMoney.Format(L"%010d", nInputAmount);		// TRANSFER	  : 10 DIGIT
					arrVG.Add(L"2.wav");			arrVG.Add(L"YesSelect.wav");
					nValidADAInput = 1;					// (1) Accept Amount
				}
				else if (GetKeyStr == L"4")				
				{
					arrVG.Add(L"4.wav");		arrVG.Add(L"RetrySelect.wav");
					nValidADAInput = 2;					// (2) Rentry 
				}
				else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
				{
					nValidADAInput = 3;					// (3) Repeat or Volume Control : US ONLY
					m_pAdaCtrl->fnExp_StopPlay();
					if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
					else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
				}
									
				if	( nValidADAInput == 0 )									// Invalid Selection
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
						bInvalidRepeat = TRUE;
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					#endif
				}
				else if( (nValidADAInput==1)||(nValidADAInput==2) )			// Valid Input
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
					#endif
					if (nValidADAInput==1)		return RES_OK;
					else						return RES_USER_PREV;
				}
				else if( nValidADAInput == 3)			// Repeat or Volume Control
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
			}
			/*
			else if (GetKeyStr == L"2")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				// CENT 포함 12자리
				if(TranCode == TC_WITHDRAWAL)
					m_sUserSelection.strMoney.Format(L"%010d00", nInputAmount);
				else // TC_Transfer의 경우
					m_sUserSelection.strMoney.Format(L"%010d", nInputAmount);
				return RES_OK;
			}
			else if (GetKeyStr == L"4")
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"Guide4InputKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
				return RES_USER_PREV;
			}
		
			//////////////////////////////////////////////////////////
			//[#2101],[#2103] US PCS 2012.01.09 "Repeating and Volume Control"
#if (US_VERSION)
			else if (GetKeyStr == L".")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeDown();
				bShowScreen = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
			else if (GetKeyStr == L"00")
			{
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_SetVolumeUp();
				bShowScreen = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
#endif
			//end of [#2101],[#2103]
			/////////////////////////////////////////////////////////
			else
			{
				////////////////////////////////////////////////////////////////
				//[#2117][#2121] US PCS 2012.01.17 "입력값에 대한 안내 문구 추가."
#if (US_VERSION)
				CString strTemp;
				strTemp.Format(L"%s.wav", GetKeyStr);
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();
				m_pAdaCtrl->fnExp_AddWaveFile(1,strTemp);
				m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				m_pAdaCtrl->fnExp_WaitUntilStop();
				bShowScreen = TRUE;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
#endif
				//end of [#2117][#2121]
				/////////////////////////////////////////////////////////////////
			}
			*/
			// End of [#2375]
		}
		Delay_Msg(50);

		// [#2375] US Justin 2015.11.03 US Justin US ADA
		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
		// End of [#2375]
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_113");	
	return RES_USER_TIMEOUT;
}
//end of [#2114]

// [#2375] US Justin 2015.10.30 ADA Transaction Result
BIZ_RETURN	CTranCmn::P_NH_ADA_TransactionResult(BOOL bPrintReceipt)
{
	NVDump('O', 'C', "00", L"P_NHADA", L"TransactionResult");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_P_NH_ADA_TransactionResult]\n"));

	#if!(US_VERSION)
		return RES_OK;
	#endif

	// [#2380] US Justin 2015.12.14 Cardtronics Additional VG
	/*
	if ( (m_pDevCmn->fnSNS_GetEnhancedAudio()==FALSE) || (m_pDevCmn->TranResult!=TRUE) )
		return RES_OK;
	*/
	if(m_pDevCmn->fnSNS_GetEnhancedAudio()==FALSE )
		return RES_OK;

	CString strReceiptLocation =  L"Receipt_Location_Other.wav";
	if		(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1500SE")	strReceiptLocation = L"Receipt_Location_1500.wav";
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH1800SE")	strReceiptLocation = L"Receipt_Location_1800.wav";
	else if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")	strReceiptLocation = L"Receipt_Location_4000.wav";

	if(m_pDevCmn->TranResult!=TRUE) 
	{
		if(bPrintReceipt)
			m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Takereceipt.wav", TRUE, strReceiptLocation);

		return RES_OK;
	}
	// End of [#2380]

	CString			GetKeyStr, strTemp;
	BOOL			bShowScreen = TRUE;
	BOOL			bStartToTimeout = FALSE;
	BOOL			bInvalidRepeat = FALSE;
	CStringArray	arrVG;				
	CString			strTransType = MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTYPE);

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); 
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_78");	// [#2024] NH KSK 2011.02.24
			return RES_USER_EXIT;
		}

		if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
		{
			bInvalidRepeat = FALSE;
			bShowScreen = TRUE;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}

			// Play Guidance
			{
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();

				// [#2380] US Justin 2015.12.09 Cardtronics Additional VG change
				if(bPrintReceipt)
				{
					m_pAdaCtrl->fnExp_AddWaveFile(150, L"Takereceipt.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(150, strReceiptLocation); 	
				}
				// End of [#2380]

				if( (strTransType==TRANTYPE_WITHDRAWAL) || (strTransType==TRANTYPE_DCC) )
				{
					// withdrawal info
					m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_1.wav");

					CString strDispenseAmt;
					strDispenseAmt.Format(L"%s",MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));
					strDispenseAmt.Replace(_T(" "), _T(""));

					// Dispensed 
					strTemp.Format(L"%s", MakeMoneyCent(strDispenseAmt) );
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, strTemp);

					// [#2413] US Justin VG for Pin4 and Popmoney
					if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE) == WITHDRAWAL_PIN4 )
					{
						// with pin 4 transaction
						m_pAdaCtrl->fnExp_AddWaveFile(150, L"with.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(150, L"Pin4Tr.wav");
					}
					// [#2445] US Justin 2016.09.27 Just.Cash
					else if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE) == WITHDRAWAL_JUSTCASH )
					{
						// with just Cash transaction
						m_pAdaCtrl->fnExp_AddWaveFile(150, L"with.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(150, L"JustCashTr.wav");
					}
					// End of [#2445]
					// [#2446] US Justin 2016.09.30 Paypal CCA
					else if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_WITHDRAWAL_TYPE) == WITHDRAWAL_PAYPAL )
					{
						m_pAdaCtrl->fnExp_AddWaveFile(150, L"with.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(150, L"PayPalTr.wav");
					}
					// End of [#2446]
					else
					{
						// from
						m_pAdaCtrl->fnExp_AddWaveFile(150, L"From.wav");

						// Withdrawal Account
						if (m_SourceAccount == S_CHECKING)				m_pAdaCtrl->fnExp_AddWaveFile(150, L"Checking.wav");	
						else if (m_SourceAccount == S_SAVINGS)			m_pAdaCtrl->fnExp_AddWaveFile(150, L"Savings.wav");	
						else if (m_SourceAccount == S_CREDITCARD)		m_pAdaCtrl->fnExp_AddWaveFile(150, L"Credit.wav");
					}
					// End of [#2413] [#2415]

					BOOL bIsDCCTran = FALSE;
					if( (m_sSTD1_DynamicFlowResp.nDCCTransaction==1) || 
						((m_STD3_TDL_Data.m_bProceedDCC==TRUE)&&(m_STD3_TDL_Data.m_DCC_Transaction.m_strReq_OfferAcceptance==L"1")) ) 
						bIsDCCTran = TRUE;

					if ( (strTransType==TRANTYPE_WITHDRAWAL) || ( (strTransType==TRANTYPE_DCC)&&(bIsDCCTran==FALSE) ) )		// Non DCC Transaction
					{
						if( (Asc2Int(strDispenseAmt) > 0) && (Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT)) > 0) )
						{
							// Surcharge Guide (if dispensed amount is greater than zero)
							m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_4.wav");		// a surcharge of
							strTemp.Format(L"%s", MakeMoneyCent(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT)));
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, strTemp);			
							m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_5.wav");		// was charged to your account
						}
					}

					if( bIsDCCTran )
					{
						if(Asc2Int(strDispenseAmt) > 0)
						{
							// access fee Guide
							m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_6.wav");		// an access fee of
							strTemp.Format(L"%s", MakeMoneyCent(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT)));
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, strTemp);			
							m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_5.wav");		// was charged to your account

							// Exchange Rate is
							m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_7.wav");

							CString strATMCurrency = CURRENCY_TYPE;									// [#2375] 2015.11.25 US Justin
							//if( m_sSTD1_DynamicFlowResp.nDCCTransaction==1 )
							if( (m_sSTD1_DynamicFlowResp.nDCCTransaction==1) && (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_CUSTOMOPTION)!=DCC_CUSTOMOPTION_CARDTRONICS) )	// [#2426] Justin Add STD1 Cardtronics Option
							{
								m_pAdaCtrl->fnExp_AddWaveFile(150, L"1.wav");
								// [#2375] 2015.11.25 US Justin
								//m_pAdaCtrl->fnExp_AddWaveFile(150, L"USDollar.wav");
								if( strATMCurrency.CompareNoCase(L"USD")==0 )
									m_pAdaCtrl->fnExp_AddWaveFile(150, L"USDollar.wav");
								else
								{
									strTemp.Format(L"%s.%s.%s", strATMCurrency.Left(1), strATMCurrency.Mid(1,1), strATMCurrency.Mid(2,1));
									m_pAdaCtrl->fnExp_AddWaveFile(150, strTemp);
								}
								// End of [#2375]
								m_pAdaCtrl->fnExp_AddWaveFile(150, L"equivalent.wav");
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, m_strVG_ExchangeRate, m_strVG_HomeCurrencyName);
							}
							else
							{
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, "1", m_strVG_HomeCurrencyName);
								m_pAdaCtrl->fnExp_AddWaveFile(150, L"equivalent.wav");
								// [#2375] 2015.11.25 US Justin
								//m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, m_strVG_ExchangeRate);
								if( strATMCurrency.CompareNoCase(L"USD")==0 )		strTemp = L"USDollars.wav";				// [#2380] US Justin 2015.12.09 Cardtronics Additional VG Change
								else												strTemp.Format(L"%s ", CURRENCY_TYPE);	// Making 4 characters => Read like C.A.D
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, m_strVG_ExchangeRate, strTemp);
								// End of [#2375]
							}

							// Markup Rate
							// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
							if (m_strVG_MarkupRate.GetLength() > 0)
							{
								// [#RWC6-13] PAI has requested changes to Dual Host DCC offering screen
							#if (APP_CUSTOM_PAI)
								m_pAdaCtrl->fnExp_AddWaveFile(150, L"121_5_2.wav"); // "this includes"
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, m_strVG_MarkupRate, L"percent.wav");
								m_pAdaCtrl->fnExp_AddWaveFile(150, L"121_5_3.wav"); // "over wholesale rate"
								// end of [#RWC6-13]
							#else
								m_pAdaCtrl->fnExp_AddWaveFile(150, L"121_5_1.wav");
								m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, m_strVG_MarkupRate, L"percent.wav");
							#endif
							}
							// end of [#RWC6-2, #2585]

							// Converted Amount
							m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_8.wav");
							m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, m_strVG_ConvertedAmt, m_strVG_HomeCurrencyName);
						}
					}
				}
	
				// Ledger Balance
				if (Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT)) > 0)
				{
					m_pAdaCtrl->fnExp_AddWaveFile(150, L"LedgerBalance.wav");
					strTemp.Format(L"%s", MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREMAINAMOUNT)));
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, strTemp);
				}

				// Available Balance
				if (Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE)) > 0)
				{
					m_pAdaCtrl->fnExp_AddWaveFile(150, L"AvailBalance.wav");
					strTemp.Format(L"%s", MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSAVAILBALANCE)));				
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, strTemp);
				}

				if (strTransType == TRANTYPE_TRANSFER)
				{
					// Transfer Amount
					m_pAdaCtrl->fnExp_AddWaveFile(150, L"Trans_Receipt_Info_9.wav");
					strTemp.Format(L"%s", MakeMoneyCent(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT)));
					m_pAdaCtrl->fnExp_AddWaveFileBreakDownNumber(150, strTemp);

					// Added Account Information
					m_pAdaCtrl->fnExp_AddWaveFile(150, L"From.wav");
					if (m_SourceAccount == S_CHECKING)			m_pAdaCtrl->fnExp_AddWaveFile(150, L"Checking.wav");
					else if (m_SourceAccount == S_SAVINGS)		m_pAdaCtrl->fnExp_AddWaveFile(150, L"Savings.wav");
					else if (m_SourceAccount == S_CREDITCARD)	m_pAdaCtrl->fnExp_AddWaveFile(150, L"Credit.wav");

					m_pAdaCtrl->fnExp_AddWaveFile(150, L"To.wav");
					if (m_DestAccount  == S_CHECKING)			m_pAdaCtrl->fnExp_AddWaveFile(150, L"Checking.wav");
					else if (m_DestAccount  == S_SAVINGS)		m_pAdaCtrl->fnExp_AddWaveFile(150, L"Savings.wav");
					else if (m_DestAccount  == S_CREDITCARD)	m_pAdaCtrl->fnExp_AddWaveFile(150, L"Credit.wav");
				}

				// Repeat or remove headhpone
				m_pAdaCtrl->fnExp_AddWaveFile(150, L"LocationRepeatKey.wav");
				m_pAdaCtrl->fnExp_AddWaveFile(150, L"Remove4Complete.wav");
				m_pAdaCtrl->fnExp_PlayScreenWave(150);
			}		
			bShowScreen = FALSE;
			bStartToTimeout = TRUE;
		}

		//[#2118] NH PCS 2012.01.16 "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_SHORT_TIMEOUT);	// [#2388] US Justin 2016.01.21 ....  g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		//end of [#2118]

		////////////////////////////////////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));
			
			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
				return RES_USER_EXIT;
			}
			else
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
				arrVG.RemoveAll();

				int nValidADAInput = 0;
				if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
				{
					nValidADAInput = 1;				// (1) Repeat or Volume Control : US ONLY
					m_pAdaCtrl->fnExp_StopPlay();
					if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
					else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
				}
									
				if	( nValidADAInput == 0 )			// Invalid Selection
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
					bInvalidRepeat = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
				else if( nValidADAInput == 1)		// Repeat or Volume Control
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
			}
		}
		Delay_Msg(50);

		#if(US_VERSION)
			if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
				if( P_NH_ADA_NeedMoreTime() == RES_OK )
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
				else
					return RES_USER_EXIT;
			}
		#endif
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_130");	
	return RES_USER_TIMEOUT;
}

// [#2413] US Justin 2016.04.13 
BIZ_RETURN	CTranCmn::P_NH_ADA_Check_TimeOUT()
{
	#if(US_VERSION)
	if( (IsAdaTransaction()) &&  (m_pDevCmn->fnSNS_GetEnhancedAudio()) && (g_TimeCheck.IsElapsedTimes()) )
	{
		NHDEBUG(DBG_INFO, (L"ADA Voice Guidance TIME OUT\n"));
		if( P_NH_ADA_NeedMoreTime() == RES_OK )
		{
			NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : NEED MORE TIME = [YES]\n"));
			g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			return RES_OK;
		}
		else
			return RES_USER_EXIT;
	}
	#endif
	return RES_NG;	
}
// End of [#2413]

// [#2375] US Justin 2015.11.03 US ADA
// return  RES_OK, RES_USER_EXIT, RES_USER_TIMEOUT
BIZ_RETURN	CTranCmn::P_NH_ADA_NeedMoreTime()
{
	#if !(US_VERSION)
		return RES_USER_EXIT;
	#endif

	NVDump('O', 'C', "00", L"P_NHADA", L"NeedMoreTime");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_NeedMoreTime]\n"));

	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE;
	BOOL			bInvalidRepeat = FALSE;
	BOOL			bShowScreen = TRUE;
	CStringArray	arrVG;

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			NVDump('O', 'C', "00", L"P_NHADA", L"NG_122");
			return RES_USER_EXIT;
		}

		if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
		{
			bInvalidRepeat = FALSE;
			bShowScreen = TRUE;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("UPDATE SCREEN & PLAY GUIDANCE\n")));

			// Show Screen
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			// Play Guidance
			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			m_pAdaCtrl->fnExp_AddWaveFile(234, L"234.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(234, L"LocateCancelKey.wav");
			m_pAdaCtrl->fnExp_AddWaveFile(234, L"LocationRepeatKey.wav");
			m_pAdaCtrl->fnExp_PlayScreenWave(234);
			bShowScreen = FALSE;
			bStartToTimeout = TRUE;
		}

		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_SHORT_TIMEOUT);	// [#2388] US Justin 2016.01.21 ....	g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}

		////////////////////////////////////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));
			
			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_123");	
				return RES_USER_EXIT;
			}
			else
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input\n"));
				arrVG.RemoveAll();

				int nValidADAInput = 0;
				if( GetKeyStr == L"2" )					
				{
					arrVG.Add(L"2.wav");			arrVG.Add(L"YesSelect.wav");
					nValidADAInput = 1;					// (1) Yes
				}
				else if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
				{
					nValidADAInput = 2;					// (2) Repeat or Volume Control : US ONLY
					m_pAdaCtrl->fnExp_StopPlay();
					if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
					else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
				}
									
				if	( nValidADAInput == 0 )									// Invalid Selection
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
					bInvalidRepeat = TRUE;
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				}
				else if(nValidADAInput==1)									// Valid Input
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
					m_pAdaCtrl->fnExp_ResetAndAddPlay(arrVG.GetAt(0), TRUE, arrVG.GetAt(1));
					return RES_OK;
				}
				else if( nValidADAInput==2)									// Repeat or Volume Control
				{
					NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
					g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					bShowScreen = TRUE;
				}
			}
		}
		Delay_Msg(50);
	}

	NHDEBUG(DBG_INFO, (_T("KEY IN TIMEOUT\n")));
	NVDump('O', 'C', "00", L"P_NHADA", L"NG_124");	
	return RES_USER_TIMEOUT;
}

// return  RES_OK, RES_USER_TIMEOUT
BIZ_RETURN	CTranCmn::P_NH_ADA_RemoveHeadPhone()
{
	#if !(US_VERSION)
		return RES_OK;
	#endif

	if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)
		return RES_OK;

	NVDump('O', 'C', "00", L"P_NHADA", L"RemoveHP");
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_NH_ADA_RemoveHeadPhone]\n"));

	CString			GetKeyStr;
	BOOL			bStartToTimeout = FALSE;
	BOOL			bInvalidRepeat = FALSE;
	BOOL			bShowScreen = TRUE;
	int				nRepeatDelay;
	int				nNum_IdleRepeat = 0;		// [#2380] US Justin 2015.12.01 Cardtronics Additional VG


	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
	while(g_TimeCheck.IsElapsedTimes() == FALSE)		// not working because of "Auto Repeat"...
	{
		///////////////////////////////////
		// CHECK STATUS
		if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			m_pAdaCtrl->fnExp_StopPlay();
			return RES_OK;
		}

		if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
		{
			bInvalidRepeat = FALSE;
			bShowScreen = TRUE;
		}

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

			m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Remove_Headphone.wav", FALSE, L"LocationRepeatKey.wav");
			nRepeatDelay = 0;
			bStartToTimeout = TRUE; 
			bShowScreen = FALSE;
		}

		if( (m_pAdaCtrl->IsPlaying()==FALSE)&&(bStartToTimeout==TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_SHORT_TIMEOUT);	// [#2388] US Justin 2016.01.21 ....	g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
				
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			nNum_IdleRepeat = 0;							// [#2380] US Justin 2015.12.01 Cardtronics Additional VG
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
				m_pAdaCtrl->fnExp_StopPlay();
				if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
				else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
				bShowScreen = TRUE;
			}
			else												// Invalid Key
			{
				NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
				m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
				bInvalidRepeat = TRUE;
				nRepeatDelay = 0;
				g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
			}
		}
		else if( m_pAdaCtrl->IsPlaying() == FALSE ) 
		{
			nRepeatDelay++;
			if( (nRepeatDelay*50)>1000 )		// 1 second later
			{
				// [#2380] US Justin 2015.12.01 Cardtronics Additional VG
				nNum_IdleRepeat++;
				if(nNum_IdleRepeat>7)
					break;
				// End of [#2380]

				bShowScreen	= TRUE;
			}
		}
		Delay_Msg(50);
	}

	return RES_USER_TIMEOUT;
}
// End of [#2375]
