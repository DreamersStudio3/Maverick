# Item / Weapon / Healing Potion Vertical Slice Plan

아이템 시스템의 전체 범위를 한 번에 구현하지 않고, 전투 대미지와 액션/스킬 귀속에 필요한 무기를 첫 번째 vertical slice로 도입한다.

회복약은 인벤토리 시스템이 아니라 전투 프로토타입용 고정 퀵 아이템 vertical slice로 함께 다룬다.

## 배경

- 현재 필요한 기능은 소울라이크 전체 아이템 루프가 아니라, 플레이어의 공격과 스킬이 어떤 무기에서 나오는지 결정하는 최소 런타임 계약이다.
- `FMVHitResolveRequest`와 `UMVHitResolverSubsystem`에는 이미 `WeaponComponent`가 현재 무기 공격력을 제공한다는 주석 계약이 있다.
- 지금은 프로토타입 호출자가 `Request.WeaponAttackPower`를 직접 주입하지만, 정식 흐름에서는 공격자의 현재 장착 무기에서 공격력을 읽어야 한다.
- 전투 프로토타입에서는 피격 이후 물러나 회복하고 다시 진입하는 흐름도 중요하다.
- 발표까지 1주일이 남았으므로 인벤토리, 루팅, 강화, 조합, 장비 UI까지 확장하면 전투 검증 리스크가 커진다.

## 결론

이번 브랜치에서는 두 개의 얇은 vertical slice만 구현한다.

- 무기: `ItemTag -> WeaponDefinition -> WeaponComponent -> HitResolver / CombatComponent`
- 회복약: `ItemTag -> HealingPotionDefinition -> PlayerConsumableComponent -> StatComponent / QuickSlotWidget`
- 스킬 HUD: `CombatComponent -> SkillHUDWidget -> Q/R cooldown / Q stack`

무기는 아이템 시스템의 일부로 정의하되, 실제 구현 범위는 다음에 한정한다.

- 맨손 기본 무기를 항상 장착한 것으로 처리한다.
- 캐릭터가 현재 장착 무기의 공격력과 장비 스타일을 조회할 수 있게 한다.
- `HitResolver`는 공격자의 현재 무기 공격력을 최종 대미지 계산에 반영한다.
- `CombatComponent`는 이번 브랜치에서 기존 장비 스타일 기반 액션/스킬 테이블 선택 흐름을 임시 유지한다.
- 장기적으로는 `BareHand`, `OneHand`, `TwoHand` 같은 장비 스타일이 아니라 무기 `ItemTag` 또는 무기 정의의 액션 세트가 공격/스킬 테이블 선택 기준이 되어야 한다.
- 블루프린트나 테스트 코드에서 기본 무기와 테스트 무기를 쉽게 바꿀 수 있게 한다.

회복약은 아이템 시스템의 일부로 정의하되, 실제 구현 범위는 다음에 한정한다.

- 플레이어에게 고정 회복약 슬롯 1개를 제공한다.
- `UseConsumable` 입력으로 회복약 사용을 시도한다.
- 사용 가능하면 HP를 회복하고 보유 카운트를 감소시킨다.
- 퀵슬롯 위젯의 카운트를 갱신한다.
- 카운트가 0이면 퀵슬롯을 비활성화한다.
- 월드 리셋 또는 사망 부활 시 회복약 카운트를 기본값으로 복구한다.

스킬 HUD는 전투 프로토타입 피드백을 위해 함께 계획한다. 실제 구현은 회복약 퀵슬롯 이후에 붙이며, Q/R 스킬 쿨타임 진행과 Q 스킬 스택 표시를 목표로 한다.

## 식별자 정책

아이템 식별자는 별도 `ItemId`를 두지 않고 `FGameplayTag ItemTag`를 표준 키로 사용한다.

- `ItemTag`는 아이템의 canonical gameplay identifier다.
- DataTable row name은 에디터/테이블 조회용 row locator일 뿐이며, 게임플레이 식별자는 아니다.
- row name은 가능하면 `ItemTag`의 leaf나 전체 경로와 맞춰 사람이 추적하기 쉽게 한다.
- 별도 ID와 태그를 동시에 둘 필요가 생기면 책임 차이와 동기화 규칙을 먼저 문서화한다.

예상 태그 예시:

- `Item.Weapon.BareHand`
- `Item.Weapon.TestSword`
- `Item.Weapon.TestCrossbow`
- `Item.HealingPotion.Basic`

## 비목표

이번 브랜치에서는 아래 기능을 구현하지 않는다.

- 인벤토리 저장소, 슬롯, 정렬, 스택 처리
- 필드 루팅, 몬스터 드랍률, 상자 보상, NPC 퀘스트 보상
- 회복약 1종 외 소모품, 투척물, 버프 아이템, 재화 쿠폰 사용
- 퀵슬롯 편집, 여러 퀵슬롯 선택, 슬롯 순환
- 장비 UI, 장비 교체 메뉴, 장비 비교
- 강화, 조합, 제작, 분해
- 아이템 세이브/로드 영속화
- 네트워크 복제

단, 나중에 위 기능들이 붙을 수 있도록 식별자와 데이터 경계는 아이템 시스템 형태로 둔다.

## 설계 원칙

