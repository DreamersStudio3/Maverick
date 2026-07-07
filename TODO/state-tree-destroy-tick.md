# StateTree Tick Destroy 오류

## 목표

- Enemy Dead StateTree task에서 actor destroy가 StateTree 실행 중 직접 호출되어 발생하는 ensure를 방지한다.

## 진행

- [x] `POLICY.md` 확인
- [x] StateTree task와 `Destroy()` 호출 경로 확인
- [x] `MVEnemyDeadTask`의 `DestroyActor` cleanup을 다음 tick으로 지연
- [ ] DebugGame 링크 검증

## 원인

- `MVEnemyDeadTask::EnterState()`가 `Tick(Context, 0.0f)`를 직접 호출한다.
- Death presentation이 이미 끝났고 cleanup delay가 0이면, StateTree Enter/Tick 실행 중 `Owner->Destroy()`가 호출됐다.
- Pawn destroy 과정에서 controller unpossess가 발생하고, AIController의 StateTreeComponent가 `StopLogic/Cleanup`을 호출하면서 StateTree tick 중 StateTree context requirement를 다시 설정하려 해 ensure가 발생했다.

## 수정 내용

- `DestroyActor` cleanup에서 `Owner->Destroy()`를 직접 호출하지 않는다.
- `SetTimerForNextTick`으로 destroy를 예약한다.
- cleanup 예약 후 task는 `Succeeded`가 아니라 `Running`을 유지한다.
- `bCleanupApplied`로 destroy/deactivate cleanup 중복 실행을 막는다.

## 검증 상태

- `MaverickEditor Win64 DebugGame` 빌드에서 compile 단계는 통과했다.
- 링크 단계는 실행 중인 `UnrealEditor-Win64-DebugGame.exe`와 `CrashReportClientEditor.exe`가 DLL을 잡고 있어 실패했다.
- 에디터와 CrashReportClient를 닫은 뒤 DebugGame 링크 검증이 필요하다.
