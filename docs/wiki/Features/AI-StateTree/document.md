---
제목: AI StateTree
부제목: StateTree 감지·판단·Action 실행과 에셋 경계
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---
# AI StateTree

```text
StateTree asset
  -> FMVGlobalSensingTask
  -> FMVAICombatContext
  -> FMVCombatDecisionCondition
  -> attack / movement / strafe / hit reaction / death task
  -> ActionComponent + cooldown
```

- `FMVGlobalSensingTask`가 거리, 각도, LOS, 전투 영역, 이동 경로, 실행 상태와 cooldown을 공유 문맥에 갱신한다.
- Condition은 문맥을 판정하지만 실제 상태 우선순위와 property binding은 StateTree 에셋 순서에 의존한다.
- 공격 task는 `EnemyCombatAction -> Enemy -> CombatComponent -> ActionComponent` 경로와 `SelectAndExecuteAttack -> ActionComponent` 직접 경로가 공존한다.
- `AMVAIController`의 AIPerception `TargetActor`와 GlobalSensing의 현재 플레이어 탐색은 텍스트 코드에서 연결되지 않은 별도 경로다.
- C++은 StateTree 컴포넌트를 직접 생성하지 않는다. 실제로 Controller의 BrainComponent, 기타 Controller component, Pawn component 중 어디에 배치되는지는 Blueprint 구성에 따라 달라질 수 있다.

StateTree 에셋을 수정할 때는 다음 계약을 함께 확인한다.

- 위에서 먼저 성립한 상태가 아래의 MoveToTarget이나 Strafe를 가릴 수 있으므로 상태 순서와 후보 범위를 함께 설계한다.
- `GlobalSensing.CombatContext`는 Condition과 Task로, 공격 Task의 `LastAttackTag`는 GlobalSensing으로 되돌아가도록 binding한다.
- 후보별 거리와 각도가 공격 가능 범위를 결정한다. `CombatMaxDistance`는 이동 상태를 나누는 기준이지 공격 사거리가 아니다.
- Focusing은 지속 부모나 이동 상태에 둔다. 공격 중 강제 회전을 원하지 않으면 공격 상태에는 두지 않는다.
- GlobalSensing과 Global Action Cooldown Task가 같은 cooldown component를 동시에 tick하지 않게 소유자를 하나만 둔다.

실제 자식 상태 순서, property binding, Task 배치, 공격 후보, Chooser, cooldown ID와 타깃 경로는 `에셋 확인 필요`다.
