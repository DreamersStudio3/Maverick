---
type: community
cohesion: 0.15
members: 20
---

# 피격 회복 방향 추적

**Cohesion:** 0.15 - loosely connected
**Members:** 20 nodes

## Members
- [[EMVActionHitReactionType_2]] - code
- [[EMVActionInputDirection_3]] - code
- [[EMVHitReactionDirection]] - code
- [[FString_18]] - code
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
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 15 edges to [[_COMMUNITY_피격 반응 상태 관리]]
- 7 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 7 edges to [[_COMMUNITY_피격 리액션 입력·행 해석]]
- 4 edges to [[_COMMUNITY_피격 반응 Row 조회]]
- 1 edge to [[_COMMUNITY_PIE 전투 디버그 도구]]
- 1 edge to [[_COMMUNITY_AI 공격 실행 로직]]
- 1 edge to [[_COMMUNITY_이동 차단과 회복 탈출]]

## Top bridge nodes
- [[MVHitReactionLogRecoveryTrace()]] - degree 16, connects to 6 communities
- [[EMVHitReactionDirection]] - degree 11, connects to 2 communities
- [[EMVActionHitReactionType_2]] - degree 7, connects to 2 communities
- [[TCHAR_7]] - degree 5, connects to 2 communities
- [[UMVHitReactionComponentMakeEscapeDodgeRecoveryActionRowName()]] - degree 4, connects to 2 communities