- 새 gameplay identity는 `FGameplayTag`를 우선 검토한다.
- `CharacterBase`는 공통 캐릭터 상태와 컴포넌트 연결만 담당한다.
- 장착 무기 정책은 `WeaponComponent`가 소유한다.
- 플레이어 회복약 정책은 `PlayerConsumableComponent`가 소유한다.
- 전투 대미지 계산은 `HitResolver`에 남긴다.
- HP 회복 값 변경은 `StatComponent` API를 통해 수행한다.
- 액션/스킬 선택과 어빌리티 인스턴스 생성은 기존 `CombatComponent`와 공격/스킬 DataTable row 흐름을 따른다.
- 퀵슬롯 위젯은 표시와 잠금 상태만 담당하고, 사용 가능 여부와 카운트 정책은 소유하지 않는다.
- 데이터는 현재 프로젝트의 테이블 기반 흐름에 맞춰 DataTable row로 시작한다.
- 무기가 없는 상태를 별도로 허용하지 않고, 항상 `BareHand` 무기를 기본 장착한다.

## Combat Ability 구조 확인

현재 Combat 구현은 무기가 어빌리티 클래스를 직접 소유하는 구조가 아니다.

- `FMVSkillDataTableColumn`은 `FMVActionRow`를 상속하고, 공격/스킬 row 단위로 `AbilityReference`, `Damage`, `StaminaCost`, `MpCost`, `GroggyDamage`, `CooldownDuration`, 체인 정보를 가진다.
- `UMVCombatComponent::ResetBasicAttackMap`과 `ResetSkillMap`은 현재 장비 스타일과 액션 타입으로 DataTable을 찾고, 각 row의 `AbilityReference`로 `UMVAbilityBase` 인스턴스를 만든다.
- 체인 공격/스킬은 row의 `NextChainName`을 따라 각 단계별 어빌리티 인스턴스를 만든다.
- `UMVAbilityBase::InitAbility`는 row 전체를 `AbilityData`로 보관하므로, 공격/스킬별 프로퍼티는 이미 row 단위로 전달된다.
- 현재 `MVAnimNotifyState_Ability`의 Start/End 호출 코드는 주석 처리되어 있어 어빌리티 실행 타이밍 연결은 별도 정리가 필요하다.

따라서 무기 데이터에는 `GrantedAbilityClasses`를 두지 않는다.

이번 vertical slice에서 무기는 공통 공격력, 범위 타입, 장착 메시 정보를 제공하고, 개별 공격/스킬의 어빌리티와 수치 프로퍼티는 기존 `FMVSkillDataTableColumn` row가 계속 소유한다. 구현상 기존 `EquippedStyle` 전달은 남기지만, 이는 발표 전 검증용 호환 경로로만 본다.

무기 row는 `GroggyDamage`, 공격 속도, 어빌리티별 계수 같은 공격 단위 값을 소유하지 않는다. 그로기 수치는 공격/스킬 row의 `GroggyDamage`가 정하고, 공격 속도는 애님/액션 타이밍 계약이 정리된 뒤 별도 시스템에서 다룬다.

장기적으로 Maverick의 무기는 엘든링식 직검/단검/레이피어 카테고리별 공용 모션셋이 아니라, 리그 오브 레전드나 세키로에 가까운 무기별 고유 모션셋과 고유 스킬을 가진다. 따라서 별도 `CombatProfileTag`를 미리 두지 않고, 이후 확장에서는 무기의 `ItemTag` 또는 무기 정의 자체가 고유 액션/스킬 세트의 기준이 되게 설계한다.

## 제안 파일 구조

### 데이터 타입

- `Source/Maverick/Public/Enum/MVItemEnums.h`
  - `EMVItemType`
  - `EMVEquipmentSlot`
- `Source/Maverick/Public/Tables/MVItemTableTypes.h`
  - `FMVItemTableRow`
- `Source/Maverick/Public/Tables/MVWeaponTableTypes.h`
  - `FMVWeaponTableRow`
- `Source/Maverick/Public/Tables/MVHealingPotionTableTypes.h`
  - `FMVHealingPotionTableRow`
- `Source/Maverick/Public/Struct/MVWeaponTypes.h`
  - `FMVEquippedWeaponState`
  - `FMVWeaponHitSnapshot`
- `Source/Maverick/Public/Struct/MVHealingPotionTypes.h`
  - `FMVHealingPotionRuntimeState`

### 런타임

- `Source/Maverick/Components/MVWeaponComponent.h`
- `Source/Maverick/Components/MVWeaponComponent.cpp`
- `Source/Maverick/Character/PC/Consumable/MVPlayerConsumableComponent.h`
- `Source/Maverick/Character/PC/Consumable/MVPlayerConsumableComponent.cpp`

### 수정 대상

