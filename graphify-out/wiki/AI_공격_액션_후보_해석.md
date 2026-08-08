# AI 공격 액션 후보 해석

> 12 nodes · cohesion 0.26

## Key Concepts

- **ExecuteAttackResolveActionCandidate()** (10 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **FMVActionRequest()** (10 connections) — `Source/Maverick/Public/Tables/MVActionTableTypes.h`
- **FMVAttackActionRowHandle** (9 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **ExecuteAttackEvaluateChooserActionRowHandle()** (8 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExecuteAttackMakeCombatChooserInput()** (6 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **ExecuteAttackResolveActionTypeTag()** (4 connections) — `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- **FDataTableRowHandle** (2 connections)
- **FSoftObjectPath** (2 connections)
- **UObject** (2 connections)
- **ActionRow** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **.Reset()** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **FGameplayTag** (1 connections)

## Relationships

- [AI 공격 실행 로직](AI_%EA%B3%B5%EA%B2%A9_%EC%8B%A4%ED%96%89_%EB%A1%9C%EC%A7%81.md) (8 shared connections)
- [AI 전투 액션 메타데이터](AI_%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_%EB%A9%94%ED%83%80%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (5 shared connections)
- [AI 공격 실행 데이터](AI_%EA%B3%B5%EA%B2%A9_%EC%8B%A4%ED%96%89_%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (4 shared connections)
- [전투 액션 Chooser 입력](%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_Chooser_%EC%9E%85%EB%A0%A5.md) (2 shared connections)
- [AI 적 사망 처리](AI_%EC%A0%81_%EC%82%AC%EB%A7%9D_%EC%B2%98%EB%A6%AC.md) (1 shared connections)
- [AI 전투 의사결정 로직](AI_%EC%A0%84%ED%88%AC_%EC%9D%98%EC%82%AC%EA%B2%B0%EC%A0%95_%EB%A1%9C%EC%A7%81.md) (1 shared connections)
- [플레이어 이동 액션 테이블](%ED%94%8C%EB%A0%88%EC%9D%B4%EC%96%B4_%EC%9D%B4%EB%8F%99_%EC%95%A1%EC%85%98_%ED%85%8C%EC%9D%B4%EB%B8%94.md) (1 shared connections)

## Source Files

- `Source/Maverick/AI/MVAICombatTypes.h`
- `Source/Maverick/AI/Task/MVExecuteAttackTask.cpp`
- `Source/Maverick/Public/Tables/MVActionTableTypes.h`

## Audit Trail

- EXTRACTED: 56 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*