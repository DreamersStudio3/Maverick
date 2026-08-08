# FMVSkillEntry

> God node · 38 connections · `Source/Maverick/Components/MVCombatComponent.h`

**Community:** [FMVSkillEntry](FMVSkillEntry.md)

## Connections by Relation

### contains
- MVCombatComponent.h `EXTRACTED`

### defines
- AbilityInstances `EXTRACTED`
- bChainActive `EXTRACTED`
- bIsChained `EXTRACTED`
- CurrentChainStageIndex `EXTRACTED`
- DataTable `EXTRACTED`
- InputWindowCloseTime `EXTRACTED`
- LastStageActivationTime `EXTRACTED`
- LastUsedTime `EXTRACTED`
- MainCooldownDuration `EXTRACTED`
- SkillRowNames `EXTRACTED`

### method
- .GetCurrentSkillData() `EXTRACTED`
- .GetCurrentActionRowHandle() `EXTRACTED`
- .GetCurrentRowName() `EXTRACTED`
- .ActivateChain() `EXTRACTED`
- .ContainsAbility() `EXTRACTED`
- .GetCurrentAbility() `EXTRACTED`
- .IsInterStageCooldownValid() `EXTRACTED`
- .StartPostAbilityResetWindow() `EXTRACTED`
- .TryAdvanceChainStage() `EXTRACTED`
- .GetRemainingInputWindowTime() `EXTRACTED`
- .IsInputWindowValid() `EXTRACTED`
- .IsMainCooldownReady() `EXTRACTED`
- .ResetChain() `EXTRACTED`

### references
- UMVCombatComponent::TryStartActionWithAbility() `EXTRACTED`
- [FName](FName.md) `EXTRACTED`
- UMVCombatComponent::SelectBasicAttackChainStageForSwing() `EXTRACTED`
- UMVCombatComponent::GetBasicAttackSkillDataAtStage() `EXTRACTED`
- UMVCombatComponent::MarkBasicAttackChainStarted() `EXTRACTED`
- UMVCombatComponent::ResolveHeavyChargeEarlyReleaseChainStageIndex() `EXTRACTED`
- UMVCombatComponent::ResolveHeavyChargeEarlyReleaseRow() `EXTRACTED`
- UMVAbilityBase `EXTRACTED`
- [int32](int32.md) `EXTRACTED`
- UMVCombatComponent::BuildSkillEntryFromRowHandle() `EXTRACTED`
- UMVCombatComponent::ResolveHeavyChargeStartChainStageIndex() `EXTRACTED`
- TObjectPtr `EXTRACTED`
- UDataTable `EXTRACTED`
- TArray `EXTRACTED`

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*