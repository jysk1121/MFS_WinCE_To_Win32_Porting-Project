# WinCE_Software_Win32Support — 프로젝트 가이드

Nexcom/Eagle ATM 펌웨어(WinCE 6.0 ↔ Win32 시뮬레이터 듀얼 타깃)의 Win32 포팅 작업 공간 전체입니다.
Claude(Claude Code, Claude 웹)가 새 세션에서 작업을 이어갈 때 이 파일을 먼저 읽습니다. 이 파일이
구조·규칙의 **단일 기준 문서**이며, 이전 세션의 상세 이력은 `PROJECT_CONTEXT.md`에 있습니다.

## 0. 최근 세션 인계 사항 (Handoff)

- **로컬 PC / 클라우드 세션 작업 환경**은 `WORKSPACE.md`에 정리되어 있다. 새 세션을
  시작하면 이 파일도 함께 읽을 것. 요지:
  - 로컬 PC 작업 경로: `E:\01_SK.Kim\01.Claude_Project\WinCE_Software_Win32Support`
  - 클라우드 세션은 매번 새로 clone되는 별도의 임시 컨테이너이며, **로컬 PC 파일 시스템에
    직접 접근할 수 없다.** 로컬 PC와 클라우드 세션은 오직 git 원격(origin, GitHub)을 통해서만
    동기화된다 — 한쪽에서 작업한 내용은 반드시 push해야 다른 쪽에서 받아볼 수 있다.
