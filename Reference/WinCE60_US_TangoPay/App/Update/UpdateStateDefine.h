#ifndef __NH_UPDATE_STATE_DEFINE_H__
#define __NH_UPDATE_STATE_DEFINE_H__

// ----------------------------------------------------------------------------
//	UPDATE STATE의 이름 정의
// ----------------------------------------------------------------------------
#define	STATE_UP_NONE						L""								// 빈 State 에러를 의미한다.

#define STATE_UP_INIT						L"UP_INIT"						// 구동에 필요한 준비작업을 수행 한다.
#define STATE_UP_FROM_RMS					L"UP_FROMRMS"					// RMS로 부터 시작되는 UPDATE를 수행한다.
#define STATE_UP_FROM_USB					L"UP_FROMUSB"					// AP로 부터 시작되는 UPDATE를 수행한다.
#define STATE_UP_FROM_REMOTE				L"UP_REMOTE"					// Load from remote repository [#RWC6-121]
#define STATE_UP_EMERGENCY_REMOTE			L"UP_EMERGENCY"					// Load latest AP from repository

#define STATE_UP_VALIDATE					L"UP_VALIDATE"					// UPDATE를 수행하는 대상 파일에 대한 유효성을 검사한다.
#define STATE_UP_UPDATE						L"UP_UPDATE"					// "\ATM" 폴더로 업데이트를 수행한다.
#define	STATE_UP_REBOOT						L"UP_REBOOT"					// UPDATE의 수행을 종료하고 REBOOT을 한다.

#endif