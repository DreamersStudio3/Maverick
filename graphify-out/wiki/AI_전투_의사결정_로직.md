# AI 전투 의사결정 로직

> 20 nodes · cohesion 0.14

## Key Concepts

- **FMVAICombatActionCondition** (21 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **CombatDecisionHasReadyCandidate()** (7 connections) — `Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp`
- **CombatDecisionIsCandidateReady()** (6 connections) — `Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp`
- **TestCondition** (6 connections) — `Source/Maverick/AI/Condition/MVCombatDecisionCondition.h`
- **CombatDecisionCanAttackImmediately()** (5 connections) — `Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp`
- **MVCombatDecisionCondition.cpp** (4 connections) — `Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp`
- **FStateTreeExecutionContext** (1 connections)
- **TArray** (1 connections)
- **ActionRequest** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **bRequiresAirbornePattern** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **bRequiresBackwardPathClear** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **bRequiresForwardPathClear** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **bRequiresLineOfSight** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **bRequiresTargetCanBeAirborne** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **bRequiresTargetNotAirborne** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **.FMVAICombatActionCondition()** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **MaxAbsAngle** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **MaxDistance** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **Metadata** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`
- **MinDistance** (1 connections) — `Source/Maverick/AI/MVAICombatTypes.h`

## Relationships

- [AI 전투 결정 설정](AI_%EC%A0%84%ED%88%AC_%EA%B2%B0%EC%A0%95_%EC%84%A4%EC%A0%95.md) (3 shared connections)
- [AI 전투 컨텍스트](AI_%EC%A0%84%ED%88%AC_%EC%BB%A8%ED%85%8D%EC%8A%A4%ED%8A%B8.md) (2 shared connections)
- [AI 전투 액션 메타데이터](AI_%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_%EB%A9%94%ED%83%80%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (2 shared connections)
- [AI 공격 실행 로직](AI_%EA%B3%B5%EA%B2%A9_%EC%8B%A4%ED%96%89_%EB%A1%9C%EC%A7%81.md) (2 shared connections)
- [AI 공격 액션 후보 해석](AI_%EA%B3%B5%EA%B2%A9_%EC%95%A1%EC%85%98_%ED%9B%84%EB%B3%B4_%ED%95%B4%EC%84%9D.md) (1 shared connections)
- [AI 공격 실행 데이터](AI_%EA%B3%B5%EA%B2%A9_%EC%8B%A4%ED%96%89_%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (1 shared connections)

## Source Files

- `Source/Maverick/AI/Condition/MVCombatDecisionCondition.cpp`
- `Source/Maverick/AI/Condition/MVCombatDecisionCondition.h`
- `Source/Maverick/AI/MVAICombatTypes.h`

## Audit Trail

- EXTRACTED: 63 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*