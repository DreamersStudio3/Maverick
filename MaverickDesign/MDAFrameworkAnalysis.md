# Maverick MDA 전투 구조 v0.3

> 작성일: 2026-07-26
> 상태: 다음 마일스톤 팀 검토안
> 기준: `develop`의 `fc3620f`
> 목적: v0.1의 전체 설명과 v0.2의 장문 진단을 반복하지 않고, 다음 전투 마일스톤에서 **바뀌는 규칙과 검증 기준**만 정리한다.

## 0. v0.3 결론

Maverick의 다음 전투 구조는 아래 일곱 문장으로 압축한다.

1. 기본공격은 **약공격과 차지공격**만 남기고 강공격을 삭제한다.
2. 전력질주·회피 뒤 기본공격은 별도 경제가 아니라 약공격/차지공격의 **문맥형 진입 모션**으로 유지한다.
3. 스킬은 **S1 Tempo / S2 Area / S3 Response / S4 Signature**의 고정 역할을 가진다.
4. S1은 짧게 순환하는 스킬 Charge로 전투 템포를 만들고, S4는 시간 쿨다운이 아니라 **유효 적중으로 채우는 게이지**를 요구한다.
5. 약공격 연타가 모든 문제를 해결하지 않도록 v0.2의 Impact/Armor와 통합 Hit 결과 계약을 선행 기반으로 유지한다.
6. 약공격과 차지공격은 한 물리 버튼의 Tap/Hold로 묶고, 반응형 스킬은 modifier 지연이 없는 직접 입력을 우선한다.
7. 락온 여부는 회피 모션을 고르지 않는다. 방향 입력이 있으면 기본 Roll, 정확한 피격 타이밍을 통과했을 때만 짧은 Perfect Dodge Step으로 결산한다.

목표 전투 순환은 다음과 같다.

`약공격·문맥공격으로 교전 → S1으로 템포 유지 → 차지로 큰 빈틈 결산 / S2로 다수전 해결 / S3로 위협에 반응 → 적중 게이지 충전 → S4로 전투 절정 결산`

한 문장 북극성:

> **Maverick의 전투는 간결한 기본공격으로 흐름을 유지하고, 역할이 분명한 네 스킬로 상황을 풀며, 적극적으로 적중시킨 공격을 필살기로 결산하는 공정하고 묵직한 액션 전투다.**

---

## 1. v0.2에서 유지하는 것과 바꾸는 것

### 유지

- 약공격/차지공격으로 기본 문법을 축소한다.
- 강공격 카테고리를 제거한다.
- 스킬은 네 슬롯의 역할을 고정하고, 장착 스킬은 같은 역할 안에서 sidegrade로 만든다.
- `bCanBeInterrupted` 하나에 의존하지 않고 Impact와 액션 구간별 Armor를 비교한다.
- Contact와 최종 적중 결과를 구분하고, 스킬 진행과 자원 획득은 최종 결과를 구독한다.
- Damage Immunity, Armor, Guard, 이동 잠금을 서로 다른 규칙으로 다룬다.

### 변경

| v0.2 | v0.3 |
| --- | --- |
| S2를 Break/Converter 중심으로 정의 | S2를 **Area** 중심으로 정의. Break는 일부 스킬의 보조 속성 |
| S1의 Q 3단 적중 연계를 주요 후보로 검토 | 단일 쿨다운도 `MaxCharges = 1`로 표현하는 공통 Charge 모델과 독립된 연속기 Stage로 분리 |
| S4 획득 방식이 미정 | 모든 기본·스킬 공격에 게이지 획득값을 두고 유효 적중으로만 충전 |
| 현재 R의 50초 쿨다운을 재평가 | 긴 시간 쿨다운을 제거하고 게이지 100%를 주 gate로 사용 |
| 18개의 팀 질문을 병렬 제시 | 이번 마일스톤에 필요한 결정만 남김 |

이 문서는 v0.1의 전체 시스템 소개나 v0.2의 수치표·몽타주 타이밍·전투 밖 시스템 분석을 다시 싣지 않는다. 해당 근거는 Git의 v0.2 문서 이력에서 확인하고, v0.3은 변경 계약과 구현 순서의 기준으로 사용한다.

---

## 2. 현재 프로젝트 진단

### 2.1 목표 방향은 정해졌지만 런타임 구조는 아직 Q/R 중심이다

현재 코드와 v0.3 사이의 핵심 간극은 다음과 같다.

