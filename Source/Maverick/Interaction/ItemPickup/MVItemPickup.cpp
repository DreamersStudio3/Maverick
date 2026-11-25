#include "Interaction/ItemPickup/MVItemPickup.h"

#include "Character/PC/InteractionDetector/MVPlayerInteractionDetector.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVInventoryComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "System/MVFieldEnemySpawnPoint.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tables/MVTableManager.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#endif

namespace
{
void MVConfigurePickupMesh(UStaticMeshComponent& Component, UStaticMesh* Mesh)
{
	Component.SetStaticMesh(Mesh);
	Component.SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component.SetGenerateOverlapEvents(false);
	Component.SetCanEverAffectNavigation(false);
	Component.SetCastShadow(false);
}
}

AMVItemPickup::AMVItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	PrimaryActorTick.TickInterval = 1.0f / 30.0f;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("GroundOrigin")));
	PickupBounds = CreateDefaultSubobject<USphereComponent>(TEXT("PickupBounds"));
	PickupBounds->SetupAttachment(GetRootComponent());
	PickupBounds->SetSphereRadius(12.0f);
	PickupBounds->SetRelativeLocation(FVector(0.0f, 0.0f, 12.0f));
	PickupBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupBounds->SetCollisionObjectType(ECC_WorldDynamic);
	PickupBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupBounds->SetGenerateOverlapEvents(false);
	PickupBounds->SetCanEverAffectNavigation(false);
	static const ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static const ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Core"));
	CoreMesh->SetupAttachment(GetRootComponent());
	CoreMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 8.0f));
	CoreMesh->SetRelativeScale3D(FVector(0.16f));
	MVConfigurePickupMesh(*CoreMesh, Sphere.Object);

	const FName BeamNames[] = { TEXT("BeamFront"), TEXT("BeamDiagonalA"), TEXT("BeamDiagonalB") };
	for (const FName Name : BeamNames)
	{
		auto* Beam = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Beam->SetupAttachment(GetRootComponent());
		const float Yaw = 60.0f * BeamMeshes.Num();
		const FVector Horizontal = FVector::ForwardVector.RotateAngleAxis(Yaw, FVector::UpVector);
		Beam->SetRelativeRotation(FRotationMatrix::MakeFromXY(Horizontal, FVector::UpVector).Rotator());
		MVConfigurePickupMesh(*Beam, Plane.Object);
		BeamMeshes.Add(Beam);
	}

	HaloMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Halo"));
	HaloMesh->SetupAttachment(GetRootComponent());
	HaloMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 1.5f));
	MVConfigurePickupMesh(*HaloMesh, Plane.Object);

	ParticleMeshes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RisingParticles"));
	ParticleMeshes->SetupAttachment(GetRootComponent());
	MVConfigurePickupMesh(*ParticleMeshes, Sphere.Object);

	RareVisual.Color = FLinearColor(0.47f, 0.14f, 1.0f);
	RareVisual.BeamHeight = 65.0f;
	RareVisual.BeamWidth = 20.0f;
	RareVisual.HaloDiameter = 46.0f;
	RareVisual.Brightness = 1.5f;
	RareVisual.ParticleCount = 4;
	EpicVisual.Color = FLinearColor(1.0f, 0.66f, 0.09f);
	EpicVisual.BeamHeight = 110.0f;
	EpicVisual.BeamWidth = 30.0f;
	EpicVisual.HaloDiameter = 75.0f;
	EpicVisual.Brightness = 2.2f;
	EpicVisual.ParticleCount = 9;
}

void AMVItemPickup::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	if (UMVTableManager* Tables = UMVTableManager::Get(this); Tables && !TableReloadHandle.IsValid())
	{
		TableReloadHandle = Tables->OnInventoryItemDefinitionsReloaded.AddUObject(this, &ThisClass::RefreshVisuals);
	}
	RefreshVisuals();
}

void AMVItemPickup::PostUnregisterAllComponents()
{
	if (UMVTableManager* Tables = UMVTableManager::Get(this))
	{
		Tables->OnInventoryItemDefinitionsReloaded.Remove(TableReloadHandle);
	}
	TableReloadHandle.Reset();
	Super::PostUnregisterAllComponents();
}

void AMVItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisuals();
}

void AMVItemPickup::BeginPlay()
{
	Super::BeginPlay();
	if (bSavePickup)
	{
		const FText IdentityError = GetPickupIdentityError();
		bValidPlacedPickup = IdentityError.IsEmpty();
		if (!bValidPlacedPickup)
		{
			UE_LOG(LogTemp, Warning, TEXT("Item pickup %s at %s: %s"), *GetPathName(),
				*GetActorLocation().ToCompactString(), *IdentityError.ToString());
		}
		else if (GetNetMode() == NM_Standalone)
		{
			if (UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this))
			{
				PickupSaveState = State;
				State->OnWorldStateChanged.AddUniqueDynamic(this, &ThisClass::RestorePickupState);
				bCollected = State->IsOneTimeSpawnConsumed(FieldId, PickupId);
			}
		}
	}
	RefreshVisuals();
}

AMVItemPickup* AMVItemPickup::SpawnEnemyDrop(UWorld& World, const TSubclassOf<AMVItemPickup> PickupClass, const FVector& GroundLocation,
	const FGameplayTag ItemTag, const int32 Quantity)
{
	const UMVTableManager* Tables = UMVTableManager::Get(&World);
	const FMVInventoryItemDefinition* Item = Tables ? Tables->FindInventoryItemDefinition(ItemTag) : nullptr;
	if (!PickupClass || !Item || Quantity <= 0 || Quantity > Item->StackLimit)
	{
		return nullptr;
	}

	const FTransform Transform(GroundLocation);
	AMVItemPickup* Pickup = World.SpawnActorDeferred<AMVItemPickup>(
		PickupClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Pickup)
	{
		return nullptr;
	}

	Pickup->bSavePickup = false;
	Pickup->ItemTag = ItemTag;
	Pickup->Quantity = Quantity;
	Pickup->FinishSpawning(Transform);
	if (!IsValid(Pickup))
	{
		return nullptr;
	}
	if (!Pickup->bAvailable)
	{
		Pickup->Destroy();
		return nullptr;
	}
	return Pickup;
}

void AMVItemPickup::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UMVWorldStateSubsystem* State = PickupSaveState.Get())
	{
		State->OnWorldStateChanged.RemoveDynamic(this, &ThisClass::RestorePickupState);
	}
	PickupSaveState.Reset();
	Super::EndPlay(Reason);
}

void AMVItemPickup::RestorePickupState()
{
	const UMVWorldStateSubsystem* State = PickupSaveState.Get();
	if (!State)
	{
		return;
	}
	const bool bIsConsumed = State->IsOneTimeSpawnConsumed(FieldId, PickupId);
	if (bCollected == bIsConsumed)
	{
		return;
	}

	bCollected = bIsConsumed;
	FeedbackUntil = 0.0;
	RefreshVisuals();
}

FText AMVItemPickup::GetPickupIdentityError() const
{
	if (!bSavePickup)
	{
		return FText::GetEmpty();
	}
	if (FieldId.IsNone() || PickupId.IsNone())
	{
		return NSLOCTEXT("MVItemPickup", "MissingPickupIdentity", "고정 드랍의 FieldId와 PickupId를 지정해야 합니다.");
	}

	for (TActorIterator<AMVItemPickup> It(GetWorld()); It; ++It)
	{
		if (*It != this && It->bSavePickup && It->FieldId == FieldId && It->PickupId == PickupId)
		{
			return FText::Format(NSLOCTEXT("MVItemPickup", "DuplicatePickupIdentity", "고정 드랍 {0}과 배치 ID가 같습니다."),
				FText::FromString(It->GetPathName()));
		}
	}
	for (TActorIterator<AMVFieldEnemySpawnPoint> It(GetWorld()); It; ++It)
	{
		if (It->GetFieldId() == FieldId && It->GetSpawnId() == PickupId)
		{
			return FText::Format(NSLOCTEXT("MVItemPickup", "PickupConflictsWithSpawn", "적 생성 지점 {0}과 배치 ID가 같습니다."),
				FText::FromString(It->GetPathName()));
		}
	}
	return FText::GetEmpty();
}

