---
제목: "Q 스킬 적중 연계"
부제목: "Q1부터 Q3까지의 단계 전진과 입력 창·HUD·지연 적중 계약"
최근수정일: 2026-08-17
최근수정자: "No-Jyun"
관련문서:
  - "[[Features/Combat/Combat-System/document|Maverick 전투 시스템]]"
  - "[[Features/Hit-Stat-HitReaction/document|Hit, Stat, HitReaction]]"
  - "[[Features/Input-to-Action/document|입력에서 Action 실행까지]]"
---

# Q 스킬 적중 연계

## 단계 전진

```mermaid
flowchart TD
    Q1["Q1 사용"] --> Q1Hit{"AttackInstanceId가 일치하는 적중"}
    Q1Hit -->|적중| Q2["Q2 단계 활성화"]
    Q1Hit -->|미적중| Q1Retry["입력 창 안에서 Q1 재시도"]
    Q2 --> Q2Hit{"AttackInstanceId가 일치하는 적중"}
    Q2Hit -->|적중| Q3["Q3 단계 활성화"]
    Q2Hit -->|미적중| Q2Retry["고정 입력 창 안에서 Q2 재시도"]
    Q3 --> Q3Use["Q3 사용 즉시 체인 완료"]
    Q3Use --> Q1Reset["Q1 초기화"]
    Q1Retry --> Q1Hit
    Q2Retry --> Q2Hit
```

| 단계 | `ChainAdvancePolicy` | 전진 조건 |
|---|---|---|
| Q1 | `OnHitConfirmed` | 현재 공격 실행의 적중 확인 후 Q2 활성화 |
| Q2 | `OnHitConfirmed` | 현재 공격 실행의 적중 확인 후 Q3 활성화 |
| Q3 | `Immediate` | 사용 성공 시 적중 여부와 무관하게 체인 완료·Q1 초기화 |

- 적중 확인 필터: 공격자, 현재 Ability, 활성 상태, `AttackInstanceId`, 현재 Row 정책, 단계별 중복 처리 여부
- `OnHitConfirmed` 재시도: 동일 단계 유지와 다음 단계 전진 지연
- 기본 정책 `Immediate`: 기존 기본 공격·단일 스킬·일반 연계 동작 유지

## 체인 시간 기준

| 상태 | 의미 | 갱신 기준 |
|---|---|---|
| `InputWindowCloseTime` | 현재 단계를 성공시킬 수 있는 전체 마감 | 단계 진입 시 설정, `OnHitConfirmed` 재시도로 연장 금지 |
| `LastStageActivationTime` | 직전 시도 후 `InterStageCooldown` 계산 기준 | 각 Ability 종료 시 갱신 |

- 입력 판정 순서: `InterStageCooldown` 확인 후 입력 창 만료·체인 초기화 확인
- 입력 창 만료가 쿨다운보다 빠른 경우: 남은 쿨다운 종료까지 입력 차단 후 Q1 재시작
- Q1·Q2 재시도: 전체 마감 유지와 시도별 쿨다운 재시작의 동시 적용

## HUD

| 표시 | 기준 |
|---|---|
| 메인 슬롯 단계 | 입력 창 유효 시 현재 단계, 만료 시 Q1 |
| 메인 슬롯 쿨다운 | `bChainActive` 동안 현재 단계의 `InterStageCooldown` |
| 작은 체인 타이머 | 입력 창 유효·현재 단계 인덱스 1 이상 |
| 작은 타이머 아이콘 | 현재 단계의 직전 단계 아이콘 |

- Q2 사용 가능 상태: 작은 타이머에 Q1 아이콘
- Q3 사용 가능 상태: 작은 타이머에 Q2 아이콘
- 입력 창 만료 후 쿨다운 잔존: 작은 타이머 숨김, Q1 메인 아이콘에 남은 쿨다운 표시

## 지연 적중 수명 계약

- `OnHitConfirmed` 체인 전진은 Ability 활성 상태에서 수신한 적중만 허용
- 이동형 Q2 ShockWave: 최대 트레이스 도달 시점까지 Ability NotifyState 활성 구간 유지
- Ability 종료가 트레이스 종료보다 빠른 경우: 대미지는 적용되지만 체인 적중 확인은 거절
- ShockWave 속도·최대 거리·트레이스 수명 변경 시 Ability NotifyState 종료 시점 동반 검토
