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
extern CTimeCheck	g_TimeCheck;

//------------------------------------------------------------------
//	Implement
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTranCmn
 FUNCTION NAME: P_EMV_CheckTransMode()
 RETURN TYPE  : -
 PARAMETER    : -
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_EMV_CheckTransMode()
{
	if (m_pDevCmn->fnMCU_IsEmvEnable())
	{
		// [#2082] CA KSK 2011.07.02
		// MS FIRST 거래
//		if (m_pDevCmn->fnMCU_IsEmvMSFirst())
//			return RES_EMV_MS_FIRST;
//		else
//			return RES_EMV_IC_FIRST;
		return RES_EMV_IC_FIRST;
		// end of [#2082]
	}
	
	return 	RES_NOR_MS_TRANS;
}

// [#2082] CA KSK 2011.07.02 함수 제거
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_CheckICCardData()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : -
-------------------------------------------------------------------*/
// BIZ_RETURN CTranCmn::P_EMV_CheckICCardData()
// {
// 	BIZ_RETURN	nRes = RES_EMV_FALLBACK;
// 	CString		strTemp;
// 
// 	m_pDevCmn->nKindOfMedia = MEDIA_MS;
// 
// 	int nResult = m_sCardData.strISO2Data.Find(L"=");	// "="이 없는 경우는 위쪽에서 return함
// 
// 	NHDEBUG(DBG_INFO, (_T("ISO2 Track Index (%d) ISO2 Track Check Data (%s)\n"), nResult, m_sCardData.strISO2Data.Mid(nResult + 5)));
// 
// 	if (m_sCardData.strISO2Data.GetAt(nResult+5) == '2' || m_sCardData.strISO2Data.GetAt(nResult+5) == '6')			// EMV CARD CHECK
// 	{
// 		// IC거래 전환, 카드를 다시 넣으라는 메시지. (어떤 화면을 사용할지는 추가 Confirm 필요 124)
// 		{
// 			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN [124]\n")));
// 			///////////////////////////////////
// 
// 			// DISPLAY SCREEN
// 			m_pDevCmn->fnSCR_DisplayPrevSet(124);
// 			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_124001));
// 			m_pDevCmn->fnSCR_DisplayScreen(124, K_NO_WAIT, KEYIN_TIME_OUT);
// 			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
// 
// 			// IC 거래를 위해 IC Entry Enable Set
// 			m_pDevCmn->nKindOfMedia = MEDIA_IC;
// 			/* IC Entry Enable & Flicker On */
// 			m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
// 			SetMcuExisted(FALSE);
// 
// 			while (g_TimeCheck.IsElapsedTimes() == FALSE)
// 			{
// 				// Timeout or MCU Media State Check
// 				if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_MCU) == TRUE)	// KSK 2010.01.04
// 				{
// 					if (m_pDevCmn->fnMCU_GetDeviceStatus() == NORMAL)
// 					{
// 						m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
// 						return RES_EMV_IC_FIRST;
// 					}
// 					else
// 					{
// 						// [#2036] CA KSK 2011.03.22 MS FIRST OPTION으로 운용 중 IC Detect되어 Card 삽입 후 Power On 시도 시 Card 제거로 인해 Power On Fail시 Cancel 처리
// 						if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	
// 							return RES_USER_EXIT;
// 						// end of [#2036]
// 
// 						// IC Card Fail시 Fall Back 거래가 되도록 함
// 						m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF);
// 						return RES_EMV_FALLBACK;				// go to EMV fall back Status
// 					}
// 				}
// 				///////////////////////////////////
// 				// GET KEY STRING
// 				if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
// 				{
// 					CString GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
// 
// 					NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));
// 
// 					GetKeyStr.TrimRight();
// 					GetKeyStr.TrimLeft();
// 
// 					if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
// 					{
// 						m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
// 						return RES_USER_EXIT;
// 					}
// 				}
// 				Delay_Msg(50);		// 카드 컨택할때까지 계속 Looping.
// 			}
// 
// 			if (g_TimeCheck.IsElapsedTimes() == TRUE)
// 			{
// 				m_pDevCmn->fnMCU_CardEnDisable(DISABLE, TRUE);
// 				return RES_USER_TIMEOUT;
// 			}
// 		}
// 	}
// 
// 	return RES_NOR_MS_TRANS;
// }
// end of [#2082]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_ICCardRead()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_EMV_ICCardRead(int screenNumber)
{
	int			nResult = 0;
	CString		strResult;

	if(!m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU))
	{
		// [#2036] CA KSK 2011.03.22 MS FIRST OPTION으로 운용 중 IC Detect되어 Card 삽입 후 Power On 시도 시 Card 제거로 인해 Power On Fail시 Cancel 처리
		if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	
			return RES_USER_EXIT;
		// end of [#2036]

		return RES_EMV_FALLBACK;
	}

	// 이 화면에는 Timeout이 없음
	m_pDevCmn->fnMCU_ClearErrorCode();
	strResult = m_pDevCmn->fstrMCU_GetICData();

	/* POWER ON Fail시 Fall Back Transaction */
	if (strResult.GetLength() <= 0)
	{
		NHDEBUG(DBG_CALL, (_T("fnMCU_ICChipInitialize Fail!!\n")));

		// [#2036] CA KSK 2011.03.22 MS FIRST OPTION으로 운용 중 IC Detect되어 Card 삽입 후 Power On 시도 시 Card 제거로 인해 Power On Fail시 Cancel 처리
		if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	
			return RES_USER_EXIT;
		// end of [#2036]

		return RES_EMV_FALLBACK;
	}

	// HexaDump(NULL, 0, 0, FALSE, FILE_LOG_TYPE, TRUE);	// [#2452] NH KSK 2016.11.10	File Initialize

	// [#2302] NH Justin 2014.11.05 Move IC Processing screen after fallback check
	if (IsAdaTransaction() == TRUE)
	{
		// 이어폰이 뽑히면 거래 중지, 캔슬이나 엑시트가 들어오면 거래 중지.
		if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
		{
			return RES_USER_EXIT;
		}

		// Wave Play
		{
			// [#2375] US Justin 2015.11.02 US ADA
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_DISABLE_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(125, L"125.wav");
			#endif
			m_pAdaCtrl->fnExp_PlayScreenWave(125);	//"Please Wait a moment" 125.wav
			m_pAdaCtrl->fnExp_WaitUntilStop();
			// End of [#2375]
		}
	}
	else
	{
		// DISPLAY SCREEN
		NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

		m_pDevCmn->fnSCR_DisplayPrevSet(screenNumber);

		// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint
		if (GetConfigFuncPointer()->GetOSVersion() == NH_OS_MX2800SE)
		{
			m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
			m_pDevCmn->fnSCR_DisplayUpdate(screenNumber);
		}
		// End of [#RWC6-188]

		// [#2164] MX Justin 2012.11.12 Change Mexico Mode Screen Text
		#if (MX_VERSION)
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125002));
		#else
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125001));
		#endif
		// End of [#2164]

		m_pDevCmn->fnSCR_DisplayScreen(screenNumber);
	}
	// End of [#2302]

	m_pDevCmn->fnNET_RMSConnectClose();		// [#2305] US Justin 2014.11.13 "EMV TRANSACTION" --- Move RMSClose, Reduce Unlatch Delay.

	// KSK 2009.9.1
	//------------------------------------------------------------------------
	BIZ_EMV_InitValue();
	BIZ_EMV_LoadAIDList();

	CString strTemp;
	strTemp.Format(L"LoadAidList");
	NVDump('O', 'C', "05", L"", strTemp );
	//------------------------------------------------------------------------

	//------------------------------------------------------------------------
	// [#2188] NH KSK 2013.05.06
	if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
	{
		nResult = m_pDevCmn->fnEMV_Sel_ApplicationSelection(PSE_SUPPOPT, &m_sAIDList, &m_nCandidateList);
	}
	else
	{
		if (m_pDevCmn->m_bPartialAIDSupport == TRUE)
			nResult = m_pDevCmn->fnEMV_V5_Sel_ApplicationSelection(PSE_SUPPOPT, &m_sAIDList_V5, &m_nCandidateList, TRUE);
		else
			nResult = m_pDevCmn->fnEMV_V5_Sel_ApplicationSelection(PSE_SUPPOPT, &m_sAIDList_V5, &m_nCandidateList, FALSE);
	}
	// end of [#2188]

	strTemp.Format(L"AppSel:%d", nResult);
	NVDump('O', 'C', "05", L"", strTemp );

	// [#2077]
	if (IsAdaTransaction() == TRUE)
	{
		// 이어폰이 뽑히면 거래 중지, 캔슬이나 엑시트가 들어오면 거래 중지.
		if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
		{
			return RES_EMV_TERMINATE;
		}
	}
	// end of [#2077]

	//------------------------------------------------------------------------
	NHDEBUG(DBG_CALL, (_T("fnEMV_Sel_ApplicationSelection():Result[%d] \n"), nResult));

	// EMV Kernel 사양에 의해 정의함
	if (nResult == EMV_RSLT_OK)
	{
		if (m_nCandidateList <= 0)
			return RES_EMV_FALLBACK;
	}
	else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
	{
		// [#2436] US Justin 2016.07.22 Not sending POS ENTRY for Unknown AID
		#if (US_VERSION)
			if( nResult == EMV_ERR_NOT_SUPPORT )
			{	
				// [#2481] US Justin 2017.04.21 Fallback for unknown AID
				/*
				if (m_pDevCmn->fnEMV_GetKernelVersion() >= EMV_KERNEL_V6)
					m_pDevCmn->m_bUnKnownAID_SkipPE = TRUE;
				*/
				// [#2564] US Justin 2018.07.17 Enable Fallback OP Options for EMV KERNEL 5.5
				//if( (m_pDevCmn->fnEMV_GetKernelVersion() >= EMV_KERNEL_V6) && ( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID)==1 ) )
				if( MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_FALLBACK_4_UNKNOWNAID)==1 )		// [#2564] US Justin 2018.07.17 Enable Fallback OP Options for EMV KERNEL 5.5
					m_pDevCmn->m_bUnKnownAID_SkipPE = TRUE;
				// End of [#2481]
			}
		#endif
		// End of [#2436]

		return RES_EMV_FALLBACK;
	}
	else
		return RES_EMV_TERMINATE;

	return RES_MAKE_AID;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_MakeAIDList()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_EMV_MakeAIDList()
{
	int			nResult = 0;
	int			nChkLoop = 0;				// [#2341] US Justin 2015.04.28 Check US COMMON AID (To Check "US COMMON AID" one time)

	while(1)
	{
		nChkLoop ++;						// [#2341] US Justin 2015.04.28 Check US COMMON AID
		NHDEBUG(DBG_CALL, (L"CTranCmn::P_EMV_MakeAIDList() START, LOOP = [%d] \n", nChkLoop));

		// [#2077]
		if (IsAdaTransaction() == TRUE)
		{
			// 이어폰이 뽑히면 거래 중지, 캔슬이나 엑시트가 들어오면 거래 중지.
			if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
			{
				return RES_EMV_TERMINATE;
			}
		}
		// end of [#2077]

		/************************************************************************/
		/* Make Application List                                                */
		/************************************************************************/
		memset(&m_sIccAppInform, 0x0, sizeof(m_sIccAppInform));			// [#2088] NH KSK 2011.08.11 AID Select Fail인 경우 Buffer 초기화를 하지 않아 화면 갱신이 안되는 Bug Fix
		memset(&m_sIccAppInform_V5, 0x0, sizeof(m_sIccAppInform_V5));	// [#2188] NH KSK 2013.04.24 

		// [#2188] NH KSK 2013.04.24
		if(m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
			nResult = m_pDevCmn->fnEMV_Sel_MakeDisplayAppList(1, (unsigned short)m_nCandidateList, &m_sIccAppInform);
		else
			nResult = m_pDevCmn->fnEMV_V5_Sel_MakeDisplayAppList(1, (unsigned short)m_nCandidateList, &m_sIccAppInform_V5);
		// end of [#2188]

		NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_MakeDisplayAppList] m_nCandidateList[%d], Result[%d] \n", m_nCandidateList, nResult));

		// KSK 2010.07.26 소프트웨어적인 것인기 때문에 정상인 경우에는 발생해서는 안된다.
		if (nResult != EMV_RSLT_OK)
			return RES_EMV_TERMINATE;

		// [#2341] US Justin 2015.04.28 Check US COMMON AID
#if (US_VERSION)
		BOOL bRemoveAID = FALSE;
		int nOption = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_USE_USCOMMONAID);

		// Kernel5 , CandidateList>1, Use Common or Use International, bFirst
		// [#2435] NH KSK 2016.07.14 Added EMV Kernel V6
//		if( (m_pDevCmn->fnEMV_GetKernelVersion()==EMV_KERNEL_V5) && (m_nCandidateList>1) && (nChkLoop==1) &&
//			( (nOption==COMMONAID_USE_COMMON)||(nOption==COMMONAID_USE_INTERNATIONAL) ) )
		if( (m_pDevCmn->fnEMV_GetKernelVersion() != EMV_KERNEL_V4) && (m_nCandidateList>1) && (nChkLoop==1) &&
			( (nOption==COMMONAID_USE_COMMON)||(nOption==COMMONAID_USE_INTERNATIONAL) ) )
		{
			//nOption = COMMONAID_USE_INTERNATIONAL;		// Testing ONLY.. .DO NOT ENABLE.....
			NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_MakeDisplayAppList] Start Checking US Common AID, Option = [%d] \n", nOption));

			int i, k, nBgPt;
			CString strTemp1, strTemp2;

			// Buffer for Country code and IIN
			// 7 bytes for each AID : Country Code[2], IIN[3], List Number[1], Remove ot not[1]
			unsigned char *pCompareCode = new unsigned char [7*m_nCandidateList + 1];
			memset(pCompareCode, NULL, 7*m_nCandidateList+1);

			// Assign Country code and IIN to buffer
			NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_MakeDisplayAppList] Total CandidateList = [%d] \n", m_nCandidateList));
			for (i=0; i< m_nCandidateList; i++)
			{
				NHDEBUG(DBG_CALL, (L"==== AID [%d]==== \n",	i));			
				NHDEBUG(DBG_CALL, (L" CountryCodeLen      = [%d] \n", m_sIccAppInform_V5.IccAppDesc[i].IsrCntryCodeLen_Alpha2));
				NHDEBUG(DBG_CALL, (L" IsrCntryCode_Alpha2 = [%02X %02X] u:75,U:55, s:73, S:53 \n", m_sIccAppInform_V5.IccAppDesc[i].IsrCntryCode_Alpha2[0],	m_sIccAppInform_V5.IccAppDesc[i].IsrCntryCode_Alpha2[1]));
				NHDEBUG(DBG_CALL, (L" IsrIdNo             = [%02X%02X%02X] \n",	m_sIccAppInform_V5.IccAppDesc[i].IsrIdNo[0], m_sIccAppInform_V5.IccAppDesc[i].IsrIdNo[1], m_sIccAppInform_V5.IccAppDesc[i].IsrIdNo[2]));			
				NHDEBUG(DBG_CALL, (L" AID                 = [%s] \n", MakeUnPack(m_sIccAppInform_V5.IccAppDesc[i].AidName, m_sIccAppInform_V5.IccAppDesc[i].AidLen) ));			
				NHDEBUG(DBG_CALL, (L" AID LABEL           = [%S] \n", m_sIccAppInform_V5.IccAppDesc[i].AppLabel));			
				NHDEBUG(DBG_CALL, (L" AID Preferred Name  = [%S] \n", m_sIccAppInform_V5.IccAppDesc[i].PreferredName));			

				nBgPt = 7*i;				
				memcpy( &pCompareCode[nBgPt], m_sIccAppInform_V5.IccAppDesc[i].IsrCntryCode_Alpha2, 2);	// Copy Country Code (2 bytes)
				memcpy( &pCompareCode[nBgPt + 2], m_sIccAppInform_V5.IccAppDesc[i].IsrIdNo, 3);			// Copy IIN (3 bytes)
				pCompareCode[nBgPt + 5] = (unsigned char) i;											// AID Order

				// Country Code => Make Capital for comparision (us => US)
				if( pCompareCode[nBgPt]   == 'u')		pCompareCode[nBgPt] = 'U';
				if( pCompareCode[nBgPt+1] == 's')		pCompareCode[nBgPt+1] = 'S';
			}

			// Sorting Buffer : "Country code + IIN" order alpabethicall...
			BOOL bChanged;
			unsigned char pBufTemp[6];
			unsigned long nCntryCode1, nCntryCode2, nIIN1, nIIN2;
			for(i=0; i<(m_nCandidateList-1); i++)
			{
				bChanged = FALSE;
				for(k=0; k<(m_nCandidateList-1); k++)
				{
					nCntryCode1 = pCompareCode[(k+0)*7]*256 + pCompareCode[(k+0)*7+1];
					nCntryCode2 = pCompareCode[(k+1)*7]*256 + pCompareCode[(k+1)*7+1];
					nIIN1 = pCompareCode[(k+0)*7+2]*255*255 + pCompareCode[(k+0)*7+3]*255 + pCompareCode[(k+0)*7+4];
					nIIN2 = pCompareCode[(k+1)*7+2]*255*255 + pCompareCode[(k+1)*7+3]*255 + pCompareCode[(k+1)*7+4];

					if( (nCntryCode1>nCntryCode2) || ( (nCntryCode1==nCntryCode2)&&(nIIN1>nIIN2) ) )
					{
						memcpy( pBufTemp,               &pCompareCode[k*7], 6 );
						memcpy( &pCompareCode[k*7],     &pCompareCode[(k+1)*7], 6 );
						memcpy( &pCompareCode[(k+1)*7], pBufTemp, 6 );
						bChanged = TRUE;
					}
				}
				if(bChanged==FALSE)
					break;
			}

			// Find the first and last AIDs which has "US" contory code.....
			int nStartOrder = -1;
			int nEndOrder	= -1;
			BOOL bFoundStart = FALSE;
			for(i=0; i<m_nCandidateList; i++)
			{
				nBgPt = 7*i;
				nIIN1 = pCompareCode[nBgPt+2]*255*255 + pCompareCode[nBgPt+3]*255 + pCompareCode[nBgPt+4];
				if(nIIN1>0)			// Is "0x00 0x00 0x00" AN INSTITUTE ???????
				{
					if( (pCompareCode[nBgPt]=='U')&&(pCompareCode[nBgPt+1]=='S') )
					{
						if(bFoundStart!=TRUE)
						{
							bFoundStart = TRUE;
							nStartOrder = i;
						}
					}
					else
					{
						if(bFoundStart==TRUE)
						{
							nEndOrder = i-1;
							break;
						}
					}
				}
			}
			if( (bFoundStart==TRUE)&&(nStartOrder>=0)&&(nEndOrder<0) )
				nEndOrder = m_nCandidateList -1;

			// Mark Common AID (Set Flag)
			int nTargetAIDOrder;
			if(	(nStartOrder>=0) && (nEndOrder>nStartOrder) )
			{
				int nWorkingOrder = nStartOrder;
				int nSubStart, nSubEnd, nNumCommonAID;
				BOOL bCheckAll = FALSE;
				do
				{
					// Finding one IIN Sub group
					nSubStart = nWorkingOrder;
					nSubEnd = -1;
					nIIN1 = pCompareCode[nSubStart*7+2]*255*255 + pCompareCode[nSubStart*7+3]*255 + pCompareCode[nSubStart*7+4];
					for(k=(nSubStart+1); k<=nEndOrder; k++)
					{
						nIIN2 = pCompareCode[k*7+2]*255*255 + pCompareCode[k*7+3]*255 + pCompareCode[k*7+4];
						if (nIIN1 != nIIN2 )
						{
							nSubEnd = k-1;
							nWorkingOrder = k;
							break;
						}
					}
					if(nSubEnd<0)
					{
						nSubEnd = nEndOrder;
						nWorkingOrder = nEndOrder+1;
					}
	
					// if one IIN sub group has multiple AIDs.......
					if( nSubEnd > nSubStart)
					{
						// Compare AID with common AID... It if is one of common AID, set a flag
						nNumCommonAID = 0;
						for(i=nSubStart; i<=nSubEnd; i++)
						{
							// Target AID
							nTargetAIDOrder = pCompareCode[i*7+5];		
							CString strAID;
							strAID.Format(L"%s", MakeUnPack(m_sIccAppInform_V5.IccAppDesc[nTargetAIDOrder].AidName, m_sIccAppInform_V5.IccAppDesc[nTargetAIDOrder].AidLen) );
							for(k=0; k<m_pDevCmn->m_arrUSCommonAID.GetSize(); k++)				// Check All US Common AID and Mark 1
							{
								strTemp1 = m_pDevCmn->m_arrUSCommonAID.GetAt(k);					// US Common AID
								if(strAID.GetLength() >= strTemp1.GetLength() )
								{
									strTemp2 = strAID.Left(strTemp1.GetLength());	// Shortened Target AID (Same Length of Common AID)
									if(strTemp2.CompareNoCase(strTemp1)==0)			// Matching with Common AID
									{
										NHDEBUG(DBG_CALL, (L"  ****** [%s] is US COMMON AID \n", strAID));	
										nNumCommonAID++;
										pCompareCode[i*7+6] = 1;					// This is Common AID....
										break;
									}
								}
							}
						}

						// Mark remove AID depends on OPTION (USE COMMON AID or USE INTERNATIONAL AID)
						if( (nNumCommonAID>0) && (nNumCommonAID<(nSubEnd-nSubStart+1)) )	// Common AIDs are mixed with other AIDs...
						{
							for(i=nSubStart; i<=nSubEnd; i++)
							{
								if(pCompareCode[i*7+6]==1)					// This is Common AID
								{
									if(nOption==COMMONAID_USE_COMMON)
										pCompareCode[i*7+6] = 0;			// Do not Remove
									// Other option (USE International AID) => Leave 1 : Remove this
								}
								else										// Other AIDs
								{
									if(nOption==COMMONAID_USE_COMMON)
										pCompareCode[i*7+6] = 1;			// Remove this
									// Other option (USE International AID) => Leave 0 : Do no Remove
								}
							}
						}
						else									// All Common AID or All Other AIDs
						{
							for(i=nSubStart; i<=nSubEnd; i++)
								pCompareCode[i*7+6] = 0;		// Do not remove All
						}
					}
					if(nWorkingOrder>=nEndOrder)
						bCheckAll = TRUE;
				} while (bCheckAll == FALSE);
			}

			// Remove marked AIDs
			int nOrgCandidateList = m_nCandidateList;
			for (i=0; i< nOrgCandidateList; i++)
			{	
				if( pCompareCode[i*7+6] == 1) // Marked AID..
				{
					nTargetAIDOrder = pCompareCode[i*7+5];
					NHDEBUG(DBG_CALL, (L"  Removing AID[%d], AID Name = [%s], Remain Candidate = [%d] \n", nTargetAIDOrder, MakeUnPack(m_sIccAppInform_V5.IccAppDesc[nTargetAIDOrder].AidName, m_sIccAppInform_V5.IccAppDesc[nTargetAIDOrder].AidLen), m_nCandidateList) );
					m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform_V5.IccAppDesc[nTargetAIDOrder].AidLen, m_sIccAppInform_V5.IccAppDesc[nTargetAIDOrder].AidName);
					m_nCandidateList--;
						
					bRemoveAID = TRUE;
				}
			}
			delete [] pCompareCode;
		}

		if (m_nCandidateList <= 0)
			return RES_EMV_TERMINATE;

		if (bRemoveAID == TRUE)
		{
			NHDEBUG(DBG_CALL, (L"  AID(s) is/are removed (US COMMON AID Checking). Start Make AID List Again. \n"));
			Delay_Msg(100);
			continue;
		}
		else
			NHDEBUG(DBG_CALL, (L"  No AID is removed (US COMMON AID Checking). \n"));

		// End of [#2341]

