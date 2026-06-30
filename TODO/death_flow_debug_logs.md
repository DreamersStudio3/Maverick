# Death Flow 테스트 로그

## 목표

lethal KnockDown/Airborne 피격에서 HP 0 확정, DeathComponent 보류, `MV HitReaction Death Handoff`, `MV HitReaction Start Getup` notify 순서를 PIE 로그로 확인한다.

## 작업

- [x] `UMVStatComponent` 피해 적용과 `OnDeathStarted` 발행 로그를 추가한다.
- [x] `UMVHitReactionComponent` lethal 예측, HitReaction 시작, StartGetup notify 진입 로그를 추가한다.
- [x] `UMVDeathComponent` death context 보류, handoff notify, action end fallback, death presentation 시작 로그를 추가한다.
- [x] PIE 피격 테스트로 Airborne 중 HP 0 사망을 재현하고 `[DeathFlowTest]` 로그 순서를 확인한다.

## 확인 결과

- 첫 Airborne 입력은 HP 100 -> 80이라 `Lethal=false`로 `HR_P1_AB_F_01`을 시작했다.
- 이후 같은 피격 테스트 입력이 StatComponent에 HP 피해를 누적해 HP 0에 도달했다.
- `OnDeathStarted`는 `HitReactionType=Airborne` 문맥으로 발행됐고, DeathComponent는 활성 `HR_P1/HR_P1_AB_F_01`을 정상 보류했다.
- `MV HitReaction Death Handoff` notify가 `HasDeferred=true`, active row 일치 상태로 들어왔고 `Death.HandoffAccepted` 뒤 `Death.BeginPresentation`까지 이어졌다.
- 이번 로그에는 `HitReaction.StartGetupNotify`가 나오지 않았다. 즉 handoff가 Getup보다 먼저 정상 소비됐다.

## 다음 확인

- 첫 타부터 lethal인 케이스는 HP를 20 이하로 낮춘 뒤 Airborne 버튼을 한 번만 눌러 별도로 확인한다.
- death action row 선택과 montage 시작 여부까지 보려면 `UMVDeathComponent::TryStartDeathAction` 성공 로그를 추가한다.
