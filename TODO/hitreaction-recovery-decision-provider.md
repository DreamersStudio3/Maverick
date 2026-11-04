# HitReactionComponent Recovery Decision Provider 정리

## 배경

플레이어의 Airborne recovery 흐름은 `HitReactionComponent`가 공통 실행을 맡고, 애니메이션 notify가 정확한 전환 시점을 알려주는 구조다.

- `MV Airborne Land Detector`
  - Airborne 중 movement mode를 감시한다.
  - 착지하면 현재 HitReaction 몽타주를 `Land` 섹션으로 점프시킨다.
- `MV Recovery Escape Window`
  - recovery 입력 또는 분기 판단이 가능한 짧은 구간을 연다.
- `MV HitReaction Start Getup`
  - 별도 입력/분기가 없을 때 기본 Getup recovery로 넘긴다.

Enemy도 같은 Airborne recovery 흐름을 재사용하려면, recovery window가 열리는 순간에 `Getup`을 할지 `EscapeDodge`를 할지 판단할 지점이 필요했다.

## 왜 HitReactionComponent를 수정했는가

`HitReactionComponent`는 이미 다음 책임을 갖고 있다.

- HitReaction 액션 시작
- Airborne 착지 감지
- recovery window open 이벤트 수신
- Getup/EscapeDodge recovery 액션 전환 실행

따라서 recovery window가 열렸을 때 "외부 정책이 선택한 recovery를 실행"하는 진입점은 `HitReactionComponent` 안에 있는 것이 자연스럽다.

다만 `HitReactionComponent`가 Enemy, AIController, 타겟 거리 같은 Enemy 전용 정책을 직접 알면 공통 컴포넌트 책임이 흐려진다. 그래서 Enemy 타입을 직접 참조하지 않고 `IMVHitReactionRecoveryDecisionProvider` 인터페이스만 호출하도록 수정했다.

## 추가한 구조

새 인터페이스:

- `Source/Maverick/Public/Interface/MVHitReactionRecoveryDecisionProvider.h`

역할:

- recovery window가 열렸을 때 owner-specific recovery 선택을 제공한다.
- 반환 가능한 결정은 다음 세 가지다.
  - `None`
  - `Getup`
  - `EscapeDodge`

`HitReactionComponent` 변경:

- `TryStartProviderRecoveryAction()` 추가
- `TryHandleRecoveryWindowOpened()` 흐름 변경
  - 먼저 기존처럼 buffered movement 입력을 소비한다.
  - 입력으로 recovery가 처리되지 않으면 owner가 `IMVHitReactionRecoveryDecisionProvider`를 구현했는지 확인한다.
  - 구현되어 있으면 provider 결정에 따라 기존 recovery 실행 함수를 호출한다.

## 왜 이런 방식이 깔끔한가

`HitReactionComponent`는 여전히 공통 실행만 담당한다.

- `Getup` 실행은 기존 `TryStartDefaultRecoveryAction()` 사용
- `EscapeDodge` 실행은 기존 `TryStartEscapeDodgeRecoveryAction()` 사용
- Enemy 전용 거리/방향 계산은 알지 않는다.

Enemy는 정책만 담당한다.

- 타겟이 없는지 확인
- 타겟과의 거리 확인
- 가까우면 타겟 반대 방향으로 EscapeDodge 결정
- 멀면 Getup 결정

즉 실행 책임과 판단 책임이 분리된다.

## StateTree에 넣지 않은 이유

Airborne recovery 분기 시점은 StateTree tick이 아니라 애니메이션 notify가 여는 recovery window다.

StateTree에서 처리하면 다음 문제가 생길 수 있다.

- notify timing과 StateTree transition timing이 섞인다.
- recovery window가 짧을 경우 tick 타이밍에 따라 반응이 늦을 수 있다.
- `MV HitReaction Start Getup` notify와 StateTree decision이 중복 실행될 수 있다.
- StateTree가 몽타주 내부 recovery 분기 정책까지 알아야 한다.

그래서 StateTree는 HitReaction 상태의 상위 흐름을 관리하고, recovery window 내부 분기는 `HitReactionComponent + provider` 구조로 처리하는 쪽이 더 명확하다.

## 데이터 테이블 사용

새 테이블은 만들지 않는다.

- Enemy는 기존 `DT_HR_E1`을 계속 사용한다.
- 멀 때는 기존 Getup row를 사용한다.
- 가까울 때는 `EscapeDodge` recovery row가 필요하다.

예상 row 예시:

- `HR_E1_Getup_F_01`
- `HR_E1_Getup_B_01`
- `HR_E1_EscapeDodge_F_B_01`
- `HR_E1_EscapeDodge_F_L_01`
- `HR_E1_EscapeDodge_F_R_01`
- `HR_E1_EscapeDodge_B_F_01`

## 현재 Enemy 정책

`AMVEnemy`가 `IMVHitReactionRecoveryDecisionProvider`를 구현한다.

- Airborne이 아니면 provider decision을 하지 않는다.
- 타겟이 없으면 Getup
- 타겟 거리가 `AirborneEscapeDodgeDistance`보다 멀면 Getup
- 타겟이 가까우면 타겟 반대 방향 EscapeDodge

기본 거리 값:

- `AirborneEscapeDodgeDistance = 500.0f`

## 유지보수 기준

Enemy recovery 정책이 단순한 동안은 `AMVEnemy` 구현으로 충분하다.

다만 적 종류별로 다른 recovery 정책이 많아지면, `AMVEnemy`에서 분기문을 늘리지 말고 별도 decision component 또는 strategy object로 분리한다.
