#pragma once

#define WM_USBMEM_NOTIFY			(WM_USER+0x2000)


class CDEV_USBMEM
{
public:
	// 생성자
	CDEV_USBMEM(void);

	// 소멸자
	virtual ~CDEV_USBMEM(void);


	// 열기
	BOOL Open(HWND hWnd);

	// 닫기
	BOOL Close(void);


	// 연결 확인
	BOOL IsConnected();

	// 드라이브 문자 취득
	TCHAR GetDriveLetter();


	// 디바이스 변경
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);


protected:
	// 통지 윈도우
	HWND	m_hNotifyWnd;

	// 연결
	BOOL	m_bConnected;

	// 드라이브 문자
	TCHAR	m_tDriveLetter;
};
