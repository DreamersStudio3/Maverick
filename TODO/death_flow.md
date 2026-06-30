# Death Flow 작업 계획

## 목표

HP가 0이 된 순간부터 사망 액션, 디졸브, 사망 오버레이, 로딩 초기화, 마지막 저장 위치 부활까지 이어지는 흐름을 단계적으로 구현한다.

사망 처리는 여러 시스템을 가로지르므로, 각 단계의 책임을 작게 나누고 이벤트 계약을 먼저 고정한다.

## 현재 확인된 기반

- [x] `UMVStatComponent`는 HP가 0 이하가 될 때 `OnDead` 이벤트를 발행한다.
- [x] `UMVActionComponent`는 `FDataTableRowHandle`을 받아 액션 row의 몽타주를 재생한다.
- [x] `UMVHitReactionComponent`는 피격 상황별 row handle을 선택하고 `UMVActionComponent`에 실행을 위임한다.
- [x] `UMVDeathOverlayWindow`와 `UMVLoadingWindow` 클래스는 존재한다.
- [x] 사망 이벤트에는 마지막 피격 문맥이 포함되어 있다.
- [x] 사망 중 look을 제외한 캐릭터 이동/액션 입력 제한 정책을 추가했다.
- [x] `UMVDeathComponent`가 사망 액션 row 선택과 actor-local 사망 표현을 담당하게 분리했다.
- [x] 사망 디졸브 notify가 `UMVDeathComponent`에 cue를 전달하게 수정했다.
- [x] 사망 액션 row 명명 규칙을 `Stand`, `Down`, `Land_Light`, `Land_Heavy` 체계로 확장했다.
- [x] 낙사 사망 모션 선택 기준을 착지 속도가 아니라 낙하 높이로 잡았다.
- [x] lethal standing hit은 HitReaction을 생략하고 바로 Death_Stand로 넘어가게 했다.
- [x] lethal KD/AB hit은 HitReaction을 먼저 재생하고 DeathComponent가 handoff 뒤 사망 표현을 시작하게 했다.
- [x] KD/AB lethal hit은 Lying 진입 전 `MV HitReaction Death Handoff` notify에서 DeathComponent가 death action으로 전환할 수 있게 했다.
- [x] 사망 흐름의 컴포넌트별 책임과 DeathComponent의 입력/출력/상태 전환 계약을 문서화했다.
- [x] 로딩 초기화 진행률 이벤트와 LoadingWindow progress API가 있다.
- [x] DeathOverlay 표시 cue를 death dissolve notify에서 분리해 `MV Death Overlay` notify와 `OnDeathOverlayRequested` 이벤트로 독립시켰다.
- [x] `MV Death Dissolve` notify에서 mesh material slot별 Dynamic Material Instance를 만들고 `DeathDissolveAmount`를 0에서 1로 구동하게 했다.
- [x] 기존 BP/컴포넌트 기본값에 남은 1.2초 dissolve duration을 런타임에서 3초로 보정했다.
- [x] dissolve material이 낮은 amount에서 완전히 사라지는 경우를 보정하기 위해 duration 끝에 도달할 material amount를 별도 설정으로 분리했다.
- [x] DeathComponent에서는 dissolve cue/reset만 관리하고 실제 DMI/material/mesh hide 처리는 instanced `UMVDeathDissolveEffect` UObject로 분리했다.
- [x] LoadingWindow 전환 시 DeathOverlay를 즉시 deactivate해 respawn 뒤 overlay가 다시 leaf-most로 올라오는 문제를 막았다.
- [x] 부활 완료 시 LoadingWindow를 즉시 deactivate하고 PlayerController 입력 모드를 GameOnly로 복구해 move/look 입력 차단을 해제한다.
- [x] 개발 빌드에서 `MV.UI.LoadingTest.Show`, `Hide`, `Advance` 콘솔 명령으로 로딩 화면과 GameGuide 카드 UI를 고정 테스트할 수 있다.
- [x] 부활 완료 시 UI 시스템을 default HUD만 남는 기본 상태로 재초기화해 사망 전/중 떠 있던 위젯을 제거한다.
- [x] 사망 시작 시 펼쳐져 있던 모든 UI를 fade-out으로 내리고, 이후 DeathOverlay만 새 레이어에 올리게 했다.
- [x] Respawn loading은 DeathOverlay fade-out 완료와 death montage 종료가 모두 확인된 뒤에만 시작한다.
- [x] 사망 중 InteractionDetector가 InteractionPrompt를 즉시 숨기고 dead state에서는 감지를 중단해 프롬프트가 다시 올라오지 않게 했다.
- [x] LoadingWindow 표시 시 HUDLayer를 비워 로딩 화면 뒤에 기본 HUD가 남지 않게 했다.
- [x] HUD hide와 기본 UI reset은 즉시 제거 대신 짧은 fade-out을 거쳐 화면에서 사라지게 했다.
- [x] HUD status bar 최초 값 반영은 RecentLoss shrinking 없이 즉시 스냅해 새 HUD 생성 시 불필요한 손실 애니메이션을 막는다.
- [x] DeathOverlay는 fade in 완료 뒤 1초간 유지하고 같은 fade duration으로 fade out하며, fade out 완료 뒤 respawn gate를 열도록 조정했다.
- [x] PIE 재확인: lethal Airborne/KD hit이 Getup으로 빠지지 않고 handoff 뒤 death presentation으로 이어지는 흐름을 확인했다.
- [x] KD/AB 몽타주의 Lying 섹션 진입 직전에 `MV HitReaction Death Handoff` notify를 배치한다.
- [x] 사망 몽타주마다 dissolve보다 뒤쪽의 의도한 프레임에 `MV Death Overlay` notify를 배치한다.
- [x] LoadingWindow는 GameGuide 테이블에서 로딩 노출 항목을 읽어 도움말 카드를 표시한다.
- [x] LoadingWindow 테스트용 콘솔 명령으로 로딩 화면을 오래 띄우고 카드 전환을 확인할 수 있다.
- [x] LoadingWindow는 `IA_Interact` 입력 액션에 매핑된 키로 가이드 카드를 넘기며, 카드 전환 때 title/body를 fade-out 후 fade-in한다.
- [x] 사망 UI 정리 중 PIE 피격 테스트 패널이 열려 있어도 기존 패널 닫기 경로를 통해 게임 입력 모드와 마우스 커서를 복원한다.
- [x] 사망 흐름 검증용 임시 로그와 별도 debug TODO를 제거했다.
- [ ] 실제 필드 액터 리셋 정책 적용은 아직 없다.

