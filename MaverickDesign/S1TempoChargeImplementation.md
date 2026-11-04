# S1 Tempo 공통 Charge·연속기 구현안 v0.1

> 작성일: 2026-07-26
> 상태: v0.3 구현 검토안
> 상위 문서: [Maverick MDA 전투 구조 v0.3](./MDAFrameworkAnalysis.md)
> 범위: 단일 쿨다운·복수 Charge·연속기를 무기별 코드 분기 없이 처리하는 방법

## 0. 결론

모든 S1을 같은 **Charge 기반 사용권 모델**로 처리한다.

- 단일 쿨다운 S1은 `MaxCharges = 1`이다.
- 연속 사용형 S1은 `MaxCharges > 1`이다.
- `AvailableCharges`는 앞으로 몇 번 더 쓸 수 있는지를 나타낸다.
- `CurrentChainStage`는 다음에 Q1·Q2·Q3 중 어떤 모션이 나갈지를 나타낸다.
- Charge 회복과 연속기 진행은 서로 독립적으로 계산한다.
- 무기별 차이는 조건문이 아니라 데이터로 설정한다.
- S1~S4는 같은 슬롯 실행 절차를 공유하고, 사용 가능 조건만 정책으로 구분한다.

`2/2 = Q1`, `1/2 = Q2`, `0/2 = Q3`처럼 Charge 수와 연속기 단계를 연결하지 않는다. `0 Charge`는 Q3가 아니라 **현재 사용할 수 없음**을 뜻한다.

---

## 1. 공통 모델

### 1.1 용어

| 이름 | 의미 |
| --- | --- |
| `SkillRole` | Tempo, Area, Response, Signature의 고정 역할 |
| `AvailabilityPolicy` | 슬롯을 다시 사용할 수 있게 만드는 규칙 |
| `AvailableCharges` | 현재 남은 스킬 사용권 |
| `MaxCharges` | 저장 가능한 최대 사용권 |
| `ActiveRechargeRemaining` | 다음 Charge 하나까지 남은 시간 |
| `CurrentChainStage` | 다음 실행에서 사용할 Q1·Q2·Q3 단계 |
| `ChainResetRemaining` | Q1로 초기화되기까지 남은 시간 |
| `SignatureGauge` | S4 전용 준비도 |
| `ActionCost` | 스태미나·MP 등의 시작 비용 |

여기서 S1의 `Charge`는 **차지공격의 Hold Charge가 아니라 스킬 사용권**이다. 구현과 HUD에서는 모호한 `Stack` 대신 `AvailableCharges`라고 부른다.

### 1.2 네 슬롯이 공유하는 절차

`입력 요청 → 슬롯 조회 → 사용 가능 검사 → 사용권 예약 → Stage 선택 → 액션 시작 → Commit → 자원 소비 → 적중 결산 → Stage·회복 갱신 → HUD 통지`

| 슬롯 | 역할 | 기본 Availability 정책 |
| --- | --- | --- |
| S1 | Tempo | `RechargeCharges`, `MaxCharges` 1 이상 |
| S2 | Area | `RechargeCharges`, 보통 `MaxCharges = 1` |
| S3 | Response | `RechargeCharges`, 보통 `MaxCharges = 1` |
| S4 | Signature | `SignatureGauge`, 게이지 100에서 Ready |

S2와 S3의 일반 쿨다운도 내부적으로는 `MaxCharges = 1`인 모델이다. S4는 게이지를 gate로 쓰지만 입력 검사, 예약, Commit, 취소 반환, 액션 시작, HUD 통지의 생명주기는 공유한다.

### 1.3 권장 데이터

```cpp
enum class EMVSkillAvailabilityPolicy : uint8
{
    RechargeCharges,
    SignatureGauge
};

enum class EMVSkillChainAdvancePolicy : uint8
{
    OnCommit,
    OnValidHit
};

struct FMVSkillUsageConfig
{
    EMVSkillRole SkillRole;
    EMVSkillAvailabilityPolicy AvailabilityPolicy;

    int32 MaxCharges = 1;
    float RechargeSecondsPerCharge = 0.0f;
    bool bSequentialRecharge = true;

    float BasicHitRechargeReduction = 0.0f;
    bool bAllowSelfHitRechargeReduction = false;

    EMVSkillChainAdvancePolicy ChainAdvancePolicy;
    float ChainResetSeconds = 0.0f;
};
```

연속기 Stage 수는 `MaxCharges`에서 계산하지 않고 Stage row 목록에서 파생한다.