#if WITH_EDITOR
void AMVItemPickup::CheckForErrors()
{
	Super::CheckForErrors();
	FText Error = GetPickupIdentityError();
	if (Error.IsEmpty())
	{
		const UMVTableManager* Tables = UMVTableManager::Get(this);
		const FMVInventoryItemDefinition* Item = Tables ? Tables->FindInventoryItemDefinition(ItemTag) : nullptr;
		if (!Item || Quantity <= 0 || Quantity > Item->StackLimit)
		{
			Error = NSLOCTEXT("MVItemPickup", "InvalidPickupItem", "등록된 아이템 태그와 보유 한도 안의 양수 수량을 지정해야 합니다.");
		}
	}
	if (!Error.IsEmpty())
	{
		FMessageLog("MapCheck").Error()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString(GetActorLocation().ToCompactString())))
			->AddToken(FTextToken::Create(Error));
	}
}
#endif

bool AMVItemPickup::CanInteract_Implementation(AActor* Interactor) const
{
	const auto* Player = Cast<AMVPlayerCharacter>(Interactor);
	const UWorld* World = GetWorld();
	if (!bAvailable || bCollected || bGrantInProgress || !World || !Player || Player->GetWorld() != World
		|| (bSavePickup && !PickupSaveState.IsValid())
		|| !Player->InventoryComponent || !Player->InteractionDetector || !Player->InteractionDetector->CanDetectInteractables())
	{
		return false;
	}

	const UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	const APlayerController* Controller = Cast<APlayerController>(Player->GetController());
	if (!Movement || !Movement->IsMovingOnGround() || !Controller || Controller->IsMoveInputIgnored()
		|| Player->GetCharacterIsLying()
		|| (Player->InputManagerComponent && Player->InputManagerComponent->IsGameplayInputBlocked())
		|| (Player->ActionComponent && Player->ActionComponent->IsActionRunning())
		|| (Player->StatComponent && Player->StatComponent->IsGroggy()))
	{
		return false;
	}

	const FVector Feet = Movement->GetActorFeetLocation();
	if (FVector::DistSquared(Feet, GetActorLocation()) > FMath::Square(PickupDistance))
	{
		return false;
	}

	FVector Eyes;
	FRotator View;
	Player->GetActorEyesViewPoint(Eyes, View);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MVItemPickupOcclusion), false, Player);
	Params.AddIgnoredActor(this);
	return !World->LineTraceTestByChannel(Eyes, PickupBounds->GetComponentLocation(), ECC_Visibility, Params)
		&& GetPickupIdentityError().IsEmpty();
}

void AMVItemPickup::Interact_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return;
	}

	TGuardValue<bool> GrantGuard(bGrantInProgress, true);
	auto& Player = *CastChecked<AMVPlayerCharacter>(Interactor);
	const EMVInventoryGrantResult Result = bSavePickup
		? Player.InventoryComponent->TryGrantPlacedPickup(ItemTag, Quantity, FieldId, PickupId)
		: Player.InventoryComponent->TryGrantItem(ItemTag, Quantity);
	if (Result == EMVInventoryGrantResult::Added)
	{
		if (!bSavePickup)
		{
			bCollected = true;
			SetPickupVisible(false);
		}
		return;
	}

	switch (Result)
	{
	case EMVInventoryGrantResult::CarryLimit:
		GrantFeedback = NSLOCTEXT("MVItemPickup", "CarryLimit", "더 소지할 수 없습니다");
		break;
	case EMVInventoryGrantResult::WeaponAlreadyOwned:
		GrantFeedback = NSLOCTEXT("MVItemPickup", "WeaponAlreadyOwned", "이미 소유한 무기입니다");
		break;
	default:
		return;
	}
	FeedbackUntil = GetWorld()->GetTimeSeconds() + 1.2;
}

FText AMVItemPickup::GetInteractionPromptText_Implementation(AActor* Interactor) const
{
	if (const UWorld* World = GetWorld(); World && World->GetTimeSeconds() < FeedbackUntil)
	{
		return GrantFeedback;
	}
	return NSLOCTEXT("MVItemPickup", "Prompt", "아이템을 얻는다");
}

int32 AMVItemPickup::GetInteractionPriority_Implementation(AActor* Interactor) const
{
	return 0;
}

