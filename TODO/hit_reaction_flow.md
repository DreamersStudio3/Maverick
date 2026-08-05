# HitReaction / Action 실행 구조 전환 작업

팀 합의 구조는 `ActionComponent`가 액션 선택자가 아니라 실행기가 되는 방향이다.
`CombatComponent`와 `HitReactionComponent`는 각자 ChooserTable을 평가해 실행할 DataTable row handle과 시작 섹션을 확정한 뒤 `ActionComponent`에 전달한다.

상황별 액션 테이블은 런타임 중 동적으로 늘어나지 않고 초기화 시점에 정해진 테이블들이므로, Chooser의 Output Struct Column에는 에디터에서 고정된 `FDataTableRowHandle`을 넣는다.
Chooser 결과는 도메인에 따라 `FDataTableRowHandle`로 최종 row까지 확정하고, `ActionComponent`에는 row handle을 넘긴다.
HitReaction 테이블은 CSV 생성 체인에서 빼고 `Content/Table/HitReaction/P1/DT_HR_P1`처럼 도메인/캐릭터별 uasset DataTable로 직접 관리한다.

## 전환 원칙

- [x] 이전 중앙 액션 인덱스/스탯 테이블 기반 실행 흐름은 제거하고, Dodge/Sprint도 도메인별 Chooser/DataTable row 흐름으로 전환한다.
- [x] 새 Combat/HitReaction 흐름에서는 `ActionComponent`가 선택 정책을 갖지 않고, 전달받은 row handle로 row를 읽어 몽타주 재생과 이벤트 브로드캐스트만 수행한다.
- [x] 액션 입력의 최소 공용 입구는 `InputManagerComponent.SubmitActionInput`으로 두고, `ActionComponent`에는 현재 몽타주 섹션 점프 API를 추가한다.
- [ ] Input buffer 내부 상태와 movement block 같은 나머지 입력 정책은 이후 공용 `InputManagerComponent`로 이동한다.
- [x] 현재 브랜치의 기존 `HitReactionIndex` 기반 구현은 팀 합의 구조와 다르므로 새 구조에 맞게 리워크한다.

## ActionComponent 실행 API

- [x] 상황별 액션 공용 row struct `FMVActionRow`를 추가한다.
  - 공통 최소 row는 `Montage`, interrupt 가능 여부, 이동 잠금 여부처럼 실행에 필요한 필드만 가진다.
  - HitReaction 전용 row는 공통 row를 상속하고 런치 여부/거리/길이/수직량을 추가한다.
  - 상황별 액션 테이블은 별도 `ActionKey`/`ActionId` 컬럼 없이 DataTable `RowName`을 실행 키로 사용한다.
- [x] `ActionComponent`에 새 실행 API를 추가한다.
  - 예: `TryStartActionFromTable(FName ActionTableName, FName ActionRowName, FName StartSection = NAME_None)`.
- [x] 새 실행 API는 `MVTableManager`로 `ActionTableName`의 해당 `RowName` row를 찾고, 그 row의 몽타주와 실행 데이터를 사용해 재생한다.
- [x] 새 실행 API에서도 `OnActionPreparing`, `OnActionStarted`, `OnActionEnded` 같은 실행 이벤트는 기존처럼 브로드캐스트한다.
- [x] 현재 액션 존재 여부와 interrupt 가능 여부를 조회할 수 있는 최소 API를 정리한다.
- [x] `TryTransitionAction...` 경로는 기존 몽타주를 선행 `Montage_Stop`하지 않고 액션 상태만 interrupt 처리한 뒤 새 몽타주 재생으로 전환한다.
- [ ] 이후 커스텀 몽타주 priority를 도입해 높은 priority 재생 요청이 낮은 priority 액션을 인터럽트할 수 있게 한다.

## HitReactionComponent

- [x] `HitReactionType`은 `Flinch`, `Stagger`, `Knockback`, `KnockDown`, `Airborne` 기준으로 유지한다.
- [x] `HitReactionComponent`를 `CharacterBase`에 부착하고 `CharacterBase.OnDamaged`에 `HandleDamaged`를 바인딩한다.
- [x] `HandleDamaged`는 `GetActionData` 성격의 함수에서 `CHT_HitReaction`을 평가한다.
- [x] Chooser 입력 필터는 최소한 캐릭터 인덱스 GameplayTag, 장비 스타일, `HitReactionType`, 피격 방향을 사용한다.
- [x] `CharacterIndex`와 `CharacterStat`은 `CharacterIndexCode` GameplayTag를 row key로 공유한다.
- [x] Chooser 결과로 `DT_HR_P1`의 최종 `FDataTableRowHandle`을 얻는다.
- [x] P1 HitReaction 테이블은 본 리액션과 recovery row를 함께 관리한다.
  - Flinch: F/L/R/B
  - Stagger: F/L/R/B
  - Knockback: F
  - KnockDown: F/B
  - Airborne: F/B