#elif (CA_VERSION)	// 캐나다 특이사양 적용 (5F56, DF62 Tag 처리)
		BOOL bExistPrimary = FALSE;
		BOOL bRemoveAID = FALSE;

		// [#2188] NH KMK 2013.09.09 신커널 로직 적용
		if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)	// [#2226] NH Justin 2013.10.22 EMV Kernel5
		{
			for(int i=0; i<m_sIccAppInform.AppCnt; i++)
			{
				// 1. Application Selection Flag (ASF) Exist Check
				if (m_sIccAppInform.IccAppDesc[i].ApSelCodeLen <= 0)
				{
					// ASF 미 존재 시 PRIMARY LIST에 등록
					m_sIccAppInform.IccAppDesc[i].ApSelCodeLen = 1;
					m_sIccAppInform.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;
					bExistPrimary = TRUE;
				}
				else
				{
					// 2. ASF Bit Check (ASF값이 0xC0 (Bit 8 & Bit7)인 경우 PRIMARY로 등록
					if (m_sIccAppInform.IccAppDesc[i].ApSelCode[0] & PRIMARY_PROC)
						m_sIccAppInform.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;

					// 3. ASF Bit Check
					if ((m_sIccAppInform.IccAppDesc[i].ApSelCode[0] != PRIMARY_PROC) && (m_sIccAppInform.IccAppDesc[i].ApSelCode[0] != SECONDARY_PROC))
					{
						// Remove AID
						m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform.IccAppDesc[i].AidLen,
																 m_sIccAppInform.IccAppDesc[i].AidName);
						NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
						m_nCandidateList--;
						bRemoveAID = TRUE;
						break;
					}
					else
					{
						if (m_sIccAppInform.IccAppDesc[i].IsrCntryCodeLen <= 0)
						{
							// 4. Issuer Country Code 미 존재 시 PRIMARY LIST에 등록
							m_sIccAppInform.IccAppDesc[i].ApSelCodeLen = 1;
							m_sIccAppInform.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;
							bExistPrimary = TRUE;
						}
						else
						{
							// 5. ICC Check
							if (memcmp(m_sIccAppInform.IccAppDesc[i].IsrCntryCode, ISSUER_COUNTRY_CODE, 3) != 0)
							{
								// ICC가 "CAN"이 아닌 경우 PRIMARY LIST에 등록
								m_sIccAppInform.IccAppDesc[i].ApSelCodeLen = 1;
								m_sIccAppInform.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;
								bExistPrimary = TRUE;
							}
							else
							{
								if (m_sIccAppInform.IccAppDesc[i].ApSelCode[0] == PRIMARY_PROC)
									bExistPrimary = TRUE;
							}
						}
					}
				}
			}

			// PRIMARY 와 SECONDARY가 섞여있는경우 SECONDARY AID를 삭제
			if ((bRemoveAID == FALSE) && (bExistPrimary == TRUE))
			{
				for(int i=0; i<m_sIccAppInform.AppCnt; i++)
				{
					if (m_sIccAppInform.IccAppDesc[i].ApSelCode[0] == SECONDARY_PROC)
					{
						// Remove SECONDARY AID
						m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform.IccAppDesc[i].AidLen,
																 m_sIccAppInform.IccAppDesc[i].AidName);
						NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
						m_nCandidateList--;
						bRemoveAID = TRUE;
						break;
					}
				}
			}
		}
		else
		{
			for(int i=0; i<m_sIccAppInform_V5.AppCnt; i++)
			{
				// 1. Application Selection Flag (ASF) Exist Check
				if (m_sIccAppInform_V5.IccAppDesc[i].ApSelCodeLen <= 0)
				{
					// ASF 미 존재 시 PRIMARY LIST에 등록
					m_sIccAppInform_V5.IccAppDesc[i].ApSelCodeLen = 1;
					m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;
					bExistPrimary = TRUE;
				}
				else
				{
					// 2. ASF Bit Check (ASF값이 0xC0 (Bit 8 & Bit7)인 경우 PRIMARY로 등록
					if (m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] & PRIMARY_PROC)
						m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;

					// 3. ASF Bit Check
					if ((m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] != PRIMARY_PROC) && (m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] != SECONDARY_PROC))
					{
						// Remove AID
						m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform_V5.IccAppDesc[i].AidLen,
																 m_sIccAppInform_V5.IccAppDesc[i].AidName);
						NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
						m_nCandidateList--;
						bRemoveAID = TRUE;
						break;
					}
					else
					{
						if (m_sIccAppInform_V5.IccAppDesc[i].IsrCntryCodeLen <= 0)
						{
							// 4. Issuer Country Code 미 존재 시 PRIMARY LIST에 등록
							m_sIccAppInform_V5.IccAppDesc[i].ApSelCodeLen = 1;
							m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;
							bExistPrimary = TRUE;
						}
						else
						{
							// 5. ICC Check
							if (memcmp(m_sIccAppInform_V5.IccAppDesc[i].IsrCntryCode, ISSUER_COUNTRY_CODE, 3) != 0)
							{
								// ICC가 "CAN"이 아닌 경우 PRIMARY LIST에 등록
								m_sIccAppInform_V5.IccAppDesc[i].ApSelCodeLen = 1;
								m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] = PRIMARY_PROC;
								bExistPrimary = TRUE;
							}
							else
							{
								if (m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] == PRIMARY_PROC)
									bExistPrimary = TRUE;
							}
						}
					}
				}
			}

			// PRIMARY 와 SECONDARY가 섞여있는경우 SECONDARY AID를 삭제
			if ((bRemoveAID == FALSE) && (bExistPrimary == TRUE))
			{
				for(int i=0; i<m_sIccAppInform_V5.AppCnt; i++)
				{
					if (m_sIccAppInform_V5.IccAppDesc[i].ApSelCode[0] == SECONDARY_PROC)
					{
						// Remove SECONDARY AID
						m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform_V5.IccAppDesc[i].AidLen,
																 m_sIccAppInform_V5.IccAppDesc[i].AidName);
						NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
						m_nCandidateList--;
						bRemoveAID = TRUE;
						break;
					}
				}
			}
		}
		// end of [#2188]		

		if (m_nCandidateList <= 0)
		{
			// Interac 사양에 의해 AID가 삭제됐을 경우에는 Terminate 시켜야 한다.
			return RES_EMV_TERMINATE;
		}

		if (bRemoveAID == TRUE)
		{
			Delay_Msg(100);
			continue;
		}
#endif
		// [#2188] NH KSK 2013.05.06
		if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
		{
			NHDEBUG(DBG_CALL, (L"AppCnt[0x%02x] CardHoladerConfirm[0x%02x] \n", m_sIccAppInform.AppCnt, m_sIccAppInform.CardHolderConfirm));

			if ((m_sIccAppInform.AppCnt == 1) && (m_sIccAppInform.CardHolderConfirm == 0x00)) // EMV 사양
			{
				// 고객 선택 없이 자동 선택한다.

				// [#2440] NH Justin 2016.08.10 Showing AID Selection even though one AID Left after removing Blocked AID 
				if( m_pDevCmn->m_bShownMultiAIDSelection == TRUE )
					return RES_MULTI_AID_SELECT;
				// End of [#2440]

				m_nSelectAID = 0;
				m_nSelectedTermDataIndex = BIZ_EMV_GetTerminalDataIndex(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName,
					m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen);

				if (m_nSelectedTermDataIndex < 0)	// [#2022] NH KSK 2011.02.22
					return RES_EMV_TERMINATE;

				NHDEBUG(DBG_CALL, (L"[GetTerminalDataIndex] Result[%d] \n", m_nSelectedTermDataIndex));

				//////////////////////////////////////////////////////////////////////////
				// EMV
				nResult = m_pDevCmn->fnEMV_Sel_FinalAppSelection(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen,
					m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName,
					&m_chSW);

				NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_FinalAppSelection] Result[%d] \n", nResult));

				if (nResult != EMV_RSLT_OK)
				{
					if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
						return RES_EMV_FALLBACK;
					else
						return RES_EMV_TERMINATE;
				}
				else
				{
					// KSK 2010.07.22 Final App Selection 이후 Init Application을 수행하여 실패 시 Make APP List를 수행하도록 함 (EMV 사양임)
					//------------------------------------------------------------------------
					BIZ_EMV_StoreDefaultTerminalValue();

					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					nResult = m_pDevCmn->fnEMV_Trans_InitApplication();	
					CString strTemp;
					strTemp.Format(L"InitAppProc:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp );

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_ACCEPT)
						{
							if (m_nCandidateList > 1)
							{
								m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen,
									m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName);
								NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
								m_nCandidateList--;

								return RES_MAKE_AID;
							}
							else
							{
								// KSK 2010.08.24 VISA Test Case 12 Issue 대응
								// Kernel 문서에는 Terminate로 되어져 있으나, CE는 Combined Reader이므로, FallBack을 하도록 수정
								// Interac , Master Card에는 없는 Test Case임.
								// return RES_EMV_TERMINATE;
								return RES_EMV_FALLBACK;
							}
						}
						else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					//[#2173] NH Justin 2013.01.15 Reading Card Track data first (IC Card) for TH V2 - US and Canada
//					#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거
					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					NHDEBUG(DBG_CALL, (L"[CTranCmn::fnEMV_Trans_ReadAppData]\n"));
					nResult = m_pDevCmn->fnEMV_Trans_ReadAppData();
					strTemp.Format(L"ReadAppData:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp);

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					CString strIDCTrack2;
					unsigned char szTempa[1024];
					int				nLen = 0;
					memset(szTempa, 0, sizeof(szTempa));

					/* Get 2 Track data from IC */
					if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_Track2EquData, &nLen, szTempa) == EMV_RSLT_OK)
					{
						strIDCTrack2 = MakeUnPack(szTempa, nLen);
						NHDEBUG(DBG_INFO, (L"Success to Get track 2 data from IC\n"));
						NHDEBUG(DBG_INFO, (L"IDC Track2 : [%s]\n", strIDCTrack2));
					}
					else
					{
						NHDEBUG(DBG_INFO, (L"Failed to Get track 2 data from IC\n"));
						return RES_EMV_TERMINATE;
					}
					BIZ_RETURN	nRes = BIZ_EMV_ICCardRead(strIDCTrack2);

					if (nRes != RES_OK)
						return RES_EMV_TERMINATE;
//					#endif	// End of [#2173]	// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거

#if (AU_VERSION)	// [#2379] AU KSK 2015.11.17
					if (m_pDevCmn->m_bDomesticFallbackTrans == TRUE)
					{
						NVDump('O', 'C', "53", L"", L"D_BIN_DETECT" );
						return RES_EMV_FALLBACK;
					}
#endif				// end of [#2379]

					// APP NAME and AID SAVE
					//[#2252] US Justin 2014.02.04 Long AID Bug Fix
					CString strAID, strAPPName;
					strAID.Format(L"%s", MakeUnPack(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen));	
					strAPPName.Format(L"%S", m_sIccAppInform.IccAppDesc[m_nSelectAID].AppLabel);	
					MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID, strAID);
					MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME, strAPPName);

					NHDEBUG(DBG_INFO, (L"AID[%s] Len[%d]\n",strAID, m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen));
					// End [#2252]

					return RES_NOR_MS_TRANS;
				}
			}
			else
			{
				break;
			}
		}
		else	// if the EMV kernel is V5 or Higher versions
		{
			NHDEBUG(DBG_CALL, (L"AppCnt[0x%02x] CardHoladerConfirm[0x%02x] \n", m_sIccAppInform_V5.AppCnt, m_sIccAppInform_V5.CardHolderConfirm));

			if ((m_sIccAppInform_V5.AppCnt == 1) && (m_sIccAppInform_V5.CardHolderConfirm == 0x00)) // EMV 사양
			{
				// 고객 선택 없이 자동 선택한다.

				// [#2440] NH Justin 2016.08.10 Showing AID Selection even though one AID Left after removing Blocked AID 
				if( m_pDevCmn->m_bShownMultiAIDSelection == TRUE )
					return RES_MULTI_AID_SELECT;
				// End of [#2440]

				m_nSelectAID = 0;
				m_nSelectedTermDataIndex = BIZ_EMV_GetTerminalDataIndex(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName,
					m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen);

				if (m_nSelectedTermDataIndex < 0)	// [#2022] NH KSK 2011.02.22
					return RES_EMV_TERMINATE;

				NHDEBUG(DBG_CALL, (L"[GetTerminalDataIndex] Result[%d] \n", m_nSelectedTermDataIndex));

				//////////////////////////////////////////////////////////////////////////
				// EMV
				nResult = m_pDevCmn->fnEMV_Sel_FinalAppSelection(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
					m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName,
					&m_chSW);

				NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_FinalAppSelection] Result[%d] \n", nResult));

				if (nResult != EMV_RSLT_OK)
				{
					if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
						return RES_EMV_FALLBACK;
					else
						return RES_EMV_TERMINATE;
				}
				else
				{
					// KSK 2010.07.22 Final App Selection 이후 Init Application을 수행하여 실패 시 Make APP List를 수행하도록 함 (EMV 사양임)
					//------------------------------------------------------------------------
					BIZ_EMV_StoreDefaultTerminalValue();

					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					nResult = m_pDevCmn->fnEMV_Trans_InitApplication();	
					CString strTemp;
					strTemp.Format(L"InitAppProc:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp );

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_ACCEPT)
						{
							if (m_nCandidateList > 1)
							{
								m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
									m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName);
								NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
								m_nCandidateList--;

								return RES_MAKE_AID;
							}
							else
							{
								// KSK 2010.08.24 VISA Test Case 12 Issue 대응
								// Kernel 문서에는 Terminate로 되어져 있으나, CE는 Combined Reader이므로, FallBack을 하도록 수정
								// Interac , Master Card에는 없는 Test Case임.
								// return RES_EMV_TERMINATE;
								return RES_EMV_FALLBACK;
							}
						}
						else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					if (IsAdaTransaction() != TRUE)		// [#2512] NH Justin 2017.10.24 Voice Guidance Bug Fix (AU, CA). Not changing screen While ATM processes ADA/EMV
					{
						// [#2464] AU KSK 2016.12.23 ReadApp시 시간이 오래 소요되어 화면 추가함
						// DISPLAY SCREEN (IC Processing)
						m_pDevCmn->fnSCR_DisplayPrevSet(125);
						#if (MX_VERSION)
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125002));
						#else
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125001));
						#endif
						m_pDevCmn->fnSCR_DisplayScreen(125);
					}
					// end of [#2464]

					//[#2173] NH Justin 2013.01.15 Reading Card Track data first (IC Card) for TH V2 - US and Canada
//					#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거
					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					NHDEBUG(DBG_CALL, (L"[CTranCmn::fnEMV_Trans_ReadAppData]\n"));
					nResult = m_pDevCmn->fnEMV_Trans_ReadAppData();
					strTemp.Format(L"ReadAppData:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp);

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					CString strIDCTrack2;
					unsigned char szTempa[1024];
					int				nLen = 0;
					memset(szTempa, 0, sizeof(szTempa));

					/* Get 2 Track data from IC */
					if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_Track2EquData, &nLen, szTempa) == EMV_RSLT_OK)
					{
						strIDCTrack2 = MakeUnPack(szTempa, nLen);
						NHDEBUG(DBG_INFO, (L"Success to Get track 2 data from IC\n"));
						NHDEBUG(DBG_INFO, (L"IDC Track2 : [%s]\n", strIDCTrack2));
					}
					else
					{
						NHDEBUG(DBG_INFO, (L"Failed to Get track 2 data from IC\n"));
						return RES_EMV_TERMINATE;
					}

					BIZ_RETURN	nRes = BIZ_EMV_ICCardRead(strIDCTrack2);

					if (nRes != RES_OK)
						return RES_EMV_TERMINATE;
//					#endif			// end of [#2173]	// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거

#if (AU_VERSION)	// [#2379] AU KSK 2015.11.17
					if (m_pDevCmn->m_bDomesticFallbackTrans == TRUE)
					{
						NVDump('O', 'C', "53", L"", L"D_BIN_DETECT" );
						return RES_EMV_FALLBACK;
					}
#endif				// end of [#2379]

					// APP NAME and AID SAVE
					// [#2252] US Justin 2014.02.04 Long AID BUg Fix
					/*
					CString strAID;
					// [#2188-1] NH KSK 2013.05.29 Issuer Code Index가 01인 경우 Preffered Name을 표시하도록 함 (ICS 문서에 1만 지원함)
					if (m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].IsrCodeTableIndex == 1)
					{
						// [#2189] TA중 수정사항
						CString strLabel;
						strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName);
						strLabel.TrimRight();
						if (strLabel.IsEmpty())
						{
							// Prefered Name이 없는 경우 label을 표시하도록 수정
							strAID.Format(L"%-17.17S = %-20.20s", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel, 
								MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));	// [#506] [CA] KSK 2009.3.23
						}
						else
						{
							strAID.Format(L"%-17.17S = %-20.20s", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName, 
								MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));	// [#506] [CA] KSK 2009.3.23
						}
					}
					else
					{
						strAID.Format(L"%-17.17S = %-20.20s", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel, 
							MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));		// [#506] [CA] KSK 2009.3.23
					}
					MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID, strAID);

					NHDEBUG(DBG_CALL, (L"AID[%s] Len[%d]\n", MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen), m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));
					*/
					CString strAID;
					strAID.Format(L"%s", MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen) );

					// APP NAME
					CString strAPPName;
					if (m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].IsrCodeTableIndex == 1)
					{
						CString strLabel;
						strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName);
						strLabel.TrimRight();
						if (strLabel.IsEmpty())
							strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel );
						else
							strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName );
					}
					else
						strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel );

					MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID, strAID);
					MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME, strAPPName);
					NHDEBUG(DBG_CALL, (L"AID[%s=%s]\n", strAPPName, strAID));
					// End of [#2252]

					return RES_NOR_MS_TRANS;
				}
			}
			else
			{
				break;
			}
		}
		// end of [#2188]
	}

	NHDEBUG(DBG_CALL, (L"return \n"));

	return RES_MULTI_AID_SELECT;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_SelectMultiAIDList()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_EMV_SelectMultiAIDList()
{
	// [#2375] US Justin 2015.11.02 Combine ADA and Screen Transaction Flows
	/************************************************************************/
	/*  Display Application ID                                              */
	/************************************************************************/
	/*
	// Setting Screen
	{
		NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

		m_pDevCmn->fnSCR_DisplayPrevSet(126);

		// Title
		m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_126001));

		// Sub Title
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_126002));

		int nIndex = 4;
		DWORD i;

		// [#2188] NH KSK 2013.05.06
		if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
		{
			for (i=1; i<=EMV_MAX_AIDLIST; i++)
			{
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppCnt[%d] \n", m_sIccAppInform.AppCnt));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabelLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].AppLabelLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabel[%S] \n", m_sIccAppInform.IccAppDesc[i-1].AppLabel));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AidLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].AidLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] priority[0x%x] \n", m_sIccAppInform.IccAppDesc[i-1].priority));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCodeLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].IsrCntryCodeLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCode[%S] \n", m_sIccAppInform.IccAppDesc[i-1].IsrCntryCode));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCodeLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].ApSelCodeLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCode[0x%x 0x%x] \n", m_sIccAppInform.IccAppDesc[i-1].ApSelCode[0], m_sIccAppInform.IccAppDesc[i-1].ApSelCode[1]));

				// [#2088] NH KSK 2011.08.10 커널 버그임, remove할 경우 이전에 얻어온 AID값도 clear해야 하나 cnt만 갱신하고 이전 정보를 그대로 올려주어 문제가 됨
				// 현재 상황에서 kernel을 수정하기엔 위험부담이 있으므로, count만큼만 설정하도록 AP 로직을 변경함
				//			if (m_sIccAppInform.IccAppDesc[i-1].AidLen > 0)
				if (i <= (DWORD) m_sIccAppInform.AppCnt)
				{
					strLabel.Format(L"%S",m_sIccAppInform.IccAppDesc[i-1].AppLabel);
					strLabel.TrimRight();
					strLabel.TrimLeft();

					// Display Label 
					m_pDevCmn->fnSCR_DisplayString(nIndex++ , strLabel);
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayString(nIndex++ , L"");
				}
			}
		}
		else
		{
			for (i=1; i<=EMV_MAX_AIDLIST; i++)
			{
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppCnt[%d] \n", m_sIccAppInform_V5.AppCnt));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabelLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AppLabelLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabel[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] PreferredName[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCodeTableIndex[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCodeTableIndex));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AidLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AidLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] priority[0x%x] \n", m_sIccAppInform_V5.IccAppDesc[i-1].priority));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCodeLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCntryCodeLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCode[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCntryCode));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCodeLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCodeLen));
				NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCode[0x%x 0x%x] \n", m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCode[0], m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCode[1]));

				// [#2088] NH KSK 2011.08.10 커널 버그임, remove할 경우 이전에 얻어온 AID값도 clear해야 하나 cnt만 갱신하고 이전 정보를 그대로 올려주어 문제가 됨
				// 현재 상황에서 kernel을 수정하기엔 위험부담이 있으므로, count만큼만 설정하도록 AP 로직을 변경함
				if (i <= (DWORD) m_sIccAppInform_V5.AppCnt)
				{
					// [#2188] NH KSK 2013.05.29
					if (m_sIccAppInform_V5.IccAppDesc[i-1].IsrCodeTableIndex == 1)
					{
						// [#2189] TA중 수정사항
						strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName);
						strLabel.TrimRight();
						if (strLabel.IsEmpty())
						{
							// Prefered Name이 없는 경우 label을 표시하도록 수정
							strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel);
						}
						else
						{
							strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName);
						}
						// end of [#2189]
					}
					else
					{
						strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel);
					}
//					strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel);
					// end of [#2188]

					strLabel.TrimRight();
					strLabel.TrimLeft();

					// Display Label 
					m_pDevCmn->fnSCR_DisplayString(nIndex++ , strLabel);
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayString(nIndex++ , L"");
				}
			}
		}
		// end of [#2188]
		m_pDevCmn->fnSCR_DisplayScreen(126, K_NO_WAIT, PIN_MENU_MODE);
		g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
	}

	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
	*/

	DWORD			i;
	CString			strLabel, strTemp;
	CStringArray	arrAIDName, arrAIDVG;

	// Check ADA Jack Before AID Selection.
	if( (IsAdaTransaction()) && (m_pDevCmn->fnSNS_GetEnhancedAudio()==FALSE) )
	{
		NHDEBUG(DBG_CALL, (L"ADA Transaction.... ADA JACK REMOVED => CANCEL TRANSACTION\n"));
		return RES_USER_EXIT;
	}

	// FILL AID List ====> arrAIDName
	arrAIDName.RemoveAll();
	if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
	{
		for (i=1; i<=EMV_MAX_AIDLIST; i++)
		{
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppCnt[%d] \n", m_sIccAppInform.AppCnt));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabelLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].AppLabelLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabel[%S] \n", m_sIccAppInform.IccAppDesc[i-1].AppLabel));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AidLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].AidLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] priority[0x%x] \n", m_sIccAppInform.IccAppDesc[i-1].priority));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCodeLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].IsrCntryCodeLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCode[%S] \n", m_sIccAppInform.IccAppDesc[i-1].IsrCntryCode));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCodeLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].ApSelCodeLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCode[0x%x 0x%x] \n", m_sIccAppInform.IccAppDesc[i-1].ApSelCode[0], m_sIccAppInform.IccAppDesc[i-1].ApSelCode[1]));

			// [#2088] NH KSK 2011.08.10 커널 버그임, remove할 경우 이전에 얻어온 AID값도 clear해야 하나 cnt만 갱신하고 이전 정보를 그대로 올려주어 문제가 됨
			// 현재 상황에서 kernel을 수정하기엔 위험부담이 있으므로, count만큼만 설정하도록 AP 로직을 변경함
			//if (m_sIccAppInform.IccAppDesc[i-1].AidLen > 0)
			if (i <= (DWORD) m_sIccAppInform.AppCnt)
			{
				strLabel.Format(L"%S",m_sIccAppInform.IccAppDesc[i-1].AppLabel);
				strLabel.TrimRight();
				strLabel.TrimLeft();
				arrAIDName.Add(strLabel);		// AID List
			}
		}
	}
	else // Kernel 5
	{
		for (i=1; i<=EMV_MAX_AIDLIST; i++)
		{
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppCnt[%d] \n", m_sIccAppInform_V5.AppCnt));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabelLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AppLabelLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabel[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] PreferredName[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCodeTableIndex[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCodeTableIndex));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AidLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AidLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] priority[0x%x] \n", m_sIccAppInform_V5.IccAppDesc[i-1].priority));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCodeLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCntryCodeLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCode[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCntryCode));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCodeLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCodeLen));
			NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCode[0x%x 0x%x] \n", m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCode[0], m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCode[1]));

			// [#2088] NH KSK 2011.08.10 커널 버그임, remove할 경우 이전에 얻어온 AID값도 clear해야 하나 cnt만 갱신하고 이전 정보를 그대로 올려주어 문제가 됨
			// 현재 상황에서 kernel을 수정하기엔 위험부담이 있으므로, count만큼만 설정하도록 AP 로직을 변경함
			if (i <= (DWORD) m_sIccAppInform_V5.AppCnt)
			{
				// [#2188] NH KSK 2013.05.29
				if (m_sIccAppInform_V5.IccAppDesc[i-1].IsrCodeTableIndex == 1)
				{
					// [#2189] TA중 수정사항
					strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName);
					strLabel.TrimRight();
					if (strLabel.IsEmpty())		// Prefered Name이 없는 경우 label을 표시하도록 수정
						strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel);
					else
						strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName);
					// end of [#2189]
				}
				else
				{
					strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel);
				}
				// end of [#2188]

				strLabel.TrimRight();
				strLabel.TrimLeft();
				arrAIDName.Add(strLabel);		// AID List
			}
		}
	}

	// FILL AID Voice Guidance File  ====> arrAIDName
	int	nOtherCount = 0;
	arrAIDVG.RemoveAll();
	if( IsAdaTransaction() ) 
	{
		CString strVGFile;
		#if !(US_VERSION)
		CNHReadiniFile	Readini;
		PINIVALUE		pwIniValue;
		#endif

		for(i=0; i< (DWORD) arrAIDName.GetSize(); i++)
		{
			strVGFile = L"";
			strLabel = arrAIDName.GetAt(i);		// AID Label......

			// US : SW TTS / OTHER COUNTRIES : Wave file 
			#if !(US_VERSION)
				strLabel.Replace(L" ", L"");		// Remove Spaces
				strLabel.MakeUpper();		
				BOOL bLabelFound = FALSE;

				if (Readini.Open(_T(AID_LIST_INI_FILE), CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII) != TRUE)
				{
					NHDEBUG(DBG_CALL, (L"[AP CAN NOT OPEN AIDLIST.INI]\n"));
					return RES_USER_EXIT;				// If AIDList.ini is not available, return..... READ ERROR.....
				}

				while (NULL != (pwIniValue = Readini.ReadiniValue()))
				{
					if (pwIniValue->Section == L"AIDLABEL_TABLE")
					{
						if (pwIniValue->Key == L"APPLICATIONLABEL")
						{
							strTemp = pwIniValue->Values[0];
							strTemp.Replace(L" ", L"");		// Remove Spaces
							strTemp.MakeUpper();
							if (strLabel == strTemp)		// Found Label
							{							
								bLabelFound = TRUE;
								strVGFile.Format(L"%s.wav", strLabel);
								break;
							}
						}
					}
				}
				if (bLabelFound == FALSE)			// Wave File Does not exist ===> Use "OtherX.wav"
				{
					nOtherCount++;
					strVGFile.Format(L"OTHER%d.wav", nOtherCount);
				}
				Readini.Close();
			#else
				strVGFile.Format(strLabel);
			#endif

			arrAIDVG.Add(strVGFile);
		}
	}
	CString GetKeyStr;
	int		nResult = EMV_RSLT_FAIL;
	BOOL	bShowScreen = TRUE;

	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// [#2440] NH Justin 2016.08.10 Showing AID Selection even though one AID Left after removing Blocked AID 
	if (m_pDevCmn->m_bShownMultiAIDSelection == FALSE)
		m_pDevCmn->m_bShownMultiAIDSelection = TRUE;		
	// end of [#2440]

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	CString strVGFile;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	while (g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				NVDump('O', 'C', "00", L"P_NHADA", L"NG_130");
				m_pAdaCtrl->fnExp_StopPlay();
				return RES_USER_EXIT;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		// End of [#2375]

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			if( IsAdaTransaction())
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				#if(US_VERSION)
					for( i = 0; i < (DWORD) arrAIDVG.GetSize(); i++)
					{
						m_pAdaCtrl->fnExp_AddWaveFile(126, L"For.wav");
						m_pAdaCtrl->fnExp_AddWaveFile(126, arrAIDVG.GetAt(i));
						strTemp.Format(L"Press%d.wav", i+1);
						m_pAdaCtrl->fnExp_AddWaveFile(126, strTemp);
					}
					m_pAdaCtrl->fnExp_AddWaveFile(126, L"LocateCancelKey.wav");
					m_pAdaCtrl->fnExp_AddWaveFile(126, L"LocationRepeatKey.wav");
				#else
					for( i = 0; i < (DWORD) arrAIDVG.GetSize(); i++)
					{
						strTemp.Format(L"Press%dFor.wav", i+1);
						m_pAdaCtrl->fnExp_AddWaveFile(126, strTemp);
						m_pAdaCtrl->fnExp_AddWaveFile(126, arrAIDVG.GetAt(i));
					}
				#endif
				m_pAdaCtrl->fnExp_PlayScreenWave(126);
				bStartToTimeout = TRUE;
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(126);

				// Title
				m_pDevCmn->fnSCR_DisplayString(2, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_126001));

				// Sub Title
				m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_126002));

				// AIDs
				int nIndex = 4;
				for (i=0; i<EMV_MAX_AIDLIST; i++)
				{
					if( i< (DWORD) arrAIDName.GetSize() )	strLabel = arrAIDName.GetAt(i);
					else									strLabel = L"";
					m_pDevCmn->fnSCR_DisplayString(nIndex++ , strLabel);
				}
				m_pDevCmn->fnSCR_DisplayScreen(126, K_NO_WAIT, PIN_MENU_MODE);
				g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			}
			bShowScreen = FALSE;
		}

		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2375]

		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));
			GetKeyStr.TrimRight();
			GetKeyStr.TrimLeft();
			
			// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
			bInvalidRepeat = FALSE;
			if( (IsAdaTransaction())&&(GetKeyStr.GetLength()>6) )
			{
				GetKeyStr = GetKeyStr.Mid(6);
				NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE AFTER ADA(Mid(6)) =  [%s]\n"), GetKeyStr));
			}
			// End of [#2375]

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				// [#2375] US Justin Combine ADA and Screen Flow
				if( IsAdaTransaction())		
				{
					#if (US_VERSION)
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"Cancel.wav", TRUE);
					#endif
				}
				// End of [#2375]
				return RES_USER_EXIT;
			}
			else
			{
				// [#2375] US Justin 2015.11.02 Combine ADA and Screen Transaction Flows
				/*
				// Index가 온다는 가정하에 작업해야함 (화면팀과 Interface 협의 필요) (1 Base)
				m_nSelectAID = Asc2Int(GetKeyStr) - 1;

				// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
				if (m_nSelectAID < 0)
					return RES_USER_EXIT;
				// end of [#2022]
				*/

				BOOL bContinueSelection = TRUE;
				if ( IsAdaTransaction()==FALSE )		// Screen mode transaction
				{
					m_nSelectAID = Asc2Int(GetKeyStr) - 1;
					if (m_nSelectAID < 0)
						return RES_USER_EXIT;
				}
				else									// Voice Guidance mode transaction
				{
					bContinueSelection = FALSE;
					int nSelected = -1;

					int nValidADAInput = 0;
					if( (GetKeyStr == L".")||(GetKeyStr == L"00")||(GetKeyStr == L"000") )
					{
						nValidADAInput = 2;								// (2) Volume Control or Repeat
						m_pAdaCtrl->fnExp_StopPlay();
						if(GetKeyStr == L".")			m_pAdaCtrl->fnExp_SetVolumeDown();
						else if(GetKeyStr == L"00")		m_pAdaCtrl->fnExp_SetVolumeUp();
					}
					else if( GetKeyStr.GetLength()==1)
					{
						nSelected = Asc2Int(GetKeyStr) - 1;
						if( (nSelected>=0) && (nSelected < arrAIDName.GetSize()) )
						{
							nValidADAInput = 1;							// (1) Valid Input
							strVGFile.Format(L"%d.wav", nSelected + 1);
						}
					}

					if( nValidADAInput == 0 )	// Invalid Input
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : INVALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
						#else
							m_pAdaCtrl->fnExp_StopAndResetWaveFile();
							m_pAdaCtrl->fnExp_AddWaveFile(1,L"InvalidKey.wav");
							m_pAdaCtrl->fnExp_PlayScreenWave(1);
							m_pAdaCtrl->fnExp_WaitUntilStop();
							bShowScreen = TRUE;
						#endif
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
					}
					else if( nValidADAInput == 1 )	// Valid Input
					{
						m_nSelectAID = nSelected;
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : VALID INPUT\n"));
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(strVGFile, TRUE, arrAIDVG.GetAt(nSelected));
						#endif

						bContinueSelection = TRUE;
					}
					else if( nValidADAInput == 2)			// Repeat or Volume Control
					{
						NHDEBUG(DBG_INFO, (L"ADA Voice Guidance Input : Volume Control or Repeat\n"));
						g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						bShowScreen = TRUE;
					}
				}

				if(bContinueSelection)
				{
					if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
					{
						m_nSelectedTermDataIndex = BIZ_EMV_GetTerminalDataIndex(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName,
							m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen);
						NHDEBUG(DBG_CALL, (L"[BIZ_EMV_GetTerminalDataIndex]\n"));

						// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
						if (m_nSelectedTermDataIndex < 0)
							return RES_USER_EXIT;
						// end of [#2022]

						// Select Final Application
						nResult = m_pDevCmn->fnEMV_Sel_FinalAppSelection(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen,
							m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName,
							&m_chSW);
						NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_FinalAppSelection]\n"));
						if (nResult != EMV_RSLT_OK)
						{
							if (nResult == EMV_ERR_NOT_ACCEPT)
							{
								// Error 발생 시 다른 AID List가 있으면 Error Message 표시
								if (m_nCandidateList > 1)
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_sIccAppInform.AppCnt, 
										m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName);
									NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
									m_nCandidateList--;

									if (m_pDevCmn->fnSNS_GetEnhancedAudio())
									{
										m_pAdaCtrl->fnExp_StopAndResetWaveFile();
										#if (US_VERSION)
											m_pAdaCtrl->fnExp_AddWaveFile(122, L"122.wav");		// Failed A-I-D Selection
										#endif
										m_pAdaCtrl->fnExp_PlayScreenWave(122);
										m_pAdaCtrl->fnExp_WaitUntilStop();

									}
									else
									{
										LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122301), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
										/*
										m_pDevCmn->fnSCR_DisplayPrevSet(122);
										m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
										m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122301));
										m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
										m_pDevCmn->fnSCR_DisplayScreen(122);
										m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
										*/
									}
									return RES_MAKE_AID;
								}
								else
								{
									return RES_EMV_TERMINATE;
								}
							}
							else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
								return RES_EMV_FALLBACK;
							else
								return RES_EMV_TERMINATE;

						}
					}
					else
					{
						m_nSelectedTermDataIndex = BIZ_EMV_GetTerminalDataIndex(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName,
							m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen);
						NHDEBUG(DBG_CALL, (L"[BIZ_EMV_GetTerminalDataIndex]\n"));

						// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
						if (m_nSelectedTermDataIndex < 0)
							return RES_USER_EXIT;
						// end of [#2022]

						// Select Final Application
						nResult = m_pDevCmn->fnEMV_Sel_FinalAppSelection(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
							m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName,
							&m_chSW);
						NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_FinalAppSelection]\n"));
						if (nResult != EMV_RSLT_OK)
						{
							if (nResult == EMV_ERR_NOT_ACCEPT)
							{
								// Error 발생 시 다른 AID List가 있으면 Error Message 표시
								if (m_nCandidateList > 1)
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_sIccAppInform_V5.AppCnt, 
										m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName);
									NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
									m_nCandidateList--;

									if (m_pDevCmn->fnSNS_GetEnhancedAudio())
									{
										m_pAdaCtrl->fnExp_StopAndResetWaveFile();
										#if (US_VERSION)
											m_pAdaCtrl->fnExp_AddWaveFile(122, L"122.wav");		// Failed A-I-D Selection
										#endif
										m_pAdaCtrl->fnExp_PlayScreenWave(122);
										m_pAdaCtrl->fnExp_WaitUntilStop();

									}
									else
									{
										LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122301), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
										/*
										m_pDevCmn->fnSCR_DisplayPrevSet(122);
										m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
										m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122301));
										m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
										m_pDevCmn->fnSCR_DisplayScreen(122);
										m_pDevCmn->fstrSCR_WaitTime(ABORT_SCR_TIMEOUT);
										*/
									}
									return RES_MAKE_AID;
								}
								else
								{
									return RES_EMV_TERMINATE;
								}
							}
							else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
								return RES_EMV_FALLBACK;
							else
								return RES_EMV_TERMINATE;
						}
					}
					// end of [#2188]

					// KSK 2010.07.22 Final App Selection 이후 Init Application을 수행하여 실패 시 Make APP List를 수행하도록 함 (EMV 사양임)
					//------------------------------------------------------------------------
					BIZ_EMV_StoreDefaultTerminalValue();

					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					nResult = m_pDevCmn->fnEMV_Trans_InitApplication();	
					CString strTemp;
					strTemp.Format(L"InitAppProc:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp );

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_ACCEPT)
						{
							if (m_nCandidateList > 1)
							{
								// [#2188] NH KSK 2013.05.06
								if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName);
								}
								else
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName);
								}
								// end of [#2188]

								m_nCandidateList--;
								if (m_pDevCmn->fnSNS_GetEnhancedAudio())
								{
									m_pAdaCtrl->fnExp_StopAndResetWaveFile();
									#if (US_VERSION)
										m_pAdaCtrl->fnExp_AddWaveFile(122, L"122.wav");		// Failed A-I-D Selection
									#endif
									m_pAdaCtrl->fnExp_PlayScreenWave(122);
									m_pAdaCtrl->fnExp_WaitUntilStop();

								}
								else
								{
									LIB_UserPopUpNotice(SCR_ICON_STOP, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122301), ABORT_SCR_TIMEOUT);	// [#2377] NH Justin Make a common Function
									/*
									m_pDevCmn->fnSCR_DisplayPrevSet(122);
									m_pDevCmn->fnSCR_DisplayString(1, SCR_ICON_STOP);
									m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_122301));
									m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
									m_pDevCmn->fnSCR_DisplayScreen(122);
									// No Timeout ???? ===>  BUG ????
									*/
								}
								return RES_MAKE_AID;
							}
							else
							{
								// KSK 2010.08.24 VISA Test Case 12 Issue 대응
								// Kernel 문서에는 Terminate로 되어져 있으나, CE는 Combined Reader이므로, FallBack을 하도록 수정
								// Interac , Master Card에는 없는 Test Case임.
								// return RES_EMV_TERMINATE;
								return RES_EMV_FALLBACK;
							}
						}
						else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					if( !IsAdaTransaction() ) 
					{
						// [#2464] AU KSK 2016.12.23 ReadApp시 시간이 오래 소요되어 화면 추가함
						// DISPLAY SCREEN (IC Processing)
						m_pDevCmn->fnSCR_DisplayPrevSet(125);
						#if (MX_VERSION)
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125002));
						#else
						m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125001));
						#endif
						m_pDevCmn->fnSCR_DisplayScreen(125);
						// end of [#2464]

					}
					//[#2173] NH Justin 2013.01.15 Reading Card Track data first (IC Card) for TH V2 - US and Canada
 //					#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거
					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					NHDEBUG(DBG_CALL, (L"[CTranCmn::fnEMV_Trans_ReadAppData]\n"));
					nResult = m_pDevCmn->fnEMV_Trans_ReadAppData();
					strTemp.Format(L"ReadAppData:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp);

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					CString strIDCTrack2;
					unsigned char szTempa[1024];
					int				nLen = 0;
					memset(szTempa, 0, sizeof(szTempa));

					/* Get 2 Track data from IC */
					if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_Track2EquData, &nLen, szTempa) == EMV_RSLT_OK)
					{
						strIDCTrack2 = MakeUnPack(szTempa, nLen);
						NHDEBUG(DBG_INFO, (L"Success to Get track 2 data from IC\n"));
						NHDEBUG(DBG_INFO, (L"IDC Track2 : [%s]\n", strIDCTrack2));
					}
					else
					{
						NHDEBUG(DBG_INFO, (L"Failed to Get track 2 data from IC\n"));
						return RES_EMV_TERMINATE;
					}
					BIZ_RETURN	nRes = BIZ_EMV_ICCardRead(strIDCTrack2);
						
					if (nRes != RES_OK)
						return RES_EMV_TERMINATE;
