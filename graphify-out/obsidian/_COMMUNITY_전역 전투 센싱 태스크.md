---
type: community
cohesion: 0.24
members: 10
---

# 전역 전투 센싱 태스크

**Cohesion:** 0.24 - loosely connected
**Members:** 10 nodes

## Members
- [[dot-GetInstanceDataType()_16]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.h
- [[EnterState_10]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.h
- [[ExitState_8]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.h
- [[FMVGlobalSensingTask]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.h
- [[FStateTreeExecutionContext_15]] - code
- [[FStateTreeTaskCommonBase_9]] - code
- [[FStateTreeTransitionResult_9]] - code
- [[MVAttackDirection.h]] - code - Source/Maverick/AI/Enum/MVAttackDirection.h
- [[MVGlobalSensingTask.h]] - code - Source/Maverick/AI/Task/MVGlobalSensingTask.h
- [[UStruct_15]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 8 edges to [[_COMMUNITY_AI 전역 감지 갱신]]
- 2 edges to [[_COMMUNITY_AI 전투 액션 메타데이터]]
- 1 edge to [[_COMMUNITY_AI 기본 공격 태스크]]
- 1 edge to [[_COMMUNITY_AI 공격 실행 데이터]]
- 1 edge to [[_COMMUNITY_필드 전환 리셋 계약]]
- 1 edge to [[_COMMUNITY_AI 전역 감지 설정]]

## Top bridge nodes
- [[MVGlobalSensingTask.h]] - degree 6, connects to 3 communities
- [[MVAttackDirection.h]] - degree 4, connects to 3 communities
- [[EnterState_10]] - degree 7, connects to 1 community
- [[FMVGlobalSensingTask]] - degree 6, connects to 1 community
- [[ExitState_8]] - degree 4, connects to 1 community