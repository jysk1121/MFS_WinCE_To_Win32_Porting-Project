#include "stdafx.h"
#include "SVC_Manager.h"
#include "EagleSVCLib.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/IniFile.h"

#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/** *************************************************************
*	@brief		Option Main
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_Main()
{   

	BOOL bShowScreen = TRUE;
	BOOL bKeyEnable = TRUE;
	CString strButtonInform;
	CString strSource, strDestination;
	
	while(TRUE)
	{
 
		CheckService();  //EPP TIME에서 Start Entry 명령 발행하기 위해 호출

		if(bShowScreen == TRUE)
		{ 
			m_strScrOutName.Empty();			
			m_strScrOutData.Empty();

            Operator_DisplayScreen(OP_SCREEN_OPTION, bKeyEnable);
            bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}
  
        //입력 Key 확인 
		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_Main - User Key Input : %s"), m_strScrOutData);

			if ( m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER )
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if ( m_strScrOutData == _T("TRANSACTION") )
			{
				m_strNextScrNum = OP_SCREEN_OPTION_TRANSACTION;
				break;
			}

			else if ( m_strScrOutData == _T("AD_SCREEN") )
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}	

			else if ( m_strScrOutData == _T("MESSAGE") ) 
			{
				m_strNextScrNum = OP_SCREEN_OPTION_MESSAGE;
				break;
			}
			else 
			{
				break;
			}

		}

		Sleep(EAGLE_SLEEP_INTERVAL);

	}

}