```cpp
struct FMVSkillSlotDefinition
{
    FMVSkillUsageConfig Usage;
    TArray<FName> StageRowNames;
    TArray<TObjectPtr<UMVAbilityBase>> StageAbilities;
};
```

`UsageConfig`는 슬롯 전체 규칙이다. Q1·Q2·Q3 row마다 중복 저장하지 않는다. 첫 구현에서는 시작 row의 설정을 `BuildSkillEntryFromRowHandle()`이 읽어 Definition에 복사하고, 장기적으로 필요할 때 장착 스킬 DataAsset 분리를 검토한다.

### 1.4 권장 런타임과 UI 투영

```cpp
struct FMVSkillSlotRuntime
{
    int32 AvailableCharges = 0;
    int32 ReservedCharges = 0;
    float ActiveRechargeRemaining = 0.0f;

    int32 CurrentChainStageIndex = 0;
    float ChainResetRemaining = 0.0f;

    float SignatureGauge = 0.0f;
    bool bSignatureReserved = false;
};
```

```cpp
struct FMVSkillSlotViewState
{
    bool bAvailable = false;
    int32 AvailableCharges = 0;
    int32 MaxCharges = 0;
    float NextChargeRemaining = 0.0f;

    int32 CurrentChainStageIndex = 0;
    int32 ChainStageCount = 0;
    float ChainResetRemaining = 0.0f;

    float SignatureGauge = 0.0f;
    bool bSignatureReady = false;
};
```

정적 Definition, 변하는 Runtime, 읽기 전용 HUD ViewState를 분리한다. 현재 `ActiveStackIndex`처럼 Stage와 아이콘 인덱스가 사용권으로 오인되는 이름은 재사용하지 않는다.

---

## 2. 동작 규칙

### 2.1 단일 쿨다운

```text
MaxCharges = 1
RechargeSecondsPerCharge = 6
```

1/1에서 Commit하면 0/1이 되고, 6초 뒤 1/1로 돌아온다. 별도의 `SingleCooldown` 코드는 필요하지 않다.

### 2.2 복수 Charge의 순차 회복

```text
MaxCharges = 3
RechargeSecondsPerCharge = 4
bSequentialRecharge = true
```

| 시점 | 상태 | 의미 |
| ---: | --- | --- |
| 0초 | 3/3 → 0/3 | 세 번 연속 Commit |
| 4초 | 1/3 | 한 번 다시 사용 가능 |
| 8초 | 2/3 | 두 번째 Charge 회복 |
| 12초 | 3/3 | 완전 충전 |

“총 쿨다운 12초”는 12초 동안 봉인된다는 뜻이 아니라 **0/3에서 3/3까지 완전히 채우는 시간**이다. 4초부터 다시 한 번 사용할 수 있다.

Charge를 간격을 두고 쓰면 회복과 사용이 겹친다.

| 시점 | 행동과 결과 |
| ---: | --- |
| 0초 | Q1 Commit → 2/3, 첫 회복 시작 |
| 3초 | Q2 Commit → 1/3, 기존 타이머 계속 |
| 4초 | 첫 Charge 회복 → 2/3 |
| 6초 | Q3 Commit → 1/3 |

이 구조가 “몰아 쓰면 긴 완전 회복, 섞어 쓰면 몇 초 간격으로 템포 유지”를 만든다.

### 2.3 회복 타이머

순차 회복에서는 다음 Charge용 타이머 하나만 활성화한다.

- Commit으로 첫 빈자리가 생기면 타이머를 시작한다.
- 타이머가 끝나면 `AvailableCharges += 1` 한다.
- 아직 Charge가 비어 있으면 다음 타이머를 시작한다.
- 최대 Charge가 되면 타이머를 멈춘다.
- 큰 `DeltaTime`에도 여러 Charge 완료를 빠뜨리지 않게 반복 처리한다.

최대 Charge에서 기다려도 보유 Charge가 사라지지는 않는다. 다만 빈자리가 없어 회복이 진행되지 않고 미래 쿨다운을 미리 저장하지도 않는다. S1을 써야 회복 순환이 시작된다.

### 2.4 Charge와 연속기

```text
AvailableCharges: 지금 몇 번 더 실행할 수 있는가?
CurrentChainStage: 다음 실행은 어떤 모션인가?
```

| Charge | Stage | 다음 입력 |
| --- | --- | --- |
| 3/3 | Q1 | Q1 실행 |
| 2/3 | Q2 | Q2 실행 |
| 1/3 | Q3 | Q3 실행 |
| 1/3 | Q1 | reset 뒤 Q1 실행 |
| 0/3 | Q3 | Charge 부족으로 실행 불가 |

