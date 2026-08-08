---
type: community
members: 7
---

# EMVDeathActionPose

**Members:** 7 nodes

## Members
- [[EMVDeathActionPose]] - code
- [[FString_18]] - code
- [[UMVDeathComponentDeathActionFacingToTableToken()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentDeathActionPoseToTableToken()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentIsLandingDeathActionPose()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentResolveDeathActionPose()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentResolveLandingDeathActionPose()]] - code - Source/Maverick/Components/MVDeathComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/EMVDeathActionPose
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_MVDeathComponent.cpp]]
- 3 edges to [[_COMMUNITY_FName]]
- 1 edge to [[_COMMUNITY_FMVDeathContext]]

## Top bridge nodes
- [[UMVDeathComponentResolveDeathActionPose()]] - degree 3, connects to 2 communities
- [[UMVDeathComponentDeathActionFacingToTableToken()]] - degree 3, connects to 2 communities
- [[EMVDeathActionPose]] - degree 5, connects to 1 community
- [[FString_18]] - degree 3, connects to 1 community
- [[UMVDeathComponentDeathActionPoseToTableToken()]] - degree 3, connects to 1 community