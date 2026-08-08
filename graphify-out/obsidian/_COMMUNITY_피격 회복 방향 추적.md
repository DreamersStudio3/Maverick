---
type: community
members: 20
---

# 피격 회복 방향 추적

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
- 2 edges to [[_COMMUNITY_피격·회복 액션 행 조회]]
- 2 edges to [[_COMMUNITY_피격 반응 행 조회]]
- 1 edge to [[_COMMUNITY_어빌리티 피격 Launch]]
- 1 edge to [[_COMMUNITY_AI 공격 액션 실행]]
- 1 edge to [[_COMMUNITY_액션 입력 처리 계약]]

## Top bridge nodes
- [[MVHitReactionLogRecoveryTrace()]] - degree 16, connects to 6 communities
- [[EMVHitReactionDirection]] - degree 11, connects to 2 communities
- [[EMVActionHitReactionType_2]] - degree 7, connects to 2 communities
- [[TCHAR_7]] - degree 5, connects to 2 communities
- [[UMVHitReactionComponentMakeHitReactionActionRowName()]] - degree 4, connects to 2 communities