- `Source/Maverick/Character/MVCharacterBase.h`
- `Source/Maverick/Character/MVCharacterBase.cpp`
- `Source/Maverick/Character/PC/MVPlayerCharacter.h`
- `Source/Maverick/Character/PC/MVPlayerCharacter.cpp`
- `Source/Maverick/Combat/MVHitResolverSubsystem.h`
- `Source/Maverick/Combat/MVHitResolverSubsystem.cpp`
- `Source/Maverick/Components/MVCombatComponent.h`
- `Source/Maverick/Components/MVCombatComponent.cpp`
- `Source/Maverick/Components/MVStatComponent.h`
- `Source/Maverick/Components/MVStatComponent.cpp`
- `Source/Maverick/UI/HUD/MVMainHUDWidget.h`
- `Source/Maverick/UI/HUD/MVMainHUDWidget.cpp`
- `Source/Maverick/UI/HUD/MVQuickSlotWidget.h`
- `Source/Maverick/UI/HUD/MVQuickSlotWidget.cpp`
- `Source/Maverick/UI/HUD/MVSkillSlotWidget.h`
- `Source/Maverick/UI/HUD/MVSkillSlotWidget.cpp`
- `Source/Maverick/UI/HUD/MVSkillHUDWidget.h`
- `Source/Maverick/UI/HUD/MVSkillHUDWidget.cpp`
- `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`

## 데이터 초안

### EMVItemType

- `None`
- `HealingPotion`
- `Equipment`
- `Quest`
- `CurrencyCoupon`
- `Material`

### EMVEquipmentSlot

- `None`
- `Weapon`
- `Armor`
- `Accessory`

### FMVItemTableRow

- `FGameplayTag ItemTag`
- `EMVItemType ItemType`
- `FText DisplayName`
- `FText Description`
- `TSoftObjectPtr<UTexture2D> Icon`

`ItemTag`가 primary gameplay key다. 별도 `ItemId`는 두지 않는다.

### FMVWeaponTableRow

- 공통 아이템 표시 필드
- `EMVEquippedStyle EquippedStyle`
- `float AttackPower`
- `EMVWeaponRangeType RangeType`
- `TSoftObjectPtr<USkeletalMesh> WeaponMesh`
- `FName AttachSocketName`
- `FTransform AttachTransform`

`EquippedStyle`은 현재 `CombatComponent` 호환용 임시 필드다. 무기별 고유 액션 세트가 정리되면 `ItemTag` 또는 별도 액션 세트 태그/참조로 대체한다.

어빌리티 클래스와 공격/스킬 수치 프로퍼티는 무기 row에 두지 않는다. 기존 Combat 구조처럼 각 공격/스킬 row의 `FMVSkillDataTableColumn`이 `AbilityReference`, `Damage`, 비용, 쿨다운, 체인 정보를 소유한다. 이후 무기별 고유 모션셋과 고유 스킬을 연결할 때는 `CombatProfileTag`가 아니라 무기 `ItemTag` 또는 무기 정의에서 액션/스킬 세트를 직접 가리키는 구조를 검토한다.

### FMVHealingPotionTableRow

- 공통 아이템 표시 필드
- `float HealAmount`
- `int32 DefaultCount`
- `int32 MaxCarryCount`
- `FDataTableRowHandle UseActionRow`
- `FName UseActionStartSection`

이번 브랜치에서는 HP 회복약 1종만 사용한다. 그래서 넓은 `ConsumableTableRow`를 만들지 않고 `HealingPotionTableRow`로 좁게 둔다. `UseActionRow`가 비어 있으면 프로토타입 검증을 위해 즉시 사용을 허용하고, row가 있으면 `ActionComponent`로 사용 액션을 실행한 뒤 회복을 적용한다.

### FMVEquippedWeaponState

- `FGameplayTag ItemTag`
- `EMVEquippedStyle EquippedStyle`
- `float AttackPower`
- `EMVWeaponRangeType RangeType`
- `TSoftObjectPtr<USkeletalMesh> WeaponMesh`
- `FName AttachSocketName`
- `FTransform AttachTransform`
- `bool bValid`

`WeaponComponent`가 소유하는 현재 장착 무기 상태다. 대미지 계산이나 UI가 테이블 row 포인터에 직접 의존하지 않도록, 장착 시점에 필요한 값을 복사해 state로 보관한다.

### FMVWeaponHitSnapshot

- `FGameplayTag ItemTag`
- `EMVEquippedStyle EquippedStyle`
- `float AttackPower`
- `EMVWeaponRangeType RangeType`
- `bool bValid`

타격이 확정되어 `FMVResolvedHitData`를 만들 때 `WeaponComponent`의 현재 state에서 캡처하는 히트 전용 사본이다. 피격 처리와 후속 이벤트는 이후 장착 변경과 무관하게 해당 타격에 사용된 무기 값을 참조한다.

### FMVHealingPotionRuntimeState

- `FGameplayTag ItemTag`
- `float HealAmount`
- `int32 CurrentCount`
- `int32 MaxCarryCount`
- `bool bCanUse`

퀵슬롯은 이 상태를 직접 계산하지 않고, `PlayerConsumableComponent`가 변환한 `FMVQuickSlotViewData`를 받는다.

## WeaponComponent 책임

- 기본 맨손 무기 데이터를 초기화한다.
- 현재 장착 무기 state를 소유한다.
- 확정된 `FMVWeaponTableRow`로만 무기를 장착한다.
- 장착 변경 시 현재 호환 경로를 위해 캐릭터의 `EquippedStyle`을 갱신한다.
- 장착 무기 row에 메시가 있으면 캐릭터 메시 소켓에 무기 메시를 부착한다.
- 장착 변경 이벤트를 브로드캐스트한다.
- `HitResolver`가 사용할 공격력 조회 API를 제공한다.
- 타격 확정 시점에 전달할 `FMVWeaponHitSnapshot` 캡처 API를 제공한다.
- 테이블 조회는 인벤토리, 보상, 테스트 코드 같은 상위 호출자가 담당하고, 이 컴포넌트는 확정된 `FMVWeaponTableRow`만 장착한다.