//					#endif	// End of [#2173]			// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거

#if (AU_VERSION)	// [#2379] AU KSK 2015.11.17
					if (m_pDevCmn->m_bDomesticFallbackTrans == TRUE)
					{
						NVDump('O', 'C', "53", L"", L"D_BIN_DETECT" );
						return RES_EMV_FALLBACK;
					}
#endif				// end of [#2379]

					// APP NAME and AID SAVE
					// [#2188] NH KSK 2013.05.06
					CString strAID;				// [#2252] US Justin Long AID Bug Fix
					CString strAPPName;			// [#2252] US Justin Long AID Bug Fix
					if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
					{
						// [#2252] US Justin 2014.02.04 Long AID Bug Fix
						//strAID.Format(L"%-17.17S = %-20.20s", m_sIccAppInform.IccAppDesc[m_nSelectAID].AppLabel, 
						//									  MakeUnPack(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen));	// [#506] [CA] KSK 2009.3.23
						strAID.Format(L"%s", MakeUnPack(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen));
						strAPPName.Format(L"%S", m_sIccAppInform.IccAppDesc[m_nSelectAID].AppLabel); 
						// End of [#2252]
					}
					else
					{
						strAID.Format(L"%s", MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen) ); // [#2252] US Justin 2014.02.04
						// [#2188] NH KSK 2013.05.29
						if (m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].IsrCodeTableIndex == 1)
						{
							// [#2189] TA중 수정사항
							CString strLabel;
							strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName);
							strLabel.TrimRight();
							if (strLabel.IsEmpty())
							{
								//strAID.Format(L"%-17.17S = %-20.20s", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel, 
								//									  MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));	// [#506] [CA] KSK 2009.3.23
								strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel);		// [#2252] US Justin 2014.02.04 Long AID Bug Fix
							}
							else
							{
								//strAID.Format(L"%-17.17S = %-20.20s", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName, 
								//	MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));	// [#506] [CA] KSK 2009.3.23
								strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName);// [#2252 US Justin 2014.02.04 Long AID Bug Fix
							}
						}
						else
						{
							//strAID.Format(L"%-17.17S = %-20.20s", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel, 
							//	MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));	// [#506] [CA] KSK 2009.3.23
							strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel);		// [#754] US Justin 2014.02.04 Long AID Bug Fix
						}
						// end of [#2188]
					}
					MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID, strAID);
					MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME, strAPPName);					// [#2252] US Justin 2014.02.04 Long AID Bug Fix
					NHDEBUG(DBG_CALL, (L"AID inform[%s=%s]\n", strAPPName, strAID));
					// end of [#2188]
					return RES_NOR_MS_TRANS;
				}
				// End of [#2375]
			}
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

	if (g_TimeCheck.IsElapsedTimes() == TRUE)
		return RES_USER_TIMEOUT;

	return RES_EMV_TERMINATE;		// 비정상적인 경우 Terminate 처리
}


#if !(US_VERSION || CA_VERSION)
//[#2077] CA PCS 2011.07.06
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_SelectMultiAIDList()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : -
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::P_EMV_ADA_SelectMultiAIDList()
{
	NHDEBUG(DBG_CALL, (L"[P_EMV_ADA_SelectMultiAIDList()]\n"));
	CString GetKeyStr;
	CString strSourceLabel, strDestLabel;
	int		nResult = EMV_RSLT_FAIL;

	// [#2240] US Justin 2013.12.11 EMV VG
	BOOL			bPlayGuidance = TRUE;
	BOOL			bStartToTimeout = FALSE;
	// End of [#2240]
	DWORD	i;

	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //"타임아웃 설정을 음성 재생 후로 변경"	// [#2240] US Justin 2013.12.11 EMV VG
	while(g_TimeCheck.IsElapsedTimes() == FALSE)	// [#2240] US Justin 2013.12.11 EMV VG
	{
		///////////////////////////////////
		// CHECK STATUS
		if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
		{
			return RES_USER_EXIT;
		}

		///////////////////////////////////
		// SHOW SCREEN & PLAY GUIDANCE
		if (bPlayGuidance == TRUE)	// [#2240] US Justin 2013.12.11 EMV VG
		{

			// [#2240] US Justin 2013.12.11 EMV VG
			// Show Screen
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);
			}
			// End of [#2240]

			// Play Guidance
			{
				CString					strDataFileName;
				BOOL					bCheck=FALSE;						
				strDataFileName.Format(L"%S", _T(AID_LIST_INI_FILE)); 
				
				//공통부분 보이스 초기화
				m_pAdaCtrl->fnExp_StopPlay();
				m_pAdaCtrl->fnExp_ResetScrWaveFile();

				CString	 strPlayAIDLabelWavFile;
				CString	 strPlayListCount;
				int		 nOtherCount = 0;
				
				CNHReadiniFile	Readini;
				PINIVALUE		pwIniValue;

				if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)	// [#2188] NH KSK 2013.05.29
				{
					for (i=1; i<=EMV_MAX_AIDLIST; i++)
					{
						/* Display not Selected Application ID's */
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppCnt[%d] \n", m_sIccAppInform.AppCnt));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabelLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].AppLabelLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabel[%S] \n", m_sIccAppInform.IccAppDesc[i-1].AppLabel));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AidLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].AidLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] priority[0x%x] \n", m_sIccAppInform.IccAppDesc[i-1].priority));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCodeLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].IsrCntryCodeLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCode[%S] \n", m_sIccAppInform.IccAppDesc[i-1].IsrCntryCode));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCodeLen[%d] \n", m_sIccAppInform.IccAppDesc[i-1].ApSelCodeLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCode[0x%x 0x%x] \n", m_sIccAppInform.IccAppDesc[i-1].ApSelCode[0], m_sIccAppInform.IccAppDesc[i-1].ApSelCode[1]));

						// [#2088] NH KSK 2011.08.10 커널 버그임, remove할 경우 이전에 얻어온 AID값도 clear해야 하나 cnt만 갱신하고 이전 정보를 그대로 올려주어 문제가 됨
						// 현재 상황에서 kernel을 수정하기엔 위험부담이 있으므로, count만큼만 설정하도록 AP 로직을 변경함
						//			if (m_sIccAppInform.IccAppDesc[i-1].AidLen > 0)
						if (i <= (DWORD) m_sIccAppInform.AppCnt)
						{
							strSourceLabel.Format(L"%S",m_sIccAppInform.IccAppDesc[i-1].AppLabel);
							strSourceLabel.Replace(L" ", L"");	// 모든 Space 제거
							strSourceLabel.MakeUpper();

							if (Readini.Open(_T(AID_LIST_INI_FILE), CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII) != TRUE)
							{
								NHDEBUG(DBG_CALL, (L"[AP CAN NOT OPEN AIDLIST.INI]\n"));
								return RES_USER_EXIT;		// If AIDList.ini is not available, return..... READ ERROR.....
							}
	
							BOOL bLabelFound = FALSE;
							while (NULL != (pwIniValue = Readini.ReadiniValue()))
							{
								if (pwIniValue->Section == L"AIDLABEL_TABLE")
								{
									if (pwIniValue->Key == L"APPLICATIONLABEL")
									{
										strDestLabel = pwIniValue->Values[0];
										strDestLabel.Replace(L" ", L"");	// 모든 Space 제거
										strDestLabel.MakeUpper();
										if (strSourceLabel == strDestLabel)
										{
											// Found Label
											bLabelFound = TRUE;
											strPlayListCount.Format(L"Press%dFor.wav", i);
											strPlayAIDLabelWavFile.Format(L"%s.wav", strDestLabel);

											m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayListCount);
											m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayAIDLabelWavFile);
											break;
										}
									}
								}
							}
							if (bLabelFound == FALSE)
							{
								// Other로 등록함
								nOtherCount++;
								strPlayListCount.Format(L"Press%dFor.wav", i);
								strPlayAIDLabelWavFile.Format(L"OTHER%d.wav", nOtherCount);

								m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayListCount);
								m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayAIDLabelWavFile);
							}
							Readini.Close();
						}
					}
				}
				else // Kernel 5
				{
					for (i=1; i<=EMV_MAX_AIDLIST; i++)
					{
						/* Display not Selected Application ID's */
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppCnt[%d] \n", m_sIccAppInform_V5.AppCnt));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabelLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AppLabelLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AppLabel[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] PreferredName[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCodeTableIndex[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCodeTableIndex));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] AidLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].AidLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] priority[0x%x] \n", m_sIccAppInform_V5.IccAppDesc[i-1].priority));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCodeLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCntryCodeLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] IsrCntryCode[%S] \n", m_sIccAppInform_V5.IccAppDesc[i-1].IsrCntryCode));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCodeLen[%d] \n", m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCodeLen));
						NHDEBUG(DBG_CALL, (L"[fnEMV_Multi AID DisplayAppList] ApSelCode[0x%x 0x%x] \n", m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCode[0], m_sIccAppInform_V5.IccAppDesc[i-1].ApSelCode[1]));

						if (i <= (DWORD) m_sIccAppInform_V5.AppCnt)
						{
							strSourceLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].AppLabel);
							strSourceLabel.Replace(L" ", L"");	// 모든 Space 제거
							strSourceLabel.MakeUpper();
							
							if (Readini.Open(_T(AID_LIST_INI_FILE), CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII) != TRUE)
							{
								NHDEBUG(DBG_CALL, (L"[AP CAN NOT OPEN AIDLIST.INI]\n"));
								return RES_USER_EXIT;		// If AIDList.ini is not available, return..... READ ERROR.....
							}
	
							BOOL bLabelFound = FALSE;
							while (NULL != (pwIniValue = Readini.ReadiniValue()))
							{
								if (pwIniValue->Section == L"AIDLABEL_TABLE")
								{
									if (pwIniValue->Key == L"APPLICATIONLABEL")
									{
										strDestLabel = pwIniValue->Values[0];
										strDestLabel.Replace(L" ", L"");	// 모든 Space 제거
										strDestLabel.MakeUpper();
										if (strSourceLabel == strDestLabel)
										{
											// Found Label
											bLabelFound = TRUE;
											strPlayListCount.Format(L"Press%dFor.wav", i);
											strPlayAIDLabelWavFile.Format(L"%s.wav", strDestLabel);

											m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayListCount);
											m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayAIDLabelWavFile);
											break;
										}
									}
								}
							}

							if (bLabelFound == FALSE)
							{
								// Other로 등록함
								nOtherCount++;
								strPlayListCount.Format(L"Press%dFor.wav", i);
								strPlayAIDLabelWavFile.Format(L"OTHER%d.wav", nOtherCount);

								m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayListCount);
								m_pAdaCtrl->fnExp_AddWaveFile(126,strPlayAIDLabelWavFile);
							}
							Readini.Close();
						}
					}
				}
					
				// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(126, L"RepeatInfo.wav");		
