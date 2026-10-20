#include "Components/MVWeaponComponent.h"

#include "MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Tags/MVGameplayTags.h"

namespace
{
constexpr const TCHAR* MVWeaponTraceStartSocketName = TEXT("Trace_Start");
constexpr const TCHAR* MVWeaponTraceEndSocketName = TEXT("Trace_End");
constexpr const TCHAR* MVWeaponTraceLeftSocketName = TEXT("Trace_Left");
constexpr const TCHAR* MVWeaponTraceRightSocketName = TEXT("Trace_Right");

TArray<FName> MVWeaponMakeRequiredTraceSocketNames()
{
	return {
		FName(MVWeaponTraceStartSocketName),
		FName(MVWeaponTraceEndSocketName),
		FName(MVWeaponTraceLeftSocketName),
		FName(MVWeaponTraceRightSocketName)
	};
}
}

UMVWeaponComponent::UMVWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BareHandWeapon.ItemTag = MVGameplayTags::Item_Weapon_BareHand;
	BareHandWeapon.DisplayName = NSLOCTEXT("MaverickWeapon", "BareHandDisplayName", "Bare Hand");
	BareHandWeapon.EquippedStyle = EMVEquippedStyle::BareHand;
	BareHandWeapon.AttackPower = 0.0f;
	BareHandWeapon.RangeType = EMVWeaponRangeType::Melee;
	BareHandWeapon.AttachSocketName = TEXT("hand_r_socket");

	

}

void UMVWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureQuickSlotStorage();

	for (UMVWeaponSkillMap* OneSkillMap : WeaponSkillMaps)
	{
		if (!OneSkillMap)
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponSkillMaps contains a null skill map. Owner=%s."), *GetNameSafe(GetOwner()));
			continue;
		}

		if (SkillBlueprint.Contains(OneSkillMap->WeaponStyle))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("Duplicate weapon skill map style ignored. Owner=%s Style=%d SkillMap=%s."),
				*GetNameSafe(GetOwner()),
				static_cast<int32>(OneSkillMap->WeaponStyle),
				*GetNameSafe(OneSkillMap));
			continue;
		}

		// Creating Search Map along with WeaponSkillMaps for each WeaponStyle
		SkillBlueprint.Add(OneSkillMap->WeaponStyle, OneSkillMap);

		// Initialize SkillTreeStates for each WeaponStyle
		FMVSkillTreeState NewState;
		NewState.EquippedStyle = OneSkillMap->WeaponStyle;
		SkillTreeStates.Add(OneSkillMap->WeaponStyle, NewState);
	}

	// Todo: Replace this disabled condition with a successful SaveGame load check.
	if (0 /* Valid SaveGame loaded */)
	{
		// Todo: Restore owned weapons, quick slots, equipped weapon, skill tree states,
		// and shared skill points before applying the loaded weapon state.
	}
	else
	{
		InitSkillTreeStates();
		if (!TryInitializeDefaultWeapon())
		{
			EquipBareHand();
		}
	}

	if (const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
		{
			InputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::Weapon);
		}
	}
}

void UMVWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
		{
			InputManager->UnregisterActionInputHandler(this);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

bool UMVWeaponComponent::CycleWeapon()
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->ActionComponent && OwnerCharacter->ActionComponent->IsActionRunning())
	{
		return false;
	}

	const int32 QuickSlotCount = QuickSlotWeaponTags.Num();
	if (QuickSlotCount == 0)
	{
		return false;
	}

	int32 CurrentIndex = ActiveQuickSlotIndex;
	if (!QuickSlotWeaponTags.IsValidIndex(CurrentIndex))
	{
		CurrentIndex = FindQuickSlotIndexForItemTag(CurrentWeaponState.ItemTag);
	}

	const int32 NextIndex = QuickSlotWeaponTags.IsValidIndex(CurrentIndex)
		? (CurrentIndex + 1) % QuickSlotCount
		: 0;
	const FGameplayTag& NextWeaponTag = QuickSlotWeaponTags[NextIndex];
	if (!NextWeaponTag.IsValid())
	{
		ActiveQuickSlotIndex = NextIndex;
		if (CurrentWeaponState.ItemTag.MatchesTagExact(BareHandWeapon.ItemTag))
		{
			return false;
		}

		EquipBareHand();
		return true;
	}

	if (NextWeaponTag.MatchesTagExact(CurrentWeaponState.ItemTag))
	{
		ActiveQuickSlotIndex = NextIndex;
		return false;
	}

	return EquipQuickSlotWeapon(NextIndex);
}

