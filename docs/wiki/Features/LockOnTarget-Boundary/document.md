---
제목: LockOnTarget 경계
부제목: LockOnTarget 플러그인 책임과 Maverick 공개 API 경계
최근수정일: 2026-08-12
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---

# LockOnTarget 경계

## 플러그인 책임

| 타입 | 책임 |
|---|---|
| `ULockOnTargetComponent` | 로컬 Pawn의 Target 선택·보관·복제 경계 |
| `UTargetHandlerBase` | 후보 탐색 전략 |
| `ULockOnTargetExtensionBase` | 카메라, 회전, Widget 확장 |
| `UTargetComponent` | 대상 Actor의 Socket, Focus, Priority, Capture 상태 |

## 복제 경계

- Target Info: 복제
- `UTargetComponent` 상태: 비복제
- Capture 설정 변경: 별도 네트워크 동기화 책임

## Maverick 경계

- 플레이어 Dodge Suppression
- Sprint·Action 실행 중 Rotation Extension 억제
- 적 사망 시 Target Release와 Capture Disable
- 필드 Reset 시 Capture Restore
- 플러그인 공개 API만 사용
- Handler·Extension 배치와 조립: Blueprint 책임
