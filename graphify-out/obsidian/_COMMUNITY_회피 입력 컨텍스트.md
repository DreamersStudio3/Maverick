---
type: community
cohesion: 0.14
members: 15
---

# 회피 입력 컨텍스트

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
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 2 edges to [[_COMMUNITY_AI 적 회피 실행]]
- 1 edge to [[_COMMUNITY_AI 적 회피 설정]]
- 1 edge to [[_COMMUNITY_회피 입력 방향 구성]]
- 1 edge to [[_COMMUNITY_캐릭터 도메인 연동]]
- 1 edge to [[_COMMUNITY_테이블 기반 UI·스탯 조회]]
- 1 edge to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_플레이어 회피 입력 계약]]
- 1 edge to [[_COMMUNITY_플레이어 이동 액션 테이블]]

## Top bridge nodes
- [[FMVDodgeInputContext]] - degree 17, connects to 4 communities
- [[FMVDodgeActionRowHandle()]] - degree 6, connects to 3 communities
- [[UMVPlayerDodgeEvaluateDodgeChooserActionRowHandle()]] - degree 4, connects to 2 communities
- [[UMVPlayerDodgeResolveDodgeActionRowHandle()]] - degree 4, connects to 2 communities
- [[UMVPlayerDodgeTryStartDodgeAction()]] - degree 2, connects to 1 community