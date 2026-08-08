---
type: community
cohesion: 0.21
members: 13
---

# MVHitReactionLogRecoveryTrace

**Cohesion:** 0.21 - loosely connected
**Members:** 13 nodes

## Members
- [[EMVActionInputDirection_3]] - code
- [[FString_19]] - code
- [[MVHitReactionDebugBoolText()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionLogRecoveryTrace()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionRecoveryDirectionToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionRecoveryInputDirectionToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[TCHAR_7]] - code
- [[UMVHitReactionComponentActionInputDirectionToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentCharacterIndexCodeToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentHitReactionDirectionToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeEscapeDodgeRecoveryActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryHandleRecoveryWindowOpened()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryStartEscapeDodgeRecoveryAction()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/MVHitReactionLogRecoveryTrace
SORT file.name ASC
```

## Connections to other communities
- 10 edges to [[_COMMUNITY_MVHitReactionComponent.cpp]]
- 6 edges to [[_COMMUNITY_MVHitReactionLogRecoveryTrace_1]]
- 5 edges to [[_COMMUNITY_FName_1]]
- 3 edges to [[_COMMUNITY_FMVResolvedHitData]]
- 2 edges to [[_COMMUNITY_MVHitReactionBuildAvailableRowNameLog]]
- 1 edge to [[_COMMUNITY_MVAbilityBase.cpp]]
- 1 edge to [[_COMMUNITY_MVExecuteAttackTask.cpp]]
- 1 edge to [[_COMMUNITY_MVActionInputHandlerInterface.h]]

## Top bridge nodes
- [[MVHitReactionLogRecoveryTrace()]] - degree 16, connects to 7 communities
- [[UMVHitReactionComponentMakeEscapeDodgeRecoveryActionRowName()]] - degree 4, connects to 3 communities
- [[FString_19]] - degree 7, connects to 2 communities
- [[TCHAR_7]] - degree 5, connects to 2 communities
- [[MVHitReactionRecoveryDirectionToken()]] - degree 4, connects to 2 communities