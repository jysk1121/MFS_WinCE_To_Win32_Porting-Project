#pragma once

#include "../../EagleCE_Framework/EagleCE_Framework/EagleConfig.h"
#include "EagleDefine.h"

/** **********************************************************
*	@file		EagleDataManager.h
*	@brief		데이터 관리자
************************************************************/
class CEagleDataManager
{
	// 인스턴스
	static CEagleDataManager* m_pInstance;
	// 생성자
	CEagleDataManager(void);
	// 소멸자
	~CEagleDataManager(void);

public:
	// 인스턴스 생성
	static void CreateInstance();
	// 인스턴스 취득
	static CEagleDataManager* GetInstance();
	// 인스턴스 해제
	static void ReleaseInstance();

	// 초기화
	BOOL Initialize(void);
	// 종결화
	BOOL Finalize(void);

	// 환경 설정
	CEagleConfig m_Config;

	// 고객 정보
	EAGLE_CLIENT_INFO m_Client_Info;
};

