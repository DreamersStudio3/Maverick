#include "Components/MVWeaponComponent.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr const TCHAR* MVWeaponEquipTracePrefix = TEXT("WeaponEquipTrace");
constexpr const TCHAR* MVWeaponBuiltInDefaultWeaponTablePath =
	TEXT("/Game/Table/Weapons/Player/DT_Weapons_Player.DT_Weapons_Player");
const FName MVWeaponBuiltInDefaultWeaponRowName(TEXT("OneHand_TestSword"));

bool MVWeaponIsWeaponRowHandleSet(const FDataTableRowHandle& RowHandle)
{
	return RowHandle.DataTable && !RowHandle.RowName.IsNone();
}

FDataTableRowHandle MVWeaponMakeBuiltInDefaultWeaponRowHandle()
{
	FDataTableRowHandle RowHandle;
	RowHandle.DataTable = LoadObject<UDataTable>(nullptr, MVWeaponBuiltInDefaultWeaponTablePath);
	RowHandle.RowName = MVWeaponBuiltInDefaultWeaponRowName;
	return RowHandle;
}
}

UMVWeaponComponent::UMVWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	DefaultWeaponRow.RowName = MVWeaponBuiltInDefaultWeaponRowName;

	static ConstructorHelpers::FObjectFinder<UDataTable> DefaultWeaponTableFinder(MVWeaponBuiltInDefaultWeaponTablePath);
	if (DefaultWeaponTableFinder.Succeeded())
	{
		DefaultWeaponRow.DataTable = DefaultWeaponTableFinder.Object;
		DefaultWeaponRow.RowName = MVWeaponBuiltInDefaultWeaponRowName;
	}
}

void UMVWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!CurrentWeaponState.bValid)
	{
		EquipDefaultWeapon();
	}
}

bool UMVWeaponComponent::EquipWeaponFromRow(const FMVWeaponTableRow& WeaponRow)
{
	FMVEquippedWeaponState NewState = MakeStateFromWeaponRow(WeaponRow);
	if (!NewState.bValid)
	{
		return false;
	}
	if (!CanEquipWeaponState(NewState))
	{
		return false;
	}

	ApplyEquippedWeaponState(NewState);
	return true;
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

bool UMVWeaponComponent::GetMeleeWeaponData_Implementation(
	const FName StartSocketName,
	const FName EndSocketName,
	TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const
{
	OutMeleeWeaponData.Reset();
	if (StartSocketName.IsNone() || EndSocketName.IsNone())
	{
		return false;
	}

	FMVMeleeWeaponData PrimaryData;
	if (TryBuildMeleeWeaponData(
		WeaponMeshComponent.Get(),
		StartSocketName,
		EndSocketName,
		false,
		PrimaryData))
	{
		OutMeleeWeaponData.Add(PrimaryData);
	}

	FMVMeleeWeaponData SecondaryData;
	if (TryBuildMeleeWeaponData(
		SecondaryWeaponMeshComponent.Get(),
		StartSocketName,
		EndSocketName,
		true,
		SecondaryData))
	{
		OutMeleeWeaponData.Add(SecondaryData);
	}

	return !OutMeleeWeaponData.IsEmpty();
}

bool UMVWeaponComponent::GetMeleeDualWeaponData_Implementation(
	const FName StartSocketName,
	const FName EndSocketName,
	TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const
{
	return GetMeleeWeaponData_Implementation(StartSocketName, EndSocketName, OutMeleeWeaponData);
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

bool UMVWeaponComponent::EquipDefaultWeapon()
{
	FDataTableRowHandle WeaponRowHandle = DefaultWeaponRow;
	bool bUsingBuiltInFallback = false;
	if (!MVWeaponIsWeaponRowHandleSet(WeaponRowHandle))
	{
		WeaponRowHandle = MVWeaponMakeBuiltInDefaultWeaponRowHandle();
		bUsingBuiltInFallback = true;
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s: DefaultWeaponRow is not configured. Owner=%s Component=%s. Falling back to TestSword."),
			MVWeaponEquipTracePrefix,
			*GetNameSafe(GetOwner()),
			*GetNameSafe(this));
	}

	const FMVWeaponTableRow* WeaponRow = WeaponRowHandle.GetRow<FMVWeaponTableRow>(TEXT("MVWeaponComponent DefaultWeaponRow"));
	if (!WeaponRow && !bUsingBuiltInFallback)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s: Default weapon row was not resolved. Owner=%s DataTable=%s Row=%s. Falling back to TestSword."),
			MVWeaponEquipTracePrefix,
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponRowHandle.DataTable),
			*WeaponRowHandle.RowName.ToString());
		WeaponRowHandle = MVWeaponMakeBuiltInDefaultWeaponRowHandle();
		bUsingBuiltInFallback = true;
		WeaponRow = WeaponRowHandle.GetRow<FMVWeaponTableRow>(TEXT("MVWeaponComponent BuiltInDefaultWeaponRow"));
	}

	if (!WeaponRow)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s: TestSword fallback row was not resolved. Owner=%s DataTable=%s Row=%s."),
			MVWeaponEquipTracePrefix,
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponRowHandle.DataTable),
			*WeaponRowHandle.RowName.ToString());
		return false;
	}

	return EquipWeaponFromRow(*WeaponRow);
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

	return true;
}

bool UMVWeaponComponent::TryBuildMeleeWeaponData(
	UMeshComponent* MeshComponent,
	const FName StartSocketName,
	const FName EndSocketName,
	const bool bSecondaryWeapon,
	FMVMeleeWeaponData& OutData) const
{
	OutData = FMVMeleeWeaponData();
	if (!MeshComponent || !MeshComponent->IsVisible())
	{
		return false;
	}

	if (!MeshComponent->DoesSocketExist(StartSocketName) || !MeshComponent->DoesSocketExist(EndSocketName))
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("Melee weapon sockets are not available on mesh component. ItemTag=%s Component=%s Start=%s End=%s."),
			*CurrentWeaponState.ItemTag.ToString(),
			*GetNameSafe(MeshComponent),
			*StartSocketName.ToString(),
			*EndSocketName.ToString());
		return false;
	}

	OutData.WeaponMesh = MeshComponent;
	OutData.StartLocation = MeshComponent->GetSocketLocation(StartSocketName);
	OutData.EndLocation = MeshComponent->GetSocketLocation(EndSocketName);
	OutData.bSecondaryWeapon = bSecondaryWeapon;
	return true;
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
		if (CurrentWeaponState.ItemTag.IsValid())
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
