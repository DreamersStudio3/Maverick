#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Struct/MVWeaponTypes.h"
#include "Struct/MVSkillTree.h" 
#include "Tables/MVWeaponTableTypes.h"
#include "Interface/MVActionInputHandlerInterface.h"
#include "DataAsset/MVWeaponSkillMap.h"

#include "MVWeaponComponent.generated.h"

class UMeshComponent;
class UObject;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnEquippedWeaponChanged, const FMVEquippedWeaponState&, WeaponState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnOwnedWeaponsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnQuickSlotsChanged);

/** 무기 종류로 걸러진 패시브 후보의 스킬 행과 현재 해금 상태. */
USTRUCT(BlueprintType)
struct FMVPassiveSkillCandidate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Skill|Passive")
	FDataTableRowHandle SkillRowHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Skill|Passive")
	int32 RequiredPoints = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Skill|Passive")
	bool bUnlocked = false;
};

/**
 * 캐릭터의 무기 소유권, 퀵슬롯, 현재 장착 상태와 스타일별 스킬 진행을 관리하는 컴포넌트.
 *
 * OwnedWeapons를 소유 여부의 단일 정본으로 유지하고 QuickSlotWeaponTags는 그 key만 참조한다.
 * Q/W/E/R 해금·장착은 EMVEquippedStyle별로 공유하고 스킬 포인트는 모든 스타일이 함께 사용한다.
 * 패시브 후보는 스타일 트리에서 무기 종류로 거르고, 선택한 한 행은 종류별로 보관한다.
 * 장착 스타일이 바뀔 때 CharacterBase의 EquippedStyle과 CombatComponent의 액션 맵을 동기화한다.
 * 맨손은 소유 목록과 퀵슬롯에 넣지 않는 내부 fallback이며, 실제 무기 메시에는 필수 Trace 소켓이 있어야 한다.
 *
 * 라이프사이클:
 *   1) 생성자 -> 기본 맨손 무기 row 값을 준비한다.
 *   2) BeginPlay -> 스킬맵과 진행 상태를 준비하고, 저장 상태가 없을 때만 DefaultWeaponRow 또는 맨손을 장착한다.
 *   3) 공개 장착은 CycleWeapon으로 제한하고, 퀵슬롯이 참조하는 row를 내부에서 검증한 뒤 관련 상태와 이벤트를 갱신한다.
 *   4) HitResolver가 타격 확정 시점에 CaptureWeaponHitSnapshot으로 히트 전용 사본을 만든다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVWeaponComponent : public UActorComponent, public IMVActionInputHandlerInterface
{
	GENERATED_BODY()

public:
	UMVWeaponComponent();

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon|Quick Slot")
	bool CycleWeapon();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon|Ownership")
	bool AcquireWeapon(const FDataTableRowHandle& WeaponRowHandle);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon|Ownership")
	bool DiscardWeapon(FGameplayTag WeaponTag);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon|Quick Slot")
	bool SetQuickSlotWeapon(int32 SlotIndex, FGameplayTag WeaponTag);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon|Quick Slot")
	bool ClearQuickSlotWeapon(int32 SlotIndex);

	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon|Ownership")
	TMap<FGameplayTag, FDataTableRowHandle> GetOwnedWeapons() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon|Ownership")
	bool IsWeaponOwned(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon|Quick Slot")
	TArray<FGameplayTag> GetQuickSlotWeapons() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon|Quick Slot")
	int32 GetActiveQuickSlotIndex() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	FMVEquippedWeaponState GetEquippedWeaponState() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	FMVWeaponHitSnapshot CaptureWeaponHitSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	float GetEquippedWeaponAttackPower() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	EMVEquippedStyle GetEquippedStyle() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	EMVWeaponRangeType GetWeaponRangeType() const;

	// Skill Tree Management Functions
	UFUNCTION(BlueprintPure, Category = "Maverick|Skill")
	bool GetWeaponSkillState(EMVEquippedStyle EquippedStyle, FMVSkillTreeState& Output) const;
	
	UFUNCTION(BlueprintPure, Category = "Maverick|Skill")
	bool GetSkillTree(EMVEquippedStyle EquippedStyle, UMVWeaponSkillMap*& Output) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Combat")
	bool GetBasicAttackRow(
		EMVEquippedStyle EquippedStyle,
		EMVCombatActionTypes ActionType,
		FDataTableRowHandle& Output) const;
	
	UFUNCTION(BlueprintPure, Category = "Maverick|Skill")
	const TMap<EMVSkillSlot, FDataTableRowHandle> GetEquippedSkill(EMVEquippedStyle EquippedStyle = EMVEquippedStyle::None) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Skill")
	bool GetEquippedSkillRow(
		EMVEquippedStyle EquippedStyle,
		EMVSkillSlot SkillSlot,
		FDataTableRowHandle& Output) const;
	
	UFUNCTION(BlueprintCallable, Category = "Maverick|Skill")
	bool UnlockSkill(EMVEquippedStyle EquippedStyle, FDataTableRowHandle RowHandle);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Skill")
	bool EquipSkill(EMVEquippedStyle EquippedStyle, FDataTableRowHandle RowHandle);

	UFUNCTION(BlueprintPure, Category = "Maverick|Skill|Passive")
	TArray<FMVPassiveSkillCandidate> GetPassiveCandidates(FGameplayTag WeaponTypeTag) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Skill|Passive")
	bool GetSelectedPassive(FGameplayTag WeaponTypeTag, FDataTableRowHandle& Output) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Skill|Passive")
	bool GetEquippedWeaponPassiveRow(FDataTableRowHandle& Output) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Skill|Passive")
	bool SelectPassive(FGameplayTag WeaponTypeTag, FDataTableRowHandle RowHandle);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Skill|Passive")
	bool ClearSelectedPassive(FGameplayTag WeaponTypeTag);

	UFUNCTION(BlueprintPure, Category = "Maverick|Skill")
	int32 GetSkillPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Skill")
	bool AddSkillPoints(int32 Points);

public:
	UPROPERTY(BlueprintAssignable, Category = "Maverick|Weapon|Event")
	FMVOnEquippedWeaponChanged OnEquippedWeaponChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Weapon|Event")
	FMVOnOwnedWeaponsChanged OnOwnedWeaponsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Weapon|Event")
	FMVOnQuickSlotsChanged OnQuickSlotsChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Default")
	FMVWeaponTableRow BareHandWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Default")
	FDataTableRowHandle DefaultWeaponRow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Weapon|Quick Slot", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxQuickSlotCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Visual")
	bool bManageWeaponMesh = true;

	// Per-style static definitions for basic attacks and skill candidates. Set in Editor or Blueprint.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Maverick|Combat")
	TArray<TObjectPtr<UMVWeaponSkillMap>> WeaponSkillMaps;

private:
	bool EquipQuickSlotWeapon(int32 SlotIndex);
	bool TryEquipWeaponTag(const FGameplayTag& WeaponTag, int32 QuickSlotIndex);
	bool TryEquipWeaponRow(const FMVWeaponTableRow& WeaponRow, int32 QuickSlotIndex);
	const FMVWeaponTableRow* ResolveWeaponRow(const FDataTableRowHandle& RowHandle, const TCHAR* Context) const;
	int32 FindQuickSlotIndexForItemTag(const FGameplayTag& ItemTag) const;
	void EnsureQuickSlotStorage();
	void EquipBareHand();
	void ApplyEquippedWeaponState(const FMVEquippedWeaponState& NewState);
	static FMVEquippedWeaponState MakeStateFromWeaponRow(const FMVWeaponTableRow& WeaponRow);
	static FMVWeaponHitSnapshot MakeHitSnapshotFromState(const FMVEquippedWeaponState& WeaponState);
	bool CanEquipWeaponState(const FMVEquippedWeaponState& WeaponState) const;
	bool TryInitializeDefaultWeapon();
	bool ValidateWeaponMesh(const UObject& WeaponMesh, const FGameplayTag& ItemTag) const;
	bool ValidateWeaponTraceSockets(const UObject& WeaponMesh, const FGameplayTag& ItemTag) const;
	void SyncOwnerEquippedStyle() const;
	void RefreshOwnerCombatComponent() const;
	void RefreshOwnerCombatSkillSlot(EMVSkillSlot SkillSlot) const;
	void ApplyWeaponVisual();
	void ClearWeaponVisual();
	void ApplyWeaponMeshVisual(
		UObject* LoadedWeaponMesh,
		TObjectPtr<UMeshComponent>& MeshComponentStorage,
		FName AttachSocketName,
		const FTransform& RelativeTransform,
		const TCHAR* ComponentNameBase);
	void ClearWeaponMeshVisual(TObjectPtr<UMeshComponent>& MeshComponentStorage);
	UMeshComponent* EnsureWeaponMeshComponent(
		TObjectPtr<UMeshComponent>& MeshComponentStorage,
		const UObject& WeaponMesh,
		const TCHAR* ComponentNameBase);
	USceneComponent* ResolveWeaponAttachParent() const;

	// When the SaveGame doesn't exist for SkillTreeStates, this function initializes the SkillTreeStates with default values based on the WeaponSkillMaps.
	void InitSkillTreeStates();
	void InitSkillSlotState(
		FMVSkillTreeState& SkillTreeState,
		EMVSkillSlot SkillSlot,
		const TSet<FMVWeaponSkillMapData>& Skills) const;
	bool ResolveOwnedWeaponTypeStyle(FGameplayTag WeaponTypeTag, EMVEquippedStyle& OutputStyle) const;
	bool IsValidPassiveCandidateRow(const FDataTableRowHandle& RowHandle) const;
	
	// Todo: Apply Skill to CombatComponent. This function will be responsible for applying the equipped skills to the character's combat component, allowing the character to use the skills in combat.
	//void ApplySkillToCombatComponent();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon", meta = (AllowPrivateAccess = "true"))
	FMVEquippedWeaponState CurrentWeaponState;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Ownership", meta = (AllowPrivateAccess = "true"))
	TMap<FGameplayTag, FDataTableRowHandle> OwnedWeapons;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Quick Slot", meta = (AllowPrivateAccess = "true"))
	TArray<FGameplayTag> QuickSlotWeaponTags;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Quick Slot", meta = (AllowPrivateAccess = "true"))
	int32 ActiveQuickSlotIndex = INDEX_NONE;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> WeaponMeshComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> SecondaryWeaponMeshComponent;

	// Runtime lookup from equipped style to its static basic attack and skill definition.
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Skill", meta = (AllowPrivateAccess = "true"))
	TMap<EMVEquippedStyle, TObjectPtr<UMVWeaponSkillMap>> SkillBlueprint;

	// SkillTreeStates is current state of the skill tree for each equipped style, which can be used to track the player's equipped skills and unlocks.
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Skill", meta = (AllowPrivateAccess = "true"))
	TMap<EMVEquippedStyle, FMVSkillTreeState> SkillTreeStates;

	// Current session selection; later save data must persist this by weapon kind.
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Skill|Passive", meta = (AllowPrivateAccess = "true"))
	TMap<FGameplayTag, FDataTableRowHandle> SelectedPassiveByWeaponType;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Skill", meta = (AllowPrivateAccess = "true"))
	int32 CurrentSkillPoints = 0;
};
