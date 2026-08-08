# Maverick 전투 MDA와 목표 전투 계약

> 상태: 팀 검토용 압축 위키
> 분석 기준: 2026-07-24 작업 트리의 C++, DataTable, Montage, Blueprint 기본값과 플레이테스트 관찰
> 목적: 현재 전투의 Mechanics, 그로 인해 생기는 Dynamics, 목표 Aesthetics를 연결하고 구현 우선순위와 검증 계약을 보존한다.

## 1. 문서 사용법과 증거 범위

이 문서는 현재 구현과 목표 설계를 섞지 않는다.

- **[현재 구현]** C++ 또는 읽어 낸 DataTable과 Montage 값으로 확인한 동작이다.
- **[에셋 확인]** Blueprint나 StateTree의 기본값과 참조를 확인했지만 모든 런타임 분기를 보장하지 않는다.
- **[관찰]** 플레이테스트에서 직접 확인한 현상이다.
- **[설계 목표]** 팀이 채택한 방향이다. 세부 수치와 API는 아직 구현 계약이 아니다.
- **[우선안]** 수직 검증을 위한 첫 설계 가설이다.
- **[검증 필요]** PIE 로그, 자동화 테스트 또는 에디터 에셋 확인이 없으면 사실로 확정하지 않는다.

바이너리 에셋과 수치는 분석 당시 snapshot을 기준으로 한다. 에셋이 바뀌면 이 문서의 수치를 고정된 현재값으로 간주하지 말고 관련 DataTable, Montage, Blueprint를 다시 확인한다. Graphify는 파일과 선언의 관계를 좁히는 지도이며 Blueprint, StateTree, Montage 내부 연결과 런타임 순서의 최종 증거가 아니다.

## 2. 설계 북극성과 MDA 판단 기준

> **Maverick의 전투는 읽을 수 있는 적의 공격과 명확한 아머 규칙 위에서, 간결한 기본공격으로 흐름을 유지하고 무기별 스킬 조합으로 주도권을 만들고 결산하는 공정하고 묵직한 액션 전투다.**

MDA는 다음 순서로 검토한다.

- **Mechanics**: 입력, 상태, 수치, 판정, 자원, 전이 조건처럼 게임이 허용하고 강제하는 규칙
- **Dynamics**: 그 규칙을 반복해서 사용할 때 플레이어와 적에게 생기는 우세 행동과 전투 리듬
- **Aesthetics**: 공정함, 타격 신뢰, 긴장, 숙련, 무기 판타지, 자기표현처럼 플레이어가 느껴야 하는 경험

모든 규칙은 “반복 사용 시 어떤 행동이 가장 합리적으로 창발하며, 그것이 목표 감각을 만드는가?”로 평가한다. 기능 수나 공격 이름이 많아도 해결하는 문제가 겹치면 실질 선택지는 적다.

목표 감각은 다음 여섯 가지다.

1. 적의 전조와 결과를 이해할 수 있는 공정하고 긴장된 무기 전투
2. 피해, 아머, 피격 모션, 행동 취소가 같은 사건을 말하는 타격 신뢰
3. 정확한 적중과 대응으로 다음 수를 여는 숙련의 리듬
4. 무기와 장착 스킬의 sidegrade 조합을 통한 자기표현
5. 거리, 속도, 범위, Impact가 만드는 무기별 판타지
6. 실패 원인을 이해하고 짧게 재도전하는 학습 구조

## 3. 현재 전투 기준선

### 3.1 현재 플레이 루프

**[현재 구현]**

현재 코드가 직접 지지하는 반복은 다음과 같다.

이동과 탐색 → 대상 인지와 락온 → 접근, 회피, 공격 교환 → HP 또는 그로기 결산 → 피니셔 또는 사망 → 전투 상태 리셋과 재도전

장기 성장, 장비 경제, 퀘스트 보상, 스킬 해금은 아직 이 루프와 완전히 연결되지 않았다. 현재 Maverick은 탐험형 RPG 전체보다 한 번의 근접 전투를 성립시키는 전투 샌드박스에 가깝다.

