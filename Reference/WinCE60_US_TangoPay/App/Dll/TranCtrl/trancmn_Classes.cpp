/////////////////////////////////////////////////////////////////////////////
// TranCmn_TritonTDL.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\Tran\TranCmn.h"
#include ".\Common\Markup.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\..\..\H\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

/////////////////////////////////////////////////////////////////////////////
// Triton TDL Data Class
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
// CSTD3_TDL_DATA

CSTD3_TDL_DATA::CSTD3_TDL_DATA()
{
	ClearAllData();
}

CSTD3_TDL_DATA::~CSTD3_TDL_DATA()
{
}

void CSTD3_TDL_DATA::ClearAllData()
{
	m_nTranType = _TDL_TRAN_NONE;
	m_nDCCProc  = _TDL_DCCPROC_NONE;
	m_bTranResult = FALSE;
	m_bProceedDCC = FALSE;
	
	m_strSurchargeAmount = _T("");
	m_strWorkingKey = _T("");

	m_strTID = _T("");
	m_strSeqNum = _T("");
	m_strTimeStamp = _T("");

	m_nHexSeqNum = 0;		// [#2577] US Justin 2018.09.12 Fix TDL Sequence Number : String => HexDecimal

	m_DCC_Lookup.ClearData();
	m_DCC_Transaction.ClearData();
	m_DCC_Reversal.ClearData();
}

CString	CSTD3_TDL_DATA::MakeTDLTag(int nTranCode, CString strTID, CString strSeqNum)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DATA::MakeTDLTag(%d, %s, %s)\n", nTranCode, strTID, strSeqNum ));
	CString sRtn = _T("");
	m_bTranResult = FALSE;

	if( nTranCode==TC_TDL_TIRSURCHARGE )
	{
		m_nTranType = _TDL_TRAN_DYNAMICSURCHARGE;
		sRtn = _T("qa");		// [#2296] US Justin 2014.10.17 Cardtronics request..Sending Empty "qa" field.
	}
	else if( (nTranCode==TC_TDL_DCC_LOOKUP)||(nTranCode==TC_TDL_DCC_TRANSACTION)||(nTranCode==TC_TDL_DCC_REVERSAL) )
	{
		m_nTranType = _TDL_TRAN_DCC;

		int nProcStep = _TDL_DCCPROC_LOOKUP;
		if (nTranCode==TC_TDL_DCC_TRANSACTION)		nProcStep = _TDL_DCCPROC_TRANSACTION;
		else if (nTranCode==TC_TDL_DCC_REVERSAL)	nProcStep = _TDL_DCCPROC_REVERSAL;

		sRtn = _T("qc") + MakeDCCRequestData(strTID, strSeqNum, nProcStep);
	}
	return sRtn;
}

BOOL  CSTD3_TDL_DATA::ParseTDLTag(CString strResponse)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DATA::ParseTDLTag(%s)\n", strResponse ));

	m_bTranResult = FALSE;

	// Initialize Working Key......
	m_strWorkingKey = _T("");

	if(strResponse.GetLength() < 2 )
		return m_bTranResult;

	CString sIdentifier = strResponse.Left(2);
	CString sValue = strResponse.Mid(2);

	if( (sIdentifier.Compare(L"qa")==0)&&(m_nTranType==_TDL_TRAN_DYNAMICSURCHARGE)	)		// TIR Surcharge
	{
		NHDEBUG(DBG_INFO, (L"  qa Tag Length Value = [%s] \n", sValue ));
		
		// [#2299] US Justin 2014.10.25 Selecting only DF0B from "qa" data
		int nLoc = strResponse.Find(L"DF0B");
		if(nLoc<0)
			return m_bTranResult;

		sValue = strResponse.Mid(nLoc);
		// End of [#2299]

		if(sValue.GetLength()>6)
		{
			CString strTag = sValue.Left(4);
			if(strTag.Compare(L"DF0B")==0)			// Tag "DF0B" : Surcharge
			{
				//[#2299]US Justin 2014.10.25 Selecting only DF0B from "qa" data
				/*
				CString strValueLength = sValue.Mid(4,2);
				CString strActualValue = sValue.Mid(6);
				NHDEBUG(DBG_INFO, (L"  Tag=[%s], Length=[%s], Value=[%s]\n", strTag, strValueLength, strActualValue ));
				if( Asc2Int(strValueLength) == strActualValue.GetLength() )
				{
					m_strSurchargeAmount.Format( L"%08d", Dollar2Cent(strActualValue) );
					m_bTranResult = TRUE;					
					NHDEBUG(DBG_INFO, (L"  Received Surcharge=[%s]\n",m_strSurchargeAmount));
				}
				*/
				int nValueLength = Asc2Int(sValue.Mid(4,2));
				m_strSurchargeAmount.Format( L"%08d", Dollar2Cent(sValue.Mid(6, nValueLength)) );
				m_bTranResult = TRUE;
				// End of [#2299]
			}
		}
	}
	else if( (sIdentifier.Compare(L"qc")==0)&&(m_nTranType==_TDL_TRAN_DCC) )				// DCC Response
		m_bTranResult = ParseDCCResponseData(sValue);

	return m_bTranResult;
}

CString	CSTD3_TDL_DATA::UpdateSurchargeFromDCCLookup()
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DATA::UpdateSurchargeFromDCCLookup\n"));
	m_strSurchargeAmount = _T("");
	if(m_bTranResult)
	{
		int surcharge = 0;
		
		if( m_DCC_Lookup.m_strRes_SurchargeType == L"3")			// Host Provided
		{
			if (!m_DCC_Lookup.m_surchargePlusIvaTaxAmount.IsEmpty())
			{
				surcharge = Dollar2Cent(m_DCC_Lookup.m_surchargePlusIvaTaxAmount);
			}
			else if (!m_DCC_Lookup.m_ivaTaxAmount.IsEmpty())
			{
				surcharge += Dollar2Cent(m_DCC_Lookup.m_ivaTaxAmount);
				surcharge += Dollar2Cent(m_DCC_Lookup.m_strRes_SurchargeAmount);
			}
			else
			{
				surcharge = Dollar2Cent(m_DCC_Lookup.m_strRes_SurchargeAmount);
			}
		}
		else if( m_DCC_Lookup.m_strRes_SurchargeType == L"1")		// No Surcharge
		{
			surcharge = 0;
		}

		m_strSurchargeAmount.Format(L"%08d", surcharge);

		NHDEBUG(DBG_INFO, (L"  Updated Surcharge - Surcharge Type = [%s], New Surcharge Amount = [%s]\n", m_DCC_Lookup.m_strRes_SurchargeType, m_strSurchargeAmount));
	}
	return m_strSurchargeAmount;
}

////////// Private ////////////////////////////////////////
CString	CSTD3_TDL_DATA::MakeDCCRequestData(CString strTID, CString strSeqNum, int nProcStep)
{
	NHDEBUG(DBG_INFO, (L"[CSTD3_TDL_DATA::MakeDCCRequestData(TID=[%s], SeqNum=[%s], Proc=[%d])]\n", strTID, strSeqNum, nProcStep));

	CString sRtn = _T("");
	if( (nProcStep!=_TDL_DCCPROC_LOOKUP)&&(nProcStep!=_TDL_DCCPROC_TRANSACTION)&&(nProcStep!=_TDL_DCCPROC_REVERSAL) )
		return sRtn;

	m_nDCCProc  = nProcStep;

	m_strTID = strTID.TrimRight();
	m_strSeqNum = strSeqNum;
	m_nHexSeqNum = Asc2Int(m_strSeqNum);		// [#2577] US Justin 2018.09.12 Fix TDL Sequence Number : String => HexDecimal

	CString strTemp;
	//////////////////////////////////////////// Temporary DATA BUFFER
	BYTE szDATA[_TDL_REQUEST_BUFFER_SIZE];
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	int nDataSz = 0;

	///////////////////////////////////////////////////////////////////
	///  Correlation Data
	///////////////////////////////////////////////////////////////////
	///////////////// Terminal ID /////////////
	// TAG DF 64
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X64;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strTID);

	///////////////// Sequence Number /////////////
	
#if (APP_CUSTOM_CIBC_MX)
	bool needSequenceNumber = nProcStep == _TDL_DCCPROC_TRANSACTION || nProcStep == _TDL_DCCPROC_REVERSAL || nProcStep == _TDL_TRAN_DCC;
#else 
	bool needSequenceNumber = nProcStep == _TDL_DCCPROC_TRANSACTION || nProcStep == _TDL_DCCPROC_REVERSAL;
#endif

	if (needSequenceNumber)
	{
		// TAG DF 68
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X68;

		// [#2577] US Justin 2018.09.12 Fix TDL Sequence Number : String => HexDecimal
		//nDataSz += TLV_FillValueString(&szDATA[nDataSz], strSeqNum);
		strTemp.Format(L"%08X", m_nHexSeqNum);
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], strTemp);
		// End of [#2577]
	}

	///////////////// TimeStamp /////////////
	// TAG DF 29
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X29;
	CTime		CurTime = CTime::GetCurrentTime();
	strTemp.Format(L"%04d-%02d-%02d %02d:%02d:%02d", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], strTemp);

	/////////////////////////////////////////////////////////////////////////////////
	// FINAL Correlation FF 28
	// Correlation Data Buffer
	BYTE pCorrelation[200];
	memset(pCorrelation, NULL, 200);
	int nLenCorrelation = 0;
	pCorrelation[nLenCorrelation++] = 0XFF;		pCorrelation[nLenCorrelation++] = 0X28;
	nLenCorrelation += TLV_FillValueBuffer(&pCorrelation[nLenCorrelation], szDATA, nDataSz);

	///////////////////////////////////////////////////////////////////
	///  Request Data
	///////////////////////////////////////////////////////////////////
	// Buffer Initialize
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	// Get Request Data
	if (nProcStep==_TDL_DCCPROC_LOOKUP)
		nDataSz = m_DCC_Lookup.MakeRequestMessage(szDATA);
	else if (nProcStep==_TDL_DCCPROC_TRANSACTION)
		nDataSz = m_DCC_Transaction.MakeRequestMessage(szDATA);
	else if (nProcStep==_TDL_DCCPROC_REVERSAL)
		nDataSz = m_DCC_Reversal.MakeRequestMessage(szDATA);

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///  Get or Set Data Request
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	// DATA BUFFER
	BYTE pDataBuffer[_TDL_REQUEST_BUFFER_SIZE];
	memset(pDataBuffer, NULL, _TDL_REQUEST_BUFFER_SIZE);
	int nLenDataBuffer = 0;

	// Get(FF 0D) or Set(FF 0F) Data Request Tag
	pDataBuffer[nLenDataBuffer++] = 0XFF;
	if (nProcStep==_TDL_DCCPROC_LOOKUP)		pDataBuffer[nLenDataBuffer++] = 0X0D;		// Get Data Request (FF 0D)
	else									pDataBuffer[nLenDataBuffer++] = 0X0F;		// Set Data Request (FF 0F)
	
	// Fill Length
	nLenDataBuffer += TLV_FillLength( &pDataBuffer[nLenDataBuffer], (nLenCorrelation+nDataSz) );

	// Fill Correlation Info
	memcpy( &pDataBuffer[nLenDataBuffer], pCorrelation, nLenCorrelation);
	nLenDataBuffer += nLenCorrelation;

	// Fill Request Data
	memcpy( &pDataBuffer[nLenDataBuffer], szDATA, nDataSz);
	nLenDataBuffer += nDataSz;

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///  Session
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	// Buffer Initialize
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	nDataSz = 0;

	///////////////// Destination SAP /////////////
	// TAG DF A7 09
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA7;	szDATA[nDataSz++] = 0X09;
	strTemp = _T("8");				// [#2340] US Justin 2015.04.17 SPEC Change, "6" => "8"
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], strTemp);

	///////////////// Session Message ID /////////////
	// TAG DF A7 0A
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA7;	szDATA[nDataSz++] = 0X0A;
	strTemp = _T("7");				// Single Request
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], strTemp);

	///////////////// Source SAP /////////////
	// TAG DF A7 0B
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA7;	szDATA[nDataSz++] = 0X0B;
	strTemp = _T("8");				// [#2340] US Justin 2015.04.17 SPEC Change, "6" => "8"
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], strTemp);
	
	///////////////// End Point ID (DCC) /////////////
	// TAG DF A7 13
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA7;	szDATA[nDataSz++] = 0X13;
	strTemp = _T("US");				// [#2340] US Justin 2015.04.17 SPEC Change, "DCC" => "US"
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], strTemp);

	///////////////// Application Data (Correlation data + Request)/////////////
	// TAG DF A7 10
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA7;	szDATA[nDataSz++] = 0X10;
	nDataSz += TLV_FillLength( &szDATA[nDataSz], nLenDataBuffer );
	memcpy(&szDATA[nDataSz], pDataBuffer ,nLenDataBuffer);
	nDataSz += nLenDataBuffer;

	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	// FINAL Session
	// Initialize Buffer
	memset(pDataBuffer, NULL, _TDL_REQUEST_BUFFER_SIZE);
	nLenDataBuffer = 0;
	///////////////// SESSION PDU /////////////
	// TAG FF A7 08
	pDataBuffer[nLenDataBuffer++] = 0XFF;	pDataBuffer[nLenDataBuffer++] = 0XA7;	pDataBuffer[nLenDataBuffer++] = 0X08;
	nLenDataBuffer += TLV_FillLength( &pDataBuffer[nLenDataBuffer], nDataSz );
	memcpy( &pDataBuffer[nLenDataBuffer], szDATA, nDataSz);
	nLenDataBuffer += nDataSz;

	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	// BASE 64 Encoding
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	nDataSz = base64_encode((char*)pDataBuffer, nLenDataBuffer, (char*) szDATA, _TDL_REQUEST_BUFFER_SIZE);

	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	// TDL Tag with Header (8 byte (T + VVV(Version) + EE(Encoding) + RR (Rule) ), 4 byte length )
	WCHAR* wchTmp = new TCHAR[_TDL_REQUEST_BUFFER_SIZE];
	memset(wchTmp, NULL, _TDL_REQUEST_BUFFER_SIZE);
	MultiToWide( (LPWSTR) wchTmp , (LPCSTR) szDATA, nDataSz);
	sRtn.Format( L"T1000100%04X%s", nDataSz, wchTmp);
	delete[] wchTmp;

	NHDEBUG(DBG_INFO, (L"RequestMsg=[%s]\n", sRtn));
	return sRtn;
}

