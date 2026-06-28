# Death / Respawn Flow

## 목적

이 문서는 플레이어 HP가 0이 된 뒤 사망 액션, 디졸브, 사망 오버레이, 로딩 초기화, 마지막 저장 위치 부활까지 이어지는 런타임 흐름을 정리한다.

핵심 원칙은 다음과 같다.

- 스탯 컴포넌트는 사망 판정과 이벤트 발행만 담당한다.
- 일반 피격 리액션은 HitReactionComponent가 담당하고, 사망 액션/래그돌/디졸브 cue 같은 actor-local 사망 표현은 DeathComponent가 담당한다.
- ActionComponent는 전달받은 row handle의 몽타주 실행기 역할에 집중한다.
- UI 창은 표시 책임만 갖고, 게임 상태 전환을 직접 수행하지 않는다.
- `UMVWorldStateSubsystem`은 저장되어야 하는 게임 상태와 세이브 슬롯을 소유한다.
- `UMVRespawnSubsystem`은 사망, 로딩, 부활 흐름을 소유한다.
- `UMVQuestSubsystem`은 퀘스트 규칙만 처리하고, 저장이 필요한 변경은 WorldState에 요청한다.

## 책임 분리

사망 흐름은 하나의 컴포넌트가 모두 처리하지 않는다. 각 컴포넌트는 자신이 확정할 수 있는 사실만 확정하고, 다음 단계에는 이벤트나 명시적인 API로 넘긴다.

| 대상 | 소유 책임 | 소유하지 않는 책임 |
| --- | --- | --- |
| `UMVHitResolverSubsystem` | 공격자/피격자/공격 계수로 `FMVResolvedHitData`를 만든다. | HP를 차감하거나 montage를 재생하지 않는다. |
| `AMVCharacterBase` | `OnHitResolved`에서 피해 대상 필터를 확인하고 `OnDamaged`를 브로드캐스트한다. | 피격/사망 정책을 직접 선택하지 않는다. |
| `UMVHitReactionComponent` | non-lethal HitReaction과 lethal KD/AB의 pre-death HitReaction을 선택한다. | death action, dissolve, ragdoll, respawn을 소유하지 않는다. |
| `UMVStatComponent` | HP 차감, lethal 확정, dead state, `FMVDeathContext` 발행을 담당한다. | montage나 UI 전환을 직접 호출하지 않는다. |
| `UMVDeathComponent` | actor-local 사망 표현을 선택하고 시작/디졸브/완료 이벤트를 발행한다. | HP 판정, 피격 리액션 선택, 플레이어 부활/월드 리셋을 처리하지 않는다. |
| `UMVActionComponent` | 전달받은 action row의 montage 실행과 action lifecycle 이벤트를 담당한다. | 어떤 row를 재생할지 정책적으로 선택하지 않는다. |
| `UMVRespawnSubsystem` | 플레이어 DeathComponent 이벤트를 구독해 overlay, loading, world reset, checkpoint respawn을 진행한다. | 개별 actor의 사망 표현 row를 고르지 않는다. |

## 사망 런타임 흐름

1. `UMVStatComponent`가 HP 0 진입을 감지한다.
2. `UMVStatComponent::OnDeathStarted`가 `FMVDeathContext`와 함께 한 번만 발행된다.
3. 입력 게이트가 사망 상태로 전환된다.
   - look 입력은 허용한다.
   - 이동, 액션, 상호작용, 메뉴성 입력은 차단한다.
4. `UMVHitReactionComponent`는 `UMVStatComponent::WouldDieFromHit()`으로 lethal 여부를 예측해 피격 리액션 경로를 고른다.
   - non-lethal hit은 기존 HitReaction을 재생한다.
   - lethal KD/AB hit은 KD/AB HitReaction을 재생한다.
   - lethal standing hit은 HitReaction을 생략한다.
5. `UMVDeathComponent`가 현재 KD/AB `HR_` 액션이 실행 중인지 확인한다.
   - KD/AB HitReaction이 있으면 사망 문맥을 보관하고 handoff notify 또는 액션 종료를 기다린다.
   - KD/AB HitReaction이 없으면 바로 사망 표현을 시작한다.