현재 공격 입력은 약공격 5단, 하나의 Hold에서 갈리는 강공격과 차지공격, 회피와 전력질주 파생 공격, Q 3단, R 단발로 구성된다. 입력 버퍼는 최신 액션 입력 하나를 최대 60프레임 보관하고 새 입력이 이전 입력을 덮는다. Recovery 창과 현재 액션의 interruptible 상태가 공격, 회피, 회복약 전환을 좌우한다.

적 AI는 거리, 각도, LOS, 공격 범위, 쿨다운과 현재 액션을 StateTree 판단에 사용하며 Heavy, Charge, Skill 위협에 대응할 회피 토큰 기반도 가진다. 다만 현재 StateTree 자산의 Task 구성은 과거 설계 설명에서 달라졌을 수 있으므로 실행 순서는 에디터 자산과 PIE 로그로 다시 확인한다. 피격 한 번이 선택된 공격을 취소할 수 있는 현재 계약에서는 AI 판단을 정교하게 만들어도 화면에는 행동을 시작하다 계속 끊기는 적만 보일 수 있다.

사망 후에는 오버레이와 로딩 전환을 거쳐 마지막 체크포인트로 이동하고 HP, 스태미나, MP, 그로기와 회복약을 복구하며 현재 월드의 적을 리셋한다. 현재 전환은 사망 표현과 오버레이의 이중 완료보다 오버레이 완료에 주로 의존한다. 네이티브 reset 대상은 E1 중심이고 이미 Destroy된 적의 재생성, 적의 최초 배치 Transform 복원, 체크포인트가 없을 때 PlayerStart fallback은 보장되지 않는다. 현재 죽음은 누적 손실보다 깨끗한 학습 재시도에 가깝다.

### 3.2 핵심 진단

현재 문제는 단순히 스킬 수치가 약한 것이 아니라 시스템별 전투 계약이 서로 다르다는 점이다.

| 확인된 규칙 | 현재 생기는 Dynamics | 예상 감각 |
| --- | --- | --- |
| E1 공격 8종의 액션 전체가 피격으로 취소 가능하고 판정 시작은 대체로 1.70~4.95초다. 약공격 판정은 약 0.4초에 시작한다. | 적 전조를 읽고 회피하기보다 빠른 약공격으로 공격 자체를 삭제한다. | 초반 지배감 뒤 단조로움, 낮은 긴장 |
| 약공격은 빠르고 저렴하면서 피해, 그로기, 수락된 Flinch에 의한 행동 억제를 함께 제공한다. | 여러 전투 문제의 가장 싼 해법이 약공격으로 수렴한다. | 선택지는 많지만 쓸 이유가 없는 버튼이 늘어난다. |
| Q 단계는 Hit Resolver 성공이 아니라 액션 시작 성공 시 진행한다. | 적중 정확도보다 시간과 순번을 관리하고, 빗나가도 다음 단계를 준비한다. | 적중으로 우위를 쌓았다는 숙련감이 약하다. |
| Q와 R은 스태미나와 MP를 함께 쓰고 단계 조건 또는 긴 쿨다운과 commitment도 가진다. | 위험과 비용을 중복 지불하므로 확실한 빈틈에서도 보존하기 쉽다. | 스킬이 전술 선택이 아니라 아끼는 버튼이 된다. |
| Action row의 bLocksMovement를 런타임 이동 차단이 소비하지 않고 검사한 피격 Montage에 이동 차단 Notify가 없다. | 피격 모션 중에도 이동할 수 있다. | 화면과 조작권이 어긋나 타격이 가볍게 느껴진다. |
| 네이티브 Invincible 상태는 HitReaction에서 확인되지만 공유 Hit Resolver와 Stat 피해 경로는 이를 확인하지 않는다. | Blueprint 선행 필터가 없다면 회피 중 HP와 그로기는 줄고 피격 모션만 생략될 수 있다. | 회피 판정 불신이 생긴다. |
| 그로기 최대치만으로는 즉시 발동하지 않고 특정 강제 반응 계열 타격을 기다린다. | 누적 뒤 강한 공격으로 결산하는 구조가 될 수 있지만 규칙이 숨겨져 있다. | 게이지가 고장 난 것처럼 보일 수 있다. |

### 3.3 현재 선택 경제

