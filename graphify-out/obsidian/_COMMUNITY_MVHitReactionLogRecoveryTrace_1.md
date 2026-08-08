---
type: community
cohesion: 0.25
members: 11
---

# MVHitReactionLogRecoveryTrace

**Cohesion:** 0.25 - loosely connected
**Members:** 11 nodes

## Members
- [[EMVActionHitReactionType_2]] - code
- [[EMVHitReactionDirection]] - code
- [[FRotator_5]] - code
- [[MVHitReactionMakeYawSnapRotation()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionShouldLogDirectionTrace()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentCanTriggerGroggyByHitReactionType()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentHitReactionTypeToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeHitReactionActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveHitReactionDirection()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveSupportedHitReactionDirection()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentSnapOwnerYawToHitDirectionForLaunch()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/MVHitReactionLogRecoveryTrace
SORT file.name ASC
```

## Connections to other communities
- 8 edges to [[_COMMUNITY_MVHitReactionComponent.cpp]]
- 6 edges to [[_COMMUNITY_MVHitReactionLogRecoveryTrace]]
- 6 edges to [[_COMMUNITY_FMVResolvedHitData]]
- 3 edges to [[_COMMUNITY_FName_1]]
- 2 edges to [[_COMMUNITY_MVHitReactionComponent.h]]

## Top bridge nodes
- [[EMVHitReactionDirection]] - degree 11, connects to 3 communities
- [[UMVHitReactionComponentSnapOwnerYawToHitDirectionForLaunch()]] - degree 7, connects to 3 communities
- [[EMVActionHitReactionType_2]] - degree 7, connects to 2 communities
- [[MVHitReactionMakeYawSnapRotation()]] - degree 5, connects to 2 communities
- [[MVHitReactionShouldLogDirectionTrace()]] - degree 5, connects to 2 communities