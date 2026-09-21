# 작업 환경 (로컬 PC / 클라우드 공용)

이 문서는 이 저장소를 **로컬 PC**와 **클라우드(Claude Code on the web 등)** 양쪽에서
작업할 때 참고하는 공용 메모입니다. 실제 작업 규칙/배경 지식은 `CLAUDE.md`를 참고하세요.

## 저장소 정보

- **원격(origin)**: https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project
- **기본 브랜치**: `main`

## 환경별 로컬 경로

| 환경 | 경로 |
|---|---|
| 로컬 PC (Windows) | `E:\01_SK.Kim\01.Claude_Project\WinCE_Software_Win32Support` |
| 클라우드 세션 (Linux, 컨테이너) | 세션마다 새로 clone되는 임시 경로 (예: `/home/user/MFS_WinCE_To_Win32_Porting-Project`). 컨테이너 종료 시 사라지므로, 남길 작업은 반드시 커밋 후 `origin`에 push할 것. |

## 로컬 PC에 새로 clone할 때

```
git clone https://github.com/jysk1121/MFS_WinCE_To_Win32_Porting-Project.git "E:\01_SK.Kim\01.Claude_Project\WinCE_Software_Win32Support"
cd "E:\01_SK.Kim\01.Claude_Project\WinCE_Software_Win32Support"
git checkout <작업 브랜치명>
```

## 원칙

- 로컬 PC와 클라우드 세션은 폴더 경로만 다를 뿐, **동일한 git 원격(origin)을 공유**한다.
  한쪽에서 작업한 내용은 반드시 push하고, 다른 쪽에서는 작업 전에 `git fetch` /
  `git pull`로 최신 상태를 받아온 뒤 이어서 작업한다.
- 클라우드 세션의 로컬 경로는 매 세션 새로 생성되는 일시적인 것이므로, 경로 자체에
  의미를 두지 말 것 (하드코딩 금지).
