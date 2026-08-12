---
제목: Hit, Stat, HitReaction
부제목: 적중 계산·수치 피해·피격 표현의 책임 분리
최근수정일: 2026-08-12
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[Features/Combat/Combat-System/document|Maverick 전투 시스템]]"
---

# Hit, Stat, HitReaction

## 실행 흐름

```mermaid
flowchart TD
    HitSource["Collision / Ability / Blueprint"] --> Request["FMVHitResolveRequest"]
    Request --> Resolver["UMVHitResolverSubsystem"]
    Resolver --> Resolved["FMVResolvedHitData"]
    Resolved --> Character["AMVCharacterBase::OnHitResolved"]
    Character --> Damaged["OnDamaged"]
    Damaged --> Stat["StatComponent"]
    Damaged --> Reaction["HitReactionComponent"]
    Stat --> Death["OnDeathStarted"]
```

## 책임

| 계층 | 책임 |
|---|---|
| Collision·Ability·Blueprint | 필터링된 `FMVHitResolveRequest` 생성 |
| `UMVHitResolverSubsystem` | 공격자·피격자 Stat, 공격 배율, 무기 Snapshot 기반 `FMVResolvedHitData` 계산 |
| `AMVCharacterBase::OnHitResolved` | CharacterIndex 확인과 `OnDamaged` 방송 |
| StatComponent | 수치 피해, Groggy, Lethal Latch, `OnDeathStarted` 소유 |
| HitReactionComponent | Row·Section, Interrupt, 무적, KD·AB Recovery 표현 |

## 무적 적용 범위

- 무적 검사: HitReaction 경로
- Resolver·Stat 피해 적용: 무적 검사와 분리
