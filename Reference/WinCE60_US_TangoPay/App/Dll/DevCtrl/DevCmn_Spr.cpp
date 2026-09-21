/////////////////////////////////////////////////////////////////////////////
//	DevSpr.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include ".\Dev\DevCmn.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1


/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnSPR_ClearErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Clear Error Code
-------------------------------------------------------------------*/
int	CDevCmn::fnSPR_ClearErrorCode()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_ClearErrorCode]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
		nReturn = m_pMwi->SprClearErrorCode();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnSPR_ClearRetractCnt()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Clear Retract Count
-------------------------------------------------------------------*/
int	CDevCmn::fnSPR_ClearRetractCnt()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_ClearRetractCnt]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
		nReturn = m_pMwi->SprClearRetractCnt();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrSPR_GetErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Error Code
-------------------------------------------------------------------*/
CString CDevCmn::fstrSPR_GetErrorCode()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fstrSPR_GetErrorCode]\n"));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_SPR))
		strReturn = m_pMwi->SprGetErrorCode();

	if(strReturn.GetLength() > 0)
	{
		NVDump('F', 'E', "56", L"", strReturn);		
	}

	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnSPR_GetDeviceStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Device Status
-------------------------------------------------------------------*/
int	CDevCmn::fnSPR_GetDeviceStatus()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_GetDeviceStatus]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
		nReturn = m_pMwi->SprGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"fnSPR_GetDeviceStatus changed [%d] => [%d]\n", nSaveReturn, nReturn));
		nSaveReturn = nReturn;
		m_strNVLog.Format(L"1:SPRST(%d)", nReturn);
		NVDump('O', 'E', "58", L"", m_strNVLog);
	}
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnSPR_GetPaperStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Paper Status
-------------------------------------------------------------------*/
int	CDevCmn::fnSPR_GetPaperStatus()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_GetPaperStatus]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
		nReturn = m_pMwi->SprGetPaperStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
		m_strNVLog.Format(L"1:PAPERST(%d)", nReturn);
		NVDump('O', 'E', "58", L"", m_strNVLog);

		NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_GetPaperStatus] [%d] \n", nReturn));
	}

	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_PAPER_LOW_SENSOR_ENABLE) == 1)
	{
		if (nReturn == SLIP_LOW_END)
			nReturn = SLIP_NORMAL;

		m_strNVLog.Format(L"1:PAPERST1(%d)", nReturn);
		NVDump('O', 'E', "58", L"", m_strNVLog);
	}

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnSPR_GetMaterialInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Material Information
-------------------------------------------------------------------*/
int	CDevCmn::fnSPR_GetMaterialInfo()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_GetMaterialInfo]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
		nReturn = m_pMwi->SprGetMaterialInfo();

	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"fnSPR_GetMaterialInfo changed [%d] => [%d]\n", nSaveReturn, nReturn));
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnSPR_Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Initialize
-------------------------------------------------------------------*/
int	CDevCmn::fnSPR_Initialize()
{
	NHDEBUG(DBG_CALL, (_T("[CDevCmn::fnSPR_Initialize]\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_SPR))
		nReturn = m_pMwi->SprInitialize();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnSPR_Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : De-Initialize
-------------------------------------------------------------------*/
int	CDevCmn::fnSPR_Deinitialize()
{
	NHDEBUG(DBG_CALL, (_T("[CDevCmn::fnSPR_Deinitialize]\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_SPR))
		nReturn = m_pMwi->SprDeinitialize();

	return nReturn;
}

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnSPR_PrintReceipt()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 명세표를 인자 한다.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnSPR_PrintReceipt(BOOL bPrintImage, CString strData, int nWaitSec, BOOL bEject, BOOL bConvertBCD)	// [#2185] US Justin 2013.05.14 Add Eject(Cut) Option  // [#2373] Add ConvertBCD Option
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_PrintReceipt]\n"));

	BOOL	bRes = FALSE;

	// KSK 2010.04.16 Print할 Data가 없는 경우 정상 return
	if (bPrintImage == FALSE && strData.GetLength() <= 0)
		return TRUE;

	// [#2205] US KSK 2013.06.28
//	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_ON);
	// end of [#2205]

	if(bEject == TRUE)
	{
		int lineCount = 0;
		int findLoc = -1;
		while ((findLoc = strData.Find('\x1c', findLoc + 1)) != -1)
		{
			lineCount++;
		}
		// The printer prints 35 lines at a time, and if the ((35 * n) + 1st) line 
		// is "%EJECT%", it will be ignored
		if (lineCount % 35 == 0) // True if the last line will be 
		{
			// Add blank line to fool the printing!
			AddPrintData(strData, L" ");
		}
		AddPrintData(strData, L"%S", SPR_EJECT_STRING);
	}
	// End of [#2562]

	while (1)
	{
		// Print Image
 		if (bPrintImage == TRUE)
 		{
 			NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintHeader()\n"));
 			if ((bRes = fnSPR_PrintHeader()) == FALSE)
 			{
 				NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintHeader() failed\n"));
 				break;
 			}
 		}


		// [#2373] US Justin 2015.09.22 Support Coupon Image on Receipt
		/*
		NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintData()\n"));
		if ((bRes = fnSPR_PrintData(strData)) == FALSE)
		{
			NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintData() failed\n"));
			break;
		}
		*/
		if(bConvertBCD!=TRUE)
		{
			// Print Text
			NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintData()\n"));
			if ((bRes = fnSPR_PrintData(strData)) == FALSE)
			{
				NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintData() failed\n"));
				break;
			}
		}
		else
		{
			// Print Text and Barcode....
			BOOL bPrintAll = FALSE;
			CString strPrintText;
			CString strBarcode, strQRcode;
			CString strRemainText = strData;

			CString strBCDBg = L"<BARCODE_NUMBER>";
			CString strBCDEd = L"</BARCODE_NUMBER>";
			CString strQRBg  = L"<BARCODE_QR>";
			CString strQREd  = L"</BARCODE_QR>";
			int nLocBCDBegin, nLocBCDEnd, nBCDLen;
			int nLocQRBegin,  nLocQREnd,  nQRLen;
			do
			{
				// Check Numeric and QR Barcode data
				// Initialize Data
				strPrintText = L"";		strBarcode = L"";		strQRcode = L"";

				// Find Numeric Barcode Data
				nLocBCDBegin	= strRemainText.Find( strBCDBg );
				nLocBCDEnd		= strRemainText.Find( strBCDEd );
				nBCDLen			= nLocBCDEnd - nLocBCDBegin - strBCDBg.GetLength();
				// Validation
				if( (nLocBCDBegin<0) || (nLocBCDEnd<0) || (nLocBCDEnd<=nLocBCDBegin) || (nBCDLen<=0) )
					nBCDLen = 0;

				// Find QR Data
				nLocQRBegin		= strRemainText.Find( strQRBg );
				nLocQREnd		= strRemainText.Find( strQREd );
				nQRLen			= nLocQREnd - nLocQRBegin - strQRBg.GetLength();
				// Validation
				if( (nLocQRBegin<0) || (nLocQREnd<0) || (nLocQREnd<=nLocQRBegin) || (nQRLen<=0) )
					nQRLen = 0;

				// Numeric Barcode comes First
				if( ( (nBCDLen>0) && (nQRLen>0) && (nLocBCDBegin<nLocQRBegin) ) ||			// Both enabled and Numeric BCD comes first
					( (nBCDLen>0) && (nQRLen==0) ) )										// Only Numeric BCD is enabled
				{
					strPrintText	= strRemainText.Left(nLocBCDBegin);
					strBarcode		= strRemainText.Mid (nLocBCDBegin + strBCDBg.GetLength(), nBCDLen);
					strBarcode.Replace( CString(FIELD_DELIMITER), L"");	
					strBarcode.TrimLeft();
					strBarcode.TrimRight();
					strRemainText	= strRemainText.Mid (nLocBCDEnd   + strBCDEd.GetLength() );
					if(strRemainText.GetLength() <= 0)
						bPrintAll = TRUE;
				}
				// QR Comes First
				else if( ( (nBCDLen>0) && (nQRLen>0) && (nLocQRBegin<nLocBCDBegin) ) ||		// Both enabled and Numeric QR comes first
						 ( (nQRLen>0) && (nBCDLen==0) ) )									// Only QR is enabled
				{
					strPrintText	= strRemainText.Left(nLocQRBegin);
					strQRcode		= strRemainText.Mid (nLocQRBegin + strQRBg.GetLength(),  nQRLen);
					strQRcode.Replace( CString(FIELD_DELIMITER), L"");	
					strQRcode.TrimLeft();
					strQRcode.TrimRight();
					strRemainText	= strRemainText.Mid (nLocQREnd   + strQREd.GetLength() );
					if(strRemainText.GetLength() <= 0)
						bPrintAll = TRUE;
				}
				// No Barcode nor QR Code
				else
				{
					strPrintText = strRemainText;
					bPrintAll = TRUE;
				}

				// Print Text
				// Remove the first and the Last "FIELD_DELIMITER" : Remove Unnecessary Line Feed
				if(strPrintText.Left(1) == CString(FIELD_DELIMITER) )
					strPrintText = strPrintText.Mid(1);
				if(strPrintText.Right(1) == CString(FIELD_DELIMITER) )
					strPrintText = strPrintText.Left(strPrintText.GetLength()-1);

				if(strPrintText.GetLength()>0)
				{						
					NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintData()\n"));
					if ((bRes = fnSPR_PrintData(strPrintText)) == FALSE)
					{
						NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_PrintData() failed\n"));
						break;
					}
				}

				// Print Numeric Barcode
				if(strBarcode.GetLength() > 0)
				{
					NHDEBUG(DBG_INFO, (L"CreateNumberBarcode(%s)\n", strBarcode));
					DeleteFile(COUPON_IMAGE_FILE_BCD);		// Delete Existing File

					if( CreateNumberBarcode(strBarcode, 512, 120, COUPON_IMAGE_FILE_BCD) )				// Width :Multiple of 64...
					{
						NHDEBUG(DBG_INFO, (L"CreateNumberBarcode(%s) SUCCESS\n", strBarcode));
		 				if ((bRes = fnSPR_PrintCouponImage(IMAGETYPE_COUPON_BCD)) == FALSE)
						{
							NHDEBUG(DBG_INFO, (L"Printing Barcode FAILURE\n"));
							break;
						}
					}
					// Continue to print even Barcode file is not generated.
					//else
					//{
					//	NHDEBUG(DBG_INFO, (L"CreateNumberBarcode(%s) FAILURE\n", strBarcode));
					//	bRes = FALSE;
					//	break;
					//}
				}

				// Print QR
				if(strQRcode.GetLength() > 0)
				{
					NHDEBUG(DBG_INFO, (L"QR Data=[%s], File Name =[%s]\n", strQRcode, COUPON_IMAGE_FILE_QR));

					// [#2503] US Justin 2017.09.15 Support Just Cash Purchasing Ethereum.. Download QR IMAGE
					BOOL bUseQRFile = FALSE;
					if(strQRcode.Left(29) == L"**USE STORED QR IMAGE FILE**:" )
					{
						bUseQRFile = TRUE;
						CString strStoredFile = strQRcode.Mid(29);
						NHDEBUG(DBG_INFO, (L"USE STORED QR IMAGE FILE =[%s]\n", strStoredFile ));
						DeleteFile(COUPON_IMAGE_FILE_QR);		// Delete Existing File
						::CopyFile(strStoredFile, COUPON_IMAGE_FILE_QR, FALSE); 
					}
					else
					{
						DeleteFile(COUPON_IMAGE_FILE_QR);		// Delete Existing File

						MakeQRCodeImageFile(strQRcode, COUPON_IMAGE_FILE_QR);
						NHDEBUG(DBG_INFO, (L"QR image generated [%s]\n", COUPON_IMAGE_FILE_QR));
					}
					// End of [#2503]

					// [#2503] US Justin 2017.09.15 Support Just Cash Purchasing Ethereum.. Download QR IMAGE
					/*
					// Find Created QR IMAGE
					WIN32_FIND_DATA FileData;
					HANDLE h = INVALID_HANDLE_VALUE;
					//if( (h = FindFirstFile( QRCODE_GENERATOR, &FileData ))  != INVALID_HANDLE_VALUE  )
					if( (h = FindFirstFile( COUPON_IMAGE_FILE_QR, &FileData ))  != INVALID_HANDLE_VALUE  )		// [#2382] US JUSTIN BUG FIX
					{
						FindClose(h);	
 						if ((bRes = fnSPR_PrintCouponImage(IMAGETYPE_COUPON_QR)) == FALSE)
						{
							NHDEBUG(DBG_INFO, (L"Printing QR FAILURE\n"));
							break;
						}
					}
					// Continue to print even QR file is not generated.
					//else
					//{
					//	NHDEBUG(DBG_INFO, (L"Generating QR File (%s) FAILURE\n", strQRcode));
					//	bRes = FALSE;
					//	break;
					//}
					*/
					CFileStatus status;
					if( CFile::GetStatus( COUPON_IMAGE_FILE_QR, status ) )
					{
						NHDEBUG(DBG_INFO, (L"QR CODE IMAGE FILE [%s] FOUND : Size =[%ld]\n", COUPON_IMAGE_FILE_QR, status.m_size));
						if(bUseQRFile) //if(status.m_size > 10000)		// Bigger than 10K
							bRes = fnSPR_PrintCouponImage(IMAGETYPE_COUPON_LARGEQR);
						else
							bRes = fnSPR_PrintCouponImage(IMAGETYPE_COUPON_QR);

 						if(bRes == FALSE)
						{
							NHDEBUG(DBG_INFO, (L"Printing QR FAILURE\n"));
							break;		// PRINTING ERROR => RETURN FALSE
						}
					}
					else				// NO QR IMAGE FILE  => SKIP PRINTING
					{
						NHDEBUG(DBG_INFO, (L"ERROR --- No QR CODE IMAGE FILE [%s] FOUND\n", COUPON_IMAGE_FILE_QR));
					}
				}
			} while (bPrintAll==FALSE);

			if(bRes==FALSE)
				break;
		}
		// End of [#2373]


		// [#2562] NH Justin 2018.07.12 Reduce PAUSE INTERVAL Between PRINT and CUT
		/*
		// Eject Paper
		if(bEject == TRUE)	// [#2185] US Justin 2013.05.14 Add Eject(Cut) Option 	
		{
			NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_EjectPaper()\n"));
			if ((bRes = fnSPR_EjectPaper(nWaitSec)) == FALSE)
			{
				NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] fnSPR_EjectPaper() failed\n"));
				break;
			}
		}
		*/
		// End of [#2562]

		bRes = TRUE;
		break;
	}

	// [#2205] US KSK 2013.06.28
//	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) != L"NH2600")
	if (!(MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"NH2600" || MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE"))	// [#2327] NH KSK 2015.01.24 SPR / CDU Flicker 없음 (NH2600 / MX2600SE)
		fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);
	// end of [#2205]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnSPR_PrintReceipt] bRETURN(%d)\n", bRes));

	return bRes;
}

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnSPR_PrintHeader()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 명세표 이미지를 인자한다.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnSPR_PrintHeader()
{
#ifndef UNDER_CE
	// skip on win32 debugging as nextware simulator doesn't support image printing
	return TRUE;
#endif

	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_PrintHeader]\n"));

	if (fnAPL_GetDefineDevice(DEV_SPR))
	{
		fnAPL_CheckDeviceAction(DEV_SPR);

		m_pMwi->SprPrint(CMD_PRINT_HEADER_IMAGE);		// [#2373] US Justin 2015.09.22 Change Variable Name

		//fnAPL_CheckDeviceAction(DEV_SPR);	// [#2307] 명세표 Print 시간 단축을 위해 Image Print 후 Delay를 줄이기 위해 대기 하지 않고 다음 Step을 수행하도록 변경
		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_SPR))
			return TRUE;
	}

	return FALSE;
}

// [#2373] US Justin 2015.09.22 Support Coupon Image
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnSPR_PrintCouponImage()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Coupon Image
-------------------------------------------------------------------*/
BOOL CDevCmn::fnSPR_PrintCouponImage(int nType)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_PrintCouponImage]\n"));

	if (fnAPL_GetDefineDevice(DEV_SPR))
	{
		fnAPL_CheckDeviceAction(DEV_SPR);

		if(nType == IMAGETYPE_COUPON_BCD)			m_pMwi->SprPrint(CMD_PRINT_COUPON_IMAGE_BCD);
		else if(nType == IMAGETYPE_COUPON_QR)		m_pMwi->SprPrint(CMD_PRINT_COUPON_IMAGE_QR);
		else if(nType == IMAGETYPE_COUPON_LARGEQR)	m_pMwi->SprPrint(CMD_PRINT_COUPON_IMAGE_LARGEQR);

		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_SPR))
			return TRUE;
	}

	return FALSE;
}
// End of [#2373]