bool UMVWeaponComponent::AcquireWeapon(const FDataTableRowHandle& WeaponRowHandle)
{
	const FMVWeaponTableRow* WeaponRow = ResolveWeaponRow(
		WeaponRowHandle,
		TEXT("UMVWeaponComponent::AcquireWeapon"));
	if (!WeaponRow || !WeaponRow->ItemTag.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Cannot acquire weapon because its row or ItemTag is invalid. DataTable=%s Row=%s."),
			*GetNameSafe(WeaponRowHandle.DataTable),
			*WeaponRowHandle.RowName.ToString());
		return false;
	}

	if (OwnedWeapons.Contains(WeaponRow->ItemTag))
	{
		return false;
	}

	OwnedWeapons.Add(WeaponRow->ItemTag, WeaponRowHandle);
	OnOwnedWeaponsChanged.Broadcast();
	return true;
}

bool UMVWeaponComponent::DiscardWeapon(const FGameplayTag WeaponTag)
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->ActionComponent && OwnerCharacter->ActionComponent->IsActionRunning())
	{
		return false;
	}

	if (!WeaponTag.IsValid() || !OwnedWeapons.Contains(WeaponTag))
	{
		return false;
	}

	bool bRemovedQuickSlotReference = false;
	for (FGameplayTag& QuickSlotWeaponTag : QuickSlotWeaponTags)
	{
		if (QuickSlotWeaponTag.MatchesTagExact(WeaponTag))
		{
			QuickSlotWeaponTag = FGameplayTag();
			bRemovedQuickSlotReference = true;
		}
	}

	OwnedWeapons.Remove(WeaponTag);
	if (CurrentWeaponState.ItemTag.MatchesTagExact(WeaponTag))
	{
		EquipBareHand();
	}

	if (bRemovedQuickSlotReference)
	{
		OnQuickSlotsChanged.Broadcast();
	}
	OnOwnedWeaponsChanged.Broadcast();
	return true;
}

bool UMVWeaponComponent::SetQuickSlotWeapon(const int32 SlotIndex, const FGameplayTag WeaponTag)
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->ActionComponent && OwnerCharacter->ActionComponent->IsActionRunning())
	{
		return false;
	}

	EnsureQuickSlotStorage();
	if (!QuickSlotWeaponTags.IsValidIndex(SlotIndex) || !WeaponTag.IsValid() || !OwnedWeapons.Contains(WeaponTag))
	{
		return false;
	}

	const int32 ExistingSlotIndex = FindQuickSlotIndexForItemTag(WeaponTag);
	if (ExistingSlotIndex != INDEX_NONE)
	{
		return false;
	}

	const FGameplayTag PreviousWeaponTag = QuickSlotWeaponTags[SlotIndex];
	QuickSlotWeaponTags[SlotIndex] = WeaponTag;
	if (ActiveQuickSlotIndex == SlotIndex && !TryEquipWeaponTag(WeaponTag, SlotIndex))
	{
		QuickSlotWeaponTags[SlotIndex] = PreviousWeaponTag;
		return false;
	}

	OnQuickSlotsChanged.Broadcast();
	return true;
}

bool UMVWeaponComponent::ClearQuickSlotWeapon(const int32 SlotIndex)
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->ActionComponent && OwnerCharacter->ActionComponent->IsActionRunning())
	{
		return false;
	}

	if (!QuickSlotWeaponTags.IsValidIndex(SlotIndex) || !QuickSlotWeaponTags[SlotIndex].IsValid())
	{
		return false;
	}

	QuickSlotWeaponTags[SlotIndex] = FGameplayTag();
	if (ActiveQuickSlotIndex == SlotIndex)
	{
		EquipBareHand();
	}

	OnQuickSlotsChanged.Broadcast();
	return true;
}

bool UMVWeaponComponent::EquipQuickSlotWeapon(const int32 SlotIndex)
{
	if (!QuickSlotWeaponTags.IsValidIndex(SlotIndex) || !QuickSlotWeaponTags[SlotIndex].IsValid())
	{
		return false;
	}

	return TryEquipWeaponTag(QuickSlotWeaponTags[SlotIndex], SlotIndex);
}

bool UMVWeaponComponent::TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput,
	bool bHasMovementInput)
{
	if (!ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_WeaponSwap))
	{
		return false;
	}
	
	CycleWeapon();
	
	return true;
}

TMap<FGameplayTag, FDataTableRowHandle> UMVWeaponComponent::GetOwnedWeapons() const
{
	return OwnedWeapons;
}

