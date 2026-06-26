# AI Combat StateTree 기술 명세

이 문서는 Maverick AI 전투 StateTree에서 사용하는 Condition과 Task의 입력값 의미, 판정 방식, 추천 설정값을 정리한다. 기준 코드는 `Source/Maverick/AI` 아래의 `MVCombatDecisionCondition`, `MVGlobalSensingTask`, `MVExecuteAttackTask`, `MVFocusingTask`, `MVStrafeMoveTask`이다.

## 전체 흐름

권장 구조는 다음과 같다.

```text
Combat
  Tasks:
    MVGlobal Sensing Task
    Focusing Task

  Children:
    Dead
    CounterAttack
    SprintAttack
    AirborneChargeAttack
    MoveToTarget / Reposition
    SkillAttack
    BasicAttack
    Strafe
    Idle
```

`MVGlobal Sensing Task`가 거리, 각도, 시야, 쿨다운 ready 목록을 `CombatContext`로 만든다. `Combat Decision Condition`은 이 `CombatContext`와 각 공격 후보 데이터를 비교해서 어떤 State에 들어갈지 결정한다. 공격 State의 실제 몽타주 실행은 `Execute Fixed Attack Task` 또는 `Select And Execute Attack Task`가 담당한다.

StateTree는 보통 위에서 아래로 먼저 true가 된 State를 선택한다. 따라서 공격 State가 너무 넓은 조건을 가지고 있으면 아래의 `MoveToTarget`, `Reposition`, `Strafe`는 실행되지 않는다.

## CombatContext

`CombatContext`는 직접 수동으로 채우기보다 `MVGlobal Sensing Task`의 Output을 바인딩해서 사용한다.

| 변수 | 의미 | 설정/주의 |
| --- | --- | --- |
| `bHasTarget` | 타겟이 있는지 | false면 Dead 외 전투 조건은 실패한다. |
| `DistanceToTarget` | AI와 타겟 사이 거리 | 공격 후보의 `MinDistance`, `MaxDistance`와 비교된다. |
| `AngleToTarget` | AI 정면과 타겟 방향 사이 각도 | 공격 후보의 `MaxAbsAngle`과 비교된다. |
| `CurrentArea` | `OutsideArea`, `DefensiveArea`, `OffensiveArea` | `DefensiveArea`, `OffensiveArea` 반경으로 계산된다. |
| `bHasLineOfSight` | 타겟까지 visibility trace가 막히지 않는지 | 후보의 `bRequiresLineOfSight`가 true일 때 필요하다. |
| `bActionRunning` | 현재 ActionComponent가 액션 실행 중인지 | true면 Dead 외 전투 조건은 실패한다. |
| `ReadyActionIds` | 쿨다운이 ready인 액션 id 목록 | 공격 후보의 `CooldownActionId`가 있으면 그 값, 없으면 `ActionRow.RowName`이 여기에 있어야 한다. |
| `LastAttackTag` | 마지막 공격 태그 | `Select And Execute Attack Task`는 1차 선택에서 같은 태그 반복을 피한다. |
| `bAttackCadenceReady` | 공통 공격 템포 쿨다운 ready 여부 | Skill/Basic 조건에서 사용된다. |
| `bCounterWindow` | 카운터 가능 타이밍 | CounterAttack 조건에서 사용된다. |
| `bSprintPathClear` | 전방 경로 trace가 비었는지 | Sprint 계열 전방 이동 공격 조건에 사용한다. |
| `bAirborneChargePathClear` | 공중 돌진 경로가 비었는지 | 현재는 `bSprintPathClear`와 같은 값으로 채워진다. |
| `bStrafePathClear` | 좌/우 회피 이동 경로가 비었는지 | Strafe 조건과 `bRequiresBackwardPathClear`에서 사용된다. |
| `bTargetCanBeAirborne` | 타겟을 띄울 수 있는지 | 후보의 `bRequiresTargetCanBeAirborne`이 true일 때 필요하다. |
| `bTargetIsAirborne` | 타겟이 공중 상태인지 | 후보의 `bRequiresTargetNotAirborne`이 true면 false여야 한다. |
| `bShouldUseAirborneCharge` | 공중 돌진 패턴을 사용할지 | 후보의 `bRequiresAirbornePattern`이 true일 때 필요하다. |
| `bNeedAttackAngle` | 공격 각도 보정이 필요한지 | `AngleToTarget > AttackAngleTolerance`로 계산된다. |
| `bNeedClearAttackPath` | 공격 경로 확보가 필요한지 | 현재는 `!bHasLineOfSight`로 계산된다. |
| `bIsDead` | 사망 상태 | Dead 조건에서 사용된다. |

