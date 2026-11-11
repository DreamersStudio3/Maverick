#include "Interaction/RestPoint/MVRestPoint.h"

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Interaction/RestPoint/MVRestPointSubsystem.h"
#include "UI/Window/MVRestPointWindow.h"
#include "UObject/ConstructorHelpers.h"

AMVRestPoint::AMVRestPoint()
{
	USceneComponent* Origin = CreateDefaultSubobject<USceneComponent>(TEXT("Origin"));
	SetRootComponent(Origin);

	RestPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RestPointMesh"));
	RestPointMesh->SetupAttachment(Origin);
	RestPointMesh->SetRelativeLocation(FVector(0.0, 0.0, 38.0));
	RestPointMesh->SetRelativeScale3D(FVector(0.55, 0.55, 0.76));
	RestPointMesh->SetCollisionProfileName(TEXT("BlockAll"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RestPointDefaultMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (RestPointDefaultMesh.Succeeded())
	{
		RestPointMesh->SetStaticMesh(RestPointDefaultMesh.Object);
	}

	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	InteractionBounds->SetupAttachment(Origin);
	InteractionBounds->SetRelativeLocation(FVector(0.0, 0.0, 40.0));
	InteractionBounds->InitBoxExtent(FVector(42.0, 42.0, 60.0));
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionBounds->SetGenerateOverlapEvents(false);

	SafetyBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SafetyBounds"));
	SafetyBounds->SetupAttachment(Origin);
	SafetyBounds->InitBoxExtent(FVector(450.0, 450.0, 200.0));
	SafetyBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RestAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("RestAnchor"));
	RestAnchor->SetupAttachment(Origin);
	RestAnchor->SetRelativeLocation(FVector(-95.0, 0.0, 0.0));
	RespawnAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnAnchor"));
	RespawnAnchor->SetupAttachment(Origin);
	RespawnAnchor->SetRelativeLocation(FVector(-180.0, 0.0, 100.0));

	DisplayName = NSLOCTEXT("MVRestPoint", "DefaultName", "휴식 거점");
	MenuClass = UMVRestPointWindow::StaticClass();
}

bool AMVRestPoint::CanInteract_Implementation(AActor* Interactor) const
{
	const AMVPlayerCharacter* Player = Cast<AMVPlayerCharacter>(Interactor);
	const UWorld* World = GetWorld();
	const UMVRestPointSubsystem* Session = World ? World->GetSubsystem<UMVRestPointSubsystem>() : nullptr;
	return Player && Session && !RestPointId.IsNone() && Player->IsLocallyControlled()
		&& Player->StatComponent && !Player->StatComponent->IsDead()
		&& IsInRange(*Player) && !Session->IsUsingRestPoint(*Player);
}

FTransform AMVRestPoint::GetRespawnTransform() const
{
	return RespawnAnchor->GetComponentTransform();
}

void AMVRestPoint::Interact_Implementation(AActor* Interactor)
{
	if (AMVPlayerCharacter* Player = Cast<AMVPlayerCharacter>(Interactor))
	{
		if (UWorld* World = GetWorld())
		{
			if (UMVRestPointSubsystem* Session = World->GetSubsystem<UMVRestPointSubsystem>())
			{
				if (CanInteract_Implementation(Player))
				{
					Session->Open(*this, *Player);
				}
			}
		}
	}
}

FText AMVRestPoint::GetInteractionPromptText_Implementation(AActor* Interactor) const
{
	const AMVPlayerCharacter* Player = Cast<AMVPlayerCharacter>(Interactor);
	const UWorld* World = GetWorld();
	const UMVRestPointSubsystem* Session = World ? World->GetSubsystem<UMVRestPointSubsystem>() : nullptr;
	if (Player && Session)
	{
		const FText Reason = Session->GetEntryBlockReason(*this, *Player);
		if (!Reason.IsEmpty())
		{
			return Reason;
		}
	}
	return NSLOCTEXT("MVRestPoint", "Use", "거점 이용");
}

int32 AMVRestPoint::GetInteractionPriority_Implementation(AActor* Interactor) const
{
	return 0;
}

bool AMVRestPoint::IsInRange(const AActor& Player) const
{
	return Player.GetWorld() == GetWorld()
		&& FVector::DistSquared(Player.GetActorLocation(), InteractionBounds->GetComponentLocation())
			<= FMath::Square(InteractionDistance);
}

bool AMVRestPoint::HasLivingEnemyNearby() const
{
	const UWorld* World = GetWorld();
	if (!World || SafetyBounds->GetScaledBoxExtent().IsNearlyZero())
	{
		return false;
	}

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps, SafetyBounds->GetComponentLocation(), SafetyBounds->GetComponentQuat(),
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeBox(SafetyBounds->GetScaledBoxExtent()));
	for (const FOverlapResult& Overlap : Overlaps)
	{
		const AMVEnemy* Enemy = Cast<AMVEnemy>(Overlap.GetActor());
		if (IsValid(Enemy) && (!Enemy->StatComponent || !Enemy->StatComponent->IsDead()))
		{
			return true;
		}
	}
	return false;
}

void AMVRestPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UMVRestPointSubsystem* Session = World->GetSubsystem<UMVRestPointSubsystem>())
		{
			Session->CloseFor(*this);
		}
	}
	Super::EndPlay(EndPlayReason);
}
