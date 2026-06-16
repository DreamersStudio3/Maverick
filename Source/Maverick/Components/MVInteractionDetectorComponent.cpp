#include "Components/MVInteractionDetectorComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interaction/MVInteractableInterface.h"
#include "UI/System/MVUIDataTypes.h"
#include "UI/System/MVUISubsystem.h"

UMVInteractionDetectorComponent::UMVInteractionDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	DefaultPromptText = NSLOCTEXT("MaverickInteraction", "DefaultInteractPrompt", "Interact");
	InteractionObjectChannels = {
		ECC_WorldDynamic,
		ECC_Pawn,
		ECC_PhysicsBody
	};
}

void UMVInteractionDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	TimeUntilNextDetection = 0.0f;
}

void UMVInteractionDetectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearFocusedInteractable();
	Super::EndPlay(EndPlayReason);
}

void UMVInteractionDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeUntilNextDetection -= DeltaTime;
	if (TimeUntilNextDetection > 0.0f)
	{
		return;
	}

	TimeUntilNextDetection = FMath::Max(0.01f, DetectionInterval);
	RefreshInteractable();
}

void UMVInteractionDetectorComponent::SetInteractionDetectionEnabled(bool bInEnabled)
{
	bDetectionEnabled = bInEnabled;
	if (!bDetectionEnabled)
	{
		ClearFocusedInteractable();
	}
}

void UMVInteractionDetectorComponent::RefreshInteractable()
{
	if (!ShouldRunDetection())
	{
		ClearFocusedInteractable();
		return;
	}

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor || DetectionRadius <= 0.0f)
	{
		ClearFocusedInteractable();
		return;
	}

	FVector Origin = OwnerActor->GetActorLocation();
	FVector ViewDirection = OwnerActor->GetActorForwardVector();
	FRotator ViewRotation = OwnerActor->GetActorRotation();
	OwnerActor->GetActorEyesViewPoint(Origin, ViewRotation);
	ViewDirection = ViewRotation.Vector();
	ViewDirection.Z = 0.0f;
	if (!ViewDirection.Normalize())
	{
		ViewDirection = OwnerActor->GetActorForwardVector();
		ViewDirection.Z = 0.0f;
		ViewDirection.Normalize();
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	for (const TEnumAsByte<ECollisionChannel> Channel : InteractionObjectChannels)
	{
		ObjectQueryParams.AddObjectTypesToQuery(Channel.GetValue());
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVInteractionDetector), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByObjectType(
		OverlapResults,
		OwnerActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(DetectionRadius),
		QueryParams);

	FMVInteractionCandidate BestCandidate;
	bool bHasBestCandidate = false;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		FMVInteractionCandidate Candidate;
		if (!TryBuildCandidate(OverlapResult.GetComponent(), OwnerActor->GetActorLocation(), ViewDirection, Candidate))
		{
			continue;
		}

		if (!bHasBestCandidate || Candidate.Score > BestCandidate.Score)
		{
			BestCandidate = Candidate;
			bHasBestCandidate = true;
		}
	}

	UObject* NewInteractable = bHasBestCandidate ? BestCandidate.InteractableObject.Get() : nullptr;
	const bool bSameInteractable = FocusedInteractable.Get() == NewInteractable;
	SetFocusedInteractable(NewInteractable);
	if (bSameInteractable && NewInteractable)
	{
		UpdateInteractionPrompt();
	}
}

void UMVInteractionDetectorComponent::ClearFocusedInteractable()
{
	SetFocusedInteractable(nullptr);
}

bool UMVInteractionDetectorComponent::TryInteract()
{
	RefreshInteractable();

	UObject* InteractableObject = FocusedInteractable.Get();
	if (!IsInteractableAvailable(InteractableObject))
	{
		return false;
	}

	IMVInteractableInterface::Execute_Interact(InteractableObject, GetOwner());
	RefreshInteractable();
	return true;
}

bool UMVInteractionDetectorComponent::ShouldRunDetection() const
{
	if (!bDetectionEnabled || !IsActive())
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn && OwnerPawn->IsPlayerControlled() && OwnerPawn->IsLocallyControlled();
}