| 영역 | 현재 구현 확인 | v0.3에 필요한 변화 |
| --- | --- | --- |
| 기본공격 종류 | `HeavyAttack`, `SprintHeavyAttack`, `DodgeHeavyAttack`이 enum·입력·Chooser·맵에 존재 | Heavy 의미와 진입 경로 제거. 필요한 문맥 공격은 Charge 문맥으로 재명명 |
| 차지 입력 | Hold를 일찍 놓으면 Heavy row/chain으로 전환 | Commit 전에 놓으면 약공격, Commit까지 유지하면 차지공격으로만 분기 |
| 문맥 공격 | 약/강 입력을 회피·전력질주 약/강으로 치환 | 약/차지 문맥으로 유지하되 별도 밸런스 카테고리로 취급하지 않음 |
| 스킬 슬롯 | 입력과 `ResetSkillMap`이 Q/R 두 슬롯만 등록 | S1~S4 네 슬롯과 역할 기반 데이터 계약 필요 |
| S1 사용권 | HUD의 `ActiveStackIndex`는 실제 Charge가 아니라 체인 단계 | `CurrentChainStage`와 `AvailableCharges`를 분리 |
| S4 제한 | 현재 R은 일반 쿨다운·스태미나·MP 비용 구조 | 긴 쿨다운 대신 적중 게이지 준비 상태와 Commit 소비 필요 |
| 적중 결과 | `FMVResolvedHitData`는 피해·반응 수치 중심이며 실행 ID와 결과 태그가 없음 | 같은 공격의 중복 지급을 막고 유효 적중을 판정할 실행 ID·결과 태그 필요 |
| HUD | 두 슬롯의 아이콘 단계와 시간 쿨다운 표시 중심 | S1 Charge, S2/S3 사용 가능 상태, S4 게이지와 Ready 피드백 필요 |
| 입력 배치 | 약/HeavyCharge 분리, Q/R 두 스킬과 LB+Y chord 기준 | 한 버튼 Basic과 네 역할 스킬의 장치별 직접 입력 |
| 회피 선택 | Strafe/Aim 문맥에서 Step, 그 외 Roll을 액션 시작 전에 선택 | 락온과 무관한 기본 Roll, 유효 Hit 타이밍 성공 시 Perfect Step |

따라서 이번 변경은 DataTable 수치만 바꾸는 작업이 아니다. 특히 S1 Charge와 S4 게이지를 현재 `FMVSkillEntry`의 체인/쿨다운에 억지로 넣으면 UI의 “스택”, 연계 단계, 실제 사용 횟수가 다시 같은 이름으로 섞인다.

### 2.2 아직 남아 있는 지배 전략 위험

v0.2 진단의 핵심 문제는 그대로다.

- 빠른 약공격의 Flinch가 적 행동을 지나치게 쉽게 취소하면 S2의 범위와 S3의 대응 기회가 나오기 전에 약공격 연타로 수렴한다.
- 차지공격이 높은 Impact·그로기·공간 보상 중 어느 것도 확실히 갖지 못하면 강공격을 삭제해도 약공격만 남는다.
- 무적과 피해 적용이 같은 최종 Hit 결과에서 결정되지 않으면 S3의 회피형 스킬은 성공 화면과 실제 피해가 어긋날 수 있다.
- 공격 실행 단위가 없으면 S2 다단/다수 적중이 S4 게이지를 비정상적으로 빠르게 채울 수 있다.

따라서 **전투 결과 계약 → 기본공격 정리 → 네 슬롯 수직 단면 → 수치 튜닝** 순서를 유지한다.

---

## 3. 기본공격 계약

### 3.1 약공격

- 차지공격과 같은 물리 버튼을 사용한다.
- 입력 시작 시 공통 LeadIn을 열고, `ChargeCommitTime` 전에 버튼을 놓으면 약공격 section으로 분기한다.
- 별도의 강공격·부분 차지 분기는 두지 않는다.
- 버튼을 놓은 뒤에야 전체 액션을 새로 시작하는 방식보다, 입력 시작 시 액션과 버퍼 문맥을 확보하고 짧은 공통 선행 모션에서 분기하는 방식을 우선한다.
- 가장 낮은 위험으로 압박과 공격 흐름을 유지한다.
- 적은 게이지를 안정적으로 생성하지만 최고 Impact, 최고 그로기 효율, 범위 해결을 함께 갖지 않는다.
- 약공격 콤보 모션은 여러 타를 유지할 수 있다. 삭제하는 것은 콘텐츠가 아니라 **강공격이라는 별도 선택 카테고리**다.

### 3.2 차지공격

- 약공격과 같은 물리 버튼을 길게 눌러 시작한다.
- `ChargeCommitTime`까지 Hold를 유지하면 약공격 예약을 취소하고 Charge section으로 전환한다.
- Commit 전에 버튼을 놓으면 Hold 길이와 관계없이 약공격을 재생한다.
- 조기 해제는 Heavy row, Heavy chain, 부분 차지, 무행동 취소로 전환하지 않는다.
- Commit 이후에는 버튼을 놓아도 차지공격을 끝까지 진행한다.
- 차지공격은 큰 빈틈을 높은 Impact·그로기·범위 중 하나 이상으로 결산해야 한다.

권장 첫 계약:

| 구간 | 결과 |
| --- | --- |
| 버튼 Press | 공통 LeadIn 시작, 아직 공격 종류 미확정 |
| `ChargeCommitTime` 전 Release | 약공격 section으로 전환 |
| `ChargeCommitTime` 도달 | 차지공격 Commit, Charge section으로 전환 |
| Commit 이후 Release | 차지공격 유지 |

이 계약으로 기본공격 결과는 약공격과 차지공격 두 가지뿐이다. Hold 길이에 따라 중간 배율이나 별도 모션을 만들면 삭제한 강공격이 이름만 바뀌어 돌아오므로 도입하지 않는다.

약공격은 Release 시점에 확정되므로 입력 시작부터 Release, 약공격 section 시작, 실제 판정까지의 시간을 기록한다. `ChargeCommitTime` 경계에서 Release와 Commit이 같은 프레임에 들어왔을 때 결과가 흔들리지 않도록 `ReleaseTime < CommitTime`만 약공격으로 고정한다.

### 3.3 전력질주·회피 문맥

