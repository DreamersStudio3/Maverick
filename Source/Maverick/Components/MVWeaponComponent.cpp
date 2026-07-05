#include "Components/MVWeaponComponent.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Tags/MVGameplayTags.h"

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
		EquipBareHand();
	}
}

bool UMVWeaponComponent::EquipWeaponFromRow(const FMVWeaponTableRow& WeaponRow)
{
	FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(WeaponRow);
	if (!NewState.bValid)
	{
		return false;
	}

	ApplyEquippedWeaponState(NewState);
	return true;
}

void UMVWeaponComponent::EquipBareHand()
{
	FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(BareHandWeapon);
	NewState.bValid = true;
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

void UMVWeaponComponent::ApplyEquippedWeaponState(const FMVEquippedWeaponState& NewState)
{
	CurrentWeaponState = NewState;
	CurrentWeaponState.AttackPower = FMath::Max(0.0f, CurrentWeaponState.AttackPower);
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

	USkeletalMesh* LoadedWeaponMesh = CurrentWeaponState.WeaponMesh.LoadSynchronous();
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

	USkeletalMeshComponent* MeshComponent = EnsureWeaponMeshComponent();
	if (!MeshComponent)
	{
		return;
	}

	if (USceneComponent* AttachParent = ResolveWeaponAttachParent())
	{
		if (const USkeletalMeshComponent* CharacterMesh = Cast<USkeletalMeshComponent>(AttachParent);
			CharacterMesh && !CurrentWeaponState.AttachSocketName.IsNone()
			&& !CharacterMesh->DoesSocketExist(CurrentWeaponState.AttachSocketName))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Weapon attach socket does not exist. ItemTag=%s Socket=%s CharacterMesh=%s."),
				*CurrentWeaponState.ItemTag.ToString(),
				*CurrentWeaponState.AttachSocketName.ToString(),
				*GetNameSafe(CharacterMesh));
		}

		MeshComponent->AttachToComponent(
			AttachParent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			CurrentWeaponState.AttachSocketName);
	}

	MeshComponent->SetSkeletalMesh(LoadedWeaponMesh);
	MeshComponent->SetRelativeTransform(CurrentWeaponState.AttachTransform);
	MeshComponent->SetVisibility(true, true);
}

void UMVWeaponComponent::ClearWeaponVisual()
{
	if (!WeaponMeshComponent)
	{
		return;
	}

	WeaponMeshComponent->SetSkeletalMesh(nullptr);
	WeaponMeshComponent->SetVisibility(false, true);
}

USkeletalMeshComponent* UMVWeaponComponent::EnsureWeaponMeshComponent()
{
	if (WeaponMeshComponent)
	{
		return WeaponMeshComponent;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	const FName WeaponMeshComponentName = MakeUniqueObjectName(
		Owner,
		USkeletalMeshComponent::StaticClass(),
		TEXT("EquippedWeaponMeshComponent"));
	WeaponMeshComponent = NewObject<USkeletalMeshComponent>(Owner, WeaponMeshComponentName);
	if (!WeaponMeshComponent)
	{
		return nullptr;
	}

	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComponent->SetVisibility(false, true);
	WeaponMeshComponent->RegisterComponent();
	return WeaponMeshComponent;
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