**[현재 구현]**

TestSword 공격력 50과 E1 방어력 1, 대상당 판정 1회 가정에서 약공격 5타는 예상 HP 피해 245, 그로기 75, 설정상 스태미나 50이다. Q1부터 Q3까지는 예상 HP 피해 222, 그로기 65, 설정상 스태미나 60과 MP 60을 요구하고 단계 대기와 입력창이 있다. R 피해 149는 약공격 약 3타와 비슷하지만 스태미나 60, MP 60, 50초 쿨다운과 긴 commitment를 가진다.

이 비교만으로 범위, 다중 적중, Launch, 타깃 수의 가치를 확정할 수는 없다. 다만 기본 Flinch가 적 공격 전체를 취소할 수 있는 현재 계약에서는 Q와 R의 제어 가치가 희소하지 않다.

비용은 입력 시점이 아니라 Ability 활성 Notify에서 차감된다. MP는 비용 전액이 필요하지만 스태미나는 1만 남아 있어도 비용 55 또는 60의 행동을 시작하고 남은 값만 소비할 수 있다. AttackPower는 HitData에 기록되지만 최종 피해 공식에는 사용되지 않는다. DataTable의 StaminaRecoveryDelay는 현재 로더가 사용하지 않으며 스태미나 회복 정지는 Montage Notify와 실제 전력질주 분기가 결정한다. 전력질주 소모 분기는 MP와 그로기 회복 tick도 함께 건너뛴다. 이런 규칙이 의도라면 UI가 설명해야 하고, 의도가 아니라면 affordability, 회복, Commit 계약을 통합해야 한다.

**[검증 필요]**

- 차지 Montage의 두 판정창이 같은 대상에 각각 유효한지와 중복 억제 규칙
- Q3와 R의 Airborne Blueprint가 실제 수직 Launch 값을 채우는지
- Blueprint가 ResolveAttackHit 전에 Invincible을 별도로 검사하는지
- Q와 R의 실제 범위, 타깃 수, 다중 적중 기대값
- 현재 DataTable과 과거 텍스트 export 사이의 수치 drift

## 4. 목표와 미결정의 경계

| 수준 | 내용 |
| --- | --- |
| **확정 방향** | 기본공격을 약공격과 차지공격으로 축소하고 강공격 카테고리를 제거한다. 전투의 중심을 스킬로 옮긴다. 슬롯별 스킬은 상위 호환이 아닌 sidegrade로 설계한다. 카타나 이후 석궁과 대형 낫으로 확장한다. 액션 전체 bCanBeInterrupted 판정을 폐기한다. |
| **우선안** | 상시 S1~S3와 조건부 S4의 3+1 슬롯, Impact I1~I3와 Armor A0~A2, Q의 카타나 S1 이전, 회피 Damage Immunity, 그로기 임계치 자동 발동, 전투 밖 loadout 장착을 수직 검증한다. |
| **미결정** | 최종 물리 키, 3+1 또는 총 3슬롯, MP와 무기 자원의 관계, S4 획득과 유지, 차지 Impact 단계, 실시간 무기 교체, 슬롯별 스킬 수와 성장 방식은 프로토타입 뒤 확정한다. |

목표안을 구현된 Mechanics처럼 문서화하지 않는다. 현재 강공격 코드와 자산은 목표 입력 문법과 대체 경로가 검증되기 전까지 보존한다.

## 5. 목표 공격 문법

### 5.1 기본공격은 두 질문만 담당한다

**[설계 목표]**

- **약공격**은 Press 즉시 발동하는 저위험 확인과 압박 유지 수단이다. 높은 Armor 관통, 최고 그로기 효율, 넓은 범위를 동시에 갖지 않는다.
- **차지공격**은 읽어 낸 큰 빈틈을 높은 Impact, 그로기 또는 공간 변화로 결산한다. 빠른 중립 DPS나 안전한 범용 공격이 되어서는 안 된다.
- 회피와 전력질주 파생 공격은 별도 경제 카테고리가 아니라 약공격 또는 차지공격의 반격형, 추격형, 재진입형 모션으로 취급한다.
- 강공격을 제거해도 약공격의 3~5단 모션은 하나의 입력 카테고리 안에서 리듬과 방향을 표현하는 콘텐츠로 유지할 수 있다.
- Hold를 빨리 놓아 별도 강공격으로 전환하는 분기는 제거한다. 부분 차지가 필요하면 같은 역할 안에서 Impact, 범위, 피해가 단계적으로 커져야 한다.