- **열려 있는 PR**: [#1 Add WORKSPACE.md documenting shared local PC / cloud repo layout](https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project/pull/1)
  — 브랜치 `claude/apply-claude-md-mm4gfe` → `main`, `WORKSPACE.md` 추가만 포함(`mergeable_state: clean`).
  새 세션에서 이어갈 때 이 PR의 CI/리뷰 상태를 먼저 확인할 것.
- 코드 변경(기능 구현)은 이번 세션에서 진행하지 않았다. 다음에 착수할 작업은 5절 참고
  (특히 5.1 TDL/DCC는 "전체 구현 vs. 단계적 구현" 결정이 선행되어야 함).

## 1. 프로젝트 개요

- **코드베이스**: C++/MFC. **WinCE 6.0**(ICM_3011, ARMv4I 실장비)과 **Win32 데스크톱 시뮬레이터**를
  동시에 지원하는 듀얼 타깃 빌드.
- 타깃 분기는 `_WIN32_WCE` 매크로와 `Nexcom/Include/Common/Eagle_Common_Define.h`의
  `EMULATION_xxx_DEVICE` 계열 매크로로 제어됩니다.
- Win32 시뮬레이터 코드는 대부분 `#ifndef _WIN32_WCE` / `#if !defined(_WIN32_WCE)` 블록 안에 있으며,
  실제 장비 대신 모달리스 Win32 패널(체크박스, 다이얼로그 등)로 하드웨어를 흉내냅니다.

### 저장소 구조

| 경로 | 내용 |
|---|---|
| `Nexcom/` | **작업 대상 소스 전체.** 솔루션·빌드 스크립트·prebuilt DLL 포함 |
| `Reference/WinCE60_US_TangoPay/` | **읽기 전용 참고용** 별도 구현체. TDL/DCC 기능 분석 시 참고. 수정하지 않는다 |
| `PROJECT_CONTEXT.md` | 이전 로컬 세션의 작업 이력·미해결 과제 요약 |
| `CLAUDE.md` | 이 파일 |

저장소에 **없는** 것: 예전 원본 참고 트리(`WinCE_Software/Nexcom`), 한글 주석 복원에 쓴 기준 트리
(`Nexcom_org`), 빌드 산출물(`Debug/`, `Release/`, `ICM_3011 (ARMv4I)/`, `obj/`, `.vs/` 등 —
`Nexcom/.gitignore`가 제외).

## 2. ⚠️ 최우선 규칙 — 소스 인코딩 (CP949)

`Nexcom/` 소스 대부분은 **CP949(한글) 인코딩 + CRLF**로 저장되어 있습니다. UTF-8을 가정하는 일반적인
파일 편집 도구(Edit/Write)로 건드리면 한글 주석이 깨지고(U+FFFD로 치환), 원본이 없으면 영구
복구가 불가능합니다.

- 파일을 고치기 **전에 반드시** 실제 인코딩을 직접 확인합니다 (Python 예시):
  ```python
  open(path, 'rb').read().decode('cp949')   # errors= 파라미터를 쓰지 말 것 (진짜 실패를 숨김)
  open(path, 'rb').read().decode('utf-8')   # BOM이 있으면 UTF-8인 파일도 있다
  ```
- CP949로 확인되면 **UTF-8 가정 편집 도구를 쓰지 말고** byte-level 스크립트로만 수정합니다.
  (cp949로 디코드 → 필요한 부분만 문자열 치환 → cp949로 다시 인코드 → 바이너리로 write)
- UTF-8(+BOM)로 확인된 파일도 존재합니다 (`EagleSVCLib.cpp`, `LineTCP.cpp`, `DEV_CDR.cpp/.h`,
  `EagleCE_Host.cpp`, 일부 `.vcproj` 등). 이런 파일은 일반 편집 도구를 써도 안전하지만,
  **파일마다 개별적으로 인코딩을 확인**해야 합니다 — 확장자나 위치로 추측하지 마세요.
- 이 저장소는 `core.autocrlf=false`로 커밋되어 CP949/CRLF 소스가 바이트 그대로 저장됩니다. clone 후에도
  `git config core.autocrlf false`를 유지하세요.

## 3. 빌드 구조 요약

경로는 모두 저장소 루트 기준입니다.

| 영역 | 경로 |
|---|---|
| 카드리더 시뮬레이터 | `Nexcom/EagleCE_Device/EagleCE_Device/DEV_CDR.h/.cpp`, `DevSimCDRDlg.h/.cpp`, `DevSimTrack2Dlg.h/.cpp` |
| DIO 센서 시뮬레이터 | `Nexcom/EagleCE_Device/EagleCE_Device/DevSimSensorDlg.cpp` |
| 호스트 통신 라인 구현 (TCP/OpenSSL/curl) | `Nexcom/EagleCE_Host/EagleCE_Host/LineTCP.*`, `LineHttps.*`, `LineCurl.*`, `LineCtrl.h` |
| 호스트 설정 로드/저장 | `Nexcom/EagleCE_Framework/EagleCE_Framework/EagleConfigDefine.h`, `EagleConfig.cpp` |
| 호스트 설정 파일 | `Nexcom/EagleCE_ATM/EagleCE_ATM/**/HOST.CFG` (여러 카피 존재, 빌드 산출물 폴더별) |
| ATM 클라이언트 카드/EMV 처리 로직 | `Nexcom/EagleCE_ATM/EagleCE_ATM/SVC_Manager_Client.cpp`, `SVC_Manager.cpp` |
| 공통 매크로 정의 | `Nexcom/Include/Common/Eagle_Common_Define.h` |
| Win32 prebuilt DLL (canonical) | `Nexcom/DLL_Win32/` |
| ARM(WinCE) prebuilt DLL (canonical) | `Nexcom/DLL_CE/` |
| 빌드 후처리 스크립트 | `Nexcom/BuildFiles/PostBuild_Win32.cmd`, `BuildScript.bat`, `BuildScript_here.bat` |

## 4. 반영된 주요 변경 이력 (요약)

세션별 상세 경위는 `PROJECT_CONTEXT.md` 3절을 참조하세요.

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
7. **DLL 위치 정리**: Win32 prebuilt DLL은 `Nexcom/DLL_Win32/`, ARM(WinCE)용은 `Nexcom/DLL_CE/`로 각각
   단일화. 레거시 OpenSSL 1.0.x(`ssleay32.dll`/`libeay32.dll`)는 미사용 확인 후 삭제.
8. **한글 주석 대규모 복원**: 여러 파일에서 인코딩 오판으로 깨졌던 한글 주석을 참고 트리와의
   diff로 복원. 일부(약 100여 자)는 원본과 다르게 수정된 라인이라 영구 복구 불가하여 `?`로
   남아있음.
9. **`WORKSPACE.md` 추가**: 로컬 PC와 클라우드 세션이 동일 git 원격을 공유하는 구조와 각
   환경의 경로를 문서화. PR #1로 오픈됨 (0절 참고).

## 5. 미해결 / 진행 중인 작업

### 5.1 TDL(Triton Data Link) 기반 DCC(Dynamic Currency Conversion) 거래 지원
- **분석만 완료된 상태이며 코드는 아직 없다.** `Reference/WinCE60_US_TangoPay`의 TDL 구현을 참고.
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
- 공개 저장소에 포함된 개인키/인증서의 폐기·재발급 여부 — 사용자 판단 대기.
- PR #1(`claude/apply-claude-md-mm4gfe`, 옛 구조 기준) 처리 — 미결정.

## 6. Git 구성

- 원격: `https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project.git` (**공개 저장소**), 기본 브랜치 `main`.
  원격에는 `main` 외에 `claude/apply-claude-md-mm4gfe`(옛 구조 기준 PR #1) 브랜치만 있습니다.
- `main`의 히스토리는 `0e108c2`, `ce77312`(옛 구조: Nexcom 내용이 루트) → `a382f82`(현재 구조:
  `Nexcom/`, `Reference/`, `PROJECT_CONTEXT.md`가 루트)로 이어집니다. 옛 커밋의 경로는 루트 기준이므로
  옛 파일 이력을 볼 때는 `git log --follow`를 사용하세요.
- 개인키/인증서(`*.pem`, `*.pfx`)가 `Nexcom/`과 `Reference/` 양쪽에 **사용자 지시로 의도적으로 포함**돼
  있습니다. 새로 추가하거나 다른 곳에 옮기지 말고, 실제 운영 키라면 폐기·재발급을 사용자에게 권합니다.
- 로컬 작업 폴더에는 예전 Nexcom 독립 저장소의 백업(`_backup_Nexcom_git/`)이 있지만 git에는 올라가지 않습니다.

## 7. 작업 규칙

- CP949로 확인된 파일은 절대 UTF-8 가정 도구로 편집하지 않습니다 (2절).
- 설계·동작에 대한 명시적 지시는 그대로 정확히 구현하고, 직전 지시와 상충해도 최신 지시를 따릅니다.
- force-push, 원격 교체, 브랜치 삭제 등 되돌리기 어렵거나 공유 상태에 영향을 주는 git 작업은
  사전에 확인받고 진행합니다. 원격이 공개 저장소이므로 push 전 민감 정보도 함께 확인합니다.
- 보류 중인 작업(5절: TDL/DCC 지원, curl 버전 업그레이드, prebuilt DLL 분리)은 사용자 답변 없이
  착수하지 않습니다.
- `PROJECT_CONTEXT.md`는 보조 자료입니다. 구조·규칙이 충돌하면 이 파일을 우선합니다.
