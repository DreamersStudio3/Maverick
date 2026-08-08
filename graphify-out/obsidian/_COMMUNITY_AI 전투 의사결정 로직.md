---
type: community
cohesion: 0.14
members: 20
---

# AI 전투 의사결정 로직

**Cohesion:** 0.14 - loosely connected
**Members:** 20 nodes

## Members
- [[dot-FMVAICombatActionCondition()]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[ActionRequest]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[CombatDecisionCanAttackImmediately()]] - code - Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp
- [[CombatDecisionHasReadyCandidate()]] - code - Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp
- [[CombatDecisionIsCandidateReady()]] - code - Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp
- [[FMVAICombatActionCondition]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[FStateTreeExecutionContext_2]] - code
- [[MVCombatDecisionCondition.cpp]] - code - Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp
- [[MaxAbsAngle_1]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[MaxDistance_1]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[Metadata]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[MinDistance_1]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[TArray_4]] - code
- [[TestCondition_2]] - code - Source/Maverick/AI/Condition/MVCombatDecisionCondition.h
- [[bRequiresAirbornePattern]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[bRequiresBackwardPathClear]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[bRequiresForwardPathClear]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[bRequiresLineOfSight]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[bRequiresTargetCanBeAirborne]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[bRequiresTargetNotAirborne]] - code - Source/Maverick/AI/MVAICombatTypes.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/AI___
SORT file.name ASC
```

## Connections to other communities
- 3 edges to [[_COMMUNITY_AI 전투 결정 설정]]
- 2 edges to [[_COMMUNITY_AI 전투 컨텍스트]]
- 2 edges to [[_COMMUNITY_AI 전투 액션 메타데이터]]
- 2 edges to [[_COMMUNITY_AI 공격 실행 로직]]
- 1 edge to [[_COMMUNITY_AI 공격 액션 후보 해석]]
- 1 edge to [[_COMMUNITY_AI 공격 실행 데이터]]

## Top bridge nodes
- [[FMVAICombatActionCondition]] - degree 21, connects to 5 communities
- [[CombatDecisionHasReadyCandidate()]] - degree 7, connects to 1 community
- [[CombatDecisionIsCandidateReady()]] - degree 6, connects to 1 community
- [[TestCondition_2]] - degree 6, connects to 1 community
- [[CombatDecisionCanAttackImmediately()]] - degree 5, connects to 1 community