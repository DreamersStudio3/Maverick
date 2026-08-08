---
type: community
cohesion: 0.26
members: 12
---

# AI 공격 액션 후보 해석

**Cohesion:** 0.26 - loosely connected
**Members:** 12 nodes

## Members
- [[dot-Reset()]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[ActionRow]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[ExecuteAttackEvaluateChooserActionRowHandle()]] - code - Source/Maverick/AI/Task/MVExecuteAttackTask.cpp
- [[ExecuteAttackMakeCombatChooserInput()]] - code - Source/Maverick/AI/Task/MVExecuteAttackTask.cpp
- [[ExecuteAttackResolveActionCandidate()]] - code - Source/Maverick/AI/Task/MVExecuteAttackTask.cpp
- [[ExecuteAttackResolveActionTypeTag()]] - code - Source/Maverick/AI/Task/MVExecuteAttackTask.cpp
- [[FDataTableRowHandle]] - code
- [[FGameplayTag]] - code
- [[FMVActionRequest()]] - code - Source/Maverick/Public/Tables/MVActionTableTypes.h
- [[FMVAttackActionRowHandle]] - code - Source/Maverick/AI/MVAICombatTypes.h
- [[FSoftObjectPath_1]] - code
- [[UObject_2]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/AI____
SORT file.name ASC
```

## Connections to other communities
- 8 edges to [[_COMMUNITY_AI 공격 실행 로직]]
- 5 edges to [[_COMMUNITY_AI 전투 액션 메타데이터]]
- 4 edges to [[_COMMUNITY_AI 공격 실행 데이터]]
- 2 edges to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_AI 적 사망 처리]]
- 1 edge to [[_COMMUNITY_AI 전투 의사결정 로직]]
- 1 edge to [[_COMMUNITY_피격 시스템 공용 타입]]

## Top bridge nodes
- [[FMVActionRequest()]] - degree 10, connects to 5 communities
- [[FMVAttackActionRowHandle]] - degree 9, connects to 3 communities
- [[ExecuteAttackResolveActionCandidate()]] - degree 10, connects to 2 communities
- [[ExecuteAttackEvaluateChooserActionRowHandle()]] - degree 8, connects to 2 communities
- [[ExecuteAttackMakeCombatChooserInput()]] - degree 6, connects to 2 communities