#endif
				// End of [#2240]

				//보이스 재생.
				m_pAdaCtrl->fnExp_PlayScreenWave(126);
			}

			// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE;
			// End of [#2240]
		}

		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
		if ((m_pAdaCtrl->IsPlaying() == FALSE) && (bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}
		// End of [#2240]


//	g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_TIMEOUT);
//	while (g_TimeCheck.IsElapsedTimes() == FALSE)
//	{
		if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE)) //[#2077] CA PCS 2011.07.21
		{
			return RES_USER_EXIT;
		}
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString(1);
			GetKeyStr = GetKeyStr.Mid(6);

			NHDEBUG(DBG_INFO, (_T("SCREEN RETURN VALUE [%s]\n"), GetKeyStr));

			GetKeyStr.TrimRight();
			GetKeyStr.TrimLeft();

			if (GetKeyStr == S_CANCEL || GetKeyStr == S_EXIT)
			{
				//[#2240] US Justin 2013.12.11 US EMV VG
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
				//end of [#2240]
				return RES_USER_EXIT;
			}
			//[#2240] US Justin 2013.12.11 US EMV VG
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
			// End of [#2240]
			else
			{
				if (Asc2Int(GetKeyStr) < 1 || Asc2Int(GetKeyStr) > m_nCandidateList)
				{
				// [#2240] US Justin 2013.12.11 EMV VG
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
				// End of [#2240]
				}
				else
				{

				//[#2240] US Justin 2013.12.16 US EMV VG
#if (US_VERSION)
					CString strTempVG;
					strTempVG.Format(L"%s.wav", GetKeyStr);
					m_pAdaCtrl->fnExp_StopPlay();
					m_pAdaCtrl->fnExp_ResetScrWaveFile();
					m_pAdaCtrl->fnExp_AddWaveFile(1,strTempVG);
					m_pAdaCtrl->fnExp_PlayScreenWave(1);
					m_pAdaCtrl->fnExp_WaitUntilStop();
#endif
				// End of [#2240]

					m_nSelectAID = Asc2Int(GetKeyStr) -1;
				
					// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
					if (m_nSelectAID < 0)
						return RES_USER_EXIT;
					// end of [#2022]

					// [#2188] NH KSK 2013.05.29
					if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
					{
						m_nSelectedTermDataIndex = BIZ_EMV_GetTerminalDataIndex(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName,
							m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen);
						NHDEBUG(DBG_CALL, (L"[BIZ_EMV_GetTerminalDataIndex]\n"));

						// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
						if (m_nSelectedTermDataIndex < 0)
							return RES_USER_EXIT;
						// end of [#2022]

						// Select Final Application
						nResult = m_pDevCmn->fnEMV_Sel_FinalAppSelection(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen, 
							m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName, &m_chSW);

						NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_FinalAppSelection]\n"));

						if (nResult != EMV_RSLT_OK)
						{
							if (nResult == EMV_ERR_NOT_ACCEPT)
							{
								// Error 발생 시 다른 AID List가 있으면 Error Message 표시
								if (m_nCandidateList > 1)
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_sIccAppInform.AppCnt, 
										m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName);
									NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
									m_nCandidateList--;

									m_pAdaCtrl->fnExp_ResetScrWaveFile();//[#2077] CA PCS 2011.07.07 [122.wav 추가]
									m_pAdaCtrl->fnExp_PlayScreenWave(122);
									m_pAdaCtrl->fnExp_WaitUntilStop();

									return RES_MAKE_AID;
								}
								else
								{
									return RES_EMV_TERMINATE;
								}
							}
							else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
								return RES_EMV_FALLBACK;
							else
								return RES_EMV_TERMINATE;

						}
					}
					else
					{
						m_nSelectedTermDataIndex = BIZ_EMV_GetTerminalDataIndex(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName,
							m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen);
						NHDEBUG(DBG_CALL, (L"[BIZ_EMV_GetTerminalDataIndex]\n"));

						// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
						if (m_nSelectedTermDataIndex < 0)
							return RES_USER_EXIT;
						// end of [#2022]

						// Select Final Application
						nResult = m_pDevCmn->fnEMV_Sel_FinalAppSelection(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen, 
							m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, &m_chSW);

						NHDEBUG(DBG_CALL, (L"[fnEMV_Sel_FinalAppSelection]\n"));

						if (nResult != EMV_RSLT_OK)
						{
							if (nResult == EMV_ERR_NOT_ACCEPT)
							{
								// Error 발생 시 다른 AID List가 있으면 Error Message 표시
								if (m_nCandidateList > 1)
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_sIccAppInform_V5.AppCnt, 
										m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName);
									NHDEBUG(DBG_CALL, (L"[fvEMV_Sel_RemoveCandidateList]\n"));
									m_nCandidateList--;

									m_pAdaCtrl->fnExp_ResetScrWaveFile();//[#2077] CA PCS 2011.07.07 [122.wav 추가]
									m_pAdaCtrl->fnExp_PlayScreenWave(122);
									m_pAdaCtrl->fnExp_WaitUntilStop();

									return RES_MAKE_AID;
								}
								else
								{
									return RES_EMV_TERMINATE;
								}
							}
							else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
								return RES_EMV_FALLBACK;
							else
								return RES_EMV_TERMINATE;

						}
					}
					// end of [#2188]

					// KSK 2010.07.22 Final App Selection 이후 Init Application을 수행하여 실패 시 Make APP List를 수행하도록 함 (EMV 사양임)
					//------------------------------------------------------------------------
					BIZ_EMV_StoreDefaultTerminalValue();

					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					nResult = m_pDevCmn->fnEMV_Trans_InitApplication();	
					CString strTemp;
					strTemp.Format(L"InitAppProc:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp );

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_ACCEPT)
						{
							if (m_nCandidateList > 1)
							{
								// [#2188] NH KSK 2013.05.29
								if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName);
								}
								else
								{
									m_pDevCmn->fvEMV_Sel_RemoveCandidateList(m_nCandidateList, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen,
										m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName);
								}
								// end of [#2188]

								m_nCandidateList--;						

								m_pAdaCtrl->fnExp_ResetScrWaveFile();//[#2077] CA PCS 2011.07.07 [122.wav 추가]
								m_pAdaCtrl->fnExp_PlayScreenWave(122);
								m_pAdaCtrl->fnExp_WaitUntilStop();
								
								return RES_MAKE_AID;
							}
							else
							{
								// KSK 2010.08.24 VISA Test Case 12 Issue 대응
								// Kernel 문서에는 Terminate로 되어져 있으나, CE는 Combined Reader이므로, FallBack을 하도록 수정
								// Interac , Master Card에는 없는 Test Case임.
								// return RES_EMV_TERMINATE;
								return RES_EMV_FALLBACK;
							}
						}
						else if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					//[#2173] NH Justin 2013.01.15 Reading Card Track data first (IC Card) for TH V2 - US and Canada
//#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거
					NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
					NHDEBUG(DBG_CALL, (L"[CTranCmn::fnEMV_Trans_ReadAppData]\n"));
					nResult = m_pDevCmn->fnEMV_Trans_ReadAppData();
					strTemp.Format(L"ReadAppData:%d", nResult);
					NVDump('O', 'C', "53", L"", strTemp);

					if (nResult != EMV_RSLT_OK)
					{
						if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
							return RES_EMV_FALLBACK;
						else
							return RES_EMV_TERMINATE;
					}

					CString strIDCTrack2;
					unsigned char szTempa[1024];
					int				nLen = 0;
					memset(szTempa, 0, sizeof(szTempa));

					/* Get 2 Track data from IC */
					if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_Track2EquData, &nLen, szTempa) == EMV_RSLT_OK)
					{
						strIDCTrack2 = MakeUnPack(szTempa, nLen);
						NHDEBUG(DBG_INFO, (L"Success to Get track 2 data from IC\n"));
						NHDEBUG(DBG_INFO, (L"IDC Track2 : [%s]\n", strIDCTrack2));
					}
					else
					{
						NHDEBUG(DBG_INFO, (L"Failed to Get track 2 data from IC\n"));
						return RES_EMV_TERMINATE;
					}
					BIZ_RETURN	nRes = BIZ_EMV_ICCardRead(strIDCTrack2);
					
					if (nRes != RES_OK)
						return RES_EMV_TERMINATE;
//#endif		// End of [#2173]		// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거

#if (AU_VERSION)	// [#2379] AU KSK 2015.11.17
					if (m_pDevCmn->m_bDomesticFallbackTrans == TRUE)
					{
						NVDump('O', 'C', "53", L"", L"D_BIN_DETECT" );
						return RES_EMV_FALLBACK;
					}
#endif				// end of [#2379]

					// APP NAME and AID SAVE
					CString strAID, strAPPName;	// [#2252] 2014.02.07 Long AID

					// [#2188] NH KSK 2013.05.29
					if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
					{
						// [#2252] 2014.02.07 Long AID
						strAID.Format(L"%s", MakeUnPack(m_sIccAppInform.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform.IccAppDesc[m_nSelectAID].AidLen));	
						strAPPName.Format(L"%S", m_sIccAppInform.IccAppDesc[m_nSelectAID].AppLabel);	
						// End of [#2252]
					}
					else
					{
						strAID.Format(L"%s", MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen) );	// [#2252] US Justin 2014.02.07
						if (m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].IsrCodeTableIndex == 1)
						{
							// [#2189] TA중 수정사항
							CString strLabel;
							strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[i-1].PreferredName);
							strLabel.TrimRight();
							if (strLabel.IsEmpty())
							{
								// [#2252] US Justin 2014.02.07
								strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel );
								// end of [#2252]
							}
							else
							{
								// [#2252] US Justin 2014.02.07
								strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName );
								// end of [#2252]
							}
						}
						else
						{
							// [#2252] US Justin 2014.02.07
							strAPPName.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel );
							// end of [#2252]
						}
						// [#2252] US Justin 2014.02.07
						//MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID, strAID);
						//NHDEBUG(DBG_CALL, (L"AID[%s] Len[%d]\n", MakeUnPack(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen), m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen));
						// end of [#2252]
					}

					// [#2252] 2014.02.07 Long AID
					MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID, strAID);
					MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME, strAPPName);
					NHDEBUG(DBG_INFO, (L"AID[%s=%s]\n",strAPPName, strAID) );
					// End of [#2252]
					// end of [#2188]

					return RES_NOR_MS_TRANS;
				}
			}
		}
		Delay_Msg(50);
	}
	return RES_EMV_TERMINATE;		// 비정상적인 경우 Terminate 처리
}
#endif

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_ICProcessing()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_EMV_ICProcessing()
{
	BIZ_RETURN	nRes = RES_OK;		// KSK 2013.04.05 AU 컴파일 에러 수정
	CString		strTemp;
	int			nResult = 0;
	
	/* "Please wait a moment" */
	{
		///////////////////////////////////
		//1. ADA 에서 들어온 경우
		if (IsAdaTransaction() == TRUE && m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE)	// [#2077]
		{
			NHDEBUG(DBG_INFO, (_T("[P_EMV_ICPROCESSING][ADA PLAY WAVE]\n")));
			// [#2375] US Justin 2015.11.02 US ADA
			//m_pAdaCtrl->fnExp_PlayScreenWave(125); //please wait a moment.
			m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
			m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_DISABLE_MODE);

			m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			#if (US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(125, L"125.wav");
			#endif
			m_pAdaCtrl->fnExp_PlayScreenWave(125);	//"Please Wait a moment" 125.wav
			m_pAdaCtrl->fnExp_WaitUntilStop();
			// End of [#2375]
		}																		// end of [#2077]
		//2 ADA 거래가 아닌 경우
		else
		{
			// DISPLAY SCREEN
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
			m_pDevCmn->fnSCR_DisplayPrevSet(125);
		// [#2164] MX Justin 2012.11.12 Change Mexico Mode Screen Text
#if (MX_VERSION)
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125002));
#else
			m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125001));
#endif
		// End of [#2164]

			// [#2188] NH KSK 2013.05.29
			if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
			{
				strTemp.Format(L"%S", m_sIccAppInform.IccAppDesc[m_nSelectAID].AppLabel);
			}
			else
			{
				if (m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].IsrCodeTableIndex == 1)
				{
					// [#2189] TA 인증 중 이슈 대응
					CString strLabel;
					strLabel.Format(L"%S",m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName);
					strLabel.TrimRight();
					if (strLabel.IsEmpty())
					{
						strTemp.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel);
					}
					else
					{
						strTemp.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName);
					}
//					strTemp.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].PreferredName);
					// end of [#2189]
				}
				else
				{
					strTemp.Format(L"%S", m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AppLabel);
				}
			}
			strTemp.TrimLeft();
			strTemp.TrimRight();
			m_pDevCmn->fnSCR_DisplayString(4, strTemp);
			// end of [#2188]
			m_pDevCmn->fnSCR_DisplayScreen(125);
		}
	}

	BIZ_EMV_StoreTransactionValue();
	strTemp.Format(L"StoreTermData");
	NVDump('O', 'C', "53", L"", strTemp);

	// [#2115] MX KSK 2012.01.28 멕시코 특이사양(5F28) 처리를 위해 Read Record 위치 변경	
	//------------------------------------------------------------------------
//#if !(MX_VERSION || US_VERSION || CA_VERSION)		//[#2173] NH Justin 2013.01.16 Read Track2 Data At first => Dynamic Flow
//	//------------------------------------------------------------------------
//	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
//	NHDEBUG(DBG_CALL, (L"[CTranCmn::fnEMV_Trans_ReadAppData]\n"));
//	nResult = m_pDevCmn->fnEMV_Trans_ReadAppData();
//
//	strTemp.Format(L"ReadAppData:%d", nResult);
//	NVDump('O', 'C', "53", L"", strTemp );
//
//	if (nResult != EMV_RSLT_OK)
//	{
//		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
//			return RES_EMV_FALLBACK;
//		else
//			return RES_EMV_TERMINATE;
//	}
//#endif	// end of [#2115]

	//------------------------------------------------------------------------
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"Call : [fnEMV_Trans_OfflineDataAuth]\n"));

	nResult = m_pDevCmn->fnEMV_Trans_OfflineDataAuth(NULL);

	strTemp.Format(L"OffAuth:%d", nResult);
	NVDump('O', 'C', "53", L"", strTemp );

	if (nResult < EMV_RSLT_FAIL)
	{
		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
			return RES_EMV_FALLBACK;
		else
			return RES_EMV_TERMINATE;
	}
	//------------------------------------------------------------------------
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"Call : [fnEMV_Trans_ProcessRestrict]\n"));
	nResult = m_pDevCmn->fnEMV_Trans_ProcessRestrict();

	strTemp.Format(L"ProcRest:%d", nResult);
	NVDump('O', 'C', "53", L"", strTemp );

	if (nResult < EMV_RSLT_FAIL)
	{
		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
			return RES_EMV_FALLBACK;
		else
			return RES_EMV_TERMINATE;
	}
	//------------------------------------------------------------------------

	/************************************************************************/
	/* Get IC Card Data & Make PAN Data										*/
	/************************************************************************/
	CString strIDCTrack2;
	unsigned char szTempa[1024];
	int				nLen = 0;

	memset(szTempa, 0, sizeof(szTempa));

	// [#2379] AU KSK 2015.11.18 AU도 적용되어 국가 Define 제거
	//[#2173] NH Justin 2013.01.16 Read Track2 Data At first => Dynamic Flow	
//#if !(MX_VERSION || US_VERSION || CA_VERSION)
	/* Get 2 Track data from IC */
//	if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_Track2EquData, &nLen, szTempa) == EMV_RSLT_OK)
//	{
//		strIDCTrack2 = MakeUnPack(szTempa, nLen);
//		NHDEBUG(DBG_INFO, (L"Success to Get track 2 data from IC\n"));
//		NHDEBUG(DBG_INFO, (L"IDC Track2 : [%s]\n", strIDCTrack2));
//	}
//	else
//	{
//		NHDEBUG(DBG_INFO, (L"Failed to Get track 2 data from IC\n"));
//		return RES_EMV_TERMINATE;
//	}
//
//	nRes = BIZ_EMV_ICCardRead(strIDCTrack2);
//
//	if (nRes != RES_OK)
//		return RES_EMV_TERMINATE;
//#endif	// end of [#2173]	// end of [#2379]

	//------------------------------------------------------------------------
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"Call : [fnEMV_Trans_CardholderVerify]\n"));
	unsigned char szTemp[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	nResult = m_pDevCmn->fnEMV_Trans_CardholderVerify(8, szTemp);

	strTemp.Format(L"CardHolder:%d", nResult);
	NVDump('O', 'C', "53", L"", strTemp );

	NHDEBUG(DBG_CALL, (L"Call : [fnEMV_Trans_CardholderVerify] nResult [%d] \n", nResult));

	if (nResult < EMV_RSLT_FAIL)
	{
		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
			return RES_EMV_FALLBACK;
		else
			return RES_EMV_TERMINATE;
	}

	//------------------------------------------------------------------------
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"Call : [fnEMV_Trans_TerminalRiskMgmt]\n"));
	nResult = m_pDevCmn->fnEMV_Trans_TerminalRiskMgmt(0x00);

	strTemp.Format(L"TerRisk:%d", nResult);
	NVDump('O', 'C', "53", L"", strTemp );

	if (nResult < EMV_RSLT_FAIL)
	{
		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
			return RES_EMV_FALLBACK;
		else
			return RES_EMV_TERMINATE;
	}

	//------------------------------------------------------------------------
	unsigned char TerminalCID;
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"Call : [fcEMV_Trans_TerminalActionAnalysis]\n"));
	TerminalCID = m_pDevCmn->fcEMV_Trans_TerminalActionAnalysis();
	NHDEBUG(DBG_INFO, (L"Terminal CID : [0x%02X]\n", TerminalCID));

	strTemp.Format(L"TerminalAnal:%d", nResult);
	NVDump('O', 'C', "53", L"", strTemp );

	//------------------------------------------------------------------------
	unsigned char OnlineFlag;
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"Call : [fnEMV_Trans_CardActionAnalysis]\n"));
	nResult = m_pDevCmn->fnEMV_Trans_CardActionAnalysis(&OnlineFlag);
	NHDEBUG(DBG_CALL, (L"Call : [fnEMV_Trans_CardActionAnalysis] nResult[%d] OnlineFlag[%x]\n", nResult, OnlineFlag));

	strTemp.Format(L"CardAnal:%d", nResult);
	NVDump('O', 'C', "53", L"", strTemp );

	if (nResult != EMV_RSLT_OK)
	{
		if (nResult == EMV_ERR_NOT_SUPPORT || nResult == EMV_ERR_CARD_SW)
			return RES_EMV_FALLBACK;
		else
			return RES_EMV_TERMINATE;
	}

	if (OnlineFlag != (unsigned char)ONLINE_TRANS)
	{
		m_pDevCmn->fnEMV_Trans_Completion();
		m_pDevCmn->m_bDisplayDeclined = TRUE;
		return RES_EMV_TERMINATE;
	}
	//------------------------------------------------------------------------

	return RES_OK;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_FallBackProc()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_EMV_FallBackProc()
{
	BIZ_RETURN	nRes;
	CString		strReturn;
	CString		m_pScrInName;
	CString		m_pScrInData;
	CString		GetKeyStr;

	NVDump('O', 'C', "05", L"", L"FBackP1" );  // RWC6-656 fallback 01/02/2024

	// [#559] [CA] 2009.8.19 Latch 푸는 위치 변경
	/* Chip Power ON Error or Chip IO(Send/Receive) Fail */
	if (m_pDevCmn->fnMCU_IsEmvEnable() == FALSE || m_pDevCmn->fnMCU_GetDeviceStatus() != NORMAL)
	{
		m_pDevCmn->nKindOfMedia = MEDIA_MS;
		return RES_USER_EXIT;
	}

	// [#225] UK KGS 2008.05.21 MS카드 Read 가능전에 사용자가 카드를 제거했을 경우 취소처리
	if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)		// fall back 시 카드가 없으면 cancel처리
	{
		if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
		{
			/* IC CHIP POWER OFF & Check Device Action */
			m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
		}
		return RES_USER_EXIT;
	}
	// end of [#225]

	// Fallback continue display?
	///////////////////////////////////
	// DISPLAY SCREEN

	// Setting Screen
	{
		m_pDevCmn->fnSCR_DisplayPrevSet(128);
		m_pDevCmn->fnSCR_DisplayString(4, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_128001));
		m_pDevCmn->fnSCR_DisplayScreen(128, K_NO_WAIT, PIN_MENU_MODE);
		g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
	}

	// [#2115] MX KSK 2012.01.28 FallBack 거래 구분을 위해 AID 삭제
	MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");
	MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME,  L"");	// [#2252] US Justin 2014.02.04
	MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE,		L"");
	// end of [#2115]

	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString();			// Key Data Waiting ...

			if (GetKeyStr == S_YES)
			{
				if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
				{
					// [#2308] US KSK 2014.11.14 SP에서 Power On 상태에서 ReadRawData를 내릴 경우 자체적으로 Power Off를 하므로 AP에서는 Power Off Skip
					/* IC CHIP POWER OFF & Check Device Action */
					//m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
					//m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
					m_pDevCmn->nIsEmvChipPowerOn = 0;
					// end of [#2308]
				}

				// KSK 2016.06.21 P_EMV_RemoveCard함수에서 수행하므로 불필요 Call 제거
//				m_pDevCmn->nKindOfMedia = MEDIA_MS;		// MS mode로 강제셋팅
//				m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);

				// nRes : RES_OK, RES_USER_TIMEOUT
				nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127002), TRUE);

				return nRes;
			}
			else if(GetKeyStr == S_NO || GetKeyStr == S_EXIT)
			{
				return RES_USER_EXIT;
			}
		}
		
		// 시간 간격이 있으므로, 강제로 Card를 빼는 경우 Cancel 처리
		if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	// IC 우선, fallback 시 카드가 없으면 cancel처리
		{
			if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
			{
				/* IC CHIP POWER OFF & Check Device Action */
				m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
			}
			return RES_USER_EXIT;
		}

		Delay_Msg(50);
	}

	if (g_TimeCheck.IsElapsedTimes() == TRUE)
	{
		if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
		{
			/* IC CHIP POWER OFF & Check Device Action */
			m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
		}

		return RES_USER_TIMEOUT;
	}

	return RES_EMV_TERMINATE;	// Logic 오류인 경우 거래 종료
}

// [#2375] US Justin USVERSION => Continue Fallback without Asking
#if !(US_VERSION || CA_VERSION)
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_ADA_FallBackProc()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_EMV_ADA_FallBackProc()
{
	BIZ_RETURN	nRes;
	CString		strReturn;
	CString		m_pScrInName;
	CString		m_pScrInData;
	CString		GetKeyStr;

	// [#559] [CA] 2009.8.19 Latch 푸는 위치 변경
	/* Chip Power ON Error or Chip IO(Send/Receive) Fail */
	if (m_pDevCmn->fnMCU_IsEmvEnable() == FALSE || m_pDevCmn->fnMCU_GetDeviceStatus() != NORMAL)
	{
		m_pDevCmn->nKindOfMedia = MEDIA_MS;
		return RES_USER_EXIT;
	}

	// [#225] UK KGS 2008.05.21 MS카드 Read 가능전에 사용자가 카드를 제거했을 경우 취소처리
	if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)		// fall back 시 카드가 없으면 cancel처리
	{
		if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
		{
			/* IC CHIP POWER OFF & Check Device Action */
			m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
		}
		return RES_USER_EXIT;
	}
	// end of [#225]

	// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
	BOOL			bPlayGuidance = TRUE;
	BOOL			bStartToTimeout = FALSE;
	// End of [#2240]

	// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
	g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT); //"타임아웃 설정을 음성 재생 후로 변경"
	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		if((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
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
				// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
#if(US_VERSION)
				m_pAdaCtrl->fnExp_AddWaveFile(128, L"RepeatInfo.wav");		// [#2240] US Justin 2013.12.11 VG for repeat Keys (Cardtronics)
#endif
				// End of [#2240]
				m_pAdaCtrl->fnExp_PlayScreenWave(128); //[#2077] CA PCS 2011.07.06 "128.wav"파일 생성(화면번호와 통일)
			}
			bPlayGuidance = FALSE;
			bStartToTimeout = TRUE;
		}

		// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
		// "타임아웃 시작점을 음성 종료 시로 설정함."
		if ((m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE))
		{
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}

//			g_TimeCheck.SetTargetTimeAfterSec(ADA_SCREEN_TIMEOUT);
//			while(g_TimeCheck.IsElapsedTimes() == FALSE)
//			{
		//[#2077] CA PCS 2011.07.06
		if((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE))
		{
			NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
			return RES_USER_EXIT;
		}
		
		///////////////////////////////////
		// GET KEY STRING
		if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
		{
			GetKeyStr = m_pDevCmn->fstrSCR_GetKeyString();			// Key Data Waiting ...
			GetKeyStr = GetKeyStr.Mid(6);

			if (GetKeyStr == S_YES || GetKeyStr==L"2")
			{
				// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
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
				// End of [#2240]

				if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
				{
					// [#2308] US KSK 2014.11.14
					/* IC CHIP POWER OFF & Check Device Action */
					//m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
					//m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
					m_pDevCmn->nIsEmvChipPowerOn = 0;
					// end of [#2308]
				}

				// KSK 2016.06.21 P_EMV_RemoveCard함수에서 수행하므로 불필요 Call 제거
//				m_pDevCmn->nKindOfMedia = MEDIA_MS;		// MS mode로 강제셋팅
//				m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
				// [#2325] 카드 거래 중 Fallback이므로 이 경우는 RFID Enable 처리 안함

				// nRes : RES_OK, RES_USER_TIMEOUT
				//ADA or NOR 모든 경우에 대해서  P_EMV_RemoveCard() 내부에서 처리함.
				//MS거래를 사용자가 원하기 때문에 메시지를 남겨두었다.
				nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127002), TRUE);;//[#2077]CA PCS 2011.07.09 

				return nRes;
			}
			else if(GetKeyStr == S_NO || GetKeyStr == S_EXIT || GetKeyStr==L"4"||GetKeyStr==S_CANCEL) //[#2077] CA PCS 2011.07.18
			{
				// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
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
				// End of [#2240]
				return RES_USER_EXIT;
			}
			// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
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
			// End of [#2240]
			else
			{
				// [#2240] US Justin 2013.12.12 EMV ADA VG, repeat, Volume control
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
				// End of [#2240]
			}
		}

		// 시간 간격이 있으므로, 강제로 Card를 빼는 경우 Cancel 처리
		if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	// IC 우선, fallback 시 카드가 없으면 cancel처리
		{
			if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
			{
				/* IC CHIP POWER OFF & Check Device Action */
				m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
				m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
			}
			return RES_USER_EXIT;
		}
		Delay_Msg(50);
	}

	if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
	{
		/* IC CHIP POWER OFF & Check Device Action */
		m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
		m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
	}
	//return RES_USER_TIMEOUT;
	return RES_EMV_TERMINATE;	// Logic 오류인 경우 거래 종료
	// End of [#2240]
}
#endif

// [#2208] US Justin 2013.07.08 Continue Fallback for initial Fallback retrial.
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_ContinueFallBackProc()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_EMV_ContinueFallBackProc(int screenNumber)
{
	BIZ_RETURN	nRes;

	NVDump('O', 'C', "05", L"", L"FBackP2" );  // RWC6-656 fallback 01/02/2024


	if (m_pDevCmn->fnMCU_IsEmvEnable() == FALSE || m_pDevCmn->fnMCU_GetDeviceStatus() != NORMAL)
	{
		m_pDevCmn->nKindOfMedia = MEDIA_MS;
		return RES_USER_EXIT;
	}

	// NO CARD => CANCEL
	if(m_pDevCmn->fnMCU_GetMaterialInfo() == 0)		
	{
		if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
		{
			m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
		}
		return RES_USER_EXIT;
	}

	MemSetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID,		L"");
	MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_EMV_APPLICATION_NAME,  L"");	// [#2252] US Justin 2014.02.04
	MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE,		L"");

	if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
		m_pDevCmn->nIsEmvChipPowerOn = 0;

	// KSK 2016.06.21 P_EMV_RemoveCard함수에서 수행하므로 불필요 Call 제거
