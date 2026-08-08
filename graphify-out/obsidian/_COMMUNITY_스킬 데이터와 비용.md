---
type: community
cohesion: 0.08
members: 25
---

# 스킬 데이터와 비용

**Cohesion:** 0.08 - loosely connected
**Members:** 25 nodes

## Members
- [[dot-FMVSkillDataTableColumn()]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[AbilityReference]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[ChargeCommitTime]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[CooldownDuration_3]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[DamageMultiplier]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[EMVAttackSwingDirection_1]] - code
- [[EarlyReleaseBlendOutTime]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[EarlyReleaseStartSection]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[FMVActionRow_2]] - code
- [[FMVSkillDataTableColumn]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[FName_44]] - code
- [[GroggyDamageMultiplier]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[InputWindowDuration]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[InterStageCooldown]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[MVSkillDataTableColumn.h]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[MpCost]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[NextChainName]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[StaminaCost]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[SwingDirection]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[TSubclassOf_3]] - code
- [[UMVAbilityBase_5]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[UMVCombatComponentCanConsumeActionCost()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveHeavyChargeCommitTime()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[bIsChained_1]] - code - Source/Maverick/Public/Tables/MVSkillDataTableColumn.h
- [[uint32_2]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_전투 액션 매핑]]
- 2 edges to [[_COMMUNITY_전투 액션 행 해석]]
- 1 edge to [[_COMMUNITY_어빌리티 피격 Launch]]
- 1 edge to [[_COMMUNITY_어빌리티 기본 계약]]
- 1 edge to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_전투 액션 런타임 상태]]
- 1 edge to [[_COMMUNITY_스킬 체인 런타임]]
- 1 edge to [[_COMMUNITY_피니셔 실행과 워핑]]
- 1 edge to [[_COMMUNITY_액션 행·몽타주 타입]]

## Top bridge nodes
- [[FMVSkillDataTableColumn]] - degree 31, connects to 5 communities
- [[MVSkillDataTableColumn.h]] - degree 6, connects to 4 communities
- [[UMVCombatComponentCanConsumeActionCost()]] - degree 2, connects to 1 community
- [[UMVCombatComponentResolveHeavyChargeCommitTime()]] - degree 2, connects to 1 community