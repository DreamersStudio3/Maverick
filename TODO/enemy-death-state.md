# Enemy Death State

## 완료

- [x] `MVGlobalSensingTask`가 Owner의 `UMVStatComponent::IsDead()`를 읽어 `CombatContext.bIsDead`에 반영하도록 수정.
- [x] `MVCombatDecisionCondition`에서 Dead 상태 판정을 타겟/액션 실행 여부보다 우선하도록 정리.
- [x] `Enemy Dead Task` C++ StateTree Task 추가. Dead 상태 진입 시 AI/CharacterMovement를 정지하고, `DeathComponent` presentation 완료 후 선택적으로 cleanup을 수행한다.

## 다음 작업

- [ ] 적 StateTree에 `Dead` 상태를 추가하고 `Combat Decision Condition`의 `DesiredState = Dead`를 enter condition으로 연결.
- [ ] Dead 상태에 `Enemy Dead Task`를 배치하고 cleanup 정책(`None`, `DestroyActor`, `DeactivateActor`)을 적 종류별로 결정.
- [ ] `DeathComponent` presentation 완료 이후 적 제거, 비활성화, 루팅, 재스폰 정책 중 어느 흐름을 사용할지 도메인별로 결정.
- [ ] 일반 몬스터가 필드 전환 때 리셋되어야 하면 `MVFieldTransitionResettableInterface` 구현 또는 스폰 관리자 정책을 추가.
- [ ] 적 전용 `Death_*` 액션 테이블과 row 이름 규칙을 실제 데이터에 맞춰 생성.
