---
type: community
cohesion: 0.40
members: 5
---

# 액션 전환 입력 관리

**Cohesion:** 0.40 - moderately connected
**Members:** 5 nodes

## Members
- [[AMVCharacterBase_7]] - code - Source/Maverick/Components/MVInputManagerComponent.h
- [[MVActionInputPhase.h]] - code - Source/Maverick/Public/Enum/MVActionInputPhase.h
- [[MVInputManagerComponent.h]] - code - Source/Maverick/Components/MVInputManagerComponent.h
- [[UMVInputManagerComponent()]] - code - Source/Maverick/Components/MVInputManagerComponent.h
- [[UMVPlayerDodgeCanTransitionCurrentAction()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_이동 입력 차단 Notify]]
- 1 edge to [[_COMMUNITY_이동 차단과 회복 탈출]]
- 1 edge to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 1 edge to [[_COMMUNITY_AI 공격 실행 로직]]
- 1 edge to [[_COMMUNITY_피격 회복 방향 추적]]
- 1 edge to [[_COMMUNITY_액션 입력 버퍼링]]
- 1 edge to [[_COMMUNITY_플레이어 회피 입력 계약]]

## Top bridge nodes
- [[UMVInputManagerComponent()]] - degree 5, connects to 3 communities
- [[UMVPlayerDodgeCanTransitionCurrentAction()]] - degree 3, connects to 2 communities
- [[MVActionInputPhase.h]] - degree 3, connects to 2 communities