bool UMVInteractionDetectorComponent::TryBuildCandidate(
	UPrimitiveComponent* OverlapComponent,
	const FVector& Origin,
	const FVector& ViewDirection,
	FMVInteractionCandidate& OutCandidate) const
{
	UObject* InteractableObject = FindInteractableObject(OverlapComponent);
	if (!IsInteractableAvailable(InteractableObject))
	{
		return false;
	}

	AActor* InteractableActor = Cast<AActor>(InteractableObject);
	if (!InteractableActor)
	{
		if (const UActorComponent* ActorComponent = Cast<UActorComponent>(InteractableObject))
		{
			InteractableActor = ActorComponent->GetOwner();
		}
	}
	if (!InteractableActor)
	{
		InteractableActor = OverlapComponent ? OverlapComponent->GetOwner() : nullptr;
	}

	const FVector CandidateLocation = OverlapComponent
		? OverlapComponent->Bounds.Origin
		: (InteractableActor ? InteractableActor->GetActorLocation() : FVector::ZeroVector);
	const float Distance = FVector::Distance(Origin, CandidateLocation);
	if (Distance > DetectionRadius)
	{
		return false;
	}

	FVector DirectionToCandidate = CandidateLocation - Origin;
	DirectionToCandidate.Z = 0.0f;
	if (!DirectionToCandidate.Normalize())
	{
		DirectionToCandidate = ViewDirection;
	}

	float Dot = 1.0f;
	if (bUseViewCone)
	{
		Dot = FVector::DotProduct(ViewDirection, DirectionToCandidate);
		const float MinDot = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(DetectionHalfAngle, 0.0f, 180.0f)));
		if (Dot < MinDot)
		{
			return false;
		}
	}

	OutCandidate.InteractableObject = InteractableObject;
	OutCandidate.InteractableActor = InteractableActor;
	OutCandidate.InteractableComponent = OverlapComponent;
	OutCandidate.Location = CandidateLocation;

	if (bRequireLineOfSight && !HasLineOfSight(OutCandidate))
	{
		return false;
	}

	const int32 Priority = IMVInteractableInterface::Execute_GetInteractionPriority(InteractableObject, GetOwner());
	const float DistanceScore = DetectionRadius > KINDA_SMALL_NUMBER
		? 1.0f - FMath::Clamp(Distance / DetectionRadius, 0.0f, 1.0f)
		: 0.0f;
	OutCandidate.Score = static_cast<float>(Priority) * 100000.0f + Dot * 1000.0f + DistanceScore * 100.0f;
	return true;
}

bool UMVInteractionDetectorComponent::HasLineOfSight(const FMVInteractionCandidate& Candidate) const
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return false;
	}

	FVector EyeLocation = OwnerActor->GetActorLocation();
	FRotator EyeRotation = OwnerActor->GetActorRotation();
	OwnerActor->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVInteractionLineOfSight), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		EyeLocation,
		Candidate.Location,
		LineOfSightChannel,
		QueryParams);

	if (!bHit)
	{
		return true;
	}

	return HitResult.GetActor() == Candidate.InteractableActor.Get()
		|| HitResult.GetComponent() == Candidate.InteractableComponent.Get();
}

UObject* UMVInteractionDetectorComponent::FindInteractableObject(UPrimitiveComponent* OverlapComponent) const
{
	AActor* Actor = OverlapComponent ? OverlapComponent->GetOwner() : nullptr;
	if (Actor && Actor->GetClass()->ImplementsInterface(UMVInteractableInterface::StaticClass()))
	{
		return Actor;
	}

	if (OverlapComponent && OverlapComponent->GetClass()->ImplementsInterface(UMVInteractableInterface::StaticClass()))
	{
		return OverlapComponent;
	}

	if (Actor)
	{
		TArray<UActorComponent*> ActorComponents;
		Actor->GetComponents(ActorComponents);
		for (UActorComponent* ActorComponent : ActorComponents)
		{
			if (ActorComponent && ActorComponent->GetClass()->ImplementsInterface(UMVInteractableInterface::StaticClass()))
			{
				return ActorComponent;
			}
		}
	}

	return nullptr;
}

bool UMVInteractionDetectorComponent::IsInteractableAvailable(UObject* InteractableObject) const
{
	return IsValid(InteractableObject)
		&& InteractableObject->GetClass()->ImplementsInterface(UMVInteractableInterface::StaticClass())
		&& IMVInteractableInterface::Execute_CanInteract(InteractableObject, GetOwner());
}

void UMVInteractionDetectorComponent::SetFocusedInteractable(UObject* NewInteractable)
{
	if (FocusedInteractable.Get() == NewInteractable)
	{
		return;
	}

	UObject* PreviousInteractable = FocusedInteractable.Get();
	FocusedInteractable = NewInteractable;
	OnFocusedInteractableChanged.Broadcast(PreviousInteractable, NewInteractable);
	UpdateInteractionPrompt();
}

void UMVInteractionDetectorComponent::UpdateInteractionPrompt()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (!UISubsystem)
	{
		return;
	}

	UObject* InteractableObject = FocusedInteractable.Get();
	if (!IsInteractableAvailable(InteractableObject))
	{
		UISubsystem->HideInteractionPrompt();
		return;
	}

	FMVInteractionPromptData PromptData;
	PromptData.PromptText = IMVInteractableInterface::Execute_GetInteractionPromptText(InteractableObject, GetOwner());
	if (PromptData.PromptText.IsEmpty())
	{
		PromptData.PromptText = DefaultPromptText;
	}
	PromptData.InputKey = InteractionInputKey;

	UISubsystem->ShowInteractionPrompt(PromptData);
}
