---
type: community
cohesion: 0.67
members: 3
---

# 캐릭터 공중 피격 추적

**Cohesion:** 0.67 - moderately connected
**Members:** 3 nodes

## Members
- [[AMVCharacterBaseOnHitResolved()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[MVCharacterLogAirborneTrace()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[TCHAR_3]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_공통 캐릭터 생명주기]]
- 2 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 1 edge to [[_COMMUNITY_PIE 전투 디버그 도구]]

## Top bridge nodes
- [[MVCharacterLogAirborneTrace()]] - degree 5, connects to 3 communities
- [[AMVCharacterBaseOnHitResolved()]] - degree 3, connects to 2 communities