- [x] Chooser Output Struct Column에서 실행할 `FDataTableRowHandle`, `SectionName`, 몽타주/HitReaction 전용 row 정보를 확정한다.
- [x] 무적 여부, 현재 액션 존재 여부, 현재 액션 interrupt 가능 여부를 확인한 뒤 실행 가능한 경우에만 `ActionComponent` 새 API를 호출한다.
- [x] HitReaction 상황별 테이블은 CSV 생성 체인에서 제외하고 `Content/Table/HitReaction/P1` 아래 uasset DataTable로 직접 관리한다.
- [x] `Refresh Table Manifest` 명령으로 직접관리 DataTable을 manifest에 등록한다.
- [x] 리커버리/팔로쓰루 window 입력은 `InputManagerComponent`가 입력 의도와 이동 입력 스냅샷으로 버퍼링/브로드캐스트한다.
- [x] PlayerCharacter.Dodge 서브모듈은 `InputManagerComponent.SubmitActionInput(Dodge)` 이벤트를 구독해 기존 Dodge 액션 실행을 담당한다.
- [x] `HitReactionComponent`는 액션 입력 이벤트를 구독하고, Dodge 입력은 별도 `EscapeDodge` recovery 액션, Dodge 외 액션 입력은 별도 `Getup` recovery 액션으로 소비한다.
- [x] 이동 입력은 HitReactionComponent가 이벤트로 직접 소비하지 않고, Recovery window가 열리는 순간 InputManager에 저장된 최근 이동 입력을 조회해 Flinch/Stagger/Knockback은 현재 HitReaction 취소, KnockDown/Airborne은 별도 `EscapeDodge` recovery 액션으로 소비한다.
- [x] Flinch/Stagger/Knockback은 넘어지는 리액션이 아니므로 Getup/EscapeDodge 섹션을 찾지 않고, Recovery window 안에서 이동 또는 Dodge 입력이 있으면 현재 피격 몽타주를 직접 cancel한다.
- [x] KnockDown/Airborne은 Recovery window 안에 저장된 입력이 있으면 별도 `EscapeDodge` recovery 액션으로 전환한다.
- [x] KD/AB 본 리액션은 상태 표현까지만 담당하고, Recovery window 안에서 입력이 없으면 별도 `MV HitReaction Start Getup` Notify가 Getup row로 전환한다.
- [x] Getup/EscapeDodge recovery 액션도 active HR recovery row로 추적해, 해당 몽타주의 Recovery window에서 이동 입력은 직접 cancel하고 Dodge 입력은 PlayerCharacter.Dodge 전환으로 넘긴다.
- [x] HR EscapeDodge 시작 직전 actor yaw를 컨트롤러 기준 yaw로 맞춰, 누운 방향과 무관하게 F/L/R/B 입력 방향이 컨트롤러 기준으로 적용되게 한다.
- [x] `CHT_HitReaction`은 0번 context로 `MVHitReactionComponent`, 1번 context로 `FMVHitReactionActionRowHandle` output struct를 받는다.
- [x] `CHT_HitReaction`은 여러 HitReaction DataTable 중 최종 row를 `FMVHitReactionActionRowHandle` Output Struct Column으로 반환한다.
- [x] 이미 HitReaction 액션이 재생 중이면 recovery window 밖에서는 추가 HitReaction으로 현재 몽타주를 끊지 않는다.
- [x] HitReaction row의 launch 필드를 실제 `LaunchCharacter` 호출에 연결한다.
- [x] Airborne `Fall` 루프 구간용 `MV Airborne Land Detector` NotifyState를 추가하고, 착지 시 `Land` 섹션으로 점프하게 한다.
- [x] Airborne 몽타주는 착지 후 `Land -> Lying` 섹션 순서로 이어지고, `Lying` 구간의 Recovery window는 EscapeDodge 입력만 받으며 `MV HitReaction Start Getup` Notify에서 Getup 전환을 처리한다.
- [x] lethal standing hit은 HitReaction을 생략하고, lethal KD/AB hit만 HitReaction을 먼저 재생한다.
- [x] KD/AB lethal hit은 `Lying` 섹션 진입 전 `MV HitReaction Death Handoff` Notify로 DeathComponent에 넘길 수 있게 한다.
- [x] `CHT_Dodge`는 조건을 통과한 최종 `FMVDodgeActionRowHandle`을 Output Struct Column으로 PlayerCharacter.Dodge에 기록하고, PlayerCharacter.Dodge가 해당 row를 실행한다.
- [x] Sprint는 Chooser를 쓰지 않고 단일 `DT_Sprint` 또는 직접 지정된 `FDataTableRowHandle`에서 row를 읽는다.
- [ ] InputManager 입력 이벤트 소비/우선순위 정책을 추가해 HitReaction/Combat/Dodge 중 하나가 입력을 소비하면 나머지 도메인이 중복 실행하지 않게 한다.
- [ ] 프로젝트 입력 바인딩에서 Dodge/Combat 입력을 `InputManagerComponent.SubmitActionInput`으로 연결한다.
- [ ] 모든 HitReaction 몽타주에서 입력을 받을 리커버리/팔로쓰루 구간에 `MV Recovery Escape Window` NotifyState를 배치한다.
- [ ] Airborne 몽타주의 `Fall` 루프 구간에 `MV Airborne Land Detector` NotifyState를 배치한다.
- [ ] KnockDown/Airborne 몽타주의 `Lying` 섹션 진입 직전에 `MV HitReaction Death Handoff` Notify를 배치한다.
- [ ] KnockDown/Airborne의 기본 Getup handoff 프레임에 `MV HitReaction Start Getup` Notify를 배치한다.
- [ ] lethal Airborne PIE 재현 케이스에서 Land 이후 death handoff 대신 Getup으로 전환되는 원인을 확인한다. 최신 로그에는 Chooser context 오류가 없으므로 HP 0 이벤트 발행, dead state 유지, `MV HitReaction Death Handoff`와 `MV HitReaction Start Getup` notify 순서를 계측한다.
- [ ] KnockDown/Airborne 본 리액션 몽타주는 Lying/Fall/Land 같은 상태 표현까지만 두고, Recovery window에서 별도 Getup/EscapeDodge 액션으로 전환하게 한다.
- [x] Combat 흐름 완성 전까지 BP_Carcass 대화가 완전히 닫힌 뒤 `MVUISubsystem`의 PIE 피격 테스트 사이드 윈도우를 열고, 버튼 선택 시 `OnHitResolved` 피격 흐름을 호출한다.

