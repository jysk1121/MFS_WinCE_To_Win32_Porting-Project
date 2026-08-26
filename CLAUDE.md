# Nexcom / Eagle ATM — WinCE → Win32 Porting

이 문서는 이 저장소에서 Claude(Claude Code 등)와 작업을 이어갈 때 필요한 배경 지식을
정리한 것입니다. 새 세션을 시작할 때 이 파일을 먼저 읽으면 이전 세션에서 어떤 작업이
있었는지, 어떤 규칙을 지켜야 하는지 알 수 있습니다.

## 1. 프로젝트 개요

- **코드베이스**: Nexcom/Eagle ATM 펌웨어. C++/MFC, **WinCE 6.0**(ICM_3011, ARMv4I 실장비)과
  **Win32 데스크톱 시뮬레이터**를 동시에 지원하는 듀얼 타깃 빌드.
- 타깃 분기는 `_WIN32_WCE` 매크로와 `Include/Common/Eagle_Common_Define.h`의
  `EMULATION_xxx_DEVICE` 계열 매크로로 제어됨.
- Win32 시뮬레이터 관련 코드는 대부분 `#ifndef _WIN32_WCE` / `#if !defined(_WIN32_WCE)` 블록
  안에 있으며, 실제 장비 대신 모달리스 Win32 패널(체크박스, 다이얼로그 등)로 하드웨어를
  흉내낸다.

## 2. ⚠️ 최우선 규칙 — 소스 인코딩 (CP949)

이 저장소의 소스 대부분은 **CP949(한글) 인코딩 + CRLF**로 저장되어 있다. UTF-8을 가정하는
일반적인 파일 편집 도구로 건드리면 한글 주석이 깨진다(U+FFFD로 치환됨).

- 파일을 고치기 **전에 반드시** 실제 인코딩을 직접 확인할 것 (Python 예시):
  ```python
  open(path, 'rb').read().decode('cp949')   # errors= 파라미터를 쓰지 말 것 (진짜 실패를 숨김)
  open(path, 'rb').read().decode('utf-8')   # BOM이 있으면 UTF-8인 파일도 있다
  ```
- CP949로 확인되면 **UTF-8 가정 편집 도구를 쓰지 말고** byte-level 스크립트로만 수정한다.
  (cp949로 디코드 → 필요한 부분만 문자열 치환 → cp949로 다시 인코드 → 바이너리로 write)
  이 규칙을 어기면 한글 주석이 U+FFFD로 깨지며, 일부는 원본이 없으면 영구 복구 불가능하다.
- UTF-8(+BOM)로 확인된 파일도 존재한다 (`EagleSVCLib.cpp`, `LineTCP.cpp`, `DEV_CDR.cpp/.h`,
  `EagleCE_Host.cpp`, 일부 `.vcproj` 등). 이런 파일은 일반 편집 도구를 써도 안전하지만,
  **파일마다 개별적으로 인코딩을 확인**해야 한다 — 확장자나 위치로 추측하지 말 것.

## 3. 빌드 구조 요약

| 영역 | 경로 |
|---|---|
| 카드리더 시뮬레이터 | `EagleCE_Device/EagleCE_Device/DEV_CDR.h/.cpp`, `DevSimCDRDlg.h/.cpp`, `DevSimTrack2Dlg.h/.cpp` |
| DIO 센서 시뮬레이터 | `EagleCE_Device/EagleCE_Device/DevSimSensorDlg.cpp` |
| 호스트 통신 라인 구현 (TCP/OpenSSL/curl) | `EagleCE_Host/EagleCE_Host/LineTCP.*`, `LineHttps.*`, `LineCurl.*`, `LineCtrl.h` |
| 호스트 설정 로드/저장 | `EagleCE_Framework/EagleCE_Framework/EagleConfigDefine.h`, `EagleConfig.cpp` |
| 호스트 설정 파일 | `EagleCE_ATM/EagleCE_ATM/**/HOST.CFG` (여러 카피 존재, 빌드 산출물 폴더별) |
| ATM 클라이언트 카드/EMV 처리 로직 | `EagleCE_ATM/EagleCE_ATM/SVC_Manager_Client.cpp`, `SVC_Manager.cpp` |
| 공통 매크로 정의 | `Include/Common/Eagle_Common_Define.h` |
| Win32 prebuilt DLL (canonical) | `DLL_Win32/` |
| ARM(WinCE) prebuilt DLL (canonical) | `DLL_CE/` |
| 빌드 후처리 스크립트 | `BuildFiles/PostBuild_Win32.cmd`, `BuildFiles/BuildScript.bat`, `BuildFiles/BuildScript_here.bat` |

## 4. 이 저장소에 반영된 주요 변경 이력 (요약)

