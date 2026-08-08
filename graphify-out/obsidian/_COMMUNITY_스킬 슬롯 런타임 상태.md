---
type: community
cohesion: 0.20
members: 10
---

# 스킬 슬롯 런타임 상태

**Cohesion:** 0.20 - loosely connected
**Members:** 10 nodes

## Members
- [[ActiveStackIndex]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[ChainWindowDuration]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[ChainWindowRemaining]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[CooldownDuration_2]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[CooldownRemaining]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[FMVSkillSlotRuntimeState]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[StackSize]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bAvailable]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bChainActive_1]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bOnCooldown]] - code - Source/Maverick/Components/MVCombatComponent.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_전투 액션 런타임 상태]]
- 1 edge to [[_COMMUNITY_전투 액션 매핑]]

## Top bridge nodes
- [[FMVSkillSlotRuntimeState]] - degree 12, connects to 2 communities