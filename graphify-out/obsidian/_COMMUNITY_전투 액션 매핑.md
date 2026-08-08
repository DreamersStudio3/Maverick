---
type: community
cohesion: 0.08
members: 53
---

# 전투 액션 매핑

**Cohesion:** 0.08 - loosely connected
**Members:** 53 nodes

## Members
- [[EMVCombatActionTypes_1]] - code
- [[FName_35]] - code
- [[FString_16]] - code
- [[MVCombatActionTypeToString()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatAppendRowCandidate()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatComponent.cpp]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsBasicAttackStartRowCandidate()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsDodgeContextualAttackRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsDodgeContextualBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsSkillStartRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatIsSprintContextualBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeFallbackAttackRowCandidates()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeSkillActionTypeGameplayTag()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeSkillFallbackRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeSkillMapKey()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatRowHandleToString()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatSkillRowNamesToString()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[TArray_13]] - code
- [[UMVCombatComponentBeginPlay()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentBroadcastCombatActionStarted()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentClearLastBasicAttackSwingDirection()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentGetBasicAttackSkillDataAtStage()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentGetDodgeAttackContextInstanceId()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentGetSkillSlotRuntimeState()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentHandleActionEnded()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentHasReachedSprintAttackSpeed()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsCurrentAbilityAction()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsDodgeAttackContext()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsHeavyChargeActionRunning()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsHeavyChargeBasicAttackMapKey()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsSprintAttackContext()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMakeActionTypeMapKey()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMakeIndexedActionRowName()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMarkBasicAttackChainStarted()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMarkContextualBasicAttackStarted()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentRefreshActionMaps()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResetBasicAttackMap()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResetOtherBasicAttackChains()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResetSkillMap()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveContextualBasicAttackActionType()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveHeavyChargeStartChainStageIndex()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentShouldSuppressChargeAttackInputForSprint()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryBasicAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryCombatAction()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryCommitHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryEarlyReleaseHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryReleaseHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTrySkill()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryUpdateHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentUMVCombatComponent()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentUpdateContextualBasicAttackResets()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[int32_15]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 19 edges to [[_COMMUNITY_전투 액션 행 해석]]
- 10 edges to [[_COMMUNITY_전투 액션 입력 처리]]
- 8 edges to [[_COMMUNITY_스킬 체인 런타임]]
- 4 edges to [[_COMMUNITY_스킬 데이터와 비용]]
- 4 edges to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_AI 전투 액션 메타데이터]]
- 1 edge to [[_COMMUNITY_피니셔 실행과 워핑]]
- 1 edge to [[_COMMUNITY_전투 액션 런타임 상태]]
- 1 edge to [[_COMMUNITY_전투 컴포넌트 Tick]]
- 1 edge to [[_COMMUNITY_스킬 슬롯 런타임 상태]]

## Top bridge nodes
- [[MVCombatComponent.cpp]] - degree 76, connects to 9 communities
- [[FName_35]] - degree 23, connects to 2 communities
- [[EMVCombatActionTypes_1]] - degree 16, connects to 2 communities
- [[int32_15]] - degree 16, connects to 2 communities
- [[UMVCombatComponentGetBasicAttackSkillDataAtStage()]] - degree 4, connects to 2 communities