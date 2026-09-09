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

	if (!CurrentWeaponState.bValid)
	{
		if (!TryEquipDefaultWeapon())
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

	const int32 LoadoutCount = WeaponLoadoutRows.Num();
	if (LoadoutCount == 0)
	{
		return false;
	}

	int32 CurrentIndex = ActiveLoadoutIndex;
	if (!WeaponLoadoutRows.IsValidIndex(CurrentIndex))
	{
		CurrentIndex = FindLoadoutIndexForItemTag(CurrentWeaponState.ItemTag);
	}

	const bool bHasCurrentIndex = WeaponLoadoutRows.IsValidIndex(CurrentIndex);
	const int32 CandidateCount = bHasCurrentIndex ? LoadoutCount - 1 : LoadoutCount;

	for (int32 Step = 0; Step < CandidateCount; ++Step)
	{
		const int32 CandidateIndex = bHasCurrentIndex ? (CurrentIndex + Step + 1) % LoadoutCount : Step;

		const FMVWeaponTableRow* WeaponRow = ResolveWeaponLoadoutRow(WeaponLoadoutRows[CandidateIndex]);
		if (!WeaponRow)
		{
			continue;
		}

		if (TryEquipWeaponRow(*WeaponRow, CandidateIndex))
		{
			return true;
		}
	}

	return false;
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

bool UMVWeaponComponent::EquipWeaponFromRow(const FMVWeaponTableRow& WeaponRow)
{
	// FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(WeaponRow);
	// if (!NewState.bValid)
	// {
	// 	return false;
	// }
	// if (!CanEquipWeaponState(NewState))
	// {
	// 	return false;
	// }
	//
	// ApplyEquippedWeaponState(NewState);
	// return true;
	
	return TryEquipWeaponRow(WeaponRow, FindLoadoutIndexForItemTag(WeaponRow.ItemTag));
}

void UMVWeaponComponent::EquipBareHand()
{
	FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(BareHandWeapon);
	NewState.bValid = true;
	
	ActiveLoadoutIndex = FindLoadoutIndexForItemTag(NewState.ItemTag);
	
	ApplyEquippedWeaponState(NewState);
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

bool UMVWeaponComponent::TryEquipWeaponRow(const FMVWeaponTableRow& WeaponRow, int32 LoadoutIndex)
{
	FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(WeaponRow);
	if (!NewState.bValid || !CanEquipWeaponState(NewState))
	{
		return false;
	}

	ActiveLoadoutIndex = WeaponLoadoutRows.IsValidIndex(LoadoutIndex) ? LoadoutIndex : INDEX_NONE;

	ApplyEquippedWeaponState(NewState);
	return true;
}

const FMVWeaponTableRow* UMVWeaponComponent::ResolveWeaponLoadoutRow(const FDataTableRowHandle& RowHandle) const
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Weapon loadout row handle is invalid. DataTable=%s Row=%s."),
			*GetNameSafe(RowHandle.DataTable),
			*RowHandle.RowName.ToString());
		return nullptr;
	}

	return RowHandle.DataTable->FindRow<FMVWeaponTableRow>(
		RowHandle.RowName,
		TEXT("UMVWeaponComponent::ResolveWeaponLoadoutRow"),
		true);
}

int32 UMVWeaponComponent::FindLoadoutIndexForHandle(const FDataTableRowHandle& RowHandle) const
{
	for (int32 Index = 0; Index < WeaponLoadoutRows.Num(); ++Index)
	{
		const FDataTableRowHandle& Candidate = WeaponLoadoutRows[Index];
		if (Candidate.DataTable == RowHandle.DataTable && Candidate.RowName == RowHandle.RowName)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 UMVWeaponComponent::FindLoadoutIndexForItemTag(const FGameplayTag& ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < WeaponLoadoutRows.Num(); ++Index)
	{
		const FDataTableRowHandle& Candidate = WeaponLoadoutRows[Index];
		if (!Candidate.DataTable || Candidate.RowName.IsNone())
		{
			continue;
		}

		const FMVWeaponTableRow* CandidateRow =
			Candidate.DataTable->FindRow<FMVWeaponTableRow>(
				Candidate.RowName,
				TEXT("UMVWeaponComponent::FindLoadoutIndexForItemTag"),
				false);

		if (CandidateRow && CandidateRow->ItemTag.MatchesTagExact(ItemTag))
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void UMVWeaponComponent::ApplyEquippedWeaponState(const FMVEquippedWeaponState& NewState)
{
	CurrentWeaponState = NewState;
	CurrentWeaponState.AttackPower = FMath::Max(0.0f, CurrentWeaponState.AttackPower);
	CurrentWeaponState.TraceRadius = FMath::Max(0.0f, CurrentWeaponState.TraceRadius);
	CurrentWeaponState.bValid = true;

	SyncOwnerEquippedStyle();
	RefreshOwnerCombatComponent();
	ApplyWeaponVisual();
	OnEquippedWeaponChanged.Broadcast(CurrentWeaponState);
}

FMVEquippedWeaponState UMVWeaponComponent::MakeStateFromWeaponRow(const FMVWeaponTableRow& WeaponRow)
{
	FMVEquippedWeaponState State;
	State.ItemTag = WeaponRow.ItemTag;
	State.EquippedStyle = WeaponRow.EquippedStyle;
	State.AttackPower = FMath::Max(0.0f, WeaponRow.AttackPower);
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

bool UMVWeaponComponent::TryEquipDefaultWeapon()
{
	if (!DefaultWeaponRow.DataTable || DefaultWeaponRow.RowName.IsNone())
	{
		return false;
	}

	const FMVWeaponTableRow* WeaponRow = DefaultWeaponRow.GetRow<FMVWeaponTableRow>(TEXT("MVWeaponComponent DefaultWeaponRow"));
	if (!WeaponRow)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Default weapon row was not resolved. DataTable=%s Row=%s."),
			*GetNameSafe(DefaultWeaponRow.DataTable),
			*DefaultWeaponRow.RowName.ToString());
		return false;
	}

	//return EquipWeaponFromRow(*WeaponRow);
	return TryEquipWeaponRow(*WeaponRow, FindLoadoutIndexForHandle(DefaultWeaponRow));
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
