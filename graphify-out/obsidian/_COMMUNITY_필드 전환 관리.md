---
type: community
cohesion: 0.08
members: 24
---

# 필드 전환 관리

**Cohesion:** 0.08 - loosely connected
**Members:** 24 nodes

## Members
- [[EMVFieldTransitionPhase]] - code
- [[FSubsystemCollectionBase_1]] - code
- [[FText_3]] - code
- [[MVFieldTransitionSettings.h]] - code - Source/Maverick/System/MVFieldTransitionSettings.h
- [[MVFieldTransitionSubsystem.cpp]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSettings()]] - code - Source/Maverick/System/MVFieldTransitionSettings.h
- [[UMVFieldTransitionSubsystemBeginLoadingReset()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemClearUIBeforeLoadingIfNeeded()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemCompleteTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemDeinitialize()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemFinishTransitionAfterScreenChange()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemGetUISubsystem()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemInitialize()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemResetUIToDefaultAfterTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemResolvePlayerCharacter()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemRestorePlayerInputAfterTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemResumePreparedPostTransitionAction()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemSetTransitionPhase()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemStartAutomaticLoadingCompletion()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemTryStartPreTransitionAction()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemUnbindPreTransitionAction()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemUpdateTransitionProgress()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVUISubsystem_2]] - code
- [[UWorld_12]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 6 edges to [[_COMMUNITY_필드 전환 요청 식별]]
- 5 edges to [[_COMMUNITY_필드 전환 요청 실행]]
- 2 edges to [[_COMMUNITY_캐릭터 도메인 연동]]
- 1 edge to [[_COMMUNITY_필드 전환 리셋 계약]]
- 1 edge to [[_COMMUNITY_필드 전환 사망 연동]]
- 1 edge to [[_COMMUNITY_월드 상태 서브시스템 접근]]
- 1 edge to [[_COMMUNITY_월드 상태 저장 레코드]]

## Top bridge nodes
- [[MVFieldTransitionSubsystem.cpp]] - degree 33, connects to 7 communities
- [[UMVFieldTransitionSubsystemResolvePlayerCharacter()]] - degree 3, connects to 1 community