# HitReaction / HitResolver 후속 작업

HitResolver 피격 계산 흐름이 들어간 뒤 이어서 정리할 작업 목록이다.

## HitReactionComponent

- [ ] `HitReactionComponent`를 추가하고 `CharacterBase.OnDamaged`에 `HandleDamaged`를 바인딩한다.
- [ ] `HitReactionType`을 `SmallHit`, `LargeHit`, `KnockDown`, `Airborne` 4종 기준으로 정리한다.
- [ ] 피격자의 캐릭터/액션 프로필과 `HitReactionType`, 피격 방향을 기준으로 피격 액션 데이터를 찾는 흐름을 만든다.
- [ ] 선택된 피격 액션의 테이블 이름, 액션 id, 섹션 이름을 `ActionComponent`에 전달해 몽타주를 재생한다.

## Collision / Notify 연계

- [ ] 공격 몽타주 NotifyState에서 충돌 컴포넌트 API를 호출하는 최소 흐름을 만든다.
- [ ] 충돌 컴포넌트가 공격자 `CharacterIndexId`, 액션 id, 이미 맞은 대상 목록을 기억해 중복 타격과 자기 자신 타격을 막는다.
- [ ] 유효 타격 후보만 `HitResolverSubsystem`에 전달한다.

## WeaponComponent 연계

- [ ] `WeaponComponent`가 추가되면 HitResolver의 `ResolveEquippedWeaponAttackPower`에서 현재 무기 스탯을 직접 조회한다.
- [ ] 무기 아이템이 없는 상태도 맨손 무기 기본 장착으로 처리한다.