/** *************************************************************
*	@brief		Option Transaction
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_Transaction()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strSurchargeType, strSurchargeOwner, strSurchargeAmount, strSurchargeMethod, strSurchargePercentage;
	int		nMaxFocusIndex = 5;
	int		nTempFocusIndex = 0;

	m_nCurrentFocusIndex=1;

	strSurchargeType = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type;
	strSurchargeOwner = CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner;
	strSurchargeOwner.TrimLeft();
	strSurchargeOwner.TrimRight();
	strSurchargeAmount.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount));
	strSurchargeMethod = CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option;
	strSurchargePercentage.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage));

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();			
            
			// 1.Surcharge Type
			strScreenInform = GET_SURCHARGETYPE_STRING(strSurchargeType);
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Surcharge Owner
			strScreenInform += strSurchargeOwner;
			strScreenInform += SCR_CMD_DELIMITER;	
            
			// 3.Surcharge Fixed Amount
			strTemp.Format(_T("%s%s"), CURRENCY_SYMBOL, CUtil::ConvertFromValueToAmountwithCent(strSurchargeAmount));
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 4. Percentage Method
			strScreenInform += GET_PERCENTAGEMETHOD_STRING(strSurchargeMethod);
			strScreenInform += SCR_CMD_DELIMITER;

			// 5.Surcharge Percentage 
			strTemp.Format(_T("%s %%"), CUtil::ConvertFromValueToAmountwithCent(strSurchargePercentage));
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

		    Operator_DisplayScreen(OP_SCREEN_OPTION_TRANSACTION, bKeyEnable, strScreenInform);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_Transaction - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));

				if( (3 == nTempFocusIndex) && (_T("1") == strSurchargeType) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}
				else if( ((4 == nTempFocusIndex) || (5 == nTempFocusIndex)) &&  (_T("0") == strSurchargeType) )
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{   
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				if (strSurchargeType == _T("0"))	// amount base
				{
					// 가능한 index는 1,2, 3임
					if (m_nCurrentFocusIndex > 2)
						m_nCurrentFocusIndex = 3;
				}
				else
				{
					// 가능한 index는 1,4,5임
					if (m_nCurrentFocusIndex == 3)
						m_nCurrentFocusIndex--;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				if (strSurchargeType == _T("0"))
				{
					if (m_nCurrentFocusIndex > 3)
						m_nCurrentFocusIndex = 1;
				}
				else
				{
					if (m_nCurrentFocusIndex == 3)
						m_nCurrentFocusIndex++;
				}

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
					case 1:						
						{   
							if (strSurchargeType == _T("0"))
								strSurchargeType = _T("1");
							else
								strSurchargeType = _T("0");
														
							bShowScreen=TRUE;
         				}
						break;

					case 2:
						{
							if(Operator_Input_Text(_T("TERMINAL OWNER"),strSurchargeOwner,m_strScrOutData, 15, UPPERCASE_MODE) == TRUE)
							{   
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();

								strSurchargeOwner = m_strScrOutData;
							}

							bShowScreen = TRUE;
						}
						break;
						
					case 3:
						{
							CString strGuideText;
							strGuideText.Format(_T("%s %s"), OP_GUIDE_MAX_INPUT_VAL, _T("99.99"));

							if(Operator_Input_Number(_T("AMOUNT"),strSurchargeAmount,m_strScrOutData, 4, CENT_TYPE, strGuideText) == TRUE)
							{   
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();

								int nAmount = CUtil::StringToInt(m_strScrOutData);

								if ((nAmount >= 0) && (nAmount < 10000))
								{
									strSurchargeAmount.Format(_T("%d"), nAmount);
								}
								else
								{
									LOG(Error, _T("Abnormal case : type : %s, amount : %d"), strSurchargeType, nAmount);
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
							}
						
							bShowScreen = TRUE;

						}
						break;

					case 4:
						{
							if (strSurchargeMethod == _T("0"))
								strSurchargeMethod = _T("1");
							else
								strSurchargeMethod = _T("0");

							bShowScreen=TRUE;
						}
						break;

					case 5:
						{
							CString strGuideText;
							strGuideText.Format(_T("%s %s"), OP_GUIDE_MAX_INPUT_VAL, _T("99.99"));

							if(Operator_Input_Number(_T("PERCENTAGE"), strSurchargePercentage, m_strScrOutData, 4, PERCENT_TYPE) == TRUE)
							{
								m_strScrOutData.TrimLeft();
								m_strScrOutData.TrimRight();

								if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
								{
									int nPercentage = CUtil::StringToInt(m_strScrOutData);

									if (nPercentage > 0 && nPercentage < 10000)
									{
										strSurchargePercentage.Format(_T("%d"), nPercentage); 
									}
									else 
									{
										LOG(Error, _T("Abnormal case : type : %s, Percentage : %d"), strSurchargeType, nPercentage);
										Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
										CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
									}
								}
								else
								{
									LOG(Error, _T("Error Case - Return Value : %s"), m_strScrOutData);
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
							}
							
							bShowScreen = TRUE;
						}
						break;

					default :
						break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type = strSurchargeType;
				CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner = strSurchargeOwner;

				strSurchargeAmount.Format(_T("%03d"), CUtil::StringToInt(strSurchargeAmount));
				//if (CUtil::StringToInt(strSurchargeAmount) <= 0)
				//	strSurchargeAmount = _T("000");

				CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount = strSurchargeAmount;

				CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option = strSurchargeMethod;
				CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage = strSurchargePercentage;

				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("surch_type"), CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_type); 
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("surch_owner"), CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_owner);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("withdrawal_surch_amt"), CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawalSurcharge_amount);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("surch_lesser_greater"), CEagleDataManager::GetInstance()->m_Config.m_Option.strWithdrawal_PercentSurcharge_Option);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("surch_percent"), CEagleDataManager::GetInstance()->m_Config.m_Option.strSurcharge_percentage);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex=1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("AMOUNTSETTINGS"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AMOUNT_SETTINGS;
				break;
			}
			else if (m_strScrOutData == _T("CONFIGUREGENERAL"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_CONFIGURE_GENERAL;
				break;
			}
			else if (m_strScrOutData == _T("BINLIST"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_BLOCK_ISO;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		Amount Settings
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AmountSettings()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strFastCash[6], strMaxWithdrawalAmount;
	int		nMaxFocusIndex = 7;
	int		i = 0;

	m_nCurrentFocusIndex=1;

	for(i=0; i<6; i++)
	{
		strFastCash[i].Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i]));
	}
	strMaxWithdrawalAmount.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount));

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();			

			// 1.FastCash 1 ~ 6
			for(i=0; i<6; i++)
			{
				strTemp.Format(_T("%s%s"), CURRENCY_SYMBOL, strFastCash[i]);
				strScreenInform += strTemp;
				strScreenInform += SCR_CMD_DELIMITER;
			}

			// 7. Max Withdrawal Amount
			strTemp.Format(_T("%s%s"), CURRENCY_SYMBOL, strMaxWithdrawalAmount);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_OPTION_AMOUNT_SETTINGS, bKeyEnable, strScreenInform);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AmountSettings - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_TRANSACTION;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{   
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
				case 1:
				case 2:
				case 3:
					{
						strTemp.Format(_T("FDK_LEFT_%d"), m_nCurrentFocusIndex);

						if(Operator_Input_Number(strTemp, strFastCash[m_nCurrentFocusIndex-1], m_strScrOutData, 3, DOLLAR_TYPE) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
							{
								int nInputAmount = CUtil::StringToInt(m_strScrOutData);

								if (nInputAmount > CUtil::StringToInt(strMaxWithdrawalAmount))
								{
									// Max값보다 높은 값 입력시 Invalid 처리
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strFastCash[m_nCurrentFocusIndex-1].Format(_T("%d"), nInputAmount);
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 4:
				case 5:
				case 6:
					{
						strTemp.Format(_T("FDK_RIGHT_%d"), (m_nCurrentFocusIndex-3));

						if(Operator_Input_Number(strTemp, strFastCash[m_nCurrentFocusIndex-1], m_strScrOutData, 3, DOLLAR_TYPE) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
							{
								int nInputAmount = CUtil::StringToInt(m_strScrOutData);

								if (nInputAmount > CUtil::StringToInt(strMaxWithdrawalAmount))
								{
									// Max값보다 높은 값 입력시 Invalid 처리
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strFastCash[m_nCurrentFocusIndex-1].Format(_T("%d"), nInputAmount);
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 7:
					{
						if(Operator_Input_Number(_T("AMOUNT"), strMaxWithdrawalAmount, m_strScrOutData, 3, DOLLAR_TYPE) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
							{
								int nInputAmount = CUtil::StringToInt(m_strScrOutData);
								strMaxWithdrawalAmount.Format(_T("%d"), nInputAmount);
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;						
					}
					break;

				default :
					break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				for(i=0; i<6; i++)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i] = strFastCash[i];
					strTemp.Format(_T("fast_amt_%d"), (i+1));
					CEagleDataManager::GetInstance()->m_Config.SaveOptionData(strTemp, CEagleDataManager::GetInstance()->m_Config.m_Option.strFast_amount[i]);

				}

				CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount = strMaxWithdrawalAmount;
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("max_withdrawal"), CEagleDataManager::GetInstance()->m_Config.m_Option.strMax_withdrawal_amount);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		Configure General
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_ConfigureGeneral()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strLanguageSupport[3], strBalanceEnable, strTransferEnable, strSavingsEnable, strCreditEnable;
	int		nMaxFocusIndex = 7;
	int		i = 0;
	int		nTempFocusIndex = 0;

	m_nCurrentFocusIndex=2;

	for(i=0; i<3; i++)
		strLanguageSupport[i] = CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[i];

	strBalanceEnable = CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable;
	strTransferEnable = CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable;

	strSavingsEnable = CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account;
	strCreditEnable = CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();			

			// 1. Language (1 ~ 3)
			for(i=0; i<3; i++)
			{
				strScreenInform += GET_ENDISABLE_STRING(strLanguageSupport[i]);
				strScreenInform += SCR_CMD_DELIMITER;
			}

			// 4. Function
			strScreenInform += GET_ENDISABLE_STRING(strBalanceEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strTransferEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			// 6. Account
			strScreenInform += GET_ENDISABLE_STRING(strSavingsEnable);
			strScreenInform += SCR_CMD_DELIMITER;

			strScreenInform += GET_ENDISABLE_STRING(strCreditEnable);
			strScreenInform += SCR_CMD_DELIMITER;
			
			Operator_DisplayScreen(OP_SCREEN_OPTION_CONFIGURE_GENERAL, bKeyEnable, strScreenInform);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_ConfigureGeneral - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_TRANSACTION;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				nTempFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));

				if(1 == nTempFocusIndex)	// ENGLISH는 설정 불가능
				{
					Operator_Processing_Screen(OP_GUIDE_TOUCH_INVALID);
					CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

					bShowScreen = TRUE;
					continue;
				}

				m_nCurrentFocusIndex = nTempFocusIndex;
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{   
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 2)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 2;		// ENGLISH는 설정 불가능

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
				case 2:
				case 3:
					{
						if (strLanguageSupport[m_nCurrentFocusIndex-1] == S_ENABLE)
							strLanguageSupport[m_nCurrentFocusIndex-1] = S_DISABLE;
						else
							strLanguageSupport[m_nCurrentFocusIndex-1] = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 4:
					{
						if (strBalanceEnable == S_ENABLE)
							strBalanceEnable = S_DISABLE;
						else
							strBalanceEnable = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 5:
					{
						if (strTransferEnable == S_ENABLE)
							strTransferEnable = S_DISABLE;
						else
							strTransferEnable = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;

				case 6:
					{
						if (strSavingsEnable == S_ENABLE)
							strSavingsEnable = S_DISABLE;
						else
							strSavingsEnable = S_ENABLE;			

						bShowScreen = TRUE;
					}
					break;

				case 7:
					{
						if (strCreditEnable == S_ENABLE)
							strCreditEnable = S_DISABLE;
						else
							strCreditEnable = S_ENABLE;					

						bShowScreen = TRUE;						
					}
					break;

				default :
					break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				for(i=1; i<3; i++)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[i] = strLanguageSupport[i];
				}

				CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable = strBalanceEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable = strTransferEnable;

				CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account = strSavingsEnable;
				CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account = strCreditEnable;

				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("Spanish"), CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[1]);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("French"), CEagleDataManager::GetInstance()->m_Config.m_Option.strLansupport[2]);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("balance_en"), CEagleDataManager::GetInstance()->m_Config.m_Option.strbalance_Enable);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("transfer_en"), CEagleDataManager::GetInstance()->m_Config.m_Option.strtransfer_Enable);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("saving_en"), CEagleDataManager::GetInstance()->m_Config.m_Option.strSaving_account);
				CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("credit_en"), CEagleDataManager::GetInstance()->m_Config.m_Option.strCredit_account);

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 2;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		BIN LIST
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_BlockISO()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp, strBinNumber[MAX_BIN_COUNT];
	CString strListIndex;

	int		nMaxFocusIndex = 22;
	int		i = 0, nCurrentPage = 1, nTotalPage = 10, nTotalCount = 0;
	int		nPageCount = 10;
	int		nIsoIndex = 0;

	m_nCurrentFocusIndex = 13;

	// Check Total Count
	nTotalCount = CEagleDataManager::GetInstance()->m_Config.m_Iso.nTotalCount;

	// Load ISO
	for(i=0; i<MAX_BIN_COUNT; i++)
	{
		strBinNumber[i] = CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i];
		strBinNumber[i].TrimLeft();
		strBinNumber[i].TrimRight();
	}

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();	
			strButtonInform.Empty();

			// 1. Total Count
			strTemp.Format(_T("%d"), nTotalCount);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 2. Page Information
			strTemp.Format(_T("%d / %d"), nCurrentPage, nTotalPage);
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// 3 ~ 12 (Numbering)
			for(i=1; i<(nPageCount+1); i++)
			{
				strTemp.Format(_T("%d"), ((nCurrentPage-1)*nPageCount) + i);
				strScreenInform += strTemp;
				strScreenInform += SCR_CMD_DELIMITER;
			}
			
			// 13 ~ 22 BIN NUMBER
			for(i=0; i<nPageCount; i++)
			{
				strScreenInform += strBinNumber[((nCurrentPage-1)*nPageCount) + i];
				strScreenInform += SCR_CMD_DELIMITER;
			}

			// Button
			if (nCurrentPage <= 1)
				strButtonInform += _T("off");
			else
				strButtonInform += _T("on");
			strButtonInform += SCR_CMD_DELIMITER;

			if (nCurrentPage >= nTotalPage)
				strButtonInform += _T("off");
			else
				strButtonInform += _T("on");
			strButtonInform += SCR_CMD_DELIMITER;


			Operator_DisplayScreen(OP_SCREEN_OPTION_BLOCK_ISO, bKeyEnable, strScreenInform, strButtonInform);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_BlockISO - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_TRANSACTION;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(2));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREVPAGE"))
			{
				nCurrentPage--;

				if (nCurrentPage < 1)
					nCurrentPage = 1;

				// Reload BinCount
// 				for(i=0; i<nPageCount; i++)
// 				{
// 					strBinNumber[i] = CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[((nCurrentPage-1)*nPageCount) + i];
// 					strBinNumber[i].TrimLeft();
// 					strBinNumber[i].TrimRight();
// 				}

				m_nCurrentFocusIndex = 13; // 초기화
				bShowScreen = TRUE;

			}
			else if (m_strScrOutData == _T("NEXTPAGE"))
			{
				nCurrentPage++;

				if (nCurrentPage >= 10)
					nCurrentPage = 10;

				// Reload BinCount
// 				for(i=0; i<nPageCount; i++)
// 				{
// 					strBinNumber[i] = CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[((nCurrentPage-1)*nPageCount) + i];
// 					strBinNumber[i].TrimLeft();
// 					strBinNumber[i].TrimRight();
// 				}

				m_nCurrentFocusIndex = 13; // 초기화
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("DELETEALL"))
			{
				if(Operator_Confirm_Screen(_T("DELETE ALL")) == TRUE)
				{
					// 초기화
					nCurrentPage = 1;
					m_nCurrentFocusIndex = 13;
				//	nTotalCount = 0;	// Set 버튼을 눌렀을 때 totalcount 적용

					for(i=0; i<MAX_BIN_COUNT; i++)
					{
						strBinNumber[i].Empty();
					}
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("DELETE"))
			{
				strBinNumber[((nCurrentPage-1)*nPageCount) + (m_nCurrentFocusIndex-13)].Empty();
			//	nTotalCount--;	// Set 버튼을 눌렀을 때 totalcount 적용

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 13)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 13;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				nIsoIndex = ((nCurrentPage-1)*nPageCount) + (m_nCurrentFocusIndex-13);
				if (Operator_Input_Number(_T("BIN NUMBER"), strBinNumber[nIsoIndex], m_strScrOutData, 10) == TRUE)
				{
					m_strScrOutData.TrimLeft();
					m_strScrOutData.TrimRight();

					// Delete도 가능하도록 함
					if ((m_strScrOutData.IsEmpty() == TRUE) || (CUtil::IsNumeric(m_strScrOutData) == TRUE))
					{
						strBinNumber[nIsoIndex] = m_strScrOutData;
					}
					else
					{
						Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
						CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
					}
				}

				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("SET"))
			{
				Operator_Processing_Screen(OP_GUIDE_COMMON_WAIT);

// 				for(i=0; i<nPageCount; i++)
// 				{
// 					CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[((nCurrentPage-1)*nPageCount) + i] = strBinNumber[((nCurrentPage-1)*nPageCount) + i];
// 					LOG(Info, _T("SET New Bin Number - Index : %d, Value : %s"), ((nCurrentPage-1)*nPageCount) + i, strBinNumber[((nCurrentPage-1)*nPageCount) + i]);
// 				}
				for(i=0; i<MAX_BIN_COUNT; i++)
				{
					if(CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i] != strBinNumber[i])
					{
						LOG(Info, _T("SET New Bin Number - Index : %d, Old Value : %s -> New Value : %s"), i, CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i], strBinNumber[i]);
						CEagleDataManager::GetInstance()->m_Config.m_Iso.strBLOCK_ISO[i] = strBinNumber[i];
					}
				}

				CEagleDataManager::GetInstance()->m_Config.SaveIso();
				CEagleDataManager::GetInstance()->m_Config.LoadIso();

				nTotalCount = CEagleDataManager::GetInstance()->m_Config.m_Iso.nTotalCount;

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				// 현재 설정된 BIN LIST 위치 유지
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		AD Screen
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AD_Screen()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strEditStateInfo;
	CString strTemp, strInterval;
	CString strAD_Enable[6];
	CString strADTitleFileName, strADTitle;
	int		nMaxFocusIndex = 7;
	int		i=0;

	m_nCurrentFocusIndex = 1;

	strInterval.Format(_T("%d"), CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Interval_time));

	for(i=0; i<6; i++)
	{
		strAD_Enable[i] = CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i];
	}

	// AD File Read
	strADTitleFileName.Format(_T("%s%s\\%d_%d%s"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY ,AD_TITLE_FILE);

	CIniFile iniFile(strADTitleFileName);

	strADTitle = iniFile.ReadString(_T("AD_INFO"), _T("AD_TITLE"), _T(""));

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			strButtonInform.Empty();
			strEditStateInfo.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// APValue1
			strTemp.Format(_T("%d SEC"), CUtil::StringToInt(strInterval));
			strScreenInform += strTemp;
			strScreenInform += SCR_CMD_DELIMITER;

			// APValue2~7
			for(i=0; i<6; i++)
			{
				strScreenInform += GET_ENDISABLE_STRING(strAD_Enable[i]);
				strScreenInform += SCR_CMD_DELIMITER;

				strTemp.Format(_T("%s%s\\%d_%d\\AD%d.jpg"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY, i+1);

				if (CUtil::IsExistFile(strTemp) == TRUE)
					strEditStateInfo += _T("ENABLE");
				else
					strEditStateInfo += _T("DISABLE");

				strEditStateInfo += SCR_CMD_DELIMITER;
			}

			// APValue8 - ADV Title
			strScreenInform += strADTitle;
			strScreenInform += SCR_CMD_DELIMITER;

			Operator_DisplayScreen(OP_SCREEN_OPTION_AD_SCREEN, bKeyEnable, strScreenInform, strButtonInform, strEditStateInfo);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AD_Screen - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
				case 1:
					{
						CString strGuideText;
						strGuideText.Format(_T("%s %d"), OP_GUIDE_MAX_INPUT_VAL, 60);

						if (Operator_Input_Number(_T("INTERVAL"), strInterval, m_strScrOutData, 2, NORMAL_TYPE, strGuideText) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();

							if (CUtil::IsNumeric(m_strScrOutData) == TRUE)
							{
								int nValue = CUtil::StringToInt(m_strScrOutData);

								if (nValue <5 || nValue > 60)
								{
									Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
									CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
								}
								else
								{
									strInterval.Format(_T("%d"), nValue);
								}
							}
							else
							{
								Operator_Processing_Screen(OP_GUIDE_COMMON_INVALID);
								CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);
							}
						}

						bShowScreen = TRUE;
					}
					break;

				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					{
						if (strAD_Enable[m_nCurrentFocusIndex-2] == S_ENABLE)
							strAD_Enable[m_nCurrentFocusIndex-2] = S_DISABLE;
						else
							strAD_Enable[m_nCurrentFocusIndex-2] = S_ENABLE;

						bShowScreen = TRUE;
					}
					break;
				
				default:
					break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Interval_time = strInterval;

				for(i=0; i<6; i++)
				{
					CEagleDataManager::GetInstance()->m_Config.m_ADS.strAdver_Screen_en[i] = strAD_Enable[i];
				}

				CEagleDataManager::GetInstance()->m_Config.SaveAds();

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else if (m_strScrOutData == _T("PREVIEW"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_1_PREVIEW;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		AD1 Preview
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AD1_Preview()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			strButtonInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// Find AD1 Files
			strTemp.Format(_T("%s%s\\%d_%d\\AD%d.jpg"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY, 1);
			if (CUtil::IsExistFile(strTemp) == TRUE)
			{
				LOG(Info, _T("AD1 File found"));
				strButtonInform = _T("off");
			}
			else
			{
				LOG(Info, _T("AD1 File not found"));
				strButtonInform = _T("on");
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_AD_1_PREVIEW, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AD1_Preview - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_strNextScrNum = OP_SCREEN_OPTION_AD_2_PREVIEW;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		AD2 Preview
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AD2_Preview()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			strButtonInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// Find AD1 Files
			strTemp.Format(_T("%s%s\\%d_%d\\AD%d.jpg"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY, 2);
			if (CUtil::IsExistFile(strTemp) == TRUE)
			{
				LOG(Info, _T("AD2 File found"));
				strButtonInform = _T("off");
			}
			else
			{
				LOG(Info, _T("AD2 File not found"));
				strButtonInform = _T("on");
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_AD_2_PREVIEW, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AD2_Preview - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_1_PREVIEW;
				break;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_strNextScrNum = OP_SCREEN_OPTION_AD_3_PREVIEW;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		AD3 Preview
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AD3_Preview()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			strButtonInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// Find AD1 Files
			strTemp.Format(_T("%s%s\\%d_%d\\AD%d.jpg"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY, 3);
			if (CUtil::IsExistFile(strTemp) == TRUE)
			{
				LOG(Info, _T("AD3 File found"));
				strButtonInform = _T("off");
			}
			else
			{
				LOG(Info, _T("AD3 File not found"));
				strButtonInform = _T("on");
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_AD_3_PREVIEW, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AD3_Preview - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_2_PREVIEW;
				break;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_strNextScrNum = OP_SCREEN_OPTION_AD_4_PREVIEW;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		AD4 Preview
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AD4_Preview()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			strButtonInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// Find AD1 Files
			strTemp.Format(_T("%s%s\\%d_%d\\AD%d.jpg"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY, 4);
			if (CUtil::IsExistFile(strTemp) == TRUE)
			{
				LOG(Info, _T("AD4 File found"));
				strButtonInform = _T("off");
			}
			else
			{
				LOG(Info, _T("AD4 File not found"));
				strButtonInform = _T("on");
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_AD_4_PREVIEW, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AD4_Preview - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_3_PREVIEW;
				break;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_strNextScrNum = OP_SCREEN_OPTION_AD_5_PREVIEW;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		AD5 Preview
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AD5_Preview()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			strButtonInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// Find AD1 Files
			strTemp.Format(_T("%s%s\\%d_%d\\AD%d.jpg"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY, 5);
			if (CUtil::IsExistFile(strTemp) == TRUE)
			{
				LOG(Info, _T("AD5 File found"));
				strButtonInform = _T("off");
			}
			else
			{
				LOG(Info, _T("AD5 File not found"));
				strButtonInform = _T("on");
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_AD_5_PREVIEW, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AD5_Preview - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_4_PREVIEW;
				break;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_strNextScrNum = OP_SCREEN_OPTION_AD_6_PREVIEW;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		AD6 Preview
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AD6_Preview()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strTemp;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			strButtonInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			// Find AD1 Files
			strTemp.Format(_T("%s%s\\%d_%d\\AD%d.jpg"), CUtil::GetAppPath(), EAGLE_ATM_AD_PATH, EAGLE_SCREEN_CX, EAGLE_SCREEN_CY, 6);
			if (CUtil::IsExistFile(strTemp) == TRUE)
			{
				LOG(Info, _T("AD6 File found"));
				strButtonInform = _T("off");
			}
			else
			{
				LOG(Info, _T("AD6 File not found"));
				strButtonInform = _T("on");
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_AD_6_PREVIEW, bKeyEnable, strScreenInform, strButtonInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AD6_Preview - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_EXIT || m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_AD_5_PREVIEW;
				break;
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_strNextScrNum = OP_SCREEN_OPTION_AD_SCREEN;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}



/** *************************************************************
*	@brief		Message
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_Message()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();			

			Operator_DisplayScreen(OP_SCREEN_OPTION_MESSAGE, bKeyEnable);	
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_Message - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION;	
				break;
			}

			// Next Action
			if (m_strScrOutData == _T("MARKETING"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_MARKETING;
				break;
			}
			else if (m_strScrOutData == _T("LOCATION"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_LOCATION;
				break;
			}
			else if (m_strScrOutData == _T("ATTRACT"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_ATTRACT;
				break;
			}
			else if (m_strScrOutData == _T("FAREWELL"))
			{
				m_strNextScrNum = OP_SCREEN_OPTION_FAREWELL;
				break;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		Marketing Message
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_MarketingMsg()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strEditStateInfo;
	CString strTemp, strMarketingMsg[4];
	int		nMaxFocusIndex = 4;
	int		i=0;

	m_nCurrentFocusIndex = 1;

	for(i=0; i<4; i++)
	{
		strMarketingMsg[i] = CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i];
		strMarketingMsg[i].TrimRight();	// 왼쪽 정렬이므로 우측만 Trim처리
	}

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			for(i=0; i<4; i++)
			{
				strScreenInform += strMarketingMsg[i];
				strScreenInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_MARKETING, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_MarketingMsg - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_MESSAGE;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
				case 1:
				case 2:
				case 3:
				case 4:
					{
						strTemp.Format(_T("LINE %d"), m_nCurrentFocusIndex);
						if (Operator_Input_Text(strTemp, strMarketingMsg[m_nCurrentFocusIndex-1], m_strScrOutData, 40, UPPERCASE_MODE) == TRUE)
						{
							m_strScrOutData.TrimRight();	// 우측만 Trim 처리
							strMarketingMsg[m_nCurrentFocusIndex-1] = m_strScrOutData;
						}

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				for(i=0; i<4; i++)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Message.strMarketing_message[i] = strMarketingMsg[i];
				}

				CEagleDataManager::GetInstance()->m_Config.SaveMessage();

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		Location Message
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_LocationMsg()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strEditStateInfo;
	CString strTemp, strLocationMsg[4];
	int		nMaxFocusIndex = 4;
	int		i=0;

	m_nCurrentFocusIndex = 1;

	for(i=0; i<4; i++)
	{
		strLocationMsg[i] = CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i];
		strLocationMsg[i].TrimRight();	// 왼쪽 정렬이므로 우측만 Trim처리
	}

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			for(i=0; i<4; i++)
			{
				strScreenInform += strLocationMsg[i];
				strScreenInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_LOCATION, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_LocationMsg - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_MESSAGE;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
				case 1:
				case 2:
				case 3:
				case 4:
					{
						strTemp.Format(_T("LINE %d"), m_nCurrentFocusIndex);
						if (Operator_Input_Text(strTemp, strLocationMsg[m_nCurrentFocusIndex-1], m_strScrOutData, 40, UPPERCASE_MODE) == TRUE)
						{
							m_strScrOutData.TrimRight();	// 우측만 Trim 처리
							strLocationMsg[m_nCurrentFocusIndex-1] = m_strScrOutData;
						}

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				for(i=0; i<4; i++)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Message.strLocation_message[i] = strLocationMsg[i];
				}

				CEagleDataManager::GetInstance()->m_Config.SaveMessage();

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		Attract Message
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_AttractMsg()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strEditStateInfo;
	CString strTemp, strAttractMsg[3];
	int		nMaxFocusIndex = 3;
	int		i=0;

	m_nCurrentFocusIndex = 1;

	for(i=0; i<3; i++)
	{
		strAttractMsg[i] = CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i];
		strAttractMsg[i].TrimLeft();
		strAttractMsg[i].TrimRight();
	}

	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			for(i=0; i<3; i++)
			{
				strScreenInform += strAttractMsg[i];
				strScreenInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_ATTRACT, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_AttractMsg - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_MESSAGE;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
				case 1:
				case 2:
				case 3:
				case 4:
					{
						strTemp.Format(_T("LINE %d"), m_nCurrentFocusIndex);
						if (Operator_Input_Text(strTemp, strAttractMsg[m_nCurrentFocusIndex-1], m_strScrOutData, 30, UPPERCASE_MODE) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();
							strAttractMsg[m_nCurrentFocusIndex-1] = m_strScrOutData;
						}

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				for(i=0; i<3; i++)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Message.strAttraction_message[i] = strAttractMsg[i];
				}

				CEagleDataManager::GetInstance()->m_Config.SaveMessage();

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}


/** *************************************************************
*	@brief		Farewell Message
*	@retval		없음
*****************************************************************/
void CSVC_Manager::Proc_Operator_Option_FarewellMsg()
{	
	BOOL	bShowScreen = TRUE;
	BOOL	bKeyEnable = TRUE;
	CString strScreenInform;
	CString strButtonInform;
	CString strEditStateInfo;
	CString strTemp, strFarewellMsg[3];
	int		nMaxFocusIndex = 3;
	int		i=0;

	m_nCurrentFocusIndex = 1;

	for(i=0; i<3; i++)
	{
		strFarewellMsg[i] = CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i];
		strFarewellMsg[i].TrimLeft();
		strFarewellMsg[i].TrimRight();
	}


	while(TRUE) 
	{ 

		CheckService();

		if (bShowScreen == TRUE)
		{  
			strScreenInform.Empty();
			m_strScrOutName.Empty();
			m_strScrOutData.Empty();

			for(i=0; i<3; i++)
			{
				strScreenInform += strFarewellMsg[i];
				strScreenInform += SCR_CMD_DELIMITER;
			}

			Operator_DisplayScreen(OP_SCREEN_OPTION_FAREWELL, bKeyEnable, strScreenInform);
			bShowScreen = FALSE;
			bKeyEnable = FALSE;
		}

		if (CSCR_Manager::GetInstance()->GetUserInputCheck() == TRUE)
		{
			CSCR_Manager::GetInstance()->GetUserInputData(m_strScrOutName, m_strScrOutData);

			LOG(Info, _T("Proc_Operator_Option_FarewellMsg - User Key Input : %s"), m_strScrOutData);

			if (m_strScrOutData == S_TIMEOVER)
			{
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}

			else if (m_strScrOutData == S_EXIT)
			{
				m_strNextScrNum = OP_SCREEN_OPTION_MESSAGE;	
				break;
			}

			// Touch로 Focus를 주기 위한 로직
			if (m_strSelectTouch.CompareNoCase(m_strScrOutData.Left(m_strSelectTouch.GetLength())) == 0)
			{
				m_nCurrentFocusIndex = CUtil::StringToInt(m_strScrOutData.Right(1));
				m_strScrOutData = _T("SELECT");
			}
			///////////////////////////////////////////////////

			// Next Action
			if (m_strScrOutData == _T("PREV"))
			{
				m_nCurrentFocusIndex--;

				if (m_nCurrentFocusIndex < 1)
					m_nCurrentFocusIndex = nMaxFocusIndex;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("NEXT"))
			{ 
				m_nCurrentFocusIndex++;

				if (m_nCurrentFocusIndex > nMaxFocusIndex)
					m_nCurrentFocusIndex = 1;

				Operator_UpdateScreen();
			}
			else if (m_strScrOutData == _T("SELECT"))
			{
				switch(m_nCurrentFocusIndex)
				{
				case 1:
				case 2:
				case 3:
				case 4:
					{
						strTemp.Format(_T("LINE %d"), m_nCurrentFocusIndex);
						if (Operator_Input_Text(strTemp, strFarewellMsg[m_nCurrentFocusIndex-1], m_strScrOutData, 30, UPPERCASE_MODE) == TRUE)
						{
							m_strScrOutData.TrimLeft();
							m_strScrOutData.TrimRight();	// 우측만 Trim 처리
							strFarewellMsg[m_nCurrentFocusIndex-1] = m_strScrOutData;
						}

						bShowScreen = TRUE;
					}
					break;

				default:
					break;
				}
			}
			else if (m_strScrOutData == _T("SET"))
			{
				for(i=0; i<3; i++)
				{
					CEagleDataManager::GetInstance()->m_Config.m_Message.strFarewell_message[i] = strFarewellMsg[i];
				}

				CEagleDataManager::GetInstance()->m_Config.SaveMessage();

				Operator_Processing_Screen(_T("SUCCESS"));
				CUtil::Sleep_Wait(EAGLE_SLEEP_SCREEN_INTERVAL);

				m_nCurrentFocusIndex = 1;
				bShowScreen = TRUE;
			}
			else
			{
				// 비정상적 값인 경우에는 OP MAIN으로 이동
				m_strNextScrNum = OP_SCREEN_MAIN;
				break;
			}
		}

		Sleep(EAGLE_SLEEP_INTERVAL);
	}
}