---
제목: "Maverick 전투 시스템"
부제목: "현재 공격 실행 흐름과 공격·스킬 구성"
최근수정일: 2026-08-12
최근수정자: "곽민규"
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[Features/Input-to-Action/document|입력에서 Action 실행까지]]"
  - "[[Features/Hit-Stat-HitReaction/document|Hit, Stat, HitReaction]]"
---

# Maverick 전투 시스템

## 전투 흐름

```mermaid
flowchart TD
    Input["공격 입력"] --> Manager["UMVInputManagerComponent<br/>입력 배분"]
    Manager --> Combat["UMVCombatComponent<br/>공격 유형·행동 Row 선택"]
    Combat --> Gate{"액션 시작 가능"}
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

| 분류 | 현재 동작 |
|---|---|
| 약공격 | 연속 Row 기반 기본 연계 |
| 강공격 | 연속 Row 기반 강공격과 누르기 유지 조기 해제 분기 |
| 차지공격 | 누르기 유지 후 확정 시간 도달 시 실행 |
| 전력질주 공격 | 전력질주 문맥에서 약·강 입력을 전용 Row로 치환 |
| 회피 공격 | 회피 문맥에서 약·강 입력을 전용 Row로 치환 |

## 스킬 시스템

- `Q`는 `Skill0`, `R`은 `Skill1`로 처리
- 현재 무기와 입력 태그를 기준으로 Chooser 또는 DataTable Row 선택
- `NextChainName`으로 단일 스킬과 연속 스킬 구성
- 주 쿨다운·단계 간격·입력 창으로 연속 단계 제어
- Ability Notify 최초 시작 시 비용 차감, 종료 시 체인·쿨다운 갱신
- HUD에 사용 가능 상태·연속 단계·입력 창·남은 쿨다운 투영