문맥형 공격은 유지한다.

| 문맥 | 약공격 | 차지공격 |
| --- | --- | --- |
| 전력질주 후 | 추격·거리 압축형 시작 모션 | 전진 관성이나 사거리 변주를 가진 Charge 시작 |
| 회피 후 | 반격·재진입형 시작 모션 | 위치를 바꾼 뒤 큰 빈틈을 결산하는 Charge 시작 |

구현에서는 기존 `SprintHeavyAttack`과 `DodgeHeavyAttack`을 그대로 남기지 않는다. 콘텐츠를 재사용하더라도 의미는 `SprintChargeAttack`과 `DodgeChargeAttack` 또는 Charge의 Context 값으로 이전한다.

문맥 공격의 원칙:

- 일반 약/차지와 같은 자원·Impact 역할을 상속한다.
- 이동에서 교전으로 이어지는 startup, 사거리, 방향만 주로 달라진다.
- 한 번의 전력질주/회피 문맥에서 한 번만 소비한다.
- 일반 공격보다 피해·안전·게이지 획득이 모두 높아지지 않는다.

---

## 4. 네 스킬의 역할

| 슬롯 | 역할 | 주 사용 상황 | 주 제한 | 목표 감각 |
| --- | --- | --- | --- | --- |
| S1 | Tempo | 기본공격 사이의 잦은 삽입 | 짧게 순환하는 스킬 Charge | 리듬과 주도권 유지 |
| S2 | Area | 다수전, 넓은 궤적, 공간 정리 | 자원 또는 긴 Commit | 공간 통제 |
| S3 | Response | 적 공격에 대한 패리·가드·특수 회피 | 타이밍과 별도 사용 제한 | 읽고 받아친 숙련감 |
| S4 | Signature | 축적한 공격 성공의 결산 | 적중 게이지 100% | 전투의 절정과 일격감 |

### 4.1 S1 Tempo: 공통 Charge 모델로 자주 섞는다

v0.3의 S1 Charge는 **연계 단계가 아니라 남은 스킬 사용권**이다.

- 단일 쿨다운 S1은 `MaxCharges = 1`로 표현한다.
- 저장형 S1은 `MaxCharges > 1`로 표현하고 빠진 Charge를 순차 회복한다.
- `AvailableCharges`와 `CurrentChainStage`를 분리한다. 0 Charge는 Q3가 아니라 사용 불가 상태다.
- Commit 시 Charge 하나를 소비하고, Commit 전 취소는 예약한 Charge를 돌려준다.
- 약/차지의 유효 적중은 현재 회복 중인 다음 Charge 시간을 줄일 수 있다.
- S1 자체 적중은 기본값으로 자기 Charge 회복을 단축하지 않는다.
- 최대 Charge에서는 빈자리가 없으므로 회복이 진행되거나 미래 쿨다운이 미리 저장되지 않는다.

단일형, 복수 Charge형, Q1→Q2→Q3 연속기의 공통 런타임과 데이터 구조는 별도 문서인 [S1 Tempo 공통 Charge·연속기 구현안](./S1TempoChargeImplementation.md)을 기준으로 한다.

MDA:

- Mechanics: 짧게 회복되는 복수 사용권, 적중으로 회복 가속.
- Dynamics: S1을 아끼지 않고 기본공격 사이에 삽입하며 Charge를 순환한다.
- Aesthetics: 공격 리듬을 직접 굴린다는 속도감과 통제감.

가드레일:

- S1이 넓은 범위, 높은 Break, 강한 방어까지 함께 해결하지 않는다.
- Charge 개수와 연계 단계는 별도 상태와 HUD로 표현한다.
- Charge가 0이어도 약공격·차지·기본 회피는 정상적으로 사용할 수 있다.

### 4.2 S2 Area: 범위와 배치 문제를 해결한다

S2의 주 역할은 적 행동 하나를 무조건 끊는 것이 아니라 **다수전과 공간 배치 해결**이다.

가능한 출력:

- 넓은 부채꼴/원형/직선 범위
- 적 모으기·밀어내기·관통
- 여러 대상에게 그로기 또는 중간 Impact 분산
- 플레이어 주변의 압박 해소와 재배치

가드레일:

- 단일 대상 피해 효율은 차지공격이나 S4보다 낮다.
- 대상 수가 늘었다고 S4 게이지가 무제한으로 배수 증가하지 않는다.
- 모든 Armor를 끊는 범위 Break를 기본값으로 주지 않는다.
- S1처럼 자주 순환하지 않으며, 범위가 필요 없는 1대1에서는 항상 최적이 아니어야 한다.

MDA:

- Mechanics: 넓은 판정과 대상 수 보상, 단일 대상 효율 제한.
- Dynamics: 적을 모으거나 공격 각도를 만든 뒤 S2로 공간을 정리한다.
- Aesthetics: 수적 압박을 한 번에 뒤집는 시원함과 공간 통제감.

### 4.3 S3 Response: 기본 회피와 다른 대응 스킬

기본 회피는 스태미나를 내고 언제든 생존 위치를 바꾸는 공용 문법이다. 기본 회피 안의 Perfect Dodge는 정확한 타이밍을 짧은 Step과 빠른 복귀로 보여주지만, 자동 반격이나 별도 스킬 효과까지 제공하지 않는다. S3는 적의 위협을 읽고 더 큰 주도권을 얻는 무기별 대응이다.

