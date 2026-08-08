---
type: community
cohesion: 0.27
members: 10
---

# 컨트롤러 기준 이동 입력

**Cohesion:** 0.27 - loosely connected
**Members:** 10 nodes

## Members
- [[AMVCharacterBaseAddMovementInput()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[AMVCharacterBaseApplyLocomotionDirectionSnapshot()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[AMVCharacterBaseCacheControllerSpaceMovementInput()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[AMVCharacterBaseResolveControllerSpaceMovementInput()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[AMVCharacterBaseResolveWorldDirectionFromControllerSpaceInput()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[AMVCharacterBaseTryGetControllerSpaceMovementInput()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[ClampCharacterControllerSpaceInput()]] - code - Source/Maverick/Character/MVCharacterBase.cpp
- [[FVector_9]] - code
- [[FVector2D_2]] - code
- [[int32_8]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 7 edges to [[_COMMUNITY_공통 캐릭터 생명주기]]

## Top bridge nodes
- [[AMVCharacterBaseResolveControllerSpaceMovementInput()]] - degree 4, connects to 1 community
- [[ClampCharacterControllerSpaceInput()]] - degree 4, connects to 1 community
- [[AMVCharacterBaseCacheControllerSpaceMovementInput()]] - degree 3, connects to 1 community
- [[AMVCharacterBaseResolveWorldDirectionFromControllerSpaceInput()]] - degree 3, connects to 1 community
- [[AMVCharacterBaseTryGetControllerSpaceMovementInput()]] - degree 3, connects to 1 community