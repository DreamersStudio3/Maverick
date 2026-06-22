# Dodge Direction Follow-up

- [x] Strafe Step 연속 입력 방향 오류 재현 및 원인 분리
  - 재현: `D`를 누른 채 Step을 밟고, 이어서 `W+D` 방향 Step을 시도하면 `S`/Back 방향 Step이 출력된다.
  - 원인 분리: Dodge buffer/prepare 경로가 world movement direction과 actor yaw 기반 snapshot에 의존해, 연속 Step 중 이전 yaw/이전 movement snapshot이 다음 방향 판정에 섞일 수 있었다.
  - 처리: 이동 입력을 `AMVCharacterBase`에서 controller-space raw 2D로 같은 프레임 누적하고, `UMVDodgeComponent`가 buffered action 소비 시 이 raw 2D를 기준으로 방향을 확정하게 변경했다.

- [x] ProjectBA 방식 재검토
  - 입력 원본은 controller-space raw 2D로 유지하고, world direction 변환은 controller yaw 기준으로만 수행한다.
  - 처리: `AddMovementInput` 호출별 world direction을 controller yaw 기준 raw 2D로 역변환해 프레임 단위로 합산한다.
  - 처리: Action buffer에는 Dodge 전용 raw 2D를 `FVector(X,Y,0)` 형태로 저장하고, prepare/launch 직전에 controller yaw 기준 world direction으로 변환한다.

- [x] Strafe Step chooser와 캐릭터 회전 정책 재검토
  - F/L/R/B는 actor yaw를 강제로 틀지 않는다.
  - FL/FR/BL/BR은 F/B chooser snapshot을 유지하되, 필요한 경우에만 diagonal launch/facing yaw를 준비한다.
  - 처리: strafe Step prepare 시 actor yaw를 controller yaw로 선회시키던 호출을 제거했다.
  - 처리: 방향 enum, chooser snapshot, prepared launch direction이 같은 raw 2D 입력 의도에서 파생되도록 정리했다.
  - 처리: FL/FR/BL/BR 입력은 F/B 몽타주를 고르더라도 몽타주 재생 전에 actor yaw를 해당 대각 launch 방향으로 보정한다.
  - 처리: BL/BR은 B 몽타주의 backward 이동 기준에 맞춰 actor yaw를 launch 방향의 반대로 보정한다.

- [x] 경사면 전환 구간 Dodge launch 걸림 해결
  - 재현: 평지에서 slope로 진입하거나 slope 각도가 바뀌는 지점에서 Dodge launch가 벽에 걸린 것처럼 끊긴다.
  - 원인 분리: `AddActorWorldOffset(..., sweep)` blocking hit를 곧바로 launch 중단으로 처리해 slope 경계의 walkable hit도 중단 조건이 될 수 있었다.
  - 처리: launch 이동을 직접 actor sweep 대신 `CharacterMovementComponent::SafeMoveUpdatedComponent`, `StepUp`, `SlideAlongSurface` 경로로 넘긴다.
  - 처리: 계단/낮은 턱처럼 `CanStepUp` 가능한 hit는 walking과 같은 `StepUp`을 시도하고, walkable ramp hit는 walking용 slide 처리를 사용한다.

- [x] 이동 입력 해제 후 Dodge buffer 방향 재사용 방지
  - 재현: 이동 입력이 있는 상태에서 Roll/Step을 실행한 뒤 이동 입력을 놓고 Dodge를 다시 시도하면 이전 Dodge 방향이 재사용된다.
  - 기대: 이동 입력 없이 Dodge를 시도하면 이전 방향과 무관하게 Backstep이 나간다.
  - 처리: Dodge 중 no-input buffered Dodge를 거절하던 조건을 제거해 Roll/Step 이후 Backstep 전환도 buffer 소비를 허용한다.

- [x] Backstep 이후 buffered Roll 런치 방향 불일치 해결
  - 재현: Roll -> Backstep -> 앞 입력 Roll을 연속으로 시도하면 앞 Roll 몽타주가 재생되지만 launch가 뒤로 적용된다.
  - 원인 분리: buffered Dodge 시작 직후 fallback launch가 prepared launch 값을 소비해 지우면, 이후 NotifyState Begin에서 이동 차단 중인 캐릭터 상태를 다시 읽어 Backstep 방향으로 재추론할 수 있었다.
  - 처리: fallback launch에서는 prepared launch 값을 보존하고, 정식 NotifyState Begin에서 같은 준비값으로 launch를 초기화한 뒤 정리한다.

- [x] Dodge launch distance 400 조정
  - 처리: `ActionStat` Dodge row의 `LaunchDistance`를 600에서 400으로 낮추고 데이터테이블을 재생성한다.

- [x] PIE 수동 확인 기준 정리
  - 입력 방향, chooser 방향, launch 방향이 같은 플레이어 입력 의도에서 파생되는지 실제 몽타주와 이동 방향으로 확인한다.
  - slope 전환 구간에서는 launch가 walkable ramp나 낮은 턱에서 끊기지 않는지 확인한다.

## 검증

- [x] 방향 입력 수정 후 `Build.bat MaverickEditor Win64 Development -Project=C:\Workspace\Maverick\Maverick.uproject -WaitMutex -NoHotReload` 성공
- [x] movement helper 추가 후 `git diff --check -- Source\Maverick\Components\MVDodgeComponent.cpp TODO\dodge_direction_followup.md` 성공
- [x] movement helper 접근 제어 수정 후 `Build.bat MaverickEditor Win64 Development -Project=C:\Workspace\Maverick\Maverick.uproject -WaitMutex -NoHotReload` 성공
- [x] diagonal F/B montage yaw 보정 후 `Build.bat MaverickEditor Win64 Development -Project=C:\Workspace\Maverick\Maverick.uproject -WaitMutex -NoHotReload` 성공
- [x] 이동 입력 해제 후 Dodge cache frame 제한 수정 후 `Build.bat MaverickEditor Win64 Development -Project=C:\Workspace\Maverick\Maverick.uproject -WaitMutex -FromMsBuild` 성공
- [x] Dodge 중 no-input buffered Dodge 소비 허용 후 빌드 확인
- [x] Backstep 이후 buffered Roll launch prepared 보존 수정 후 빌드 확인
- [x] `MV.Table.GenerateDataTables`로 `DT_ActionStat` 재생성 확인
- [x] 임시 디버그 로그 제거 후 `Build.bat MaverickEditor Win64 Development -Project=C:\Workspace\Maverick\Maverick.uproject -WaitMutex -FromMsBuild` 성공

PIE 수동 확인 기준:
- `D` Step 이후 `W+D` buffered Step이 Back이 아닌 diagonal/front-right 의도로 기록되는지 확인한다.
- slope 전환 구간에서 launch가 끊기지 않고 자연스럽게 이어지는지 확인한다.
- `Roll -> Backstep -> W Roll`에서 앞 Roll 몽타주와 launch 방향이 모두 전방으로 일치하는지 확인한다.

## 향후 Root Motion 전환 메모

- 8방향 Dodge/Roll 애니메이션을 root motion으로 재제작하면 현재 curve 기반 수동 launch 이동은 제거하거나 보조 용도로 축소한다.
- root motion Dodge도 기본적으로 `MOVE_Walking`을 유지해 walkable floor, step-up, slope 처리를 CharacterMovement에 맡긴다.
- 별도 `CustomMovementMode`는 Dodge가 walking과 다른 충돌/공중/네트워크 예측 규칙을 가져야 할 때만 다시 검토한다.
