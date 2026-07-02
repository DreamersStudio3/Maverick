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

- [x] 전력질주 초당 스태미나 소모량은 단일 `DT_Sprint` 또는 직접 지정된 `FDataTableRowHandle`의 `FMVSprintActionRow`에서 읽는다.
- [x] `AMVPlayerCharacter`가 Sprint row handle을 우선 사용하고, 없으면 `Sprint` / `Sprint_{Character}_01` fallback row에서 전력질주 비용/재시작 기준을 캐시한다.
- [x] `AMVPlayerCharacter` 전력질주 상태에서 스태미나를 소모하고 지연 후 회복한다.
- [x] 스태미나가 고갈되면 일정량 회복 전까지 전력질주를 막는다.
- [x] 스태미나 고갈 후 재개 기준을 `FMVSprintActionRow.SprintRestartStaminaPercent = 70`으로 관리한다.
- [x] 스태미나 회복 지연 사용 여부를 `bUseStaminaRecoveryDelay` 변수로 분리하고 기본값을 끈다.
- [x] ProjectBA 회피 액션 데이터를 참고해 `Dodge` 액션의 즉시 스태미나 소모 진입점을 추가한다.
- [x] 회피 스태미너 소모 후 몽타주 종료 알림 전까지 스태미너 회복과 노란 손실바 감소를 잠근다.
- [x] 액션 중 회복 pause를 `BeginRecoverableStatRecoveryPause` / `EndRecoverableStatRecoveryPause` 공통 API로 일반화한다.
- [x] MP 회복도 캐릭터의 공통 회복 틱에 연결하고 액션 회복 pause를 따르게 한다.
- [x] `MVActionComponent`는 선택/비용 소모를 하지 않고, 전달받은 `ActionTableName + ActionRowName` 실행만 담당한다.
- [x] Dodge 입력은 `InputManagerComponent.SubmitActionInput(Dodge)` 이벤트를 통해 `PlayerCharacter.Dodge` 서브모듈이 처리한다.
- [x] Dodge는 `CHT_Dodge`가 출력한 `FMVDodgeActionRowHandle` 또는 `Dodge_{Character}` 명명 규칙 fallback으로 `FMVDodgeActionRow`를 찾고 비용/launch를 직접 처리한다.
- [x] 입력 의도 ID를 `MVActionIds`/`ACTIONID_*` 상수와 BP용 `EMVActionId` enum으로 중앙화한다.
- [x] BP 회피 입력은 `InputManagerComponent.SubmitActionInput(Dodge)`를 통해 도메인 컴포넌트에 전달한다.
- [x] Dodge Chooser가 직접 조건을 관리하도록 C++ 방향 variant 결정을 제거하고 `bHasDodgeMovementInput`/`CharacterInputState.WantsToStrafe` 원천 조건을 사용한다.
- [x] Dodge Chooser가 락온 회피를 8방향으로 나눌 때 `LocomotionDirection`을 방향 조건으로 사용하도록 정리한다.
- [x] 몽타주 NotifyState 기반으로 이동입력 차단, 무적, 입력 버퍼 구간을 `ActionComponent` 상태에 연결한다.
- [x] `GenerateDataTables` 성공 직후 `MVTableManager` 캐시를 reload해 새 `CharacterIndex` 테이블이 PIE에 즉시 반영되도록 한다.
- [ ] `Content/Table/Sprint`에 `FMVSprintActionRow` 기반 `DT_Sprint` row `Sprint_P1_01`을 생성하고 manifest를 refresh한다.
- [ ] `Content/Table/Dodge/P1`에 `FMVDodgeActionRow` 기반 `DT_Dodge_P1` rows를 생성하고 `CHT_Dodge` Output Struct Column의 `FMVDodgeActionRowHandle`로 연결한다.
- [ ] BP 회피 입력 경로는 직접 몽타주/API 호출 대신 `InputManagerComponent.SubmitActionInput(Dodge)`만 호출하도록 정리한다.
- [ ] Combat 쪽 공격/가드도 각 도메인 Chooser가 상황별 row를 확정한 뒤 `ActionComponent.TryStartActionFromTable`을 호출하게 연결한다.

## 검증

- [x] C++ 빌드 또는 가능한 정적 검증을 수행한다.
- [x] `MaverickEditor Win64 Development` UHT 리플렉션 코드 생성을 확인한다.
- [x] 변경된 `MVActionComponent.cpp`, `MVCharacterBase.cpp`, `MVStatComponent.cpp`를 UBT response file 기반으로 단위 컴파일한다.
- [x] Live Coding 종료 후 전체 `MaverickEditor Win64 Development` 빌드를 다시 실행한다.
