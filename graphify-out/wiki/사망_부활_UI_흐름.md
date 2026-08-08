# 사망 부활 UI 흐름

> 9 nodes

## Key Concepts

- **MVDeathRespawnFlow.h** (6 connections) — `Source/Maverick/System/MVDeathRespawnFlow.h`
- **UMVFieldTransitionSubsystem** (3 connections) — `Source/Maverick/System/MVDeathRespawnFlow.h`
- **UMVFieldTransitionSubsystem::Get()** (3 connections) — `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`
- **UMVDeathRespawnFlow::Initialize()** (2 connections) — `Source/Maverick/System/MVDeathRespawnFlow.cpp`
- **AMVCharacterBase** (1 connections) — `Source/Maverick/System/MVDeathRespawnFlow.h`
- **UMVDeathComponent** (1 connections) — `Source/Maverick/System/MVDeathRespawnFlow.h`
- **UMVUISubsystem** (1 connections) — `Source/Maverick/System/MVDeathRespawnFlow.h`
- **UMVDeathRespawnFlow()** (1 connections) — `Source/Maverick/System/MVDeathRespawnFlow.h`
- **UObject** (1 connections)

## Relationships

- [사망 부활 오케스트레이션](%EC%82%AC%EB%A7%9D_%EB%B6%80%ED%99%9C_%EC%98%A4%EC%BC%80%EC%8A%A4%ED%8A%B8%EB%A0%88%EC%9D%B4%EC%85%98.md) (1 shared connections)
- [필드 전환 리셋 계약](%ED%95%84%EB%93%9C_%EC%A0%84%ED%99%98_%EB%A6%AC%EC%85%8B_%EA%B3%84%EC%95%BD.md) (1 shared connections)
- [필드 전환 관리](%ED%95%84%EB%93%9C_%EC%A0%84%ED%99%98_%EA%B4%80%EB%A6%AC.md) (1 shared connections)

## Source Files

- `Source/Maverick/System/MVDeathRespawnFlow.cpp`
- `Source/Maverick/System/MVDeathRespawnFlow.h`
- `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`

## Audit Trail

- EXTRACTED: 19 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*