#pragma once

#include <mmsystem.h>

class CDEV_SND
{
public:
	// 생성자
	CDEV_SND();
	// 소멸자
	virtual ~CDEV_SND();

	// Play Sound
	BOOL PlaySound(CString strFileName, DWORD dwPlayMode = SND_ASYNC/* 1회 재생*/);

	// Stop Sound
	BOOL StopSound();

	// Set Volume
	BOOL SetVolume(DWORD nVolume);

	// Get Volume
	BOOL GetVolume(DWORD& nVolume);
};