bool UMVWeaponComponent::IsWeaponOwned(const FGameplayTag WeaponTag) const
{
	return WeaponTag.IsValid() && OwnedWeapons.Contains(WeaponTag);
}

TArray<FGameplayTag> UMVWeaponComponent::GetQuickSlotWeapons() const
{
	return QuickSlotWeaponTags;
}

int32 UMVWeaponComponent::GetActiveQuickSlotIndex() const
{
	return ActiveQuickSlotIndex;
}

FMVEquippedWeaponState UMVWeaponComponent::GetEquippedWeaponState() const
{
	return CurrentWeaponState;
}

FMVWeaponHitSnapshot UMVWeaponComponent::CaptureWeaponHitSnapshot() const
{
	return MakeHitSnapshotFromState(CurrentWeaponState);
}

float UMVWeaponComponent::GetEquippedWeaponAttackPower() const
{
	return FMath::Max(0.0f, CurrentWeaponState.AttackPower);
}

EMVEquippedStyle UMVWeaponComponent::GetEquippedStyle() const
{
	return CurrentWeaponState.EquippedStyle;
}

EMVWeaponRangeType UMVWeaponComponent::GetWeaponRangeType() const
{
	return CurrentWeaponState.RangeType;
}

bool UMVWeaponComponent::GetWeaponSkillState(EMVEquippedStyle EquippedStyle, FMVSkillTreeState& Output) const
{
	const FMVSkillTreeState* Result = SkillTreeStates.Find(EquippedStyle);
	if(Result)
	{
		Output = *Result;
		return true;
	}

	Output = FMVSkillTreeState();
	return false;
}

bool UMVWeaponComponent::GetSkillTree(EMVEquippedStyle EquippedStyle, UMVWeaponSkillMap*& Output) const
{
	const TObjectPtr<UMVWeaponSkillMap>* Result = SkillBlueprint.Find(EquippedStyle);

	if(Result && *Result)
	{
		Output = Result->Get();
		return true;
	}

	Output = nullptr;
	return false;
}

// Return Type is Copy TMap. But the number's of element is small and value is FDataTableRowHandle which is small struct. So Copying is not expensive. 
const TMap<EMVSkillSlot, FDataTableRowHandle> UMVWeaponComponent::GetEquippedSkill(EMVEquippedStyle EquippedStyle) const
{
	FMVSkillTreeState State;
	TMap<EMVSkillSlot, FDataTableRowHandle> Result;
	EMVEquippedStyle StyleToCheck;
	
	// If EquippedStyle is None, use the current weapon's equipped style to get the skill tree state.
	if(EquippedStyle == EMVEquippedStyle::None)
	{
		StyleToCheck = CurrentWeaponState.EquippedStyle;
	}
	else
	{
		StyleToCheck = EquippedStyle;
	}
	
	if (GetWeaponSkillState(StyleToCheck, State))
	{
		Result = State.EquippedSkill;
	}
	
	// If the skill tree state is not found, return an empty map
	return Result;
}

bool UMVWeaponComponent::UnlockSkill(EMVEquippedStyle EquippedStyle, FDataTableRowHandle RowHandle)
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return false;
	}

	TObjectPtr<UMVWeaponSkillMap>* SkillMapPtr = SkillBlueprint.Find(EquippedStyle);
	
	// If the skill map is not found for the given equipped style, return false
	if(SkillMapPtr == nullptr || *SkillMapPtr == nullptr)
	{
		return false;
	}
	
	EMVSkillSlot SkillSlot = SkillMapPtr->Get()->FindSkillSlot(RowHandle);
	
	// If the skill slot is NONE, it means the skill is not found in the skill map for the given equipped style
	if(SkillSlot == EMVSkillSlot::NONE)
	{
		return false;
	}

	// If the skill is already unlocked, return false
	FMVSkillTreeState* SkillTreeStatePtr = SkillTreeStates.Find(EquippedStyle);
	if (SkillTreeStatePtr == nullptr || SkillTreeStatePtr->UnlockedSkills.Contains(FMVHashDataTableRowHandle(RowHandle)))
	{
		return false;
	}

	const int32 RequiredPoints = SkillMapPtr->Get()->GetRequiredPointsForSkill(RowHandle);
	const int32 SkillCost = FMath::Max(0, RequiredPoints);
	if (CurrentSkillPoints < SkillCost)
	{
		return false;
	}

	SkillTreeStatePtr->UnlockedSkills.Add(FMVHashDataTableRowHandle(RowHandle));
	CurrentSkillPoints -= SkillCost;

	return true;
}