| 구분 | 기본 회피 | S3 반응 스킬 |
| --- | --- | --- |
| 사용 가능성 | 스태미나가 있으면 반복 가능 | 스택·짧은 쿨다운·무기 자원 중 하나로 제한 |
| 성공 조건 | 공격을 피하고 위치를 바꿈 | 위협 판정과 반응 창이 실제로 겹쳐야 성공 |
| 성공 보상 | Roll로 피해 회피. Perfect Dodge이면 짧은 Step과 빠른 중립 복귀 | 패리, 반격권, 특수 이동, 공격 연결 |
| 실패 손실 | 스태미나와 회피 Recovery | 사용권/쿨다운과 더 명확한 Recovery |
| 역할 | 항상 가능한 생존 기반 | 상황을 읽었을 때 주도권을 얻는 고급 해법 |

S3 후보:

- 정밀 패리와 반격
- 짧은 가드 후 받아치기
- 적을 통과하거나 측후방으로 이동하는 특수 회피
- 투사체 대응 이동이나 반사

회피형 S3 계약:

- 기본 회피처럼 스태미나를 소비하지 않아도 되지만, 별도 사용권이나 짧은 쿨다운을 가진다.
- 단순히 더 긴 무적 시간을 주는 무료 회피가 아니라 위치 변화·후속 공격·대상 조건 중 하나가 고유해야 한다.
- 순수 이동 성공만으로 S4 게이지를 지급하지 않는다. 후속 공격이 유효 적중했을 때 해당 공격의 게이지를 얻는다.
- 실패해도 무조건 반격이 나가지 않으며, 성공/실패 피드백을 분리한다.
- 기본 Perfect Dodge보다 판정이 넓고 안전하기만 한 상위 호환으로 만들지 않는다. S3는 별도 사용 제한과 무기별 결과를 지불하고 얻는다.

MDA:

- Mechanics: 짧은 반응 창, 위협과의 교차 판정, 성공 전용 후속 결과.
- Dynamics: 평소에는 기본 회피를 쓰고, 읽은 위협에는 S3로 위험을 감수해 주도권을 빼앗는다.
- Aesthetics: “피한 것”보다 “읽고 받아친 것”에서 오는 숙련감.

### 4.4 S4 Signature: 적중 게이지로 여는 필살기

현재 R의 긴 시간 쿨다운은 기다림으로 준비된다. v0.3의 S4는 **공격을 성공시켜야 준비되는 전투 결산기**다.

기본 계약:

1. 모든 기본공격과 S1~S3의 공격 결과에 `SignatureGain` 값을 둔다.
2. Contact가 아니라 최종 Hit 결과가 유효할 때만 게이지를 지급한다.
3. 게이지가 100이 되면 S4가 Ready 상태가 된다.
4. S4 시작 시 게이지를 예약하고, 되돌릴 수 없는 Commit 시점에 100을 소비한다.
5. Commit 전 취소는 게이지를 돌려주고, Commit 후 빗나감은 게이지를 소비한다.
6. S4 자체는 게이지를 생성하지 않는다.
7. 긴 시간 쿨다운은 제거한다. 연출 중 중복 입력을 막는 짧은 실행 잠금만 둔다.

초기 게이지 상대값:

| 유효 결과 | `SignatureGain` 시작안 |
| --- | ---: |
| 약공격 1회 실행 | 3 |
| 전력질주/회피 약공격 | 4 |
| 차지공격 | 10 |
| S1 적중 | 5 |
| S2 첫 대상 적중 | 6 |
| S2 추가 대상 | 대상당 +1, 실행 합계 최대 10 |
| S3 반격 공격 적중 | 8 |
| S4 | 0 |

이 표는 게이지 최대 100을 전제로 한 첫 플레이테스트 값이다. 약공격만으로도 언젠가는 채울 수 있지만, 차지와 역할에 맞는 스킬을 섞으면 더 자연스럽게 준비되어야 한다.

유효 적중 기본 정의:

- 살아 있는 적대 대상에게 실제 HP 피해가 적용된 Hit
- 무적, 잘못된 대상, 사망한 대상, 중복 필터에 막힌 Contact는 0
- Guard chip과 환경 오브젝트는 기본 0
- 같은 AttackExecution의 다단 판정은 데이터의 실행당 상한을 넘지 않음
- S2는 다수전 가치를 인정하되 추가 대상 보너스에 상한을 둠

게이지 유지 정책은 첫 프로토타입에서 다음처럼 시험한다.

- 전투 중 시간 감쇠 없음
- S4 사용과 사망 시 0
- 체크포인트 휴식 시 0
- 일반 전투 종료 뒤에는 유지

이 정책은 작은 전투의 성공을 다음 전투에 가져갈 수 있지만 보스 앞 사전 충전도 허용한다. 플레이테스트에서 사전 충전이 의무 행동이 되면 “전투 종료 후 일부만 유지” 또는 “보스 교전 시작 시 최소/최대 보정”을 비교한다.

MDA:

- Mechanics: 공격별 획득값, 유효 HitConfirm, 게이지 100의 사용 조건.
- Dynamics: 시간을 기다리지 않고 기본공격과 스킬을 적극적으로 적중시켜 절정을 준비한다.
- Aesthetics: 필살기가 공짜 쿨다운 버튼이 아니라 내가 만든 전투 성과의 결산으로 느껴진다.

