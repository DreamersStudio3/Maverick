---
type: community
cohesion: 0.22
members: 9
---

# 사망 부활 UI 흐름

**Cohesion:** 0.22 - loosely connected
**Members:** 9 nodes

## Members
- [[AMVCharacterBase_9]] - code - Source/Maverick/System/MVDeathRespawnFlow.h
- [[MVDeathRespawnFlow.h]] - code - Source/Maverick/System/MVDeathRespawnFlow.h
- [[MVUISettings.cpp]] - code - Source/Maverick/UI/System/MVUISettings.cpp
- [[UMVDeathComponent_2]] - code - Source/Maverick/System/MVDeathRespawnFlow.h
- [[UMVDeathOverlayWindow]] - code - Source/Maverick/System/MVDeathRespawnFlow.h
- [[UMVDeathRespawnFlow()]] - code - Source/Maverick/System/MVDeathRespawnFlow.h
- [[UMVMainHUDWidget]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[UMVUISettingsUMVUISettings()]] - code - Source/Maverick/UI/System/MVUISettings.cpp
- [[UMVUISubsystem_1]] - code - Source/Maverick/System/MVDeathRespawnFlow.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__UI_
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_필드 전환 리셋 계약]]
- 1 edge to [[_COMMUNITY_필드 전환 사망 연동]]
- 1 edge to [[_COMMUNITY_상호작용 프롬프트 UI]]

## Top bridge nodes
- [[MVDeathRespawnFlow.h]] - degree 6, connects to 1 community
- [[UMVUISettingsUMVUISettings()]] - degree 4, connects to 1 community
- [[UMVMainHUDWidget]] - degree 2, connects to 1 community