### 5.2 스킬은 3+1 역할 문법을 공유한다

**[우선안]**

| 슬롯 | 역할 | 주 질문 | 주 출력 |
| --- | --- | --- | --- |
| S1 Tempo / Engine | 흐름 생성 | 안전하게 스킬 순환을 시작할 수 있는가? | 빠른 적중, 토큰, 표식, 자원 생성 |
| S2 Break / Converter | 조건을 우위로 전환 | 지금 이 Armor나 배치를 무너뜨릴 것인가? | 높은 Impact, 그로기, 범위, 제어 |
| S3 Response / Utility | 위협 대응 | 피할지, 받아칠지, 거리를 바꿀지? | 패리, 회피 이동, 가드, 반격, 재장전 |
| S4 Signature / Cash-out | 전투 국면 결산 | 쌓은 우위를 지금 어디에 쓸 것인가? | 큰 범위, 상태 변화, 피니시 |

같은 슬롯의 대안은 같은 문제를 다른 위험, 거리, 조건으로 해결한다. 피해, startup, 비용, 안전성은 같은데 범위와 제어까지 더 좋은 선택지는 sidegrade가 아니다. 한 스킬은 주 역할 하나와 눈에 띄는 보조 역할 하나로 제한한다.

첫 구현은 S1~S3에 기본안을 포함한 선택지 두 개씩과 무기당 고정 S4 하나를 권장한다. 이것만으로 무기당 8개 loadout이 생긴다. 전투 중에는 장착된 3+1만 보이고 loadout 변경은 체크포인트 또는 장비 화면에서 시작한다.

### 5.3 자원은 주 gate 하나를 가진다

일반 스킬은 특별한 이유가 없으면 스태미나와 MP를 동시에 내지 않는다.

1. 행동 시작 전 비용 전액을 검사한다.
2. 시작 시 비용을 예약하고 명시된 Commit에서 차감한다.
3. Commit 전 취소는 예약만 해제하고 진행 상태를 유지한다.
4. Commit 후 빗나감은 비용과 명시된 토큰을 잃는다.
5. 진행과 쿨다운은 시전 성공이 아니라 Commit 또는 ResolvedHitTransaction의 결과를 기준으로 한다.
6. 한 스킬은 자원, 조건, 긴 쿨다운 중 하나를 주 제한으로 사용한다.

공용 스태미나는 이동, 회피, 차지 같은 신체적 commitment를 담당한다. S1~S3는 MP 또는 무기 자원 하나를 사용하고, S4는 전투 행동으로 얻은 게이지나 스택을 결산한다. 석궁이 탄창을 사용한다면 열과 별도 MP를 동시에 추가하지 않는다.

### 5.4 무기는 전투 문법을 바꾼다

- **카타나**: 빠른 근거리 확인, 정밀 차지와 패리, 유효 적중으로 쌓는 단계 우위
- **석궁**: 투사체, 탄창 또는 열 하나, 관통 차지, 표식, 위험한 재장전과 거리 관리
- **대형 낫**: 긴 사거리와 넓은 궤적, 느린 commitment, 높은 Impact와 그로기, 끌기와 모으기

세 무기는 약공격과 차지공격, S1~S4라는 상위 문법을 공유하되 같은 적에게 다른 거리와 순환을 요구해야 한다. 카타나보다 느리고 피해만 높은 낫, 모든 문제를 안전한 원거리에서 해결하는 석궁은 목표가 아니다.

## 6. 목표 피격과 행동권 계약

### 6.1 액션 종료 권한을 분리한다

**[설계 목표]**

- **자발적 취소**: ChainWindow, RecoveryEscapeWindow, DodgeCancelPolicy가 결정한다.
- **외부 피격 취소**: AttackImpact와 현재 ActionPhase의 RequiredImpact를 비교한다.
- **강제 종료**: Death, GroggyBreak, Finisher, WorldReset 같은 우선순위가 결정한다.

