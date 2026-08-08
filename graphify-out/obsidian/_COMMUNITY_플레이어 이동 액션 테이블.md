---
type: community
cohesion: 0.18
members: 11
---

# 플레이어 이동 액션 테이블

**Cohesion:** 0.18 - loosely connected
**Members:** 11 nodes

## Members
- [[AMVPlayerCharacterFindSprintActionRow()]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.cpp
- [[FMVSprintActionRow()]] - code - Source/Maverick/Public/Tables/MVMovementActionTableTypes.h
- [[MVActionHitReactions()]] - code - Source/Maverick/Public/Tables/MVActionTableTypes.h
- [[MVActionTableTypes.h]] - code - Source/Maverick/Public/Tables/MVActionTableTypes.h
- [[MVMovementActionTableTypes.h]] - code - Source/Maverick/Public/Tables/MVMovementActionTableTypes.h
- [[MVPlayerCharacter.h]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.h
- [[UCameraShakeBase]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.h
- [[UMVPlayerConsumable_1]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.h
- [[UMVPlayerDodge]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.h
- [[UMVPlayerInteractionDetector]] - code - Source/Maverick/Character/PC/MVPlayerCharacter.h
- [[namespace]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_플레이어 캐릭터 전력질주]]
- 2 edges to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 1 edge to [[_COMMUNITY_AI 전투 액션 메타데이터]]
- 1 edge to [[_COMMUNITY_AI 적 회피 설정]]
- 1 edge to [[_COMMUNITY_AI 공격 실행 데이터]]
- 1 edge to [[_COMMUNITY_플레이어 회복약 액션]]
- 1 edge to [[_COMMUNITY_플레이어 회피 입력 계약]]
- 1 edge to [[_COMMUNITY_테이블 기반 UI·스탯 조회]]
- 1 edge to [[_COMMUNITY_플레이어 캐릭터 기능 연결]]
- 1 edge to [[_COMMUNITY_피격 리액션 회복 결정]]
- 1 edge to [[_COMMUNITY_피격 시스템 공용 타입]]
- 1 edge to [[_COMMUNITY_액션 행·몽타주 타입]]
- 1 edge to [[_COMMUNITY_AI 공격 액션 후보 해석]]
- 1 edge to [[_COMMUNITY_테이블 Row 공통 타입]]

## Top bridge nodes
- [[MVActionTableTypes.h]] - degree 8, connects to 6 communities
- [[MVMovementActionTableTypes.h]] - degree 9, connects to 5 communities
- [[MVPlayerCharacter.h]] - degree 7, connects to 2 communities
- [[AMVPlayerCharacterFindSprintActionRow()]] - degree 3, connects to 2 communities