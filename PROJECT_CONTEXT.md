# Nexcom / Eagle ATM — WinCE → Win32 Porting 작업 컨텍스트

이 문서는 Claude Code(터미널/로컬 세션)에서 진행한 작업 내용의 요약본입니다. 다른 세션(Claude 웹
포함)에서 맥락을 다시 설명하지 않고 이어가기 위한 참고 자료입니다.

> 이 저장소의 **현재 구조와 작업 규칙은 루트 `CLAUDE.md`가 기준**입니다.
> 이 문서는 작업 이력과 미해결 과제를 담은 보조 자료이며, 둘이 충돌하면 `CLAUDE.md`를 따르세요.
>
> - claude.ai/code에서 이 GitHub 저장소를 연결하면 `CLAUDE.md`가 자동으로 로드되어, 코드 편집을
>   포함한 작업을 그대로 이어갈 수 있습니다.
> - 저장소 접근이 없는 일반 claude.ai 채팅에서는 `CLAUDE.md`와 이 문서의 내용을 대화 맨 위에
>   붙여넣고 시작하세요. 이 경우 코드는 직접 편집할 수 없습니다.

---

## 1. 프로젝트 개요

- **코드베이스**: Nexcom/Eagle ATM 펌웨어. C++/MFC, **WinCE 6.0**(ICM_3011, ARMv4I 실장비)과
  **Win32 데스크톱 시뮬레이터**를 동시에 지원하는 듀얼 타깃 빌드.
- 타깃 분기는 `_WIN32_WCE` 매크로와 `Eagle_Common_Define.h`의 `EMULATION_xxx_DEVICE` 계열
  매크로로 제어됨.
- **저장소 루트**: `WinCE_Software_Win32Support/` (원격
  `https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project.git`, 기본 브랜치 `main`).
  - `Nexcom/` — 작업 대상 소스 전체. 아래 문서의 `Nexcom/...` 경로는 저장소 루트 기준입니다.
  - `Reference/WinCE60_US_TangoPay/` — TDL/DCC 기능 분석 시 참고한 별도 구현체(읽기 전용, git에 포함됨).
  - `CLAUDE.md`, `PROJECT_CONTEXT.md` — 세션 인수인계용 문서.
- git에 **포함되지 않은** 옛 참고 트리(로컬에만 존재했음):
  - `WinCE_Software/Nexcom` — 예전 원본 참고 트리
  - `Nexcom_org` — 한글 주석 복원 시 사용한 "손상되지 않은" 기준 트리

## 2. ⚠️ 최우선 규칙 — 소스 인코딩 (CP949)

Nexcom 소스 대부분은 **CP949(한글) 인코딩 + CRLF**로 저장되어 있습니다. Edit/Write 같은
UTF-8 가정 도구로 건드리면 한글 주석이 깨집니다(U+FFFD).

- 파일을 고치기 **전에 반드시** 아래로 실제 인코딩을 직접 확인할 것:
  ```python
  open(path, 'rb').read().decode('cp949')   # errors= 파라미터 쓰지 말 것 (진짜 실패를 숨김)
  open(path, 'rb').read().decode('utf-8')   # BOM 있는 파일은 UTF-8인 경우도 있음
  ```
- CP949로 확인되면 **Edit/Write 도구를 쓰지 말고** byte-level Python 스크립트로만 수정.
  (이번 세션에서 이 규칙을 무시하고 Edit을 썼다가 `DEV_CDR.cpp/.h`를 두 번 더 깨뜨린 전례 있음 —
  `Nexcom_org` 기준 diff 복원 스크립트로 복구했으나 약 138자는 영구 복구 불가로 `?` 처리됨.)
- UTF-8(+BOM)로 확인된 파일 예: `EagleSVCLib.cpp`, `LineTCP.cpp`, `DEV_CDR.cpp/.h`,
  `EagleCE_Host.cpp`, `EagleCE_Device.vcproj` 등 4개 vcproj — 이런 파일은 Edit 도구 사용 가능.
- 개인 메모리 파일에도 이 규칙이 저장되어 있음: `cp949-source-encoding.md`.

