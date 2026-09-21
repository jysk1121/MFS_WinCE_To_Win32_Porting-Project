#ifndef __SERIAL_COMM_H__
#define	__SERIAL_COMM_H__
#pragma once

/* Protocol Packet =
+ SOH
+ Control
+ (Data)Length
+ DATA
+ CRC
*/

typedef int (*EventCallBackFunction)(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);

#define		MAX_RECV_BUFFER_SIZE		8192
#define		SENDFLAG_IDLE				0x00	
#define		SENDFLAG_SEND_REQ			0x01	//ACK 대기중 모드를 설정하기 위한 DEFINE
#define		RECVFLAG_IDLE				0x00
#define		RECVFLAG_RECV_REQ			0x01	//Command에 대한 응답 대기 모드를 설정하기 위한 DEFINE

#define		KEYFLAG_IDLE				0x00	//Key input mode 아님
#define		KEYFLAG_NORMAL				0x30	//Normal key input mode
#define		KEYFLAG_ENC					0x31	//Encryption key input mode
#define		KEYFLAG_INPUT_KEY			0x03	//Pinpad를 통한 KEY 입력모드
#define		KEYFLAG_INPUT_KEY_TDES		0x04	//Pinpad를 통한 KEY 입력모드 Triple DES.

#define		SOH							0x01
#define		STX							0x02
#define		ETX							0x03
#define		ACK							0x06
#define		NACK						0x15

#define		TIMEOUT_CONSTANT			5					/* Serial port timout */
#define		PACKET_TIMEOUT				((int)60)				/* Timeout between 2 packets */
#define		ACK_TIMEOUT					((unsigned int)((((float)6000) * ((float)1000)) / ((float)BAUD_RATE)) )/* ACK timeout in ms. (Mult. by 1000 converts to ms). */
#define		CHAR_TIMEOUT				((unsigned int)100) /* Wait (100) => 109ms *//* CHAR timeout in ms. */
#define		TIME_PER_CALL				((unsigned int)55)/* The actual time (in ms) that Wait() waits per call.  */


#define		MAX_RETRANSMISSION			3		/* Maximum number of retransmissions*/
#define		CRC_POLYNOMIAL				0x8005  /* CRC polynomial */

#define STATE_OK					'O'
#define STATE_FAIL					'F'


typedef struct _CMD_StartEntry_
{
	byte			byEntryMode;
	short			sMinKeys;
	short			sMaxKeys;
	unsigned long	ulTerminateKeys;
	unsigned long	ulTerminateFDKeys;
	unsigned long	ulActiveKeys;
	unsigned long	ulACTIVEFDKeys;
	byte			byAutoEnd;
}CMD_START_ENTRY, *LPCMD_START_ENTRY;


#define	DLLTEST_EXPORTS
//#define		STD_CALL

#ifdef DLLTEST_EXPORTS
#define DLLTEST_API extern "C" __declspec(dllexport)
#else
#define DLLTEST_API extern "C" __declspec(dllimport)
#endif

//DLLTEST_API int EPPOpenDevice(int nPortNum, DWORD dwBaudRate, BYTE);
DLLTEST_API int EPPOpenDevice(int nPortNum, DWORD dwBaudRate);
DLLTEST_API int EPPCloseDevice();
DLLTEST_API int EPPExecuteCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus, BOOL bWaitResponse);
DLLTEST_API int EPPRegistEventCallBackFunction(EventCallBackFunction pFunc);
DLLTEST_API int EPPGetCommandResultData(LPBYTE lpData, int nBuffLeng, int* nLength);

#define GET_NORMAL_KEY			0xb1
#define INPUT_CANCEL			0x42/**< PIN input cancel command */

#define		ERR_SEND_DATA_FAIL				0xFE01
#define		ERR_RECV_ACK_TIMEOUT			0xFE02
#define		ERR_RECV_RESPONSE_TIMEOUT		0xFE03
#define		ERR_BUFFER_SIZE_ERROR			0xFE04
#define		ERR_RECV_BCC_ERROR				0xFE05
#define		ERR_CMD_MISSMATCH				0xFE06

#endif	/