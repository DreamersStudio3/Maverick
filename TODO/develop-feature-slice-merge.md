# develop 기준 기능 분리 병합 조사

## 목표

- 최신 `develop` 기준에서 `feat/potion-ability-collision-inputmanager` 변경사항을 기능 단위로 나눈다.
- 충돌 위험이 낮고 의존성이 적은 순서대로 병합 후보를 정리한다.

## 진행 상태

- [x] 루트 `POLICY.md` 확인
- [x] `develop` 체크아웃
- [x] `develop` pull
- [x] 기능 브랜치 변경 범위 확인
- [x] `develop` 최신 변경과 겹치는 파일 확인
- [x] 우선 병합 후보 선정

## 조사 결과

- `develop` pull 범위: `8e44edd..135716e`
- 전체 브랜치 가상 병합 충돌:
  - `Content/Characters/CharacterLogic/BP_ThirdPersonCharacter.uasset`
  - `Content/Characters/Components/TEST_OneHand_Skill.uasset`
  - `Source/Maverick/Components/MVCombatComponent.cpp`
- `develop` 최신 변경과 기능 브랜치 변경의 파일 교집합:
  - `Content/Characters/CharacterLogic/BP_ThirdPersonCharacter.uasset`
  - `Content/Characters/Components/TEST_OneHand_Skill.uasset`
  - `Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp`
  - `Source/Maverick/Components/MVCombatComponent.cpp`
  - `Source/Maverick/Components/MVCombatComponent.h`

## 병합 후보 순서

0. 필요하면 `2ebe042` 회피 노티파이 애셋을 먼저 별도 병합한다.
   - 단독 충돌이 없고 본 병합 흐름과 의존성이 거의 없다.
1. InputManager 라우팅 코어와 RecoveryEscapeWindow 일반화를 첫 본 작업 조각으로 병합한다.
   - `MVInputManagerComponent`, `MVActionInputHandlerInterface`, GameplayTag 액션 입력 정의를 중심으로 적용한다.
   - RecoveryEscapeWindow가 열렸을 때 `InputManager`가 버퍼 입력을 전체 handler 순서로 재라우팅한다.
   - Dodge/Combat/HitReaction의 `OnRecoveryEscapeWindowChanged` 직접 구독과 직접 `ClearBufferedActionInput` 호출은 제거한다.
   - Dodge의 `CanTransitionActiveDodgeAction`처럼 현재 액션이 같은 도메인인지 검사하는 조건은 제거한다.
   - 어떤 액션이 재생 중이든 `RecoveryEscapeWindow && ActionComponent.CanInterruptActiveAction()`이면 handler가 선택한 다음 액션으로 전환할 수 있어야 한다.
   - Combat 병합 시 `develop`의 Ability 인스턴스 종료/교체 처리와 `SendDataToActionComp`의 start/transition 분기, 버퍼 유지 예외 처리를 유지한다.
2. 무기 런타임 코드를 다음 조각으로 병합한다.
   - `MVWeaponComponent`, 무기/item row 타입, HitResolver weapon snapshot, CharacterBase의 WeaponComponent 생성 연결을 적용한다.
   - `MVCombatComponent.h`의 `ChangeWeapon` 공개는 포함하되, `MVCombatComponent.cpp`는 건드리지 않는다.
   - develop에서 합의된 `Trace_Start`, `Trace_End`, `Trace_Left`, `Trace_Right` 무기 소켓 계약을 `WeaponComponent`에 반영한다.
   - 실제 무기 메시 장착 시 네 소켓이 모두 있는지 검증하고, 누락 시 명확히 실패하거나 경고하도록 한다.
   - `BP_ThirdPersonCharacter.uasset`, 테스트 무기 자동 장착, 테스트 DataTable/아이콘 연결은 바이너리 충돌과 검증 범위 때문에 뒤로 미룬다.
3. 회복약 컴포넌트와 HUD/Stat/Notify를 그 다음 조각으로 병합한다.
   - InputManager 라우터 위에 `UseConsumable` handler로 붙이는 형태가 자연스럽다.
4. 블루프린트/테스트 애셋은 마지막에 수동 병합한다.
   - `BP_ThirdPersonCharacter.uasset`, `TEST_OneHand_Skill.uasset`는 자동 병합 불가.
   - develop의 새 Ability/Combat 애셋 구조를 기준으로 기능 브랜치의 테스트 연결을 재구성한다.

## 추가 확인된 문제점

