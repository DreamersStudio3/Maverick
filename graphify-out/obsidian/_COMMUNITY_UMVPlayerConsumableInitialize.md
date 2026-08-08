---
type: community
members: 8
---

# UMVPlayerConsumable::Initialize

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
TABLE source_file, type FROM #community/UMVPlayerConsumableInitialize
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_EMVEnemyDodgeTokenGrantReason]]
- 2 edges to [[_COMMUNITY_MVPlayerConsumable.cpp]]
- 2 edges to [[_COMMUNITY_MVEnemyDodgeTokenComponent.cpp]]
- 1 edge to [[_COMMUNITY_AMVPlayerCharacter]]
- 1 edge to [[_COMMUNITY_Type_1]]

## Top bridge nodes
- [[UMVPlayerConsumableInitialize()]] - degree 4, connects to 2 communities
- [[UMVEnemyDodgeTokenComponentEndPlay()]] - degree 4, connects to 2 communities
- [[UnbindOwnerEvents]] - degree 4, connects to 1 community
- [[UMVEnemyDodgeTokenComponentBeginPlay()]] - degree 3, connects to 1 community
- [[BindOwnerEvents]] - degree 3, connects to 1 community