필수 API 초안:

- `EquipWeaponFromRow(const FMVWeaponTableRow& WeaponRow)`
- `EquipBareHand()`
- `GetEquippedWeaponState()`
- `CaptureWeaponHitSnapshot()`
- `GetEquippedWeaponAttackPower()`
- `GetEquippedStyle()`
- `GetWeaponRangeType()`
- `OnEquippedWeaponChanged`

## Player Test Weapon 연동

인벤토리와 장비 UI가 없으므로 플레이어 캐릭터에는 발표 전 검증용 장착 경로를 둔다.

- `bEquipTestWeaponOnBeginPlay`가 켜져 있으면 BeginPlay에서 테스트 무기를 장착한다.
- `TestWeaponRow` DataTable row handle이 설정되어 있으면 해당 `FMVWeaponTableRow`를 우선 사용한다.
- DataTable row handle이 없으면 `InlineTestWeapon` row를 사용한다.
- 이 경로도 최종적으로는 `WeaponComponent.EquipWeaponFromRow(const FMVWeaponTableRow&)`만 호출한다.
- 테이블 조회는 플레이어 테스트 코드에만 있고, `WeaponComponent`에는 넣지 않는다.

## PlayerConsumableComponent 책임

- 플레이어 전용 회복약 슬롯 상태를 소유한다.
- `InputManagerComponent`의 액션 입력 handler로 등록하고 `UseConsumable` 입력을 처리한다.
- 회복약 사용 가능 여부를 판정한다.
- 사용 액션 row가 설정되어 있으면 `ActionComponent`로 회복약 사용 액션을 실행한다.
- 사용이 확정되면 HP를 회복하고 카운트를 1 감소시킨다.
- 카운트 변경 이벤트를 브로드캐스트한다.
- 월드 리셋 또는 사망 부활 시 카운트를 기본값으로 복구한다.

필수 API 초안:

- `Initialize(AMVPlayerCharacter* InOwnerCharacter)`
- `TryUseHealingPotion()`
- `ApplyHealingPotionEffect()`
- `RestoreHealingPotionCountForWorldReset()`
- `SetHealingPotionCount(int32 NewCount)`
- `GetHealingPotionState()`
- `BuildHealingPotionQuickSlotViewData()`
- `OnHealingPotionStateChanged`

사용 가능 조건 초안:

- 플레이어가 사망 상태가 아니다.
- 회복약 카운트가 1 이상이다.
- HP가 최대치여도 회복약은 사용할 수 있다. 이 경우 HP는 최대치로 유지되고 카운트만 감소한다.
- `ActionComponent`가 다른 액션을 실행 중이지 않거나, 현재 액션의 `RecoveryEscapeWindow`가 열려 있어 전환할 수 있다.
- 회복약 사용 액션 row가 지정되어 있다면 입력 시점에는 해당 액션만 시작하고, 실제 HP 회복과 카운트 감소는 몽타주 안의 `MV Apply Healing Potion` notify에서 적용한다.
- 회복약 사용 몽타주에는 회복 확정 지점부터 `MV Recovery Escape Window` notify state를 붙여, 회복 직후 연속 회복 또는 다른 액션 몽타주로 탈출할 수 있게 한다.

`InputManagerComponent`는 `Combat`, `Dodge`, `HitReaction`, `PlayerConsumableComponent`가 등록한 handler를 우선순위대로 호출한다. 입력 의도는 `Action.Input.*` GameplayTag로 전달한다. 입력 시점에 처리되지 않은 액션 입력은 짧게 버퍼에 남고, `RecoveryEscapeWindow`가 열릴 때 같은 라우팅 경로로 한 번 더 처리된다. 개별 도메인 컴포넌트가 window 이벤트를 직접 구독해 buffer를 비우는 방식은 늘리지 않는다.

회복 몽타주는 full body가 아니라 upper body 슬롯으로 제작한다. 액션 row는 이동 입력을 막지 않고, 회복약 액션 실행 중 플레이어는 걷기만 가능하며 달리기/전력질주는 막는다. 피격 취소와 사용 도중 소모 롤백은 이후 확장으로 둔다.

## QuickSlot 연동

현재 `UMVQuickSlotWidget`은 `FMVQuickSlotViewData.Count`와 `bLocked`를 표시할 수 있다.

목표 흐름:

- `PlayerConsumableComponent`가 회복약 상태 변경 이벤트를 발생시킨다.
- `MVMainHUDWidget`이 플레이어의 `PlayerConsumableComponent`에 바인딩한다.
- `HPSlot.SetViewData()` 또는 `HPSlot.UpdateCount()`로 카운트를 갱신한다.
- `CurrentCount <= 0`이면 `FMVQuickSlotViewData.bLocked = true`로 전달한다.
- `UMVQuickSlotWidget`은 `bLocked`에 따라 `SetIsEnabled(false)`를 적용한다.

필요하면 `UMVQuickSlotWidget::UpdateCount`는 카운트만 바꾸지 않고 잠금 상태도 함께 갱신할 수 있게 확장한다. 단, 사용 가능 여부 계산은 위젯에 넣지 않는다.

