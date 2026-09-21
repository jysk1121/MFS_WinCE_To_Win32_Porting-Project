#include ".\..\common\usbcommon.h"

/*
이 파일은 운영체제에 따라서 종속적으로 작성되는 파일입니다
*/

#include ".\..\common\common.h"

#ifndef _USBIO_H_
#define _USBIO_H_

#define EPADDRESS		(uint32_t)(0x81)

// PIPE State
#define USBPIPESTS_READY			(0)
#define USBPIPESTS_REMOVED			(1)

// Transfer State
#define USBTRANSFERSTS_ISSUED			(0)
#define USBTRANSFERSTS_COMPLETED		(1)

typedef LONG USBD_STATUS;

#define EP_BULKIN			(uint32_t)(0x81)

typedef struct _USBD_ISO_PACKET_DESCRIPTOR {
	ULONG Offset;
	ULONG Length;
	USBD_STATUS Status;
} USBD_ISO_PACKET_DESCRIPTOR, *PUSBD_ISO_PACKET_DESCRIPTOR;

typedef struct _USBDATA {
	USBD_ISO_PACKET_DESCRIPTOR	Packet[ISOCH_TRANSFER_PACKETS];
	unsigned char Data[ISOCH_TRANSFER_PACKETS * ISOCH_MAX_PACKET_SIZE];
} USBDATA, *PUSBDATA;

typedef struct _USBIO_DEVICEINFO
{
	uint16_t			DeviceName[MAX_PATH];
	uint32_t			Reserved;
	uint32_t			hMaster;
	void_t *			pfnInsertCallback;
	void_t *			pfnRemoveCallback;

	HANDLE				internalMonitorThread;
}USBIO_DEVICEINFO_t, *PUSBIO_DEVICEINFO_t;

typedef PUSBIO_DEVICEINFO_t USBIO_DEVICEHANDLE_t;

typedef struct _USBIO_PIPEINFO
{
	USBIO_DEVICEHANDLE_t	DeviceHandle;
	uint32_t				EPAddress;
	uint32_t				State;
}USBIO_PIPEINFO_t, *PUSBIO_PIPEINFO_t;

typedef PUSBIO_PIPEINFO_t USBIO_PIPEHANDLE_t;

typedef struct _USBIO_TRANSFER
{
	USBIO_PIPEHANDLE_t	PipeHandle;
	void_t *			pBuffer;
	uint32_t			RequestLength;
	uint32_t			ActualLength;
	void_t *			pContext;
	void_t *			pfnCallback;
	uint32_t			Stat;

	// 아래는 Win32 Desktop환경과 호환성을 두고 작성된 변수
	HANDLE				internalThreadHandle;
	OVERLAPPED			internalOverlapped;
	HANDLE				internalEvent;
}USBIO_TRANSFER_t, *PUSBIO_TRANSFER_t;

typedef
uint32_t
FNTRANSFER_CALLBACK(
	IN USBIO_PIPEHANDLE_t	PipeHandle,
	IN void_t *				pBuffer,
	IN uint32_t				RequestLength,
	IN uint32_t				ActualLength,
	IN void_t *				pContext
);

typedef FNTRANSFER_CALLBACK *PFNTRANSFER_CALLBACK;

typedef
void_t
FNEVENT_CALLBACK(
	IN USBIO_DEVICEHANDLE_t	DeviceHandle
);

typedef FNEVENT_CALLBACK *PFNEVENT_CALLBACK;

UTIL_API void_t		USBIO_ReadyResource();
UTIL_API void_t		USBIO_FreeResource();

UTIL_API bool_t		USBIO_CreateDeviceHandle(
	IN uint32_t DeviceIndex,
	OUT USBIO_DEVICEHANDLE_t * pDeviceHandle,
	IN PFNEVENT_CALLBACK pInsertCallBack,
	IN PFNEVENT_CALLBACK pRemoveCallBack
);

UTIL_API bool_t		USBIO_CloseDeviceHandle(IN USBIO_DEVICEHANDLE_t DeviceHandle);

UTIL_API bool_t		USBIO_CreateBundlePipe(IN USBIO_DEVICEHANDLE_t DeviceHandle, IN uint32_t EpAddress, OUT USBIO_PIPEHANDLE_t * pPipeHandle);
UTIL_API bool_t		USBIO_ReadBundlePipeAsync(
	IN USBIO_PIPEHANDLE_t PipeHandle,
	OUT void_t * pBuffer,
	IN uint32_t Length,
	IN PFNTRANSFER_CALLBACK	pfnCallback,
	IN void_t * pContext,
	OUT PUSBIO_TRANSFER_t * ppTransfer
	);
UTIL_API bool_t		USBIO_AbortTransfer(
	IN PUSBIO_TRANSFER_t pTransfer
);
UTIL_API bool_t		USBIO_CloseTransfer(
	IN PUSBIO_TRANSFER_t pTransfer
);
UTIL_API bool_t		USBIO_CloseBundlePipe(IN USBIO_PIPEHANDLE_t PipeHandle);

#endif //_USBIO_H_