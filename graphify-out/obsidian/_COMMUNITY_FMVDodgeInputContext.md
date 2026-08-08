---
type: community
cohesion: 0.14
members: 15
---

# FMVDodgeInputContext

**Cohesion:** 0.14 - loosely connected
**Members:** 15 nodes

## Members
- [[ControllerSpaceInput]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h
- [[ELocomotionDirection_2]] - code
- [[FMVDodgeActionRowHandle()]] - code - Source/Maverick/Public/Tables/MVMovementActionTableTypes.h
- [[FMVDodgeInputContext]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h
- [[FVector_11]] - code
- [[FVector2D_5]] - code
- [[FacingDirection]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h
- [[InputDirection]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h
- [[MovementDirection]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h
- [[RowDirection]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h
- [[UMVPlayerDodgeEvaluateDodgeChooserActionRowHandle()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeResolveDodgeActionRowHandle()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeTryStartDodgeAction()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[bHasMovementInput]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h
- [[bUsesStep]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/FMVDodgeInputContext
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_FVector]]
- 2 edges to [[_COMMUNITY_MVEnemyDodgeActionTask.cpp]]
- 2 edges to [[_COMMUNITY_Player Dodge Tables]]
- 1 edge to [[_COMMUNITY_FMVEnemyDodgeActionTaskInstanceData]]
- 1 edge to [[_COMMUNITY_UMVTableManager]]
- 1 edge to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_UMVPlayerDodgeMakeDodgeActionRowName]]

## Top bridge nodes
- [[FMVDodgeInputContext]] - degree 17, connects to 3 communities
- [[FMVDodgeActionRowHandle()]] - degree 6, connects to 3 communities
- [[UMVPlayerDodgeEvaluateDodgeChooserActionRowHandle()]] - degree 4, connects to 2 communities
- [[UMVPlayerDodgeResolveDodgeActionRowHandle()]] - degree 4, connects to 2 communities
- [[UMVPlayerDodgeTryStartDodgeAction()]] - degree 2, connects to 1 community