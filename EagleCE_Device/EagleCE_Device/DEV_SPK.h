#pragma once

#include <sapi.h>
#pragma warning(disable:4996)
#include <sphelper.h>
#pragma warning(default:4996)

/************************************************************
* Eagle Speak
************************************************************/
class CDEV_SPK
{
public:
	// 생성자
	CDEV_SPK(void);
	// 소멸자
	~CDEV_SPK(void);

	// 초기화
	BOOL Initialize(void);

	// 속도 설정
	void SetRate(int nRate);
	// 음성 설정
	void SetVoices(CString strLanguage, CString strGender);
	// 볼륨 설정
	void SetVolume(int nVolume);

	// 재생
	BOOL SpeakAsync(CString strText);
	// 중지
	BOOL SpeakStop();

protected:
    CComPtr<ISpVoice> m_VoiceObject;
};

