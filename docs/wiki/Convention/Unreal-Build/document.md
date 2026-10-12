---
제목: Unreal Editor 빌드
부제목: Windows 일반 빌드와 Live Coding 기본 바이너리 일치
최근수정일: 2026-10-07
최근수정자: Codex
관련문서:
  - "[[Convention/Header-Documentation/document|C++ 헤더 책임 문서화]]"
  - "[[Features/AI-StateTree/document|AI StateTree]]"
---

# Unreal Editor 빌드

## 일반 빌드

Editor의 미저장 에셋 백업·저장 후 프로세스 종료, 일반 빌드 완료 후 Editor 실행
Live Coding 지속 사용 시 `-NoLiveCoding` 제외, 메모리 부족 시 `-MaxParallelActions=1` 적용

```powershell
& 'D:/UE_5.8/Engine/Build/BatchFiles/Build.bat' MaverickEditor Win64 Development '-Project=D:/Project/Maverick/Maverick.uproject' -WaitMutex -NoHotReloadFromIDE -NoXGE -MaxParallelActions=1
```

- `-NoHotReloadFromIDE`: IDE Hot Reload 제외, Live Coding 지원 비활성화와 별개
- `-NoXGE`: 해당 일반 빌드의 XGE 실행 제외, 사용자 전역 설정 변경 없음
- `-MaxParallelActions=1`: 병렬 컴파일 수 제한, 코드의 unity build 규칙 변경 없음

## Live Coding 연결 실패 판정

- `Result: Succeeded` 뒤 패치 연결 오류 가능, C++ 컴파일 성공과 실행 중 코드 반영 분리
- `LNK2011`: PCH 생성 오브젝트 연결 누락, [Microsoft 오류 정의](https://learn.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-error-lnk2011?view=msvc-170)
- `Not enough space near function`: 해당 함수 변경의 패치 설치 불가 메시지, 전체 컴파일 성공만으로 적용 보장 불가
- UBT `TargetRules.bWithLiveCoding`: Windows Editor 기본 활성, `-NoLiveCoding`으로 비활성
- Windows `bCreateHotPatchableImage`: `bWithLiveCoding`을 기본값으로 사용, 활성 빌드의 링크 옵션 `/FUNCTIONPADMIN:6`
- 판정 기준: Live Coding 연결 성공·Editor 성공 로그·수정한 함수의 실제 출력 또는 동작 변화 확인

## 2026-10-07 복구 검증

Codex / Windows / UE 5.8.3 / MSVC 14.44.35228 실제 실행

- 이전 일반 빌드의 `-NoLiveCoding` 사용 후 Live Coding 전환에서 PCH 재생성·Maverick과 LockOnTarget 모듈의 패치 연결 실패 확인
- 옵션 제외 일반 Editor 빌드 24개 작업 통과, Maverick·LockOnTarget DLL 링크 응답 파일의 `/FUNCTIONPADMIN:6` 확인
- 미저장 플레이어 Blueprint 디스크 원본 백업·편집 저장 후 정상 종료, 새 Editor 프로세스에서 임시 `GetTargetRotation` 로그 추가·제거 패치 2회 성공
- 실제 함수 호출로 임시 로그 출력과 제거 확인, 최종 C++ 파일 SHA256과 진단 전 백업 일치, 최종 미저장 패키지 0개
- 이번 복구의 추가 C++ 기능 변경 없음, 이전 요청의 기능 변경과 사용자의 에셋 편집 보존
- 잔여 진단: 복구 전후 Live Coding의 `Cannot find image section .voltbl` 출력, 패치 연결·실제 함수 변경 반영은 성공; 메시지 원인과 다른 함수 영향 미확정
- 검증 범위: 현재 Windows 호스트의 로컬 Development Editor 빌드·패치·함수 호출 실제 실행, 패키징·네트워크·전체 게임 재시험 미실행 — 이번 빌드 복구 범위 밖, 해당 동작 보장 제외

원본 빌드·패치 로그와 백업: `Saved/LiveCodingRecovery`, Git 제외