6. KD/AB HitReaction은 Lying 섹션 진입 직전의 `MV HitReaction Death Handoff` notify에서 보류된 사망 문맥을 확인한다.
   - HP가 0이면 현재 HitReaction을 끝내고 death action으로 전환한다.
   - HP가 0이 아니면 보류된 사망 문맥이 없으므로 기존 Lying 흐름으로 진행한다.
7. HitReaction handoff 또는 death event 즉시 `UMVDeathComponent`가 사망 액션 row handle 또는 ragdoll/immediate 표현 방식을 결정한다.
8. `UMVActionComponent`가 사망 몽타주를 재생한다.
9. `UMVAnimNotify_DeathDissolve`가 `UMVDeathComponent`에 디졸브 cue를 알린다.
10. 플레이어의 디졸브 cue는 `UMVRespawnSubsystem`이 구독해 `UMVUISubsystem::ShowDeathOverlay()`로 연결한다.
11. 사망 표현이 끝나면 `UMVRespawnSubsystem`이 `UMVUISubsystem::ShowLoadingWindow()`를 호출한다.
12. LoadingWindow는 필드 초기화 진행률과 도움말 카드 캐러셀을 표시한다.
13. 필드 초기화가 끝나면 마지막 저장 위치에서 플레이어를 부활시킨다.
14. 부활 몽타주를 재생하고 현재 필드 이름 알림을 표시한다.

## UI 책임 분리

### DeathOverlayWindow

`UMVDeathOverlayWindow`는 사망 알림 전용 창이다.

- 고정 문구 `YOU DIED`를 표시한다.
- 문구는 항상 같으므로 DataTable로 관리하지 않는다.
- 문구는 `NSLOCTEXT`를 사용해 로컬라이징 수집 대상이 되게 한다.
- 이 창은 LoadingWindow를 직접 열지 않는다.
- 최소 표시 시간이 지난 사실만 이벤트로 알릴 수 있다.
- 기본 1초 fade in/out을 사용하고, 최소 표시 시간은 fade in 완료 뒤부터 계산한다.
- look 입력은 막지 않는다.

DeathOverlay가 LoadingWindow를 직접 열면 사망 몽타주 종료 시점과 UI 타이머가 경쟁하게 된다. 따라서 로딩 전환은 반드시 `UMVRespawnSubsystem`이 몽타주 종료 이벤트를 기준으로 처리한다.

## Stat / Death Context

`UMVStatComponent`는 기존 호환 이벤트 `OnDead`를 유지하고, 신규 이벤트 `OnDeathStarted`를 함께 발행한다.

`FMVDeathContext` 포함 값:

| 필드 | 의미 |
| --- | --- |
| `DeadActor` | 사망한 actor |
| `Reason` | HP depletion 또는 MaxHP 감소 |
| `bHasHitData` | 마지막 피격 문맥 존재 여부 |
| `HitData` | killing hit으로 판단된 `FMVResolvedHitData` |

부활 시에는 `UMVStatComponent::ResetDeathState()`를 먼저 호출한 뒤 HP/스태미너/MP를 회복한다.

## Death Component

`UMVDeathComponent`는 캐릭터에 붙는 actor-local 사망 표현 컴포넌트다.

핵심 관점은 사망 판정이 아니라 사망 표현이다. `UMVStatComponent`가 이미 HP 0과 dead state를 확정한 뒤 `FMVDeathContext`를 넘기면, `UMVDeathComponent`는 그 actor를 어떤 방식으로 보이게 할지만 결정한다.

### 입력