bool UMVWeaponComponent::EquipSkill(EMVEquippedStyle EquippedStyle, FDataTableRowHandle RowHandle)
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return false;
	}

	TObjectPtr<UMVWeaponSkillMap>* SkillMapPtr = SkillBlueprint.Find(EquippedStyle);

	// If the skill map is not found for the given equipped style, return false
	if (SkillMapPtr == nullptr || *SkillMapPtr == nullptr)
	{
		return false;
	}

	EMVSkillSlot SkillSlot = SkillMapPtr->Get()->FindSkillSlot(RowHandle);

	// If the skill slot is NONE, it means the skill is not found in the skill map for the given equipped style
	if (SkillSlot == EMVSkillSlot::NONE)
	{
		return false;
	}

	// Check if the skill is unlocked before equipping
	FMVSkillTreeState* SkillTreeStatePtr = SkillTreeStates.Find(EquippedStyle);
	if (SkillTreeStatePtr == nullptr
		|| !SkillTreeStatePtr->UnlockedSkills.Contains(FMVHashDataTableRowHandle(RowHandle)))
	{
		return false;
	}

	if (const FDataTableRowHandle* EquippedSkill = SkillTreeStatePtr->EquippedSkill.Find(SkillSlot);
		EquippedSkill && *EquippedSkill == RowHandle)
	{
		return true;
	}

	// Equip the skill by adding it to the EquippedSkill map
	SkillTreeStatePtr->EquippedSkill.Add(SkillSlot, RowHandle);

	if (EquippedStyle == CurrentWeaponState.EquippedStyle)
	{
		// Todo: Replace this full refresh with a slot-level CombatComponent API for SkillSlot.
		RefreshOwnerCombatComponent();
	}

	return true;
}

int32 UMVWeaponComponent::GetSkillPoints() const
{
	return CurrentSkillPoints;
}

bool UMVWeaponComponent::AddSkillPoints(const int32 Points)
{
	if (Points <= 0 || Points > MAX_int32 - CurrentSkillPoints)
	{
		return false;
	}

	CurrentSkillPoints += Points;
	return true;
}

bool UMVWeaponComponent::TryEquipWeaponTag(const FGameplayTag& WeaponTag, const int32 QuickSlotIndex)
{
	const FDataTableRowHandle* WeaponRowHandle = OwnedWeapons.Find(WeaponTag);
	if (!WeaponRowHandle)
	{
		return false;
	}

	const FMVWeaponTableRow* WeaponRow = ResolveWeaponRow(
		*WeaponRowHandle,
		TEXT("UMVWeaponComponent::TryEquipWeaponTag"));
	if (!WeaponRow || !WeaponRow->ItemTag.MatchesTagExact(WeaponTag))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Owned weapon row no longer matches its key. Key=%s DataTable=%s Row=%s."),
			*WeaponTag.ToString(),
			*GetNameSafe(WeaponRowHandle->DataTable),
			*WeaponRowHandle->RowName.ToString());
		return false;
	}

	return TryEquipWeaponRow(*WeaponRow, QuickSlotIndex);
}

bool UMVWeaponComponent::TryEquipWeaponRow(const FMVWeaponTableRow& WeaponRow, const int32 QuickSlotIndex)
{
	FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(WeaponRow);
	if (!NewState.bValid || !CanEquipWeaponState(NewState))
	{
		return false;
	}

	ActiveQuickSlotIndex =
		QuickSlotWeaponTags.IsValidIndex(QuickSlotIndex)
		&& QuickSlotWeaponTags[QuickSlotIndex].MatchesTagExact(WeaponRow.ItemTag)
			? QuickSlotIndex
			: INDEX_NONE;

	ApplyEquippedWeaponState(NewState);
	return true;
}

const FMVWeaponTableRow* UMVWeaponComponent::ResolveWeaponRow(
	const FDataTableRowHandle& RowHandle,
	const TCHAR* Context) const
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Weapon row handle is invalid. Context=%s DataTable=%s Row=%s."),
			Context,
			*GetNameSafe(RowHandle.DataTable),
			*RowHandle.RowName.ToString());
		return nullptr;
	}

	return RowHandle.DataTable->FindRow<FMVWeaponTableRow>(
		RowHandle.RowName,
		Context,
		true);
}