BOOL CSTD3_TDL_DATA::ParseCorrelationInformation(BYTE* pTargetBuffer, int nBufSize)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DATA::ParseCorrelationInformation\n"));
	int nBufLoc, nTagID, nTagType, nDataLength;
	
	nBufLoc = 0;
	CString strValue;
	BOOL bEndOfData = FALSE;
	do
	{
		strValue = _T("");
		nTagID = TLV_GetTagIDandType(&pTargetBuffer[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )			// Incorrect Tag ID
			return FALSE;
		else
		{
			nDataLength = TLV_GetDataLength(&pTargetBuffer[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nBufSize )		// Size Error
				return FALSE;
			else
			{
				if( (nBufLoc + nDataLength) == nBufSize )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 100:				// DF 64 : Terminal ID
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &strValue, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF64 (TID) =[%s]\n", strValue));
						if (strValue.TrimRight().CompareNoCase(m_strTID) != 0)		// Terminal ID Check
							return FALSE;
						break;

					case 104:				// DF 68 : Sequence Number
						{
							nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &strValue, nDataLength);
							NHDEBUG(DBG_INFO, (L"DF68 (SEQ NUM) =[%s]\n", strValue));

							// [#2577] US Justin 2018.09.12 Fix TDL Sequence Number : String => HexDecimal
							//if ( strValue.CompareNoCase( m_strSeqNum) != 0 )
							//	return FALSE;

							int nHexDataLen = (int)(0.5 * strValue.GetLength() + 0.1);
							if( (nHexDataLen*2) != strValue.GetLength())
								strValue = L"0" + strValue;				// Make Even Length
							
							char *pSeqBuffer = new char[strValue.GetLength()+1];
							memset(pSeqBuffer, 0, strValue.GetLength()+1);
							WideToMulti(pSeqBuffer, strValue, strValue.GetLength());
							
							BYTE* pSeqbytes = new BYTE[nHexDataLen];
							Asc2Byten(pSeqBuffer, pSeqbytes, nHexDataLen);

							long nReceivedSeq = 0;
							long nMultiFactor = 1;
							for(int nbt = nHexDataLen-1; nbt>=0; nbt--)
							{
								nReceivedSeq += pSeqbytes[nbt] * nMultiFactor;
								nMultiFactor *= 256;
							}
							delete [] pSeqbytes;
							delete [] pSeqBuffer;

							NHDEBUG(DBG_INFO, (L"DF68 (SEQ NUM, Received, Converted) =[%ld], Original = [%ld]\n", nReceivedSeq, m_nHexSeqNum));

#if (APP_CUSTOM_CIBC_MX)
							if (nReceivedSeq == 0)
							{
								// Ignore 0 sequence number
								break;
							}
							else 
#endif
							if (nReceivedSeq != m_nHexSeqNum)
							{
								return FALSE;
							}
							// End of [#2577]
						}
						break;

					case 41:				// DF 29 : Timestamp
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strTimeStamp, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF29 (TimeStamp) =[%s]\n", m_strTimeStamp));
						if ( m_strTimeStamp.GetLength() != 19 )
							return FALSE;
						break;

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);
	return TRUE;
}

BOOL CSTD3_TDL_DATA::ParseSetDataResponseCode(BYTE* pTargetBuffer, int nBufSize)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DATA::ParseSetDataResponseCode\n"));
	int nBufLoc, nTagID, nTagType, nDataLength;
	
	nBufLoc = 0;
	CString strValue = _T("");
	BOOL bEndOfData = FALSE;
	do
	{
		nTagID = TLV_GetTagIDandType(&pTargetBuffer[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )			// Incorrect Tag ID
			return FALSE;
		else
		{
			nDataLength = TLV_GetDataLength(&pTargetBuffer[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nBufSize )		// Size Error
				return FALSE;
			else
			{
				if( (nBufLoc + nDataLength) == nBufSize )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 22:					// DF 16 : Response Code (1:Success)
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &strValue, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 16 (Response Code) =[%s]\n", strValue));
						break;

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	NHDEBUG(DBG_INFO, (L"SET DATA RESPONSE CODE ( 1 is success) =[%s]\n", strValue));
	if(strValue.CompareNoCase(L"1") == 0 )
		return TRUE;
	return FALSE;
}

BOOL CSTD3_TDL_DATA::ParseDCCResponseData(CString strResponse)
{
	NHDEBUG(DBG_INFO, (L"[CSTD3_TDL_DATA::ParseDCCResponseData(Res = [%s])]\n", strResponse));

	if(strResponse.GetLength() <13)
	{
		NHDEBUG(DBG_INFO, (L" TDL Parse ERROR : Length is too short, Received length = [%d]\n", strResponse.GetLength() ));
		return FALSE;
	}

	// Separate Header(8byte), Length(4), Data
	CString strTemp;
	CString strHeader = strResponse.Left(8);
	CString strLength = strResponse.Mid(8, 4);
	CString strData   =	strResponse.Mid(12);
	
	////////////////////////////////////////////////////////////
	// Check Length
	strTemp.Format(L"%04X", strData.GetLength() );
	if(strTemp.CompareNoCase(strLength) != 0)
	{
		NHDEBUG(DBG_INFO, (L" TDL Parse ERROR : Length is incorrect, Received[%s], Calculated[%s]\n", strLength, strTemp));
		return FALSE;
	}

	//////////////////////////////////////////// Temporary DATA BUFFERs
	BYTE szDATA[_TDL_REQUEST_BUFFER_SIZE], szTemp[_TDL_REQUEST_BUFFER_SIZE];
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	memset(szTemp, NULL, _TDL_REQUEST_BUFFER_SIZE);

	// Change CString => BYTE*
	WideToMulti( (char*)szDATA, strData, _TDL_REQUEST_BUFFER_SIZE );
	memcpy(szTemp, szDATA, strData.GetLength());

	// Base 64 Decoding
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);

	// NOW "szDATA" has Binary Data
	int nTDLSize = base64_decode( (char*)szTemp, strData.GetLength(), (unsigned char*) szDATA, strData.GetLength() );

	int nBufLoc, nTagID, nTagType, nDataLength;
	////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////
	// Check Session PDU ID 5000
	////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////
	nBufLoc = 0;
	nTagID = TLV_GetTagIDandType(&szDATA[nBufLoc], &nTagType, &nBufLoc);
	if( (nTagID != 5000)||(nTagType!=_TDL_TAGTYPE_CONSTRUCTIVE) )
	{
		NHDEBUG(DBG_INFO, (L" TDL Parse ERROR : PDU ID(5000) or Tag Type ERROR - Received ID = [%d]\n", nTagID));
		return FALSE;
	}
	// Check length of Session PDU Data
	nDataLength = TLV_GetDataLength(&szDATA[nBufLoc], &nBufLoc);
	if( (nTDLSize != (nBufLoc+nDataLength)) || (nDataLength<4) ) 
	{
		NHDEBUG(DBG_INFO, (L" TDL Parse ERROR : PDU Length Error - Msg [%d] != Actual [%d]\n", nDataLength, (nTDLSize-nBufLoc) ));
		return FALSE;
	}
	NHDEBUG(DBG_INFO, (L"PDU ID (5000) received\n"));

	// Get Data
	memset(szTemp, NULL, _TDL_REQUEST_BUFFER_SIZE);		// For Application Data
	int nApplicationDataLen=0;
	int nContainedValidData = 0;

	CString strValue;
	BOOL bEndOfData = FALSE;
	do
	{
		strValue = _T("");
		nTagID = TLV_GetTagIDandType(&szDATA[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )
		{
			NHDEBUG(DBG_INFO, (L" PDU Data Parse ERROR : TAG ID Error\n" ));
			return FALSE;
		}
		else
		{
			nDataLength = TLV_GetDataLength(&szDATA[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nTDLSize )
			{
				NHDEBUG(DBG_INFO, (L" PDU Data Parse ERROR : (Data Length + Buf Loc) [%d] is bigger than total data length [%d]\n", (nBufLoc + nDataLength), nTDLSize ));
				return FALSE;
			}
			else
			{
				if( (nBufLoc + nDataLength) == nTDLSize )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 5001:				// DF A7 09  (Destion SAP) "8" : DCC
						nBufLoc += TLV_GetDataString(&szDATA[nBufLoc], &strValue, nDataLength);
						NHDEBUG(DBG_INFO, (L"DFA709 (Des SAP) =[%s]\n", strValue));
						if ( strValue.CompareNoCase( L"8") != 0 )		// [#2340] US Justin 2015.04.17 SPEC Change, "6" => "8"
						{
							NHDEBUG(DBG_INFO, (L" PDU Data Parse ERROR : Destination SAP(8) Error - received[%s]\n", strValue ));
							return FALSE;
						}
						nContainedValidData++;
						break;

					case 5002:				// DF A7 0A  (Message ID) "8" : Single Msg
						nBufLoc += TLV_GetDataString(&szDATA[nBufLoc], &strValue, nDataLength);
						NHDEBUG(DBG_INFO, (L"DFA70A (Message ID) =[%s]\n", strValue));
						// [#2340] US Justin 2015.04.27 Cardtronics DCC SPEC is implemented incorrectly by ACI. TEMP CODE......(accepting both 7 and 8, 8 is correct number)
						//if ( strValue.CompareNoCase( L"8") != 0 )
						if ( (strValue.CompareNoCase( L"7") != 0)&&(strValue.CompareNoCase( L"8") != 0) )		// End of [#2340]
						{
							NHDEBUG(DBG_INFO, (L" PDU Data Parse ERROR : Message ID(8) Error - received[%s]\n", strValue ));
							return FALSE;
						}
						nContainedValidData++;
						break;

					case 5003:				// DF A7 0B  (Source SAP) "8" : DCC
						nBufLoc += TLV_GetDataString(&szDATA[nBufLoc], &strValue, nDataLength);
						NHDEBUG(DBG_INFO, (L"DFA70B (Source SAP) =[%s]\n", strValue));
						if ( strValue.CompareNoCase( L"8") != 0 )		// [#2340] US Justin 2015.04.17 SPEC Change, "6" => "8"
						{
							NHDEBUG(DBG_INFO, (L" PDU Data Parse ERROR : Source SAP (8) Error - received[%s]\n", strValue ));
							return FALSE;
						}
						nContainedValidData++;
						break;

					case 5011:				// DF A7 13  (EndPoint ID) "DCC"
						nBufLoc += TLV_GetDataString(&szDATA[nBufLoc], &strValue, nDataLength);
						NHDEBUG(DBG_INFO, (L"DFA713 (End point ID) =[%s]\n", strValue));
						if ( strValue.CompareNoCase( L"US") != 0 )		// [#2340] US Justin 2015.04.17 SPEC Change, "DCC" => "US"
						{
							NHDEBUG(DBG_INFO, (L" PDU Data Parse ERROR : End Point ID (US) Error - received[%s]\n", strValue ));
							return FALSE;
						}
						nContainedValidData++;
						break;

					case 5008:				// DF A7 10  Application Data
						NHDEBUG(DBG_INFO, (L"DFA710 (Aplication Data) Length =[%d]\n", nDataLength));
						memcpy(szTemp, &szDATA[nBufLoc], nDataLength);
						nBufLoc += nDataLength;
						nApplicationDataLen = nDataLength;						
						break;

					default:
						NHDEBUG(DBG_INFO, (L"Undefined TAG received = [ID:%d, Type:%d, Length:%d]\n", nTagID, nTagType, nDataLength));
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	if( (nContainedValidData<3)||(nApplicationDataLen<10) )
	{
		NHDEBUG(DBG_INFO, (L" TDL ERROR : Not enough Data, Contained number of Data = [%d], Application Data Size = [%d]\n", nContainedValidData, nApplicationDataLen ));
		return FALSE;
	}

	////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////
	// Check GET Data Response ID(14) or SET Data Response (16)
	////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////

	// Use szDATA for Application Data
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	memcpy(szDATA, szTemp, nApplicationDataLen);
	nBufLoc = 0;

	// Tag Check : FF 0E (14) or  FF 10 (16)
	nTagID = TLV_GetTagIDandType(&szDATA[nBufLoc], &nTagType, &nBufLoc);
	if (nTagType!=_TDL_TAGTYPE_CONSTRUCTIVE)
	{
		NHDEBUG(DBG_INFO, (L"  ApplicationData Parse ERROR : GET/SET DATA Respons Tag is not Constructive\n"));
		return FALSE;
	}
	if( (nTagID != 14)&&(nTagID != 16) )
	{
		NHDEBUG(DBG_INFO, (L" ApplicationData Parse ERROR : GET Response (14) /SET DATA Respons(16) Tag is wrong = [%d]\n", nTagID));
		return FALSE;
	}
	if( ( (m_nDCCProc==_TDL_DCCPROC_LOOKUP)&&(nTagID!=14) ) || ( (m_nDCCProc==_TDL_DCCPROC_LOOKUP)&&(nTagID==16) ) )
	{
		NHDEBUG(DBG_INFO, (L" ApplicationData Parse ERROR : Tag ID[%d] Mis-Match with Proc[%d]\n", nTagID, m_nDCCProc ));
		return FALSE;
	}
	NHDEBUG(DBG_INFO, (L"Data Type = [%d] received 14=Get response, 16=SET DATA response\n", nTagID));

	// Check Length
	nDataLength = TLV_GetDataLength(&szDATA[nBufLoc], &nBufLoc);
	if( (nApplicationDataLen != (nBufLoc+nDataLength)) || (nDataLength<4) ) 
	{
		NHDEBUG(DBG_INFO, (L" ApplicationData Parse ERROR : PDU Length Error - Msg [%d] != Actual [%d]\n", nDataLength, (nApplicationDataLen-nBufLoc) ));
		return FALSE;
	}

	// Check Data
	BOOL bGetResponse = FALSE;
	bEndOfData = FALSE;
	do
	{
		strValue = _T("");
		nTagID = TLV_GetTagIDandType(&szDATA[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )
		{
			NHDEBUG(DBG_INFO, (L" GET/SET Data Response Parse Error : TAG ID Error\n" ));
			return FALSE;
		}
		else
		{
			nDataLength = TLV_GetDataLength(&szDATA[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nApplicationDataLen )
			{
				NHDEBUG(DBG_INFO, (L" GET/SET Data Response Parse ERROR : (Data Length + Buf Loc) [%d] is bigger than total data length [%d]\n", (nBufLoc + nDataLength), nApplicationDataLen ));
				return FALSE;
			}
			else
			{
				if( (nBufLoc + nDataLength) == nApplicationDataLen )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 40:				// FF 28 (Correlation Information)
						if( !ParseCorrelationInformation( &szDATA[nBufLoc], nDataLength) )
						{
							NHDEBUG(DBG_INFO, (L" GET/SET Data Response Parse ERROR : Correlation Parse ERROR\n"));
							return FALSE;
						}
						nBufLoc += nDataLength;
						break;

					case 29:				// DF 1D (Error Code)
						if( m_nDCCProc==_TDL_DCCPROC_LOOKUP )
						{
							TLV_GetDataString(&szDATA[nBufLoc], &strValue, nDataLength);
							NHDEBUG(DBG_INFO, (L"DF 10 (GET DATA ERROR CODE) =[%s]\n", strValue));
							if( !strValue.IsEmpty() )
							{
								NHDEBUG(DBG_INFO, (L" GET Data Response: Received Error Code From Host [%s]\n", strValue ));
								// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
								// Changed application error handling logic as below, requested by Anny Hyder in Cardtronics on 04/15/2019
								//return FALSE;
								m_DCC_Lookup.m_strRes_DCCEligible = L"0"; // proceed as non-DCC
								return TRUE;
								// end of [#RWC6-2, #2585]
							}
						}
						nBufLoc += nDataLength;
						break;

					case 30:				// DF 1E (Error Description), 
						nBufLoc += nDataLength;	// Ignore
						break;

					case 21:				// FF 15 Set Data Response Code
						if( (m_nDCCProc==_TDL_DCCPROC_TRANSACTION)||(m_nDCCProc==_TDL_DCCPROC_REVERSAL) )
						{
							if( !ParseSetDataResponseCode(&szDATA[nBufLoc], nDataLength) )
							{
								NHDEBUG(DBG_INFO, (L" GET/SET Data Response code Parse ERROR : Set Data Res. code ERROR\n"));
								return FALSE;
							}
						}
						nBufLoc += nDataLength;
						break;

					case 1009:				// FF 87 71		DCC Look up response
						if(m_nDCCProc==_TDL_DCCPROC_LOOKUP)
						{
							NHDEBUG(DBG_INFO, (L"FF 87 71 LOOK UP RESPONSE RECEIVED\n"));
							if( !m_DCC_Lookup.ParseResponseMessage(&szDATA[nBufLoc], nDataLength) )
							{
								NHDEBUG(DBG_INFO, (L" LOOK UP Response ERROR\n"));
								return FALSE;
							}
							m_strWorkingKey = m_DCC_Lookup.m_strRes_WorkingKey;
							bGetResponse = TRUE;
						}
						nBufLoc += nDataLength;
						break;

					case 1032:				// FF 88 08		DCC Transaction Response
						if(m_nDCCProc==_TDL_DCCPROC_TRANSACTION)
						{
							NHDEBUG(DBG_INFO, (L"FF 88 08 TRANSACTION RESPONSE RECEIVED\n"));
							if( !m_DCC_Transaction.ParseResponseMessage(&szDATA[nBufLoc], nDataLength) )
							{
								NHDEBUG(DBG_INFO, (L" Transaction Response ERROR\n"));
								return FALSE;
							}
							m_strWorkingKey = m_DCC_Transaction.m_strRes_WorkingKey;
							bGetResponse = TRUE;
						}
						nBufLoc += nDataLength;
						break;

					case 1047:				// FF 88 17		DCC Reversal Response
						if(m_nDCCProc==_TDL_DCCPROC_REVERSAL)
						{
							NHDEBUG(DBG_INFO, (L"FF 88 17 REVERSAL RESPONSE RECEIVED\n"));
							if( !m_DCC_Reversal.ParseResponseMessage(&szDATA[nBufLoc], nDataLength) )
							{
								NHDEBUG(DBG_INFO, (L" Reversal Response ERROR\n"));
								return FALSE;
							}
							m_strWorkingKey = m_DCC_Reversal.m_strRes_WorkingKey;
							bGetResponse = TRUE;
						}
						nBufLoc += nDataLength;
						break;

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	// Assign Transaction Date and Time
	if( (bGetResponse==TRUE) && (m_nDCCProc==_TDL_DCCPROC_TRANSACTION) && (m_strTimeStamp.GetLength() == 19) )
	{
		m_DCC_Transaction.m_strRes_TranDateTime.Format(L"%s%s%s%s%s%s", m_strTimeStamp.Mid(5,2), m_strTimeStamp.Mid(8,2), m_strTimeStamp.Mid(2,2),		// MMDDYY
			m_strTimeStamp.Mid(11,2), m_strTimeStamp.Mid(14,2), m_strTimeStamp.Mid(17,2) );																// HHMMSS
	}

	return bGetResponse;
}

///////////////////////////////////////////////////
// Parsing Working Key....
///////////////////////////////////////////////////
CString GetWorkingKey(BYTE* pTargetBuffer, int nBufSize)
{
	NHDEBUG(DBG_INFO, (L"[CSTD3_TDL_DATA::GetWorkingKey]\n"));

	CString strWorkingKey = _T("");
	int nBufLoc, nTagID, nTagType, nDataLength;
	
	nBufLoc = 0;
	CString strKeyID = _T("");
	CString strKey = _T("");
	BOOL bEndOfData = FALSE;
	do
	{
		nTagID = TLV_GetTagIDandType(&pTargetBuffer[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )			// Incorrect Tag ID
			bEndOfData = TRUE;
		else
		{
			nDataLength = TLV_GetDataLength(&pTargetBuffer[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nBufSize )		// Size Error
				bEndOfData = TRUE;
			else
			{
				if( (nBufLoc + nDataLength) == nBufSize )
					bEndOfData = TRUE;

				switch (nTagID)
				{
					case 138:					// DF 81 0A : Key ID
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &strKeyID, nDataLength);
						break;

					case 113:					// DF 71	: Working Key
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &strKey, nDataLength);
						break;

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	if( (strKeyID.CompareNoCase(L"1")==0) && (strKey.GetLength()==32) )
	{
		strWorkingKey = strKey;
		NHDEBUG(DBG_INFO, (L"WORKING KEY RECEIVED = [%s]\n", strKey));
	}

	return strWorkingKey;
}

///////////////////////////////////////////////////
// CSTD3_TDL_DCC_LookUp

CSTD3_TDL_DCC_LookUp::CSTD3_TDL_DCC_LookUp()
{
	ClearData();
}

CSTD3_TDL_DCC_LookUp::~CSTD3_TDL_DCC_LookUp()
{
}

void CSTD3_TDL_DCC_LookUp::ClearData()
{
	m_strReq_PAN = _T("");
	m_strReq_LocalCurrency = _T("");
	m_strReq_RequestAmount = _T("");
	m_strReq_Language = _T("");
	ClearResponseData();
}

void CSTD3_TDL_DCC_LookUp::ClearResponseData()
{
	m_strRes_DCCEligible = _T("");
	m_strRes_SurchargeType = _T("");
	m_strRes_SurchargeAmount = _T("");
	m_strRes_DCCOfferType = _T("");
	m_strRes_DCCOfferConversionRate = _T("");
	m_strRes_DCCOfferHomeCurrencyCode = _T("");
	m_strRes_DCCOfferHomeCurrencyISO = _T("");
	m_strRes_DCCOfferQuotationType = _T("0");			//[#2302] US Justin 2014.11.05 Remove Quotation Type => Assign "0" Direct as default.
	//m_strRes_DCCDisclaimerType = _T("");				//[#2302] US Justin 2014.11.05 Remove DCC Disclaimer Type
	m_strRes_DCCDisclaimerScheme = _T("");				//[#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
	m_strRes_DCCConvertedTranAmount = _T("");			//[#2297] US Justin 2014.10.20 Add Converted Amount
	m_strRes_DCCConvertedSurcharge = _T("");			//[#2297] US Justin 2014.10.20 Add Converted Amount
	m_strRes_WorkingKey = _T("");
	m_strDisplayedExchangeRate = _T("");
	m_ivaTaxAmount = _T("");
	m_convertedIvaTaxAmount = _T("");
	m_surchargePlusIvaTaxAmount = _T("");
	m_convertedSurchargePlusIvaTaxAmount = _T("");
}

void CSTD3_TDL_DCC_LookUp::FillCurrencyISOWithCurrencyCode(LPCTSTR sCode)
{
	m_strRes_DCCOfferHomeCurrencyISO = _T("");
	int nCurrencyCode = Asc2Int(sCode);
	switch(nCurrencyCode)
	{
		case   4:      m_strRes_DCCOfferHomeCurrencyISO = L"AFA";            break;
		case   8:      m_strRes_DCCOfferHomeCurrencyISO = L"ALL";            break;
		case  12:      m_strRes_DCCOfferHomeCurrencyISO = L"DZD";            break;
		case  20:      m_strRes_DCCOfferHomeCurrencyISO = L"ADP";            break;
		case  24:      m_strRes_DCCOfferHomeCurrencyISO = L"AON";            break;
		case  31:      m_strRes_DCCOfferHomeCurrencyISO = L"AZM";            break;
		case  32:      m_strRes_DCCOfferHomeCurrencyISO = L"ARS";            break;
		case  36:      m_strRes_DCCOfferHomeCurrencyISO = L"AUD";            break;
		case  40:      m_strRes_DCCOfferHomeCurrencyISO = L"ATS";            break;
		case  44:      m_strRes_DCCOfferHomeCurrencyISO = L"BSD";            break;
		case  48:      m_strRes_DCCOfferHomeCurrencyISO = L"BHD";            break;
		case  50:      m_strRes_DCCOfferHomeCurrencyISO = L"BDT";            break;
		case  51:      m_strRes_DCCOfferHomeCurrencyISO = L"AMD";            break;
		case  52:      m_strRes_DCCOfferHomeCurrencyISO = L"BBD";            break;
		case  56:      m_strRes_DCCOfferHomeCurrencyISO = L"BEF";            break;
		case  60:      m_strRes_DCCOfferHomeCurrencyISO = L"BMD";            break;
		case  64:      m_strRes_DCCOfferHomeCurrencyISO = L"BTN";            break;
		case  72:      m_strRes_DCCOfferHomeCurrencyISO = L"BWP";            break;
		case  84:      m_strRes_DCCOfferHomeCurrencyISO = L"BZD";            break;
		case  90:      m_strRes_DCCOfferHomeCurrencyISO = L"SBD";            break;
		case  96:      m_strRes_DCCOfferHomeCurrencyISO = L"BND";            break;
		case 100:      m_strRes_DCCOfferHomeCurrencyISO = L"BGL";            break;
		case 104:      m_strRes_DCCOfferHomeCurrencyISO = L"MMK";            break;
		case 108:      m_strRes_DCCOfferHomeCurrencyISO = L"BIF";            break;
		case 116:      m_strRes_DCCOfferHomeCurrencyISO = L"KHR";            break;
		case 124:      m_strRes_DCCOfferHomeCurrencyISO = L"CAD";            break;
		case 132:      m_strRes_DCCOfferHomeCurrencyISO = L"CVE";            break;
		case 136:      m_strRes_DCCOfferHomeCurrencyISO = L"KYD";            break;
		case 144:      m_strRes_DCCOfferHomeCurrencyISO = L"LKR";            break;
		case 152:      m_strRes_DCCOfferHomeCurrencyISO = L"CLP";            break;
		case 156:      m_strRes_DCCOfferHomeCurrencyISO = L"CNY";            break;
		case 170:      m_strRes_DCCOfferHomeCurrencyISO = L"COP";            break;
		case 174:      m_strRes_DCCOfferHomeCurrencyISO = L"KMF";            break;
		case 180:      m_strRes_DCCOfferHomeCurrencyISO = L"ZRN";            break;
		case 188:      m_strRes_DCCOfferHomeCurrencyISO = L"CRC";            break;
		case 191:      m_strRes_DCCOfferHomeCurrencyISO = L"HRK";            break;
		case 192:      m_strRes_DCCOfferHomeCurrencyISO = L"CUP";            break;
		case 196:      m_strRes_DCCOfferHomeCurrencyISO = L"CYP";            break;
		case 203:      m_strRes_DCCOfferHomeCurrencyISO = L"CZK";            break;
		case 208:      m_strRes_DCCOfferHomeCurrencyISO = L"DKK";            break;
		case 214:      m_strRes_DCCOfferHomeCurrencyISO = L"DOP";            break;
		case 218:      m_strRes_DCCOfferHomeCurrencyISO = L"ECS";            break;
		case 222:      m_strRes_DCCOfferHomeCurrencyISO = L"SVC";            break;
		case 230:      m_strRes_DCCOfferHomeCurrencyISO = L"ETB";            break;
		case 232:      m_strRes_DCCOfferHomeCurrencyISO = L"ERN";            break;
		case 233:      m_strRes_DCCOfferHomeCurrencyISO = L"EEK";            break;
		case 238:      m_strRes_DCCOfferHomeCurrencyISO = L"FKP";            break;
		case 242:      m_strRes_DCCOfferHomeCurrencyISO = L"FJD";            break;
		case 246:      m_strRes_DCCOfferHomeCurrencyISO = L"FIM";            break;
		case 250:      m_strRes_DCCOfferHomeCurrencyISO = L"FRF";            break;
		case 262:      m_strRes_DCCOfferHomeCurrencyISO = L"DJF";            break;
		case 270:      m_strRes_DCCOfferHomeCurrencyISO = L"GMD";            break;
		case 276:      m_strRes_DCCOfferHomeCurrencyISO = L"DEM";            break;
		case 288:      m_strRes_DCCOfferHomeCurrencyISO = L"GHC";            break;
		case 292:      m_strRes_DCCOfferHomeCurrencyISO = L"GIP";            break;
		case 300:      m_strRes_DCCOfferHomeCurrencyISO = L"GRD";            break;
		case 320:      m_strRes_DCCOfferHomeCurrencyISO = L"GTQ";            break;
		case 324:      m_strRes_DCCOfferHomeCurrencyISO = L"GNF";            break;
		case 328:      m_strRes_DCCOfferHomeCurrencyISO = L"GYD";            break;
		case 332:      m_strRes_DCCOfferHomeCurrencyISO = L"HTG";            break;
		case 340:      m_strRes_DCCOfferHomeCurrencyISO = L"HNL";            break;
		case 344:      m_strRes_DCCOfferHomeCurrencyISO = L"HKD";            break;
		case 348:      m_strRes_DCCOfferHomeCurrencyISO = L"HUF";            break;
		case 352:      m_strRes_DCCOfferHomeCurrencyISO = L"ISK";            break;
		case 356:      m_strRes_DCCOfferHomeCurrencyISO = L"INR";            break;
		case 360:      m_strRes_DCCOfferHomeCurrencyISO = L"IDR";            break;
		case 364:      m_strRes_DCCOfferHomeCurrencyISO = L"IRR";            break;
		case 368:      m_strRes_DCCOfferHomeCurrencyISO = L"IQD";            break;
		case 372:      m_strRes_DCCOfferHomeCurrencyISO = L"IEP";            break;
		case 376:      m_strRes_DCCOfferHomeCurrencyISO = L"ILS";            break;
		case 380:      m_strRes_DCCOfferHomeCurrencyISO = L"ITL";            break;
		case 388:      m_strRes_DCCOfferHomeCurrencyISO = L"JMD";            break;
		case 392:      m_strRes_DCCOfferHomeCurrencyISO = L"JPY";            break;
		case 398:      m_strRes_DCCOfferHomeCurrencyISO = L"KZT";            break;
		case 400:      m_strRes_DCCOfferHomeCurrencyISO = L"JOD";            break;
		case 404:      m_strRes_DCCOfferHomeCurrencyISO = L"KES";            break;
		case 408:      m_strRes_DCCOfferHomeCurrencyISO = L"KPW";            break;
		case 410:      m_strRes_DCCOfferHomeCurrencyISO = L"KRW";            break;
		case 414:      m_strRes_DCCOfferHomeCurrencyISO = L"KWD";            break;
		case 417:      m_strRes_DCCOfferHomeCurrencyISO = L"KGS";            break;
		case 418:      m_strRes_DCCOfferHomeCurrencyISO = L"LAK";            break;
		case 422:      m_strRes_DCCOfferHomeCurrencyISO = L"LBP";            break;
		case 426:      m_strRes_DCCOfferHomeCurrencyISO = L"LSL";            break;
		case 428:      m_strRes_DCCOfferHomeCurrencyISO = L"LVL";            break;
		case 430:      m_strRes_DCCOfferHomeCurrencyISO = L"LRD";            break;
		case 434:      m_strRes_DCCOfferHomeCurrencyISO = L"LYD";            break;
		case 440:      m_strRes_DCCOfferHomeCurrencyISO = L"LTL";            break;
		case 442:      m_strRes_DCCOfferHomeCurrencyISO = L"LUF";            break;
		case 446:      m_strRes_DCCOfferHomeCurrencyISO = L"MOP";            break;
		case 450:      m_strRes_DCCOfferHomeCurrencyISO = L"MGF";            break;
		case 454:      m_strRes_DCCOfferHomeCurrencyISO = L"MWK";            break;
		case 458:      m_strRes_DCCOfferHomeCurrencyISO = L"MYR";            break;
		case 462:      m_strRes_DCCOfferHomeCurrencyISO = L"MVR";            break;
		case 470:      m_strRes_DCCOfferHomeCurrencyISO = L"MTL";            break;
		case 478:      m_strRes_DCCOfferHomeCurrencyISO = L"MRO";            break;
		case 480:      m_strRes_DCCOfferHomeCurrencyISO = L"MUR";            break;
		case 484:      m_strRes_DCCOfferHomeCurrencyISO = L"MXN";            break;
		case 496:      m_strRes_DCCOfferHomeCurrencyISO = L"MNT";            break;
		case 498:      m_strRes_DCCOfferHomeCurrencyISO = L"MDL";            break;
		case 504:      m_strRes_DCCOfferHomeCurrencyISO = L"MAD";            break;
		case 508:      m_strRes_DCCOfferHomeCurrencyISO = L"MZM";            break;
		case 512:      m_strRes_DCCOfferHomeCurrencyISO = L"OMR";            break;
		case 516:      m_strRes_DCCOfferHomeCurrencyISO = L"NAD";            break;
		case 524:      m_strRes_DCCOfferHomeCurrencyISO = L"NPR";            break;
		case 528:      m_strRes_DCCOfferHomeCurrencyISO = L"NLG";            break;
		case 532:      m_strRes_DCCOfferHomeCurrencyISO = L"ANG";            break;
		case 533:      m_strRes_DCCOfferHomeCurrencyISO = L"AWG";            break;
		case 548:      m_strRes_DCCOfferHomeCurrencyISO = L"VUV";            break;
		case 554:      m_strRes_DCCOfferHomeCurrencyISO = L"NZD";            break;
		case 558:      m_strRes_DCCOfferHomeCurrencyISO = L"NIO";            break;
		case 566:      m_strRes_DCCOfferHomeCurrencyISO = L"NGN";            break;
		case 578:      m_strRes_DCCOfferHomeCurrencyISO = L"NOK";            break;
		case 586:      m_strRes_DCCOfferHomeCurrencyISO = L"PKR";            break;
		case 590:      m_strRes_DCCOfferHomeCurrencyISO = L"PAB";            break;
		case 598:      m_strRes_DCCOfferHomeCurrencyISO = L"PGK";            break;
		case 600:      m_strRes_DCCOfferHomeCurrencyISO = L"PYG";            break;
		case 604:      m_strRes_DCCOfferHomeCurrencyISO = L"PEN";            break;
		case 608:      m_strRes_DCCOfferHomeCurrencyISO = L"PHP";            break;
		case 620:      m_strRes_DCCOfferHomeCurrencyISO = L"PTE";            break;
		case 624:      m_strRes_DCCOfferHomeCurrencyISO = L"GWP";            break;
		case 626:      m_strRes_DCCOfferHomeCurrencyISO = L"TPE";            break;
		case 634:      m_strRes_DCCOfferHomeCurrencyISO = L"QAR";            break;
		case 642:      m_strRes_DCCOfferHomeCurrencyISO = L"ROL";            break;
		case 643:      m_strRes_DCCOfferHomeCurrencyISO = L"RUB";            break;
		case 646:      m_strRes_DCCOfferHomeCurrencyISO = L"RWF";            break;
		case 654:      m_strRes_DCCOfferHomeCurrencyISO = L"SHP";            break;
		case 678:      m_strRes_DCCOfferHomeCurrencyISO = L"STD";            break;
		case 682:      m_strRes_DCCOfferHomeCurrencyISO = L"SAR";            break;
		case 690:      m_strRes_DCCOfferHomeCurrencyISO = L"SCR";            break;
		case 694:      m_strRes_DCCOfferHomeCurrencyISO = L"SLL";            break;
		case 702:      m_strRes_DCCOfferHomeCurrencyISO = L"SGD";            break;
		case 703:      m_strRes_DCCOfferHomeCurrencyISO = L"SKK";            break;
		case 704:      m_strRes_DCCOfferHomeCurrencyISO = L"VND";            break;
		case 705:      m_strRes_DCCOfferHomeCurrencyISO = L"SIT";            break;
		case 706:      m_strRes_DCCOfferHomeCurrencyISO = L"SOS";            break;
		case 710:      m_strRes_DCCOfferHomeCurrencyISO = L"ZAR";            break;
		case 716:      m_strRes_DCCOfferHomeCurrencyISO = L"ZWD";            break;
		case 724:      m_strRes_DCCOfferHomeCurrencyISO = L"ESP";            break;
		case 736:      m_strRes_DCCOfferHomeCurrencyISO = L"SDD";            break;
		case 740:      m_strRes_DCCOfferHomeCurrencyISO = L"SRG";            break;
		case 748:      m_strRes_DCCOfferHomeCurrencyISO = L"SZL";            break;
		case 752:      m_strRes_DCCOfferHomeCurrencyISO = L"SEK";            break;
		case 756:      m_strRes_DCCOfferHomeCurrencyISO = L"CHF";            break;
		case 760:      m_strRes_DCCOfferHomeCurrencyISO = L"SYP";            break;
		case 762:      m_strRes_DCCOfferHomeCurrencyISO = L"TJR";            break;
		case 764:      m_strRes_DCCOfferHomeCurrencyISO = L"THB";            break;
		case 776:      m_strRes_DCCOfferHomeCurrencyISO = L"TOP";            break;
		case 780:      m_strRes_DCCOfferHomeCurrencyISO = L"TTD";            break;
		case 784:      m_strRes_DCCOfferHomeCurrencyISO = L"AED";            break;
		case 788:      m_strRes_DCCOfferHomeCurrencyISO = L"TND";            break;
		case 792:      m_strRes_DCCOfferHomeCurrencyISO = L"TRL";            break;
		case 795:      m_strRes_DCCOfferHomeCurrencyISO = L"TMM";            break;
		case 800:      m_strRes_DCCOfferHomeCurrencyISO = L"UGX";            break;
		case 807:      m_strRes_DCCOfferHomeCurrencyISO = L"MKD";            break;
		case 810:      m_strRes_DCCOfferHomeCurrencyISO = L"RUR";            break;
		case 818:      m_strRes_DCCOfferHomeCurrencyISO = L"EGP";            break;
		case 826:      m_strRes_DCCOfferHomeCurrencyISO = L"GBP";            break;
		case 834:      m_strRes_DCCOfferHomeCurrencyISO = L"TZS";            break;
		case 840:      m_strRes_DCCOfferHomeCurrencyISO = L"USD";            break;
		case 858:      m_strRes_DCCOfferHomeCurrencyISO = L"UYU";            break;
		case 860:      m_strRes_DCCOfferHomeCurrencyISO = L"UZS";            break;
		case 862:      m_strRes_DCCOfferHomeCurrencyISO = L"VEB";            break;
		case 882:      m_strRes_DCCOfferHomeCurrencyISO = L"WST";            break;
		case 886:      m_strRes_DCCOfferHomeCurrencyISO = L"YER";            break;
		case 891:      m_strRes_DCCOfferHomeCurrencyISO = L"YUM";            break;
		case 894:      m_strRes_DCCOfferHomeCurrencyISO = L"ZMK";            break;
		case 901:      m_strRes_DCCOfferHomeCurrencyISO = L"TWD";            break;
		case 950:      m_strRes_DCCOfferHomeCurrencyISO = L"XAF";            break;
		case 951:      m_strRes_DCCOfferHomeCurrencyISO = L"XCD";            break;
		case 952:      m_strRes_DCCOfferHomeCurrencyISO = L"XOF";            break;
		case 953:      m_strRes_DCCOfferHomeCurrencyISO = L"XPF";            break;
		case 955:      m_strRes_DCCOfferHomeCurrencyISO = L"XBA";            break;
		case 956:      m_strRes_DCCOfferHomeCurrencyISO = L"XBB";            break;
		case 957:      m_strRes_DCCOfferHomeCurrencyISO = L"XBC";            break;
		case 958:      m_strRes_DCCOfferHomeCurrencyISO = L"XBD";            break;
		case 959:      m_strRes_DCCOfferHomeCurrencyISO = L"XAU";            break;
		case 960:      m_strRes_DCCOfferHomeCurrencyISO = L"XDR";            break;
		case 961:      m_strRes_DCCOfferHomeCurrencyISO = L"XAG";            break;
		case 962:      m_strRes_DCCOfferHomeCurrencyISO = L"XPT";            break;
		case 963:      m_strRes_DCCOfferHomeCurrencyISO = L"XTS";            break;
		case 964:      m_strRes_DCCOfferHomeCurrencyISO = L"XPD";            break;
		case 972:      m_strRes_DCCOfferHomeCurrencyISO = L"TJS";            break;
		case 974:      m_strRes_DCCOfferHomeCurrencyISO = L"BYR";            break;
		case 975:      m_strRes_DCCOfferHomeCurrencyISO = L"BGN";            break;
		case 976:      m_strRes_DCCOfferHomeCurrencyISO = L"CDF";            break;
		case 977:      m_strRes_DCCOfferHomeCurrencyISO = L"BAM";            break;
		case 978:      m_strRes_DCCOfferHomeCurrencyISO = L"EUR";            break;
		case 979:      m_strRes_DCCOfferHomeCurrencyISO = L"MXV";            break;
		case 980:      m_strRes_DCCOfferHomeCurrencyISO = L"UAH";            break;
		case 981:      m_strRes_DCCOfferHomeCurrencyISO = L"GEL";            break;
		case 982:      m_strRes_DCCOfferHomeCurrencyISO = L"AOR";            break;
		case 983:      m_strRes_DCCOfferHomeCurrencyISO = L"ECV";            break;
		case 985:      m_strRes_DCCOfferHomeCurrencyISO = L"PLN";            break;
		case 986:      m_strRes_DCCOfferHomeCurrencyISO = L"BRL";            break;
		case 990:      m_strRes_DCCOfferHomeCurrencyISO = L"CLF";            break;
		case 991:      m_strRes_DCCOfferHomeCurrencyISO = L"ZAL";            break;
		case 997:      m_strRes_DCCOfferHomeCurrencyISO = L"USN";            break;
		case 998:      m_strRes_DCCOfferHomeCurrencyISO = L"USS";            break;
		case 999:      m_strRes_DCCOfferHomeCurrencyISO = L"XXX";            break;
	}
}

// [#2348] US Justin 2015.06.02 Get Decimal Point
int	CSTD3_TDL_DCC_LookUp::GetCurrencyDecimalNumber(LPCTSTR sCode)
{
	int nRtn = 2;

	int nCurrencyCode = Asc2Int(sCode);
	switch(nCurrencyCode)
	{
		case 108:   nRtn=0;   break;  // BIF - BURUNDI
		case 152:   nRtn=0;   break;  // CLP - CHILE
		case 174:   nRtn=0;   break;  // KMF - COMOROS
		case 262:   nRtn=0;   break;  // DJF - DJIBOUTI
		case 324:   nRtn=0;   break;  // GNF - GUINEA
		case 352:   nRtn=0;   break;  // ISK - ICELAND
		case 392:   nRtn=0;   break;  // JPY - JAPAN
		case 410:   nRtn=0;   break;  // KRW - KOREA, REPUBLIC OF
		case 548:   nRtn=0;   break;  // VUV - VANUATU
		case 600:   nRtn=0;   break;  // PYG - PARAGUAY
		case 646:   nRtn=0;   break;  // RWF - RWANDA
		case 704:   nRtn=0;   break;  // VND - VIET NAM
		case 800:   nRtn=0;   break;  // UGX - UGANDA
		case 940:   nRtn=0;   break;  // UYI - URUGUAY
		case 950:   nRtn=0;   break;  // XAF - CAMEROON
		case 952:   nRtn=0;   break;  // XOF - BENIN
		case 953:   nRtn=0;   break;  // XPF - FRENCH POLYNESIA
		case 974:   nRtn=0;   break;  // BYR - BELARUS		
		case 48:	nRtn=3;   break;  // BHD - BAHRAIN
		case 368:   nRtn=3;   break;  // IQD - IRAQ
		case 400:   nRtn=3;   break;  // JOD - JORDAN
		case 414:   nRtn=3;   break;  // KWD - KUWAIT
		case 434:   nRtn=3;   break;  // LYD - LIBYA
		case 512:   nRtn=3;   break;  // OMR - OMAN
		case 788:   nRtn=3;   break;  // TND - TUNISIA
		case 990:   nRtn=4;   break;  // CLF - CHILE
		default:	nRtn=2;	  break;
	}
	return nRtn;
}
// End of [#2348]

int CSTD3_TDL_DCC_LookUp::MakeRequestMessage(BYTE* pTargetBuffer)
{
	ClearResponseData();

	//////////////////////////////////////////// Temporary DATA BUFFER
	BYTE szDATA[_TDL_REQUEST_BUFFER_SIZE];
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	int nDataSz = 0;

	///////////////// PAN /////////////
	// TAG DF 88 03
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X03;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_PAN);

	///////////////// Local Currency Code /////////////
	// TAG DF A8 5C
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA8;	szDATA[nDataSz++] = 0X5C;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_LocalCurrency);

	///////////////// Requested Amount /////////////
	// TAG DF 87 70
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X87;	szDATA[nDataSz++] = 0X70;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_RequestAmount);

	///////////////// Language /////////////
	// TAG DF 88 00
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X00;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_Language);

	/////////////////////////////////////////////////////////////////////////////////
	// FINAL LOOK UP REQUEST FF 87 6F
	int nDataLen = 0;
	pTargetBuffer[nDataLen++] = 0XFF;	pTargetBuffer[nDataLen++] = 0X87;	pTargetBuffer[nDataLen++] = 0X6F;
	nDataLen += TLV_FillValueBuffer(&pTargetBuffer[nDataLen], szDATA, nDataSz);

	return nDataLen;
}

BOOL CSTD3_TDL_DCC_LookUp::ParseDCCOfferMessage(BYTE* pTargetBuffer, int nBufSize)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DCC_LookUp::ParseDCCOfferMessage\n"));
	int nBufLoc, nTagID, nTagType, nDataLength;
	
	nBufLoc = 0;
	BOOL bEndOfData = FALSE;
	do
	{
		nTagID = TLV_GetTagIDandType(&pTargetBuffer[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )			// Incorrect Tag ID
			return FALSE;
		else
		{
			nDataLength = TLV_GetDataLength(&pTargetBuffer[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nBufSize )		// Size Error
				return FALSE;
			else
			{
				if( (nBufLoc + nDataLength) == nBufSize )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 1011:					// DF 87 73 : DCC Offer Type
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCOfferType, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 87 73 : DCC OFFER = [%s]\n", m_strRes_DCCOfferType));
						break;

					case 5248:					// DF A9 00 : DCC Conversion Rate
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCOfferConversionRate, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF A9 00 : DCC Rate = [%s]\n", m_strRes_DCCOfferConversionRate));
						break;

					//[#2297] US Justin 2014.10.20 Add Converted Amount
					case 1013:					// DF 87 75 : DCC Converted Surcharge Amount
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCConvertedSurcharge, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 87 75 : DCC converted Surcharge = [%s]\n", m_strRes_DCCConvertedSurcharge));
						break;

					case 1014:					// DF 87 76 : DCC Converted Tran Amount
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCConvertedTranAmount, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 87 76 : DCC converted Amount = [%s]\n", m_strRes_DCCConvertedTranAmount));
						break;
					// End of [#2297]

					case 5212:					// DF A8 5C : Currency Code
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCOfferHomeCurrencyCode, nDataLength);
						FillCurrencyISOWithCurrencyCode(m_strRes_DCCOfferHomeCurrencyCode);
						NHDEBUG(DBG_INFO, (L"DF A8 5C : Currency CODE = [%s], Alphabetic CODE = [%s]\n", m_strRes_DCCOfferHomeCurrencyCode, m_strRes_DCCOfferHomeCurrencyISO));
						break;

					// [#2302] US Justin 2014.11.05 Remove Quotation Type
					/*
					case 5249:					// DF A9 01 : Quotation Type
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCOfferQuotationType, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 87 76 : Quotation Type = [%s]\n", m_strRes_DCCOfferQuotationType));
						break;
					*/
					// End of [#2302]

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						NHDEBUG(DBG_INFO, (L"UNDEVINED TAG (%d) received\n", nTagID));
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	return TRUE;
}

BOOL CSTD3_TDL_DCC_LookUp::ParseResponseMessage(BYTE* pTargetBuffer, int nBufSize)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DCC_LookUp::ParseResponseMessage\n"));

	ClearResponseData();

	int nBufLoc, nTagID, nTagType, nDataLength;
	nBufLoc = 0;
	BOOL bEndOfData = FALSE;
	do
	{
		nTagID = TLV_GetTagIDandType(&pTargetBuffer[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )			// Incorrect Tag ID
			return FALSE;
		else
		{
			nDataLength = TLV_GetDataLength(&pTargetBuffer[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nBufSize )		// Size Error
				return FALSE;
			else
			{
				if( (nBufLoc + nDataLength) == nBufSize )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 1006:					// DF 87 6E : DCC Eligibility (0 or 1)
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCEligible, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 87 6E : EDD Eligibility = [%s]\n", m_strRes_DCCEligible));
						break;

					case 1029:					// DF 88 05 : Surcharge Type
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_SurchargeType, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 88 05 : Surcharge Type = [%s]\n", m_strRes_SurchargeType));
						break;

					case 114:					// DF 72    : Surcharge Amount
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_SurchargeAmount, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 72 : Surcharge Amount = [%s]\n", m_strRes_SurchargeAmount));
						break;

					// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
					// Disclaimer Scheme = Disclaimer Type, need this to determine if it is VISA, MasterCard, or None
					case 1030:					// DF 88 06 : DCC Disclaimer Scheme 0:None, 1:VISA, 2:MasterCard
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCDisclaimerScheme, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 88 06 : DCC Disclaimer Scheme = [%s]\n", m_strRes_DCCDisclaimerScheme));
						break;
					// end of [#RWC6-2, #2585]


					// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
					case 1045:					// DF 88 15 : Markup Rate
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCMarkupRate, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 88 15 : Markup Rate = [%s]\n", m_strRes_DCCMarkupRate));
						break;
					// end of [#RWC6-2, #2585]

					// [#2302] US Justin 2014.11.05 Remove DCC Disclaimer Type
					/*
					case 1030:					// DF 88 06 : DCC Disclaimer Type
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_DCCDisclaimerType, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 88 06 : DCC Disclaimer Type = [%s]\n", m_strRes_DCCDisclaimerType));
						break;
					*/
					// End of [#2302]

					case 1010:					// FF 87 72 : DCC Offer
						if( !ParseDCCOfferMessage(&pTargetBuffer[nBufLoc], nDataLength) )
						{
							NHDEBUG(DBG_INFO, (L" DCC OFFER Message ERROR\n"));
							return FALSE;
						}
						nBufLoc += nDataLength;
						break;

					case 137:					// FF 81 09  : Working Key
						NHDEBUG(DBG_INFO, (L"FF 81 09 : Working Tag received\n"));
						m_strRes_WorkingKey = GetWorkingKey( &pTargetBuffer[nBufLoc], nDataLength);
						nBufLoc += nDataLength;
						break;

					case 161:					// DF 81 21 : IVA Tax for Withdrawal
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_ivaTaxAmount, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 81 2 : IVA Tax Amount = [%s]\n", m_ivaTaxAmount));
						break;

					case 1048:					// DF 88 18 : Converted IVA Tax for Withdrawal
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_convertedIvaTaxAmount, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 88 18 : Converted IVA Tax for Withdrawal = [%s]\n", m_convertedIvaTaxAmount));
						break;

					case 162:					// DF 81 22 : Surcharge + IVA Tax for Withdrawal
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_surchargePlusIvaTaxAmount, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 81 22 : Surcharge + IVA Tax for Withdrawal = [%s]\n", m_surchargePlusIvaTaxAmount));
						break;

					case 1049:					// DF 88 19 : Converted Surcharge + IVA Tax for Withdrawal
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_convertedSurchargePlusIvaTaxAmount, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 88 19 : Converted Surcharge + IVA Tax for Withdrawal = [%s]\n", m_convertedSurchargePlusIvaTaxAmount));
						break;

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	// [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message
	// Cardtronics Special requirement,  From Anny at 04/15/2019
	/*
	// Validate Data
	// Check DCC Offer Data
	if( m_strRes_DCCEligible == L"1") 
	{
		if( m_strRes_DCCOfferConversionRate.IsEmpty() || m_strRes_DCCOfferHomeCurrencyCode.IsEmpty() || m_strRes_DCCOfferHomeCurrencyISO.IsEmpty() ) 
		//  m_strRes_DCCOfferQuotationType.IsEmpty()  || m_strRes_DCCDisclaimerType.IsEmpty() )		// [#2302] US Justin 2014.11.05 Remove Qutotation Type and Dcc Discalimer Type
		{
			NHDEBUG(DBG_INFO, (L"LOOK UP DATA ERROR - DCC is Eligible but not enough data\n"));
			return FALSE;
		}
	}
	*/

	// Check Surcharge Type
	if( (m_strRes_SurchargeType == L"3") && (m_strRes_SurchargeAmount.IsEmpty()) )
	{
		// Cardtronics Special requirement,  From Anny at 04/15/2019
		//NHDEBUG(DBG_INFO, (L"LOOK UP DATA ERROR - Surcharge Type is [3](host supplied) but no surcharge amount is provided\n"));
		//return FALSE;
		NHDEBUG(DBG_INFO, (L"LOOK UP DATA ERROR - Surcharge Type is [3](host supplied) but no surcharge amount is provided.\n"));
		m_strRes_DCCEligible = L"0"; // proceed as non-DCC

		if ( (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT).GetLength() == 0) || (Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT)) <= 0) )
		{	
			NHDEBUG(DBG_INFO, (L"Trying to use the local surcharge but since it is either NOT-assigned or less than or equal to zero, forced to cancel the transaction\n"));
			return FALSE;
		}
		else
		{
			NHDEBUG(DBG_INFO, (L"Forcing to use the local surcharge\n"));
			CString sLocalSurcharge = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_SURCHAREAMT);
			m_strRes_SurchargeAmount.Format(L"%0.2f", (float)(Asc2Int(sLocalSurcharge) * 0.01)); 
		}
		// end of [#RWC6-2, #2585]
	}

	// [#2301] US Justin 2014.10.31 Cardtronics Special requirement,  From Laura at 10/28/2014
	// If an ATM does not get converted amounts(converted transactionamount and surchargeamount), ATM should handle as non DCC transaction
	// Validate Data converted amounts for DCC
	if( m_strRes_DCCEligible == L"1" ) 
	{
		//if( m_strRes_DCCConvertedTranAmount.IsEmpty() || m_strRes_DCCConvertedSurcharge.IsEmpty() )
		if( m_strRes_DCCConvertedTranAmount.IsEmpty() || m_strRes_DCCConvertedSurcharge.IsEmpty() || m_strRes_DCCOfferConversionRate.IsEmpty() || 
			m_strRes_DCCDisclaimerScheme.IsEmpty() || m_strRes_DCCOfferHomeCurrencyCode.IsEmpty() || m_strRes_DCCOfferHomeCurrencyISO.IsEmpty() ) // [#RWC6-2, #2585] US Brandon 2019.02.11 Support VISA DCC for STD3 message // // Cardtronics Special requirement,  From Anny at 04/15/2019
			m_strRes_DCCEligible = L"0";
	}
	// End of [#2301]

	return TRUE;
}

///////////////////////////////////////////////////
// CSTD3_TDL_DCC_Transaction

CSTD3_TDL_DCC_Transaction::CSTD3_TDL_DCC_Transaction()
{
	ClearData();
}

CSTD3_TDL_DCC_Transaction::~CSTD3_TDL_DCC_Transaction()
{
}

void CSTD3_TDL_DCC_Transaction::ClearData()
{
	m_strReq_Account = _T("");
	m_strReq_OfferAcceptance = _T("");
	m_strReq_RequestAmount = _T("");
	m_strReq_LocalCurrency = _T("");
	m_strReq_SurchargeAmount = _T("");
	m_strReq_ConvertedAmount = _T("");
	m_strReq_HomeCurrencyCode = _T("");
	m_strReq_ConvertedSurcharge = _T("");
	m_strReq_PresentedAmount = _T("");
	m_strReq_Track2 = _T("");
	m_strReq_PINBlock = _T("");
	m_strReq_EMVData = _T("");
	m_strReq_ivaTaxAmount = _T("");
	m_strReq_convertedIvaTaxAmount = _T("");
	m_strReq_surchargePlusIvaTaxAmount = _T("");
	m_strReq_convertedSurchargePlusIvaTaxAmount = _T("");

	ClearResponseData();
}

void CSTD3_TDL_DCC_Transaction::ClearResponseData()
{
	m_strRes_TranDateTime = _T("");
	m_strRes_AuthCode = _T("");
	m_strRes_AuthNumber = _T("");
	m_strRes_BusDate = _T("");
	m_strRes_EMVData = _T("");
	m_strRes_WorkingKey = _T("");
}

int CSTD3_TDL_DCC_Transaction::MakeRequestMessage(BYTE* pTargetBuffer)
{
	ClearResponseData();

	//////////////////////////////////////////// Temporary DATA BUFFER
	BYTE szDATA[_TDL_REQUEST_BUFFER_SIZE];
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	int nDataSz = 0;

	///////////////// Source Account /////////////
	// TAG DF 66
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X66;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_Account);

	///////////////// DCC Offer Acceptance Flag /////////////
	// TAG DF 88 09 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X09;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_OfferAcceptance);

	///////////////// Requested Amount /////////////
	// TAG DF 87 70 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X87;	szDATA[nDataSz++] = 0X70;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_RequestAmount);

	///////////////// Local Currency Code /////////////
	// TAG DF A8 5C 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA8;	szDATA[nDataSz++] = 0X5C;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_LocalCurrency);

	///////////////// Surcharge Amount /////////////
	// TAG DF 72 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X72;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_SurchargeAmount);

	///////////////// Converted Amount /////////////
	// TAG DF 87 76 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X87;	szDATA[nDataSz++] = 0X76;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_ConvertedAmount);

	///////////////// Home Currency Code /////////////
	// TAG DF 87 74 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X87;	szDATA[nDataSz++] = 0X74;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_HomeCurrencyCode);

	///////////////// Converted Surcharge Amount /////////////
	// TAG DF 87 75
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X87;	szDATA[nDataSz++] = 0X75;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_ConvertedSurcharge);

	if (!m_strReq_ivaTaxAmount.IsEmpty())
	{
		///////////////// IVA Tax Amount /////////////
		// TAG DF 81 21
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X81;	szDATA[nDataSz++] = 0X21;
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_ivaTaxAmount);
	}

	if (!m_strReq_convertedIvaTaxAmount.IsEmpty())
	{
		///////////////// Converted IVA Tax Amount /////////////
		// TAG DF 88 18
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X18;
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_convertedIvaTaxAmount);
	}

	if (!m_strReq_surchargePlusIvaTaxAmount.IsEmpty())
	{
		///////////////// IVA Tax Amount + Surcharge /////////////
		// TAG DF 81 22
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X81;	szDATA[nDataSz++] = 0X22;
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_surchargePlusIvaTaxAmount);
	}

	if (!m_strReq_convertedSurchargePlusIvaTaxAmount.IsEmpty())
	{
		///////////////// Converted IVA Tax Amount + Surcharge /////////////
		// TAG DF 88 19
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X19;
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_convertedSurchargePlusIvaTaxAmount);
	}

	///////////////// Presented Amount /////////////
	// TAG DF 88 0A
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X0A;
	// [#2348] US Justin 2015.06.12 fix decimal point 
	//nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_PresentedAmount);
	CString strModifiedAmt = m_strReq_PresentedAmount;
	int nDecimalPt = strModifiedAmt.Find(L".");
	if(nDecimalPt<0)
		strModifiedAmt += L".0";
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], strModifiedAmt);
	// End of [#2348]

	///////////////// Track Data Item /////////////
	{
		// Temporary DATA BUFFER2
		BYTE szDATA2[_TDL_REQUEST_BUFFER_SIZE];
		memset(szDATA2, NULL, _TDL_REQUEST_BUFFER_SIZE);
		int nDataSz2 = 0;

		///////////////// Track Number /////////////
		// TAG DF 6B
		szDATA2[nDataSz2++] = 0XDF;	szDATA2[nDataSz2++] = 0X6B;
		CString strTemp = _T("2");
		nDataSz2 += TLV_FillValueString(&szDATA2[nDataSz2], strTemp);

		///////////////// Track Data /////////////
		// TAG DF 6C
		szDATA2[nDataSz2++] = 0XDF;	szDATA2[nDataSz2++] = 0X6C;
		nDataSz2 += TLV_FillValueString(&szDATA2[nDataSz2], m_strReq_Track2);

		///////////////// Track Data Group /////////////
		// Tag FF 69
		szDATA[nDataSz++] = 0XFF;	szDATA[nDataSz++] = 0X69;
		nDataSz += TLV_FillLength(&szDATA[nDataSz], nDataSz2 + 3);		// For FF 6A and Length(1, Length of Track2 is Maximum 37)

		///////////////// Track Data Item /////////////
		// Tag FF 6A
		szDATA[nDataSz++] = 0XFF;	szDATA[nDataSz++] = 0X6A;
		nDataSz += TLV_FillValueBuffer(&szDATA[nDataSz], szDATA2, nDataSz2);
	}

	///////////////// PIN BLOCK /////////////
	// TAG DF 6D
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X6D;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_PINBlock);

	///////////////// EMV Data /////////////
	if(m_strReq_EMVData.GetLength() >= 8 )		// Minimum length (fallback)
	{
		// TAG DF 81 14
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X81;	szDATA[nDataSz++] = 0X14;
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_EMVData);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// FINAL TRANSACTION REQUEST FF 88 07
	int nDataLen = 0;
	pTargetBuffer[nDataLen++] = 0XFF;	pTargetBuffer[nDataLen++] = 0X88;	pTargetBuffer[nDataLen++] = 0X07;
	nDataLen += TLV_FillValueBuffer(&pTargetBuffer[nDataLen], szDATA, nDataSz);

	return nDataLen;
}

BOOL CSTD3_TDL_DCC_Transaction::ParseResponseMessage(BYTE* pTargetBuffer, int nBufSize)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DCC_Transaction::ParseResponseMessage\n"));

	ClearResponseData();

	int nBufLoc, nTagID, nTagType, nDataLength;
	nBufLoc = 0;
	BOOL bEndOfData = FALSE;
	do
	{
		nTagID = TLV_GetTagIDandType(&pTargetBuffer[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )			// Incorrect Tag ID
			return FALSE;
		else
		{
			nDataLength = TLV_GetDataLength(&pTargetBuffer[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nBufSize )		// Size Error
				return FALSE;
			else
			{
				if( (nBufLoc + nDataLength) == nBufSize )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 112:					// DF 70    : Authorization Code 
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_AuthCode, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 70 : Authorization Code = [%s]\n", m_strRes_AuthCode));
						break;

					case 117:					// DF 75    : Authorization Number 
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_AuthNumber, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 75 : Authorization Number = [%s]\n", m_strRes_AuthNumber));
						break;

					case 144:					// DF 81 10 : Business Date 
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_BusDate, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 81 10 : Business Date = [%s]\n", m_strRes_BusDate));
						break;

					case 148:					// DF 81 14 : EMV Data 
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_EMVData, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 81 14 : EMV Data = [%s]\n", m_strRes_EMVData));
						break;

					case 137:					// FF 81 09  : Working Key
						NHDEBUG(DBG_INFO, (L"FF 81 09 : Working Tag received\n"));
						m_strRes_WorkingKey = GetWorkingKey( &pTargetBuffer[nBufLoc], nDataLength);
						nBufLoc += nDataLength;
						break;

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	// Validate Data
	// Check Auth Code
	if( m_strRes_AuthCode.IsEmpty()||m_strRes_AuthNumber.IsEmpty() ) 
	{
		NHDEBUG(DBG_INFO, (L"No AuthorCode or Auth Number is Empty\n"));
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////
// CSTD3_TDL_DCC_Reversal

CSTD3_TDL_DCC_Reversal::CSTD3_TDL_DCC_Reversal()
{
	ClearData();
}

CSTD3_TDL_DCC_Reversal::~CSTD3_TDL_DCC_Reversal()
{
}

void CSTD3_TDL_DCC_Reversal::ClearData()
{
	m_strReq_ReversalReason = _T("");
	m_strReq_OfferAcceptance = _T("");
	m_strReq_RequestAmount = _T("");
	m_strReq_DispensedAmount = _T("");
	m_strReq_LocalCurrency = _T("");
	m_strReq_SurchargeAmount = _T("");
	m_strReq_PresentedAmount = _T("");
	m_strReq_Track2 = _T("");
	m_strReq_EMVData = _T("");
	m_strReq_ivaTaxAmount = _T("");

	ClearResponseData();
}

void CSTD3_TDL_DCC_Reversal::ClearResponseData()
{
	m_strRes_AuthCode = _T("");
	m_strRes_WorkingKey = _T("");
}

int CSTD3_TDL_DCC_Reversal::MakeRequestMessage(BYTE* pTargetBuffer)
{
	ClearResponseData();

	//////////////////////////////////////////// Temporary DATA BUFFER
	BYTE szDATA[_TDL_REQUEST_BUFFER_SIZE];
	memset(szDATA, NULL, _TDL_REQUEST_BUFFER_SIZE);
	int nDataSz = 0;

	///////////////// Reversal Reason /////////////
	// TAG DF 79
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X79;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_ReversalReason);

	///////////////// DCC Offer Acceptance Flag /////////////
	// TAG DF 88 09 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X09;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_OfferAcceptance);

	///////////////// Requested Amount /////////////
	// TAG DF 87 70 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X87;	szDATA[nDataSz++] = 0X70;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_RequestAmount);

	///////////////// Dispensed Amount /////////////
	// TAG DF 88 14 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X14;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_DispensedAmount);

	///////////////// Local Currency Code /////////////
	// TAG DF A8 5C 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0XA8;	szDATA[nDataSz++] = 0X5C;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_LocalCurrency);

	///////////////// Surcharge Amount /////////////
	// TAG DF 72 
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X72;
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_SurchargeAmount);

	///////////////// Surcharge Amount /////////////
	// TAG DF 72 
	if (!m_strReq_ivaTaxAmount.IsEmpty())
	{
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X81;	szDATA[nDataSz++] = 0X21;
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_ivaTaxAmount);
	}

	///////////////// Presented Amount /////////////
	// TAG DF 88 0A
	szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X88;	szDATA[nDataSz++] = 0X0A;
	// [#2348] US Justin 2015.06.12 fix decimal point 
	//nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_PresentedAmount);
	CString strModifiedAmt = m_strReq_PresentedAmount;
	int nDecimalPt = strModifiedAmt.Find(L".");
	if(nDecimalPt<0)
		strModifiedAmt += L".0";
	nDataSz += TLV_FillValueString(&szDATA[nDataSz], strModifiedAmt);
	// End of [#2348]

	///////////////// Track Data Item /////////////
	{
		// Temporary DATA BUFFER2
		BYTE szDATA2[_TDL_REQUEST_BUFFER_SIZE];
		memset(szDATA2, NULL, _TDL_REQUEST_BUFFER_SIZE);
		int nDataSz2 = 0;

		///////////////// Track Number /////////////
		// TAG DF 6B
		szDATA2[nDataSz2++] = 0XDF;	szDATA2[nDataSz2++] = 0X6B;
		CString strTemp = _T("2");
		nDataSz2 += TLV_FillValueString(&szDATA2[nDataSz2], strTemp);

		///////////////// Track Data /////////////
		// TAG DF 6C
		szDATA2[nDataSz2++] = 0XDF;	szDATA2[nDataSz2++] = 0X6C;
		nDataSz2 += TLV_FillValueString(&szDATA2[nDataSz2], m_strReq_Track2);

		///////////////// Track Data Group /////////////
		// Tag FF 69
		szDATA[nDataSz++] = 0XFF;	szDATA[nDataSz++] = 0X69;
		nDataSz += TLV_FillLength(&szDATA[nDataSz], nDataSz2 + 3);		// For FF 6A and Length(1, Length of Track2 is Maximum 37)

		///////////////// Track Data Item /////////////
		// Tag FF 6A
		szDATA[nDataSz++] = 0XFF;	szDATA[nDataSz++] = 0X6A;
		nDataSz += TLV_FillValueBuffer(&szDATA[nDataSz], szDATA2, nDataSz2);
	}

	///////////////// EMV Data /////////////
	if(m_strReq_EMVData.GetLength() >=8 )		// Minimum length (fallback)
	{
		// TAG DF 81 14
		szDATA[nDataSz++] = 0XDF;	szDATA[nDataSz++] = 0X81;	szDATA[nDataSz++] = 0X14;
		nDataSz += TLV_FillValueString(&szDATA[nDataSz], m_strReq_EMVData);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// FINAL REVERSAL REQUEST FF 88 16
	int nDataLen = 0;
	pTargetBuffer[nDataLen++] = 0XFF;	pTargetBuffer[nDataLen++] = 0X88;	pTargetBuffer[nDataLen++] = 0X16;
	nDataLen += TLV_FillValueBuffer(&pTargetBuffer[nDataLen], szDATA, nDataSz);

	return nDataLen;
}

BOOL CSTD3_TDL_DCC_Reversal::ParseResponseMessage(BYTE* pTargetBuffer, int nBufSize)
{
	NHDEBUG(DBG_INFO, (L"CSTD3_TDL_DCC_Reversal::ParseResponseMessage\n"));

	ClearResponseData();

	int nBufLoc, nTagID, nTagType, nDataLength;
	nBufLoc = 0;
	BOOL bEndOfData = FALSE;
	do
	{
		nTagID = TLV_GetTagIDandType(&pTargetBuffer[nBufLoc], &nTagType, &nBufLoc);
		if( nTagID==0 )			// Incorrect Tag ID
			return FALSE;
		else
		{
			nDataLength = TLV_GetDataLength(&pTargetBuffer[nBufLoc], &nBufLoc);
			if( (nBufLoc + nDataLength) > nBufSize )		// Size Error
				return FALSE;
			else
			{
				if( (nBufLoc + nDataLength) == nBufSize )
					bEndOfData = TRUE;

				NHDEBUG(DBG_INFO, (L"Get Tag ID =[%d] and Data Length = [%d]\n", nTagID, nDataLength));

				switch (nTagID)
				{
					case 112:					// DF 70    : Authorization Code 
						nBufLoc += TLV_GetDataString(&pTargetBuffer[nBufLoc], &m_strRes_AuthCode, nDataLength);
						NHDEBUG(DBG_INFO, (L"DF 70: Reversal Authorization Code = [%s]\n", m_strRes_AuthCode));
						break;

					case 137:					// FF 81 09  : Working Key
						NHDEBUG(DBG_INFO, (L"FF 81 09 : Working Tag received\n"));
						m_strRes_WorkingKey = GetWorkingKey( &pTargetBuffer[nBufLoc], nDataLength);
						nBufLoc += nDataLength;
						break;

					default:
						nBufLoc += nDataLength;	// Skip Undefined Tag
						break;

				}
			}
		}
	} while (bEndOfData==FALSE);

	// Validate Data
	// Check Auth Code
	if( m_strRes_AuthCode.IsEmpty() ) 
	{
		NHDEBUG(DBG_INFO, (L"No AuthorCode receied\n"));
		return FALSE;
	}
	return TRUE;
}


CDualBalanceInquiry::CDualBalanceInquiry()
{
	ClearSetting();
	ClearTranData();
}

CDualBalanceInquiry::~CDualBalanceInquiry()
{
}

void CDualBalanceInquiry::ClearSetting()
{
	m_bDualBalance = FALSE;
	m_nDisplaceDevice = _DUAL_BALANCE_DISPLAY_NOTSET;
}

void CDualBalanceInquiry::ClearTranData()
{
	m_strAuthorizationNumber = L"";
	m_strSequenceNumber = L"";
	m_strTranDate = L"";
	m_strTranTime = L"";
	m_strBusinessDate = L"";

	m_bTranResult = FALSE;

	m_bCheckingBalance = FALSE;
	m_strCheckingLedgerBalance = L"";
	m_strCheckingAvailBalance = L"";

	m_bSavingBalance = FALSE;
	m_strSavingLedgerBalance = L"";
	m_strSavingAvailBalance = L"";
}


// [#2317] US Justin 2015.01.05 Support XML BIN Range
///////////////////////////////////////////////////
// XML BIN Range Data

CXMLBinRangeData::CXMLBinRangeData()
{
	m_bufBIN = NULL;
	m_nNumBIN = 0;	
	m_strUpdateDate = _T("");
}

CXMLBinRangeData::~CXMLBinRangeData()
{
	if( m_bufBIN !=  NULL )
		delete [] m_bufBIN;
}

BOOL CXMLBinRangeData::IsDataLoaded()
{
	if( (m_bufBIN == NULL) || (m_nNumBIN <1) )
		return FALSE;
	return TRUE;	
}

BOOL CXMLBinRangeData::LoadXMLBinRangeData()
{
	NHDEBUG(DBG_INFO, (L"CXMLBinRangeData::LoadXMLBinRangeData()\n"));

	if( m_bufBIN != NULL )
	{
		delete [] m_bufBIN;
		m_bufBIN = NULL;
	}
	m_nNumBIN = 0;
	m_strUpdateDate.Empty();

	// Read XML Files ////////////////////////////////////////////////////////////////////////
	CString strXMLPath = _DCC_DOMESTIC_BINDATA_PATH;		// "\\ATM\\DCC_DomesticBIN.bin"
	CFile cf;
	if( cf.Open( strXMLPath, CFile::modeRead | CFile::typeText) == FALSE )
	{
		NHDEBUG(DBG_INFO, (L"[  ERROR ==> FAIL TO OPEN(%s)]\n", strXMLPath));
		return FALSE;
	}

	// [#2322] US Justin 2015.02.12 Using Binary DCC Domestic BIN
	unsigned long nREAD_SIZE = 1024;
	unsigned char sReadBuf[1024];

	// Read Version
	memset(sReadBuf, NULL, nREAD_SIZE);
	cf.Read(sReadBuf, 4);
	NHDEBUG(DBG_INFO, (L"BIN Data Version = [%S]\n", sReadBuf));

	// Read Updated Date
	memset(sReadBuf, NULL, nREAD_SIZE);
	cf.Read(sReadBuf, 20);
	m_strUpdateDate.Format(L"%S", sReadBuf);
	m_strUpdateDate.TrimRight();
	m_strUpdateDate.TrimLeft();
	NHDEBUG(DBG_INFO, (L"BIN Updated Date = [%s]\n", m_strUpdateDate));

	// Read Number of BIN Data
	cf.Read(&m_nNumBIN, sizeof(unsigned long));
	NHDEBUG(DBG_INFO, (L"Number of BIN Range = [%ld]\n", m_nNumBIN));

	// Allocate Buffer
	m_bufBIN = new unsigned char [ _SIZE_ONE_BINRANGE*m_nNumBIN + 1 ];
	memset( m_bufBIN, NULL, _SIZE_ONE_BINRANGE*m_nNumBIN + 1);
	NHDEBUG(DBG_INFO, (L"Memory Allocated\n"));

	// Read BIN Data
	unsigned long nBufSize = 0;
	DWORD nReadLen;
	BOOL  bReadAll = FALSE;
	do
	{
		nReadLen = cf.Read(sReadBuf, nREAD_SIZE);
		if(nReadLen<nREAD_SIZE)
			bReadAll = TRUE;
		if(nReadLen>0)
		{
			memcpy( m_bufBIN + nBufSize, sReadBuf, nReadLen);
			nBufSize += nReadLen;
		}
	} while(bReadAll == FALSE);
	cf.Close();
	NHDEBUG(DBG_INFO, (L"Local BIN Data is read\n"));

	/*
	unsigned long i,j;
	CString strTemp, strLow, strHigh;

	long nXMLFileLen = (int)cf.GetLength();
	unsigned char* pBuffer = new unsigned char[nXMLFileLen + 2];	// Allocate buffer for binary file data
	if( pBuffer == NULL )
	{
		NHDEBUG(DBG_INFO, (L"[  ERROR ==> FAIL TO NEW BUFFER(%d)]\n", nXMLFileLen + 2));
		cf.Close();
		return FALSE;
	}
	nXMLFileLen = cf.Read( pBuffer, nXMLFileLen );
	pBuffer[nXMLFileLen] = '\0';
	pBuffer[nXMLFileLen+1] = '\0';	
	cf.Close();
	NHDEBUG(DBG_INFO, (L"File [%s] read \n", strXMLPath));

	// Check File Contents (unicode) /////////////////////////////////////////////////////////
	CString strXML(_T(""));
	if ( pBuffer[0] == 0xFF && pBuffer[1] == 0xFE )					// Windows Unicode file is detected if starts with FEFF	
	{
		strXML = (LPCWSTR)(&pBuffer[2]);
		NHDEBUG(DBG_CALL, (L"[  INFO ==> File starts with hex FFFE, assumed to be wide char format.]\n"));
	}
	else
		strXML = (LPCSTR)pBuffer;
	delete [] pBuffer;

	// Read BIN Date /////////////////////////////////////////////////////////////////////////
	long nLocStr = strXML.Find(L"DCCDL");
	if(nLocStr>=0)
	{
		strTemp = strXML.Mid( nLocStr + 5 );
		nLocStr = strTemp.Find (L"\"");
		if (nLocStr >= 0)
		{
			strTemp = strTemp.Mid( nLocStr + 1);
			nLocStr = strTemp.Find (L"\"");
			if(nLocStr >= 0)
				m_strUpdateDate = strTemp.Left(nLocStr);
		}
	}
	NHDEBUG(DBG_INFO, (L"Domestic BIN Date = [%s]\n", m_strUpdateDate));

	// Read Low and High BIN from XML File and store in arrLowBIN and arrHighBin /////////////
	CStringArray arrLowBin, arrHighBin;

	// Byte Operation => to improve speed ////////////////////////////////////////////////////
	// Byte Operation ==> Takes 7 seconds
	WCHAR* wchTmp = new TCHAR[20];
	unsigned char* sFileBuf = new unsigned char [strXML.GetLength() + 2];
	memset( sFileBuf, NULL, strXML.GetLength() + 2);
	WideToMulti( (LPSTR)sFileBuf, strXML, strXML.GetLength() );	
	NHDEBUG(DBG_INFO, (L"File Contents converted to Multi Bytes\n"));
	for(i=0; i<(unsigned long)strXML.GetLength(); i++)
	{
		if( (sFileBuf[i]=='r')&&(sFileBuf[i+1]=='e')&&(sFileBuf[i+2]=='c')&&(sFileBuf[i+3]=='o')&&(sFileBuf[i+4]=='r')&&(sFileBuf[i+5]=='d') )						// record
		{
			strLow = _T("");	strHigh=_T("");
			i += 6;
			int nStart, nOrder, nStartPointer, nEndPointer;
			nStart = nOrder = nStartPointer = nEndPointer = 0;
			for(int k=0; k<100; k++)
			{
				if( (sFileBuf[i+k]=='l')&&(sFileBuf[i+k+1]=='o')&&(sFileBuf[i+k+2]=='w')&&(sFileBuf[i+k+3]=='b')&&(sFileBuf[i+k+4]=='i')&&(sFileBuf[i+k+5]=='n') )	// lowbin
				{
					nStart = 1;
					nOrder = 1;
					k += 6;
				}
				else if( (sFileBuf[i+k]=='h')&&(sFileBuf[i+k+1]=='i')&&(sFileBuf[i+k+2]=='g')&&(sFileBuf[i+k+3]=='h')&&(sFileBuf[i+k+4]=='b')&&(sFileBuf[i+k+5]=='i')&&(sFileBuf[i+k+6]=='n') )	// highbin
				{
					nStart = 1;
					nOrder = 2;
					k += 7;
				}
				else if( sFileBuf[i+k] == '\"' )
				{
					if (nStart==1)
					{
						k++;
						nStartPointer = k;
						nStart = 2;
					}
					else if(nStart==2)
					{
						nEndPointer = k;
						nStart = 3;
					}
					else
						nStart = 0;
				}
				if(nStart==3)
				{
					if( ( (nEndPointer-nStartPointer)>2 ) && ( (nEndPointer-nStartPointer)<12 ) )
					{
						memset(wchTmp, NULL, 20);
						MultiToWide( (LPWSTR) wchTmp , (LPCSTR)&sFileBuf[i+nStartPointer], (nEndPointer-nStartPointer) );
						if(nOrder == 1)			
							strLow.Format( L"%s", wchTmp);
						else if(nOrder == 2)	
						{
							strHigh.Format( L"%s", wchTmp);
							break;
						}
					}
					nStart = 0;
				}
			}
			i += nEndPointer;

			if( (strLow.GetLength()>2)&&(strLow.GetLength()<12)&&(strLow.GetLength()==strHigh.GetLength()) )		//  3 ~ 11 digits
			{
				//NHDEBUG(DBG_INFO, (L"BIN Range Added, [%s] ~ [%s]\n", strLow, strHigh));
				arrLowBin.Add(strLow);
				arrHighBin.Add(strHigh);
			}
		}
	}
	delete [] sFileBuf;
	delete [] wchTmp;
	NHDEBUG(DBG_INFO, (L"Bin Numbers are converted to strings and saved in array\n"));

	// Check Number of BIN ///////////////////////////////////////////////////////////////////
	unsigned long nNumReadBin = arrLowBin.GetSize();
	unsigned long nNumStoredBIN = 0;	
	for(i=0; i<nNumReadBin; i++)
	{
		strLow	= arrLowBin.GetAt(i);
		strHigh	= arrHighBin.GetAt(i);
		unsigned char nFirstTwoLow  = (unsigned char) Asc2Int(strLow.Left(2));
		unsigned char nFirstTwoHigh = (unsigned char) Asc2Int(strHigh.Left(2));
		if( nFirstTwoLow <= nFirstTwoHigh)
			nNumStoredBIN += (nFirstTwoHigh-nFirstTwoLow+1);
	}
	NHDEBUG(DBG_INFO, (L"CXMLBinRangeData:: NumReadBin=[%ld], NumConvertBin=[%ld]\n", nNumReadBin,nNumStoredBIN ));
	if( (nNumReadBin <= 0)||(nNumStoredBIN <= 0) )
	{
		NHDEBUG(DBG_INFO, (L"  ERROR ==> Number of BIN is ZERO\n" ));
		return FALSE;
	}

	// Memory Allocation /////////////////////////////////////////////////////////////////////
	m_nNumBIN = nNumStoredBIN;
	m_bufBIN = new unsigned char [ _SIZE_ONE_BINRANGE*m_nNumBIN + 1 ];
	memset( m_bufBIN, NULL, _SIZE_ONE_BINRANGE*m_nNumBIN + 1);

	// Fill Buffer ///////////////////////////////////////////////////////////////////////////
	CString sZeros = _T("00000000000000");
	CString sNines = _T("99999999999999");
	unsigned long nLocBuf = 0;
	for(i=0; i<nNumReadBin; i++)
	{
		strLow	= arrLowBin.GetAt(i);
		strHigh	= arrHighBin.GetAt(i);

		// Make 11 digit
		strLow  += sZeros.Left(11 - strLow.GetLength() );
		strHigh += sNines.Left(11 - strHigh.GetLength() );
		if( strLow.GetLength() == 11)
		{
			unsigned char nFirstTwoLow  = (unsigned char) Asc2Int(strLow.Left(2));
			unsigned char nFirstTwoHigh = (unsigned char) Asc2Int(strHigh.Left(2));
			unsigned long nLowBinNum, nHighBinNum;

			if( nFirstTwoLow == nFirstTwoHigh)
			{
				m_bufBIN[nLocBuf++] = nFirstTwoLow;

				nLowBinNum = (unsigned long) Asc2Long(strLow.Mid(2));
				memcpy( (char*) &m_bufBIN[nLocBuf], &nLowBinNum, sizeof (unsigned long) );		nLocBuf += sizeof(unsigned long);

				nHighBinNum = (unsigned long) Asc2Long(strHigh.Mid(2));
				memcpy( (char*) &m_bufBIN[nLocBuf], &nHighBinNum, sizeof (unsigned long) );		nLocBuf += sizeof(unsigned long);
			}
			else if( nFirstTwoLow < nFirstTwoHigh)
			{
				for(unsigned char k= nFirstTwoLow; k<=nFirstTwoHigh; k++)
				{
					m_bufBIN[nLocBuf++] = k;

					if(k==nFirstTwoLow)				nLowBinNum = (unsigned long) Asc2Long(strLow.Mid(2));
					else							nLowBinNum = 0;
					memcpy( (char*) &m_bufBIN[nLocBuf], &nLowBinNum, sizeof (unsigned long) );	nLocBuf += sizeof(unsigned long);
					
					if(k==nFirstTwoHigh)			nHighBinNum = (unsigned long) Asc2Long(strHigh.Mid(2));
					else							nHighBinNum = 999999999;
					memcpy( (char*) &m_bufBIN[nLocBuf], &nHighBinNum, sizeof (unsigned long) );	nLocBuf += sizeof(unsigned long);
				}
			}
		}
	}

	// Sorting Bin ///////////////////////////////////////////////////////////////////////////
	bool bChanged, bReversed;
	unsigned char bufTempBin[10];
	unsigned char nIndicator1, nIndicator2;
	unsigned long nLowBinNum1, nLowBinNum2;
	for(i=0; i<m_nNumBIN; i++)
	{
		bChanged = false;
		for(j=0; j<(m_nNumBIN-1); j++)
		{
			bReversed = false;
			nIndicator1 = m_bufBIN[j*_SIZE_ONE_BINRANGE];
			nIndicator2 = m_bufBIN[(j+1)*_SIZE_ONE_BINRANGE];
			memcpy( &nLowBinNum1, &m_bufBIN[j*_SIZE_ONE_BINRANGE+1],		sizeof (unsigned long));
			memcpy( &nLowBinNum2, &m_bufBIN[(j+1)*_SIZE_ONE_BINRANGE+1],	sizeof (unsigned long));

			if( nIndicator1 > nIndicator2 )
				bReversed = true;
			else if( (nIndicator1==nIndicator2) && (nLowBinNum1>nLowBinNum2) )
				bReversed = true;

			if(	bReversed == true )
			{
				memcpy( bufTempBin, &m_bufBIN[j*_SIZE_ONE_BINRANGE], _SIZE_ONE_BINRANGE);
				memcpy( &m_bufBIN[j*_SIZE_ONE_BINRANGE], &m_bufBIN[(j+1)*_SIZE_ONE_BINRANGE], _SIZE_ONE_BINRANGE);
				memcpy( &m_bufBIN[(j+1)*_SIZE_ONE_BINRANGE], bufTempBin, _SIZE_ONE_BINRANGE);
				bChanged = true;
			}
		}
		if(bChanged==false)
			break;
	}
	*/
	// End of [#2332]

	// DEBUG MESSAGE /////////////////////////////////////////////////////////////////////////
	unsigned char nIndicator1 = m_bufBIN[0];
	unsigned char nIndicator2 = m_bufBIN[(m_nNumBIN-1)*_SIZE_ONE_BINRANGE];
	unsigned long nLowBinNum1, nLowBinNum2;
	memcpy( &nLowBinNum1, &m_bufBIN[1],										sizeof (unsigned long));
	memcpy( &nLowBinNum2, &m_bufBIN[(m_nNumBIN-1)*_SIZE_ONE_BINRANGE+1],	sizeof (unsigned long));
	NHDEBUG(DBG_INFO, (L"BIN Stored and Sorted : [%d][%ld] ~ [%d][%ld]\n", nIndicator1, nLowBinNum1,  nIndicator2, nLowBinNum2 ));
	return TRUE;
}

BOOL CXMLBinRangeData::IsCardListed(CString strCardNumber)
{
	NHDEBUG(DBG_INFO, (L"CXMLBinRangeData::IsCardListed(%s)\n", strCardNumber));

	if(!IsDataLoaded())
	{
		NHDEBUG(DBG_INFO, (L"BIN RANGE Data is not loaded\n"));
		return FALSE;
	}

	if(strCardNumber.GetLength() <11)
	{
		NHDEBUG(DBG_INFO, (L"Card Number is too short\n", strCardNumber));
		return FALSE;
	}

	BOOL bFound = FALSE;
	unsigned char nLimitMin = (unsigned char) (m_bufBIN[0]);
	unsigned char nLimitMax = (unsigned char) (m_bufBIN[(m_nNumBIN-1)*_SIZE_ONE_BINRANGE]);
	unsigned char nIndicatorInput = (unsigned char) Asc2Int( strCardNumber.Left(2)  );
	if( (nIndicatorInput>=nLimitMin) && (nIndicatorInput<=nLimitMax) )
	{
		unsigned char nIndicator;
		unsigned long nLowBinNum, nHighBinNum;
		unsigned long nBinNumInput = (unsigned long) Asc2Long( strCardNumber.Mid(2,9) );
		for( unsigned long i=0; i<m_nNumBIN; i++)
		{
			nIndicator = m_bufBIN[i*_SIZE_ONE_BINRANGE];
			if ( nIndicatorInput == nIndicator)
			{
				memcpy( &nLowBinNum,  &m_bufBIN[i*_SIZE_ONE_BINRANGE + 1], sizeof (unsigned long));
				memcpy( &nHighBinNum, &m_bufBIN[i*_SIZE_ONE_BINRANGE + 5], sizeof (unsigned long));
				if( (nBinNumInput>=nLowBinNum) && (nBinNumInput<=nHighBinNum) )
				{
					NHDEBUG(DBG_INFO, (L"Card Number Found \n"));
					bFound = TRUE;
					break;
				}
				else if(nHighBinNum > nBinNumInput)
				{
					NHDEBUG(DBG_INFO, (L"9 digit bin number range exceeded =>Input[%09ld] < BufCheck[%09ld]\n", nBinNumInput, nHighBinNum));
					break;
				}
			}
			else if( nIndicator > nIndicatorInput)
			{
				NHDEBUG(DBG_INFO, (L"2 digit indicator exceeded input => Input[%02d] < BufCheck[%02d]\n", nIndicatorInput, nIndicator));
				break;
			}
		}
	}
	else
		NHDEBUG(DBG_INFO, (L"2 digit indicator exceeded Range => Min[%02d], Max[%02d], Input[%02d]\n", nLimitMin, nLimitMax, nIndicatorInput));

	return bFound;
}

// End of [#2317]
