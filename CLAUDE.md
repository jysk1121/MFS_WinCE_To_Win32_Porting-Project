# WinCE_Software_Win32Support — 워크스페이스 가이드

이 저장소는 Nexcom/Eagle ATM 펌웨어(WinCE 6.0 ↔ Win32 시뮬레이터 듀얼 타깃)의 Win32 포팅
작업 공간 전체입니다. Claude(Claude Code, Claude 웹)가 새 세션에서 작업을 이어갈 때 이 파일을
먼저 읽습니다. 세부 내용은 아래 문서로 나뉘어 있습니다.

- 소스/빌드/변경 이력/진행 중 작업: @Nexcom/CLAUDE.md
- 이전 세션 요약본(참고용, 아래 "문서 관계" 참조): `PROJECT_CONTEXT.md`

## 저장소 구조

| 경로 | 내용 |
|---|---|
| `Nexcom/` | **작업 대상 소스 전체.** C++/MFC, WinCE(ICM_3011, ARMv4I) + Win32 시뮬레이터. 솔루션·빌드 스크립트·prebuilt DLL 포함 |
| `Reference/WinCE60_US_TangoPay/` | **읽기 전용 참고용** 별도 구현체. TDL/DCC 기능 분석 시 참고. 수정하지 않는다 |
| `PROJECT_CONTEXT.md` | 이전 로컬 세션의 작업 요약 |
| `CLAUDE.md` | 이 파일 |

저장소에 **없는** 것: 예전 원본 참고 트리(`WinCE_Software/Nexcom`), 한글 주석 복원에 쓴 기준 트리
(`Nexcom_org`), 빌드 산출물(`Debug/`, `Release/`, `ICM_3011 (ARMv4I)/`, `obj/`, `.vs/` 등 —
`Nexcom/.gitignore`가 제외).

## ⚠️ 최우선 규칙 — 소스 인코딩 (CP949)

`Nexcom/` 소스 대부분은 **CP949(한글) + CRLF**입니다. UTF-8을 가정하는 Edit/Write 도구로 고치면
한글 주석이 U+FFFD로 깨지고, 원본이 없으면 영구 복구가 불가능합니다.

- 파일을 고치기 전에 인코딩을 직접 확인합니다 (`errors=` 파라미터 없이 디코드 시도).
  ```python
  open(path, 'rb').read().decode('cp949')
  open(path, 'rb').read().decode('utf-8')   # UTF-8(+BOM) 파일도 일부 존재
  ```
- CP949로 확인되면 **Edit/Write를 쓰지 말고** byte-level 스크립트로만 수정합니다
  (cp949 디코드 → 필요한 부분만 치환 → cp949 인코드 → 바이너리 write).
- 확장자나 위치로 추측하지 말고 **파일마다** 확인합니다. 자세한 예외 목록은 `Nexcom/CLAUDE.md` 2절.
- 이 저장소는 `core.autocrlf=false`로 커밋되었습니다. 줄바꿈 변환이 일어나지 않게, clone 후에도
  `git config core.autocrlf false`를 유지하세요.

## Git 구성

- 원격: `https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project.git` (**공개 저장소**), 기본 브랜치 `main`.
- `main`의 히스토리는 `0e108c2`, `ce77312`(옛 구조: Nexcom 내용이 루트) → `a382f82`(현재 구조:
  `Nexcom/`, `Reference/`, `PROJECT_CONTEXT.md`가 루트)로 이어집니다. 옛 커밋의 경로는 루트 기준이므로
  옛 파일 이력을 볼 때는 `git log --follow`를 사용하세요.
- 원격에 `workspace-full-sync`(현재 `main`과 같은 스냅샷)와 `claude/apply-claude-md-mm4gfe`(옛 구조 기준
  PR #1) 브랜치가 남아 있습니다.
- 개인키/인증서(`*.pem`, `*.pfx`)가 `Nexcom/`과 `Reference/` 양쪽에 **사용자 지시로 의도적으로 포함**돼
  있습니다. 새로 추가하거나 다른 곳에 옮기지 말고, 실제 운영 키라면 폐기·재발급을 사용자에게 권합니다.
- 로컬 작업 폴더에는 예전 Nexcom 독립 저장소의 백업(`_backup_Nexcom_git/`)이 있지만 git에는 올라가지 않습니다.

## 문서 관계

`PROJECT_CONTEXT.md`는 이전 세션들의 작업 이력(3절)과 미해결 과제(4절)를 담은 보조 자료이며,
현재 저장소 구조에 맞게 갱신되어 있습니다. 구조·규칙이 충돌하면 이 파일과 `Nexcom/CLAUDE.md`를
우선합니다.

## 작업 규칙

- 설계·동작에 대한 명시적 지시는 그대로 정확히 구현하고, 직전 지시와 상충해도 최신 지시를 따릅니다.
- force-push, 원격 교체, 브랜치 삭제 등 되돌리기 어렵거나 공유 상태에 영향을 주는 git 작업은
  사전에 확인받고 진행합니다. 원격이 공개 저장소이므로 push 전 민감 정보도 함께 확인합니다.
- 보류 중인 작업(TDL/DCC 지원, curl 버전 업그레이드, prebuilt DLL 분리)은 사용자 답변 없이
  착수하지 않습니다 (`Nexcom/CLAUDE.md` 5절).