---

## 5. 공용 전투 계약

### 5.1 유효 적중은 한 곳에서 결정한다

S1 회복 가속과 S4 게이지가 서로 다른 Blueprint의 Overlap 이벤트를 직접 세면 안 된다. 한 번의 공격 실행은 하나의 최종 결과 흐름을 가져야 한다.

최소 필요 정보:

| 정보 | 용도 |
| --- | --- |
| `AttackExecutionId` | 같은 실행의 다단·다수 적중 묶기 |
| `HitWindowId` | 한 실행 안의 판정창 구분 |
| 공격 row/슬롯 역할 | 공격별 S1 회복·S4 획득값 조회 |
| `DamageResult` | 면역/방어/실제 피해 구분 |
| `ImpactResult` | Armor 유지/행동 취소/Armor 관통 구분 |
| `HitConfirmTags` | DirectDamage, GuardedHit, ParrySuccess 등 스킬별 성공 판정 |
| 대상·실행당 지급 누계 | S2·다단 공격의 중복 게이지 방지 |

권장 순서:

`Contact → 대상/중복/무적 검사 → 피해·Guard → Impact/Armor → Reaction·이동 → 최종 HitConfirm → S1/S4 자원 지급 → HUD·VFX`

### 5.2 Impact/Armor는 계속 선행 조건이다

슬롯 역할을 바꾸더라도 v0.2의 최소 구조는 유지한다.

| 단계 | 의미 |
| --- | --- |
| I1 | 약공격과 빠른 S1의 견제 |
| I2 | 일부 S2의 압박 |
| I3 | 차지공격과 조건부 스킬의 붕괴 |
| A0 | I1 이상에 중단 |
| A1 | I2 이상에 중단 |
| A2 | I3 이상에 중단 |

Armor는 피해 면역이 아니다. Impact가 부족하면 피해와 그로기는 적용하되 현재 행동과 강한 Reaction을 유지한다. 그래야 약공격은 압박에 기여하면서도 모든 적 공격을 삭제하지 못하고, 차지와 S3 대응이 실제 선택지가 된다.

### 5.3 자원 이름을 분리한다

다음 상태를 한 `Stack`이나 `Cooldown`에 합치지 않는다.

- `ChainStage`: 한 스킬 안의 연계 단계
- `AvailableCharges`: S1~S3의 남은 스킬 사용권
- `ActiveRechargeRemaining`: 다음 Charge 하나의 재사용 제한
- `SignatureGauge`: S4 준비도
- `ActionCost`: 스태미나/MP 등의 시작 비용

HUD와 로그도 같은 이름을 사용한다. 현재 `ActiveStackIndex`가 체인 단계를 가리키는 상태는 v0.3 Charge 도입 전에 정리한다.

---

## 6. 입력·회피 계약 v0.3

### 6.1 입력 배치 원칙