종료 이유는 Completed, VoluntaryChain, RecoveryEscape, IncomingHit, GroggyBreak, Death, Finisher, WorldReset처럼 기록한다. 피격을 버티면서 회피로 빠져나오는 공격을 표현하려면 이 세 권한을 다시 하나의 bool로 합치지 않는다.

### 6.2 Impact와 Armor는 행동 중단만 결정한다

| Impact | 역할 |
| --- | --- |
| I1 견제 | 노출된 행동과 가벼운 반응을 만든다. |
| I2 압박 | 경량 Armor를 관통하고 중간 반응을 만든다. |
| I3 붕괴 | 중량 Armor를 관통하는 차지, Break, 조건부 Signature다. |

| Armor | 중단에 필요한 최소 Impact |
| --- | --- |
| A0 노출 | I1 이상 |
| A1 경량 | I2 이상 |
| A2 중량 | I3 이상 |

AttackImpact가 RequiredImpact 이상일 때만 행동 중단을 허용한다. Impact가 부족해도 HP와 그로기 피해는 적용할 수 있지만 현재 행동, 일반 Reaction, Launch는 유지하지 않는다. Armor는 피해 면역이 아니다.

Armor는 Startup, Active, Recovery마다 달라진다. 일반 적의 가벼운 공격은 Startup A0, Active A1, Recovery A0부터 시험한다. 다단 공격은 구간을 반복한다. Profile 또는 Phase가 누락되면 A0로 처리하고 개발 경고를 남긴다.

피격 반응 종료 직후에는 짧은 PostReactionControlResistance로 I1 연속 반응과 재취소를 거부한다. 피해와 그로기는 받으며 I2, I3, GroggyBreak, Death는 통과한다. 이는 무적이 아니라 적이 다음 행동을 시작할 최소 행동권을 보장하는 군중 제어 저항이다.

### 6.3 한 타격은 하나의 결과 거래다

Contact, ResolvedHit, HitConfirm을 구분한다.

1. 실행 ID와 대상 ID로 중복 적중을 검사한다.
2. 대상, 팀, 사망, Damage Immunity를 검사한다.
3. Guard와 실제 HP, 그로기 피해를 계산한다.
4. Impact와 Armor를 비교한다.
5. Groggy와 특수 상태를 판정한다.
6. 행동 취소, Reaction, MovementResult를 한 번 결정한다.
7. HitStop, 카메라, VFX, SFX, HUD에 같은 결과를 배포한다.
8. 공격자 스킬에 같은 결과의 HitConfirmTags를 전달한다.

ResolvedHitTransaction은 DamageResult, DefenseResult, ImpactResult, ReactionResult, MovementResult, AttackerConsequence, HitConfirmTags와 실행 정보를 함께 기록한다. DirectDamage와 ArmoredHit는 동시에 존재할 수 있다. ArmorPierced는 행동 중단을 포함하지만 영구 Armor 파괴를 뜻하지 않는다. Guard chip은 GuardedHit으로 기록하고 스킬이 이를 인정할 때만 별도 정책을 둔다.

우선순위는 Death > Scripted 또는 Grab > GroggyBreak > ArmorPierced 또는 강제 상태 > 일반 Reaction > ArmoredHit으로 고정한다. 같은 프레임에 이미 Contact가 성립한 상호 타격은 v1에서 trade를 허용하고 안정적인 처리 순서를 기록한다.

Impact는 행동을 끊을 자격이며 Reaction 강도와 별개다. Airborne과 KnockDown은 ControlTier와 대상의 체중, 보스, Launch 저항을 추가로 통과해야 한다. Armor가 유지되면 요청된 Reaction과 Launch를 함께 거부하고 전용 피드백으로 “피해는 주었지만 행동은 끊지 못했다”를 알려야 한다.

### 6.4 무적, Guard, 이동, 그로기를 분리한다

