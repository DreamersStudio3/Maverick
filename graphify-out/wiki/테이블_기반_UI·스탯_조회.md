# 테이블 기반 UI·스탯 조회

> 6 nodes

## Key Concepts

- **UMVTableManager** (13 connections) — `Source/Maverick/Components/MVStatComponent.h`
- **UMVUISubsystem::ShowDialogueWindowById()** (4 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVUISubsystem::ShowPopupMessageById()** (4 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVStatComponent::LoadStatsFromTable()** (2 connections) — `Source/Maverick/Components/MVStatComponent.cpp`
- **FName** (2 connections)
- **UMVLoadingWindow::LoadLoadingGuideCards()** (2 connections) — `Source/Maverick/UI/Window/MVLoadingWindow.cpp`

## Relationships

- [DataTable 에셋 생성](DataTable_%EC%97%90%EC%85%8B_%EC%83%9D%EC%84%B1.md) (2 shared connections)
- [UI 서브시스템 팝업 관리](UI_%EC%84%9C%EB%B8%8C%EC%8B%9C%EC%8A%A4%ED%85%9C_%ED%8C%9D%EC%97%85_%EA%B4%80%EB%A6%AC.md) (2 shared connections)
- [캐릭터 스탯 관리](%EC%BA%90%EB%A6%AD%ED%84%B0_%EC%8A%A4%ED%83%AF_%EA%B4%80%EB%A6%AC.md) (1 shared connections)
- [회피 입력 컨텍스트](%ED%9A%8C%ED%94%BC_%EC%9E%85%EB%A0%A5_%EC%BB%A8%ED%85%8D%EC%8A%A4%ED%8A%B8.md) (1 shared connections)
- [회피·질주 액션 데이터](%ED%9A%8C%ED%94%BC%C2%B7%EC%A7%88%EC%A3%BC_%EC%95%A1%EC%85%98_%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (1 shared connections)
- [액션 Row 몽타주 해석](%EC%95%A1%EC%85%98_Row_%EB%AA%BD%ED%83%80%EC%A3%BC_%ED%95%B4%EC%84%9D.md) (1 shared connections)
- [사망 액션 행 이름 해석](%EC%82%AC%EB%A7%9D_%EC%95%A1%EC%85%98_%ED%96%89_%EC%9D%B4%EB%A6%84_%ED%95%B4%EC%84%9D.md) (1 shared connections)
- [피격 반응 행 조회](%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_%ED%96%89_%EC%A1%B0%ED%9A%8C.md) (1 shared connections)
- [회복 스탯 일시정지 노티파이](%ED%9A%8C%EB%B3%B5_%EC%8A%A4%ED%83%AF_%EC%9D%BC%EC%8B%9C%EC%A0%95%EC%A7%80_%EB%85%B8%ED%8B%B0%ED%8C%8C%EC%9D%B4.md) (1 shared connections)
- [런타임 테이블 조회](%EB%9F%B0%ED%83%80%EC%9E%84_%ED%85%8C%EC%9D%B4%EB%B8%94_%EC%A1%B0%ED%9A%8C.md) (1 shared connections)
- [대화창 표시 대기열 수명주기](%EB%8C%80%ED%99%94%EC%B0%BD_%ED%91%9C%EC%8B%9C_%EB%8C%80%EA%B8%B0%EC%97%B4_%EC%88%98%EB%AA%85%EC%A3%BC%EA%B8%B0.md) (1 shared connections)
- [UI 팝업 메시지 데이터](UI_%ED%8C%9D%EC%97%85_%EB%A9%94%EC%8B%9C%EC%A7%80_%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (1 shared connections)

## Source Files

- `Source/Maverick/Components/MVStatComponent.cpp`
- `Source/Maverick/Components/MVStatComponent.h`
- `Source/Maverick/UI/System/MVUISubsystem.cpp`
- `Source/Maverick/UI/Window/MVLoadingWindow.cpp`

## Audit Trail

- EXTRACTED: 27 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*