## 3. 이번 세션에서 완료한 작업

1. **DIO 센서 시뮬레이터 버그** — `FRONT_DOOR_REVERSE`(active-low) 극성 미반영.
   `DevSimSensorDlg.cpp`의 `GetSensorByte()`에서 front-door 비트만 `ACTIVE_LOW_MASK`로 XOR.
2. **Win32 Release 빌드 C1001 내부 컴파일러 오류** — 5개 vcproj의 Release|Win32에
   `WholeProgramOptimization="1"`이 잘못 남아있던 것이 원인 → `"0"`으로 수정.
3. **WinCE Release 빌드 LNK1181** — `.sln`에 `MFS_EJL_WEC7` ARM 구성의 `.Build.0` 항목 누락 →
   추가하여 해결.
4. **LNK4221 경고** — 사용자 확인 결과 무해하므로 그대로 둠(수정 안 함).
5. **PinPad 모달리스 패널** — 이전 세션에서 이미 구현 완료된 것을 최종 확인만 함(수정 없음).
6. **CDR(카드리더) 시뮬레이터 재설계** (여러 차례 반복 후 최종 확정된 3단계 설계):
   - 카드 삽입 감지용 모달리스 패널(`DevSimCDRDlg.h/.cpp`) — Track2 없음, OK만.
   - ICC Power On 성공/실패를 별도 Confirm 다이얼로그로 분리.
   - **블로킹** Track2 입력 패널(`DevSimTrack2Dlg.h/.cpp`) — ATM이 실제로
     `ReadCardData_ISO2()`를 호출할 때만(마그네틱/폴백 경로) 표시, 별도 스레드+이벤트로 블로킹.
   - "카드를 빼주세요" 대기 다이얼로그 — 같은 CDR 패널을 라벨만 바꿔 재사용(`ShowForRemoval`),
     `Proc_Client_EMV_Remove_Card()`의 모든 종료 경로에 연결.
7. **EMV tag-57("IC Track2 length error") 오류로 거래가 취소되는 문제** —
   `EMVSim_Tlv_GetVal`에서 tag `"5700"`을 `DeviceSim.ini [CDR] TrackISO2` 값으로 **다이얼로그 없이
   조용히** BCD 인코딩하여 채우도록 복원 (`EncodeTrack2AsTag57`). 처음엔 사용자가 "무시해도 된다"고
   했으나 실제 트레이스 로그 분석 결과 Win32에서도 EMV 플로우가 진행되며 tag-57이 필요함을 확인,
   설명 후 재적용함.
8. **카드 삽입 다이얼로그가 OK 눌러도 안 사라지는 버그** — `Show()`에 `if (IsCardDetected()) return;`
   가드 추가로 해결.
9. **libcurl로 호스트 TLS 통신 마이그레이션** (클라이언트 아웃바운드 경로만; AMS 인바운드
   리슨 경로는 curl이 accept를 못하므로 OpenSSL 유지):
   - `LineCtrl.h`에 `LINE_CURL` 열거값 추가.
   - `LineCurl.h/.cpp` 신규 구현 — `CURLOPT_CONNECT_ONLY` 모드로 TCP+TLS만 수행하고
     `curl_easy_send/recv`로 raw I/O, `CRITICAL_SECTION`으로 송수신 스레드 경합 보호.
   - `EagleConfig`에 `strUseCurl` 설정 추가(`HOST.CFG`의 `use_curl` 키), `DEV_Manager.cpp`에서
     이 설정값으로 `LINE_TCP`/`LINE_CURL` 선택.
   - `EagleCE_Host.cpp`의 `Set_FlowInform()`에서 AMS(인바운드)는 curl 선택 시에도 강제로
     `LINE_TCP`(OpenSSL) 사용하도록 처리.
   - **실제 호스트 대상 테스트로 정상 동작 확인 완료**(TLS 핸드셰이크 성공, Triton 메시지 교환 성공).
   - curl 최신 버전으로의 업그레이드는 **사용자가 명시적으로 보류 요청** — 미착수.