## 공격 후보 `FMVAICombatActionCandidate`

Counter, Sprint, Airborne, Skill, Basic 공격은 모두 같은 후보 구조를 사용한다.

| 변수 | 의미 | 설정/주의 |
| --- | --- | --- |
| `ActionRow` | 실행할 액션 row handle | 필수다. `FMVActionRow` 또는 그 자식 row struct를 쓰는 DataTable row를 지정한다. |
| `StartSection` | 시작 섹션 | 비워두면 row의 `DefaultStartSection`을 사용한다. |
| `ActionId` | 임시 식별자 | 새 실행 흐름에서는 실행 키가 아니다. 기존 에셋 호환용 fallback으로만 남아 있다. |
| `ActionTag` | 마지막 공격 비교용 태그 | 비워두면 `ActionRow.RowName`을 태그로 쓴다. 같은 태그는 1차 선택에서 반복 방지된다. |
| `CooldownActionId` | 쿨다운 id | 비워두면 `ActionRow.RowName`을 쓴다. `ActionCooldowns`에도 같은 이름이 있어야 ready가 된다. |
| `Role` | 후보의 전투 역할 | 현재 C++ 선택 필터에는 직접 사용되지 않는다. 에디터 분류/디자인 용도다. |
| `MinDistance` | 이 거리보다 가까우면 후보 실패 | SprintAttack처럼 원거리에서만 나와야 하는 공격에 필수다. |
| `MaxDistance` | 이 거리보다 멀면 후보 실패 | `0` 이하는 상한 없음이다. 대부분 공격에는 실제 사거리를 넣어야 한다. |
| `MaxAbsAngle` | 허용 정면 각도 | 30이면 정면 좌우 30도 안에서만 통과한다. 180이면 거의 모든 방향에서 통과한다. |
| `bRequiresLineOfSight` | 시야 필요 여부 | true면 벽/장애물에 막히면 후보 실패한다. |
| `bRequiresForwardPathClear` | 전방 경로 필요 여부 | true면 `bSprintPathClear`가 true여야 한다. |
| `bRequiresBackwardPathClear` | 후방/측면 이동 경로 필요 여부 | true면 현재 코드상 `bStrafePathClear`가 true여야 한다. |
| `bRequiresTargetCanBeAirborne` | 타겟을 띄울 수 있어야 하는지 | true면 `bTargetCanBeAirborne` 필요. |
| `bRequiresTargetNotAirborne` | 타겟이 지상이여야 하는지 | true면 `bTargetIsAirborne`가 false여야 한다. |
| `bRequiresAirbornePattern` | 공중 돌진 패턴 플래그 필요 여부 | true면 `bShouldUseAirborneCharge` 필요. |

거리 판정은 다음과 같다.

```text
DistanceToTarget < MinDistance -> 실패
MaxDistance <= 0 -> 상한 없음
DistanceToTarget > MaxDistance -> 실패
```

따라서 `MaxDistance = 0`은 "0 거리"가 아니라 "무제한 사거리"다. 테스트 중 공격 State가 너무 자주 잡히면 먼저 각 후보의 `MaxDistance`를 확인한다.

## Combat Decision Condition

모든 전투 State의 Enter Condition으로 사용한다. `DesiredState`에 따라 다른 조건을 검사한다.

| 변수 | 의미 | 설정/주의 |
| --- | --- | --- |
| `CombatContext` | Global Sensing의 전투 컨텍스트 | 반드시 `MVGlobal Sensing Task.CombatContext`에 바인딩한다. |
| `DesiredState` | 검사할 목표 State | State 이름과 맞춘다. 예: SkillAttack State는 `SkillAttack`. |
| `CounterAttack` | 카운터 후보 | CounterAttack State에서 검사된다. |
| `SprintAttack` | 스프린트 공격 후보 | SprintAttack State에서 검사된다. |
| `AirborneChargeAttack` | 공중 돌진 후보 | AirborneChargeAttack State에서 검사된다. |
| `SkillAttacks` | 스킬 후보 배열 | SkillAttack State에서 ready 후보가 하나라도 있으면 true. |
| `BasicAttacks` | 기본 공격 후보 배열 | BasicAttack State에서 ready 후보가 하나라도 있으면 true. |
| `SprintMaxReach` | 너무 멀어서 추적해야 하는 기준 거리 | MoveToTarget 조건에서 사용된다. 보통 SprintAttack의 `MaxDistance`와 맞춘다. |
| `CombatMaxDistance` | 전투권 반경 | MoveToTarget/Strafe 판단에 쓰인다. 공격 후보의 사거리를 직접 제한하지 않는다. |

