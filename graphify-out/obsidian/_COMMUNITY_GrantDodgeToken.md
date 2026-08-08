---
type: community
cohesion: 0.33
members: 6
---

# GrantDodgeToken

**Cohesion:** 0.33 - loosely connected
**Members:** 6 nodes

## Members
- [[GrantDodgeToken]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[IsRelevantEnemyHitData]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[IsRelevantLandedHitData]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[UMVEnemyDodgeTokenComponentHandleOwnerGroggyEnded()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentNotifyEnemyDamaged()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentNotifyEnemyLandedHit()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/GrantDodgeToken
SORT file.name ASC
```

## Connections to other communities
- 3 edges to [[_COMMUNITY_MVEnemyDodgeTokenComponent.cpp]]
- 3 edges to [[_COMMUNITY_EMVEnemyDodgeTokenGrantReason]]
- 2 edges to [[_COMMUNITY_FMVResolvedHitData]]

## Top bridge nodes
- [[UMVEnemyDodgeTokenComponentNotifyEnemyDamaged()]] - degree 4, connects to 2 communities
- [[UMVEnemyDodgeTokenComponentNotifyEnemyLandedHit()]] - degree 4, connects to 2 communities
- [[GrantDodgeToken]] - degree 4, connects to 1 community
- [[IsRelevantEnemyHitData]] - degree 2, connects to 1 community
- [[IsRelevantLandedHitData]] - degree 2, connects to 1 community