## UI 흐름

사망 UI는 사망 액션의 타이밍을 직접 결정하지 않는다. UI는 표시 책임만 갖고, 사망 진행 상태 전환은 `UMVRespawnSubsystem`이 담당한다.

### DeathOverlayWindow

- [x] 고정 문구 `YOU DIED`를 창 내부에 표시한다.
- [x] 문구는 항상 동일하므로 DataTable로 관리하지 않는다.
- [x] 단, 로컬라이징 수집을 고려해 `NSLOCTEXT` 기반 `FText`로 둔다.
- [x] Death overlay는 이동 입력을 막되 look 입력은 막지 않는다.
- [x] Death overlay는 게임 입력 모드와 마우스 캡처를 요구해 UI 표시 중에도 카메라 look 입력이 분리되지 않게 한다.
- [x] Death overlay는 타이머가 끝났다는 이유만으로 `LoadingWindow`를 직접 열지 않는다.
- [x] Death overlay는 1초 fade in, 1초 hold, 1초 fade out을 사용하고 fade out 완료를 표시 완료 시점으로 알린다.
- [x] 디졸브 시작 notify에서 `UMVDeathComponent::NotifyDeathDissolveStarted()`를 호출한다.
- [x] DeathOverlay 표시 notify에서 `UMVDeathComponent::NotifyDeathOverlayRequested()`를 호출한다.
- [x] 사망 몽타주 종료 이벤트에서 `UMVDeathComponent`가 `OnDeathPresentationFinished`를 발행한다.

### LoadingWindow

