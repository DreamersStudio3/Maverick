# Death / Respawn Flow

## 목적

이 문서는 플레이어 HP가 0이 된 뒤 사망 액션, 디졸브, 사망 오버레이, 로딩 초기화, 마지막 저장 위치 부활까지 이어지는 런타임 흐름을 정리한다.

핵심 원칙은 다음과 같다.

- 스탯 컴포넌트는 사망 판정과 이벤트 발행만 담당한다.
- 액션 선택 정책은 HitReaction/Respawn 같은 도메인 컴포넌트가 담당한다.
- ActionComponent는 전달받은 row handle의 몽타주 실행기 역할에 집중한다.
- UI 창은 표시 책임만 갖고, 게임 상태 전환을 직접 수행하지 않는다.
- `UMVWorldStateSubsystem`은 저장되어야 하는 게임 상태와 세이브 슬롯을 소유한다.
- `UMVRespawnSubsystem`은 사망, 로딩, 부활 흐름을 소유한다.
- `UMVQuestSubsystem`은 퀘스트 규칙만 처리하고, 저장이 필요한 변경은 WorldState에 요청한다.

## 사망 런타임 흐름

1. `UMVStatComponent`가 HP 0 진입을 감지한다.
2. `UMVStatComponent::OnDeathStarted`가 `FMVDeathContext`와 함께 한 번만 발행된다.
3. 입력 게이트가 사망 상태로 전환된다.
   - look 입력은 허용한다.
   - 이동, 액션, 상호작용, 메뉴성 입력은 차단한다.
4. HitReaction/Respawn 계열 도메인이 사망 액션 row handle을 결정한다.
5. `UMVActionComponent`가 사망 몽타주를 재생한다.
6. `UMVAnimNotify_DeathDissolve`가 디졸브 시작을 알린다.
7. 디졸브 시작과 동시에 `UMVUISubsystem::ShowDeathOverlay()`가 호출된다.
8. 사망 몽타주가 끝나면 `UMVRespawnSubsystem`이 `UMVUISubsystem::ShowLoadingWindow()`를 호출한다.
9. LoadingWindow는 필드 초기화 진행률과 도움말 카드 캐러셀을 표시한다.
10. 필드 초기화가 끝나면 마지막 저장 위치에서 플레이어를 부활시킨다.
11. 부활 몽타주를 재생하고 현재 필드 이름 알림을 표시한다.

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

## Death Action Row

`UMVHitReactionComponent`가 `OnDeathStarted`를 구독해 사망 액션 row를 고른다.

우선순위:

1. 컴포넌트의 `DeathActionRow` override.
2. `DeathActionTableName` override.
3. 명명 규칙 기반 `Death_<Character>` 테이블.

기본 row 명명 규칙:

```text
Death_<Character>_<Direction>_<Index>
Death_<Character>_<Index>
```

예: `Death_P1_F_01`, fallback `Death_P1_01`.

사망 몽타주가 끝나면 `UMVHitReactionComponent`가 ActionComponent의 `OnActionEnded`를 받아 `UMVRespawnSubsystem::NotifyDeathMontageEnded()`를 호출한다.

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
- 디졸브 notify, 사망 몽타주 종료 notify가 호출할 진입점 제공.
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
