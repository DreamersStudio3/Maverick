# AI 공격 실행 로직

> 32 nodes · cohesion 0.14

## Key Concepts

- **MVExecuteAttackTask.cpp** (21 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **EnterState** (12 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **EnterState** (10 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **FStateTreeExecutionContext** (7 connections)
- **ExecuteAttackIsStartedActionRunning()** (7 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExecuteAttackTryStartAction()** (7 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExecuteAttackResolveOwner()** (6 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExecuteAttackSelectCandidate()** (6 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **FMVExecuteFixedAttackTask** (6 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **FMVSelectAndExecuteAttackTask** (6 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **ExecuteAttackCanSelectCandidate()** (5 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExecuteAttackResolveActionRequest()** (5 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExecuteAttackStartCooldown()** (5 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExitState** (5 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **Tick** (5 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **ExitState** (5 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **Tick** (5 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **UMVActionComponent()** (5 connections) — `Source/Maverick/Components/MVActionComponent.h`
- **EStateTreeRunStatus** (4 connections)
- **FStateTreeTransitionResult** (4 connections)
- **ExecuteAttackActionTableNameFromDataTable()** (4 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **APawn** (3 connections)
- **FName** (3 connections)
- **.GetInstanceDataType()** (2 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- **.GetInstanceDataType()** (2 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- *... and 7 more nodes in this community*

## Relationships

- [AI 공격 액션 후보 해석](AI_%EA%B3%B5%EA%B2%A9_%EC%95%A1%EC%85%98_%ED%9B%84%EB%B3%B4_%ED%95%B4%EC%84%9D.md) (8 shared connections)
- [AI 전투 액션 메타데이터](AI_%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_%EB%A9%94%ED%83%80%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (3 shared connections)
- [AI 전투 의사결정 로직](AI_%EC%A0%84%ED%88%AC_%EC%9D%98%EC%82%AC%EA%B2%B0%EC%A0%95_%EB%A1%9C%EC%A7%81.md) (2 shared connections)
- [AI 전투 컨텍스트](AI_%EC%A0%84%ED%88%AC_%EC%BB%A8%ED%85%8D%EC%8A%A4%ED%8A%B8.md) (2 shared connections)
- [AI 공격 실행 데이터](AI_%EA%B3%B5%EA%B2%A9_%EC%8B%A4%ED%96%89_%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (2 shared connections)
- [무기 장착 시각화](%EB%AC%B4%EA%B8%B0_%EC%9E%A5%EC%B0%A9_%EC%8B%9C%EA%B0%81%ED%99%94.md) (1 shared connections)
- [액션 전환 입력 관리](%EC%95%A1%EC%85%98_%EC%A0%84%ED%99%98_%EC%9E%85%EB%A0%A5_%EA%B4%80%EB%A6%AC.md) (1 shared connections)
- [액션 행·몽타주 타입](%EC%95%A1%EC%85%98_%ED%96%89%C2%B7%EB%AA%BD%ED%83%80%EC%A3%BC_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [피격 회복 방향 추적](%ED%94%BC%EA%B2%A9_%ED%9A%8C%EB%B3%B5_%EB%B0%A9%ED%96%A5_%EC%B6%94%EC%A0%81.md) (1 shared connections)

## Source Files

- `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- `Source/Maverick/AI/Task/MVExecuteAttackTask.h`
- `Source/Maverick/Components/MVActionComponent.h`

## Audit Trail

- EXTRACTED: 159 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*