/*----------------------------------------------------------------------

	ATM과 RMS간의 송 수신 및 명령어를 처리 한다.

	작성자 : AIREAT
	작성일 : 2008.04.01
----------------------------------------------------------------------*/


#ifndef __NH_RMS_CTRL_H__
#define __NH_RMS_CTRL_H__

#ifndef _RMSCTRL_OWNER_
#pragma comment(lib, "RmsCtrl.lib")
#endif

#include "RMSDefine.h"
#include ".\Dev\DevCmn.h"
#include ".\Tran\TranCmn.h"

class AFX_CLASS_EXPORT CRmsCtrl
{

public:
	CRmsCtrl();
	~CRmsCtrl();

public:
	RMS_CMD_ADD_CASH	m_AddCashInfo;		// [#530] NH KSK 2009.06.03
	RMS_CMD_ATM_DATE_TIME m_DateTimeInfo;	// [#611] SOOK 2010.01.16 Change ATM Time추가 

public:
	//void SetDevCmn(CDevCmn *pDevCmn);
	void SetDevCmn(CDevCmn *pDevCmn, CTranCmn *pTranCmn);		// [#2467] US Justin 2017.01.25 Dual Host Loan
//	BOOL RMS_Communication(int LineType, int OpenType);
	BOOL RMS_Communication(int LineType, int OpenType, char SubCommand = 0);	// [#613] AU_C KSK 2010.01.18

	BYTE RMS_EncryptXOR(BYTE *pData, int nLen);
	void RMS_DecryptXOR(BYTE nTableIndex, BYTE *pData, int nLen);

private:
	int	RMS_SendControl(BYTE ctl);
	int RMS_Send(BYTE *pData, int nLen, int nSendSec);
	int RMS_Recv(BYTE *pData, int *pnLen, int nRecvSec);

private:
	// verify MSG
	BOOL RMS_VerifyMsg(BYTE *pData, int Len);
	// Parse RMS Data
	BOOL RMS_Proc_Dial();			// Modem Call Wait
	BOOL RMS_Status_Dial();			// Modem Status Send
	BOOL RMS_JournalUpload_Dial();	// [#613] AU_C KSK 2010.01.18 Scheduled Journal Upload Function Add
	BOOL RMS_Proc_Tcp();			// TCP Call Wait
	BOOL RMS_Status_Tcp();			// TCP Status Send
	BOOL RMS_Status_Tcp_NotRecvRmsCmd();	// [#2580] AU Kook 2018.11.07 RMS Test
	BOOL RMS_JournalUpload_Tcp();	// [#613] AU_C KSK 2010.01.18 Scheduled Journal Upload Function Add

private:

	BOOL RMS_ParseCMD(BYTE *pBuffer);

	BOOL RMS_SendATMStatus(NUINT8 MsgID, int nFrameNumber=0);	// proc RMS_REQ_STATUS or Inform to RMS

	// Process CMD
	BOOL RMS_ProcReqSetup(void);	// proc RMS_REQ_SETUP
	BOOL RMS_ProcSetSetup(void);	// proc RMS_SET_SETUP
	BOOL RMS_ProcJNLUpload(void);	// Proc RMS_REQ_JNL
	BOOL RMS_ProcJNLXUpload(void);	// Proc RMS_REQ_X_JNL
	BOOL RMS_ProcJNLDateUpload(BYTE *pBuffer);	// Proc RMS_REQ_DATE_JNL	// [#524] US KSK 2009.05.19
	BOOL RMS_ProcFileDownLoad(BYTE *pBuffer);
	BOOL RMS_ProcFileUpLoad(BYTE *pBuffer);			// [#523] US KSK 2009.05.19
	BOOL RMS_ProcReqErrTotal(BOOL bClear = FALSE);	// [#424] [NH] KSK 2008.9.18
	BOOL RMS_ProcReqRejectTotal(BOOL bClear = FALSE);	// [#533] NH KSK 2009.06.05

	/**
	 * Handle the remote update command sent by RMS
	 * @param pBuffer[in] The buffer containing the data sent by RMS
	 * @return true when successful
	 */
	BOOL RMS_ProcRemoteUpdate(BYTE *pBuffer);	// [RWC6-134] US William 2020.03.24 RMS Remote Updates
	
#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	BOOL RMS_ProcReqBinList();	//[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
	BOOL RMS_ProcSetBinList();	//[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
#endif

	BOOL RMS_ProcReqEnhancedCoupon();	// [#620] US KSK 2010.02.22
	BOOL RMS_ProcSetEnhancedCoupon();	// [#620] US KSK 2010.02.22