1. **DIO 센서 시뮬레이터**: `FRONT_DOOR_REVERSE`(active-low) 극성을 시뮬레이터 패널에 반영
   (`DevSimSensorDlg.cpp`의 `GetSensorByte()`에서 front-door 비트만 XOR).
2. **Win32 Release 빌드 C1001 내부 컴파일러 오류**: vcproj Release|Win32 구성에
   `WholeProgramOptimization`이 잘못 켜져 있던 것이 원인 → 비활성화로 해결.
3. **WinCE Release 빌드 LNK1181**: `.sln`에 `MFS_EJL_WEC7` ARM 구성의 `.Build.0` 항목이
   누락되어 있던 것을 추가.
4. **CDR(카드리더) 시뮬레이터**: 카드 삽입 감지 패널(Track2 없음) → ICC Power On 성공/실패
   Confirm 다이얼로그 → 실제로 `ReadCardData_ISO2()`가 호출될 때만 뜨는 블로킹 Track2 입력
   패널, 순서로 동작. "카드를 빼주세요" 대기도 같은 패널을 라벨만 바꿔 재사용.
5. **EMV tag-57(IC Track2 length error)**: `EMVSim_Tlv_GetVal`에서 tag `"5700"`을
   `DeviceSim.ini [CDR] TrackISO2` 값으로 다이얼로그 없이 조용히 BCD 인코딩하여 채움
   (`EncodeTrack2AsTag57`). Win32 시뮬레이터에서도 EMV 온라인 거래 플로우가 실제로 진행되며
   이 태그가 필요하다.
6. **호스트 TLS 통신 libcurl 마이그레이션**: 클라이언트(아웃바운드) 경로만 `LineCurl`로 대체
   가능(`HOST.CFG`의 `use_curl=1`). AMS 인바운드 리슨 경로는 curl이 accept를 지원하지 않으므로
   항상 OpenSSL(`LineTCP`/`LineHttps`) 유지. `CURLOPT_CONNECT_ONLY`로 TCP+TLS 핸드셰이크만
   수행하고 `curl_easy_send/recv`로 raw I/O. 실제 호스트 대상 테스트로 정상 동작 확인됨.
   curl 라이브러리 자체의 최신 버전 업그레이드는 **보류 중**(아직 미착수).
7. **DLL 위치 정리**: Win32 prebuilt DLL은 `DLL_Win32/`, ARM(WinCE)용은 `DLL_CE/`로 각각
   단일화. 레거시 OpenSSL 1.0.x(`ssleay32.dll`/`libeay32.dll`)는 미사용 확인 후 삭제.
8. **한글 주석 대규모 복원**: 여러 파일에서 인코딩 오판으로 깨졌던 한글 주석을 참고 트리와의
   diff로 복원. 일부(약 100여 자)는 원본과 다르게 수정된 라인이라 영구 복구 불가하여 `?`로
   남아있음.

## 5. 미해결 / 진행 중인 작업

### 5.1 TDL(Triton Data Link) 기반 DCC(Dynamic Currency Conversion) 거래 지원
- **분석만 완료된 상태이며 코드는 아직 없다.**
- Triton STD3 프로토콜의 `q` 접두 FID(`qa`=TIR surcharge, `qc`=DCC)를 사용한 중첩 TLV 블록
  처리가 필요.
- 착수 전 결정 필요:
  1. 전체 구현(약 2,300줄) vs. 단계적 구현(Phase 1: TLV 유틸 + `qa` surcharge만 →
     Phase 2: DCC 조회만 → Phase 3: 전체 DCC 거래+반전).
  2. 테스트 도구(RetailHostSimulator 등)에 `qc` DCC 처리를 추가할지 여부.
  3. DCC 오퍼 화면에 필요한 Screen 리소스 에셋은 별도 준비 필요(도구로 생성 불가).

### 5.2 기타
- curl 라이브러리 버전 업그레이드 — 보류 중.
- prebuilt DLL을 git 히스토리에서 분리(GitHub Releases 등으로 이동)하는 것 — 제안됨, 미실행.

## 6. 작업 시 지켜야 할 규칙

- CP949로 확인된 파일은 절대 UTF-8 가정 도구로 편집하지 말 것 (2번 항목 참조).
- 설계/동작에 대한 명시적 지시가 주어지면 그대로 정확히 구현하고, 직전 지시와 상충하더라도
  최신 지시를 우선한다.
- git force-push, 원격 저장소 교체 등 되돌리기 어렵거나 공유 상태에 영향을 주는 작업은
  사전에 확인받고 진행한다.
- 이 저장소에는 개인키/인증서 파일이 의도적으로 포함되어 있다(사용자 지시에 따름). 빌드
  산출물(`Debug/`, `Release/`, `obj/`, `.vs/` 등)은 `.gitignore`로 제외되어 있다.
