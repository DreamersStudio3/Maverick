---
type: community
cohesion: 0.16
members: 20
---

# 이동 차단과 회복 탈출

**Cohesion:** 0.16 - loosely connected
**Members:** 20 nodes

## Members
- [[FAnimNotifyEventReference_8]] - code
- [[FAnimNotifyEventReference_11]] - code
- [[FString_10]] - code
- [[FString_13]] - code
- [[FindBlockMovementInputInputManagerComponent()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.cpp
- [[FindRecoveryEscapeInputManagerComponent()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_RecoveryEscapeWindow.cpp
- [[MVAnimNotifyState_BlockMovementInput.cpp]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.cpp
- [[MVAnimNotifyState_RecoveryEscapeWindow.cpp]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_RecoveryEscapeWindow.cpp
- [[UAnimSequenceBase_8]] - code
- [[UAnimSequenceBase_11]] - code
- [[UMVAnimNotifyState_BlockMovementInputGetNotifyName_Implementation()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.cpp
- [[UMVAnimNotifyState_BlockMovementInputNotifyBegin()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.cpp
- [[UMVAnimNotifyState_BlockMovementInputNotifyEnd()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.cpp
- [[UMVAnimNotifyState_RecoveryEscapeWindowGetNotifyName_Implementation()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_RecoveryEscapeWindow.cpp
- [[UMVAnimNotifyState_RecoveryEscapeWindowNotifyBegin()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_RecoveryEscapeWindow.cpp
- [[UMVAnimNotifyState_RecoveryEscapeWindowNotifyEnd()]] - code - Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_RecoveryEscapeWindow.cpp
- [[UMVInputManagerComponent()]] - code - Source/Maverick/Components/MVInputManagerComponent.h
- [[UMVPlayerDodgeCanTransitionCurrentAction()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[USkeletalMeshComponent_8]] - code
- [[USkeletalMeshComponent_11]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 1 edge to [[_COMMUNITY_AI 공격 실행 로직]]
- 1 edge to [[_COMMUNITY_피격 회복 방향 추적]]
- 1 edge to [[_COMMUNITY_플레이어 회피 입력 계약]]

## Top bridge nodes
- [[UMVInputManagerComponent()]] - degree 5, connects to 2 communities
- [[UMVPlayerDodgeCanTransitionCurrentAction()]] - degree 3, connects to 2 communities