State별 판정 요약:

| DesiredState | true 조건 |
| --- | --- |
| `Dead` | `CombatContext.bIsDead` |
| `CounterAttack` | 카운터 윈도우가 열려 있고 Counter 후보가 ready |
| `SprintAttack` | Sprint 후보가 ready |
| `AirborneChargeAttack` | Airborne 후보가 ready |
| `SkillAttack` | 공격 템포 ready이고 Skill 후보 중 하나가 ready |
| `BasicAttack` | 공격 템포 ready이고 Basic 후보 중 하나가 ready |
| `MoveToTarget` | `DistanceToTarget > SprintMaxReach` 또는 `DistanceToTarget > CombatMaxDistance` |
| `Strafe` | 전투권 안이고 즉시 공격할 후보가 없고, 이동/각도/시야/템포 조정이 필요 |
| `Idle` | 항상 true |

`CombatMaxDistance`는 공격 사거리 제한이 아니다. 공격 가능 여부는 후보별 `MinDistance`, `MaxDistance`, `MaxAbsAngle`, 쿨다운 ready 여부가 결정한다.

## MVGlobal Sensing Task

Combat 부모 State의 Task로 두는 것을 권장한다. 타겟 정보와 전투 컨텍스트를 계속 갱신한다.

| 변수 | 의미 | 추천 |
| --- | --- | --- |
| `Target` | 추적할 타겟 | 비워두면 PlayerPawn 0을 찾는다. |
| `Owner` | AI Pawn | 보통 AIController의 Pawn으로 자동 해결되므로 비워도 된다. |
| `DefensiveArea` | 방어권 반경 | 예: 900~1200. 디버그 원의 노란색. |
| `OffensiveArea` | 공격권 반경 | 예: 300~500. 디버그 원의 빨간색. |
| `SprintTriggerDistance` | 스프린트 판단용 거리 | 현재 CombatContext에는 직접 복사되지 않는다. 후보 Min/MaxDistance를 우선 사용한다. |
| `SprintMaxReach` | 스프린트 최대 도달 거리 | 현재 CombatContext에는 직접 복사되지 않는다. Condition의 `SprintMaxReach`와 값만 맞춘다. |
| `SprintAttackAngle` | 스프린트 공격 각도 | 현재 CombatContext에는 직접 복사되지 않는다. 후보 `MaxAbsAngle`을 우선 사용한다. |
| `AirborneChargeMaxReach` | 공중 돌진 최대 거리 | 현재 CombatContext에는 직접 복사되지 않는다. 후보 `MaxDistance`를 우선 사용한다. |
| `AirborneChargeAngle` | 공중 돌진 각도 | 현재 CombatContext에는 직접 복사되지 않는다. 후보 `MaxAbsAngle`을 우선 사용한다. |
| `BasicAttackMaxReach` | 기본 공격 최대 거리 | 현재 CombatContext에는 직접 복사되지 않는다. 후보 `MaxDistance`를 우선 사용한다. |
| `AttackAngleTolerance` | 공격 각도 보정 필요 기준 | 예: 30. 이보다 크면 `bNeedAttackAngle` true. |
| `ForwardPathTraceDistance` | 전방 경로 trace 길이 | Sprint/돌진 공격 경로 검사 거리. |
| `BackwardPathTraceDistance` | 후방 경로 trace 길이 | 현재 코드에서는 직접 사용되지 않는다. |
| `StrafePathTraceDistance` | 좌우 strafe 경로 trace 길이 | Strafe 가능 여부 계산. |
| `ActionCooldowns` | AI 전투 쿨다운 목록 | 모든 후보의 `CooldownActionId` 또는 `ActionRow.RowName`을 등록해야 한다. |
| `AttackCadenceActionId` | 공통 공격 템포 쿨다운 id | 비우면 항상 ready. 쓰려면 `ActionCooldowns`에도 등록한다. |
| `CurrentPhase` | 패턴 페이즈 | 현재 기본 조건에는 직접 사용되지 않는다. |
| `bCounterWindow` | 카운터 가능 플래그 | 외부 로직에서 바인딩해서 사용한다. |
| `bTargetCanBeAirborne` | 타겟 공중화 가능 여부 | 공중 패턴 후보 조건용. |
| `bTargetIsAirborne` | 타겟 공중 상태 | 지상 전용 공격 제한용. |
| `bShouldUseAirborneCharge` | 공중 돌진 패턴 사용 여부 | Airborne 후보 조건용. |
| `LastAttackTag` | 마지막 공격 태그 | 공격 Task의 `LastAttackTag` output을 다시 바인딩한다. |
| `bIsDead` | 사망 여부 | Dead 조건으로 전달한다. |
| `bDrawCombatAreaDebug` | 디버그 표시 | 거리/권역 문제를 볼 때 true. |
| `CombatAreaDebugDuration` | 디버그 유지 시간 | 0이면 한 프레임성 표시. |
| `CombatAreaDebugHeightOffset` | 디버그 높이 | 바닥과 겹치면 5~30 정도. |

