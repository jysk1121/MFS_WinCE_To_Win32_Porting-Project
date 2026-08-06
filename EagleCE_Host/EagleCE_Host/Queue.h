/** *************************************************************************
*	@file Queue.h 
*	@date 2017/10/20
*	@author MFS
*	@brief 라인으로 부터 받은 데이터를 저장하는 버퍼를 구현한 헤더파일입니다.
*****************************************************************************/

// Queue.h: interface for the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUEUE_H__77721570_5C0E_49DF_B939_278BFD2B0FEC__INCLUDED_)
#define AFX_QUEUE_H__77721570_5C0E_49DF_B939_278BFD2B0FEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define		QUEUE_SIZE		(1024 * 500)

#define		DISCONNECT_EVT	'D'
#define		CONNECT_EVT		'C'
#define		ACCEPT_EVT		'A'
#define		NEWCALL_EVT		'N'
#define		TIMEOUT_EVT		'O'

// MODE DISCONNECT DETAIL INFO
#define		USER_NODIALTONE		0x01
#define		USER_NOANSWER		0x02
#define		USER_LINEBUSY		0x04

#define		RECV_DATA		1
#define		RECV_EVENT		2

/** ****************************************
*	@class RECV_QUEUE
*	@date 2017/10/19 
*	@author MFS 
*	@brief 라인 영역에서 받은 데이터들을 저장 및 처리하는 기능을 담당합니다. 
********************************************/
class RECV_QUEUE
{
public:
	RECV_QUEUE();
	virtual ~RECV_QUEUE();

public:
	int		recv_sp; ///< 세이브 포인트입니다. 
	int		recv_pp; ///< 프로그레스 포인트입니다. 
	char*	recv_kind; ///< 데이터 또는 이벤트를 저장합니다. 
	char*	recv_buff; ///< 수신 버퍼입니다. 
};

#endif // !defined(AFX_QUEUE_H__77721570_5C0E_49DF_B939_278BFD2B0FEC__INCLUDED_)
