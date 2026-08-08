---
type: community
members: 20
---

# MVHitReactionLogRecoveryTrace

**Members:** 20 nodes

## Members
- [[EMVActionHitReactionType_2]] - code
- [[EMVActionInputDirection_3]] - code
- [[EMVHitReactionDirection]] - code
- [[FString_19]] - code
- [[MVHitReactionDebugBoolText()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionLogRecoveryTrace()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionRecoveryDirectionToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionRecoveryInputDirectionToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[TCHAR_7]] - code
- [[UMVHitReactionComponentActionInputDirectionToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentCanTriggerGroggyByHitReactionType()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentCharacterIndexCodeToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentHitReactionDirectionToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentHitReactionTypeToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeEscapeDodgeRecoveryActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeGetupRecoveryActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeHitReactionActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveSupportedHitReactionDirection()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryHandleRecoveryWindowOpened()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryStartEscapeDodgeRecoveryAction()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/MVHitReactionLogRecoveryTrace
SORT file.name ASC
```

## Connections to other communities
- 15 edges to [[_COMMUNITY_MVHitReactionComponent.cpp]]
- 7 edges to [[_COMMUNITY_FMVResolvedHitData]]
- 7 edges to [[_COMMUNITY_FName_1]]
- 2 edges to [[_COMMUNITY_MVHitReactionBuildAvailableRowNameLog]]
- 2 edges to [[_COMMUNITY_MVHitReactionComponent.h]]
- 1 edge to [[_COMMUNITY_AMVCharacterBase]]
- 1 edge to [[_COMMUNITY_MVExecuteAttackTask.cpp]]
- 1 edge to [[_COMMUNITY_FindBlockMovementInputInputManagerComponent]]

## Top bridge nodes
- [[MVHitReactionLogRecoveryTrace()]] - degree 16, connects to 6 communities
- [[EMVHitReactionDirection]] - degree 11, connects to 2 communities
- [[EMVActionHitReactionType_2]] - degree 7, connects to 2 communities
- [[TCHAR_7]] - degree 5, connects to 2 communities
- [[UMVHitReactionComponentMakeHitReactionActionRowName()]] - degree 4, connects to 2 communities