## CombatComponent 연계

- [ ] Combat 쪽도 `CHT_CombatAction` 계열 ChooserTable에서 상황별 액션 테이블 이름, `RowName`, 섹션 이름을 확정하는 구조로 맞춘다.
- [ ] Combat과 HitReaction이 같은 `ActionComponent` 실행 API와 `FMVActionRow` 기반 필드를 공유하되, 도메인별 추가 컬럼은 전용 row struct로 분리한다.
- [ ] 공격 액션의 Notify/NotifyState는 이후 충돌 컴포넌트 API를 호출하고, 충돌 컴포넌트가 유효 타격만 `HitResolverSubsystem`에 전달한다.

## Collision / Notify 연계

- [x] `HitResolverSubsystem` 호출 입구를 `FMVHitResolveRequest`를 받는 `ResolveAttackHit` 하나로 통일한다.
- [ ] 공격 몽타주 NotifyState에서 충돌 컴포넌트 API를 호출하는 최소 흐름을 만든다.
- [ ] 충돌 컴포넌트가 공격자 `CharacterIndexCode`, 실행 row, 이미 맞은 대상 목록을 기억해 중복 타격과 자기 자신 타격을 막는다.
- [ ] 유효 타격 후보만 `HitResolverSubsystem`에 전달한다.

## WeaponComponent 연계

- [ ] `WeaponComponent`가 추가되면 HitResolver의 `ResolveEquippedWeaponAttackPower`에서 현재 무기 스탯을 직접 조회한다.
- [ ] 무기 아이템이 없는 상태도 맨손 무기 기본 장착으로 처리한다.

## develop 병합 정리

- [x] `CharacterIndex`/`CharacterStat` 충돌은 `CharacterIndexCode` GameplayTag row key 기준으로 정리한다.
- [x] develop의 `NamelessPuppet` 데이터는 `Character.NPC.Enemy.NamelessPuppet` row로 이식한다.
- [x] develop AI 공격 실행 태스크는 `ActionId` 직접 실행 대신 `FDataTableRowHandle`을 ActionComponent에 전달하게 정리한다.
- [ ] 남은 `ActionIndex` 충돌은 legacy 액션 테이블을 복구하지 않고 AI 공격 데이터를 새 ActionRow/RowHandle 구조로 옮길지 결정한다.
- [ ] `BP_Carcass` 바이너리 충돌은 에디터에서 대화/PIE 테스트 연결과 develop AI 변경을 함께 확인해 수동 병합한다.
- [ ] 남은 충돌 처리 후 `Tools > Maverick > Generate Data Tables`를 실행해 `DT_CharacterIndex`/`DT_CharacterStat`을 CSV 원본과 맞춘다.