키 바인딩은 패드 배치를 키보드에 그대로 옮기지 않는다. 참고한 [P의 거짓 키보드·마우스 세팅 문제 사례](https://gall.dcinside.com/mgallery/board/view/?id=liesofp&no=93218)처럼 WASD에서 손을 떼거나 이동 손가락을 과도하게 묶는 조합은 전투 기능이 있어도 사용률을 낮출 수 있다.

v0.3 원칙:

1. 약공격/차지공격은 한 물리 버튼의 Tap/Hold로 시험한다.
2. Tempo와 Response처럼 자주 쓰거나 반응 시간이 중요한 스킬은 직접 입력을 우선한다.
3. modifier 조합은 기본 액션보다 먼저 판정하고, chord가 성립하면 원래 버튼 액션을 차단한다.
4. 같은 버튼을 “즉시 발동 액션”과 “다른 액션의 modifier”로 동시에 사용하지 않는다.
5. 키보드·마우스와 게임패드는 같은 기능 구조를 공유하되 동일한 손가락 동작까지 강제하지 않는다.
6. HUD는 현재 입력 장치에 맞는 키와 item hotkey layer를 별도로 표시한다.

### 6.2 첫 키 바인딩안

| 기능 | 키보드·마우스 | Xbox 게임패드 | v0.3 판단 |
| --- | --- | --- | --- |
| 약공격/차지공격 | LMB Release / Hold Commit | RB Release / Hold Commit | Commit 전 Release는 약공격, Commit 도달은 차지공격 |
| S1 Tempo | RMB | LB | 직접 입력 |
| S2 Area | Shift + LMB | **RT 우선** / LB + RB 비교 후보 | LB를 Tempo와 modifier로 겸용하지 않는 안을 우선 |
| S3 Defensive | Shift + RMB | **LT 우선** / Y 비교 후보 | 반응 지연이 없는 직접 입력 우선 |
| S4 Ultimate | R | **Y 우선** / LB + Y, LT + RT 비교 후보 | Ready일 때만 실행되므로 직접 Y가 가장 단순 |
| 대상 고정 | Q | R3 | 직접 입력 |
| 회피/전력질주 | Space Tap / Hold | B Tap / Hold | Tap Dodge, Hold Sprint 통합 액션 |
| 걷기 | Z Toggle | Left Stick 조금만 기울이기 | R3는 락온이므로 보행 축으로 사용하지 않음 |
| 웅크리기 | C | L3 | Toggle |
| 아이템 슬롯 선택 | CapsLock Tap 또는 CapsLock + Wheel | D-Pad Up/Down | UI에 현재 선택 슬롯 표시 |
| 아이템 사용 | F | X | 선택 슬롯 사용 |
| 아이템 핫키 | 1 / 2 / 3 / 4 | D-Pad Up/Down + X/Y/A/B 후보 | 4개 직접 슬롯. 패드 layer는 후속 검증 |
| 무기 변경 | Tab Tap 또는 Tab + Wheel | D-Pad Left/Right | 좌우로 이전/다음 무기 |
| 상호작용 | E | A | 직접 입력 |

굵게 표시한 게임패드 배치를 첫 수직 단면의 우선안으로 삼는다.

`RB Basic / LB Tempo / RT Area / LT Defensive / Y Ultimate`는 네 스킬을 모두 직접 누를 수 있고, Tempo나 Defensive가 chord 대기 때문에 늦어지지 않는다. S4는 게이지가 준비되기 전에는 실행되지 않으므로 조건부 chord보다 Y 직접 입력이 더 단순하다.

비교 후보의 문제:

- `LB + RB Area`: LB를 누르는 순간 Tempo가 먼저 실행될 수 있다.
- `LB + Y Ultimate`: 같은 이유로 Tempo와 충돌한다.
- `LT + RT Ultimate`: Defensive나 Area가 먼저 실행될 수 있다.
- `Y Defensive`: 직접 반응성은 좋지만 Ultimate를 다시 chord에 넣어야 한다.

키보드의 `Shift + LMB/RMB`는 각각 기본공격과 Tempo보다 chord 판정이 우선해야 한다. Shift가 눌렸다면 LMB의 Tap/Hold Basic과 RMB의 Tempo를 동시에 시작하지 않는다. Shift 자체에는 액션을 연결하지 않고 modifier 상태만 제공한다.

아이템과 무기 입력은 다음 충돌을 구현 전에 해결한다.

- `CapsLock`은 Windows/IME 상태와 포커스에 영향을 줄 수 있으므로 PIE와 패키징 빌드에서 Tap 및 Hold+Wheel 인식 여부를 확인한다.
- D-Pad + Face Button hotkey layer는 A 상호작용, B 회피, X 아이템 사용, Y Ultimate를 반드시 차단해야 한다.
- D-Pad Up/Down을 슬롯 선택과 hotkey modifier로 함께 쓰면 선택이 먼저 이동할 수 있으므로 첫 마일스톤에서는 슬롯 선택만 구현하고 hotkey layer를 뒤로 미룬다.
- D-Pad Left/Right는 v0.3에서 무기 변경이 소유한다. 기존 게임패드 입력 TODO의 상호작용 후보 전환 배치는 폐기하거나 다른 입력으로 옮긴다.

### 6.3 Roll과 Perfect Dodge Step

현재 `MVPlayerDodge`는 이동 입력이 있고 Strafe/Aim 문맥이면 `bUsesStep=true`로 정해 Step row를 고른다. 락온이 Strafe를 켜는 현재 구성에서는 사실상 락온 문맥이 Roll/Step 선택에 관여한다.

v0.3에서는 락온을 회피 모션 선택 조건에서 제거한다.

| 입력·결과 | 목표 모션 |
| --- | --- |
| 방향 입력 + 일반 회피 | 입력 방향 Roll |
| 무방향 입력 + 일반 회피 | Backstep 또는 후방 Roll |
| 방향 입력 + Perfect Dodge | 입력 방향의 짧은 Step |
| 무방향 입력 + Perfect Dodge | 짧은 Backstep |

락온은 캐릭터가 바라볼 기준과 방향 해석만 제공한다. 같은 방향과 타이밍이라면 락온 여부 때문에 Roll과 Step이 달라지지 않는다.

Perfect Dodge는 입력 시점에 적의 애니메이션만 보고 미리 성공 처리하지 않는다. **실제로 피해를 줄 수 있었던 Hit이 짧은 Perfect Dodge Window와 교차했는가**로 판정한다.

권장 첫 계약:

1. Dodge 입력이 수락되면 `DodgeAttemptId`, 시작 시각, 입력 방향을 기록하고 스태미나를 소비한다.
2. 모든 방향 회피는 짧은 공통 LeadIn과 Perfect Dodge Window에서 시작한다. LeadIn부터 이동을 시작하며 정지한 채 결과를 기다리지 않는다.
3. 유효한 적 공격 Contact가 Window 안에 들어오면 피해·그로기·Reaction을 막고 `PerfectDodge` 결과를 만든다.
4. Perfect이면 Roll로 이어지지 않고 짧은 Step section과 빠른 Recovery로 끝낸다.
5. Window 안에 qualifying Hit이 없으면 끊김 없이 일반 Roll section으로 이어진다.
6. 일반 회피의 더 넓은 Damage Immunity 구간에서 피한 공격은 성공 회피지만 Perfect로 승격하지 않는다.
7. Perfect Dodge도 기본 회피와 같은 스태미나를 지불하며 S4 게이지나 자동 반격을 직접 지급하지 않는다.

첫 Perfect Window는 Dodge 시작부터 약 0.10초 동안을 후보로 두되, 실제 공격 Contact 로그를 수집해 조정한다. 입력 장치나 프레임레이트에 따라 판정 폭이 달라지지 않도록 월드 시간과 Hit 결과를 기준으로 기록한다.

이 구조는 하나의 회피가 결과에 따라 시각적으로 갈라져야 하므로, 현재처럼 Action 시작 전에 Chooser가 Roll/Step row를 완전히 결정하는 구조만으로는 부족하다. 공통 Dodge Montage의 section 분기 또는 Perfect 결과를 받은 뒤 Step Recovery로 전환하는 계약이 필요하다.

MDA:

- Mechanics: 락온과 무관한 Roll, Hit 기반의 짧은 Perfect Window, 성공 시 Step 분기.
- Dynamics: 플레이어는 안전한 Roll을 언제든 사용하되 공격을 정확히 읽으면 더 짧은 이동과 Recovery로 반격 기회를 얻는다.
- Aesthetics: 일반 회피의 안정감과 Perfect Dodge의 간결하고 정밀한 숙련감이 모션만 봐도 구분된다.

S3 Response와의 경계:

- Perfect Dodge는 모든 무기가 공유하는 스태미나 기반 숙련 보상이다.
- S3는 별도 사용 제한을 가진 무기별 대응이며 패리, 자동 반격, 특수 위치 이동처럼 더 구체적인 결과를 제공한다.
- S3 회피형은 Perfect Dodge보다 단순히 Window가 넓거나 무적이 긴 상위 호환이 되어서는 안 된다.

---

## 7. 다음 마일스톤 구현 순서

### M1. 적중 결과 수직 단면

- `AttackExecutionId`와 최소 HitConfirm 결과를 추가한다.
- 무적 중 HP·그로기·Reaction·게이지가 모두 막히는지 고정한다.
- 한 공격 실행의 다단·다수 적중 지급 상한을 테스트한다.
- POLICY에 따라 `HitTransactionTrace`를 먼저 붙여 PIE 결과를 수집한다.

완료 기준:

- 동일 실행의 중복 적중을 재현하고 의도한 횟수만 자원을 지급한다.
- DirectDamage와 Immune/Guarded 결과를 로그와 HUD 테스트에서 구분한다.

### M2. 기본공격 약/차지 전환

- enum, 입력 태그, `BasicAttackMap`, Chooser 후보, DataTable row에서 Heavy 의미를 제거한다.
- LMB/RB의 공통 LeadIn에서 Commit 전 Release는 약공격, Commit까지 Hold하면 차지공격으로 분기한다.
- Hold 조기 해제의 Heavy 전환 코드를 약공격 section 전환으로 교체한다.
- Sprint/Dodge Heavy 콘텐츠는 Charge 문맥으로 이전하거나 제거한다.
- 약/차지 및 문맥형 공격의 자원·Impact·게이지 값을 기록한다.

완료 기준:

- 어떤 입력·문맥에서도 Heavy action이 시작되지 않는다.
- 전력질주/회피 뒤 약공격과 차지공격 문맥이 각 1회 정상 소비된다.
- Commit 전 Release는 Hold 시간과 관계없이 항상 약공격을 재생한다.
- Commit 도달 뒤 Release는 차지공격을 약공격으로 되돌리지 않는다.
- Commit 경계 같은 프레임에서도 약/차지 결과가 결정론적으로 고정된다.

### M3. 입력 배치와 Perfect Dodge 수직 단면

- 키보드·마우스 Shift chord와 게임패드 직접 스킬 입력을 별도 IA로 구성한다.
- Tap Dodge / Hold Sprint 통합 액션을 Space와 B에서 같은 규칙으로 검증한다.
- Dodge의 사전 `bUsesStep` 결정을 제거하고 방향 입력 시 락온과 무관하게 Roll을 선택한다.
- Dodge Attempt와 Hit 결과를 연결해 Perfect Window 성공 시 Step Recovery로 분기한다.
- `DodgeTimingTrace`에 AttemptId, InputTime, ContactTime, Window, LockOn, Direction, Outcome을 기록한다.

완료 기준:

- Shift chord가 Basic/Tempo를 함께 실행하지 않는다.
- 게임패드 S1~S4가 직접 입력되고 modifier 선행 액션이 발생하지 않는다.
- 같은 방향·타이밍이면 락온 여부와 무관하게 같은 Roll 결과가 나온다.
- 실제 qualifying Hit이 Perfect Window와 교차했을 때만 Step이 나온다.
- 일반 Roll, Perfect Step, S3 회피형의 비용과 결과가 구분된다.

### M4. 네 슬롯 런타임과 HUD

- Q/R 인덱스 하드코딩을 S1~S4 역할 기반 등록으로 바꾼다.
- `ChainStage`, `AvailableCharges`, `ActiveRechargeRemaining`, `SignatureGauge` 상태를 분리한다.
- HUD에 S1 Charge 수, S2/S3 사용 가능 상태, S4 게이지와 Ready를 표시한다.
- 현재 입력 장치에 맞는 버튼 안내를 표시한다.

완료 기준:

- 네 슬롯이 장착 데이터와 같은 역할로 표시·실행된다.
- S1 Charge와 연계 아이콘이 서로 오인되지 않는다.
- S4가 게이지 100 전에는 시작되지 않는다.

### M5. 카타나 3+1 수직 검증

- S1 Tempo 한 개
- S2 Area 한 개
- S3는 패리형 또는 회피형 중 한 개
- 현재 R 애셋을 S4 Signature 후보로 이전

한 번에 대체 스킬과 추가 무기까지 만들지 않는다. 네 역할의 전투 질문이 실제 플레이에서 분리되는지 먼저 검증한 뒤 sidegrade와 석궁·대형 낫으로 확장한다.

완료 기준:

- 약공격만 쓰는 전투보다 스킬을 섞은 전투에서 상황 해결 방식이 늘어난다.
- S2는 다수전에서 선택되고 1대1 만능 DPS가 되지 않는다.
- S3 성공 후 반격 또는 위치 우위가 명확하며 기본 회피 사용도 남는다.
- S4는 시간 대기가 아니라 적중 행동으로 준비되고 한 전투의 절정에서 사용된다.

---

## 8. 플레이테스트에서 볼 것

| 질문 | 지표 |
| --- | --- |
| S1을 실제로 자주 섞는가? | 최대 Charge 체류 시간, 분당 사용 횟수, 0 Charge 체류 시간, 연속 Commit 간격 |
| S2가 범위 문제를 푸는가? | 사용당 대상 수, 다수전 사용률, 단일 대상 대비 효율 |
| S3가 기본 회피와 구분되는가? | 위협 기회 대비 사용률·성공률, 성공 후 반격률, 기본 회피 사용률 |
| S4가 전투 성과의 결산인가? | 준비까지 걸린 유효 적중 수·시간, 획득원 비율, Ready 후 보존 시간, 전투당 사용 횟수 |
| 약공격이 다시 만능이 되지 않는가? | 공격별 피해·그로기·행동 취소 기여, A1/A2 행동의 실제 발동률 |
| 차지가 큰 빈틈의 답인가? | 차지 적중률, 사용 후 피격률, Armor 관통과 그로기 기여 |
| 한 버튼 Basic이 반응적인가? | Press→Release→약공격 section→적중까지 시간, Commit 경계 오분류율 |
| Perfect Step이 정확한 보상인가? | Dodge Attempt 대비 Perfect 비율, Contact와 Window 차이, 성공 후 반격률 |
| 입력 조합이 오발을 만드는가? | chord와 base action 동시 실행, 잘못된 스킬 발동, 입력 장치별 실패율 |

첫 목표는 사용률을 똑같이 만드는 것이 아니다. **각 행동이 자기 상황에서 선택되고, 다른 행동의 역할까지 먹지 않는지**를 확인하는 것이다.

---

## 9. 이번 마일스톤의 남은 결정

1. S3 첫 수직 단면을 패리형으로 할지 회피형으로 할지 정한다.
2. 카타나 S1의 `MaxCharges`, Charge당 회복 시간, Chain reset 시간과 적중당 회복 단축값을 정한다.
3. S4 게이지의 일반 전투 종료 후 유지 정책과 보스 진입 사전 충전을 허용할지 정한다.
4. 한 버튼 Basic의 `ChargeCommitTime`과 경계 프레임 우선순위를 확정한다.
5. 게임패드 `RT Area / LT Defensive / Y Ultimate` 직접 입력안을 확정할지 정한다.
6. D-Pad item hotkey layer를 첫 입력 마일스톤에 포함할지 뒤로 미룰지 정한다.
7. Perfect Dodge Window 시작값과 Step Recovery 이득을 정한다.
8. 현재 MP를 S1~S3의 공용 자원으로 유지할지, 이번 수직 단면에서는 슬롯 gate만 사용하고 MP 재설계를 미룰지 정한다.

이 항목 외의 무기 확장, 스킬 트리, 다수 sidegrade, 성장 수치는 카타나 3+1 수직 단면 이후로 미룬다.

---

## 10. 구현 근거 위치

- `Source/Maverick/Components/MVCombatComponent.h/.cpp`
  - Heavy/Charge Hold 전환, 문맥형 공격, Q/R 두 슬롯, 체인/쿨다운 상태
- `Source/Maverick/Public/Enum/MVCombatActionTypes.h`
  - Heavy 및 Sprint/Dodge Heavy action 종류
- `Source/Maverick/Public/Tables/MVSkillDataTableColumn.h`
  - 현재 공격 비용·배율·차지·쿨다운·체인 데이터
- `Source/Maverick/Combat/MVAbilityBase.h/.cpp`
  - Notify 시점 비용 소비와 Ability 종료
- `Source/Maverick/Public/Struct/MVHitTypes.h`
  - 현재 Hit 요청/결과에 실행 ID와 결과 태그가 없는 상태
- `Source/Maverick/Combat/MVHitResolverSubsystem.h/.cpp`
  - 현재 피해·그로기·반응 결과 생성과 전파
- `Source/Maverick/Character/PC/Dodge/MVPlayerDodge.h/.cpp`
  - 스태미나 기반 기본 회피, Strafe/Aim에 따른 사전 Roll/Step 선택
- `TODO/gamepad-input-mapping.md`
  - 현재 Q/R chord와 D-Pad 상호작용 후보 배치. v0.3 입력안과 충돌하는 과거 구현 메모
- `Source/Maverick/UI/HUD/MVPlayerSkillHUDWidget.h/.cpp`
  - 현재 Q/R 슬롯, 체인 단계, 시간 쿨다운 표시

v0.3의 설계 판단은 위 코드와 현재 DataTable/Blueprint 참조 상태를 기준으로 했다. 실제 애셋 그래프의 최종 분기와 타격 횟수는 각 마일스톤의 PIE 추적 로그로 다시 확인한다.
