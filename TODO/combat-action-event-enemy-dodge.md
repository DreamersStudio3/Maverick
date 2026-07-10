# 전투 액션 이벤트와 Enemy 회피 연결 계획

## 목적

- 플레이어 또는 Enemy가 공격 액션을 실제로 시작했을 때 공통 이벤트를 발행한다.
- Enemy AI는 이 이벤트를 받아 거리, 각도, 쿨타임, 현재 상태를 판단한 뒤 StateTree 회피 상태로 전이할 수 있게 한다.
- 이번 범위에서는 "공격을 실제로 맞췄는지" 이벤트는 구현하지 않는다.

## 이번 구현 범위

- `UMVCombatComponent`에 범용 전투 액션 이벤트 구조체 `FMVCombatActionEvent`를 추가한다.
- `UMVCombatComponent::OnCombatActionStarted` 델리게이트를 추가한다.
- `TryCombatAction()`에서 `TryBasicAttack()` 또는 `TrySkill()`이 `true`를 반환한 시점에 이벤트를 발행한다.
- 이벤트에는 현재 액션을 시작한 owner, 액션 타입, 액션 인덱스, 활성 액션 테이블/row 이름을 담는다.
- AI 회피 요청 payload 구조체 `FMVAIDodgeRequest`를 `Public/Struct/MVAIDodgeTypes.h`에 둔다.
- `Enemy Dodge Action Task`는 `FMVAIDodgeRequest.Direction`을 받아 방향별 dodge row handle 중 하나를 실행한다.

## 현재 이벤트 발행 흐름

```text
입력 또는 AI 요청
 -> UMVCombatComponent::TryCombatAction()
 -> TryBasicAttack() 또는 TrySkill()
 -> 액션 시작 성공
 -> OnCombatActionStarted(FMVCombatActionEvent) 발행
```

## Enemy 회피 연결 방식

- Enemy 또는 AIController 쪽에서 타겟의 `UMVCombatComponent::OnCombatActionStarted`를 구독한다.
- 이벤트의 `Instigator`가 현재 Enemy의 타겟인지 확인한다.
- 타겟이 맞다면 Enemy 기준으로 거리, 전방 각도, 회피 쿨타임, 현재 액션 실행 여부를 검사한다.
- 조건이 맞으면 `FMVAIDodgeRequest`를 만들고 StateTree에 회피 요청 이벤트를 보낸다.
- StateTree에서는 회피 상태가 해당 이벤트 payload를 받아 `Enemy Dodge Action Task`에 연결한다.

예상 흐름:

```text
타겟 CombatComponent.OnCombatActionStarted
 -> Enemy AI/감지 컴포넌트가 수신
 -> 내 타겟의 공격인지 확인
 -> 회피 가능 조건 검사
 -> FMVAIDodgeRequest 생성
 -> StateTree 회피 이벤트 발행
 -> Dodge 상태 진입
 -> Enemy Dodge Action Task가 Direction에 맞는 row 실행
```

## StateTree/BP 연결 방법

- 바인딩 Task에서 타겟의 `CombatComponent`를 얻고 `Bind Event to OnCombatActionStarted`를 연결한다.
- delegate 핀에서 직접 custom event를 생성해 `FMVCombatActionEvent` 시그니처를 맞춘다.
- custom event 이름은 `HandleTargetCombatActionStarted`를 권장한다.
- `HandleTargetCombatActionStarted` 안에서 거리/각도/쿨타임을 검사하고 회피 방향을 결정한다.
- `Make FMVAIDodgeRequest`로 `Direction`, `ThreatActor`, `ThreatLocation`을 채운다.
- `Make StateTree Event`에서 payload에 `FMVAIDodgeRequest`를 넣는다.
- StateTree 전이 조건은 동일한 event tag를 사용하고, Dodge 상태의 `Enemy Dodge Action Task.DodgeRequest` 입력에 event payload를 바인딩한다.

권장 event tag:

```text
AI.Event.DodgeThreatDetected
```

프로젝트 GameplayTag 테이블/설정에 이미 AI 이벤트 태그 체계가 있으면 그 체계를 따른다. 없으면 새 태그를 만들어 쓰는 것이 좋다. 문자열 이름만 맞추는 방식보다 GameplayTag로 관리하는 편이 StateTree 전이와 BP 설정에서 오타를 줄일 수 있다.

## Enemy Dodge Action Task 설정

- Task 이름: `Enemy Dodge Action Task`
- 입력:
  - `Owner`: 보통 비워두면 AIController의 Pawn으로 자동 해석된다.
  - `DodgeRequest`: StateTree event payload에서 바인딩한다.
  - `DefaultDirection`: payload 방향이 `None`일 때 사용할 방향.
  - `Forward/Back/Left/Right Dodge Action Row`: 방향별 `FMVDodgeActionRow` row handle을 지정한다.
  - `bTransitionFromCurrentAction`: 현재 액션 중에도 회피 액션으로 전환할지 설정한다.
  - `bWaitForActionEnd`: 회피 액션이 끝날 때까지 상태를 유지할지 설정한다.

실행:

```text
FMVAIDodgeRequest.Direction
 -> 방향별 DodgeActionRow 선택
 -> ActionComponent.TryStartActionFromRowHandle 또는 TryTransitionActionFromRowHandle
```

## Enemy가 피격 받았을 때의 흐름

- Enemy가 피격을 받았는지는 기존 `AMVEnemy::OnEnemyDamaged`를 사용한다.
- `AMVEnemy::BindDamageHandlers()`에서 `OnDamaged`가 `AMVEnemy::HandleEnemyDamaged()`로 연결된다.
- `HandleEnemyDamaged()`는 `OnEnemyDamaged`를 브로드캐스트한다.
- StateTree에서 Enemy 피격 반응을 쓰려면 기존 `STT_BindHitEvent` 같은 StateTree Task 또는 BP Task가 `OnEnemyDamaged`를 구독하고, 필요한 StateTree 이벤트를 발행하면 된다.

피격 흐름:

```text
HitResolver
 -> Victim->OnHitResolved()
 -> AMVCharacterBase::OnDamaged
 -> AMVEnemy::HandleEnemyDamaged()
 -> AMVEnemy::OnEnemyDamaged
 -> StateTree 피격 이벤트 발행
```

## 나중에 설정할 수 있는 값

- 회피 반응 거리
- 회피 반응 각도
- 회피 쿨타임
- 공격 타입별 회피 확률
- 공격 타입별 회피 지연 시간
- 회피 방향 선택 규칙
- 현재 Enemy 액션 중 회피 허용 여부

## 제외한 내용

- Enemy가 공격을 실제로 맞췄는지 확인하는 명중 성공 이벤트는 이번 구현에서 제외한다.
- 정확한 타격 직전 회피 타이밍은 아직 AnimNotifyState 기반 threat window로 분리하지 않았다.
- 슈퍼아머, 피격 캔슬 가능 hit type 정책은 이번 구현 범위가 아니다.