	BOOL RMS_ProcSetWeatherInfo();		//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 
	BOOL RMS_ProcUploadJNLImage(BYTE *pBuffer);		//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 


	// Make Msg
	BYTE RMS_GetMachineType();								// [#GLDV-2853] AU Kook 2021.02.17 Support MX2800SE

	BOOL RMS_MakeStatusMsg(BYTE *pMsg, int Len);
	BOOL RMS_MakeSetupReadDataMsg(BYTE *pMsg, int Len);
	BOOL RMS_MakeSetupDataMsg(BYTE *pMsg, int Len);
	BOOL RMS_MakeJNLUploadMsg(BYTE *pMsg, int MsgLen, PATM_JNL_INFO pATMJNLInfo);
	BOOL RMS_MakeJNLUploadLastXMsg(BYTE *pMsg, int Len);
	int  RMS_MakeErrTotalMsg(BYTE *pMsg);	// [#424] [NH] KSK 2008.9.18
	BOOL RMS_MakeUploadFileDataMsg(BYTE *pMsg, int Len);	// [#523] US KSK 2009.05.19
	int	 RMS_MakeRejectTotalMsg(BYTE *pMsg);	// [#533] NH KSK 2009.06.08

	BOOL RMS_MakeEnhancedCouponReadDataMsg(BYTE *pMsg);	// [#620] US KSK 2010.02.22
	BOOL RMS_MakeEnhancedCouponMsg(BYTE *pMsg);			// [#620] US KSK 2010.02.22

	BOOL RMS_MakeJNLImageDataMsg(BYTE *pMsg, int Len);		//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 

	// Write Msg
	BOOL RMS_WriteSetupDataMsg(BYTE *pMsg, int Len);
	BOOL RMS_WirteWeatherDataMsg(BYTE *pMsg, int Len);		//[#586] SOOK 2009.12.04 iTM MoniView 연동 부분 추가 

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	int RMS_MakeBinListMsg(BYTE *pMsg);	//[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
	BOOL RMS_WriteBinListMsg(BYTE *pMsg, int Len);//[#546] SOOK 2009.07.23 다기능 BIN LIST 기능 추가함 
	BOOL RMS_WriteBinList(BYTE *pBuffer, int nLen);
#endif

	BOOL RMS_WriteEnhancedCouponMsg(BYTE *pMsg, int Len);	// [#620] US KSK 2010.02.22

	/*	ATM <-> RMS  E-JOUNAL */
	int  GetJournalCount();
	BOOL MakeRMSJNLUpload(BYTE *pMsg, int MsgLen, CString strJnlData);
	int  JNLConvertTran(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertReversalFailure(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertAddCash(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertDenomi(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertDayTotal(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertCassetteTotal(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertChangeProcessor(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertChangePassword(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertOperatorAction(BYTE *pMsg, int Len, CStringArray &arJnlField);				// [#408] AU AIREAT 2008.08.26
	int  JNLConvertChangeExchangeRate(BYTE *pMsg, int Len, CStringArray &arJnlField);			// KSK 2009.3.30
	int	 JNLConvertUserCancel(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int	 JNLConvertChangeParameter(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int	 JNLConvertTrxEMVData(BYTE *pMsg, int Len, CStringArray &arJnlField);					// [#2279] NH Justin 2014.06.17 Upload EMV data to Moniview
	int  JNLConvertBriefDayTotal(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertBriefCassetteTotal(BYTE *pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertLibertyXTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertLibertyXDispenseTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField);
	int  JNLConvertB4UTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField);	
	int  JNLConvertDigitalMintTransactions(BYTE* pMsg, int Len, CStringArray &arJnlField);		// [#RWC6-399] US ryan.payton 2022.08.02 DigitalMint

	BOOL GetCSTStatus(PCST_STATUS pCSTStatus);
	void RMS_WideToMulti(LPCTSTR pWideChar, char *pMultiChar, int nLenMultiChar);
	void RMS_MultiToWide(char *pMultiChar, LPWSTR pWideChar, int nLenWideChar);

#if (AU_VERSION)
	BOOL RMS_CheckFileHashValue(LPCTSTR pFileName, BYTE *pHashValue, WORD wHashLength, BYTE *pCertiValue= NULL, WORD wCertiLen = 0);	//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
	BOOL RMS_SaveCertificateFile(BYTE *pFileName, BYTE *pCertiValue, WORD wCertiLen );	//[#635] SOOK 2010.04.01 호주 APCA 인증 대응 Download File Authentication 
#endif

private:
	CDevCmn*			m_pDevCmn;
	CTranCmn*			m_pTranCmn;
};

#endif __NH_RMS_CTRL_H__

