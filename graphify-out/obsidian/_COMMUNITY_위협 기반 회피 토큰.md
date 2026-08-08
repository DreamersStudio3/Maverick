---
type: community
members: 5
---

# 위협 기반 회피 토큰

**Members:** 5 nodes

## Members
- [[AActor_18]] - code
- [[FMVDodgeThreatConfig]] - code
- [[FMVDodgeThreatDecision]] - code
- [[TrySpendDodgeToken]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[UMVEnemyDodgeTokenComponentTryConsumeDodgeTokenForThreat()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_전투 액션 런타임 상태]]
- 1 edge to [[_COMMUNITY_적 회피 토큰 지급]]
- 1 edge to [[_COMMUNITY_AI 회피 방향 결정]]
- 1 edge to [[_COMMUNITY_적 회피 토큰 상태]]

## Top bridge nodes
- [[UMVEnemyDodgeTokenComponentTryConsumeDodgeTokenForThreat()]] - degree 7, connects to 3 communities
- [[TrySpendDodgeToken]] - degree 2, connects to 1 community