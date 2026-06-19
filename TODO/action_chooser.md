# Action Chooser TODO

액션 Chooser Table 구성에 필요한 C++ 원천 조건 노출 작업을 관리한다.

- [x] 캐릭터에 Chooser 조건으로 사용할 `EquippedStyle` 상태를 추가한다.
- [x] Dodge 액션 시작 시 `LaunchCharacter`로 이동하도록 Dodge 전용 Launch 함수를 추가한다.
- [x] Launch 거리/시간/수직 속도를 ActionStat 컬럼으로 이동한다.
- [x] Launch 거리/시간 기반 계산과 Backstep 전용 Launch 비율 컬럼을 제거한다.
- [x] Dodge Launch 기본 거리를 400cm로 적용하고 Backstep은 코드에서 기본 거리의 50%로 처리한다.
- [x] Dodge 자동 선형 보정 launch를 제거하고 `MV Dodge Launch` NotifyState 구간에서만 이동하도록 변경한다.
- [x] Dodge LaunchDuration이 0이면 `MV Dodge Launch` NotifyState 구간 길이에 맞춰 거리 커브를 재생한다.
- [x] Dodge 이동 처리를 `MVDodgeComponent`로 분리하고 `ActionComponent`는 액션 실행/테이블/몽타주/입력버퍼만 담당하게 한다.
- [x] 무적과 이동입력 차단 상태를 `AMVCharacterBase`로 이동해 Damage/Movement 시스템이 직접 조회할 수 있게 한다.
- [x] ActionStat에 조회용 `ActionName` 컬럼을 추가한다.
- [x] Dodge ActionStat의 root motion 사용 의도를 false로 변경한다.
- [x] Dodge 기본 이동 거리를 600cm로 조정한다.
- [x] Dodge 시작 시 Chooser 방향과 실제 Launch 방향이 같은 기준을 쓰도록 방향 스냅샷을 고정한다.
- [ ] Dodge Chooser 루트에서 이동 입력/스트레이프 여부로 Backstep, Roll, Step 하위 Chooser를 나누는 구성을 에디터에서 확인한다.
- [ ] 하위 Dodge Chooser에서 `EquippedStyle`과 `LocomotionDirection` 조합으로 몽타주를 선택하는 구성을 에디터에서 확인한다.
- [x] 몽타주 구간 제어용 NotifyState를 추가해 입력 버퍼 구간, 이동입력 불가 구간, 무적 구간을 액션 애니메이션별로 배치할 수 있게 한다.
- [ ] Dodge 몽타주별 Notify Track에 `MV Dodge Launch`, 이동입력 불가, 무적, 입력 버퍼 NotifyState 구간을 배치한다.
