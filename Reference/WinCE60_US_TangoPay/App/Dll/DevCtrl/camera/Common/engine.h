#include "common.h"
#include "usbio.h"
#include "utility.h"

#ifndef _ENGINE_H_
#define _ENGINE_H_

#define CAMAPI

#define CAMERADEVICE_STS_INSERTED		(uint32_t)(0)
#define CAMERADEVICE_STS_REMOVED		(uint32_t)(1)

#define MAX_TRANSFER_OVERLAPPED_COUNT	(uint32_t)(4)
#define MAX_TRANSFER_ISSUED_COUNT		(uint32_t)(4)

typedef struct _CAMERA_DEVICEINFO
{
	USBIO_DEVICEHANDLE_t			DeviceHandle;
	USBIO_PIPEHANDLE_t				ActivePipeHandle;
	LOOPQUEUE_t						LoopQueue;
	HANDLE_t						InternalThread;
	HANDLE_t						InternalThreadKillEvent;
	uint32_t						State;
}CAMERA_DEVICEINFO_t, *PCAMERA_DEVICEINFO_t;

typedef PCAMERA_DEVICEINFO_t CAMERA_DEVICEHANDLE_t;

CAMAPI bool_t Engine_Start(IN bool_t bMainCamera, IN bool_t bSubCamera);
CAMAPI void_t Engine_Stop();
CAMAPI bool_t Engine_GetFrame(HANDLE hCameraHandle, uint8_t **pFrameBuffer, uint32_t *pSize);

#endif // _ENGINE_H_
