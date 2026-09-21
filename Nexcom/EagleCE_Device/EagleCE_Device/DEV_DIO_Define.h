#pragma once

#define	DIO_1							(BYTE)0x01			// Receipt Exit LED
#define	DIO_2							(BYTE)0x02			// Card Reader LED
#define	DIO_3							(BYTE)0x04			// A4 or Cash LED
#define	DIO_4							(BYTE)0x08			// EPP LED
#define	DIO_5							(BYTE)0x10			// TOP1 LED
#define	DIO_6							(BYTE)0x20			// TOP2 LED
#define	DIO_7							(BYTE)0x40			// Bottom LED
#define	DIO_RESERVED					(BYTE)0x80			// Bottom LED
#define DIO_ALL							(BYTE)0x7F			// 
#define DIO_EXCEPT_EPP					(BYTE)0x77			// EPP를 제외한 LED ID

// SENSOR Status Value ( 0 = OPEN, 1 = CLOSE )
#define SNS_FRONT						0x01
#define SNS_CDMPOS						0x02
#define SNS_PRTPOS						0x04
#define SNS_TOPPOS						0x08
#define SNS_AUDIO						0x10

#define DIO_CMD_OFF							0		// Off
#define	DIO_CMD_ON							1		// On
#define DIO_CMD_FLICKING					2		// Flicking
#define DIO_CMD_NONE						3		// Action 미 수행

#define DIO_OPEN						'0'		// Open
#define	DIO_CLOSE						'1'		// Close

// SIU Command
#define	SIU_VERSION				0x56 //'V'
#define	SIU_SENSOR				0x53 //'S'
#define	SIU_FLICKER				0x4C //'L'
#define	SIU_FLICKING			0x54 //'T'

#define SIU_PRTLED_FLICKING			0x30
#define SIU_CDRLED_FLICKING			0x31
#define SIU_CDMLED_FLICKING			0x32
#define SIU_EPPLED_FLICKING			0x33
#define SIU_TOPLED_FLICKING			0x34
#define SIU_EPPSIDELED_FLICKING		0x35
#define SIU_BOTTOMLED_FLICKING		0x36
#define SIU_ATMLED_FLICKING			0x37
#define SIU_CIMLED_FLICKING			0x38

#define SIU_SUPPORTFLICKING_VERSION		'4'		// V01.04부터 Flicking 지원

#define DIO_SEND_DATA_FIXED_LEN		5	// STX, LEN, CMD, ETX, BCC

#define WM_DIO_NOTIFY					(WM_USER+0x900)
