# HitReaction Ability Launch 흐름 전환

목표는 HitReaction DataTable이 피격 모션 선택과 Launch 사용 여부만 갖고, 실제 Launch 세기는 공격 Ability가 결정하게 만드는 것이다.

## 원하는 런타임 흐름

1. 공격 Ability row가 `DamageMultiplier`, `GroggyDamageMultiplier`, `HitReactionType`을 제공한다.
2. 공격 Ability가 기본값 또는 Blueprint override로 Launch 값을 결정한다.
3. 공격 판정 또는 Ability Blueprint가 `FMVHitResolveRequest`를 만들 때 Ability의 Launch 값을 함께 담는다.
4. `UMVHitResolverSubsystem`은 request의 Launch 값을 `FMVResolvedHitData`로 복사한다.
5. 피격자는 기존처럼 `OnHitResolved -> OnDamaged -> UMVHitReactionComponent.HandleDamaged` 흐름을 탄다.
6. `UMVHitReactionComponent`는 HitReaction row의 `bUseLaunch`가 true일 때만 `HitData`의 Launch 값으로 `LaunchCharacter`를 호출한다.

## 코드 작업

- [x] Launch 수치 전용 struct를 만든다.
  - 후보: `FMVHitLaunchData` 또는 `FMVAbilityLaunchData`
  - 필드: `LaunchDistance`, `LaunchDuration`, `LaunchVerticalSpeed`
- [x] `UMVAbilityBase`가 Ability별 Launch 값을 기본 property와 Blueprint override로 제공한다.
- [x] `FMVSkillDataTableColumn`에서 Launch 값을 제거해 Ability row가 Launch 책임을 갖지 않게 한다.
- [x] `FMVHitResolveRequest`와 `FMVResolvedHitData`에 Launch 값을 추가한다.
- [x] `UMVHitResolverSubsystem::BuildResolvedHitData`에서 request Launch 값을 resolved data로 넘긴다.
- [x] Ability Blueprint가 `FMVHitResolveRequest`에 현재 Ability의 Launch 값을 주입할 수 있는 헬퍼를 추가한다.
- [x] `FMVHitReactionActionRow`는 `bUseLaunch`만 남기고 Launch 수치 필드는 제거한다.
- [x] `UMVHitReactionComponent::ApplyHitReactionLaunch`가 row 수치 대신 `HitData`의 Launch 값을 사용하게 바꾼다.
- [x] PIE 디버그 피격 위젯도 기본 Launch 값을 넣거나 Launch 없음 기본값이 안전한지 확인한다.
- [x] HitReaction fallback RowName 생성 규칙을 `Flinch_F`, `Getup_B`, `EscapeDodge_R`처럼 간소화된 DT row 이름에 맞춘다.

## 데이터/에셋 작업

- [x] 서 있는 피격 네이밍을 `Flinch/Stagger/Knockback`으로 확정한다.
- [ ] Flinch row는 현재 `AS_HR_Weak_방향` 에셋을 쓰도록 Player HitReaction DataTable/Chooser를 갱신한다.
- [ ] Stagger row는 기존 중간 경직급 반응을 이동해 사용한다.
- [ ] Knockback row는 범위 공격처럼 크게 밀려나는 반응으로 두고 HitReaction row에서 `bUseLaunch=true`가 되도록 설정한다.
- [ ] 기존 HitReaction DataTable에 박힌 `LaunchDistance`, `LaunchDuration`, `LaunchVerticalSpeed` 값을 공격 Ability 기본값 또는 Blueprint 계산으로 이관한다.
- [ ] Ability Blueprint 또는 공격 판정 노드가 Ability의 Launch 값을 `FMVHitResolveRequest`에 넣도록 연결한다.

## 검증

- [ ] Flinch: `AS_HR_Weak_방향` 모션이 재생되고 Ability Launch 값으로 직접 밀린다.
- [ ] Stagger: 기존 중간 경직급 반응으로 이동했고 Ability Launch 값으로 밀린다.
- [ ] Knockback: 크게 밀리는 반응이 재생되고 Ability Launch 값으로 밀린다.
- [ ] Launch 미사용 HitReaction row는 Ability에 Launch 값이 있어도 밀리지 않는다.
- [ ] Groggy로 HitReactionType이 바뀌는 경우에도 Launch 사용 여부와 수치 적용이 의도대로인지 확인한다.
- [ ] KD/Airborne/death handoff 기존 흐름이 깨지지 않는지 PIE로 확인한다.
- [ ] Airborne 미재생 재현 시 `AirborneTrace` 임시 로그로 Victim 전달, OnHitResolved, HitReactionComponent return 지점을 확인한다.
- [ ] Ability Launch 입력 단위가 cm/cm/s로 고정 표시되는지 확인하고, 기존 BP에 저장된 과대 VerticalSpeed 값을 600~900cm/s대로 정리한다.
- [x] KnockDown/Airborne row 방향은 공격이 들어온 방향이 아니라 피격자가 밀려나거나 쓰러지는 힘 방향 기준으로 판정하게 분리한다.
- [ ] KnockDown 재현 시 `HitDirectionTrace` 임시 로그로 `Mode=ForceDirection`, `ForwardDot`, 선택 row가 의도한 앞/뒤 방향인지 확인한다.
- [x] EscapeDodge row 이름 생성이 실제 `DT_HR_Player`의 `EscapeDodge_F_R` 같은 2축 row와 맞지 않아 시작되지 않던 원인을 확인하고 복구한다.
- [ ] EscapeDodge 재현 시 `RecoveryTrace` 임시 로그로 `RequestedRow=EscapeDodge_<fall>_<escape>`, `EscapeDodgeStarted` 여부를 확인한다.
- [x] EscapeDodge recovery가 최신 `DT_HR_Player` 대신 stale manifest 이름 `HR_P1`로 테이블을 찾던 문제를 active HitReaction DataTable 재사용으로 우회한다.
- [ ] KnockDown Launch 재현 시 Ability 쪽 `AbilityDefault/AbilityApplyToRequest` 로그가 찍히는지 확인한다. 현재 로그에서는 `HitReactionType=4` request에 Launch 값이 모두 0으로 들어온다.
- [ ] Stagger 재현 시 `HitDirectionTrace`의 `ResolveDirection`, `BeforeActionStart`, `AfterActionStart` Forward/Rotation을 비교해 방향 계산 전후 회전 여부를 확인한다.
- [x] 수직 Launch도 `LaunchDuration` 뒤 상승 속도를 0으로 잘라 낙하로 전환하게 제어 타이머를 추가한다.
- [ ] Airborne 재현 시 `ReactionLaunchVerticalTimerSet` 이후 `ReactionLaunchVerticalTimerElapsed`에서 Z 속도가 0으로 잘리는지 확인한다.

## 주의점

- HitReaction row struct를 바꾸면 기존 uasset DataTable 컬럼이 사라지므로 에디터에서 데이터 손실/리로드 상태를 확인한다.
- 실제 공격 request 생성 경로가 C++에 거의 없고 Blueprint/Ability 쪽일 가능성이 높으므로, C++ 계약 변경 뒤 BP 연결 확인이 필수다.
- `HitReactionComponent`의 기존 direction 계산은 공격자에서 피격자로 향하는 힘 방향을 Launch에 그대로 쓰고 있으므로 유지한다.
