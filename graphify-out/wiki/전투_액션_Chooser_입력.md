# 전투 액션 Chooser 입력

> 15 nodes · cohesion 0.14

## Key Concepts

- **FMVCombatActionTableInput** (15 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **UChooserTable** (7 connections) — `Source/Maverick/Components/MVCombatComponent.h`
- **UMVCombatComponent::GetActionRowHandleFromChooserTable()** (5 connections) — `Source/Maverick/Components/MVCombatComponent.cpp`
- **UMVCombatComponent::GetDataTableRowFromChooserTable_Implementation()** (4 connections) — `Source/Maverick/Components/MVCombatComponent.cpp`
- **UMVHitReactionComponent::EvaluateHitReactionChooserActionRowHandle()** (3 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **.SetActionType()** (2 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **FGameplayTag** (2 connections)
- **ActionType** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **ActionTypeTags** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **CurrentWeaponStyle** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **.FMVCombatActionTableInput()** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **.RefreshActionTypeTags()** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **EMVEquippedStyle** (1 connections)
- **FGameplayTagContainer** (1 connections)
- **UPROPERTY** (1 connections)

## Relationships

- [FDataTableRowHandle](FDataTableRowHandle.md) (4 shared connections)
- [ExecuteAttackResolveActionCandidate](ExecuteAttackResolveActionCandidate.md) (2 shared connections)
- [MVCombatComponent.cpp](MVCombatComponent.cpp.md) (2 shared connections)
- [MVEnemyDodgeActionTask.cpp](MVEnemyDodgeActionTask.cpp.md) (1 shared connections)
- [FMVDodgeInputContext](FMVDodgeInputContext.md) (1 shared connections)
- [FMVCombatActionEvent](FMVCombatActionEvent.md) (1 shared connections)
- [MVFinisherComponent.cpp](MVFinisherComponent.cpp.md) (1 shared connections)
- [FMVSkillDataTableColumn](FMVSkillDataTableColumn.md) (1 shared connections)
- [MVHitReactionComponent.cpp](MVHitReactionComponent.cpp.md) (1 shared connections)
- [MVHitReactionComponent.h](MVHitReactionComponent.h.md) (1 shared connections)
- [MVWeaponComponent.cpp](MVWeaponComponent.cpp.md) (1 shared connections)

## Source Files

- `Source/Maverick/Components/MVCombatComponent.cpp`
- `Source/Maverick/Components/MVCombatComponent.h`
- `Source/Maverick/Components/MVHitReactionComponent.cpp`
- `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`

## Audit Trail

- EXTRACTED: 46 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*