const WCHAR wchAccentData[] = { L"À", };

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnSPR_PrintData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 주어진 데이터를 인자한다.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnSPR_PrintData(CString strData)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_PrintData]\n"));

	CString strPrintData;	// [RWC6-676] Start SKKim 2024.05.10
	WCHAR wchsplash[2];

	wchsplash[0] = 92;
	wchsplash[1] = 0;

	if (strData.GetLength() <= 0)
		return FALSE;

	// [RWC6-676] Start SKKim 2024.05.10 replace spanish accent data to non accent data (only apply tangopay service)
#if (APP_TANGOPAY)
				//   "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"
	const char* tr = "AAAAAAECEEEEIIIIDNOOOOOx0UUUUYPsaaaaaaeceeeeiiiiOnooooo/0uuuuypy";
	//char chValue = 0;
	WCHAR wchValue = 0;
	int nDataLen = strData.GetLength();

	for(int i=0; i<nDataLen; i++)
	{
		wchValue = strData.GetAt(i);

		//if (wchValue >= 0xa0)
		{
			if ( wchValue >= 0xc0 && wchValue <= 0xff)
			{
				//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_PrintData - Before (%c)]\n", wchValue));
				wchValue = tr[wchValue - 0xc0];
				//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_PrintData - After (%c)]\n", wchValue));
			}
			//else if (wchValue > 0xff)
			//{
			//	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_PrintData - range over Before (0x%X)]\n", wchValue));
			//	wchValue = '.';	// ('.'으로 인자)
			//}
		}

		strPrintData += (CString)wchValue;
	}
