---
제목: 사망과 필드 전환
부제목: 치명 판정부터 부활·필드 상태 복구까지의 런타임 순서
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---
# 사망과 필드 전환

1. StatComponent가 lethal을 한 번만 확정하고 `FMVDeathContext`를 발행한다.
2. 실행 중인 Action이 `HR_*`이면 DeathComponent는 사망 표현을 보류한다. `MVAnimNotify_HitReactionDeathHandoff` 또는 해당 Action 종료가 handoff를 완료한다.
3. DeathComponent가 Death Action, ragdoll, immediate 중 하나를 선택한다. Death Action의 실행은 ActionComponent에 위임하고, dissolve/overlay/presentation 이벤트는 DeathComponent가 관리한다.
4. `UMVDeathRespawnFlow`는 `UMVFieldTransitionSubsystem`이 소유하는 transient coordinator이며 world마다 플레이어 DeathComponent에 재바인딩한다.
5. Death overlay 최소 표시가 끝나면 FieldTransitionSubsystem이 loading UI, resettable actor, checkpoint 이동, stat/input/UI 복구를 순서대로 실행한다.
6. WorldStateSubsystem은 checkpoint, consumed spawn, flag, quest와 SaveGame 상태를 보존한다. GameInstance 수명의 QuestSubsystem은 WorldState 의존성을 초기화하고 quest 읽기·쓰기와 선택적 저장을 위임하는 facade다.

현재 전환 gate는 DeathOverlay 최소 표시 완료다. presentation finished는 overlay 요청이 누락된 경우 전환을 보장하는 fallback이며, 사망 몽타주 종료를 별도 필수 gate로 기다리지 않는다. 사망 상태에서는 이동·액션·상호작용 입력을 막지만 DeathOverlay는 시점 입력을 허용한다. LoadingWindow가 받는 입력은 도움말 카드 전환에만 쓰고 창 종료와 UI·입력 복원은 FieldTransition이 맡는다.

필드 상태 전용 서브시스템은 두지 않는다. 저장할 사실은 WorldState가 보관하고, 실제 숨김·복원·제거는 `IMVFieldTransitionResettableInterface`를 구현한 actor가 맡는다. 현재 C++에서 확인된 reset policy 적용은 `AMVEnemy`의 `ResetEveryTransition`이며, `PersistIfConsumed`, `PersistState`, `TransientOnly`는 각 도메인 구현을 추가할 때 검증해야 하는 설계 계약이다.

`DT_Death_P1`, `AM_Death_*`, `WBP_DeathOverlayWindow`, `WBP_LoadingWindow`의 row 참조, notify, widget binding은 `에셋 확인 필요`다. 사망 직후 전투·입력을 한곳에서 중단하는 기능과 로딩 화면의 현재 필드 이름 표시는 아직 C++에서 확인되지 않은 `설계 목표`다.
