---
type: community
cohesion: 0.22
members: 9
---

# 캐릭터 도메인 연동

**Cohesion:** 0.22 - loosely connected
**Members:** 9 nodes

## Members
- [[AMVCharacterBase()]] - code - Source/Maverick/Character/MVCharacterBase.h
- [[UMVFieldTransitionSubsystemResetPlayerStatsForTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVPIEActionTestWidgetResolveAttackerCharacter()]] - code - Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp
- [[UMVPIEActionTestWidgetResolveTargetCharacter()]] - code - Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp
- [[UMVPIEActionTestWidgetSetTargetCharacter()]] - code - Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp
- [[UMVPlayerDodgeApplyDodgeChooserSnapshot()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeApplyDodgeInputContext()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeBeginLockOnPawnRotationSuppressionForDodge()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVUISubsystemResolvePIEActionTestTargetCharacter()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_PIE 피격 판정 테스트]]
- 4 edges to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 3 edges to [[_COMMUNITY_회피 방향 벡터 변환]]
- 3 edges to [[_COMMUNITY_어빌리티 피격 Launch]]
- 2 edges to [[_COMMUNITY_피격 판정 해석 파이프라인]]
- 2 edges to [[_COMMUNITY_필드 전환 관리]]
- 2 edges to [[_COMMUNITY_UI 서브시스템 공용 선언]]
- 2 edges to [[_COMMUNITY_UI 서브시스템 팝업 관리]]
- 1 edge to [[_COMMUNITY_무적 구간 노티파이 상태]]
- 1 edge to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 1 edge to [[_COMMUNITY_공통 캐릭터 선언]]
- 1 edge to [[_COMMUNITY_무기 장착 시각화]]
- 1 edge to [[_COMMUNITY_피격 회복 방향 추적]]
- 1 edge to [[_COMMUNITY_사망 부활 오케스트레이션]]
- 1 edge to [[_COMMUNITY_필드 전환 요청 실행]]
- 1 edge to [[_COMMUNITY_회피 입력 방향 구성]]
- 1 edge to [[_COMMUNITY_회피 입력 컨텍스트]]

## Top bridge nodes
- [[AMVCharacterBase()]] - degree 28, connects to 15 communities
- [[UMVPlayerDodgeApplyDodgeChooserSnapshot()]] - degree 4, connects to 3 communities
- [[UMVPlayerDodgeApplyDodgeInputContext()]] - degree 4, connects to 3 communities
- [[UMVPlayerDodgeBeginLockOnPawnRotationSuppressionForDodge()]] - degree 2, connects to 1 community
- [[UMVFieldTransitionSubsystemResetPlayerStatsForTransition()]] - degree 2, connects to 1 community