int32 UMVWeaponComponent::FindQuickSlotIndexForItemTag(const FGameplayTag& ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < QuickSlotWeaponTags.Num(); ++Index)
	{
		if (QuickSlotWeaponTags[Index].MatchesTagExact(ItemTag))
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void UMVWeaponComponent::EnsureQuickSlotStorage()
{
	QuickSlotWeaponTags.SetNum(FMath::Max(0, MaxQuickSlotCount));
}

void UMVWeaponComponent::EquipBareHand()
{
	FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(BareHandWeapon);
	NewState.bValid = true;
	ApplyEquippedWeaponState(NewState);
}

void UMVWeaponComponent::ApplyEquippedWeaponState(const FMVEquippedWeaponState& NewState)
{
	const bool bShouldRefreshCombat =
		!CurrentWeaponState.bValid
		|| CurrentWeaponState.EquippedStyle != NewState.EquippedStyle;

	CurrentWeaponState = NewState;
	CurrentWeaponState.AttackPower = FMath::Max(0.0f, CurrentWeaponState.AttackPower);
	CurrentWeaponState.TraceRadius = FMath::Max(0.0f, CurrentWeaponState.TraceRadius);
	CurrentWeaponState.bValid = true;

	SyncOwnerEquippedStyle();
	if (bShouldRefreshCombat)
	{
		RefreshOwnerCombatComponent();
	}
	ApplyWeaponVisual();
	OnEquippedWeaponChanged.Broadcast(CurrentWeaponState);
}

FMVEquippedWeaponState UMVWeaponComponent::MakeStateFromWeaponRow(const FMVWeaponTableRow& WeaponRow)
{
	FMVEquippedWeaponState State;
	State.ItemTag = WeaponRow.ItemTag;
	State.EquippedStyle = WeaponRow.EquippedStyle;
	State.AttackPower = FMath::Max(0.0f, WeaponRow.AttackPower);
	State.WeaponPoise = FMath::Max(0.0f, WeaponRow.WeaponPoise);
	State.RangeType = WeaponRow.RangeType;
	State.WeaponMesh = WeaponRow.WeaponMesh;
	State.AttachSocketName = WeaponRow.AttachSocketName;
	State.AttachTransform = WeaponRow.AttachTransform;
	State.SecondaryWeaponMesh = WeaponRow.SecondaryWeaponMesh;
	State.SecondaryAttachSocketName = WeaponRow.SecondaryAttachSocketName;
	State.SecondaryAttachTransform = WeaponRow.SecondaryAttachTransform;
	State.TraceRadius = FMath::Max(0.0f, WeaponRow.TraceRadius);
	State.bValid = WeaponRow.ItemTag.IsValid();
	return State;
}

FMVWeaponHitSnapshot UMVWeaponComponent::MakeHitSnapshotFromState(const FMVEquippedWeaponState& WeaponState)
{
	FMVWeaponHitSnapshot Snapshot;
	Snapshot.ItemTag = WeaponState.ItemTag;
	Snapshot.EquippedStyle = WeaponState.EquippedStyle;
	Snapshot.AttackPower = FMath::Max(0.0f, WeaponState.AttackPower);
	Snapshot.RangeType = WeaponState.RangeType;
	Snapshot.bValid = WeaponState.bValid;
	return Snapshot;
}

bool UMVWeaponComponent::CanEquipWeaponState(const FMVEquippedWeaponState& WeaponState) const
{
	if (WeaponState.EquippedStyle == EMVEquippedStyle::BareHand)
	{
		return true;
	}

	if (WeaponState.WeaponMesh.IsNull())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Cannot equip weapon because it has no mesh. ItemTag=%s."),
			*WeaponState.ItemTag.ToString());
		return false;
	}

	UObject* LoadedWeaponMesh = WeaponState.WeaponMesh.LoadSynchronous();
	if (!LoadedWeaponMesh)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Cannot equip weapon because mesh failed to load. ItemTag=%s Mesh=%s."),
			*WeaponState.ItemTag.ToString(),
			*WeaponState.WeaponMesh.ToString());
		return false;
	}

	if (!ValidateWeaponMesh(*LoadedWeaponMesh, WeaponState.ItemTag))
	{
		return false;
	}

	if (!WeaponState.SecondaryWeaponMesh.IsNull())
	{
		UObject* LoadedSecondaryWeaponMesh = WeaponState.SecondaryWeaponMesh.LoadSynchronous();
		if (!LoadedSecondaryWeaponMesh)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("Cannot equip weapon because secondary mesh failed to load. ItemTag=%s Mesh=%s."),
				*WeaponState.ItemTag.ToString(),
				*WeaponState.SecondaryWeaponMesh.ToString());
			return false;
		}

		if (!ValidateWeaponMesh(*LoadedSecondaryWeaponMesh, WeaponState.ItemTag))
		{
			return false;
		}
	}

	return true;
}

