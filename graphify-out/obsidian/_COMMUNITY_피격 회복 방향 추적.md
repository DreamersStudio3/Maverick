---
type: community
cohesion: 0.14
members: 22
---

# 피격 회복 방향 추적

**Cohesion:** 0.14 - loosely connected
**Members:** 22 nodes

## Members
- [[EMVActionHitReactionType_2]] - code
- [[EMVActionInputDirection_3]] - code
- [[EMVHitReactionDirection]] - code
- [[FMVHitReactionActionRowHandle()]] - code - Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h
- [[FString_18]] - code
- [[MVHitReactionDebugBoolText()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionLogRecoveryTrace()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionRecoveryDirectionToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionRecoveryInputDirectionToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[TCHAR_7]] - code
- [[UMVHitReactionComponentActionInputDirectionToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentCanTriggerGroggyByHitReactionType()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentHitReactionDirectionToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentHitReactionTypeToTableToken()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeEscapeDodgeRecoveryActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeGetupRecoveryActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeHitReactionActionRowHandleFromNames()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeHitReactionActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveHitReactionActionRowHandle()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveSupportedHitReactionDirection()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryHandleRecoveryWindowOpened()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryStartEscapeDodgeRecoveryAction()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 16 edges to [[_COMMUNITY_피격 반응 상태 관리]]
- 8 edges to [[_COMMUNITY_피격 리액션 입력·행 해석]]
- 7 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 3 edges to [[_COMMUNITY_피격 반응 Row 조회]]
- 1 edge to [[_COMMUNITY_캐릭터 도메인 연동]]
- 1 edge to [[_COMMUNITY_AI 공격 실행 로직]]
- 1 edge to [[_COMMUNITY_액션 전환 입력 관리]]
- 1 edge to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_테이블 기반 UI·스탯 조회]]
- 1 edge to [[_COMMUNITY_액션 행·몽타주 타입]]

## Top bridge nodes
- [[MVHitReactionLogRecoveryTrace()]] - degree 16, connects to 6 communities
- [[FMVHitReactionActionRowHandle()]] - degree 5, connects to 3 communities
- [[UMVHitReactionComponentMakeHitReactionActionRowHandleFromNames()]] - degree 4, connects to 3 communities
- [[FString_18]] - degree 7, connects to 2 communities
- [[TCHAR_7]] - degree 5, connects to 2 communities