## Select And Execute Attack Task

SkillAttack, BasicAttack처럼 여러 후보 중 하나를 고르는 State에 사용한다.

| 변수 | 의미 | 설정/주의 |
| --- | --- | --- |
| `Owner` | AI Pawn | Global Sensing Owner 또는 비워서 자동 해결. |
| `CombatContext` | 전투 컨텍스트 | Global Sensing의 `CombatContext`에 바인딩한다. |
| `SelectionMode` | 선택 모드 | 현재 C++에서는 실제 선택에 사용되지 않는다. 표시용에 가깝다. |
| `Candidates` | 실행 후보 배열 | 반드시 Skill/Basic 후보 배열을 여기에 바인딩하거나 직접 추가한다. 0개면 무조건 실패한다. |
| `SelectedAttack` | 선택된 후보 output | 입력값이 아니다. 여기에 값을 써도 후보로 선택되지 않는다. |
| `LastAttackTag` | 선택된 공격 태그 output | Global Sensing의 `LastAttackTag`로 되돌려 반복 방지에 사용한다. |

선택 순서:

1. 후보 배열을 앞에서부터 검사한다.
2. 거리, 각도, 쿨다운, 시야/path/airborne 조건을 모두 통과해야 한다.
3. 1차 선택에서는 `LastAttackTag`와 같은 후보를 피한다.
4. 모두 실패하면 같은 태그 반복을 허용하고 다시 검사한다.
5. 그래도 없으면 Task는 Failed가 된다.

`SelectionMode`만 SkillAttack으로 바꾸고 `Candidates`를 비워두면 공격은 실행되지 않는다.

## Execute Fixed Attack Task

CounterAttack, SprintAttack, AirborneChargeAttack처럼 단일 후보를 실행하는 State에 사용한다.

| 변수 | 의미 | 설정/주의 |
| --- | --- | --- |
| `Owner` | AI Pawn | 보통 비워도 자동 해결된다. |
| `Attack` | 실행할 후보 | `ActionRow`가 비어 있으면 실패한다. |
| `FallbackAttackDirection` | ActionComponent가 없을 때 `AMVEnemy::Attack`에 넘길 방향 | 현재 일반 액션 시스템을 쓰면 거의 fallback 용도다. |
| `LastAttackTag` | 실행된 공격 태그 output | Global Sensing의 `LastAttackTag`로 되돌려 반복 방지에 사용한다. |

ActionComponent가 있으면 `TryStartActionFromRowHandle(Attack.ActionRow, Attack.StartSection)`로 몽타주를 실행한다. 실행 후 후보의 쿨다운을 시작한다.

## Focusing Task

AI가 타겟을 바라보도록 회전시키는 보조 Task다.

| 변수 | 의미 | 추천 |
| --- | --- | --- |
| `Target` | 바라볼 타겟 | 비워두면 PlayerPawn 0을 찾는다. |
| `Owner` | 회전시킬 Pawn | 보통 AIController의 Pawn으로 자동 해결. |
| `TurnSpeed` | 회전 보간 속도 | 5 전후부터 조정. 빠른 보스는 8~12. |
| `bCanFocus` | 포커싱 활성화 | false면 Task 실패. |

이 Task는 계속 `Running`을 반환한다. 따라서 독립 leaf State에 단독으로 넣기보다 Combat 부모 State, MoveToTarget, Strafe, Idle 같은 지속 State의 보조 Task로 두는 것이 좋다. 공격 몽타주 중 회전을 원하지 않으면 공격 State에는 넣지 않는다.

## MVStrafe Move Task

타겟 주변을 좌우로 이동하면서 거리 보정을 하는 Task다.

| 변수 | 의미 | 추천 |
| --- | --- | --- |
| `MoveDuration` | 한 번 strafe를 지속할 시간 | 1~2초. |
| `TraceDistance` | 이동 방향 장애물 검사 거리 | 200~400. |
| `DesiredDistance` | 유지하려는 타겟 거리 | 근접 적이면 300~500. |
| `DistanceCorrectionRange` | 거리 보정이 최대치가 되는 오차 범위 | 150~300. |
| `DistanceCorrectionWeight` | 거리 보정 가중치 | 0.3~0.8. 높을수록 원형 이동보다 거리 유지가 강해진다. |

