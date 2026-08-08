---
type: community
members: 4
---

# 회피 전투 카운터 조회

**Members:** 4 nodes

## Members
- [[dot-GetDodgeTokenCount()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[dot-GetLandedHitCount()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[dot-GetReceivedHitCount()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[int32_18]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_적 회피 토큰 상태]]

## Top bridge nodes
- [[int32_18]] - degree 4, connects to 1 community
- [[dot-GetDodgeTokenCount()]] - degree 2, connects to 1 community
- [[dot-GetReceivedHitCount()]] - degree 2, connects to 1 community
- [[dot-GetLandedHitCount()]] - degree 2, connects to 1 community