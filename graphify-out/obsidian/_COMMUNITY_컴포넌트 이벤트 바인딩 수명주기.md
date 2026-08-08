---
type: community
cohesion: 0.25
members: 8
---

# 컴포넌트 이벤트 바인딩 수명주기

**Cohesion:** 0.25 - loosely connected
**Members:** 8 nodes

## Members
- [[BindObservedTargetDamage]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[BindOwnerEvents]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[UMVEnemyDodgeTokenComponentBeginPlay()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentEndPlay()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVPlayerConsumableDeinitialize()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableInitialize()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UnbindObservedTargetDamage]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[UnbindOwnerEvents]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_적 회피 토큰 상태]]
- 2 edges to [[_COMMUNITY_플레이어 회복약 액션]]
- 2 edges to [[_COMMUNITY_적 회피 토큰 지급]]
- 1 edge to [[_COMMUNITY_플레이어 캐릭터 기능 연결]]
- 1 edge to [[_COMMUNITY_피니셔 실행과 워핑]]

## Top bridge nodes
- [[UMVPlayerConsumableInitialize()]] - degree 4, connects to 2 communities
- [[UMVEnemyDodgeTokenComponentEndPlay()]] - degree 4, connects to 2 communities
- [[UnbindOwnerEvents]] - degree 4, connects to 1 community
- [[BindOwnerEvents]] - degree 3, connects to 1 community
- [[UMVEnemyDodgeTokenComponentBeginPlay()]] - degree 3, connects to 1 community