bool UMVWeaponComponent::TryInitializeDefaultWeapon()
{
	const FMVWeaponTableRow* WeaponRow = ResolveWeaponRow(
		DefaultWeaponRow,
		TEXT("UMVWeaponComponent::TryInitializeDefaultWeapon"));
	if (!WeaponRow || !WeaponRow->ItemTag.IsValid())
	{
		return false;
	}

	const FGameplayTag DefaultWeaponTag = WeaponRow->ItemTag;
	if (!AcquireWeapon(DefaultWeaponRow) && !OwnedWeapons.Contains(DefaultWeaponTag))
	{
		return false;
	}

	if (!SetQuickSlotWeapon(0, DefaultWeaponTag)
		&& (!QuickSlotWeaponTags.IsValidIndex(0)
			|| !QuickSlotWeaponTags[0].MatchesTagExact(DefaultWeaponTag)))
	{
		return false;
	}

	return EquipQuickSlotWeapon(0);
}

bool UMVWeaponComponent::ValidateWeaponMesh(const UObject& WeaponMesh, const FGameplayTag& ItemTag) const
{
	if (!WeaponMesh.IsA<USkeletalMesh>() && !WeaponMesh.IsA<UStaticMesh>())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Cannot equip weapon because mesh asset is not SkeletalMesh or StaticMesh. ItemTag=%s Mesh=%s Class=%s."),
			*ItemTag.ToString(),
			*WeaponMesh.GetName(),
			*GetNameSafe(WeaponMesh.GetClass()));
		return false;
	}

	return ValidateWeaponTraceSockets(WeaponMesh, ItemTag);
}

bool UMVWeaponComponent::ValidateWeaponTraceSockets(
	const UObject& WeaponMesh,
	const FGameplayTag& ItemTag) const
{
	TArray<FName> MissingSocketNames;
	for (const FName SocketName : MVWeaponMakeRequiredTraceSocketNames())
	{
		bool bHasSocket = false;
		if (const USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(&WeaponMesh))
		{
			bHasSocket = SkeletalMesh->FindSocket(SocketName) != nullptr;
		}
		else if (const UStaticMesh* StaticMesh = Cast<UStaticMesh>(&WeaponMesh))
		{
			bHasSocket = StaticMesh->FindSocket(SocketName) != nullptr;
		}

		if (!bHasSocket)
		{
			MissingSocketNames.Add(SocketName);
		}
	}

	if (MissingSocketNames.IsEmpty())
	{
		return true;
	}

	TArray<FString> MissingSocketStrings;
	for (const FName SocketName : MissingSocketNames)
	{
		MissingSocketStrings.Add(SocketName.ToString());
	}

	UE_LOG(
		LogTemp,
		Error,
		TEXT("Weapon mesh is missing required trace sockets. ItemTag=%s Mesh=%s Missing=%s."),
		*ItemTag.ToString(),
		*WeaponMesh.GetName(),
		*FString::Join(MissingSocketStrings, TEXT(", ")));
	return false;
}

void UMVWeaponComponent::SyncOwnerEquippedStyle() const
{
	if (AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		OwnerCharacter->SetEquippedStyle(CurrentWeaponState.EquippedStyle);
	}
}

void UMVWeaponComponent::RefreshOwnerCombatComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		if (UMVCombatComponent* CombatComponent = Owner->FindComponentByClass<UMVCombatComponent>())
		{
			CombatComponent->ChangeWeapon(CurrentWeaponState.EquippedStyle);
		}
	}
}

