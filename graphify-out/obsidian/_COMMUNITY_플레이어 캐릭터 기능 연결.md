---
type: community
cohesion: 0.40
members: 5
---

# 플레이어 캐릭터 기능 연결

**Cohesion:** 0.40 - moderately connected
**Members:** 5 nodes

## Members
- [[AMVPlayerCharacter()]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.h
- [[UMVPlayerDodgeGetPlayerCharacter()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeInitialize()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerInteractionDetectorGetPlayerCharacter()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp
- [[UMVPlayerInteractionDetectorInitialize()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 2 edges to [[_COMMUNITY_플레이어 상호작용 감지]]
- 1 edge to [[_COMMUNITY_컴포넌트 이벤트 바인딩 수명주기]]
- 1 edge to [[_COMMUNITY_플레이어 이동 액션 테이블]]

## Top bridge nodes
- [[AMVPlayerCharacter()]] - degree 6, connects to 2 communities
- [[UMVPlayerDodgeGetPlayerCharacter()]] - degree 2, connects to 1 community
- [[UMVPlayerDodgeInitialize()]] - degree 2, connects to 1 community
- [[UMVPlayerInteractionDetectorGetPlayerCharacter()]] - degree 2, connects to 1 community
- [[UMVPlayerInteractionDetectorInitialize()]] - degree 2, connects to 1 community