- `UMVStatComponent::OnDeathStarted(FMVDeathContext)`: 사망 표현 시작의 유일한 런타임 진입점이다.
- `UMVAnimNotify_HitReactionDeathHandoff`: lethal KD/AB HitReaction이 Lying으로 들어가기 전, 보류된 사망 표현을 시작할 수 있게 한다.
- `UMVAnimNotify_DeathDissolve`: death action 안의 디졸브 시작 시점을 알린다.
- `UMVActionComponent::OnActionEnded`: 보류 중인 HR action 또는 실행 중인 death action의 종료를 감지한다.
- `SetPendingLandingDeathHeight(FallHeight)`: 외부 낙사 판정이 있을 때 LandLight/LandHeavy 선택에 쓸 낙하 높이를 전달한다.
- 캐릭터 movement mode 변화: falling 시작 높이와 최근 착지 높이를 추적해 hit data 없는 낙하 사망을 보조한다.

### 출력

- `OnDeathPresentationStarted`: DeathComponent가 사망 표현 phase를 `Running`으로 전환했음을 알린다.
- `OnDeathDissolveStarted`: death montage notify나 immediate fallback에서 디졸브 cue를 알린다.
- `OnDeathPresentationFinished`: death action 종료, ragdoll 적용, immediate 완료 뒤 표현이 끝났음을 알린다.

이 이벤트는 actor-local 표현 이벤트다. 플레이어 UI, 로딩, 월드 리셋, 체크포인트 부활은 `UMVRespawnSubsystem`이 플레이어의 DeathComponent 이벤트를 구독해 이어간다.

### 사망 표현 선택

`DeathPresentationMode`는 DeathComponent가 어떤 표현 방식을 사용할지 정한다.

- `DeathAction`: death action row를 찾아 `UMVActionComponent`에 실행을 요청한다.
- `Ragdoll`: 캐릭터 movement를 멈추고 mesh physics를 켠 뒤 표현을 완료한다.
- `Immediate`: 별도 montage 없이 dissolve cue와 표현 완료를 바로 발행한다.

death action을 사용할 때 row 선택 우선순위는 다음과 같다.

1. `DeathActionRow`가 지정되어 있으면 그대로 사용한다.
2. `DeathActionTableName`이 지정되어 있으면 해당 테이블을 사용한다.
3. 없으면 `Death_<Character>` 명명 규칙으로 테이블을 고른다.

row pose 선택은 다음 순서로 결정한다.

1. 명시적인 pending landing height가 있으면 `Land_Light` 또는 `Land_Heavy`.
2. hit data 없이 falling 중이거나 최근 착지 직후 사망했고 추적 높이가 유효하면 `Land_Light` 또는 `Land_Heavy`.
3. death hit data가 KnockDown 또는 Airborne이면 `Down_F/B`.
4. 그 외에는 `Stand_F/B`.

facing은 killing hit direction 기준으로 고른다. hit data가 없거나 방향이 비어 있으면 `Front`를 기본값으로 둔다.

### KD/AB 보류 흐름

lethal KD/AB는 HitReactionComponent가 먼저 KD/AB HitReaction을 시작한다. 이후 StatComponent가 HP 0을 확정하면 DeathComponent는 현재 action table이 `HR_`이고 death context의 `HitReactionType`이 KD/AB일 때만 death context를 보류한다.

보류 중인 context는 두 방법 중 먼저 도착한 쪽에서 소비된다.

- KD/AB montage의 `MV HitReaction Death Handoff` notify: Lying 진입 직전 death action으로 전환한다.
- `OnActionEnded`: handoff notify가 없거나 montage가 종료되는 경우의 안전망이다.

standing lethal은 HitReactionComponent가 HitReaction을 생략하므로 DeathComponent가 보류하지 않고 즉시 `Stand_F/B` death action을 시작한다.

### 상태 초기화

`ResetDeathPresentationForRespawn()`은 actor-local 표현 상태만 초기화한다.

- death phase, active/deferred action row, dissolve flag를 초기화한다.
- pending landing height와 falling/landing 추적값을 지운다.
- ragdoll이 적용되어 있으면 physics와 collision profile을 기본값으로 되돌리고 movement mode를 walking으로 복구한다.

`UMVDeathComponent`는 `UMVRespawnSubsystem`을 직접 호출하지 않는다. 플레이어의 월드 리셋, 로딩, 부활은 `UMVRespawnSubsystem`이 플레이어 DeathComponent 이벤트를 구독해 처리한다.