#else
	strPrintData = strData;
#endif
	// [RWC6-676] End SKKim 2024.05.10

	fnAPL_CheckDeviceAction(DEV_SPR);
	DeviceOperationFlag = TRUE;

	if ((fnAPL_GetDefineDevice(DEV_SPR)) && 
		//(strData.GetLength() > 0))
		(strPrintData.GetLength() > 0))
	{
		//m_pMwi->SprPrint(strData);
		m_pMwi->SprPrint(strPrintData);

		fnAPL_CheckDeviceAction(DEV_SPR);
		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_SPR))
			return TRUE;
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnSPR_EjectPaper()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 종이를 배출한다.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnSPR_EjectPaper(int nWaitSec)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_EjectPaper]\n"));

	fnAPL_CheckDeviceAction(DEV_SPR);
	DeviceOperationFlag = TRUE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
	{
		fnAPL_CheckDeviceAction(DEV_SPR);

		m_pMwi->SprEject(nWaitSec);

		fnAPL_CheckDeviceAction(DEV_SPR);
		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_SPR))
			return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnSPR_WaitTaken()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 명세표 수취를 기다린다.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnSPR_WaitTaken()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_WaitTaken]\n"));

	DeviceOperationFlag = TRUE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
	{
		fnAPL_CheckDeviceAction(DEV_SPR);

		m_pMwi->SprWaitTaken();

		fnAPL_CheckDeviceAction(DEV_SPR);
		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_SPR))
			return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnSPR_Retract()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 명세표를 회수 한다.
-------------------------------------------------------------------*/
BOOL CDevCmn::fnSPR_Retract()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnSPR_Retract]\n"));

	DeviceOperationFlag = TRUE;

	if (fnAPL_GetDefineDevice(DEV_SPR))
	{
		fnAPL_CheckDeviceAction(DEV_SPR);

		m_pMwi->SprRetract();

		fnAPL_CheckDeviceAction(DEV_SPR);
		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_SPR))
			return TRUE;
	}

	return FALSE;
}