기본 연속기 진행은 `OnCommit`으로 둔다.

- Q1 Commit 후 Q2, Q2 Commit 후 Q3, Q3 Commit 후 Q1로 간다.
- Commit 전 취소는 Stage를 넘기지 않는다.
- Commit 후 빗나가도 Stage는 진행된다.
- 적중 연계가 핵심인 특수 S1만 `OnValidHit`을 데이터로 선택한다.
- 마지막 Commit 뒤 `ChainResetSeconds`가 지나면 Q1로 돌아간다.
- Charge가 0인 동안에도 reset 시간은 흐른다.

3초 간격의 Q1→Q2→Q3을 허용하려면 `ChainResetSeconds`를 3초보다 조금 긴 3.5~4초부터 시험한다.

### 2.5 Commit과 소비

1. `CanActivateSlot()`이 Charge·게이지·ActionCost·현재 액션 상태를 검사한다.
2. 통과하면 사용권 하나를 예약하고 Stage 액션을 시작한다.
3. 액션 시작 실패 또는 Commit 전 취소면 예약을 반환한다.
4. Commit에서 예약을 실제 소비로 확정하고 회복을 시작한다.
5. Commit 후 빗나가거나 중단돼도 Charge를 돌려주지 않는다.
6. Stage는 설정된 진행 정책에 따라 갱신한다.

예약 상태는 Commit 전 짧은 구간에 동일 Charge가 중복 사용되는 것을 막는다. S4도 같은 흐름에서 Ready 게이지를 예약하고, Commit 전 취소에는 복구하며 Commit 후에는 100을 소비한다.

### 2.6 기본공격 적중 회복

첫 프로토타입에서는 약공격·차지공격의 유효 적중이 현재 회복 중인 다음 Charge를 실행당 1초 줄인다.

```text
BasicHitRechargeReduction = 1.0
bAllowSelfHitRechargeReduction = false
```

- 공용 최종 `HitConfirm`이 인정한 기본공격만 적용한다.
- 전력질주·회피 문맥의 약/차지도 적용한다.
- 같은 `AttackExecutionId`의 다단·다수 적중은 한 번만 적용한다.
- S1 자체 적중은 기본값으로 자기 회복을 줄이지 않는다.
- 빠진 Charge가 없으면 아무 효과가 없다.
- 현재 활성 타이머만 줄이고 초과 단축은 다음 Charge에 넘기지 않는다.

남은 시간이 0.4초일 때 1초 단축이 들어오면 Charge 하나만 즉시 회복하고 초과 0.6초는 버린다. 한 번의 적중이 여러 Charge를 연쇄 복구하는 것을 막기 위한 규칙이다.

---

## 3. 무기·슬롯별 설정 예시

| 예시 | Policy | MaxCharges | 회복 | Stage | Reset | 결과 |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| 단발 S1 | RechargeCharges | 1 | 6초 | 1 | 0초 | 기존 단일 쿨다운 |
| 저장형 S1 | RechargeCharges | 2 | 5초 | 1 | 0초 | 같은 기술을 최대 2회 저장 |
| Q1→Q2→Q3 S1 | RechargeCharges | 3 | 4초 | 3 | 3.5초 | 몰아 쓰기와 간격 사용 |
| S2 범위기 | RechargeCharges | 1 | 10초 | 1 | 0초 | 일반 범위기 쿨다운 |
| S3 받아치기 | RechargeCharges | 1 | 8초 | 1 | 0초 | 반응형 스킬 제한 |
| S4 일격기 | SignatureGauge | - | - | 1 | 0초 | 게이지 100에서만 사용 |

공통 코드에서 `WeaponStyle == Katana` 같은 분기로 사용 모델을 고르지 않는다. 무기별 차이는 이 데이터와 Stage row·Ability 구성으로 만든다.

---

## 4. HUD 계약

- S1 Charge는 pip 또는 `2/3` 숫자로 표시한다.
- 쿨다운 overlay는 **다음 Charge 하나**의 남은 시간을 표시한다.
- Q1·Q2·Q3을 보여줄 필요가 있다면 Charge와 별도 표식으로 표시한다.
- S2·S3는 `MaxCharges = 1`이므로 기존 일반 쿨다운처럼 보이게 할 수 있다.
- S4는 Charge 대신 0~100 게이지와 Ready 상태를 표시한다.
- UI는 Runtime을 수정하지 않고 `FMVSkillSlotViewState`만 읽는다.

---