- RecoveryEscapeWindow 정책이 도메인 컴포넌트에 흩어져 있다.
  - 현재 Dodge는 활성 액션이 Dodge table에서 시작한 액션인지 확인해 Dodge끼리만 recovery 전환을 허용한다.
  - 병합 후에는 InputManager가 공통 버퍼 재라우팅을 담당하고, 도메인은 자기 입력 tag를 처리할지만 판단해야 한다.
- 기능 브랜치의 라우터는 방향은 맞지만 Dodge 내부 도메인 제한을 제거하지 못했다.
  - `CanTransitionActiveDodgeAction`의 active Dodge row 검사 제거가 필요하다.
- `develop`의 CombatComponent 변경은 라우터 병합 중 유실되면 안 된다.
  - `PreviousAbilityInstance`, `CurrentAbilityInstance`, `IMVAbilityInterface::EndAbility` 호출을 유지한다.
  - 액션 실행 중에는 `RecoveryEscapeWindow && CanInterruptActiveAction`일 때만 transition하고, 그 외에는 버퍼를 보존한다.
- 무기 메시 소켓 계약이 추가됐다.
  - `Trace_Start`, `Trace_End`, `Trace_Left`, `Trace_Right`는 무기별 필수 소켓으로 취급한다.
  - 현재 기능 브랜치의 `MVWeaponComponent`에는 해당 소켓 검증/조회 API가 없다.

## 참고

- 커밋 단위 cherry-pick은 적합하지 않다.
  - `cd85063`은 무기/회복약/블루프린트가 한 덩어리이며 `BP_ThirdPersonCharacter.uasset`에서 충돌한다.
  - `d1465dc`는 라우터 커밋이지만 이전 커밋의 회복약 컴포넌트를 수정하므로 단독 적용 시 modify/delete 충돌이 난다.
  - `2ebe042` 회피 노티파이 애셋 커밋은 단독 충돌이 없다.

## `codex/merge-potion-inputmanager-slices` 적용 결과

- [x] `develop` 최신 커밋 `135716e`에서 새 브랜치 생성
- [x] InputManager GameplayTag 라우터 적용
- [x] `SubmitActionInput` 입구를 `FGameplayTag` 기반 단일 API로 정리
- [x] 기존 `SubmitActionInput(EMVActionId)` / `SubmitActionInputById(int32)` / action id 상수 제거
- [x] Dodge/Combat/HitReaction의 직접 action input/recovery window 구독 제거
- [x] RecoveryEscapeWindow 재라우팅과 버퍼 clear를 InputManager 중심으로 정리
- [x] RecoveryEscapeWindow 입력 소비 흐름에 active action cancel 경로를 추가하고, 일반 이동 입력은 로코모션 cancel 전용 함수/블렌드 시간으로 처리
- [x] Dodge의 active Dodge row 제한 제거
- [x] CombatComponent는 develop의 Ability 인스턴스 종료/교체와 start/transition 분기를 유지
- [x] WeaponComponent 런타임 코드 적용
- [x] 무기 메시 `Trace_Start`, `Trace_End`, `Trace_Left`, `Trace_Right` 필수 소켓 검증 추가
- [x] WeaponComponent visual mesh를 SkeletalMesh 전용에서 StaticMesh/SkeletalMesh 양쪽 지원으로 확장
- [x] WeaponComponent에 BeginPlay 기본 장착용 `DefaultWeaponRow` 추가
- [x] `DT_Weapons`와 무기 아이콘 애셋, 테이블용 gameplay tag config 추가
- [x] 공격 row의 Damage/GroggyDamage 입력을 DamageMultiplier/GroggyDamageMultiplier 배율로 전환
- [x] HitResolver의 HP/그로기 피해 계산을 무기 공격력 기반 배율 계산으로 전환
- [x] 회복약 컴포넌트, HP 회복 notify, HUD quick slot, transition reset 연결 적용
- [x] FieldTransition의 소비 아이템 카운트 복구 진입점을 회복약 전용 API에서 월드 리셋용 컨슈머블 복구 API로 정리
- [ ] 최신 배율 변경 후 `MaverickEditor Win64 Development` 재빌드
  - UHT는 통과했으나 Live Coding 활성화로 UBT 컴파일 단계가 막힘

## 이번 브랜치에서 제외한 것

- `BP_ThirdPersonCharacter.uasset`, `TEST_OneHand_Skill.uasset` 같은 충돌 가능성이 큰 바이너리 애셋
- 입력 매핑/회복약 몽타주 같은 애셋 연결
- 테스트 무기 자동 장착 디버그 코드
