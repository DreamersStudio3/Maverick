# Dodge Direction Follow-up

- [ ] Strafe Step 연속 입력 방향 오류 재현 및 원인 분리
  - 재현: `D`를 누른 채 Step을 밟고, 이어서 `W+D` 방향 Step을 시도하면 `S`/Back 방향 Step이 출력된다.
  - 가설: 다음 Step 방향 판정에 현재 입력이 아니라 이전 actor yaw, 이전 world movement direction, 또는 chooser용 locomotion snapshot이 섞이고 있다.
- [ ] ProjectBA 방식 재검토
  - 입력 원본은 controller-space raw 2D로 유지하고, world direction 변환은 controller yaw 기준으로만 수행해야 한다.
  - 직전 raw 2D 입력 경로 추가 시도는 증상을 해결하지 못했으므로 BP 입력 연결 여부, chooser 입력값, action buffer 소비 시점의 실제 값 로그를 함께 검증한다.
- [ ] Strafe Step chooser와 캐릭터 회전 정책 재검토
  - F/L/R/B는 actor yaw를 강제로 틀지 않는다.
  - FL/FR/BL/BR만 필요한 경우 F/B montage를 대각 yaw로 보정한다.
  - 방향 enum, chooser snapshot, launch direction이 같은 입력 의도를 공유하는지 확인한다.
- [ ] 경사면 전환 구간 Dodge launch 걸림 해결
  - 재현: 평지에서 slope로 진입하거나 slope 각도가 바뀌는 지점에서 Dodge launch가 벽에 걸린 것처럼 끊긴다.
  - 현재 `AddActorWorldOffset(..., sweep)` blocking hit를 launch 중단으로 처리하는 구조가 slope 경계의 walkable hit까지 막는지 확인한다.
  - CharacterMovement의 walking ground move/slide 처리와 유사하게 walkable surface에서는 이동을 이어가도록 설계한다.
- [ ] 수정 전후 PIE 로그 기준 추가
  - 입력 원본, controller yaw, actor yaw, chooser 방향, prepared launch direction, sweep hit normal/walkable 여부를 같은 프레임 기준으로 남긴다.
