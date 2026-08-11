---
제목: LockOnTarget 경계
부제목: LockOnTarget 플러그인 책임과 Maverick 공개 API 경계
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---
# LockOnTarget 경계

플러그인은 targeting 정책과 상태를 소유한다.

- `ULockOnTargetComponent`: 로컬 Pawn의 target 선택·보관·복제 경계. target info는 복제되지만 `UTargetComponent` 자체 상태는 비복제이므로 capture 설정 변경은 별도 네트워크 동기화를 고려한다.
- `UTargetHandlerBase`: 후보 탐색 전략.
- `ULockOnTargetExtensionBase`: 카메라, 회전, widget 같은 부가기능.
- `UTargetComponent`: 대상 Actor의 socket/focus/priority/capture 상태.

Maverick은 플레이어 dodge suppression, sprint, Action 실행 중 rotation extension 억제, 적 사망 시 target release/capture disable, 필드 reset 시 capture restore처럼 공개 API만 호출한다. Blueprint에 실제 배치된 handler/extension은 에디터 확인이 필요하다.