void UMVWeaponComponent::ApplyWeaponVisual()
{
	if (!bManageWeaponMesh)
	{
		return;
	}

	if (CurrentWeaponState.WeaponMesh.IsNull())
	{
		if (CurrentWeaponState.ItemTag.IsValid() && CurrentWeaponState.EquippedStyle != EMVEquippedStyle::BareHand)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Equipped weapon has no visual mesh. ItemTag=%s."),
				*CurrentWeaponState.ItemTag.ToString());
		}
		ClearWeaponVisual();
		return;
	}

	UObject* LoadedWeaponMesh = CurrentWeaponState.WeaponMesh.LoadSynchronous();
	if (!LoadedWeaponMesh)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Failed to load weapon mesh. ItemTag=%s Mesh=%s."),
			*CurrentWeaponState.ItemTag.ToString(),
			*CurrentWeaponState.WeaponMesh.ToString());
		ClearWeaponVisual();
		return;
	}
	if (!ValidateWeaponMesh(*LoadedWeaponMesh, CurrentWeaponState.ItemTag))
	{
		ClearWeaponVisual();
		return;
	}

	ApplyWeaponMeshVisual(
		LoadedWeaponMesh,
		WeaponMeshComponent,
		CurrentWeaponState.AttachSocketName,
		CurrentWeaponState.AttachTransform,
		TEXT("EquippedWeaponMeshComponent"));

	if (CurrentWeaponState.SecondaryWeaponMesh.IsNull())
	{
		ClearWeaponMeshVisual(SecondaryWeaponMeshComponent);
		return;
	}

	UObject* LoadedSecondaryWeaponMesh = CurrentWeaponState.SecondaryWeaponMesh.LoadSynchronous();
	if (!LoadedSecondaryWeaponMesh)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Failed to load secondary weapon mesh. ItemTag=%s Mesh=%s."),
			*CurrentWeaponState.ItemTag.ToString(),
			*CurrentWeaponState.SecondaryWeaponMesh.ToString());
		ClearWeaponMeshVisual(SecondaryWeaponMeshComponent);
		return;
	}
	if (!ValidateWeaponMesh(*LoadedSecondaryWeaponMesh, CurrentWeaponState.ItemTag))
	{
		ClearWeaponMeshVisual(SecondaryWeaponMeshComponent);
		return;
	}

	ApplyWeaponMeshVisual(
		LoadedSecondaryWeaponMesh,
		SecondaryWeaponMeshComponent,
		CurrentWeaponState.SecondaryAttachSocketName,
		CurrentWeaponState.SecondaryAttachTransform,
		TEXT("SecondaryEquippedWeaponMeshComponent"));
}

void UMVWeaponComponent::ApplyWeaponMeshVisual(
	UObject* LoadedWeaponMesh,
	TObjectPtr<UMeshComponent>& MeshComponentStorage,
	const FName AttachSocketName,
	const FTransform& RelativeTransform,
	const TCHAR* ComponentNameBase)
{
	if (!LoadedWeaponMesh)
	{
		return;
	}

	UMeshComponent* MeshComponent = EnsureWeaponMeshComponent(
		MeshComponentStorage,
		*LoadedWeaponMesh,
		ComponentNameBase);
	if (!MeshComponent)
	{
		return;
	}

	if (USceneComponent* AttachParent = ResolveWeaponAttachParent())
	{
		if (const USkeletalMeshComponent* CharacterMesh = Cast<USkeletalMeshComponent>(AttachParent);
			CharacterMesh && !AttachSocketName.IsNone()
			&& !CharacterMesh->DoesSocketExist(AttachSocketName))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Weapon attach socket does not exist. ItemTag=%s Socket=%s CharacterMesh=%s."),
				*CurrentWeaponState.ItemTag.ToString(),
				*AttachSocketName.ToString(),
				*GetNameSafe(CharacterMesh));
		}

		MeshComponent->AttachToComponent(
			AttachParent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);
	}

	if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponent))
	{
		SkeletalMeshComponent->SetSkeletalMesh(CastChecked<USkeletalMesh>(LoadedWeaponMesh));
	}
	else if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
	{
		StaticMeshComponent->SetStaticMesh(CastChecked<UStaticMesh>(LoadedWeaponMesh));
	}
	MeshComponent->SetRelativeTransform(RelativeTransform);
	MeshComponent->SetVisibility(true, true);
}

void UMVWeaponComponent::ClearWeaponVisual()
{
	ClearWeaponMeshVisual(WeaponMeshComponent);
	ClearWeaponMeshVisual(SecondaryWeaponMeshComponent);
}

void UMVWeaponComponent::ClearWeaponMeshVisual(TObjectPtr<UMeshComponent>& MeshComponentStorage)
{
	if (!MeshComponentStorage)
	{
		return;
	}

	if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponentStorage))
	{
		SkeletalMeshComponent->SetSkeletalMesh(nullptr);
	}
	else if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponentStorage))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
	}
	MeshComponentStorage->SetVisibility(false, true);
}

