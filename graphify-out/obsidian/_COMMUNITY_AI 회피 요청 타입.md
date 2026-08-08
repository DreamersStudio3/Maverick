---
type: community
cohesion: 0.33
members: 6
---

# AI 회피 요청 타입

**Cohesion:** 0.33 - loosely connected
**Members:** 6 nodes

## Members
- [[AActor_25]] - code - Source/Maverick/Public/Struct/MVAIDodgeTypes.h
- [[EMVActionInputDirection]] - code
- [[EnemyDodgeActionTaskResolveDirection()]] - code - Source/Maverick/AI/Task/MVEnemyDodgeActionTask.cpp
- [[FMVAIDodgeRequest()]] - code - Source/Maverick/Public/Struct/MVAIDodgeTypes.h
- [[MVAIDodgeTypes.h]] - code - Source/Maverick/Public/Struct/MVAIDodgeTypes.h
- [[MVDodgeThreatEvaluatorLibrary.h]] - code - Source/Maverick/Public/AI/MVDodgeThreatEvaluatorLibrary.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/AI___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_AI 적 회피 실행]]
- 1 edge to [[_COMMUNITY_적 회피 StateTree 선언]]
- 1 edge to [[_COMMUNITY_AI 적 회피 설정]]
- 1 edge to [[_COMMUNITY_위협 기반 회피 토큰]]
- 1 edge to [[_COMMUNITY_AI 회피 위협 판정]]

## Top bridge nodes
- [[FMVAIDodgeRequest()]] - degree 4, connects to 2 communities
- [[EnemyDodgeActionTaskResolveDirection()]] - degree 4, connects to 1 community
- [[MVAIDodgeTypes.h]] - degree 4, connects to 1 community
- [[MVDodgeThreatEvaluatorLibrary.h]] - degree 2, connects to 1 community