10. **레거시 OpenSSL DLL 정리** — `DLL_CE`의 `ssleay32.dll`/`libeay32.dll`(구버전, 미사용 확인)
    삭제.
11. **Win32/ARM DLL 폴더 통일**:
    - Win32용 prebuilt DLL은 신규 `Nexcom/DLL_Win32/`로 이동, `PostBuild_Win32.cmd`가 여기서 복사.
    - ARM용은 `Nexcom/DLL_CE/`로 단일화(중복 폴더였던 `EagleCE_Host/EagleCE_Host/Dll/` 삭제).
12. **vcproj Solution Explorer 한글 깨짐(mojibake) 수정** — 6개 vcproj 파일이
    `encoding="ks_c_5601-1987"`로 선언되어 있었지만 실제로는 이미 깨진 UTF-8로 저장되어 있던 것을
    올바른 CP949 한글 필터명으로 복원.
13. **Nexcom 전체 소스의 한글 주석 대규모 복원** — `Nexcom_org` 기준 트리와 diff하여 22개 파일,
    약 19,883자 복원(약 150자는 코드가 함께 바뀐 부분이라 영구 복구 불가, `?`로 대체).
14. **Git 저장소 초기화 및 업로드** (초기 구조, 커밋 `0e108c2`·`ce77312`):
    - 처음에는 `Nexcom` 폴더만 대상으로 git init (당시 저장소 루트 = Nexcom 내용, 참고 트리는 제외).
    - `.gitignore` 작성 (`.claude/`, 빌드 산출물, IDE 상태 파일 등 제외; **개인키/인증서는
      사용자 지시에 따라 그대로 포함**). 현재는 `Nexcom/.gitignore`로 위치가 바뀜.
    - 최초 업로드: `https://github.com/jysk1121/MFS.git` → 이후 원격지를 교체하여
      **현재 origin = `https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project.git`**
      (사용자가 "origin을 이 저장소로 교체" 옵션 선택, 완료됨). MFS.git 쪽 저장소 이름이
      바뀐 것처럼 보였던 것은 실제 rename이 아니라 다른 URL/탭을 보고 있었을 가능성이 높음
      (git 명령으로는 원격 저장소 이름을 바꿀 수 없음 — GitHub 소유자가 UI/API로만 가능).