- **Damage Immunity**는 HP, 그로기, 일반 HitConfirm과 Reaction을 막는다. 회피 i-frame의 v1 기본안이다.
- **Phase Armor**는 부족한 Impact의 행동 취소를 막지만 피해와 그로기는 막지 않는다.
- **PostReactionControlResistance**는 피격 직후 I1 연속 반응을 막는다.
- **Guard**는 별도의 피해, 그로기, Impact 정책이다.
- **Movement Lock**은 플레이어 이동 적용을 막되 입력 버퍼 자체는 보존할 수 있다.

HitReaction 시작 시 source token으로 이동 잠금을 획득하고 Recovery, 조기 취소, 사망, Montage 종료에서 같은 token을 회수한다. 사용되지 않는 row bool과 Montage Notify를 중복 진실 공급원으로 두지 않는다.

그로기는 v1에서 유효 Hit이 최대치에 도달한 즉시 GroggyBreak를 만드는 안을 우선한다. 특정 Break 타격을 기다리는 구조를 원하면 숨은 조건 대신 GroggyPrimed 상태와 명확한 HUD를 별도로 설계한다.

## 7. Q와 R의 목표 역할

현재 Q는 카타나 S1 Tempo 후보로 이전한다.

Q 시전 → 유효 HitConfirm → 다음 단계 token 획득 → 제한 시간 동안 다른 행동 혼합 → 다음 Q가 token을 Commit에서 소비 → Q3 결산

기본 성공은 DirectDamage다. Guard chip, 무적, 잘못된 대상, 중복 필터는 실패이며 여러 적을 맞혀도 한 실행에서 단계는 한 번만 진행한다. Commit 전 취소는 비용 예약을 해제하고 token을 유지한다. Commit 후 Q2 또는 Q3가 실패하면 비용과 token을 잃고 Q1으로 돌아가는 안을 먼저 시험한다. Q3의 Airborne이 통하지 않는 중량 적과 보스에는 긴 Stagger, 큰 그로기 또는 전용 피드백으로 동등한 결산 가치를 준다.

현재 R은 50초 시간 쿨다운에서 분리해 카타나 S4 후보로 재평가한다. S4는 전투 참여로 준비되고 매 전투 실제 사용되며, 전투 종료까지 보존하거나 준비 즉시 생각 없이 누르는 추가 DPS가 되지 않아야 한다.

HUD는 장착 슬롯, Q 단계 token 획득, 유지 시간, 실패 이유, S4 준비를 사건 중심으로 표시한다.

## 8. 목표 Dynamics와 가드레일

목표 교전은 다음 순환이다.

약공격 또는 S1로 확인과 상태 생성 → 적이 Armor 공격으로 주도권 시도 → 맞는 Impact의 차지 또는 S2, 회피 또는 S3 선택 → Recovery punish → 그로기, token, Signature 결산 → 중립 복귀

가드레일:

- 약공격은 자주 쓰여도 되지만 Armor, 거리, 다수전, 빠른 그로기 누적까지 모두 최고여서는 안 된다.
- 차지는 피해만 높은 약공격이 아니라 최소 한 단계 높은 Impact 또는 명확한 공간 역할을 가진다.
- S1은 S2와 S3의 문제까지 해결하지 않는다.
- S2 Break는 대상 Armor와 조건을 요구하며 모든 적을 항상 넘어뜨리지 않는다.
- S3는 최고의 안전성과 높은 DPS를 동시에 갖지 않는다.
- S4는 보존품도 무조건 추가 DPS도 아니다.
- UninterruptiblePhase와 강제 무적은 짧고 읽을 수 있으며 이후 확실한 대응 창을 제공한다.
- 같은 슬롯의 한 스킬이 여러 상황에서 선택률과 성공 효율을 함께 압도하면 수치보다 역할 중복을 먼저 점검한다.

## 9. 구현 순서

### P0. 현재 취소 경로 추적

bCanBeInterrupted 사용처를 외부 피격, 자발적 취소, 강제 종료로 분류한다. 변경 전 HitTransactionTrace와 ActionPhaseTrace를 붙이고 PIE 재현 로그를 수집한다.

최소 로그는 Frame, HitId, ExecutionId, Attacker, Victim, Action, Phase, Impact, RequiredImpact, DamageResult, Groggy, Reaction, CancelReason, HitConfirmTags를 포함한다.