## Skill HUD 연동

현재 `UMVCombatComponent`는 `FMVSkillEntry`에 `MainCooldownDuration`, `LastUsedTime`, 체인 상태를 갖고 있고, Q/R 스킬은 `TrySkill(EMVCombatActionTypes::Skill, SkillIndex)` 흐름으로 실행된다.

목표 흐름:

- `UMVCombatComponent`가 Q/R 스킬 슬롯별 HUD view data를 제공한다.
- `MVMainHUDWidget`이 플레이어의 `CombatComponent`에 바인딩하고 `MVSkillHUDWidget`으로 view data를 전달한다.
- Q 슬롯은 쿨타임 남은 시간/비율과 현재 스택/최대 스택을 표시한다.
- R 슬롯은 쿨타임 남은 시간/비율을 표시한다.
- 쿨타임 시작, 스킬 맵 refresh, 장비 변경 시 HUD가 즉시 갱신된다.
- 쿨타임이 진행 중인 동안에는 HUD가 매 프레임 또는 타이머로 남은 시간을 갱신한다.

필수 API 초안:

- `FMVSkillSlotViewData`
  - `FName SkillRowName`
  - `float CooldownRemaining`
  - `float CooldownDuration`
  - `float CooldownPercent`
  - `int32 CurrentStack`
  - `int32 MaxStack`
  - `bool bLocked`
- `UMVCombatComponent::BuildSkillSlotViewData(int32 SkillIndex)`
- `UMVCombatComponent::OnSkillStateChanged`
- `UMVSkillSlotWidget::SetViewData(const FMVSkillSlotViewData& InViewData)`
- `UMVSkillHUDWidget::SetQSkillViewData(...)`
- `UMVSkillHUDWidget::SetRSkillViewData(...)`

Q 스킬 스택은 아직 책임 소유자가 명확하지 않다. 구현 시에는 어빌리티 인스턴스, `CombatComponent`, 또는 별도 스킬 상태 컴포넌트 중 하나를 정하고, HUD가 내부 맵을 직접 읽지 않게 view data API로만 노출한다.

## World Reset / Respawn 연동

현재 필드 전환 리셋은 월드 actor 대상으로 `MVFieldTransitionResettableInterface`를 호출하고, 플레이어 사망 부활 시 `UMVFieldTransitionSubsystem::ResetPlayerStatsForTransition`에서 HP/Stamina/MP/Groggy를 복구한다.

회복약 카운트 복구는 플레이어 전용 런타임 상태이므로 actor reset interface에 기대지 않는다.

목표 흐름:

- `UMVFieldTransitionSubsystem::ResetPlayerStatsForTransition`에서 플레이어의 `PlayerConsumableComponent`를 찾는다.
- 컴포넌트가 있으면 `RestoreHealingPotionCountForWorldReset()`을 호출한다.
- 복구 후 `OnHealingPotionStateChanged`를 브로드캐스트해 퀵슬롯 카운트와 잠금 상태를 갱신한다.

이번 브랜치에서는 회복약 카운트를 저장하지 않는다. 사망 부활 또는 월드 리셋 시 항상 `DefaultCount`로 복구한다.

## HitResolver 연동

현재 흐름:

- 공격 요청이 `Request.WeaponAttackPower`를 들고 온다.
- `ResolveWeaponHitSnapshot`이 요청 값을 폴백 스냅샷으로 사용한다.

목표 흐름:

- 공격자에게 `UMVWeaponComponent`가 있으면 현재 장착 무기 state에서 `FMVWeaponHitSnapshot`을 캡처한다.
- 캡처한 스냅샷의 공격력을 최종 대미지 계산과 `FMVResolvedHitData.WeaponAttackPower` 호환 필드에 반영한다.
- 무기 컴포넌트가 없거나 snapshot이 유효하지 않으면 `Request.WeaponAttackPower`를 폴백 스냅샷으로 사용한다.
- 둘 다 없으면 0으로 처리하고 캐릭터 기본 공격력만 사용한다.

`Request.WeaponAttackPower`는 PIE 디버그와 임시 테스트 경로를 위해 당분간 제거하지 않는다.

## CombatComponent 연동

현재 `CombatComponent`는 `CurrentWeaponStyle`을 기준으로 액션/스킬 테이블을 고른다.

이번 브랜치에서는 구조를 크게 바꾸지 않는다.

- `WeaponComponent` 장착 변경 시 `CombatComponent.ChangeWeapon(NewStyle)` 호출 또는 동등한 갱신 경로를 만든다.
- `ChangeWeapon`은 `CurrentWeaponStyle` 갱신 후 `RefreshActionMaps()`를 호출한다.
- 무기는 개별 공격/스킬 row의 `AbilityReference`와 프로퍼티를 덮어쓰지 않는다.
- 무기별 고유 모션셋과 고유 스킬 귀속은 이번 범위에서 제외하고, 일단 기존 `EquippedStyle` 기반 테이블 선택을 유지한다.
- 이후에는 `BareHand`, `OneHand`, `TwoHand` 분기가 아니라 무기 `ItemTag` 또는 무기 정의의 액션/스킬 참조를 `CombatComponent` 입력으로 전달하는 구조로 바꾼다.

## 발견된 Combat 구조 보완점