Strafe 조건은 전투권 안에서 즉시 공격할 후보가 없고, 각도/시야/템포/이동 경로 조정이 필요할 때 true가 된다.

## 쿨다운 관련 Task/Condition

### ActionCooldowns

`ActionCooldowns`는 `UMVActionCooldownComponent`에 등록되는 쿨다운 목록이다.

| 변수 | 의미 | 설정/주의 |
| --- | --- | --- |
| `ActionId` | 쿨다운 이름 | 후보의 `CooldownActionId`와 같아야 한다. 후보의 `CooldownActionId`가 비어 있으면 후보 `ActionRow.RowName`과 같아야 한다. |
| `CooldownDuration` | 쿨다운 시간 | 0이면 시작해도 즉시 ready. |
| `bStartReady` | 시작 시 ready 여부 | false면 시작부터 쿨다운 중이다. |

### Global Action Cooldown Task

쿨다운 컴포넌트를 만들고 ticking한다. 현재 `MVGlobal Sensing Task`도 쿨다운 컴포넌트를 만들고 갱신하므로, 같은 StateTree에서 둘을 중복 배치하지 않도록 주의한다.

### Start Action Cooldown Task

특정 `ActionId`의 쿨다운을 시작한다. 공격 Task는 실행 성공 후 자체적으로 후보 쿨다운을 시작하므로 별도 배치가 항상 필요한 것은 아니다.

### Action Cooldown Ready Condition

특정 쿨다운 id가 ready인지 검사한다. 단일 쿨다운만 직접 조건으로 보고 싶을 때 사용한다.

## 추천 거리 설정

NamelessPuppet 같은 근접 AI 기준 예시:

| State | MinDistance | MaxDistance | MaxAbsAngle | 비고 |
| --- | ---: | ---: | ---: | --- |
| `CounterAttack` | 0 | 350 | 90 | 카운터 윈도우가 있을 때만. |
| `BasicAttack` | 0 | 300 | 45 | 일반 근접 공격. |
| `SkillAttack` 근거리 | 0 | 500 | 60 | 스킬 성격에 맞게 조정. |
| `SprintAttack` | 700 | 2200 | 25 | 가까우면 나오지 않게 `MinDistance` 필수. |
| `AirborneChargeAttack` | 600 | 1600 | 30 | 전방 path clear와 패턴 플래그 사용 권장. |

`CombatMaxDistance`는 300~500 정도로 두고, `SprintMaxReach`는 SprintAttack의 `MaxDistance`와 맞춘다. 단, 실제 공격 가능 여부는 후보별 거리값이 결정한다.

## 흔한 문제 체크리스트

### 너무 멀리 있는데 공격한다

- 공격 후보의 `MaxDistance`가 0인지 확인한다. 0은 무제한이다.
- 공격 State가 MoveToTarget/Reposition보다 위에 있고 조건이 넓은지 확인한다.
- 후보의 `MaxAbsAngle`이 180이라서 뒤돌아 있어도 통과하는지 확인한다.

### SprintAttack이 가까이서도 나온다

- Sprint 후보의 `MinDistance`를 설정한다. 예: 700.
- Sprint 후보의 `MaxDistance`도 설정한다. 예: 2200.

### SkillAttack State에는 들어가는데 몽타주가 안 나온다

- `Select And Execute Attack Task.Candidates`가 0개인지 확인한다.
- `SelectedAttack`은 output이다. 후보 입력은 `Candidates`다.
- 후보 `CooldownActionId` 또는 `ActionRow.RowName`이 `ReadyActionIds`에 있는지 확인한다.
- 후보 `ActionRow`가 비어 있지 않고, 해당 DataTable이 manifest에 등록되어 있는지 확인한다.

### MoveToTarget/Reposition에 들어오지 않는다

- 위쪽 공격 State 후보가 먼저 true가 되는지 확인한다.
- 공격 후보 `MaxDistance`가 실제 사거리로 제한되어 있는지 확인한다.
- `CombatMaxDistance`는 공격 후보 사거리를 직접 막지 않는다.

### AI가 안 바라보고 공격한다

- Focusing Task를 Combat 부모 또는 이동/대기 State에 배치한다.
- 공격 후보 `MaxAbsAngle`을 줄인다. 근접 공격은 30~60 권장.
- 공격 State 우선순위가 Focusing/Strafe보다 너무 앞서 있는지 확인한다.