## Death Action Row

`UMVDeathComponent`가 `OnDeathStarted`를 구독해 사망 액션 row를 고른다.

lethal hit의 HitReaction 재생 여부는 `UMVHitReactionComponent`가 `UMVStatComponent::WouldDieFromHit()`으로 예측해 결정한다. lethal KD/AB는 KD/AB HitReaction을 먼저 재생하고, lethal SmallHit/LargeHit/None 같은 standing 사망은 HitReaction을 생략해 death action으로 바로 넘어간다.

KD/AB 몽타주는 본 리액션이 끝나고 Lying 섹션으로 넘어가기 전 프레임에 `MV HitReaction Death Handoff` notify를 배치한다. 이 notify는 보류된 사망 문맥이 있을 때 현재 HitReaction을 끊고 death action을 시작한다. 보류된 사망 문맥이 없으면 아무 동작도 하지 않으므로 생존 피격은 기존처럼 Lying 구간으로 들어간다.

handoff notify가 없는 KD/AB HR 액션에 대해서는 기존 안전망으로 해당 HR row의 `OnActionEnded`를 받은 뒤 death action을 시작한다. KD/AB의 default getup 또는 escape dodge recovery는 dead state에서 차단되어, 사망 상태에서 회복 액션으로 빠지지 않는다.

우선순위:

1. 컴포넌트의 `DeathActionRow` override.
2. `DeathActionTableName` override.
3. 명명 규칙 기반 `Death_<Character>` 테이블.

`DeathActionTableName`은 TableManager manifest의 `TableName` 키 기준이다. 에디터에서 asset 이름인 `DT_Death_P1`을 입력해도 런타임에서는 `Death_P1`로 정규화해 조회한다.

기본 row 명명 규칙:

```text
Death_<Character>_<Pose>_<Facing>_<Index>
Death_<Character>_<LandPose>_<Index>
Death_<Character>_<Index>
```

`Pose`:

- `Stand`: 서 있는 상태에서 사망한다.
- `Down`: KnockDown/Airborne 피격 뒤 누운 상태에서 사망한다.

`Facing`:

- `F`: 전방 피격/정면 사망.
- `B`: 후방 피격/후면 사망.

`LandPose`:

- `Land_Light`: 낮은 낙하 사망.
- `Land_Heavy`: 높은 낙하 사망.

예:

```text
Death_P1_Stand_F_01
Death_P1_Stand_B_01
Death_P1_Down_F_01
Death_P1_Down_B_01
Death_P1_Land_Light_01
Death_P1_Land_Heavy_01
```

몽타주 에셋은 row 이름과 맞춰 다음 이름을 권장한다.

```text
AM_Death_Stand_F
AM_Death_Stand_B
AM_Death_Down_F
AM_Death_Down_B
AM_Death_Land_Light
AM_Death_Land_Heavy
```

호환을 위해 기존 `Death_P1_F_01`, `Death_P1_B_01`, 최종 fallback `Death_P1_01`도 계속 탐색한다.

`UMVDeathComponent` 선택 규칙:

- `SetPendingLandingDeathHeight(FallHeight)`가 호출되어 있으면 낙하 높이로 `Land_Light` 또는 `Land_Heavy`를 고른다.
- pending landing height가 없고 hit data 없이 falling 또는 최근 착지 직후 사망하면 DeathComponent가 추적한 낙하 높이로 `Land_Light` 또는 `Land_Heavy`를 고른다.
- death hit data의 `HitReactionType`이 KnockDown 또는 Airborne이면 `Down_F/B`를 고른다.
- 그 외에는 `Stand_F/B`를 고른다.

사망 몽타주가 끝나면 `UMVDeathComponent`가 ActionComponent의 `OnActionEnded`를 받아 `OnDeathPresentationFinished`를 발행한다.

## Death Input Gate

현재 구현된 차단 지점:

- `AMVCharacterBase::AddMovementInput`은 dead state에서 이동 입력 캐시와 이동 입력 이벤트를 만들지 않는다.
- `AMVCharacterBase::UpdateMovement`는 dead state에서 `MaxAcceleration`을 0으로 둔다.
- `UMVInputManagerComponent::SubmitActionInputById`는 dead state에서 액션 입력 버퍼를 비우고 입력을 거절한다.
- `UMVDeathOverlayWindow`는 move input을 무시하지만 look input은 무시하지 않는다.

메뉴성 입력은 프로젝트의 실제 메뉴 입력 entry point가 확정되면 같은 방식으로 Respawn/Stat 상태를 조회해 막는다.

### LoadingWindow

`UMVLoadingWindow`는 초기화 진행률과 도움말 카드만 표시한다.

- progress bar는 `UMVRespawnSubsystem`이 넘기는 단계 진행률을 반영한다.
- 도움말 카드는 `GameGuide` DataTable에서 로딩 노출이 허용된 항목을 가져온다.
- 상호작용 키 입력은 카드 넘김에만 사용한다.
- 초기화 완료 전에는 창을 닫거나 게임 입력으로 복귀하지 않는다.

## GameGuide 데이터

게임 도움말, 시스템 안내, 전투/탐험 팁처럼 여러 UI에서 재사용 가능한 가이드 텍스트는 DataTable로 관리한다. 로딩 화면의 카드 섹션은 이 데이터의 표시 위치 중 하나이므로, 테이블 이름은 `LoadingCard`가 아니라 데이터 성격을 나타내는 `GameGuide`로 둔다.

`GameGuide`는 특정 도메인의 원본 데이터를 소유하지 않는다. 아이템 설명과 아이템 로어는 추후 `Item` 테이블이 소유하고, 로딩 카드에서 아이템 정보를 보여줘야 할 때는 별도의 노출/큐레이션 구조가 `Item` row를 참조하도록 확장한다.

현재 row 구조는 `FMVGameGuideRow`다.

| 필드 | 의미 |
| --- | --- |
| `GuideId` | 가이드 항목 식별자이자 row key |
| `GuideKind` | GameplayTip, SystemHelp, CombatTip, ExplorationTip, RespawnTip 같은 가이드 종류 |
| `Title` | 카드 제목, `FText` |
| `Body` | 카드 본문, `FText` |
| `ContextTags` | 필드/상황 필터용 GameplayTagContainer |
| `bShowInLoading` | LoadingWindow 카드 후보 포함 여부 |
| `Priority` | 우선 노출 순서 |
| `Weight` | 무작위 선택 가중치 |
| `bEnabled` | 항목 활성 여부 |

초기 데이터는 `MaverickDesign/Json/GameGuide.json`에 둔다.

## 월드 상태와 저장 구조

필드 상태 초기화는 별도 `FieldStateSubsystem`을 만들지 않는다. 대신 저장되어야 하는 사실만 `UMVWorldStateSubsystem`에 남기고, 실제 액터 복원은 이후 해당 액터/컴포넌트가 WorldState를 조회해 적용한다.

### WorldStateSubsystem

`UMVWorldStateSubsystem`은 `UGameInstanceSubsystem`으로 둔다. 맵 전환과 로딩 윈도우를 거쳐도 마지막 저장 지점, 1회성 스폰 소비 상태, 월드 플래그, 퀘스트 진행 상태가 유지되어야 하기 때문이다.

책임:

- 기본 세이브 슬롯 이름과 user index 관리.
- `UMVWorldSaveGame`을 통한 저장/로드.
- 마지막 체크포인트 field/map/transform 저장.
- 1회성 몬스터/아이템 소비 상태 저장.
- 숏컷, 열린 문, 보스 처치 같은 영구 월드 플래그 저장.
- 퀘스트 상태와 목표 카운트 저장.

### RespawnSubsystem

`UMVRespawnSubsystem`도 `UGameInstanceSubsystem`으로 둔다. 사망 뒤 로딩 윈도우, 맵 전환, 플레이어 재스폰까지 같은 흐름을 이어가야 하므로 월드 생명주기보다 GameInstance 생명주기에 맞춘다.

책임:

- 플레이어 사망 이벤트 구독.
- 플레이어 DeathComponent의 표현 시작, 디졸브 cue, 표현 완료 이벤트 구독.
- DeathOverlayWindow와 LoadingWindow 표시 요청.
- 로딩 progress 이벤트 발행.
- 현재 월드의 `MVRespawnResettableInterface` actor에게 사망 리셋 context 전달.
- WorldState의 마지막 체크포인트를 조회해 플레이어 transform과 스탯을 초기화.
- 실제 몬스터/아이템/필드 오브젝트 초기화 방식은 각 actor 도메인 구현에 위임.

### QuestSubsystem

`UMVQuestSubsystem`은 퀘스트 규칙만 처리한다. 저장이 필요한 변경은 직접 SaveGame을 수정하지 않고 `UMVWorldStateSubsystem`에 요청한다.

예상 호출 관계:

```cpp
QuestSubsystem -> WorldStateSubsystem::SetQuestState()
QuestSubsystem -> WorldStateSubsystem::SetQuestObjectiveValue()
QuestSubsystem -> WorldStateSubsystem::CompleteQuest()
```

### 필드 오브젝트 정책

별도 `FieldStateSubsystem`은 만들지 않는다. 현재 월드의 리셋 대상 actor가 `MVRespawnResettableInterface`를 구현하고, `UMVRespawnSubsystem`은 로딩 단계에서 이 actor들에게 `FMVRespawnResetContext`를 전달한다.

인터페이스 계약:

```cpp
UENUM(BlueprintType)
enum class EMVRespawnResetPolicy : uint8
{
    RespawnEveryDeath,
    PersistIfConsumed,
    PersistState,
    TransientOnly
};

GetRespawnResetPolicy()
GetRespawnResetFieldId()
GetRespawnResetObjectId()
HandleRespawnReset(const FMVRespawnResetContext& ResetContext)
```

`PersistIfConsumed` 대상은 `WorldStateSubsystem::IsOneTimeSpawnConsumed(FieldId, ObjectId)` 결과를 `ResetContext.bIsConsumed`로 받는다. 실제 actor를 숨길지, 제거할지, 초기 상태로 복구할지는 해당 actor 구현이 결정한다.

### ResetPolicy 의미

`RespawnEveryDeath`

일반 몬스터, 일반 필드 아이템, 다시 켜져야 하는 장치에 사용한다. 사망 리셋 때 초기 transform, HP, 활성 상태, AI 상태를 복구한다.

`PersistIfConsumed`

1회성 아이템, 1회성 몬스터에 사용한다. 저장 데이터에 소비됨으로 기록되어 있으면 사망 리셋 때 다시 스폰하지 않는다.

`PersistState`

숏컷 문, 보스 처치, 영구 해금 장치에 사용한다. 사망 리셋 때도 저장된 상태를 유지한다.

`TransientOnly`

투사체, 임시 소환물, 지속 이펙트에 사용한다. 리셋 때 제거만 하고 재생성하지 않는다.

## 사망 리셋 단계

`UMVRespawnSubsystem`은 LoadingWindow가 열린 뒤 다음 단계를 순서대로 처리한다.

1. 게임 입력과 전투 처리를 정지한다.
2. `MVRespawnResettableInterface`를 구현한 actor를 수집한다.
3. 마지막 체크포인트 FieldId와 actor FieldId를 기준으로 리셋 대상을 필터링한다.
4. 1회성 대상의 소비 상태를 WorldState 저장 데이터에서 확인해 context로 전달한다.
5. 일반 리셋, 임시 actor 제거, 영구 상태 재적용은 각 actor의 `HandleRespawnReset` 구현에 위임한다.
6. 플레이어를 마지막 저장 위치로 이동 또는 재스폰한다.
7. 플레이어 스탯과 사망 상태를 초기화한다.
8. 부활 몽타주를 재생한다.
9. 현재 필드 이름 알림을 표시한다.
10. 입력 게이트를 일반 플레이 상태로 되돌린다.

각 단계는 LoadingWindow progress bar에 반영할 수 있도록 명시적인 진행률을 발행한다.