아래 항목은 현재 vertical slice를 막는 작업이 아니므로 이번 브랜치에서는 구현하지 않는다. 발표 전에는 문서화와 최소 검증만 하고, 발표 이후 Combat 리팩터링 항목으로 넘긴다.

- [ ] `BareHand`, `OneHand`, `TwoHand`를 액션 테이블 선택 기준으로 쓰는 구조를 제거한다. 기본 로코모션은 공용 애니메이션을 사용하고, 공격/스킬은 무기별 고유 액션 세트가 선택하게 한다.
- [ ] 무기별 액션 세트 식별자는 새 정수 ID나 문자열 ID보다 `FGameplayTag`를 우선 검토한다. 단, DataTable row locator가 필요한 곳은 row name을 별도로 둘 수 있다.
- [ ] 일반공격 row에 시작 손/발, 종료 손/발, 스윙 방향 같은 제작 규칙 메타데이터를 추가할지 검토한다.
- [ ] 약공격, 강공격, 차지공격을 서로 섞어 입력할 때 현재 공격 타입의 체인 인덱스만 보지 말고 이전 공격의 종료 자세와 다음 공격의 시작 자세가 교차되도록 row를 고르는 선택기를 설계한다.
- [ ] 예: 약공격이 `R-L-R-L`, 강공격이 `R-L`일 때 `LightAttack1` 다음 강공격은 `HeavyAttack1`이 아니라 반대 시작점의 `HeavyAttack2`가 선택되어야 한다.
- [ ] 스킬 체인 유효시간은 단순 입력 윈도우가 아니라 타격 성공으로 획득한 스택/강화 상태의 유효시간으로 분리한다.
- [ ] Q 스킬은 요네 Q처럼 적중 시 스택과 유효시간을 갱신하고, 강화 Q 사용 시 유효시간을 초기화하는 별도 런타임 상태가 필요하다.
- [ ] Skill HUD는 최종적으로 `Skill0` row의 입력 가능 시간 자체가 아니라, 스킬 상태가 제공하는 스택 유효시간 remaining/duration을 표시한다.
- [ ] 공격 중 recovery escape가 공격 입력에만 닫혀 있는 한계를 정리하고, Dodge, 일반 이동, 다른 액션 도메인이 어떤 윈도우에서 어떤 우선순위로 캔슬 가능한지 공통 입력 소비 정책을 만든다.
- [ ] 공격 중 방향 회전 가능 구간을 표현하는 AnimNotifyState를 추가한다.
- [ ] 스태미나/MP 비용 소비 시점을 몽타주 시작이 아니라 실제 어빌리티 윈도우 시작으로 옮긴다.
- [ ] 스태미나 회복 재개 시점을 몽타주 종료가 아니라 실제 어빌리티 윈도우 종료로 옮긴다.

## 단계별 작업

### 1단계: 공통 아이템 데이터 껍데기 추가

- [x] `EMVItemType`, `EMVEquipmentSlot` 추가
- [x] `FMVItemTableRow` 추가
- [x] `ItemId` 없이 `FGameplayTag ItemTag`를 primary key로 사용
- [x] 주요 헤더 상단 문서 블록 작성

### 2단계: 무기 데이터와 WeaponComponent 추가

- [x] `FMVWeaponTableRow` 추가
- [x] `EMVWeaponRangeType` 추가
- [x] 무기 row에 메시, 부착 소켓, 부착 오프셋 추가
- [x] `FMVEquippedWeaponState` 추가
- [x] `FMVWeaponHitSnapshot` 추가
- [x] `GrantedAbilityClasses` 없이 기존 Combat row 어빌리티 구조를 사용
- [x] `UMVWeaponComponent` 생성
- [x] 기본 맨손 무기 값 제공
- [x] 현재 무기 state 저장
- [x] 타격 확정용 무기 snapshot 캡처 API 추가
- [x] 장착 무기 메시 부착 경로 추가
- [x] 장착 API와 조회 API 추가
- [x] 장착 변경 이벤트 추가
- [x] `AMVCharacterBase`에서 컴포넌트 생성 및 노출

### 3단계: 무기 대미지 계산 연결

- [x] `UMVHitResolverSubsystem::ResolveWeaponHitSnapshot`이 공격자의 `WeaponComponent`를 조회하게 변경
- [x] `Request.WeaponAttackPower`는 폴백으로 유지
- [x] `FMVResolvedHitData.WeaponSnapshot`에 타격 확정 시점의 무기 snapshot을 기록
- [x] `FMVResolvedHitData.WeaponAttackPower`에 실제 무기 공격력이 기록되는지 확인

### 4단계: 무기 액션 테이블 갱신 연결

- [x] 무기 장착 시 캐릭터 `EquippedStyle` 갱신
- [x] `CombatComponent`의 현재 무기 스타일 갱신 경로 연결
- [ ] 장착 변경 후 기본 공격/스킬 맵 refresh 확인
- [x] `CombatProfileTag`를 제거하고 1차 구현은 `EquippedStyle`, `AttackPower`, `RangeType`, 메시 정보만 유지

### 4.5단계: 플레이어 테스트 무기 장착 경로 추가

