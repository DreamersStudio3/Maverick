# 스탯 컴포넌트 UI 연결

> 4 nodes · cohesion 0.50

## Key Concepts

- **MVStatComponent.h** (5 connections) — `Source/Maverick/Components/MVStatComponent.h`
- **UMVStatComponent()** (5 connections) — `Source/Maverick/Components/MVStatComponent.h`
- **UMVPlayerStatusWidget::BindToStatComponent()** (2 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **AActor** (1 connections) — `Source/Maverick/Components/MVStatComponent.h`

## Relationships

- [사망 표현 보류·시작](%EC%82%AC%EB%A7%9D_%ED%91%9C%ED%98%84_%EB%B3%B4%EB%A5%98%C2%B7%EC%8B%9C%EC%9E%91.md) (1 shared connections)
- [테이블 기반 UI·스탯 조회](%ED%85%8C%EC%9D%B4%EB%B8%94_%EA%B8%B0%EB%B0%98_UI%C2%B7%EC%8A%A4%ED%83%AF_%EC%A1%B0%ED%9A%8C.md) (1 shared connections)
- [피격 시스템 공용 타입](%ED%94%BC%EA%B2%A9_%EC%8B%9C%EC%8A%A4%ED%85%9C_%EA%B3%B5%EC%9A%A9_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [스탯 회복 정지 Notify](%EC%8A%A4%ED%83%AF_%ED%9A%8C%EB%B3%B5_%EC%A0%95%EC%A7%80_Notify.md) (1 shared connections)
- [HUD 스탯 회복 연동](HUD_%EC%8A%A4%ED%83%AF_%ED%9A%8C%EB%B3%B5_%EC%97%B0%EB%8F%99.md) (1 shared connections)
- [메인 HUD 상태 바인딩](%EB%A9%94%EC%9D%B8_HUD_%EC%83%81%ED%83%9C_%EB%B0%94%EC%9D%B8%EB%94%A9.md) (1 shared connections)
- [플레이어 상태 UI 생명주기](%ED%94%8C%EB%A0%88%EC%9D%B4%EC%96%B4_%EC%83%81%ED%83%9C_UI_%EC%83%9D%EB%AA%85%EC%A3%BC%EA%B8%B0.md) (1 shared connections)

## Source Files

- `Source/Maverick/Components/MVStatComponent.h`
- `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`

## Audit Trail

- EXTRACTED: 13 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*