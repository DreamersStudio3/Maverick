---
type: community
cohesion: 0.07
members: 55
---

# MVCombatComponent.cpp

**Cohesion:** 0.07 - loosely connected
**Members:** 55 nodes

## Members
- [[ELevelTick_2]] - code
- [[EMVAttackSwingDirection]] - code
- [[EMVCombatActionTypes_1]] - code
- [[EMVEquippedStyle_2]] - code
- [[FActorComponentTickFunction_1]] - code
- [[FString_17]] - code
- [[MVCombatActionTypeToString()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatComponent.cpp]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatGetOppositeSwingDirection()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsBasicAttackStartRowCandidate()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsDodgeContextualAttackRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsDodgeContextualBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsSkillStartRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsSprintContextualBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeSkillActionTypeGameplayTag()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeSkillFallbackRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeSkillMapKey()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatRowHandleToString()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatSkillRowNamesToString()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVAbilityBase_2]] - code
- [[UMVCombatComponentBeginPlay()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentBroadcastCombatActionStarted()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentChangeWeapon()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentClearLastBasicAttackSwingDirection()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentGetBasicAttackSkillDataAtStage()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentGetDodgeAttackContextInstanceId()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentGetSkillSlotRuntimeState()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentHandleAbilityEnded()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentHasReachedSprintAttackSpeed()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsDodgeAttackContext()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsHeavyChargeActionRunning()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsSprintAttackContext()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMakeActionTypeMapKey()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMakeIndexedActionRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMarkBasicAttackChainStarted()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMarkContextualBasicAttackStarted()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentRefreshActionMaps()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResetBasicAttackMap()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResetSkillMap()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveContextualBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveHeavyChargeEarlyReleaseChainStageIndex()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveHeavyChargeStartChainStageIndex()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentSelectBasicAttackChainStageForSwing()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentShouldSuppressChargeAttackInputForSprint()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTickComponent()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryBasicAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryCombatAction()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryCommitHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryReleaseHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTrySkill()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryUpdateHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentUMVCombatComponent()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentUpdateContextualBasicAttackResets()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[int32_16]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/MVCombatComponentcpp
SORT file.name ASC
```

## Connections to other communities
- 35 edges to [[_COMMUNITY_FDataTableRowHandle]]
- 8 edges to [[_COMMUNITY_FGameplayTag]]
- 5 edges to [[_COMMUNITY_FMVSkillEntry]]
- 4 edges to [[_COMMUNITY_FMVSkillDataTableColumn]]
- 2 edges to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_MVAICombatTypes.h]]
- 1 edge to [[_COMMUNITY_Type]]
- 1 edge to [[_COMMUNITY_FMVCombatActionEvent_1]]
- 1 edge to [[_COMMUNITY_FMVCombatActionEvent]]

## Top bridge nodes
- [[MVCombatComponent.cpp]] - degree 76, connects to 7 communities
- [[EMVCombatActionTypes_1]] - degree 16, connects to 2 communities
- [[UMVCombatComponentGetBasicAttackSkillDataAtStage()]] - degree 4, connects to 2 communities
- [[UMVCombatComponentMarkBasicAttackChainStarted()]] - degree 4, connects to 2 communities
- [[int32_16]] - degree 16, connects to 1 community