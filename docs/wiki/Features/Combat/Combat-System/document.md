---
제목: "Maverick 전투 시스템"
부제목: "현재 공격 실행 흐름과 공격·스킬 구성"
최근수정일: 2026-10-21
최근수정자: "Kwon-Nahoo-Man"
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[Features/Input-to-Action/document|입력에서 Action 실행까지]]"
  - "[[Features/Hit-Stat-HitReaction/document|Hit, Stat, HitReaction]]"
  - "[[Features/Combat/Skill/document|플레이어 스킬 구조]]"
  - "[[Features/Combat/Ranged-Projectile/document|원거리 발사체와 자동 유도 대상 선택]]"
  - "[[Features/Combat/Weapon-Swap/document|플레이어 무기 교체]]"
  - "[[Features/Combat/Box-Damage-Ability/document|박스 범위 피해 Ability]]"
---

# Maverick 전투 시스템

## 전투 흐름

```mermaid
flowchart TD
    Input["공격 입력"] --> Manager["UMVInputManagerComponent<br/>입력 배분"]
    Manager --> Combat["UMVCombatComponent<br/>공격 입력 규칙·런타임 상태"]
    Combat --> Weapon["UMVWeaponComponent<br/>현재 스타일 공격·장착 Skill Row"]
    Weapon --> Gate{"액션 시작 가능"}
    Gate -->|아니오| Buffer["입력 버퍼 유지"]
    Gate -->|예| Action["UMVActionComponent<br/>Montage 실행"]
    Action --> Notify["Ability Notify<br/>비용 차감·공격 구간 활성화"]
    Notify --> Hit["Ability 구현·충돌 판정<br/>Hit Resolve 요청"]
    Hit --> Resolver["UMVHitResolverSubsystem<br/>피해·그로기·피격 결과 계산"]
    Resolver --> Damaged["AMVCharacterBase::OnHitResolved<br/>OnDamaged 브로드캐스트"]
    Damaged --> Stat["UMVStatComponent<br/>HP·Groggy·Death"]
    Damaged --> Reaction["UMVHitReactionComponent<br/>피격 표현·행동 취소"]
    Action --> End["Montage 종료<br/>체인·쿨다운 갱신"]
```

## 공격 종류

무기 교체·스타일별 공격 선택·듀얼 소드 손별 판정: [[Features/Combat/Weapon-Swap/document|플레이어 무기 교체]]

| 분류 | 현재 동작 |
|---|---|
| 약공격 | 연속 Row 기반 기본 연계 |
| 강공격 | 연속 Row 기반 강공격과 누르기 유지 조기 해제 분기 |
| 차지공격 | 누르기 유지 후 확정 시간 도달 시 실행 |
| 전력질주 공격 | 전력질주 문맥에서 약·강 입력을 전용 Row로 치환 |
| 회피 공격 | 회피 문맥에서 약·강 입력을 전용 Row로 치환 |

## 스킬 시스템

- 입력 인덱스: Q `Skill0`, R `Skill1`, W `Skill2`, E `Skill3`
- 런타임 맵 키: Q `QSkill`, R `RSkill`, W `WSkill`, E `ESkill`
- 조합 입력: Shift+좌클릭 W, Shift+우클릭 E
- 현재 무기 스타일과 슬롯을 기준으로 WeaponComponent의 장착 Row 조회
- 미등록·미지원 스킬 입력 즉시 소비로 Action 종료·무기 교체 후 지연 실행 차단
- `NextChainName`으로 단일 스킬과 연속 스킬 구성
- Chain 진행은 DataTable Row `FName`과 현재 단계 인덱스로 관리하며 런타임 맵 키와 분리
- `ChainAdvancePolicy`: Row별 `Immediate`·`OnHitConfirmed` 단계 전진 정책
- 주 쿨다운·단계 간격·입력 창으로 연속 단계 제어
- Ability Notify 최초 시작 시 비용 차감, 종료 시 체인·쿨다운 갱신
- HUD에 사용 가능 상태·연속 단계·입력 창·남은 쿨다운 투영
- Q 적중 연계·R 게이지·W/E 조합 입력 계약: [[Features/Combat/Skill/document|플레이어 스킬 구조]]

## Passive 스킬 생명주기

```mermaid
flowchart LR
    Request["Passive Row 적용 요청"] --> Clear["기존 Passive 종료·슬롯 비우기"]
    Clear --> Validate{"Row·Ability·비용·체인 검증"}
    Validate -->|실패| Vacant["Passive 공석"]
    Validate -->|성공| Create["UMVAbilityBase 생성·초기화"]
    Create --> Start["Execute_StartAbility"]
    Start -->|활성 실패| Vacant
    Start -->|활성 성공| Active["활성 Row·인스턴스 보관"]
    Active --> End["명시적 해제 또는 EndPlay"]
    End --> Clear
```

- `UMVCombatComponent`가 QWER 실행 상태와 분리된 Passive 단일 슬롯 소유
- 기존 `FMVSkillDataTableColumn`과 `UMVAbilityBase` 재사용
- Passive Row의 HP·Stamina·MP 비용과 `NextChainName` 기반 체인 사용 금지
- 동일 Row 재적용은 현재 인스턴스를 유지하고 성공 반환
- 교체 실패 시 이전 Passive 복구 없이 공석 유지
- 사망 시 유지, 현재 스타일의 Passive 장착 변경 시 해당 슬롯만 다시 적용
