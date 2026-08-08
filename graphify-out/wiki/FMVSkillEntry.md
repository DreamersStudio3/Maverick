# FMVSkillEntry

> God node · 38 connections · `Source/Maverick/Components/MVCombatComponent.h`

**Community:** [스킬 체인 런타임](%EC%8A%A4%ED%82%AC_%EC%B2%B4%EC%9D%B8_%EB%9F%B0%ED%83%80%EC%9E%84.md)

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
- FName `EXTRACTED`
- UMVCombatComponent::SelectBasicAttackChainStageForSwing() `EXTRACTED`
- UMVCombatComponent::GetBasicAttackSkillDataAtStage() `EXTRACTED`
- UMVCombatComponent::MarkBasicAttackChainStarted() `EXTRACTED`
- UMVCombatComponent::ResolveHeavyChargeEarlyReleaseChainStageIndex() `EXTRACTED`
- UMVCombatComponent::ResolveHeavyChargeEarlyReleaseRow() `EXTRACTED`
- UMVAbilityBase `EXTRACTED`
- int32 `EXTRACTED`
- UMVCombatComponent::BuildSkillEntryFromRowHandle() `EXTRACTED`
- UMVCombatComponent::ResolveHeavyChargeStartChainStageIndex() `EXTRACTED`
- TObjectPtr `EXTRACTED`
- UDataTable `EXTRACTED`
- TArray `EXTRACTED`

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*