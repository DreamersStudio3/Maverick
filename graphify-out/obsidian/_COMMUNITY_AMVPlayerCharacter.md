---
type: community
cohesion: 0.40
members: 5
---

# AMVPlayerCharacter

**Cohesion:** 0.40 - moderately connected
**Members:** 5 nodes

## Members
- [[AMVPlayerCharacter()]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.h
- [[UMVPlayerDodgeGetPlayerCharacter()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeInitialize()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerInteractionDetectorGetPlayerCharacter()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp
- [[UMVPlayerInteractionDetectorInitialize()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/AMVPlayerCharacter
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_FVector]]
- 2 edges to [[_COMMUNITY_MVPlayerInteractionDetector.cpp]]
- 1 edge to [[_COMMUNITY_MVPlayerConsumable.cpp]]
- 1 edge to [[_COMMUNITY_MVMovementActionTableTypes.h]]

## Top bridge nodes
- [[AMVPlayerCharacter()]] - degree 6, connects to 2 communities
- [[UMVPlayerDodgeGetPlayerCharacter()]] - degree 2, connects to 1 community
- [[UMVPlayerDodgeInitialize()]] - degree 2, connects to 1 community
- [[UMVPlayerInteractionDetectorGetPlayerCharacter()]] - degree 2, connects to 1 community
- [[UMVPlayerInteractionDetectorInitialize()]] - degree 2, connects to 1 community