- [x] `AMVPlayerCharacter`에 `bEquipTestWeaponOnBeginPlay` 추가
- [x] `AMVPlayerCharacter`에 `TestWeaponRow` DataTable row handle 추가
- [x] `AMVPlayerCharacter`에 `InlineTestWeapon` fallback row 추가
- [x] `EquipConfiguredTestWeapon()`이 확정된 `FMVWeaponTableRow`만 `WeaponComponent`에 전달하게 구현

### 5단계: Healing Potion vertical slice 추가

- [x] `FMVHealingPotionTableRow` 추가
- [x] `FMVHealingPotionRuntimeState` 추가
- [x] `UMVPlayerConsumableComponent` 생성
- [x] `AMVPlayerCharacter`에서 `PlayerConsumableComponent` 생성 및 초기화
- [x] `UseConsumable` 입력을 `PlayerConsumableComponent.TryUseHealingPotion()`으로 연결
- [x] `UMVStatComponent::RecoverHP(float Amount)` 추가
- [x] `MV Apply Healing Potion` notify 추가
- [x] 회복약 사용 액션 row가 있으면 입력 시점에는 몽타주만 시작하고 notify 시점에 HP 회복과 카운트 감소 적용
- [x] 회복약 카운트 변경 이벤트 브로드캐스트
- [x] `MVMainHUDWidget`에서 플레이어 회복약 상태를 `HPSlot`에 바인딩
- [x] 회복약 카운트 변경 시 퀵슬롯 카운트 갱신
- [x] 회복약 카운트가 0이면 퀵슬롯 `bLocked` 처리
- [x] 회복약 액션이 재생 중일 때 플레이어 이동을 걷기로 제한하고 전력질주를 막는다.
- [x] `UMVFieldTransitionSubsystem::ResetPlayerStatsForTransition`에서 회복약 카운트 기본값 복구
- [ ] 회복약 사용 몽타주를 upper body 슬롯으로 만들고 `MV Apply Healing Potion`, `MV Recovery Escape Window` notify를 배치한다.
- [ ] 월드 리셋/사망 부활 후 퀵슬롯 카운트와 잠금 상태가 갱신되는지 확인

### 5.5단계: Skill HUD vertical slice 추가

- [ ] `FMVSkillSlotViewData` 추가
- [ ] `UMVSkillSlotWidget` 추가
- [ ] `UMVSkillHUDWidget` 추가
- [ ] `UMVCombatComponent`에서 Q/R 스킬 쿨타임 view data 제공
- [ ] 이번 브랜치에서 Q 스택은 임시 Combat 상태로 노출할지, 어빌리티 적중 이벤트 기반으로 최소 구현할지 결정
- [ ] Q 스킬 현재/최대 스택 view data 제공
- [ ] Q 스킬 스택 유효시간 remaining/duration view data 제공
- [ ] 장기 구현에서는 Q 스택 유효시간이 입력 윈도우가 아니라 타격 성공으로 갱신되는 스킬 상태임을 주석으로 남긴다.
- [ ] `MVMainHUDWidget`에서 플레이어 `CombatComponent`와 스킬 HUD 바인딩
- [ ] Q/R 스킬 사용 시 쿨타임 UI가 진행되는지 확인
- [ ] Q 스킬 스택 증감이 UI에 반영되는지 확인
- [ ] Q 스킬 스택 유효시간이 HUD에서 감소하고 만료 시 사라지는지 확인

### 6단계: 테스트 데이터와 검증

- [ ] 맨손 기본값으로 기존 공격이 동작하는지 확인
- [ ] 테스트 한손 무기 공격력이 최종 대미지에 반영되는지 확인
- [ ] 테스트 무기 장착 시 현재 호환 경로의 `EquippedStyle` 갱신과 Combat map refresh가 깨지지 않는지 확인
- [ ] 테스트 무기 장착 후 기존 공격/스킬 row의 `AbilityReference` 흐름이 유지되는지 확인
- [ ] `UseConsumable` 입력으로 회복약이 사용되는지 확인
- [ ] `UseConsumable` 키를 한 번 눌렀을 때 회복약이 1개만 소비되는지 확인
- [ ] 회복약 HP 회복과 카운트 감소가 입력 프레임이 아니라 `MV Apply Healing Potion` notify 프레임에서 적용되는지 확인
- [ ] 회복약 몽타주의 recovery escape window에서 연속 회복 또는 다른 액션 전환이 가능한지 확인
- [ ] 회복약 몽타주 재생 중 하체 걷기는 가능하고 달리기/전력질주는 막히는지 확인
- [ ] HP가 최대치여도 회복약이 사용되고 카운트가 감소하는지 확인
- [ ] 회복약 카운트가 0이면 사용되지 않고 퀵슬롯이 비활성화되는지 확인
- [ ] 사망 부활 또는 월드 리셋 이후 회복약 카운트가 기본값으로 복구되는지 확인
- [ ] Q/R 스킬 쿨타임이 HUD에 표시되는지 확인
- [ ] Q 스킬 스택이 HUD에 표시되는지 확인
- [ ] Q 스킬 스택 유효시간이 HUD에 표시되는지 확인
- [ ] PIE 디버그 위젯의 임시 `WeaponAttackPower` 주입 경로가 깨지지 않는지 확인
- [x] 가능한 범위에서 `MaverickEditor Win64 Development` 빌드 실행

## 발표 전 1주일 권장 일정

