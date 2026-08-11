---
제목: Hit, Stat, HitReaction
부제목: 적중 계산·수치 피해·피격 표현의 책임 분리
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[Research/Combat-Design-MDA/document|Maverick 전투 MDA와 목표 전투 계약]]"
---
# Hit, Stat, HitReaction

1. Collision/Ability/Blueprint 계층이 필터링된 `FMVHitResolveRequest`를 `UMVHitResolverSubsystem`에 전달한다. production 호출부는 `에셋 확인 필요`.
2. Resolver가 공격자·피격자 스탯, 공격 배율, 무기 snapshot으로 `FMVResolvedHitData`를 만든다.
3. Resolver가 피해자 `AMVCharacterBase::OnHitResolved`를 호출한다.
4. CharacterBase가 CharacterIndex를 확인하고 `OnDamaged`를 broadcast한다.
5. StatComponent는 수치 피해, groggy, lethal latch와 `OnDeathStarted`를 소유한다.
6. HitReactionComponent는 row/section, interrupt, 무적, KD/AB recovery 표현만 소유한다.

현재 native 코드 기준 무적 검사는 HitReaction 경로에 있고 Resolver/Stat의 피해 적용 경로에는 없다. 따라서 무적이 피해 자체를 막는다는 전제는 별도 검증 없이 사용하지 않는다.