void AMVItemPickup::SetPickupVisible(const bool bVisible)
{
	GetRootComponent()->SetVisibility(bVisible, true);
	PickupBounds->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	if (!bVisible)
	{
		ParticleMeshes->ClearInstances();
		ParticleTransforms.Reset();
		SetActorTickEnabled(false);
	}
}

void AMVItemPickup::RefreshVisuals()
{
	const UMVTableManager* Tables = UMVTableManager::Get(this);
	const FMVInventoryItemDefinition* Item = Tables ? Tables->FindInventoryItemDefinition(ItemTag) : nullptr;
	bAvailable = Item && Quantity > 0 && Quantity <= Item->StackLimit && CoreMaterial && GlowMaterial;
	ResolvedRarity = Item ? Item->Rarity : EMVItemRarity::UseItemType;
	const bool bVisible = bAvailable && bValidPlacedPickup && !bCollected;
	SetPickupVisible(bVisible);
	if (!bVisible)
	{
		return;
	}

	const FMVItemPickupVisual* Visual = &CommonVisual;
	switch (ResolvedRarity)
	{
	case EMVItemRarity::Rare: Visual = &RareVisual; break;
	case EMVItemRarity::Epic: Visual = &EpicVisual; break;
	default: break;
	}

	const auto MakeMaterial = [this, Visual](UMaterialInterface* Parent)
	{
		auto* Material = UMaterialInstanceDynamic::Create(Parent, this);
		Material->SetFlags(RF_Transient);
		Material->SetVectorParameterValue(TEXT("Tint"), Visual->Color);
		Material->SetScalarParameterValue(TEXT("Brightness"), Visual->Brightness);
		return Material;
	};
	UMaterialInstanceDynamic* Core = MakeMaterial(CoreMaterial);
	CoreMesh->SetMaterial(0, Core);
	ParticleMeshes->SetMaterial(0, Core);

	CurrentBeamHeight = Visual->BeamHeight;
	CurrentBeamWidth = Visual->BeamWidth;
	UMaterialInstanceDynamic* Beam = MakeMaterial(GlowMaterial);
	for (UStaticMeshComponent* Mesh : BeamMeshes)
	{
		Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, CurrentBeamHeight * 0.5f));
		Mesh->SetRelativeScale3D(FVector(CurrentBeamWidth / 100.0f, CurrentBeamHeight / 100.0f, 1.0f));
		Mesh->SetMaterial(0, Beam);
	}

	UMaterialInstanceDynamic* Halo = MakeMaterial(GlowMaterial);
	Halo->SetScalarParameterValue(TEXT("IsHalo"), 1.0f);
	HaloMesh->SetRelativeScale3D(FVector(Visual->HaloDiameter / 100.0f, Visual->HaloDiameter / 100.0f, 1.0f));
	HaloMesh->SetMaterial(0, Halo);

	const int32 ParticleCount = FMath::Clamp(Visual->ParticleCount, 0, 16);
	ParticleTransforms.SetNum(ParticleCount);
	ParticleMeshes->ClearInstances();
	for (int32 Index = 0; Index < ParticleCount; ++Index)
	{
		ParticleMeshes->AddInstance(FTransform::Identity);
	}
	UpdateParticles();
	SetActorTickEnabled(HasActorBegunPlay() && ParticleCount > 0);
}

void AMVItemPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ParticleElapsed = FMath::Fmod(ParticleElapsed + DeltaSeconds, 2.4f);
	UpdateParticles();
}

void AMVItemPickup::UpdateParticles()
{
	const int32 Count = ParticleTransforms.Num();
	if (Count == 0)
	{
		return;
	}
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Phase = FMath::Frac((Index + 0.45f) / Count + ParticleElapsed / 2.4f);
		const float Angle = Index * 2.399963f + Phase * 2.0f;
		const float Radius = CurrentBeamWidth * (0.20f + Phase * 0.30f);
		const float Size = FMath::Sin(PI * Phase) * 0.020f;
		ParticleTransforms[Index] = FTransform(FQuat::Identity,
			FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 6.0f + Phase * CurrentBeamHeight * 0.90f),
			FVector(Size));
	}
	ParticleMeshes->BatchUpdateInstancesTransforms(0, ParticleTransforms, false, true);
}