UMeshComponent* UMVWeaponComponent::EnsureWeaponMeshComponent(
	TObjectPtr<UMeshComponent>& MeshComponentStorage,
	const UObject& WeaponMesh,
	const TCHAR* ComponentNameBase)
{
	const bool bNeedsSkeletalMeshComponent = WeaponMesh.IsA<USkeletalMesh>();
	const bool bHasCompatibleComponent =
		(bNeedsSkeletalMeshComponent && Cast<USkeletalMeshComponent>(MeshComponentStorage))
		|| (!bNeedsSkeletalMeshComponent && Cast<UStaticMeshComponent>(MeshComponentStorage));
	if (bHasCompatibleComponent)
	{
		return MeshComponentStorage;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	if (MeshComponentStorage)
	{
		MeshComponentStorage->DestroyComponent();
		MeshComponentStorage = nullptr;
	}

	const FName WeaponMeshComponentName = MakeUniqueObjectName(
		Owner,
		bNeedsSkeletalMeshComponent ? USkeletalMeshComponent::StaticClass() : UStaticMeshComponent::StaticClass(),
		ComponentNameBase);
	MeshComponentStorage = bNeedsSkeletalMeshComponent
		? Cast<UMeshComponent>(NewObject<USkeletalMeshComponent>(Owner, WeaponMeshComponentName))
		: Cast<UMeshComponent>(NewObject<UStaticMeshComponent>(Owner, WeaponMeshComponentName));
	if (!MeshComponentStorage)
	{
		return nullptr;
	}

	MeshComponentStorage->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponentStorage->SetVisibility(false, true);
	MeshComponentStorage->RegisterComponent();
	return MeshComponentStorage;
}

USceneComponent* UMVWeaponComponent::ResolveWeaponAttachParent() const
{
	if (const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		if (USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh())
		{
			return CharacterMesh;
		}
	}

	return GetOwner() ? GetOwner()->GetRootComponent() : nullptr;
}

void UMVWeaponComponent::InitSkillTreeStates()
{
	for (auto& Pair : SkillTreeStates)
	{
		// Find the SkillMap corresponding to the WeaponStyle of the SkillTreeState
		TObjectPtr<UMVWeaponSkillMap>* FoundSkillMap = SkillBlueprint.Find(Pair.Key);
		if (!FoundSkillMap || !*FoundSkillMap)
		{
			continue;
		}

		UMVWeaponSkillMap* SkillMap = FoundSkillMap->Get();
		InitSkillSlotState(Pair.Value, EMVSkillSlot::PassiveSkill, SkillMap->PassiveSkill);
		InitSkillSlotState(Pair.Value, EMVSkillSlot::QSkill, SkillMap->QSkill);
		InitSkillSlotState(Pair.Value, EMVSkillSlot::WSkill, SkillMap->WSkill);
		InitSkillSlotState(Pair.Value, EMVSkillSlot::ESkill, SkillMap->ESkill);
		InitSkillSlotState(Pair.Value, EMVSkillSlot::RSkill, SkillMap->RSkill);
	}
}

void UMVWeaponComponent::InitSkillSlotState(
	FMVSkillTreeState& SkillTreeState,
	const EMVSkillSlot SkillSlot,
	const TSet<FMVWeaponSkillMapData>& Skills) const
{
	bool bFoundAutoUnlockedSkill = false;

	for (const FMVWeaponSkillMapData& Skill : Skills)
	{
		const FDataTableRowHandle& RowHandle = Skill.SkillRowHandle.RowHandle;
		if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Invalid weapon skill row ignored. Owner=%s Style=%d Slot=%d DataTable=%s Row=%s."),
				*GetNameSafe(GetOwner()),
				static_cast<int32>(SkillTreeState.EquippedStyle),
				static_cast<int32>(SkillSlot),
				*GetNameSafe(RowHandle.DataTable),
				*RowHandle.RowName.ToString());
			continue;
		}

		if (Skill.RequiredPoints > 0)
		{
			continue;
		}

		bFoundAutoUnlockedSkill = true;
		SkillTreeState.UnlockedSkills.Add(Skill.SkillRowHandle);
		if (!SkillTreeState.EquippedSkill.Contains(SkillSlot))
		{
			SkillTreeState.EquippedSkill.Add(SkillSlot, RowHandle);
		}
	}

	if (!bFoundAutoUnlockedSkill && !SkillTreeState.EquippedSkill.Contains(SkillSlot))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Weapon skill slot has no valid auto-unlocked default. Owner=%s Style=%d Slot=%d."),
			*GetNameSafe(GetOwner()),
			static_cast<int32>(SkillTreeState.EquippedStyle),
			static_cast<int32>(SkillSlot));
	}
}