## 5. 현재 구현과 변경 방향

| 현재 위치 | 현재 상태 | 변경 방향 |
| --- | --- | --- |
| `FMVSkillEntry` | Ability, row, 메인 쿨다운, 체인 상태가 혼재 | Definition과 Runtime 책임 분리 |
| `TrySkill()` | 체인 활성 여부와 메인 쿨다운을 직접 분기 | `CanActivate → Reserve → Start → Commit` 공통화 |
| `ResetSkillMap()` | Q/R 두 슬롯 등록 | S1~S4 역할 기반 등록 |
| `FMVSkillDataTableColumn` | 쿨다운과 체인 링크 중심 | 슬롯 단위 Usage 설정 추가 |
| `FMVSkillSlotRuntimeState` | `ActiveStackIndex`가 Stage·아이콘 인덱스 | Charge, Stage, 다음 회복 시간 분리 |
| `MVPlayerSkillHUDWidget` | Q/R과 시간 쿨다운 중심 | S1~S4 정책별 ViewState 표시 |

`FMVSkillEntry`를 한 번에 없앨 필요는 없다. 먼저 `UsageConfig`와 `Runtime` 그룹을 추가하고 호출부를 공통 함수로 옮긴 뒤, 사용하지 않는 기존 필드를 제거한다.

### 구현 순서

1. **데이터 분리**
   - `SkillRole`, `AvailabilityPolicy`, `ChainAdvancePolicy`, `UsageConfig` 추가
   - 단일형과 3 Charge형을 데이터만으로 표현
2. **Charge 런타임**
   - Charge 예약, 순차 회복, 큰 `DeltaTime` 처리
   - `MaxCharges = 1`과 3이 같은 경로로 동작
3. **Commit 통합**
   - 입력 검사, 예약, Commit 소비, 취소 반환 공통화
   - S4 게이지도 같은 생명주기에 연결
4. **Stage 독립**
   - Charge와 Stage 분리, 진행 정책과 reset 구현
5. **적중·HUD 연결**
   - `AttackExecutionId`당 기본공격 회복 단축 1회
   - Charge, Stage, 다음 회복, S4 게이지를 별도 표시
6. **수직 단면**
   - 단발형 S1 하나와 3단 Charge형 S1 하나를 같은 런타임에서 검증

---

## 6. 완료 기준

| 시나리오 | 기대 결과 |
| --- | --- |
| `MaxCharges = 1` Commit | 0/1, 정해진 시간 뒤 1/1 |
| 3/3에서 세 번 연속 Commit | 즉시 0/3, 4·8·12초에 순차 회복 |
| Commit 전 취소 | Charge와 Stage 유지 |
| Commit 후 빗나감 | Charge 소비, 회복 시작, Stage 진행 |
| 0 Charge 입력 | 액션·비용 소비 없이 실패 |
| 회복 중 약/차지 유효 적중 | 활성 타이머만 실행당 1초 단축 |
| 한 공격으로 여러 대상 적중 | `AttackExecutionId`당 한 번만 단축 |
| S1 자체 적중 | 기본 설정에서는 단축 없음 |
| 0.4초 남았을 때 1초 단축 | Charge 하나 회복, 초과분 폐기 |
| Stage reset 시간 만료 | 다음 입력은 Q1 |
| 최대 Charge 장시간 대기 | 숨은 회복 시간 누적 없음 |
| S4 Commit 전 취소/후 빗나감 | 취소 시 복구, Commit 후 소비 |

추적 로그는 `SkillUsageTrace`에 슬롯 역할, 스킬 ID, 정책, 현재/최대 Charge, 예약 수, 다음 회복 시간, Stage, 이벤트, `AttackExecutionId`를 기록한다.

플레이테스트에서는 분당 S1 사용 횟수, 최대/0 Charge 체류 시간, 연속 Commit 간격, 한 번에 소비한 Charge 수, 기본공격 회복 단축 기여를 본다.

---

## 7. 첫 수직 단면에서 정할 값

1. 카타나 S1의 `MaxCharges`
2. `RechargeSecondsPerCharge`
3. 3단형의 `ChainResetSeconds`
4. 기본공격 실행당 1초 단축값
5. 기본 `OnCommit`과 일부 `OnValidHit` 적용 범위

구조상 확정할 것은 **단일 쿨다운과 복수 Charge를 별도로 구현하지 않는 것**, **Charge와 ChainStage를 분리하는 것**, **무기 차이를 데이터로 관리하는 것**이다. Charge 수와 시간은 무기별 밸런스 값으로 남긴다.