### P1. 최소 전투 계약 수직 단면

Contact, ResolvedHit, HitConfirm을 분리하고 I1~I3와 A0~A2의 3×3 판정을 자동화 테스트로 고정한다. E1 공격 하나에 Startup A0, Active A1, Recovery A0를 적용하고 약공격 I1, 차지 I3으로 검증한다. 테스트 전용 Phase 강제 전환을 제공해 Startup에서 계속 끊기는 상황을 피한다. ArmoredHit, ArmorPierced, Invulnerable 피드백을 임시 색과 소리로 구분한다.

### P2. 이동과 기본공격 문법

HitReaction 이동 잠금을 생명주기 token으로 통합하고 공격 중 이동과 회전을 MovementPolicy로 정의한다. 목표 입력에서 강공격 진입을 제거하되 자산은 보존한다. 약공격과 차지의 반응성, startup, Impact, 비용 역할을 다시 측정한다. 회피와 전력질주 공격을 문맥형 변주로 재매핑한다.

### P3. 카타나 슬롯 수직 검증

먼저 SlotRole, 장착 SkillId, HitConfirmPolicy, 주 gate, 비용 예약과 Commit을 데이터 계약에 추가한다. 현재 Q를 S1 적중 체인으로 이전하고 HUD에는 S1과 token만 표시한다. 다음으로 차지와 Q3와 겹치지 않는 S2 Break 하나, 패리 또는 회피형 S3 하나를 추가한다. 마지막으로 현재 R을 전투 중 획득하는 S4로 재평가한다.

### P4. Sidegrade와 무기 확장

카타나에서 슬롯당 두 선택지의 역할과 지표를 검증한 뒤 석궁의 거리와 재장전 리듬, 대형 낫의 범위와 commitment 리듬을 추가한다. 스킬 트리와 저장 구조에는 무기, 장착 SkillId, 해금 상태를 연결한다. 성장 선택은 단순 피해량 상승보다 다른 Dynamics를 선택하게 해야 한다.

## 10. 검증 계약

### 10.1 자동화와 런타임 검증

- Impact와 Armor 9조합의 행동 취소 결과
- Damage Immunity 중 HP, 그로기, Reaction 미적용
- Armor 유지 중 피해와 그로기 적용, 액션과 Launch 유지
- ArmorPierced 시 취소, Reaction, 이동 결과 일치
- GroggyBreak, Death, Scripted 결과의 우선순위
- Montage 조기 종료, 사망, 재생 실패 뒤 Armor와 이동 token 해제
- 한 공격 실행의 다중 판정창과 대상별 중복 적중
- 비용 예약, Commit 전 취소, Commit 후 실패의 자원과 token 결과
- 같은 프레임 상호 타격의 결정적 순서

### 10.2 플레이테스트 이벤트

공격 실행 단위로 입력, 시작 성공과 실패 이유, ExecutionId, HitWindowId, 비용 전후 자원, 유효 대상 수, HP와 그로기, 당시 ActionPhase, Impact와 RequiredImpact, 최종 결과, CancelReason, HitConfirmTags, 이동 token, 종료 이유를 연결한다.

Q는 단계별 유효 적중, token 획득과 소비, 만료와 초기화, Q2와 Q3 도달률, Q3 제어 또는 대체 결산 성공률, Commit 전 취소와 Commit 후 실패, 단계 사이 기본공격과 회피 횟수를 기록한다.

슬롯은 사용 기회 발생 → 인지 → 입력 → 시작 → 성공 → 사용 후 주도권 획득 전환율을 기록한다. 사용하지 않은 이유를 사용 불가, 자원 부족, 기회 미인지, 가치 부족, 입력 기억 실패로 구분한다. 같은 슬롯 대안의 선택률, 성공률, 사용 후 피격률, 해결한 문제를 비교한다.

### 10.3 판단 지표

단순 사용률 대신 시작 성공률, 적중률, 초당 피해, 자원당 피해, 그로기 기여, 적 행동 취소 기여, 사용 후 피격률을 본다. 약공격만 사용한 전투와 스킬을 섞은 전투의 클리어 시간, 피격 수, 그로기 횟수를 비교한다.