- 1일차: 공통 아이템 데이터와 `WeaponComponent` 골격 추가
- 2일차: `HitResolver` 대미지 계산 연결
- 3일차: `CombatComponent` 장비 스타일 갱신 연결
- 4일차: 충돌 처리 테스트와 대미지 수치 검증
- 5일차: `PlayerConsumableComponent`, 회복약 사용, 퀵슬롯 카운트 갱신 연결
- 6일차: 월드 리셋 카운트 복구, 테스트 무기 row / 회복약 row / 스킬 HUD / BP 세팅 정리
- 7일차: 회귀 확인과 발표용 안정화만 진행

## 수용 기준

- 무기를 별도로 세팅하지 않아도 맨손 공격으로 기존 전투가 동작한다.
- 장착 무기의 공격력이 `FMVResolvedHitData.WeaponAttackPower`에 반영된다.
- 최종 대미지 계산이 캐릭터 공격력 + 무기 공격력 구조로 유지된다.
- 아이템의 canonical key는 `FGameplayTag ItemTag`이며 별도 `ItemId`를 두지 않는다.
- 테스트 무기 장착 시 현재 호환 경로의 액션 테이블 갱신 흐름이 깨지지 않는다.
- 근거리/원거리 무기 유형을 `EMVWeaponRangeType`으로 구분할 수 있다.
- 무기 row에 메시가 있으면 장착 시 캐릭터 메시 소켓에 무기 메시가 부착된다.
- 무기가 개별 어빌리티 클래스를 중복 소유하지 않고, 공격/스킬 row의 `AbilityReference` 흐름을 유지한다.
- 인벤토리나 UI 없이도 BP 또는 테스트 코드에서 무기 교체를 검증할 수 있다.
- `UseConsumable` 입력으로 회복약을 사용할 수 있다.
- `UseConsumable` 키 입력 한 번에 회복약은 1개만 소비된다.
- 회복약 사용 몽타주의 `MV Apply Healing Potion` notify 시점에 HP가 회복되고 카운트가 1 감소한다.
- 회복약 사용 몽타주의 `MV Recovery Escape Window` 구간에서는 연속 회복 또는 다른 액션 전환이 가능하다.
- 회복약 사용 몽타주 재생 중 플레이어는 걷기만 가능하고 달리기/전력질주는 불가능하다.
- HP가 최대치여도 회복약은 사용되고 카운트가 1 감소한다.
- 회복약 카운트가 0이면 퀵슬롯이 비활성화되고 사용이 실패한다.
- 월드 리셋 또는 사망 부활 이후 회복약 카운트가 기본값으로 복구된다.
- Q/R 스킬 쿨타임이 HUD에서 진행 상태로 표시된다.
- Q 스킬 스택이 HUD에 현재/최대 값으로 표시된다.
- Q 스킬 스택 유효시간이 HUD에서 진행 상태로 표시된다.
- 기존 PIE 디버그 대미지 테스트 경로가 완전히 막히지 않는다.

## 이후 확장 순서

발표 이후에는 아래 순서로 확장한다.

1. 인벤토리 저장소와 아이템 스택
2. 장비 슬롯과 장착/해제 UI
3. 여러 소모품과 퀵슬롯 편집
4. 필드 pickup, 상자, 몬스터 드랍, NPC 보상
5. 회복약 사용 확정 AnimNotify, 사용 중 피격 취소, 소비 롤백 정책
6. 무기 `ItemTag` 또는 무기 정의 기반 고유 모션셋/고유 스킬 연결
7. 공격 시작/종료 자세 메타데이터와 혼합 입력 체인 선택기
8. Q 스킬 적중 기반 스택/강화 상태와 HUD 유효시간 표시 정식화
9. 공격/스킬 row 어빌리티 실행 타이밍 정리
10. 강화/조합/제작
11. 세이브/로드

## 현재 작업 목록

- [x] 아이템 시스템 전체가 아니라 무기 vertical slice로 범위를 확정한다.
- [x] 회복약을 전투 프로토타입용 고정 퀵 아이템 vertical slice로 포함한다.
- [x] `ItemId`/`ItemTag` 중복을 제거하고 `FGameplayTag ItemTag`를 primary key로 정리한다.
- [x] Combat 어빌리티 구조를 확인하고 무기 row가 어빌리티 클래스를 중복 소유하지 않게 정리한다.
- [x] 넓은 `ConsumableTableRow` 대신 `HealingPotionTableRow`로 범위를 축소한다.
- [x] 발표 전 비목표를 명시한다.
- [x] 데이터/런타임/전투/회복약 연동 구조를 문서화한다.
- [x] 1단계 공통 아이템 데이터 껍데기부터 구현한다.
- [x] 2단계 `WeaponComponent`를 구현한다.
- [x] 3단계 `HitResolver`와 연결한다.
- [x] 4단계 `CombatComponent` 장비 스타일 갱신을 연결한다.
- [x] 5단계 `PlayerConsumableComponent`와 회복약 퀵슬롯을 구현한다.
- [x] Combat 구조 보완점을 발표 이후 리팩터링 TODO로 분리한다.
- [ ] 5.5단계 `Skill HUD` Q/R 쿨타임과 Q 스택 표시를 구현한다.
- [ ] 6단계 테스트 무기/회복약과 검증 절차를 완료한다.