- [x] 로딩 진행률을 표시하는 progress bar를 가진다.
- [x] 도움말 카드/캐러셀을 표시한다.
- [x] `IA_Interact` 입력 액션에 매핑된 키로 가이드 카드를 넘길 수 있다.
- [x] 가이드 카드 전환은 0.25초 fade-out 뒤 다음 카드로 교체하고 0.25초 fade-in한다.
- [x] 초기화 완료 전에는 닫히거나 게임 입력으로 복귀하지 않는다.
- [x] 진행률이 100%가 되면 `UMVRespawnSubsystem`이 부활 단계로 넘어갈 수 있다.

## GameGuide 테이블

로딩 카드 내용은 로딩 화면에만 종속된 데이터가 아니라 게임 도움말, 시스템 안내, 전투/탐험 팁처럼 여러 UI에서 재사용할 수 있는 가이드 텍스트다. 따라서 테이블 이름은 표시 위치가 아니라 데이터의 성격을 기준으로 `GameGuide`로 둔다.

아이템 설명과 아이템 로어는 추후 `Item` 도메인 테이블이 소유한다. 로딩 카드에서 아이템 정보를 보여줘야 하는 경우에는 가이드 row에 아이템 텍스트를 복사하지 않고, 별도의 노출/큐레이션 구조가 `Item` row를 참조하도록 확장한다.

### `FMVGameGuideRow`

- `GuideId`: 가이드 항목 식별자이자 row key.
- `GuideKind`: GameplayTip, SystemHelp, CombatTip, ExplorationTip, RespawnTip 같은 가이드 종류.
- `Title`: 제목. `FText`.
- `Body`: 본문. `FText`, multiline.
- `ContextTags`: 특정 필드, 보스, 시스템 문맥에 맞춘 필터 태그.
- `bShowInLoading`: LoadingWindow 카드 후보 포함 여부.
- `Priority`: 같은 조건에서 먼저 보여줄 항목 우선순위.
- `Weight`: 무작위 선택 가중치.
- `bEnabled`: 비활성 항목 필터링.

`MaverickDesign/Json/SheetRecipe.json`에는 `GameGuide: GuideId`를 등록한다.

## 월드 상태와 필드 초기화 구조

필드 상태 초기화는 별도 `FieldStateSubsystem`을 만들지 않고 `UMVWorldStateSubsystem`에 저장되어야 하는 상태만 요청한다. 월드 안의 실제 액터 복원은 이후 각 액터/컴포넌트가 WorldState를 조회해 적용하는 구조로 확장한다.

### WorldStateSubsystem 책임

- [x] 마지막 저장 지점과 부활 transform을 저장한다.
- [x] 세이브 슬롯 저장/로드를 담당한다.
- [x] 1회성 몬스터/아이템 소비 상태를 저장한다.
- [x] 숏컷, 열린 문, 보스 처치 같은 영구 월드 플래그를 저장한다.
- [x] 퀘스트 진행 상태를 저장하되, QuestSubsystem은 WorldState에 요청만 한다.

### QuestSubsystem 책임

- [x] 퀘스트 상태 전환, 목표 카운트 변경, 완료 처리 요청을 받는다.
- [x] 퀘스트 데이터 저장이 필요하면 `UMVWorldStateSubsystem` API를 호출한다.
- [x] 퀘스트 전용 규칙은 QuestSubsystem에 두고, 저장 포맷은 WorldState에 집중한다.

### RespawnSubsystem 책임

- [x] 사망 상태 진입부터 로딩, 부활까지의 상태 머신을 소유한다.
- [x] LoadingWindow progress에 연결될 진행률 이벤트를 발행한다.
- [x] 마지막 체크포인트 조회와 플레이어 부활 처리를 위해 WorldState에 요청한다.
- [x] 사망 로딩 중 `MVRespawnResettableInterface`를 구현한 월드 actor에게 리셋 context를 전달한다.

### Spawn/Reset 정책

- `RespawnEveryDeath`: 일반 몬스터, 일반 소모성 필드 오브젝트. 사망 리셋 때 다시 생성하거나 초기 상태로 복원한다.
- `PersistIfConsumed`: 1회성 아이템, 1회성 몬스터. 이미 획득/처치했다면 다시 생성하지 않는다.
- `PersistState`: 숏컷, 열린 문, 보스 처치 상태처럼 저장 이후 유지되어야 하는 상태.
- `TransientOnly`: 투사체, 임시 소환물, 이펙트. 리셋 때 제거만 하고 재생성하지 않는다.

