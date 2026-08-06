#include "stdafx.h"
#include "SVC_Manager.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/** **********************************************************
*	@brief		고객용 커맨드 - 고객 변경
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ChangeCustom(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_CHANGE_CUSTOM);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 투입
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_InsertCard(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_INSERT_CARD);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - EMV 카드 Read
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_EMV_CardRead(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_EMV_CARD_READ);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - Make Application
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_EMV_MakeAppLists(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_EMV_MAKEAPPLISTS);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - Final Select
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_EMV_FinalSelect(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_EMV_FINALSELECT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - Final Select
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_EMV_MultiAppSelect(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_EMV_MULTIAPPSELECT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - EMV FallBack
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_EMV_FallBack(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_EMV_FALLBACK);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 제거
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_RemoveCard(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_REMOVE_CARD);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 카드 읽기
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ReadCard(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_READ_CARD);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 언어 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_SelectLanguage(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_SELECT_LANGUAGE);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - PIN 읽기
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ReadPin(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_READ_PIN);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 거래 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Select_Transaction(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_SELECT_TRANSACTION);

	m_ServiceLock.Unlock();
}

/** **********************************************************
*	@brief		고객용 커맨드 - 계좌 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Select_Accounts(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_SELECT_ACCOUNT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 계좌 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Select_TransferAccounts(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_SELECT_TRANSFERACCOUNT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 금액 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Select_Amount(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_SELECT_AMOUNT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 금액 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ADA_ConfirmAmount(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_ADA_CONFIRMAMOUNT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 수수료 표시
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Confirm_Surcharge(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_CONFIRM_SURCHARGE);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 이체 금액 입력
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Enter_TransferAmount(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_ENTER_TRANSFERAMOUNT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 출금 금액 입력
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Enter_WithdrawalAmount(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_ENTER_WITHDRAWALAMOUNT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 영수증 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Select_Receipt(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_SELECT_RECEIPT);

	m_ServiceLock.Unlock();
}

/** **********************************************************
*	@brief		고객용 커맨드 - 영수증 불가 확인 화면
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Confirm_ReceiptError(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_CONFIRM_RECEIPTERROR);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 영수증 선택
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Confirm_ScreenReceipt(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_CONFIRM_RECEIPTSCREEN);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - HOST 송/수신
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Host_Processing(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_HOST_PROCESSING);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - EMV Remove Card
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_EMV_RemoveCard(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_EMV_REMOVECARD);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - HOST 송/수신
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_Cash_Dispensing(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_CASH_DISPENSING);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 영수증 출력
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_PrintReceipt(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_PRINT_RECEIPT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - 영수증 화면 출력
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ScreenReceipt(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_SCREEN_RECEIPT);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		고객용 커맨드 - Reversal 처리
*	@retval		없음
************************************************************/
void CSVC_Manager::Client_ReversalProcessing(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_CL_REVERSAL);

	m_ServiceLock.Unlock();
}


/** **********************************************************
*	@brief		AMS 커맨드 - AMS Accept OK
*	@retval		없음
************************************************************/
void CSVC_Manager::AMS_AcceptOK(void)
{
	m_ServiceLock.Lock();

	m_CmdQueue.Enqueue(EAGLE_CMD_AMS_ACCEPT_OK);

	m_ServiceLock.Unlock();
}


