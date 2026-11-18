---
제목: "그로기 Finisher"
부제목: "Attacker·Victim 짝 애니메이션 선택과 실행 책임"
최근수정일: 2026-11-18
최근수정자: "Kwon-Nahoo-Man"
관련문서:
  - "[[Architecture/document|Maverick 아키텍처]]"
  - "[[Features/Combat/Combat-System/document|Maverick 전투 시스템]]"
  - "[[Features/Input-to-Action/document|입력에서 Action 실행까지]]"
  - "[[Features/Hit-Stat-HitReaction/document|Hit, Stat, HitReaction]]"
---

# 그로기 Finisher

## 핵심

- `UMVFinisherComponent`: 그로기 대상 탐색과 Attacker·Victim 짝 애니메이션 실행 요청 담당
- `BeginPlay`에서 약공격 입력 처리기로 등록, `EndPlay`에서 해제

## 실행 흐름

```mermaid
flowchart TD
    Input["약공격 입력"] --> Try["TryFinisherMove"]
    Try --> Detect["전방 Sphere Trace<br/>그로기·거리·대상 정면 각도 검사"]
    Detect --> Clear["기존 공격 데이터·Ability·대상 정리"]
    Clear --> Choose["현재 장착 스타일·피격자 태그로<br/>양측 Chooser 평가"]
    Choose --> Warp["Attacker Warp 목표 설정<br/>대상 정면 130, 대상과 마주보는 회전"]
    Warp --> Play{"Attacker → Victim 순서로<br/>Action 시작 성공?"}
    Play -->|양측 성공| Ability["Attacker Row로 Ability 생성·초기화"]
    Play -->|실패| Rollback["시작한 Attacker 취소<br/>기존 Groggy 보존 또는 복구 시도"]
    Ability --> Target["대상 저장"]
    Target --> Groggy["EnemyDodgeToken에 종료 통지<br/>대상 그로기 수치 0 설정"]
```

## 기준

- 탐색: 전방 200·반경 50의 단일 Sphere Trace, `ECC_GameTraceChannel1` 사용
- 조건은 생존 중인 `IsGroggy()` 대상, 2D 거리 `DetectDistance` 이내, 대상 정면 기준 `DetectAngle` 이내이며 기본값은 200·45도
- Chooser: 양측에 같은 `WeaponType`·`VictimTag` 입력 사용, 출력 Row 우선·Row 이름 누락 또는 DataTable 반환 시 `FallBack` 사용
- 데이터: Attacker는 `FMVSkillDataTableColumn`, Victim은 `FMVActionRow`; 실제 애니메이션 실행은 각 `UMVActionComponent` 담당

## Victim 인계

- Finisher는 선택한 Victim Row의 교체를 대상 ActionComponent에 요청
- ActionComponent는 기존 실행의 이동·회전 제한을 성공한 Victim 실행에 인계하고, 이전 실행 ID·섹션과 교체 결과를 방송
- HitReaction은 Action의 교체 결과에서 자신의 Groggy 실행을 확인해 성공 시 기록 정리, 실패로 기존 실행이 중단됐을 때 같은 Row·섹션 복구 시도
- Victim 실행은 Stat의 그로기 해제 뒤에도 유지되며, 인계된 제한은 Victim 실행 종료 시 해제
- 양측 시작이 확인되면 공격 데이터·대상을 기록하고 그로기 게이지를 0으로 설정

## 구현 근거

[MVFinisherComponent.h](../../../../../Source/Maverick/Components/MVFinisherComponent.h) · [MVFinisherComponent.cpp](../../../../../Source/Maverick/Components/MVFinisherComponent.cpp)

현재 C++ 구현 기준 문서이며 Chooser·Montage 에셋 연결과 실제 재생 타이밍은 Editor 검증 범위
