# Death Component 리팩터링 작업

## 목표

사망 표현과 후속 부활 흐름을 `HitReactionComponent`에서 분리한다.

`UMVStatComponent`는 HP 0 판정과 사망 이벤트 발행만 담당하고, `UMVDeathComponent`가 캐릭터 단위의 사망 표현을 소유한다. 플레이어의 월드 리셋과 부활은 `UMVRespawnSubsystem`이 DeathComponent 이벤트를 구독해 처리한다.

## 진행 상황

- [x] 기존 death 책임 위치 확인.
- [x] `UMVDeathComponent`를 추가해 `OnDeathStarted` 구독, death action 선택, 사망 표현 완료 이벤트를 소유하게 한다.
- [x] `UMVAnimNotify_DeathDissolve`가 RespawnSubsystem 대신 DeathComponent에 신호를 전달하게 한다.
- [x] `UMVRespawnSubsystem`이 플레이어 DeathComponent 이벤트를 구독해 overlay/loading 전환을 처리하게 한다.
- [x] `UMVHitReactionComponent`에서 death action/respawn 직접 호출 책임을 제거한다.
- [x] 문서와 기존 death flow TODO를 새 책임 경계에 맞게 갱신한다.
- [x] Landing death 선택 기준을 착지 속도에서 낙하 높이로 바꾼다.