### `MVRespawnResettableInterface`

필드 상태 전용 서브시스템은 만들지 않는다. 대신 로딩 단계에서 `UMVRespawnSubsystem`이 현재 월드의 actor 중 `MVRespawnResettableInterface`를 구현한 대상만 찾아 `FMVRespawnResetContext`를 전달한다.

- `GetRespawnResetPolicy`: 대상의 리셋 정책을 반환한다.
- `GetRespawnResetFieldId`: 특정 필드에만 속한 대상이면 FieldId를 반환한다. 비워두면 현재 리셋에 항상 참여할 수 있다.
- `GetRespawnResetObjectId`: 1회성 소비 상태 조회가 필요한 대상의 ObjectId를 반환한다.
- `HandleRespawnReset`: 실제 초기화, 비활성화, 제거, 저장 상태 재적용을 actor 도메인에서 수행한다.

`PersistIfConsumed` 대상은 `WorldStateSubsystem::IsOneTimeSpawnConsumed(FieldId, ObjectId)` 결과를 `FMVRespawnResetContext::bIsConsumed`로 받는다. 실제 actor를 숨길지, 제거할지, 초기화할지는 해당 actor 구현이 결정한다.

### 사망 리셋 단계

- [x] 플레이어 입력과 전투 처리를 정지한다.
- [ ] 임시 액터와 투사체를 제거한다.
- [ ] 일반 몬스터/아이템을 초기 상태로 되돌린다.
- [x] 1회성 몬스터/아이템의 소비 상태를 저장 데이터에서 확인한다.
- [ ] 숏컷/문/보스 처치처럼 보존 상태를 재적용한다.
- [x] 플레이어를 마지막 저장 위치로 이동 또는 재스폰한다.
- [x] HP/스태미너/MP 등 스탯을 부활 기준으로 초기화한다.
- [ ] 부활 몽타주를 재생한다.
- [ ] 현재 필드 이름 알림 위젯을 표시했다가 숨긴다.

## 구현 순서

- [x] Death flow 문서화.
- [x] DeathOverlayWindow가 고정 문구를 표시하고 LoadingWindow 전환 책임을 내려놓게 수정한다.
- [x] GameGuide DataTable row 구조와 recipe를 추가한다.
- [x] `UMVStatComponent` 사망 이벤트에 dead state와 사망 문맥을 추가한다.
- [x] 사망 중 look 외 입력 차단 게이트를 추가한다.
- [x] `UMVDeathComponent`가 사망 이벤트로 death row를 선택해 `UMVActionComponent`에 전달하게 한다.
- [x] 사망 디졸브 notify를 추가한다.
- [x] 사망 오버레이 notify를 디졸브 notify에서 분리한다.
- [x] DeathComponent의 dissolve cue를 받아 런타임 dissolve와 respawn 복구를 처리하는 `UMVDeathDissolveEffect` UObject를 추가한다.
- [x] `UMVWorldStateSubsystem` 저장/로드 골격을 추가한다.
- [x] `UMVRespawnSubsystem` 골격을 추가한다.
- [x] `UMVQuestSubsystem`이 WorldState에 요청하는 구조를 추가한다.
- [x] LoadingWindow progress API를 추가한다.
- [x] 사망 로딩 중 월드 actor 리셋을 요청하는 `MVRespawnResettableInterface` 계약을 추가한다.
- [x] `UMVHitReactionComponent`에서 death action과 RespawnSubsystem 직접 호출 책임을 제거한다.
- [x] lethal standing hit은 HitReaction 없이 DeathComponent가 death action을 즉시 시작하게 한다.
- [x] lethal KD/AB hit에서 활성 HitReaction 완료 또는 handoff 뒤 DeathComponent가 death action을 시작하게 한다.
- [x] KD/AB lethal hit에서 Lying 진입 직전 death action으로 넘기는 handoff notify를 추가한다.
- [x] LoadingWindow card UI를 구현한다.