//	m_pDevCmn->nKindOfMedia = MEDIA_MS;		// MS mode
//	m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
	// end of KSK 2016.06.21
	// [#2325] 카드 거래 중 Fallback이므로 이 경우는 RFID Enable하지 않음

	// nRes : Available : RES_OK or RES_USER_TIMEOUT
	nRes = P_EMV_RemoveCard(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127002), TRUE, FALSE, screenNumber);
	return nRes;
}
// End of [#2208]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: P_EMV_RemoveCard()
RETURN TYPE  : -
PARAMETER    : -
DESCRIPTION  : 
-------------------------------------------------------------------*/
BIZ_RETURN	CTranCmn::P_EMV_RemoveCard(CString strDisplayMsg, int MSDataWaitFlag, int bForceErrDisp, int screenNumber)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_EMV_RemoveCard]\n"));

	BIZ_RETURN	nRes= RES_OK;
	CString		GetKeyStr;
	CString		strTemp;

	NHDEBUG(1, (_T("***TranBizProc_EMV***CTranCmn::P_EMV_RemoveCard() m_pDevCmn->fnMCU_GetDeviceStatus():[%d] \n"),m_pDevCmn->fnMCU_GetDeviceStatus()));

	/* IC CHIP POWER OFF & Check Device Action */
	if(m_pDevCmn->fnMCU_IsEmvChipPowerOn())
	{
		// [#2308] US KSK 2014.11.14
		if (MSDataWaitFlag == TRUE)
		{
			m_pDevCmn->nIsEmvChipPowerOn = 0;
		}
		else
		{
			m_pDevCmn->fnMCU_ICChipPower(IC_POWER_OFF);
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_ON);  // [RWC6-650] Return card start Card Reader Lite 12/19/2023
			m_pDevCmn->fnAPL_CheckDeviceAction(DEV_MCU);
		}
		// end of [#2308]
	}

	if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)	// 매체가 없는 경우 정상 return
	{
		if (bForceErrDisp == TRUE)
		{
			//[#2077] CA PCS 2011.07.06
			if (IsAdaTransaction() != TRUE)
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(screenNumber);
				// [#2175] US Justin 2013.01.24 US EMV
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
				{
					m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
					m_pDevCmn->fnSCR_DisplayImage(2, TRUE);
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
					m_pDevCmn->fnSCR_DisplayImage(2, FALSE);
				}
				// End of [#2175]
			}

			if (strDisplayMsg.IsEmpty())
			{
				// Message가 없을 경우
				if (IsAdaTransaction() == TRUE && m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE)	//[#2077] CA PCS 2011.07.06 카드를 제거하라는 음성파일 재생
					m_pAdaCtrl->fnExp_ResetAndAddPlay(L"127_1.wav", TRUE);						//"127_1.wav" Remove your card. 
				else
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127001));
			}
			else //Message가 있을 경우
			{
				if (IsAdaTransaction() == TRUE && m_pDevCmn->fnSNS_GetEnhancedAudio() == TRUE)
				{
					if ((strDisplayMsg) == (m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127002))) // To proceed your transaction 일 경우.
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"127_2.wav", TRUE);					//"127_2.wav" remove your card for MS transaction.
					else if((strDisplayMsg)==(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003)))
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"127_3.wav", TRUE);					//"127_3.wav" CHIP CARD ERROR
					else //CASE: (strDisplayMsg)==(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127004))
						m_pAdaCtrl->fnExp_ResetAndAddPlay(L"127_4.wav", TRUE);					//"127_4.wav" Transaction is approved
				}
				else 
				{
					strTemp.Format(L"%s\n%s", strDisplayMsg, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127001));
					m_pDevCmn->fnSCR_DisplayString(3, strTemp);
				}
			}

			if (IsAdaTransaction() == FALSE)
			{
				m_pDevCmn->fnSCR_DisplayScreen(screenNumber);
				Delay_Msg(3000);	// 3초 Display후 return
			}
		}
		m_pDevCmn->fnNET_RMSConnectClose();		// [#2305] US Justin 2014.11.13 "Card is removed before displaying screen" --- Move RMSClose Timing to reduce Unlatch Delay. 
		return RES_OK;
	}

	// [#2305] US Justin 2014.11.13 "REMOVE CARD" --- Move RMSClose Timing to reduce Unlatch Delay. 
	if (MSDataWaitFlag == TRUE)
	{
		m_pDevCmn->nKindOfMedia = MEDIA_MS;		// MS mode로 강제셋팅
		m_pDevCmn->fnMCU_CardEnDisable(ENABLE, TRUE);
	}
	// End of [#2305]

	// KSK 2016.06.21 Read Command 완료 전 Card가 제거 되었을 경우 취소 처리
	if (m_pDevCmn->fnMCU_GetMaterialInfo() == 0)
		return RES_USER_EXIT;
	// end of 2016.06.21

	// [#2325] 카드 거래 중 Fallback이므로 이 경우는 RFID Enable 처리 안함
	//////////////////////////////////////////////////////////////////////////
	// Remove Your Card
	//////////////////////////////////////////////////////////////////////////

	// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
	long nScrTimeOut = GetTransactionScreenTimeOut();
	if(nScrTimeOut==0)							// ADA Mode....Jack removed.
		return RES_USER_EXIT;

	// ADA Variable
	BOOL	bStartToTimeout = FALSE;
	BOOL	bInvalidRepeat = FALSE;
	BOOL	bShowScreen = TRUE;
	int		nSensor = 1;
	int		nRepeatDelay;

	NHDEBUG(DBG_INFO, (L"Screen Initial Time Out Start [%d] Seconds\n", nScrTimeOut));
	g_TimeCheck.SetTargetTimeAfterSec(nScrTimeOut);
	// [#2375]

	while(g_TimeCheck.IsElapsedTimes() == FALSE)
	{
		///////////////////////////////////
		// CHECK STATUS
		// [#2375] US Justin 2015.10.27 Combine ADA and Screen Flow
		if( IsAdaTransaction() ) 
		{
			if (m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE )
			{
				NHDEBUG(DBG_INFO, (_T("ADA JACK NOT DETECTED\n")));
				m_pAdaCtrl->fnExp_StopPlay();
				break;
			}

			if( (bInvalidRepeat)&&(m_pAdaCtrl->IsPlaying()==FALSE)  )
			{
				bInvalidRepeat = FALSE;
				bShowScreen = TRUE;
			}
		}
		else
		{
#if defined(_WIN32_WCE)
			sndPlaySound(ATM_PATH L"\\DING.WAV", SND_ASYNC);	// [RWC6-651] Audio Beep to return card Start  12/19/2023
#endif
		}

		///////////////////////////////////
		// DISPLAY SCREEN
		if (bShowScreen == TRUE)
		{
			NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));

			if (IsAdaTransaction() == TRUE)
			{
				m_pDevCmn->fnSCR_DisplayAdaPrevSet(004);
				m_pDevCmn->fnSCR_DisplayAdaScreen(004, 0, PIN_MENU_MODE);

				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
				if (strDisplayMsg.IsEmpty())		// No message
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"127_1.wav");
				else
				{
					if ((strDisplayMsg)==(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127002)))					// To proceed your transaction 일 경우.
						m_pAdaCtrl->fnExp_AddWaveFile(1,L"127_2.wav");
					else if(	( (strDisplayMsg)==(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127003)) )||
								( (strDisplayMsg)==(m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127005)) ) )		// [#2215] NH Justin 2013.08.05 Screen Message change for EMV Denial Transaction : CHIP ERROR => Transaction Error
						m_pAdaCtrl->fnExp_AddWaveFile(1,L"127_3.wav");
					else
						m_pAdaCtrl->fnExp_AddWaveFile(1,L"127_4.wav");
				}	
				#if(US_VERSION)
					m_pAdaCtrl->fnExp_AddWaveFile(1,L"LocationRepeatKey.wav");
				#endif	

				m_pAdaCtrl->fnExp_PlayScreenWave(1);
				nRepeatDelay = 0;
				bStartToTimeout = TRUE; 
			}
			else
			{
				m_pDevCmn->fnSCR_DisplayPrevSet(screenNumber);
				if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX4000W")
				{
					m_pDevCmn->fnSCR_DisplayImage(1, FALSE);
					m_pDevCmn->fnSCR_DisplayImage(2, TRUE);
				}
				else
				{
					m_pDevCmn->fnSCR_DisplayImage(1, TRUE);
					m_pDevCmn->fnSCR_DisplayImage(2, FALSE);
				}

				if (strDisplayMsg.IsEmpty())	// No Message
					m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127001));
				else
				{
					strTemp.Format(L"%s\n%s", strDisplayMsg, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_127001));
					m_pDevCmn->fnSCR_DisplayString(3, strTemp);
				}

#if (AU_VERSION)
				// KSK 2016.06.21 MS Read전 Card가 미리 제거되어 Sensor 정보를 갱신 못할 경우(Magtek) CANCEL KEY를 이용하여 거래 취소되도록 수정
				if (MSDataWaitFlag == TRUE)
					m_pDevCmn->fnSCR_DisplayScreen(screenNumber, K_NO_WAIT, PIN_MENU_MODE);
				else
					m_pDevCmn->fnSCR_DisplayScreen(screenNumber);
#else
				m_pDevCmn->fnSCR_DisplayScreen(screenNumber);
#endif
			}
			bShowScreen = FALSE;
		}

		if( (IsAdaTransaction())&&(m_pAdaCtrl->IsPlaying() == FALSE)&&(bStartToTimeout == TRUE) )
		{
			NHDEBUG(DBG_INFO, (L"ADA TimeOut Start [%d] Seconds\n", KEYIN_TIME_OUT));
			g_TimeCheck.SetTargetTimeAfterSec(KEYIN_TIME_OUT);
			bStartToTimeout = FALSE;
		}

		if (MSDataWaitFlag == TRUE)			// 2009.10.20 MS Data Flag On시 Event가 올때까지 대기한다.
		{
			if (m_pDevCmn->fnAPL_GetDeviceEvent(DEV_MCU) == TRUE)	// KSK 2010.01.04
			{
				if (m_pDevCmn->fnSNS_GetEnhancedAudio() )
					m_pAdaCtrl->fnExp_StopPlay();
				nSensor = 0;
				break;
			}
		}
		else
		{
			nSensor = m_pDevCmn->fnMCU_GetMaterialInfo();
		}	

		if( nSensor )
		{
			if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
			{
				///////////////////////////////////
				// GET KEY STRING
				if (m_pDevCmn->fnSCR_ScanDeviceAction(DEV_SCR, 1, EVENT_IN) == DEV_SCR)
				{
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
						#if (US_VERSION)
							m_pAdaCtrl->fnExp_ResetAndAddPlay(L"WrongKey.wav", FALSE);
							bInvalidRepeat = TRUE;
							nRepeatDelay = 0;
							g_TimeCheck.SetTargetTimeAfterSec(ADA_VOICE_MAX_TIMEOUT);
						#endif
					}
				}
				else if( m_pAdaCtrl->IsPlaying() == FALSE ) 
				{
					nRepeatDelay++;
					if( (nRepeatDelay*50)>1000 )		// 1 second later
						bShowScreen	= TRUE;
				}
			}
		}
		else
		{
			if( m_pDevCmn->fnSNS_GetEnhancedAudio() )
				m_pAdaCtrl->fnExp_StopAndResetWaveFile();
			break;
		}
		Delay_Msg(500);
	}

	if (g_TimeCheck.IsElapsedTimes() == TRUE)
		nRes = RES_USER_TIMEOUT;

	// [#2205] US KSK 2013.06.28
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600")
		SetHaloLedControl(SKIP_HALOLED_COLOR, SKIP_HALOLED_MODE, OFF_MCULED);
	else
	{
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_MCU, FLICKER_OFF); // Normal FLICKER_OFF Card reader  12/19/2023
        if( !IsAdaTransaction() ) 
		{
			m_pAdaCtrl->fnExp_WaitUntilStop();                    // [RWC6-651] Audio Beep to return card Stop 12/19/2023
		}
	}
	// end of [#2205]

	return nRes;
}

// [#2391] US Justin 2016.01.26  Support Multi-Transactions per latch
BIZ_RETURN	CTranCmn::P_EMV_FinalSelection4MultiTransactionPerLatch()
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::P_EMV_FinalSelection4MultiTransactionPerLatch]\n"));

	if(!m_pDevCmn->fnMCU_IsEmvTransaction())
		return RES_NG;

	// DISPLAY SCREEN
	NHDEBUG(DBG_INFO, (_T("SHOW SCREEN\n")));
	m_pDevCmn->fnSCR_DisplayPrevSet(125);

	#if (MX_VERSION)
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125002));
	#else
		m_pDevCmn->fnSCR_DisplayString(3, m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_125001));
	#endif

	m_pDevCmn->fnSCR_DisplayScreen(125);

	////////////////////////////////////////////////////////////////////////////////////////////
	// EMV Process from "Final Selection for Dual Transaction per latch 
	NHDEBUG(DBG_CALL, (L" EMV FINAL SELECTION for Multi Transactin\n"));

	// EMV Final Selection 
	// US => EMV_KERNEL_V5 ONLY
	int nResult = m_pDevCmn->fnEMV_Sel_FinalAppSelection(m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidLen, m_sIccAppInform_V5.IccAppDesc[m_nSelectAID].AidName, &m_chSW);
	NHDEBUG(DBG_CALL, (L"  fnEMV_Sel_FinalAppSelection = [%d] \n", nResult));

	if (nResult != EMV_RSLT_OK)
	{
		NHDEBUG(DBG_CALL, (L"  ERROR => fnEMV_Sel_FinalAppSelection FAILED => Return RES_NG\n"));
		return RES_NG;
	}

	CString strTemp;
	// Store Default EMV Termal Values
	NHDEBUG(DBG_CALL, (L"  BIZ_EMV_StoreDefaultTerminalValue() \n"));
	BIZ_EMV_StoreDefaultTerminalValue();

	// EMV Initialize Transaction Variables()
	nResult = m_pDevCmn->fnEMV_Trans_InitApplication();	
	NHDEBUG(DBG_CALL, (L"  m_pDevCmn->fnEMV_Trans_InitApplication = [%d] \n", nResult));
	strTemp.Format(L"InitAppProc:%d", nResult);
	NVDump('O', 'C', "53", L"", strTemp );

	if (nResult != EMV_RSLT_OK)
	{
		NHDEBUG(DBG_CALL, (L"  ERROR => fnEMV_Trans_InitApplication FAILED => Return RES_NG\n"));
		return RES_NG;
	}

	// Enable "fnEMV_Trans_ReadAppData"
	//#if (US_VERSION || CA_VERSION || MX_VERSION)	[#2472] US Justin 2017.02.08 Remove Country restriciton.. AU use the same flow as that of US
		nResult = m_pDevCmn->fnEMV_Trans_ReadAppData();
		NHDEBUG(DBG_CALL, (L"  m_pDevCmn->fnEMV_Trans_ReadAppData() = [%d] \n", nResult));
		strTemp.Format(L"ReadAppData:%d", nResult);
		NVDump('O', 'C', "53", L"", strTemp);

		if (nResult != EMV_RSLT_OK)
		{
			NHDEBUG(DBG_CALL, (L"  ERROR => fnEMV_Trans_ReadAppData FAILED => Return RES_NG\n"));
			return RES_NG;
		}
	//#endif

	NHDEBUG(DBG_CALL, (L"  P_EMV_FinalSelection4MultiTransactionPerLatch SUCCESS => Return RES_OK\n"));
	return RES_OK;
}
// End of [#2391]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_InitValue()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::BIZ_EMV_InitValue()
{
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"[CTranCmn::fnAPP_Emv_InitValue]\n"));

	m_nCandidateList = 0;
	memset(&m_sIccAppInform, 0x0, sizeof(m_sIccAppInform));

	m_nSelectAID = -1;
	m_nSelectedTermDataIndex = -1;
	m_chSW = 0x0;
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_LoadAIDList()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::BIZ_EMV_LoadAIDList()
{
	CNHReadiniFile	Readini;
	PINIVALUE		pwIniValue;
	CString			strDataFileName;
	int				nLoadCount;
	char			szTemp[1024];

	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"[CTranCmn::fnAPP_LoadAIDList]\n"));

	// [#2435] NH KSK 2016.07.14 Term Data는 V5와 모든게 호환이 되므로 내부적으로 V6인 경우 V5 FileName을 그대로 사용하도록 유지 (NHA 검토 필요)
	// [#2226] NH Justin 2013.10.22 Load term data depends on Kernel Version
	//strDataFileName.Format(L"%S", TERMDATA_INI_FILE);
	strDataFileName.Format(L"%S", GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()) );
	// End of [#2226]

	memset(&m_TerminalData, 0x0, sizeof(m_TerminalData));
	memset(&m_sAIDList, 0x0, sizeof(m_sAIDList));
	memset(&m_sAIDList_V5, 0x0, sizeof(m_sAIDList_V5));		// [#2188] NH KSK 2013.05.06

	nLoadCount = -1;

	// open Screen Text File
	if (Readini.Open(strDataFileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII) == TRUE)
	{
		// AID를 MAX 60개까지만 Load하도록 수정
		while(nLoadCount < ICEMV_APP_CNT)
		{
			if (NULL != (pwIniValue = Readini.ReadiniValue()))
			{
				// [TERMINAL] section
				if (pwIniValue->Section.Left(8) == L"TERMINAL")
				{
					if (pwIniValue->Key == pwIniValue->Section)
					{
						nLoadCount++;
					}
					else if (pwIniValue->Key == L"APPLICATIONNAME")
					{
						WideToMulti((char*)m_TerminalData[nLoadCount].AidName, pwIniValue->Values[0], sizeof(m_TerminalData[nLoadCount].AidName));
					}
					else if (pwIniValue->Key == L"AID")
					{
						// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
						if (nLoadCount < 0)
							break;
						// end of [#2022]

						// [#2188] NH KSK 2013.05.06
						if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
						{
							WideToMulti((char*)&m_sAIDList.aidI[nLoadCount].aid[0], pwIniValue->Values[0], 15);
							m_sAIDList.aidI[nLoadCount].aidLen = pwIniValue->Values[0].GetLength();


							memset(szTemp, NULL, sizeof(szTemp));
							WideToMulti(szTemp, pwIniValue->Values[0], sizeof(szTemp));

							m_sAIDList.aidI[nLoadCount].aidLen = MakePack(szTemp, m_sAIDList.aidI[nLoadCount].aid, pwIniValue->Values[0].GetLength());

							m_TerminalData[nLoadCount].nAidLen = m_sAIDList.aidI[nLoadCount].aidLen;
							memcpy(m_TerminalData[nLoadCount].szAid, m_sAIDList.aidI[nLoadCount].aid, m_TerminalData[nLoadCount].nAidLen);
						}
						else
						{
							WideToMulti((char*)&m_sAIDList_V5.aidI[nLoadCount].aid[0], pwIniValue->Values[0], 15);
							m_sAIDList_V5.aidI[nLoadCount].aidLen = pwIniValue->Values[0].GetLength();


							memset(szTemp, NULL, sizeof(szTemp));
							WideToMulti(szTemp, pwIniValue->Values[0], sizeof(szTemp));

							m_sAIDList_V5.aidI[nLoadCount].aidLen = MakePack(szTemp, m_sAIDList_V5.aidI[nLoadCount].aid, pwIniValue->Values[0].GetLength());

							m_TerminalData[nLoadCount].nAidLen = m_sAIDList_V5.aidI[nLoadCount].aidLen;
							memcpy(m_TerminalData[nLoadCount].szAid, m_sAIDList_V5.aidI[nLoadCount].aid, m_TerminalData[nLoadCount].nAidLen);
						}
						// end of [#2188]

						NHDEBUG(DBG_INFO, (L"AID[%d] : Load[%s]\n", nLoadCount, pwIniValue->Values[0]));
					}
					else if (pwIniValue->Key == L"TERMINALDATA")
					{
						// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
						if (nLoadCount < 0)
							break;
						// end of [#2022]

						m_TerminalData[nLoadCount].nRecordLen = pwIniValue->Values[0].GetLength();
						WideToMulti((char*)m_TerminalData[nLoadCount].szRecord, pwIniValue->Values[0], m_TerminalData[nLoadCount].nRecordLen);
						NHDEBUG(DBG_INFO, (L"TERMINAL DATA[%d] : ", nLoadCount));
					}
				}
			}
			else
				break;
		}

		Readini.Close();
	}

	nLoadCount++;

	// [#2188] NH KSK 2013.05.06
	if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
	{
		m_sAIDList.aidCNT = nLoadCount;
		NHDEBUG(DBG_INFO, (L"AID LOAD COUNT [%d]\n", m_sAIDList.aidCNT));
	}
	else
	{
		m_sAIDList_V5.aidCNT = nLoadCount;
		NHDEBUG(DBG_INFO, (L"AID LOAD COUNT [%d]\n", m_sAIDList_V5.aidCNT));
	}
	// end of [#2188]
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_GetTerminalDataIndex()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int CTranCmn::BIZ_EMV_GetTerminalDataIndex(unsigned char *pAID, int nAID)
{
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_EMV_GetTerminalDataIndex]\n"));

	int	nIndex = -1;

	NHDEBUG(DBG_INFO, (L"SELECT AID : "));

	// [#2188] NH KSK 2013.05.06
	if (m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)
	{
		for (int i = 0; i < m_sAIDList.aidCNT; i++)
		{
			NHDEBUG(DBG_INFO, (L"COMPAR AID : "));

			if (memcmp(m_TerminalData[i].szAid, pAID, m_TerminalData[i].nAidLen) == 0)
			{
				nIndex = i;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_sAIDList_V5.aidCNT; i++)
		{
			NHDEBUG(DBG_INFO, (L"COMPAR AID : "));

			if (memcmp(m_TerminalData[i].szAid, pAID, m_TerminalData[i].nAidLen) == 0)
			{
				nIndex = i;
				break;
			}
		}
	}
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"Match Index (%d)\n", nIndex));

	return nIndex;
}

// [RWC6-676] Start SKKim 2024.4.8 TangoPay에서도 사용해야 하므로 Cmnlib로 이동
//static float u[98], c, cd, cm;
//static unsigned short i97, j97;
//static unsigned short test = 0;
//
//void rmarin(unsigned short ij, unsigned short kl)
//{
//	unsigned short i, j, k, l, ii, jj, m;
//	float s, t;
//
//	if (ij<0 || ij>31328 || kl<0 || kl>30081)
//	{
//		return;
//	}
//
//	i = (ij/177)%177 + 2;
//	j = ij%177 + 2;
//	k = (kl/169)%178 + 1;
//	l = kl%169;
//
//	for (ii=1; ii<=97; ii++)
//	{
//		s = 0.0;
//		t = 0.5;
//		for (jj=1; jj<=24; jj++)
//		{
//			m = (((i*j)%179)*k) % 179;
//			i = j;
//			j = k;
//			k = m;
//			l = (53*l + 1) % 169;
//			if ((l*m)%64 >= 32) s += t;
//			t *= 0.5;
//		}
//		u[ii] = s;
//	}
//
//	c = 362436.0 / 16777216.0;
//	cd = 7654321.0 / 16777216.0;
//	cm = 16777213.0 / 16777216.0;
//
//	i97 = 97;
//	j97 = 33;
//
//	test = 1;
//}
//
//void ranmar(float rvec[], unsigned short len)
//{
//	unsigned short ivec;
//	float uni;
//
//	if (test == (unsigned short)0) {
//		return;
//	}
//	for (ivec=1; ivec<=len; ivec++)
//	{
//		uni = u[i97] - u[j97];
//		if (uni < 0.0) uni += 1.0;
//		u[i97] = uni;
//		i97--;
//		if (i97==0) i97 = 97;
//		j97--;
//		if (j97==0) j97 = 97;
//		c -= cd;
//		if (c<0.0) c += cm;
//		uni -= c;
//		if (uni<0.0) uni += 1.0;
//		rvec[ivec] = uni;
//	}
//}
//
//void generate_srand(unsigned short seed)
//{
//	unsigned short iFirst, iSecond, i;
//	unsigned long ulTmp;
//	unsigned short ucDateTime[14+1];
//
//	typedef union {
//		unsigned int	x;
//		unsigned char	h[4];
//	}dWORD;
//
//	dWORD	TimeTick;	
//
//	TimeTick.x = ::GetTickCount();
//	ucDateTime[2] = (TimeTick.h[0]);
//	ucDateTime[3] = (BYTE)(TimeTick.h[1]);
//	ucDateTime[4] = (BYTE)(TimeTick.h[2]);
//	ucDateTime[5] = (BYTE)(TimeTick.h[3]);
//	ucDateTime[6] = (BYTE)(~TimeTick.h[2]);
//	ucDateTime[7] = (BYTE)(~TimeTick.h[3]);
//
//	seed ^= TimeTick.h[1];
//	while (seed>=200)seed>>=1;
//	Sleep(seed*10);
//
//	TimeTick.x = ::GetTickCount();
//	ucDateTime[8] = (TimeTick.h[3]);
//	ucDateTime[9] = (BYTE)(TimeTick.h[2]);
//	ucDateTime[10] = (BYTE)(TimeTick.h[2]);
//	ucDateTime[11] = (BYTE)(TimeTick.h[1]);
//	ucDateTime[12] = (BYTE)(~TimeTick.h[2]);
//	ucDateTime[13] = (BYTE)(~TimeTick.h[1]);
//
//
//	for(i = 0, ulTmp = 0L; i < 6; i++)
//	{
//		ulTmp *= (unsigned long)10;
//		ulTmp += (unsigned long)(ucDateTime[2 + i]);
//	}
//	iFirst = (unsigned short)(ulTmp % 31320) + seed;
//
//	for(i = 0, ulTmp = 0L; i < 6; i++)
//	{
//		ulTmp *= (unsigned long)10;
//		ulTmp += (unsigned long)(ucDateTime[8 + i]);
//	}
//	iSecond = (unsigned short)(ulTmp % 30080);
//
//	rmarin(iFirst, iSecond);
//}
//
//BOOL	bExec_SRand = FALSE;
//unsigned short generate_rand(void)
//{
//	float faTemp[2] = {0, };	// CodeSonar 지적
//
//	if (bExec_SRand == FALSE)
//	{
//		bExec_SRand = TRUE;
//
//		unsigned short seed;
//		seed = (unsigned short)GetTickCount();
//		generate_srand(seed);
//	}
//
//	ranmar(&faTemp[0], 1);	
//
//	return (unsigned short)(4096.0 * 4096.0 * faTemp[1]);
//}
// [RWC6-676] End SKKim 2024.04.08

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_StoreDefaultTerminalValue()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::BIZ_EMV_StoreDefaultTerminalValue()
{
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"[CTranCmn::BIZ_EMV_StoreDefaultTerminalValue]\n"));

	int	nTid, nLen;
	unsigned char szValue[1024];
	char	szTemp[1024];

	//////////////////////////////////////////////////////////////////////////
	// Store terminal Value
	memset(szValue, 0x0, sizeof(szValue));
	nLen = MakePack(m_TerminalData[m_nSelectedTermDataIndex].szRecord, szValue, m_TerminalData[m_nSelectedTermDataIndex].nRecordLen);

	NHDEBUG(DBG_INFO, (L"Terminal Data Index(%d)\n", m_nSelectedTermDataIndex));
	NHDEBUG(DBG_INFO, (L"Terminal Data(Pack) -- Len(%d) : ", nLen));
	m_pDevCmn->fnEMV_Tlv_StoreValFromRecord(szValue, 0, nLen, 162);

	//////////////////////////////////////////////////////////////////////////
	// Store Transaction Value

	// 9A, 9F21 Tag
	{
		SYSTEMTIME localTime;
		::GetLocalTime(&localTime);

		nTid = EMVTid_TrDate;
		nLen = 0x03;
		memset(szTemp, 0x0, sizeof(szTemp));
		memset(szValue, 0x0, sizeof(szValue));

		sprintf(szTemp, "%02d%02d%02d", (localTime.wYear - 2000), localTime.wMonth, localTime.wDay);
		MakePack(szTemp, szValue, 6);
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);

		nTid = EMVTid_TrTime;
		nLen = 0x03;
		memset(szTemp, 0x0, sizeof(szTemp));
		memset(szValue, 0x0, sizeof(szValue));

		sprintf(szTemp, "%02d%02d%02d", localTime.wHour, localTime.wMinute, localTime.wSecond);
		MakePack(szTemp, szValue, 6);
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}

	// 9F1C
	{
		nTid = EMVTid_TerminalId;
		nLen = 0x08;
		memset(szTemp, 0x0, sizeof(szTemp));
		memset(szValue, 0x0, sizeof(szValue));
#if (APP_LIBERTYX)
		if (m_HostConfig == HC_LIBERTYX)
		{
			sprintf((char*)szValue, "%-8.8S", m_LXConfig.LocationID);
		}
		else
#endif
		{
			sprintf((char*)szValue, "%-8.8S", MemGetStr(_MEM_FLD_APP_ATMINFO,_MEM_VAR_APP_TERMINALID));
		}
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}

	// 9F41, 9F37
	{
		nTid = EMVTid_TrSeqCnt;
		nLen = 0x04;
		memset(szTemp, 0x0, sizeof(szTemp));
		memset(szValue, 0x0, sizeof(szValue));

		CString strSequenceNum;
		int		nSequenceNum = (Asc2Int(m_pDevCmn->fnAPL_GetSerialNo(m_HostConfig)) % 9999) + 1;
		strSequenceNum.Format(L"%08d", nSequenceNum);
		WideToMulti((LPSTR)szTemp, strSequenceNum, 8);
		MakePack((void*)szTemp, (void*)szValue, 8);
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);

		nTid = EMVTid_UnpredictNo;

		// [#2188]
		unsigned short unpredictNo;
		unpredictNo = generate_rand();
		szValue[0] = (unsigned char)((unpredictNo & (unsigned short)0xff00) >> 8);
		szValue[1] = (unsigned char)(unpredictNo & (unsigned short)0x00ff);
		unpredictNo = generate_rand();
		szValue[2] = (unsigned char)((unpredictNo & (unsigned short)0xff00) >> 8);
		szValue[3] = (unsigned char)(unpredictNo & (unsigned short)0x00ff);
		szValue[4] = 0;
