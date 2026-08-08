---
type: community
members: 7
---

# 액션 입력 처리 계약

**Members:** 7 nodes

## Members
- [[AMVCharacterBase_7]] - code - Source/Maverick/Components/MVInputManagerComponent.h
- [[IMVActionInputHandlerInterface()]] - code - Source/Maverick/Public/Interface/MVActionInputHandlerInterface.h
- [[MVActionInputHandlerInterface.h]] - code - Source/Maverick/Public/Interface/MVActionInputHandlerInterface.h
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
- 1 edge to [[_COMMUNITY_이동 입력 차단 노티파이]]
- 1 edge to [[_COMMUNITY_회복 탈출 윈도우 노티파이]]
- 1 edge to [[_COMMUNITY_회복 물약 런타임 상태]]
- 1 edge to [[_COMMUNITY_회피 입력 방향 구성]]
- 1 edge to [[_COMMUNITY_AI 공격 액션 실행]]
- 1 edge to [[_COMMUNITY_회피·질주 액션 데이터]]
- 1 edge to [[_COMMUNITY_전투 액션 런타임 상태]]
- 1 edge to [[_COMMUNITY_피니셔 Chooser 입출력]]
- 1 edge to [[_COMMUNITY_피격 회복 방향 추적]]
- 1 edge to [[_COMMUNITY_피격 반응 행 조회]]
- 1 edge to [[_COMMUNITY_입력 차단 회복 취소]]
- 1 edge to [[_COMMUNITY_액션 입력 버퍼링]]

## Top bridge nodes
- [[MVActionInputHandlerInterface.h]] - degree 8, connects to 6 communities
- [[UMVInputManagerComponent()]] - degree 5, connects to 3 communities
- [[UMVPlayerDodgeCanTransitionCurrentAction()]] - degree 3, connects to 2 communities
- [[MVActionInputPhase.h]] - degree 3, connects to 1 community