15. **워크스페이스 전체를 하나의 저장소로 통합** (커밋 `a382f82`, 이후 `5051663`):
    - 상위 폴더(`WinCE_Software_Win32Support`)를 저장소 루트로 삼아 `Nexcom/`, `Reference/`,
      `PROJECT_CONTEXT.md`를 함께 추적. Nexcom 안의 독립 `.git`은 로컬 `_backup_Nexcom_git/`으로
      옮겨 보관(git 미포함).
    - 기존 `main` 히스토리를 보존하는 fast-forward 커밋으로 구조를 변경(강제 push 없음). 옛 커밋의
      경로는 루트 기준이므로 옛 파일 이력은 `git log --follow`로 추적.
    - `core.autocrlf=false`로 커밋하여 CP949/CRLF 소스를 바이트 그대로 저장.
    - `Reference/`의 개인키·`.pfx`를 포함해 공개 저장소에 올라갔음(사용자 결정). 실제 운영 키라면
      폐기·재발급 필요.
    - 통합 중 임시로 올렸던 `workspace-full-sync` 브랜치(= `main`과 같은 스냅샷)는 삭제함. 원격에는
      `main`과 `claude/apply-claude-md-mm4gfe`(옛 구조 기준 PR #1)만 남아 있음.
    - 루트 `CLAUDE.md` 추가 및 이 문서의 경로/구조 설명 갱신. 이후 `Nexcom/CLAUDE.md`(옛 Nexcom 단독
      시기 문서)를 루트 `CLAUDE.md`에 통합하고 삭제함.

## 4. 미해결 / 보류 중인 작업

### 4.1 TDL(Triton Data Link) 기반 DCC(Dynamic Currency Conversion) 거래 지원
- **분석만 완료, 코드는 전혀 작성하지 않음.**
- `Reference/WinCE60_US_TangoPay`의 TDL 구현을 참고. Triton STD3 프로토콜의 `q` 접두 FID
  (`qa`=TIR surcharge, `qc`=DCC)를 사용한 중첩 TLV 블록 처리가 필요.
- 사용자에게 확인 필요한 사항:
  1. 전체 기능을 한번에 구현할지(약 2,300줄, 1.5~2주), 단계적으로 할지
     (Phase 1: TLV 유틸+`qa` surcharge만, ~400줄/1일 → Phase 2: DCC 조회만 → Phase 3: 전체
     DCC 거래+반전).
  2. 테스트 도구 `RetailHostSimulator`에 `qc` DCC 처리를 추가할지 여부(현재 없음).
  3. DCC 오퍼 화면은 Screen/ 리소스 에셋이 필요한데, 이는 Claude가 만들 수 없는 영역이라
     별도 준비 필요.
- **다음 액션**: 사용자가 위 3가지에 답하기 전까지는 착수하지 않음.

### 4.2 기타
- curl 최신 버전 업그레이드 — 보류 중(사용자 요청).
- prebuilt DLL을 git 히스토리에서 빼고 GitHub Releases 등으로 옮기는 것 — 제안만 했고
  아직 실행 안 함 (curl 버전 업그레이드 시점에 같이 고려하기로 함).
- 공개 저장소에 포함된 개인키/인증서(`Nexcom/`, `Reference/`)의 폐기·재발급 여부 — 사용자 판단 대기.
- PR #1(`claude/apply-claude-md-mm4gfe`, 옛 구조 기준) 처리 — 미결정.

## 5. 핵심 파일 위치 요약

| 영역 | 경로 |
|---|---|
| 카드리더 시뮬레이터 | `Nexcom/EagleCE_Device/EagleCE_Device/DEV_CDR.h/.cpp`, `DevSimCDRDlg.h/.cpp`, `DevSimTrack2Dlg.h/.cpp` |
| DIO 센서 시뮬레이터 | `Nexcom/EagleCE_Device/EagleCE_Device/DevSimSensorDlg.cpp` |
| curl 라인 구현 | `Nexcom/EagleCE_Host/EagleCE_Host/LineCurl.h/.cpp`, `LineCtrl.h` |
| 호스트 설정 로드/저장 | `Nexcom/EagleCE_Framework/EagleCE_Framework/EagleConfigDefine.h`, `EagleConfig.cpp` |
| 호스트 설정 파일 | `Nexcom/EagleCE_ATM/EagleCE_ATM/**/HOST.CFG` (여러 카피 존재) |
| ATM 클라이언트 카드 처리 로직 | `Nexcom/EagleCE_ATM/EagleCE_ATM/SVC_Manager_Client.cpp` |
| 공통 매크로 정의 | `Nexcom/Include/Common/Eagle_Common_Define.h` |
| Win32 prebuilt DLL | `Nexcom/DLL_Win32/` |
| ARM prebuilt DLL | `Nexcom/DLL_CE/` |
| 빌드 후처리 스크립트 | `Nexcom/BuildFiles/PostBuild_Win32.cmd`, `BuildScript.bat`, `BuildScript_here.bat` |

## 6. 작업 방식/선호 관련 규칙 (개인 메모리에도 저장됨)

- CP949 소스는 Edit/Write 금지, 반드시 byte-level Python 스크립트 사용 (위 2번 항목).
- 설계 지시가 명시적으로 주어지면 정확히 그대로 구현할 것 — 직전에 준 지시라도 사용자가
  번복하면 그 번복을 그대로 반영.
- 위험하거나 되돌리기 어려운 git 작업(강제 push, 원격 교체 등)은 사전에 확인받고 진행.

---
*이 문서는 로컬 Claude Code 세션에서 작성·갱신한 요약본입니다. 최신 상태는 git 저장소
(`https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project.git`, `main`)를 기준으로 하며,
세부 구현은 실제 코드를, 현재 구조와 규칙은 루트 `CLAUDE.md`를 참조하세요.*