//		szValue[0] = ((rand()%256));
//		szValue[1] = ((rand()%256));
//		szValue[2] = ((rand()%256));
//		szValue[3] = ((rand()%256));
//		szValue[4] = 0;
		// end of [#2188]
		nLen = 0x04;

		NHDEBUG(DBG_INFO, (L"UNPREDICTABLE NO[0x%02X 0x%02X 0x%02X 0x%02X]\n", szValue[0], szValue[1], szValue[2], szValue[3]));
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}

	// 9F39
	{
		nTid = EMVTid_POSEntryMode;
		nLen = 0x01;

		memset(szValue, 0x0, sizeof(szValue));

		szValue[0] = 0x05;

		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}

	// 9F1E
	{
		nTid = EMVTid_IFDSerialNo;
		nLen = 0x08;

		memset(szTemp, 0x0, sizeof(szTemp));
		memset(szValue, 0x0, sizeof(szValue));

		sprintf((char*)szValue, "%-8.8S", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ATMSERIALNO));

		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}

	// [#2438] US Justin 2016.07.26 Change 9F53 Tag Value for Kernel V6
	// 9F53  Transaction Category
	{
		if (m_pDevCmn->fnEMV_GetKernelVersion() >= EMV_KERNEL_V6)
		{
			nTid = EMVTid_TCC;
			nLen = 0x01;

			memset(szValue, 0x0, sizeof(szValue));
			szValue[0] = 0x5A;

			NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
			m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
		}
	}
	// End of [#2438]

	// [#2180] NH Justin 2013.03.08 Store Currency ID 
	// 1. 9F1A - Terminal Country Code (ISO 3166)
	nTid = EMVTid_TCountryCod;
	nLen = 0x02;
	
	memset(szValue, 0x0, sizeof(szValue));

	if (m_pDevCmn->m_strCurrencyID == L"USD")		// US $
	{		
		// 1. 9F1A - Terminal Country Code (ISO 3166)
		// [#2336] US Justin 2015.03.12 MX Country Code Fix
		//szValue[0] = 0x08;
		//szValue[1] = 0x40;
		#if (MX_VERSION)	
			szValue[0] = 0x04;
			szValue[1] = 0x84;
		#else
			// [#2549] NH Justin 2018.05.07 Add Perto Rico Option
			//szValue[0] = 0x08;
			//szValue[1] = 0x40;
			int nUSTerritory = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_US_TERRITORY_OPTION);
			switch(nUSTerritory)
			{
			case US_TERRITORY_PUERTORICO:		// 630
				szValue[0] = 0x06;
				szValue[1] = 0x30;
				break;

			case US_TERRITORY_GUAM:				// 316
				szValue[0] = 0x03;
				szValue[1] = 0x16;
				break;

			case US_TERRITORY_MARIANA:			// 580
				szValue[0] = 0x05;
				szValue[1] = 0x80;
				break;

			case US_TERRITORY_VIRGINISLAND:		// 850
				szValue[0] = 0x08;
				szValue[1] = 0x50;
				break;

			case US_TERRITORY_SAMOA:			// 016
				szValue[0] = 0x00;
				szValue[1] = 0x16;
				break;

			default:		// US_TERRITORY_US	// 840
				szValue[0] = 0x08;
				szValue[1] = 0x40;
				break;
			}
			// End of [#2549]
		#endif
		// End of [#2336]

		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);

		// 2. 5F2A - Transaction Currency Code
		szValue[0] = 0x08;			// [#2336] US Justin 2015.03.12 MX Country Code Fix
		szValue[1] = 0x40;			// [#2336] US Justin 2015.03.12 MX Country Code Fix
		nTid = EMVTid_TrCurCod;
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}
	else if (m_pDevCmn->m_strCurrencyID == L"CAD")	// Canadian $	
	{
		// 1. 9F1A - Terminal Country Code
		szValue[0] = 0x01;
		szValue[1] = 0x24;
		
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
		
		// 2. 5F2A - Transaction Currency Code
		nTid = EMVTid_TrCurCod;
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}
	else if (m_pDevCmn->m_strCurrencyID == L"MXN")	// Mexican Peso	
	{
		// 1. 9F1A - Terminal Country Code
		szValue[0] = 0x04;
		szValue[1] = 0x84;
		
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
		
		// 2. 5F2A - Transaction Currency Code
		nTid = EMVTid_TrCurCod;
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}
	else if (m_pDevCmn->m_strCurrencyID == L"AUD")	// Austraillia $
	{
		// 1. 9F1A - Terminal Country Code
		szValue[0] = 0x00;
		szValue[1] = 0x36;
		
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
		
		// 2. 5F2A - Transaction Currency Code
		nTid = EMVTid_TrCurCod;
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}
	else if (m_pDevCmn->m_strCurrencyID == L"NZD")	// NewZealand $
	{
		// 1. 9F1A - Terminal Country Code
		szValue[0] = 0x05;
		szValue[1] = 0x54;
		
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
		
		// 2. 5F2A - Transaction Currency Code
		nTid = EMVTid_TrCurCod;
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> [0x05]", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}
	else
	{
		// OTHER => Use default value (defined at TermData.ini)
	}
	NHDEBUG(DBG_INFO, (L"\n"));
	// end of [#2180]
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_StoreTransactionValue()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
void CTranCmn::BIZ_EMV_StoreTransactionValue()
{
	NHDEBUG(DBG_CALL, (L"\n\n////////////////////////////////////////////////////////////\n"));
	NHDEBUG(DBG_CALL, (L"[CTranCmn::fnAPP_Emv_StoreTransactionValue]\n"));

	int	nTid, nLen;
	unsigned char szValue[1024];
	char	szTemp[1024];

	//////////////////////////////////////////////////////////////////////////
	// Store Transaction Value

	// 9C Tag
	{
		nTid = EMVTid_TrType;
		nLen = 0x01;
		memset(szValue, 0x0, sizeof(szValue));

		// [#2226] US Justin 2013.10.22 EMV Kernel V5
 		if (TranCode == TC_WITHDRAWAL)
 			szValue[0] = (unsigned char)TRTYPE_CASH;
 		else if (TranCode == TC_INQUIRY)
			szValue[0] = (unsigned char)TRTYPE_INQUIRY;
 		else if (TranCode == TC_TRANSFER)
			szValue[0] = (unsigned char)TRTYPE_TRANSFER;
		else									// Default, Cash Withdrawal
 			szValue[0] = (unsigned char)TRTYPE_CASH;
		// End of [#2226]

		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}

	// 9F02
	{
		nTid = EMVTid_AmountAuthN;
		nLen = 0x06;
		memset(szTemp, 0x0, sizeof(szTemp));
		memset(szValue, 0x0, sizeof(szValue));

		int	nAmount = Asc2Int(m_sUserSelection.strMoney);

		sprintf((char*)szTemp, "%012d", nAmount);
		MakePack(szTemp, szValue, 12);
		NHDEBUG(DBG_INFO, (L"STORE : [%d] -> ", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}

	// 5F57 Tag
	{
		nTid = EMVTid_AccountType;
		nLen = 0x01;
		memset(szValue, 0x0, sizeof(szValue));

		NHDEBUG(DBG_INFO, (L"TranCode(%d), SourceAccount(%s)\n", TranCode, m_SourceAccount));

		if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER)
		{
			if (m_SourceAccount == S_CHECKING)
				szValue[0] = 0x20;
			else if (m_SourceAccount == S_SAVINGS)
				szValue[0] = 0x10;
			else if (m_SourceAccount == S_CREDITCARD)
				szValue[0] = 0x30;
		}

		NHDEBUG(DBG_INFO, (L"STORE : [%d]\n", nTid));
		m_pDevCmn->fnEMV_Tlv_StoreVal(nTid, nLen, szValue);
	}
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_ICCardRead()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : IC CARD 정보를 추출한다.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_EMV_ICCardRead(CString strCardData)
{
	NHDEBUG(DBG_CALL, (_T("[CTranCmn::BIZ_EMV_ICCardRead] CardData(%d)\n"), strCardData.GetLength()));

	NHDEBUG(DBG_INFO, (L"BEFORE CARD READ\n"));
	NHDEBUG(DBG_INFO, (L"CARD DATA : [%s]\n", strCardData));
	NHDEBUG(DBG_INFO, (L"BANK CODE : [%s]\n", m_sCardData.strBankID));
	NHDEBUG(DBG_INFO, (L"ACCOUNT   : [%s]\n", m_sCardData.strAccountNo));
	NHDEBUG(DBG_INFO, (L"SHOW NUMB : [%s]\n", m_sCardData.strShowNumber));
	NHDEBUG(DBG_INFO, (L"ISO2 DATA : [%s]\n", m_sCardData.strISO2Data));

	CString strJNLCardData;		// [###] AU KSK 2015.12.17

	if (strCardData.GetLength() > 0)
	{
		int	nSearchIndex;
		CString strPackCardData;

		// IC Read시에는 MS Read와 다르게 Unpack Data가 아닌 Original Data가 오므로 Pack할 필요가 없음

		// remove padding data 'F'	for EMV PSC 2008.10.14; standard1 EMV 전문에 'F'가 padding되면 37 bytes를 넘어가기 때문에 
		// padding data 'F' 를 전문에서 삭제해달라는 요청으로(미국지사 김지훈 과장) 수정함.
		nSearchIndex = strCardData.FindOneOf(L"F");
		if(nSearchIndex != -1)
			strCardData.Replace(L"F", L"");

		/* Replace 'D' to '=' in the Track 2 data from the IC */
		/* Upper & Lower Case Check!! "D" or "d"*/
		nSearchIndex = strCardData.FindOneOf(L"D");

		if(nSearchIndex != -1)
			strCardData.Replace(L"D",L"=");

		if (nSearchIndex >= 13)
		{
			// Save ISO2
			m_sCardData.strISO2Data = strCardData;
			// Card PAN for EPP
			m_sCardData.strAccountNo = strCardData.Mid(nSearchIndex - 13, 12);
			// Bank ID
			m_sCardData.strBankID  = strCardData.Mid(nSearchIndex - 4, 4);

			// [###] AU KSK 2015.12.17 Journal용 Card Data 보관
			strJNLCardData = strCardData.Mid(0, nSearchIndex);
		}
		else if (nSearchIndex != -1)
		{
			// Save ISO2
			m_sCardData.strISO2Data = strCardData;
			// Card PAN for EPP
			m_sCardData.strAccountNo = L"0";
			m_sCardData.strAccountNo += strCardData.Mid(1, 12);
			// Bank ID
			m_sCardData.strBankID  = strCardData.Mid(13, 4);
		}

		// strShowNumber for Receipt
		if (m_sCardData.strAccountNo.GetLength() > 0)
			m_sCardData.strShowNumber.Format(L"************%s", m_sCardData.strBankID);		// KSK 2010.08.25 19자리일 경우 Bug Fix
	}

	// Check ISO 2 Track
	if (m_sCardData.strISO2Data.GetLength() <= 0)
	{
		NHDEBUG(DBG_INFO, (L"DON'T READ ISO2 TRACK Data\n"));
		return RES_NG;
	}

	// Check the mandatory information.
	if (m_sCardData.strAccountNo.GetLength() <= 0)
	{
		NHDEBUG(DBG_INFO, (L"DON'T READ ACCOUNT INFO.\n"));
		return RES_NG;
	}

	// [###] AU KSK 2015.12.17 EMV IC 거래시 MOD10 미체크로직 누락분 적용
	//////////////////////////////////////////////////////////////////////////
	// 4. Check Validation

	// Check MOD10
	if(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_MOD10_ENABLE) == ENABLE)
	{
		int index;
		char tmpData[128];		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
		CString strIDCTrack2;

		strIDCTrack2 = m_sCardData.strISO2Data;
		index = strIDCTrack2.Find(L"=");

		memset(tmpData, 0x00, sizeof(tmpData));
		WideToMulti(tmpData, strIDCTrack2.Left(index), __min(index*2, 128));	// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책

		if(IsValidMod10Data(tmpData, index) == 0)
		{
			NHDEBUG(DBG_INFO, (L"MOD 10 ERROR\n"));
			return RES_CARD_MOD_10;
		}
	}
	// end of [###]

#if (AU_VERSION)	// [###] AU KSK 2015.12.17 AU Multiple Bin Check 누락분 적용
	WORD nBinCount = m_pDevCmn->m_BINMgr.MakeTransactionBinList(m_sCardData.strISO2Data);
	//DECLINE 카드 일 경우 거래 중단함  
	//ALLOW ONLY LISTED BIN ENABLE일 경우 BIN 이  없거나 
	DWORD dwValue = 0;

	if (  m_pDevCmn->m_BINMgr.GetTransactionBinProperties(ACTION_DECLINE_CARD, &dwValue)== TRUE
		|| ( m_pDevCmn->m_BINMgr.GetAllowOnlyBinEnable() == 1  && nBinCount == 0))
	{
		int nReason = 0; //Decline Card로 등록된 경우 
		if ( m_pDevCmn->m_BINMgr.GetAllowOnlyBinEnable() == 1 && nBinCount == 0)
		{
			nReason = 1; //BIN이 등록이 안 된경우 
		}

		CString strReason;
		strReason.Format(L"RESTRICT BIN(%6.6s), REASON=%s", m_sCardData.strISO2Data.Left(6),
			(nReason == 0 ? L"DECLINE CARD": L"ALLOWED ONLY LISTED BIN"));  //[#568] SOOK 2009.09.30 REASON 설명 추가 
		m_pDevCmn->m_JNLMgr.Save(CANCEL_AT_READ_CARD, strReason);

		return RES_CARD_BIN_NG;
	}
#endif				//end of [###]

	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKCODE,	m_sCardData.strBankID);
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSBANKACCOUNT,	m_sCardData.strAccountNo);

#if (AU_VERSION)	// [###] AU KSK 2015.12.17 호주 JNL CARD DATA 저장 사용 누락분 추가 적용
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	strJNLCardData);
#else
	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSCARDDATA,	m_sCardData.strShowNumber);
#endif				// end of [###]

	MemSetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSTRACK2,		m_sCardData.strISO2Data);

	NHDEBUG(DBG_INFO, (L"AFTER CARD READ\n"));
	NHDEBUG(DBG_INFO, (L"BANK CODE : [%s]\n", m_sCardData.strBankID));
	NHDEBUG(DBG_INFO, (L"ACCOUNT   : [%s]\n", m_sCardData.strAccountNo));
	NHDEBUG(DBG_INFO, (L"SHOW NUMB : [%s]\n", m_sCardData.strShowNumber));
	NHDEBUG(DBG_INFO, (L"ISO2 DATA : [%s]\n", m_sCardData.strISO2Data));

#if (AU_VERSION)	// [#2379] AU KSK 2015.11.18
	if (m_pDevCmn->IsBinExist(m_sCardData.strISO2Data, m_pDevCmn->m_arrAUDomesticBinList) == TRUE)
	{
		// DomesticBin Detected
		m_pDevCmn->m_bDomesticFallbackTrans = TRUE;
	}
#endif				// end of [#2379]

	return RES_OK;
}

