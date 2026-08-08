---
type: community
members: 3
---

# 피격 공중 상태 추적

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
- 2 edges to [[_COMMUNITY_공통 캐릭터 동작]]
- 2 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 1 edge to [[_COMMUNITY_어빌리티 피격 Launch]]

## Top bridge nodes
- [[MVCharacterLogAirborneTrace()]] - degree 5, connects to 3 communities
- [[AMVCharacterBaseOnHitResolved()]] - degree 3, connects to 2 communities