---
type: community
members: 17
---

# 피격 방향과 Launch 처리

**Members:** 17 nodes

## Members
- [[FMVResolvedHitData()]] - code - Source/Maverick/Public/Struct/MVHitTypes.h
- [[FRotator_5]] - code
- [[FVector_14]] - code
- [[MVHitReactionLogAirborneTrace()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionLogHitLaunchTrace()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionMakeYawSnapRotation()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionResolveHitDirection()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionResolveHitSourceDirection()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionShouldLogDirectionTrace()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentApplyHitReactionLaunch()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentCanTriggerGroggy()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentGetActionData()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentHandleDamaged()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveHitReactionDirection()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentSnapOwnerYawToHitDirectionForLaunch()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVStatComponentResetDeathState()]] - code - Source/Maverick/Components/MVStatComponent.cpp
- [[UObject_6]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__Launch_
SORT file.name ASC
```

## Connections to other communities
- 12 edges to [[_COMMUNITY_피격 반응 상태 관리]]
- 7 edges to [[_COMMUNITY_피격 회복 방향 추적]]
- 4 edges to [[_COMMUNITY_적 회피 토큰 지급]]
- 3 edges to [[_COMMUNITY_캐릭터 스탯 관리]]
- 2 edges to [[_COMMUNITY_AI 피격 반응 태스크]]
- 2 edges to [[_COMMUNITY_피격 공중 상태 추적]]
- 2 edges to [[_COMMUNITY_피격 판정 해석 파이프라인]]
- 2 edges to [[_COMMUNITY_적 회피 토큰 부여]]
- 1 edge to [[_COMMUNITY_AI 피격 진입 조건]]
- 1 edge to [[_COMMUNITY_적 캐릭터 전투 실행]]
- 1 edge to [[_COMMUNITY_피격·회복 액션 행 조회]]
- 1 edge to [[_COMMUNITY_피격 리액션 입력·행 해석]]
- 1 edge to [[_COMMUNITY_피격 리액션 액션 데이터]]
- 1 edge to [[_COMMUNITY_피격 시스템 공용 타입]]
- 1 edge to [[_COMMUNITY_보스 체력바 상태 연동]]

## Top bridge nodes
- [[FMVResolvedHitData()]] - degree 29, connects to 10 communities
- [[UMVHitReactionComponentSnapOwnerYawToHitDirectionForLaunch()]] - degree 7, connects to 3 communities
- [[UMVHitReactionComponentGetActionData()]] - degree 5, connects to 3 communities
- [[MVHitReactionLogHitLaunchTrace()]] - degree 6, connects to 2 communities
- [[MVHitReactionLogAirborneTrace()]] - degree 5, connects to 2 communities