// [#2081] NH KSK 2011.06.27
/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_UpdateAIDListProc()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Host에서 AID List Update시 수행한다.
-------------------------------------------------------------------*/
BIZ_RETURN CTranCmn::BIZ_EMV_UpdateAIDListProc()
{
	// Initialize Variable
	///////////////////////////////////////////////////////
	nTerminalRemainAIDCnt = 0;
	nHostRemainBlock = 0;
	nCalcRecvTotalAIDCnt = 0;
	///////////////////////////////////////////////////////

	// ATM AID LIST COUNT를 얻어온다.
	//CINIFile iniEMV(TERMDATA_INI_FILE);
	CINIFile iniEMV(GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()) );	// [#2226] NH Justin 2013.10.22 Load term data depends on Kernel Version

	char buf[512]		= { 0, };
	char section[64]	= { 0, };	// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
	CString strTemp;

	iniEMV.GetPrivateProfileString("AIDCOUNT","TOTALCOUNT", buf, sizeof(buf));		
	m_Standard1_Terminal_AIDList.nTotalAIDCount = __min(Asc2Int((CString)buf), MAX_AIDLIST_CNT);
	nTerminalRemainAIDCnt = m_Standard1_Terminal_AIDList.nTotalAIDCount;	// Terminal Remain Count를 Set한다.

	NHDEBUG(1, (_T("***BIZ_EMV_UpdateAIDListProc*** TotalAID Count [%d]\n"), m_Standard1_Terminal_AIDList.nTotalAIDCount));

	for(int i=0; i<m_Standard1_Terminal_AIDList.nTotalAIDCount; i++)
	{
		memset(buf, 0, sizeof(buf));
		memset(section, 0, sizeof(section));

		sprintf(section,"TERMINAL%d",i+1);
		NHDEBUG(1, (_T("***BIZ_EMV_UpdateAIDListProc*** Section [%S]\n"), section));

		iniEMV.GetPrivateProfileString(section, "AID", m_Standard1_Terminal_AIDList.chAID[i], sizeof(m_Standard1_Terminal_AIDList.chAID[i]));
		NHDEBUG(1, (_T("***BIZ_EMV_UpdateAIDListProc*** AID [%S]\n"), m_Standard1_Terminal_AIDList.chAID[i]));
	}

	while(TRUE)
	{
		BIZ_HostConfigExtendedProc(TRAN_EXTENDED_AID_UPDATE, TC_EXTENDED_AID_UPDATE);

		// while문을 빠져나가는 조건을 건다.
		if (m_pDevCmn->TranResult == TRUE)
		{
			if (nTerminalRemainAIDCnt <= 0 && nHostRemainBlock <= 0)
			{
				// 정합성 Check
				if ((nCalcRecvTotalAIDCnt == m_Standard1_Host_AIDList.nTotalAIDCount) && (m_Standard1_Host_AIDList.nTotalAIDCount != 0))
				{
					// 송 / 수신 완료 AID를 update한다.
					if (BIZ_EMV_SaveAIDList() == T_OK)
					{
						NHDEBUG(1, (_T("***BIZ_EMV_UpdateAIDListProc*** File update SUCCESS!! [%d] [%d] \n"), nCalcRecvTotalAIDCnt, m_Standard1_Host_AIDList.nTotalAIDCount));
						return RES_OK;
					}
					else
					{
						// File Write Fail (NVRAM LOG WRITE)
						NHDEBUG(1, (_T("***BIZ_EMV_UpdateAIDListProc*** File update Write Fail \n")));
						return RES_NG;
					}
				}
				else
				{
					// Total Count Error (NVRAM LOG WRITE)
					NHDEBUG(1, (_T("***BIZ_EMV_UpdateAIDListProc*** Total Count Error [%d] [%d] \n"), nCalcRecvTotalAIDCnt, m_Standard1_Host_AIDList.nTotalAIDCount));
					return RES_NG;
				}
			}
		}
		else
		{
			// 송 / 수신 장애이므로 AID를 update하지 않는다.
			NHDEBUG(1, (_T("***BIZ_EMV_UpdateAIDListProc*** Total Count Error [%d] [%d] \n"), nCalcRecvTotalAIDCnt, m_Standard1_Host_AIDList.nTotalAIDCount));
			return RES_NG;
		}
	}

	return RES_OK;
}
// end of [#2081]

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_MakeICDataforTrans()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Transaction을 위한 IC CARD 정보를 추출한다.
-------------------------------------------------------------------*/
//void CTranCmn::BIZ_EMV_MakeICDataforTrans()
void CTranCmn::BIZ_EMV_MakeICDataforTrans(CString sMsgFormat, CString* pstrEMV)		// [#2237] US Justin 2013.12.05 Dual Host DCC EMV field separator Bug fix  // [#2292] US Justin 2014.09.25 Support TDL. Add Parameter to get whole value (default = NULL)
{
	CString			strTmp_Script;
	CString			strTagLenValue;
	CString			strValue;
	unsigned char	szTemp[1024] = {0,};
	int				nLen = 0;

	/************************************************************************/
	/*  Common EMV Send Data (Withdrawal, Inquiry )				            */
	/************************************************************************/
	// Initialization Temp Script Buffer
	strTmp_Script.Empty();

	// KSK 2009.8.31 캐나다 Interac 사양에 맞춰서 Data를 전송하도록 수정
	// [#397] NH PSC 2008.12.15 EMV 거래시 Host로 전송하는 Tag 추가
	// [#2496] US Jutin 2017.08.18 Add bitcoin(NonCash Withdrawal)
	if (TranCode == TC_WITHDRAWAL || TranCode == TC_INQUIRY || TranCode == TC_TRANSFER || TranCode == TC_JUSTCASH_BITCOIN || TranCode == TC_DIGITALMINT)
	{
		//////////////////////////////////////////////////////////////////////////
		// Acquirer Identifier (9F01) (Len : 6Byte) - Interac 사양서에 값이 정의되어 있지 않음

		//////////////////////////////////////////////////////////////////////////
		// amount authorized (9F02) (Len : 6Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AmountAuthN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* transaction category code Max Len 6 */
			if( nLen > 0 && nLen <= 6 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", AMOUNT_AUTHORIZED_TAG_9F02, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() amount authorized[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// amount other (9F03) (Len : 6Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AmountOtherN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* transaction category code Max Len 6 */
			if( nLen > 0 && nLen <= 6 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", AMOUNT_OTHER_TAG_9F03, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() amount other[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}
		else
		{
			// 캐나다 opensolution 요청사항 (INTERAC 사양서오류)
			// OTHER AMOUNT값이 없을 경우에도 반드시 0으로 채워서 송신해야함.
			strTagLenValue.Format(L"%s%02X%s", AMOUNT_OTHER_TAG_9F03, 0x06, L"000000000000");
			NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() amount other[%s]\n"), strTagLenValue));

			strTmp_Script += strTagLenValue; 
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Cryptogram (9F26)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AC_REQ_9F26_VALUE, strValue);	// [#2115] MX KSK 2012.03.05 Journal에 저장하기 위해 NVRAM에 저장 Request 값 유지 (고객 요청사항)

			/* Application Cryptogram  Max Len 8 */
			if( nLen > 0 && nLen <= 8 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_CRYPTOGRAM_TAG_9F26, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Application Cryptogram[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}	

		//////////////////////////////////////////////////////////////////////////
		// Application Expired Date (5F24) (Len : 3)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AppExpiredDate, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Expired Date  Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_EXPIRED_DATE_TAG_5F24, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Application Expired Date[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Interchange Profile (82) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AIP, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Interchange Profile Max Len 2*/
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_INTERCHANGE_PROFILE_TAG_82, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Application Interchange Profile[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Primary Account Number(PAN) (5A) (Len : var)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_PAN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Primary Account Number Sequence */
			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APPLICATION_PAN_TAG_5A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Application Primary Account Number[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Primary Account Number Sequence Number (5F34) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_PANSeqNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Primary Account Number Sequence Number Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APPLICATION_PAN_SEQ_TAG_5F34, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Application Primary Account Number Sequence Number[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Transaction Counter (9F36) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_ATC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Transaction Counter Len 2*/
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_TRANSACTION_COUNTER_TAG_9F36, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block()  Application Transaction Counter[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Card holder Verification Method(CVM) Result (9F34) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CVMRslt, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Cardholder Verification Method(CVM) Result Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", CVM_RESULT_TAG_9F34, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Cardholder Verification Method(CVM) Result[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Cryptogram Information Data  (9F27) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Cryptogram Information Data Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", CRYPTOGRAM_INFO_DATA_TAG_9F27, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Cryptogram Information[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Issuer Application Data (9F10) (Len : 32Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuAppData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Issuer Application Data Max Len 32*/
			if( nLen > 0 && nLen <= 32 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", ISSUER_APP_DATA_TAG_9F10, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Issuer Application Data[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Point Of Service Entry Mode Code [POS -> 0x05 Fixed] (9F39) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_POSEntryMode, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Point Of Service Entry Mode Code Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", POS_ENTRY_MODE_TAG_9F39, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Point Of Service Entry Mode Code[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Capabilities (9F33) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TerminalCapa, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Capabilities Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_CAPABILITIES_TAG_9F33, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Terminal Capabilities[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Country Code (9F1A) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TCountryCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Country Code Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_COUNTRY_CODE_TAG_9F1A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Terminal Country Code[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}	
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Type (9F35) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TerminalType, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Type Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_TYPE_TAG_9F35, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Terminal Type Max[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Verification Result (95) (Len : 5Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TVR, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Verification Result Max Len 5 */
			if( nLen > 0 && nLen <= 5 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_VERIFICATION_RESULT_TAG_95, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Terminal Verification Result[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Track2 Equivalent Data (57) (Len : 19Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_Track2EquData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRACK2_EQUIVALENT_DATA_TAG_57, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Track2 Equivalent[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Currency Code (5F2A) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrCurCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Currency Code Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_CURRENCY_CODE_TAG_5F2A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Terminal Currency Code[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Transaction Date (9A) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrDate, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Transaction Date Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_DATE_TAG_9A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Transaction Date[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// transaction sequence counter (9F41) (Len : 4Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrSeqCnt, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* transaction category code Max Len 4 */
			if( nLen > 0 && nLen <= 4 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_SEQUENCE_COUNTER_TAG_9F41, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() transaction sequence counter[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Transaction Time (9F21) (Len : 3 Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrTime, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Transaction Time Max Len 3 */
			if( nLen > 0 && nLen <= 3) 
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_TIME_TAG_9F21, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Transaction Time[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Transaction Type (9C) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrType, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Transaction Type Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_TYPE_TAG_9C, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Transaction Type[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Unpredictable Number (9F37) (Len : 4Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_UnpredictNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Unpredictable Number Max Len 4 */
			if( nLen > 0 && nLen <= 4 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", RANDOM_NUMBER_TAG_9F37, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Unpredictable Number[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}
		// 여기까지 캐나다 Interac 최소 전송 사양임

		//////////////////////////////////////////////////////////////////////////
		// Application Identifier (AID) (9F06) (Len : 16Byte) - Interac 사양서에는 N/T이나 현재 송신하고 있음
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TerminalAID, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application ID Max Len 16 */
			if( nLen > 0 && nLen <= 16) 
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APPLICATION_ID_TAG_9F06, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Application ID[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Version Number (9F09)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrAppVerNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Version Number Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_VERSION_NUMBER_TAG_9F09, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block()Application Version Number[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Dedicated file name (84)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_DFName, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Dedicated file name Max Len 16 */
			if( nLen > 0 && nLen <= 16 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", DEDICATED_FILE_NAME_TAG_84, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Dedicated file name[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// IFD(Interface Device) serial number (9F1E)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IFDSerialNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* IFD(Interface Device) serial number Max Len 8 */
			if( nLen > 0 && nLen <= 8 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", IFD_SERIAL_NUMBER_TAG_9F1E, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() IFD serial number[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// transaction category code (9F53)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TCC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* transaction category code Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_CATEGORY_CODE_TAG_9F53, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() transaction category code[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Cardholder Verification Method(CVM) List (8E)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CVMList, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Cardholder Verification Method(CVM) List Len 252 */
			if( nLen > 0 && nLen <= 252 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", CVM_LIST_TAG_8E, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block() Cardholder Verification Method[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Transaction Status Information (9B)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TSI, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Transaction Status Information Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_STATUS_INFO_TAG_9B, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::Make_EMV_Data_Block()Transaction Status Information[%s]\n"), strTagLenValue));

				strTmp_Script += strTagLenValue; 
			}
		}

		/* Tag : 9F18 */ // Reversal에만 9F18을 송신하도록 함 (TNS 요청사항)
//		strTagLenValue.Empty();
//		strTagLenValue = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_ID_TLV);
//		if( strTagLenValue.GetLength() != 0 )
//			strTmp_Script += strTagLenValue;
//		strTagLenValue.Empty();

		// [#68] UK HWANG 2008.05.13 source review 수정
		// Script Data가 없을 경우, 'ud' Field를 보내지 않는다.
		if( strTmp_Script.GetLength() != 0 )
		{
			// [#2292] US Justin 2014.09.25 Get EMV Whold Data, not attaching in Msg for TDL
			/*
			//if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)	// standard#3 와 standard#1 은 FS가 붙는 위치가 틀림.
			if (sMsgFormat == MSG_TRITON_TYPE)	// standard#3 와 standard#1 은 FS가 붙는 위치가 틀림. // [#2237] US Justin 2013.12.05 Dual Host DCC EMV Field Separator Bug Fix
			{
				m_strSendData += L"ud" + strTmp_Script;
				m_strSendData += FIELD_DELIMITER;
			}
			else
			{
				m_strSendData += FIELD_DELIMITER;
				m_strSendData += L"ud" + strTmp_Script;
			}
			*/
			if(pstrEMV != NULL)
				pstrEMV->Format(L"%s", strTmp_Script);
			else
			{
				if (sMsgFormat == MSG_TRITON_TYPE)	// standard#3 와 standard#1 은 FS가 붙는 위치가 틀림. // [#2237] US Justin 2013.12.05 Dual Host DCC EMV Field Separator Bug Fix
				{
					m_strSendData += L"ud" + strTmp_Script;
					m_strSendData += FIELD_DELIMITER;
				}
				else
				{
					m_strSendData += FIELD_DELIMITER;
					m_strSendData += L"ud" + strTmp_Script;
				}
			}
			// End of [#2292]
		}
	}
	/************************************************************************/
	/*  Common EMV Send Data ( Reversal )						            */
	/************************************************************************/
	else
	{
		// KSK 2009.8.31 캐나다 Interac 사양에 맞춰서 Data를 전송하도록 수정

		CString strReversalTLV;

		/* Tag : 9F02 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AMOUNT_AUTHORIZED_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F03 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AMOUNT_OTHER_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F26 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_CRYPTOGRAM_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 5F24 */ // Reversal 전문에는 송신 안함
//		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_EXPIRED_DATE_TLV);
//		if( strReversalTLV.GetLength() != 0 )
//			strTmp_Script += strReversalTLV;
//		strReversalTLV.Empty();

		/* Tag : 82 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_INTERCHANGE_PRO_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 5A */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_ACCOUNT_NUMBER_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 5F34 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_ACCOUNT_NUMBER_SEQ_NUMBER_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();		

		/* Tag : 9F36 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_TRAN_COUNTER_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F27 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_CRYPTOGRAM_INFO_DATA_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F10 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_APP_DATA_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F39 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_POS_ENTRY_MODE_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F33 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_CAPABILITIES_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F1A */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_COUNTRY_CODE_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F35 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_TYPE_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 95 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_VERIFY_RESULT_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 57 */ // 사양협의 필요
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRACK2_EQU_DATA_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 5F2A */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_CURRENCY_CODE_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9A */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_DATE_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();	

		/* Tag : 9F41 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_SEQ_COUNTER_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();	

		/* Tag : 9C */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_TYPE_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F37 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_RANDOM_NUMBER_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();	
		// 여기까지 캐나다 Interac 사양임

		/* Tag : 9F06 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APPLICATION_ID_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F1E */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_IFD_SERIAL_NUMBER_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9F18 */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_ID_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		/* Tag : 9B */
		strReversalTLV = MemGetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_STATUS_INFO_TLV);
		if( strReversalTLV.GetLength() != 0 )
			strTmp_Script += strReversalTLV;
		strReversalTLV.Empty();

		// [#68] UK HWANG 2008.05.13 source review 수정
		// Script Data가 없을 경우, 'ud' Field를 보내지 않는다.
		if( strTmp_Script.GetLength() != 0 )
		{
			// [#2292] US Justin 2014.09.25 Get EMV Whold Data, not attaching in Msg for TDL
			/*
			//if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)	// standard#3 와 standard#1 은 FS가 붙는 위치가 틀림.
			if (sMsgFormat == MSG_TRITON_TYPE)	// standard#3 와 standard#1 은 FS가 붙는 위치가 틀림. // [#2237] US Justin 2013.12.05 Dual Host DCC EMV Field Separator Bug Fix
			{
				m_strSendData += L"ud" + strTmp_Script;
				m_strSendData += FIELD_DELIMITER;
			}
			else
			{
				m_strSendData += FIELD_DELIMITER;
				m_strSendData += L"ud" + strTmp_Script;
			}
			*/
			if(pstrEMV != NULL)
				pstrEMV->Format(L"%s", strTmp_Script);
			else
			{
				if (sMsgFormat == MSG_TRITON_TYPE)	// standard#3 와 standard#1 은 FS가 붙는 위치가 틀림. // [#2237] US Justin 2013.12.05 Dual Host DCC EMV Field Separator Bug Fix
				{
					m_strSendData += L"ud" + strTmp_Script;
					m_strSendData += FIELD_DELIMITER;
				}
				else
				{
					m_strSendData += FIELD_DELIMITER;
					m_strSendData += L"ud" + strTmp_Script;
				}
			}
			// End of [#2292]
		}
	}
}

/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_MakeICDataforReversal()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Reversal을 위한 IC CARD 정보를 추출한다.
-------------------------------------------------------------------*/
void CTranCmn::BIZ_EMV_MakeICDataforReversal(int nIssuerScriptLen)
{
	unsigned char szTemp[1024];
	CString strValue;
	CString strTagLenValue;
	int		nLen;

	// [#397] NH PSC 2008.12.15 EMV 거래시 EMV_OnlineApproval 이후 커널에서 달라지는 데이터 다시 저장
	{	// Make reversal data for canada EMV
		// KSK 2009.8.31 캐나다 Interac 최소 사양으로 인해 수정
		//////////////////////////////////////////////////////////////////////////
		// amount authorized (9F02) (Len : 6Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AmountAuthN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* transaction category code Max Len 6 */
			if( nLen > 0 && nLen <= 6 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", AMOUNT_AUTHORIZED_TAG_9F02, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() amount authorized[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AMOUNT_AUTHORIZED_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// amount other (9F03) (Len : 6Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AmountOtherN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* transaction category code Max Len 6 */
			if( nLen > 0 && nLen <= 6 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", AMOUNT_OTHER_TAG_9F03, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() amount other[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AMOUNT_OTHER_TLV, strTagLenValue);
			}
		}
		else		// [#559] CA KSK 2009.08.19
		{
			// 캐나다 opensolution 요청사항 (INTERAC 사양서오류)
			// OTHER AMOUNT값이 없을 경우에도 반드시 0으로 채워서 송신해야함.
			strTagLenValue.Format(L"%s%02X%s", AMOUNT_OTHER_TAG_9F03, 0x06, L"000000000000");
			NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() amount other[%s]\n"), strTagLenValue));

			/* Save Reversal Data into NV-RAM */
			MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_AMOUNT_OTHER_TLV, strTagLenValue); 
		}			// end of [#559]

		//////////////////////////////////////////////////////////////////////////
		// Application Cryptogram (9F26)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Cryptogram  Max Len 8 */
			if( nLen > 0 && nLen <= 8 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_CRYPTOGRAM_TAG_9F26, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Application Cryptogram[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_CRYPTOGRAM_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Interchange Profile (82) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_AIP, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Interchange Profile Max Len 2*/
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_INTERCHANGE_PROFILE_TAG_82, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Application Interchange Profile[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_INTERCHANGE_PRO_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Primary Account Number(PAN) (5A) (Len : var)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_PAN, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Primary Account Number Sequence */
			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APPLICATION_PAN_TAG_5A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Application Primary Account Number[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_ACCOUNT_NUMBER_TLV, strTagLenValue); 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Primary Account Number Sequence Number (5F34) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_PANSeqNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Primary Account Number Sequence Number Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APPLICATION_PAN_SEQ_TAG_5F34, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Application Primary Account Number Sequence Number[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_ACCOUNT_NUMBER_SEQ_NUMBER_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Application Transaction Counter (9F36)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_ATC, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Application Transaction Counter Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", APP_TRANSACTION_COUNTER_TAG_9F36, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal()  Application Transaction Counter[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_APP_TRAN_COUNTER_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Cryptogram Information Data (9F27)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Cryptogram Information Data Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", CRYPTOGRAM_INFO_DATA_TAG_9F27, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Cryptogram Information[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_CRYPTOGRAM_INFO_DATA_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Issuer Application Data (9F10)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuAppData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Issuer Application Data Max Len 32 */
			if( nLen > 0 && nLen <= 32 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", ISSUER_APP_DATA_TAG_9F10, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Issuer Application Data[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_APP_DATA_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Point Of Service Entry Mode Code [POS -> 0x05 Fixed] (9F39) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_POSEntryMode, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Point Of Service Entry Mode Code Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", POS_ENTRY_MODE_TAG_9F39, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Point Of Service Entry Mode Code[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_POS_ENTRY_MODE_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Capabilities (9F33) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TerminalCapa, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Capabilities Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_CAPABILITIES_TAG_9F33, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Terminal Capabilities[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_CAPABILITIES_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Country Code (9F1A) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TCountryCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Country Code Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_COUNTRY_CODE_TAG_9F1A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Terminal Country Code[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_COUNTRY_CODE_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Type (9F35) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TerminalType, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Type Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_TYPE_TAG_9F35, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Terminal Type Max[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_TYPE_TLV, strTagLenValue); 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Verification Result (95)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TVR, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Verification Result Max Len 5 */
			if( nLen > 0 && nLen <= 5 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TERMINAL_VERIFICATION_RESULT_TAG_95, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Terminal Verification Result[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TERMINAL_VERIFY_RESULT_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Track2 Equivalent Data (57) (Len : 19Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_Track2EquData, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			if( nLen > 0 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRACK2_EQUIVALENT_DATA_TAG_57, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Track2 Equivalent[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRACK2_EQU_DATA_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Terminal Currency Code (5F2A) (Len : 2Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrCurCod, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Terminal Currency Code Max Len 2 */
			if( nLen > 0 && nLen <= 2 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_CURRENCY_CODE_TAG_5F2A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Terminal Currency Code[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_CURRENCY_CODE_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Transaction Date (9A) (Len : 3Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrDate, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Transaction Date Max Len 3 */
			if( nLen > 0 && nLen <= 3 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_DATE_TAG_9A, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Transaction Date[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_DATE_TLV, strTagLenValue);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// transaction sequence counter (9F41) (Len : 4Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrSeqCnt, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* transaction category code Max Len 4 */
			if( nLen > 0 && nLen <= 4 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_SEQUENCE_COUNTER_TAG_9F41, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() transaction sequence counter[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_SEQ_COUNTER_TLV, strTagLenValue); 
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Transaction Type (9C) (Len : 1Byte)
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_TrType, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Transaction Type Max Len 1 */
			if( nLen > 0 && nLen <= 1 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", TRANSACTION_TYPE_TAG_9C, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Transaction Type[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_TRANSACTION_TYPE_TLV, strTagLenValue); 
			}
		}

		// [#498] [CA] TNS 요청사항으로 Reversal시 9F37 tag값 송신함 2009.3.18
		//////////////////////////////////////////////////////////////////////////
		// Unpredictable Number
		nLen = 0;
		strValue.Empty();
		memset(szTemp, 0x0, sizeof(szTemp));
		if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_UnpredictNo, &nLen, szTemp) == EMV_RSLT_OK)
		{
			strValue = MakeUnPack(szTemp, nLen);

			/* Unpredictable Number Max Len 4 */
			if( nLen > 0 && nLen <= 4 )
			{
				/* Tag + Length + Value */
				strTagLenValue.Format(L"%s%02X%s", RANDOM_NUMBER_TAG_9F37, nLen, strValue);
				NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Unpredictable Number[%s]\n"), strTagLenValue));

				/* Save Reversal Data into NV-RAM */
				MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_RANDOM_NUMBER_TLV, strTagLenValue);	// [#487] [NH] KSK 2009.1.20 Host 송/수신후 저장하도록 위치 수정
			}
		}
		// end of [#498]

		// 여기까지 캐나다 Interac 최소 송신 사양임.

		//////////////////////////////////////////////////////////////////////////
		// issuer script ID and results (9F18)
		if(nIssuerScriptLen > 0)
		{
			nLen = 0;
			strValue.Empty();
			memset(szTemp, 0x0, sizeof(szTemp));
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuScriptId, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strValue = MakeUnPack(szTemp, nLen);

				/* Issuer script identifier Max Len 4 */
				if( nLen > 0 && nLen <= 4 )
				{
					CString strTagLenValue;
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02X%s", ISSUER_SCRIPT_ID_TAG_9F18, nLen, strValue);
					NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Issuer script ID[%s]\n"), strTagLenValue));

					/* Save Reversal Data into NV-RAM */
					MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_ID_TLV, strTagLenValue);
				}
			}

			nLen = 0;
			strValue.Empty();
			memset(szTemp, 0x0, sizeof(szTemp));
			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_IsuScriptRslt, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strValue = MakeUnPack(szTemp, nLen);

				/* Issuer script result Max Len 128 */
				if( nLen > 0 && nLen <= 128 )
				{
					CString strTagLenValue;
					/* Tag + Length + Value */
					strTagLenValue.Format(L"%s%02X%s", ISSUER_SCRIPT_RESULTS_TAG_DF05, nLen, strValue);
					NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_MakeICDataforReversal() Issuer script result[%s]\n"), strTagLenValue));

					/* Save Reversal Data into NV-RAM */
					MemSetStr(_MEM_FLD_EMVREVERSAL, _MEM_VAR_ISSUER_SCRIPT_RESULTS_TLV, strTagLenValue);
				}
			}
		}
	}	
}


/*-------------------------------------------------------------------
CLASS    NAME: CTranCmn
FUNCTION NAME: BIZ_EMV_Anal_ICDataforTrans()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Host에서 받은 IC CARD를 가지고 처리한다.
-------------------------------------------------------------------*/
int CTranCmn::BIZ_EMV_Anal_ICDataforTrans(CString strMicellaneous)
{
	NHDEBUG(1, (_T("strMicellaneous [%s]\n"), strMicellaneous));

	// [#488] [NH] KSK 2009.1.22
	CString strEMVDataBlock;
	char	szResponseCd[8];		/* MAX 2  Bytes */
	char	szARPCData[40];			/* MAX 16  Bytes */
	char	szIssuScriptData1[266]; /* MAX 261 Bytes */		// [#2022] NH KSK 2011.02.22
	char	szIssuScriptData2[266]; /* MAX 261 Bytes */		// [#2022] NH KSK 2011.02.22
	char	szIssuScriptData[532];  /* MAX 522 Bytes */		// [#2022] NH KSK 2011.02.22
	char	szEMVScriptData[2048];
	char	szEMVPackData[1024];
	UINT    nARCLen				= 0;
	UINT	nARPCLen			= 0;
	UINT	nIssuScriptData1    = 0;
	UINT	nIssuScriptData2    = 0;
	UINT	nIsuScriptLen		= 0;
	UINT	nValueLen			= 0;
	int		nResult				= 0;
	int		nTagLen				= 0;
	int		nIndex				= 0;
	int		i					= 0;
	int		j					= 0;

	int		nReturnCode = 0;	// [#487] [NH] KSK 2009.1.20
	CString strTemp;

	/* Local variable initialization */
	memset(szResponseCd, 0x00, sizeof(szResponseCd));
	memset(szARPCData, 0x00, sizeof(szARPCData));
	memset(szIssuScriptData1, 0x00, sizeof(szIssuScriptData1));
	memset(szIssuScriptData2, 0x00, sizeof(szIssuScriptData2));
	memset(szIssuScriptData, 0x00, sizeof(szIssuScriptData));
	memset(szEMVScriptData, 0x00, sizeof(szEMVScriptData));
	memset(szEMVPackData, 0x00, sizeof(szEMVPackData));

	/* search FS after 'ud' */
	nIndex = strMicellaneous.Find(FIELD_DELIMITER);

	/* ud Field 이후 Micellaneous data가 존재 할 경우 */
	if( nIndex != -1)
		strEMVDataBlock = strMicellaneous.Left(nIndex);
	else
		strEMVDataBlock = strMicellaneous;

	//////////////////////////////////////////////////////////////////////////
	// Covert Wide to Multi Byte
	int nLen =  strEMVDataBlock.GetLength();

	if( nLen > sizeof(szEMVScriptData))
		nLen = 2048;

	WideToMulti((LPSTR)szEMVScriptData, strEMVDataBlock, nLen);
	NHDEBUG(1, (_T("szEMVScriptData[%s]\n"), strEMVDataBlock));

	//////////////////////////////////////////////////////////////////////////
	// Convert ASCII to HEX Data.
	nLen = MakePack(szEMVScriptData, szEMVPackData, nLen);

	// [#559] CA KSK 2009.08.19
	// EMV Data 정합성을 Check 위해 Tag별로 Parsing을 한다.
	BOOL	bTagValidCheck		= FALSE;
	nIndex			 = 0;

	// AUTH RESP, AUTH DATA, ISSUER SCRP1, ISSUER SCRP2
//	char	szTagTable[4] = { (char)0x8A, (char)0x91, (char)0x71, (char)0x72 };	// Tag Define
	char	szTagTable[5] = { (char)0x8A, (char)0x91, (char)0x71, (char)0x72, (char)0x89 };	// Tag Define KSK 2010.09.13 89수신 시 Invalid 처리 안하도록 수정

	for (i=0; i<nLen; i = nIndex)
	{
		NHDEBUG(1, (_T("Remain Data [%s] i Value [%d] Total Length [%d] nIndex [%d]\n"), MakeUnPack(&szEMVPackData[nIndex], nLen - nIndex), i, nLen, nIndex));

		bTagValidCheck = FALSE;
		nValueLen = 0;

		for(j=0; j<sizeof(szTagTable); j++)
		{
			// Define 되어져 있는 Tag를 찾는다.
			if (szEMVPackData[nIndex] == szTagTable[j])
			{
				bTagValidCheck = TRUE;
				break;
			}
		}

		if (bTagValidCheck == TRUE)
		{
			if (j == 0)		// Tag : "8A"
			{
				nARCLen = szEMVPackData[nIndex+1];

				/* AUTHORISATION RESPONSE 2Bytes*/
				if (nARCLen != 2)	// Fixed Length Error
				{
					bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
					break;
				}

				memcpy(szResponseCd, &szEMVPackData[nIndex+2], nARCLen);

				nIndex += 1 + 1 + nARCLen;	// Tag(1Byte) + Len(1Byte) + Value

				// KSK 2010.02.10 Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
				if (nIndex > nLen)
				{
					bTagValidCheck = FALSE;
					break;
				}
				// end of KSK 2010.02.10

				// [#643] CA KSK 2010.07.13
				// Master Card 인증 대응 (ARC 값이 3030 ~ 3939 값 이외에는 3035로 처리 하도록 수정)
//				if ((szResponseCd[0] < 0x30 || szResponseCd[0] > 0x39) && (szResponseCd[1] < 0x30 || szResponseCd[0] > 0x39))
				if ((szResponseCd[0] < 0x30 || szResponseCd[0] > 0x39) || (szResponseCd[1] < 0x30 || szResponseCd[1] > 0x39))	// KSK 2010.08.26 Bug Fix
				{
					nARCLen = 0x02;
					szResponseCd[0] = m_szDefaultResponseCd[0];	// KSK 2010.08.29
					szResponseCd[1] = m_szDefaultResponseCd[1];	// KSK 2010.08.29
				}
				// end of [#643]

				NHDEBUG(1, (_T("[8A] PROC szResposeCode[%s] nARCLen[%d] nIndex[%d]\n"), MakeUnPack(szResponseCd, nARCLen), nARCLen, nIndex));
			}
			else if (j == 1)	// "91" Tag
			{
				nARPCLen = szEMVPackData[nIndex+1];

				/* ISSUER AUTHENTICATION 8~16 Bytes : MAX 16Bytes */
				if (nARPCLen < 8 || nARPCLen > 16)
				{
					bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
					break;
				}

				memcpy(szARPCData, &szEMVPackData[nIndex+2], nARPCLen);

				nIndex += 1 + 1 + nARPCLen;	// Tag(1Byte) + Len(1Byte) + Value

				// KSK 2010.02.10 Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
				if (nIndex > nLen)
				{
					bTagValidCheck = FALSE;
					break;
				}
				// end of KSK 2010.02.10

				NHDEBUG(1, (_T("[91] PROC ARPCData[%s] nARPCLen[%d] nIndex[%d]\n"), MakeUnPack(szARPCData, nARPCLen), nARPCLen, nIndex));
			}
			else if (j == 2)	// "71" Tag
			{
				// 0x82 or 0x83일 경우에 한해서 length field가 3byte가 됨
				if (szEMVPackData[nIndex+1] == 0x82)			// 0x82에 대해서는 coding되어져 있음
				{
					/* Double Length(0xFFFF) MSB */
					nValueLen = (BYTE)szEMVPackData[nIndex + 2];
					nValueLen <<= 8;
					/* Double Length LSB */
					nValueLen |= szEMVPackData[nIndex + 3];

					if (nValueLen < 0)		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
						break;
					}						// end of [#2022]

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData1 > 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
						break;
					}

					// [#GLDV-2522] NH Kook 2019.07.05 Support EMV Kernel V7.0
					// V7.0) extract 71 tag as 'Value' only like other tags.
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION) == EMV_KERNEL_V7)
					{
						memcpy(&szIssuScriptData1[nIssuScriptData1], &szEMVPackData[nIndex + 1 + 3], nValueLen);
						nIssuScriptData1 += nValueLen;

						NHDEBUG(1, (_T("[71] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData1, nIssuScriptData1), nValueLen, nIndex));
						nIndex += 1 + 3 + nValueLen;	// Tag(1Byte) + Len(3Byte) + Value
					}
					else
					// ~ V6.0) extract 71 tag as 'Tag + Len + Value'.
					// end of [#GLDV-2522]
					{
						memcpy(&szIssuScriptData1[nIssuScriptData1], &szEMVPackData[nIndex], 1 + 3 + nValueLen);
						nIssuScriptData1 += 1 + 3 + nValueLen;

						NHDEBUG(1, (_T("[71] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData1, nIssuScriptData1), 1 + 3 + nValueLen, nIndex));
						nIndex += 1 + 3 + nValueLen;	// Tag(1Byte) + Len(3Byte) + Value
					}

					// KSK 2010.02.10 Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}
					// end of KSK 2010.02.10
				}
				else if (szEMVPackData[nIndex+1] == 0x83)	// 0x83은 추후 필요시 추가 coding 필요
				{
					;
				}
				else
				{
					nValueLen = szEMVPackData[nIndex+1];

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData1> 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
						break;
					}

					// [#GLDV-2522] NH Kook 2019.07.05 Support EMV Kernel V7.0
					// V7.0) extract 71 tag as 'Value' only like other tags.
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION) == EMV_KERNEL_V7)
					{
						memcpy(&szIssuScriptData1[nIssuScriptData1], &szEMVPackData[nIndex + 1 + 1], nValueLen);
						nIssuScriptData1 += nValueLen;

						NHDEBUG(1, (_T("[71] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData1, nIssuScriptData1), nValueLen, nIndex));
						nIndex += 1 + 1 + nValueLen;	// Tag(1Byte) + Len(1Byte) + Value
					}
					else
					// ~ V6.0) extract 71 tag as 'Tag + Len + Value'.
					// end of [#GLDV-2522]
					{
						memcpy(&szIssuScriptData1[nIssuScriptData1], &szEMVPackData[nIndex], 1 + 1 + nValueLen);
						nIssuScriptData1 += 1 + 1 + nValueLen;

						NHDEBUG(1, (_T("[71] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData1, nIssuScriptData1), 1 + 1 + nValueLen, nIndex));
						nIndex += 1 + 1 + nValueLen;	// Tag(1Byte) + Len(1Byte) + Value
					}

					// KSK 2010.02.10 Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}
					// end of KSK 2010.02.10
				}
			}
			else if (j == 3)	// "72" Tag
			{
				// 0x82 or 0x83일 경우에 한해서 length field가 3byte가 됨
				if (szEMVPackData[nIndex+1] == 0x82)			// 0x82에 대해서는 coding되어져 있음
				{
					/* Double Length(0xFFFF) MSB */
					nValueLen = (BYTE)szEMVPackData[nIndex + 2];
					nValueLen <<= 8;
					/* Double Length LSB */
					nValueLen |= szEMVPackData[nIndex + 3];

					if (nValueLen < 0)		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
						break;
					}						// end of [#2022]

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData2 > 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
						break;
					}

					// [#GLDV-2522] NH Kook 2019.07.05 Support EMV Kernel V7.0
					// V7.0) extract 72 tag as 'Value' only like other tags.
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION) == EMV_KERNEL_V7)
					{
						memcpy(&szIssuScriptData2[nIssuScriptData2], &szEMVPackData[nIndex + 1 + 3], nValueLen);
						nIssuScriptData2 += nValueLen;
						NHDEBUG(1, (_T("[72] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData2, nIssuScriptData2), nValueLen, nIndex));
						nIndex += 1 + 3 + nValueLen;	// Tag(1Byte) + Len(3Byte) + Value
					}
					else
					// ~ V6.0) extract 72 tag as 'Tag + Len + Value'.
					// end of [#GLDV-2522]
					{
						memcpy(&szIssuScriptData2[nIssuScriptData2], &szEMVPackData[nIndex], 1 + 3 + nValueLen);
						nIssuScriptData2 += 1 + 3 + nValueLen;
						NHDEBUG(1, (_T("[72] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData2, nIssuScriptData2), nValueLen, nIndex));
						nIndex += 1 + 3 + nValueLen;	// Tag(1Byte) + Len(3Byte) + Value
					}

					// KSK 2010.02.10 Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}
					// end of KSK 2010.02.10

					NHDEBUG(1, (_T("[72] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData2, nIssuScriptData2), 1 + 3 + nValueLen, nIndex));

				}
				else if (szEMVPackData[nIndex+1] == 0x83)	// 0x83은 추후 필요시 추가 coding 필요
				{
					;
				}
				else
				{
					nValueLen = szEMVPackData[nIndex+1];

					/* ISSUER SCRIPT MAX 261 */
					if (nValueLen + nIssuScriptData2> 261)	// Length Error
					{
						bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
						break;
					}

					// [#GLDV-2522] NH Kook 2019.07.05 Support EMV Kernel V7.0
					// V7.0) extract 72 tag as 'Value' only like other tags.
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_KERNELVERSION) == EMV_KERNEL_V7)
					{
						memcpy(&szIssuScriptData2[nIssuScriptData2], &szEMVPackData[nIndex + 1 + 1], nValueLen);
						nIssuScriptData2 += nValueLen;
						NHDEBUG(1, (_T("[72] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData2, nIssuScriptData2), nValueLen, nIndex));
						nIndex += 1 + 1 + nValueLen;	// Tag(1Byte) + Len(1Byte) + Value
					}
					else
					// ~ V6.0) extract 72 tag as 'Tag + Len + Value'.
					// end of [#GLDV-2522]
					{
						memcpy(&szIssuScriptData2[nIssuScriptData2], &szEMVPackData[nIndex], 1 + 1 + nValueLen);
						nIssuScriptData2 += 1 + 1 + nValueLen;
						NHDEBUG(1, (_T("[72] PROC ISSUER SCRIPT[%s] nValueLen[%d] nIndex[%d]\n"), MakeUnPack(szIssuScriptData2, nIssuScriptData2), 1 + 1 + nValueLen, nIndex));
						nIndex += 1 + 1 + nValueLen;	// Tag(1Byte) + Len(1Byte) + Value
					}

					// KSK 2010.02.10 Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
					if (nIndex > nLen)
					{
						bTagValidCheck = FALSE;
						break;
					}
					// end of KSK 2010.02.10
				}
			}
			else if (j == 4)	// "89" Tag
			{
				// KSK 2010.09.13
				nValueLen = szEMVPackData[nIndex+1];

				/* AUTHORISATION CODE 6Bytes*/
				if (nValueLen != 6)	// Fixed Length Error
				{
					bTagValidCheck = FALSE;			// Length Error시 AAC 처리 KSK 2009.12.21
					break;
				}

				nIndex += 1 + 1 + 6;	// Tag(1Byte) + Len(1Byte) + Value(6)

				// KSK 2010.02.10 Length정보와 실제 Value가 맞지 않을 경우 Error 처리하도록 수정
				if (nIndex > nLen)
				{
					bTagValidCheck = FALSE;
					break;
				}
				// end of KSK 2010.02.10
				// end of KSK 2010.09.13
			}
		}
		else
			break;
	}

	nIsuScriptLen = nIssuScriptData1 + nIssuScriptData2;

	if (nIsuScriptLen > 522 || bTagValidCheck == FALSE)	// KSK 2009.12.21 Length Error시에 AAC 처리
	{
		// Error 처리함
		NHDEBUG(1, (_T("Parsing Error [%d] \n"), nIsuScriptLen));
		// Parsing Error는 "ud"를 안받은 것처럼 처리함

		if (m_bOnlineProcessing == FALSE)	// Online Processing을 한번만 호출하도록 Flag 사용
		{
			// KSK 2010.08.29 Invalid Tag인 경우 Default 처리 추가
			m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

			NHDEBUG(DBG_INFO, (L"Call [fnEMV_Trans_OnlineProcess]\n"));
			nResult = m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC);

			if (nResult == EMV_RSLT_OK)
			{
				m_pDevCmn->m_bDisplayDeclined = TRUE;	// [#2188] NH KSK 2013.05.22
				m_pDevCmn->fnEMV_Trans_Completion();	// 이러한 경우에는 항상 Kernel에서 DECLINED로 return함
			}

			// KSK 2010.09.03 CID가 안바뀔 경우 reason for reversal code "08"로 설정
			if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
			{
				int nLen = 0;
				CString strValue;
				unsigned char	szTemp[1024] = {0,};

				if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
				{
					strValue = MakeUnPack(szTemp, nLen);

					// CID가 ARQC (0x80)인 경우 Reason for Reversal을 "08"로 설정
					if (strValue == L"80")
					{
						if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)
							MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 8);
					}
				}
			}
			// end of KSK 2010.09.03

			BIZ_EMV_MakeICDataforReversal(nIsuScriptLen);

			m_bOnlineProcessing = TRUE;
		}
		return EMV_ERROR_ONLINE_DATA;	// Online Data Error로 표시
	}

	if (m_bOnlineProcessing == FALSE)
	{
		// SAVE ARC / ARPC / Issuer Script
		// Online Processing 이전에 ARC / ARPC / Issuer Script를 저장한다.
		if (nARCLen > 0)
			m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, nARCLen, (unsigned char*)szResponseCd);
		else	// KSK 2010.08.29 8A Tag를 받지 못한 경우
			m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_AuthRespCod, 2, (unsigned char*)m_szDefaultResponseCd);

		if (nARPCLen > 0)
			m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_IsuAuthData, nARPCLen, (unsigned char*)szARPCData);

		if (nIssuScriptData1 > 0)
			m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_IsuScriptTemp1, nIssuScriptData1, (unsigned char*)szIssuScriptData1);

		if (nIssuScriptData2 > 0)
			m_pDevCmn->fnEMV_Tlv_StoreVal(EMVTid_IsuScriptTemp2, nIssuScriptData2, (unsigned char*)szIssuScriptData2);

		//////////////////////////////////////////////////////////////////////////
		// AP에서 TC or AAC 처리를 해야한다.

		// [#2198] NH Justin 2013.05.30 Handling "85" response code for balance Inquiry
		BOOL bApproved = FALSE;
		BOOL bBI_85Response = FALSE;		
		// MASTER CARD EMV SPEC for "85" response (8A Tag) for Balance Inquiry 
		//	1) ATM should send AAC to the card reader
		//	2) ATM should handle the ramain transaction normally (Print balance) (CUSTOMER SIDE POINT OF VIEW).

		// [#2237] US Justin 2013 Dual Host EMV data Bug fix
		if( m_HostConfig == HC_DUALHOST ) 
		{
			if( (m_sSTD1_TranResp.ResponseCode_2 == L"00")||(m_sSTD1_TranResp.ResponseCode_2 == L"85") ) 
				bApproved = TRUE;
		}
		// [#2291] NH Justin 2014.08.27 Add Cardtronics TDL Option for Response "085"
		//#if (APP_TDL_OPTION)			// [#2302] US Justin 2014.11.05 Handle US BI 085
		else if( (TranCode==TC_INQUIRY)&&(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) && (m_sSTD3_TranResp.m_strResponseCode_3==L"085") )
				bApproved = TRUE;
		//#endif						// [#2302] US Justin 2014.11.05 Handle US BI 085
		// End of [#2291]
#if (APP_LIBERTYX)
		else if (m_HostConfig == HC_LIBERTYX)
		{
			if( (m_LXConfig.HostProtocol == LX_STANDARD3) && (m_sSTD3_TranResp.m_strResponseCode_3 == L"000")  )	
				bApproved = TRUE;
			else if( (m_LXConfig.HostProtocol == LX_STANDARD1) && ( (m_sSTD1_TranResp.ResponseCode_2 == L"00")||(m_sSTD1_TranResp.ResponseCode_2 == L"85") ) )
				bApproved = TRUE;
		}
#endif
		else
		{
			if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)  && (m_sSTD3_TranResp.m_strResponseCode_3 == L"000")  )	
				bApproved = TRUE;
			else if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE) && ( (m_sSTD1_TranResp.ResponseCode_2 == L"00")||(m_sSTD1_TranResp.ResponseCode_2 == L"85") ) )
				bApproved = TRUE;
		}
		// end of [#2237]

		if (bApproved)
		{
			if( (TranCode ==TC_INQUIRY)&&(szResponseCd[0]==0x38)&&(szResponseCd[1]==0x35) )			// Balance inquiry and "85" response
			{
				bBI_85Response = TRUE;
				nResult = m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC);								// AAC even the transaction is authorized....
				// [#2291] NH Justin 2014.09.23 Add Cardtronics TDL Option for Response "085"
				//#if (APP_TDL_OPTION)			// [#2302] US Justin 2014.11.05 Handle US BI 085
				if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) && (m_sSTD3_TranResp.m_strResponseCode_3==L"085") )
					m_sSTD3_TranResp.m_strResponseCode_3 = L"000";
				//#endif						// [#2302] US Justin 2014.11.05 Handle US BI 085
				// End of [#2291]
			}
			// [#2245] NH Justin 2013. 12.19 Issue AAC when 8A tag = "00" for Balance Inquiry. (Master Card Case)
			else if( (TranCode ==TC_INQUIRY)&&(szResponseCd[0]==0x30)&&(szResponseCd[1]==0x30) )			// Balance inquiry and "00" response
			{
				// [#2302] US Justin 2014.11.05 Handle US BI 085
				/*
				BOOL bIsAAC = TRUE;
				// [#2291] NH Justin 2014.09.23 Add Cardtronics TDL Option for Response "085"
				//#if (APP_TDL_OPTION)		
				//if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) && (m_sSTD3_TranResp.m_strResponseCode_3==L"085")  )
				//	m_sSTD3_TranResp.m_strResponseCode_3 = L"000";
				//#endif
				// Disabled 2014.09.23  AAC for both Master Card and VISA (Previous : AAC for Master Card Only)
				//CString sCoreAID = _T("");
				//CString sCurAID = MemGetStr(_MEM_FLD_EMVINFO, _MEM_VAR_EMV_APPLICATION_ID);
				//if(sCurAID.GetLength()>0)
				//{
				//	//int nLoc = sCurAID.Find(L"=");
				//	//if(nLoc>0)
				//	{
				//		//sCoreAID = sCurAID.Mid(nLoc+1);
				//		sCoreAID = sCurAID;		// [#2252] US Justin 2014.02.04 Long AID Bug Fix
				//		sCoreAID.TrimLeft();
				//		sCoreAID.TrimRight();
				//	}
				//}
				//if(sCoreAID.GetLength() >=10)
				//{
				//	CString sAID10 = sCoreAID.Left(10);
				//	if(sAID10.CompareNoCase(_T("A000000004")) == 0 )		// Only for Master Card Cases.... => Need to check......
				//		bIsAAC = TRUE;
				//}
				// End of [#2291]
				if(bIsAAC==TRUE)
				{
					bBI_85Response = TRUE;
					nResult = m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC);
				}
				else
					nResult = m_pDevCmn->fnEMV_Trans_OnlineProcess(1, TC);
				*/
				bBI_85Response = TRUE;
				nResult = m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC);
				if( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE)==MSG_TRITON_TYPE) && (m_sSTD3_TranResp.m_strResponseCode_3==L"085")  )
					m_sSTD3_TranResp.m_strResponseCode_3 = L"000";
				// End of [#2302]
			}
			// End of [#2245]
			else
				nResult = m_pDevCmn->fnEMV_Trans_OnlineProcess(1, TC);
		}	
		else
			nResult = m_pDevCmn->fnEMV_Trans_OnlineProcess(1, AAC);
		// End of [#2198]

		strTemp.Format(L"OnlineProc:%d", nResult);
		NVDump('O', 'C', "45", L"", strTemp );

		//////////////////////////////////////////////////////////////////////////
		NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_Anal_ICDataforTrans() fnEMV_Trans_OnlineProcess nResult[%d]\n"), nResult));


		// [#487] [NH] KSK 2009.1.20 위치를 이곳으로 이동함
		if( nResult != EMV_RSLT_OK )
		{
			nReturnCode = EMV_ERROR_DECLINED;
		}
		else /* nResult == RSLT_SUCCESS */
		{
			//////////////////////////////////////////////////////////////////////////
			nResult = m_pDevCmn->fnEMV_Trans_Completion();	// 이러한 경우에는 항상 Kernel에서 DECLINED로 return함

			NHDEBUG(1, (_T("***TranHostProc***CTranCmn::BIZ_EMV_Anal_ICDataforTrans() fnEMV_Trans_Completion nResult[%d]\n"), nResult));

			strTemp.Format(L"CompleteProc:%d", nResult);
			NVDump('O', 'C', "45", L"", strTemp );

			if (nResult == EMV_APPROVED)
				return EMV_TR_SUCCESS;	// [#508] [CA] KSK 2009.3.26
			else
			{
				// [#2198] NH Justin 2013.05.30 Handling "85" response code for balance Inquiry
				if(bBI_85Response)
					return EMV_TR_SUCCESS;
				// End of [#2198]
				nReturnCode = EMV_ERROR_DECLINED;
			}
		}

		// KSK 2010.09.03 CID가 안바뀔 경우 reason for reversal code "08"로 설정
		if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_HYOSUNG_TYPE)
		{
			int nLen = 0;
			CString strValue;
			unsigned char	szTemp[1024] = {0,};

			if (m_pDevCmn->fstrEMV_Tlv_GetVal(EMVTid_CryptInfData, &nLen, szTemp) == EMV_RSLT_OK)
			{
				strValue = MakeUnPack(szTemp, nLen);

				// CID가 ARQC (0x80)인 경우 Reason for Reversal을 "08"로 설정
				if (strValue == L"80")
				{
					if (MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL) <= 2)	// KSK 2010.09.03 Value 변경 방지를 위해 조건 추가
						MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_REASONFORREVERSAL, 8);
				}
			}
		}
		// end of KSK 2010.09.03

		BIZ_EMV_MakeICDataforReversal(nIsuScriptLen);

		m_bOnlineProcessing = TRUE;
	}

	return nReturnCode;
}

// [#2081] NH KSK 2011.06.27
int	CTranCmn::BIZ_EMV_SaveAIDList()
{
	CString strTemp;
	FILE	*Stream;
	DWORD	dwFileWriten = 0;
	char	buf[512] = { 0, };

	CString strRcvAID;		// [#2342] US Justin 2015.05.05 AID Selection
	char chAPVer[5] = {0,};	// [#2342] US Justin 2015.05.05 AID Selection

	// 기존의 TERM_DATA.ini File 삭제
	//strTemp.Format(L"%S", TERMDATA_INI_FILE);

	// [#2342] US Justin 2015.05.12		HOST AID => Update POOL file and enabled AIDs together.
	//strTemp.Format(L"%S", GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()) );
	strTemp.Format(L"%S", GET_EMV_AID_POOLFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()) );		
	CString strEnabledAIDList = _T("");
	// End of [#2342]

	DeleteFile(strTemp);

	// 새로운 TERM_DATA.ini File 생성
	// open file
	if ( (Stream = _tfopen(strTemp, _T("wb+"))) == NULL)
	{
		NHDEBUG(1, (_T("***BIZ_EMV_SaveAIDList*** File Open Fail[%s]\n"), strTemp));
		return -1;
	}

	// make section
	dwFileWriten = fwrite("[AIDCOUNT]\r\n", sizeof(char), strlen("[AIDCOUNT]\r\n"), Stream);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "TOTALCOUNT=%d\r\n\r\n", __min(m_Standard1_Host_AIDList.nTotalAIDCount, MAX_AIDLIST_CNT));
	dwFileWriten = fwrite(buf, sizeof(char), strlen(buf), Stream);

	for(int i=0; i<__min(m_Standard1_Host_AIDList.nTotalAIDCount, MAX_AIDLIST_CNT); i++)
	{
		// make section
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "[TERMINAL%d]\r\n", i+1);
		dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

		// make data - application name
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "APPLICATIONNAME=%s%d\r\n", UPDATE_APPLICATION_NAME, i+1);
		dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

		// make data - application index
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "APPLICATION=%02d\r\n", i);
		dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

		// make data - AID
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "AID=%s\r\n", m_Standard1_Host_AIDList.chAID[i]);
		dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
		// [#2342] US Justin 2015.05.12 Add to Enabled AID List.....
		strTemp.Format(L"%S", m_Standard1_Host_AIDList.chAID[i]);
		strEnabledAIDList += strTemp;
		strEnabledAIDList += AID_FIELD_DELIMITER;
		// End of [#2342]

		// make data - Terminal Len
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "TERMINALLEN=120\r\n");	// 현재 120개로 fix해서 사용하고 있음
		dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);

		// make data - Terminal Data
		memset(buf, 0, sizeof(buf));
		// [#2342] US Justin 2015.05.05 AID Selection
		//sprintf(buf, "TERMINALDATA=%s%10.10s%s%10.10s%s%10.10s%s\r\n\r\n", 
		//													TAG_TAC_DENIAL, m_Standard1_Host_AIDList.chTAC_Denial[i],
		//													TAG_TAC_ONLINE, m_Standard1_Host_AIDList.chTAC_Online[i],
		//													TAG_TAC_DEFAULT, m_Standard1_Host_AIDList.chTAC_Default[i],
		//													DEFAULT_TERMINAL_EMVDATA);
		if( m_pDevCmn->fnEMV_GetKernelVersion() == EMV_KERNEL_V4)		// Canada Only
		{
			strRcvAID.Format(L"%S", m_Standard1_Host_AIDList.chAID[i]);
			m_pDevCmn->Get_AID_AP_VersionNumber(strRcvAID, chAPVer, sizeof(chAPVer)); 
			sprintf(buf, "TERMINALDATA=%s%10.10s%s%10.10s%s%10.10s%s%4.4s%s\r\n\r\n", 
																TAG_TAC_DENIAL, m_Standard1_Host_AIDList.chTAC_Denial[i],
																TAG_TAC_ONLINE, m_Standard1_Host_AIDList.chTAC_Online[i],
																TAG_TAC_DEFAULT, m_Standard1_Host_AIDList.chTAC_Default[i],
																TAG_EMV_APVERSION, chAPVer,
																GET_EMV_DEFAULT_TERMINALDATA(m_pDevCmn->fnEMV_GetKernelVersion()));
		}
		else
		{
			sprintf(buf, "TERMINALDATA=%s%10.10s%s%10.10s%s%10.10s\r\n\r\n", 
																TAG_TAC_DENIAL, m_Standard1_Host_AIDList.chTAC_Denial[i],
																TAG_TAC_ONLINE, m_Standard1_Host_AIDList.chTAC_Online[i],
																TAG_TAC_DEFAULT, m_Standard1_Host_AIDList.chTAC_Default[i]);
		}
		// End of [#2342]
		dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
	}
	fflush(Stream);
	fclose(Stream);

	// [#2342] US Justin 2015.05.12 AID Enable...
	CString strCurEnabledAIDList = MemGetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID);
	if(strCurEnabledAIDList.CompareNoCase(strEnabledAIDList) != 0 )
	{
		MemSetStr(_MEM_FLD_EMVCONFIG, _MEM_VAR_EMV_CONFIG_ENABLEDAID, strEnabledAIDList);
		m_pDevCmn->Create_EMV_Termdata_From_POOL();
	}
	// End of [#2342]

	return T_OK;
}
// end of [#2081]

// [#2342] US Justin 2015.05.12 Move to CDevCmn
/*
// [#2226] US Justin 2013.10.23 Update AID List File
int	CTranCmn::BIZ_EMV_CreateAIDList_From_POOL()
{
	NHDEBUG(1, (_T("BIZ_EMV_CreateAIDList_From_POOL()\n")) );

	CString strTemp;
	FILE	*Stream;

	DWORD	dwFileWriten = 0;
	char	buf[512] = { 0, };
	char	bufContents[512] = { 0, };
	char	szIndex[16]	= { 0, };
	int		i;

	CString strAIDEnDisable = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EMV_AIDENDISABLE);

	// Open AID Pool
	CINIFile iniEMV(GET_EMV_AID_POOLFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()) );

	// Total number of listed AID
	iniEMV.GetPrivateProfileString("AIDCOUNT","TOTALCOUNT",buf, 512);
	strTemp.Format(L"%S",buf);
	int nListedTotalAIDList = Asc2Int(strTemp);

	// Delete Existing TERM_DATA File
	strTemp.Format(L"%S", GET_EMV_AID_WORKFILENAME(m_pDevCmn->fnEMV_GetKernelVersion()) );
	DeleteFile(strTemp);

	// Create new TERM_DATA File
	if ( (Stream = _tfopen(strTemp, _T("wb+"))) == NULL)
	{
		NHDEBUG(1, (_T("***BIZ_EMV_SaveAIDList*** File Open Fail[%s]\n"), strTemp));
		return -1;
	}

	// TOTAL COUNT 
	int nEnabledAID = 0;
	TCHAR chEnable;
	{
		// make section  
		dwFileWriten = fwrite("[AIDCOUNT]\r\n", sizeof(char), strlen("[AIDCOUNT]\r\n"), Stream);
		// Count Enabled AID
		for (i = 0; i < nListedTotalAIDList ; i++)
		{
			chEnable = strAIDEnDisable.GetAt(i);
			if( chEnable == '1')
				nEnabledAID ++;
		}
		if(nEnabledAID>MAX_AIDLIST_CNT)
			nEnabledAID = MAX_AIDLIST_CNT;
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "TOTALCOUNT=%d\r\n\r\n", nEnabledAID);
		dwFileWriten = fwrite(buf, sizeof(char), strlen(buf), Stream);
	}
	NHDEBUG(1, (_T("Total Enabled AID[%d]\n"), nEnabledAID));

	// Write Each AID inforamtion

	nEnabledAID = 0;
	for(i=0; i<__min(nListedTotalAIDList, MAX_AIDLIST_CNT); i++)
	{
		sprintf(szIndex,"TERMINAL%d",i+1);
		chEnable = strAIDEnDisable.GetAt(i);

		if( chEnable == '1')
		{
			// make section
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "[TERMINAL%d]\r\n", nEnabledAID+1);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
			NHDEBUG(1, (_T("\n\nRecoding : %S\n"), buf));

			// make data - application name
			memset(bufContents, 0, sizeof(bufContents));
			iniEMV.GetPrivateProfileString(szIndex,"APPLICATIONNAME",bufContents, 512); 
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "APPLICATIONNAME=%s\r\n", bufContents);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
			NHDEBUG(1, (_T("Recoding : %S\n"), buf));

			// make data - application index
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "APPLICATION=%02d\r\n", nEnabledAID);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
			NHDEBUG(1, (_T("Recoding : %S\n"), buf));

			// make data - AID
			memset(bufContents, 0, sizeof(bufContents));
			iniEMV.GetPrivateProfileString(szIndex,"AID",bufContents, 512); 
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "AID=%s\r\n", bufContents);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
			NHDEBUG(1, (_T("Recoding : %S\n"), buf));

			// make data - Terminal Len
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "TERMINALLEN=120\r\n");	// fixed 
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
			NHDEBUG(1, (_T("Recoding : %S\n"), buf));

			// make data - Terminal Data
			memset(bufContents, 0, sizeof(bufContents));
			iniEMV.GetPrivateProfileString(szIndex,"TERMINALDATA",bufContents, 512); 
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "TERMINALDATA=%s\r\n\r\n", bufContents);
			dwFileWriten = fwrite (buf, sizeof(char), strlen(buf), Stream);
			NHDEBUG(1, (_T("Recoding : %S\n"), buf));

			nEnabledAID++;
		}
	}

	fflush(Stream);
	fclose(Stream);
	return T_OK;
}
// End of [#2226] 
*/
// End of [#2342]