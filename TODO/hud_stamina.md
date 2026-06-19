# HUD / Stamina TODO

플레이어 기본 HUD와 전력질주 스태미나 소모 로직 작업 항목을 관리한다.

## 기본 HUD

- [x] C++ 기본 `MVMainHUDWidget`에서 HP, Stamina, MP 표시 영역을 생성한다.
- [x] `MVPlayerStatusWidget`이 스탯 컴포넌트 변경 이벤트를 통해 세 막대를 갱신한다.
- [x] HUD용 WBP가 없어도 기본 HUD가 화면에 보이도록 네이티브 레이아웃을 제공한다.
- [x] PlayerStatus 기본 막대 크기를 HP > MP > Stamina 순서로 차등 적용한다.
- [x] StatusBar의 LabelText/ValueText 표시 여부를 C++과 WBP에서 제어할 수 있게 한다.
- [x] PlayerStatus 기본 슬롯 비율을 HP:MP:Stamina = 1.5:1:1 Fill 구조로 맞춘다.
- [x] StatusBar 메인 수치는 보간 없이 즉시 반영하고, 큰 즉시 소모량은 노란색 손실 바로 표시한다.
- [x] PlayerStatus bar 폭을 Max 스탯 기준으로 계산해 레벨업에 따라 늘어날 수 있게 한다.
- [x] 손실 발생 시점부터 1초 뒤 RecentLossBar 감소가 시작되도록 액션 종료 이벤트와 분리한다.
- [x] RecentLossBar 감소 속도를 기본 스태미나 회복량 35 units/sec 기준으로 선형 처리한다.
- [x] RecentLossBar가 이미 표시 중이면 추가 손실이 발생해도 hold 타이머를 갱신하지 않도록 한다.
- [ ] WBP_StatusBar 안에 `BarSizeBox`와 `Bar` 이름을 맞춰 HP/MP/Stamina별 크기 적용 여부를 확인한다.
- [ ] WBP_StatusBar에 노란 손실 표시가 필요하면 `RecentLossBar` ProgressBar를 `Bar` 뒤에 배치한다.

## 전력질주 스태미나

- [x] 전력질주 초당 스태미나 소모량은 `PlayerStat`에 두지 않고 캐릭터 임시 fallback 값으로 분리한다.
- [x] 액션 식별용 `ActionIndex` 테이블과 수치 관리용 `ActionStat` 테이블을 분리한다.
- [x] `ActionIndex` Sprint row가 `ActionStat` Sprint row를 가리키도록 데이터 구조를 잡는다.
- [x] `AMVCharacterBase`가 `ActionIndex` -> `ActionStat` 순서로 전력질주 비용/재시작 기준을 캐시한다.
- [x] `AMVCharacterBase` 전력질주 상태에서 스태미나를 소모하고 지연 후 회복한다.
- [x] 스태미나가 고갈되면 일정량 회복 전까지 전력질주를 막는다.
- [x] 스태미나 고갈 후 재개 기준을 `ActionStat.SprintRestartStaminaPercent = 70`으로 관리한다.
- [x] `/Game/Table` DataTable 에셋 재생성 후 `ActionIndex`/`ActionStat` manifest 포함 여부를 확인한다.
- [x] 스태미나 회복 지연 사용 여부를 `bUseStaminaRecoveryDelay` 변수로 분리하고 기본값을 끈다.
- [x] ProjectBA 회피 액션 데이터를 참고해 `Dodge` 액션의 즉시 스태미나 소모 진입점을 추가한다.
- [x] 회피 스태미너 소모 후 몽타주 종료 알림 전까지 스태미너 회복과 노란 손실바 감소를 잠근다.
- [x] 액션 중 회복 pause를 `BeginActionStatRecoveryPause` / `EndActionStatRecoveryPause` 공통 API로 일반화한다.
- [x] MP 회복도 캐릭터의 공통 회복 틱에 연결하고 액션 회복 pause를 따르게 한다.
- [x] `MVActionComponent`로 액션 테이블 조회, 비용 소모, 회복 pause, 몽타주 종료 처리를 분리한다.
- [x] 구르기/스태미나 전용 호환 API를 제거하고 액션 시작 API를 `ActionComponent.TryStartAction(ActionId)`로 단일화한다.
- [x] `ActionIndex.AnimationChooserTable`을 통해 Chooser Table 결과 몽타주를 평가하도록 연결한다.
- [x] raw 액션 ID를 `MVActionIds`/`ACTIONID_*` 상수와 BP용 `EMVActionId` enum으로 중앙화한다.
- [x] BP에서 ID/enum 선택 없이 회피를 실행할 수 있는 `TryStartDodge` 노드를 추가한다.
- [x] Dodge Chooser가 직접 조건을 관리하도록 C++ `DodgeVariant` 결정을 제거하고 `bHasDodgeMovementInput`/`CharacterInputState.WantsToStrafe` 원천 조건을 사용한다.
- [x] Dodge Chooser가 락온 회피를 8방향으로 나눌 때 `LocomotionDirection`을 방향 조건으로 사용하도록 정리한다.
- [x] 몽타주 NotifyState 기반으로 이동입력 차단, 무적, 입력 버퍼 구간을 `ActionComponent` 상태에 연결한다.
- [ ] 공격/회피/가드가 `ActionComponent.TryStartAction(ActionId)`를 통해 `ActionIndex`로 액션을 찾고 `ActionStat`으로 비용, 배율, 피격유형을 읽도록 연결한다.
- [ ] BP 회피 실행 경로를 `TryStartAction(Dodge)` 호출로 교체하고, `ActionIndex.AnimationChooserTable`에 회피 Chooser Table 경로를 채운다.
- [ ] Chooser Table 에셋에서 결과 타입을 `AnimMontage`로 설정하고 조건별 애니메이션 선택 로직을 연결한다.
- [ ] 액션 테이블 에셋 로드가 안정화되면 `FallbackSprintStaminaCost` fallback을 제거하거나 디버그 전용 값으로 낮춘다.
- [ ] 에디터 Live Coding 종료 후 `/Game/Table` DataTable 에셋을 다시 생성해 Dodge 변경분을 반영한다.

## 검증

- [x] C++ 빌드 또는 가능한 정적 검증을 수행한다.
- [x] `MaverickEditor Win64 Development` UHT 리플렉션 코드 생성을 확인한다.
- [x] 변경된 `MVActionComponent.cpp`, `MVCharacterBase.cpp`, `MVStatComponent.cpp`를 UBT response file 기반으로 단위 컴파일한다.
- [x] Live Coding 종료 후 전체 `MaverickEditor Win64 Development` 빌드를 다시 실행한다.
