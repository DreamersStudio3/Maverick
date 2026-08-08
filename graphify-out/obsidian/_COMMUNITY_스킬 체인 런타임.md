---
type: community
cohesion: 0.09
members: 28
---

# 스킬 체인 런타임

**Cohesion:** 0.09 - loosely connected
**Members:** 28 nodes

## Members
- [[dot-ActivateChain()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-ContainsAbility()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-GetCurrentAbility()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-GetCurrentSkillData()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-GetRemainingInputWindowTime()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-IsInputWindowValid()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-IsInterStageCooldownValid()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-IsMainCooldownReady()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-ResetChain()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-StartPostAbilityResetWindow()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[dot-TryAdvanceChainStage()]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[AbilityInstances]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[CurrentChainStageIndex]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[DataTable_1]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[EMVAttackSwingDirection]] - code
- [[FMVSkillEntry]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[InputWindowCloseTime]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[LastStageActivationTime]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[LastUsedTime]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[MVCombatGetOppositeSwingDirection()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MainCooldownDuration]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[SkillRowNames]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[TArray_14]] - code
- [[UMVAbilityBase_3]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[UMVCombatComponentResolveHeavyChargeEarlyReleaseChainStageIndex()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentSelectBasicAttackChainStageForSwing()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[bChainActive]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bIsChained]] - code - Source/Maverick/Components/MVCombatComponent.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 8 edges to [[_COMMUNITY_전투 액션 매핑]]
- 8 edges to [[_COMMUNITY_전투 액션 런타임 상태]]
- 3 edges to [[_COMMUNITY_전투 액션 행 해석]]
- 1 edge to [[_COMMUNITY_스킬 데이터와 비용]]

## Top bridge nodes
- [[FMVSkillEntry]] - degree 38, connects to 3 communities
- [[dot-GetCurrentSkillData()]] - degree 6, connects to 1 community
- [[MVCombatGetOppositeSwingDirection()]] - degree 4, connects to 1 community
- [[UMVAbilityBase_3]] - degree 4, connects to 1 community
- [[UMVCombatComponentResolveHeavyChargeEarlyReleaseChainStageIndex()]] - degree 4, connects to 1 community