다음 상황을 분리한다: 중립 1대1, Armor Active, Recovery punish, 원거리 재진입, 다수전, 보스 제어 저항, 낮은 자원. 카타나, 석궁, 대형 낫이 같은 적에게 다른 거리와 순환을 만드는지도 검증한다. 한 sidegrade가 여러 적과 숙련도에서 선택률과 효율을 함께 압도하면 역할 중복을 우선 조사한다.

## 11. 공격 데이터의 최소 문서 계약

각 공격 row는 다음 항목을 한 곳에 기록한다.

- 이름과 주 역할, 무기와 슬롯, 입력과 문맥
- 시작 조건, 비용 전액 검사, 예약과 Commit, 실패 손실
- Startup, Active, Recovery와 각 구간 Armor
- 자발적 취소, 외부 피격 취소, 강제 종료 우선순위
- MovementPolicy, 회전, Damage Immunity, Guard, Control Resistance
- ExecutionId, HitWindow, 범위, 궤적, 타깃 수, 중복 적중
- HP, 그로기, Impact, RequestedReaction, Launch
- A0~A2, Guard, 보스와 중량 적 저항에 대한 결과
- 인정하는 HitConfirmTags와 한 실행당 진행 횟수
- 적중, ArmoredHit, 방어, 무적, 빗나감, Commit 전후 취소의 후속 상태
- 연계 조건과 유지 시간, 일반 적중과 Armor 결과의 피드백
- 기대 피해뿐 아니라 startup, recovery, 자원, 조건, 실패 위험
- 목표 Dynamics와 Aesthetics

DataTable, Montage Notify, Blueprint, C++이 같은 필드에 다른 의미를 부여하면 구현 전에 문서 계약부터 해결한다.

## 12. 구현 전 결정할 질문

1. 3+1 슬롯을 확정할지 총 3슬롯으로 줄일지
2. S1 Tempo, S2 Break, S3 Response, S4 Signature 역할을 모든 무기에 고정할지
3. 약공격과 차지공격의 키와 Hold 규칙
4. 차지를 I2와 완전 차지 I3로 나눌지 항상 I3로 둘지
5. I1~I3와 A0~A2가 v1에 충분한지
6. UninterruptiblePhase 허용 목록과 시각적 규칙
7. 회피 i-frame을 Damage Immunity로 확정할지
8. Reaction별 MovementPolicy와 token 해제 시점
9. 그로기를 임계치 즉시 자동 발동으로 확정할지
10. Q2와 Q3의 Commit 후 실패 시 Q1 초기화 여부
11. Q 유지 시간과 보스 대체 결산
12. MP 공용 자원 또는 무기별 자원 선택
13. S4 획득 행동과 교전 사이 유지 여부
14. 슬롯당 첫 sidegrade 제작 수
15. 석궁의 탄창 또는 열 선택
16. 무기와 loadout 교체 시점
17. 스킬 트리의 sidegrade 해금과 수직 성장 범위
18. 죽음을 손실 장치로 만들지 빠른 학습 리셋으로 유지할지

## 13. 주요 근거

### 현재 구현

- Source/Maverick/Components/MVCombatComponent.cpp/.h
- Source/Maverick/Components/MVActionComponent.cpp
- Source/Maverick/Components/MVHitReactionComponent.cpp/.h
- Source/Maverick/Components/MVStatComponent.cpp
- Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- Source/Maverick/Combat/MVAbilityBase.cpp
- Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- Source/Maverick/AI/Task/MVEnemyCombatActionTask.cpp
- Source/Maverick/System/MVDeathRespawnFlow.cpp
- Source/Maverick/UI/HUD/

### 에셋과 데이터

- Content/Table/Attack/Player/Yone_Attack.uasset
- Content/Table/Attack/NPC/E1/DT_E1_Attack.uasset
- Content/Table/HitReaction/Player/
- Content/Table/HitReaction/NPC/E1/
- Content/Table/Weapons/
- Content/Table/Stat/
- 플레이어와 E1의 공격, 회피, 피격 Montage
- BP_ThirdPersonCharacter, BP_MeleeAttack, BP_ShockWave, BP_ThirdSkill, BP_PlayerSkillR
