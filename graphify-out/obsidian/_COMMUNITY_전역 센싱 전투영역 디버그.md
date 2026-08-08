---
type: community
cohesion: 0.33
members: 7
---

# 전역 센싱 전투영역 디버그

**Cohesion:** 0.33 - loosely connected
**Members:** 7 nodes

## Members
- [[DrawGlobalSensingCombatAreaCircle()]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.cpp
- [[DrawGlobalSensingCombatAreaDebug()]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.cpp
- [[EMVBossCombatArea_2]] - code
- [[FVector_7]] - code
- [[GlobalSensingCombatAreaText()]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.cpp
- [[TCHAR]] - code
- [[UWorld_3]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_AI 전역 감지 갱신]]
- 1 edge to [[_COMMUNITY_상호작용 대상 시각 갱신]]
- 1 edge to [[_COMMUNITY_AI 전역 감지 설정]]

## Top bridge nodes
- [[DrawGlobalSensingCombatAreaDebug()]] - degree 6, connects to 2 communities
- [[DrawGlobalSensingCombatAreaCircle()]] - degree 5, connects to 2 communities
- [[GlobalSensingCombatAreaText()]] - degree 4, connects to 1 community
- [[FVector_7]] - degree 3, connects to 1 community