#include "Character/PC/InteractionDetector/MVPlayerInteractionDetector.h"

#include "Character/MVCharacterBase.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/ActorComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Interaction/MVInteractableInterface.h"
#include "TimerManager.h"
#if ENABLE_DRAW_DEBUG
#include "DrawDebugHelpers.h"
#endif
#include "UI/System/MVUIDataTypes.h"
#include "UI/System/MVUISubsystem.h"

UMVPlayerInteractionDetector::UMVPlayerInteractionDetector()
{
	DefaultPromptText = NSLOCTEXT("MaverickInteraction", "DefaultInteractPrompt", "Interact");
	InteractionObjectChannels = {
		ECC_WorldDynamic,
		ECC_Pawn,
		ECC_PhysicsBody
	};
}

UWorld* UMVPlayerInteractionDetector::GetWorld() const
{
	if (const AMVPlayerCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetWorld();
	}

	return Super::GetWorld();
}

void UMVPlayerInteractionDetector::Initialize(AMVPlayerCharacter& InOwnerCharacter)
{
	if (bInitialized)
	{
		Deinitialize();
	}

	OwnerPlayerCharacter = &InOwnerCharacter;
	bInitialized = true;
	TimeUntilNextDetection = 0.0f;
}

void UMVPlayerInteractionDetector::Deinitialize()
{
	ClearFocusedInteractable();
	OwnerPlayerCharacter.Reset();
	TimeUntilNextDetection = 0.0f;
	bInitialized = false;
	bWaitForInteractionInputRelease = false;
}

void UMVPlayerInteractionDetector::Tick(const float DeltaTime)
{
	if (!bInitialized)
	{
		return;
	}

	UpdateInteractionInputReleaseGate();
	UpdateDialogueEscapeState();

	TimeUntilNextDetection -= DeltaTime;
	if (TimeUntilNextDetection > 0.0f)
	{
		return;
	}

	TimeUntilNextDetection = FMath::Max(0.01f, DetectionInterval);
	RefreshInteractable();
}

AMVPlayerCharacter* UMVPlayerInteractionDetector::GetPlayerCharacter() const
{
	return OwnerPlayerCharacter.Get();
}

AActor* UMVPlayerInteractionDetector::GetOwnerActor() const
{
	return GetPlayerCharacter();
}

void UMVPlayerInteractionDetector::SetInteractionDetectionEnabled(bool bInEnabled)
{
	bDetectionEnabled = bInEnabled;
	if (!bDetectionEnabled)
	{
		ClearFocusedInteractable();
	}
}

void UMVPlayerInteractionDetector::RefreshInteractable()
{
	if (!ShouldRunDetection())
	{
		ClearFocusedInteractable();
		return;
	}

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwnerActor();
	if (!World || !OwnerActor || DetectionRange <= 0.0f)
	{
		ClearFocusedInteractable();
		return;
	}

	if (SuppressedInteractable.IsValid())
	{
		const bool bInteractionHoldActive = IsDialogueInteractionBlocked() || IsInteractionSessionActive();
		if (!bInteractionHoldActive)
		{
			ReleaseSuppressedInteractable(false);
		}
		else
		{
			if (!IsInteractableWithinDetectionRange(SuppressedInteractable.Get()))
			{
				RestoreDialogueCameraZoom();
			}
			if (!IsInteractableWithinDialogueEscapeRange(SuppressedInteractable.Get()))
			{
				ReleaseSuppressedInteractable(true);
			}
		}
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

#if ENABLE_DRAW_DEBUG
	const float DebugDrawDuration = FMath::Max(0.01f, DetectionInterval * 1.2f);
	if (bDrawDebugDetection)
	{
		DrawDebugCircle(
			World,
			OwnerActor->GetActorLocation(),
			DetectionRange,
			96,
			FColor(80, 220, 255),
			false,
			DebugDrawDuration,
			0,
			2.0f,
			FVector::ForwardVector,
			FVector::RightVector,
			false);
		DrawDebugLine(
			World,
			OwnerActor->GetActorLocation(),
			OwnerActor->GetActorLocation() + ViewDirection * DetectionRange,
			FColor::Blue,
			false,
			DebugDrawDuration,
			0,
			2.0f);
		if (bUseViewCone)
		{
			const FVector LeftViewConeDirection = ViewDirection.RotateAngleAxis(DetectionHalfAngle, FVector::UpVector);
			const FVector RightViewConeDirection = ViewDirection.RotateAngleAxis(-DetectionHalfAngle, FVector::UpVector);
			DrawDebugLine(
				World,
				OwnerActor->GetActorLocation(),
				OwnerActor->GetActorLocation() + LeftViewConeDirection * DetectionRange,
				FColor(80, 220, 255),
				false,
				DebugDrawDuration,
				0,
				1.5f);
			DrawDebugLine(
				World,
				OwnerActor->GetActorLocation(),
				OwnerActor->GetActorLocation() + RightViewConeDirection * DetectionRange,
				FColor(80, 220, 255),
				false,
				DebugDrawDuration,
				0,
				1.5f);
		}
	}
#endif

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByObjectType(
		OverlapResults,
		OwnerActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(DetectionRange),
		QueryParams);

	TArray<FMVInteractionCandidate> DetectedCandidates;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		UPrimitiveComponent* OverlapComponent = OverlapResult.GetComponent();
#if ENABLE_DRAW_DEBUG
		if (bDrawDebugDetection && OverlapComponent)
		{
			DrawDebugSphere(
				World,
				OverlapComponent->Bounds.Origin,
				12.0f,
				8,
				FColor::Yellow,
				false,
				DebugDrawDuration,
				0,
				1.5f);
		}
#endif

		FMVInteractionCandidate Candidate;
		if (!TryBuildCandidate(OverlapComponent, OwnerActor->GetActorLocation(), ViewDirection, Candidate))
		{
			continue;
		}

#if ENABLE_DRAW_DEBUG
		if (bDrawDebugDetection)
		{
			DrawDebugLine(
				World,
				OwnerActor->GetActorLocation(),
				Candidate.Location,
				FColor::Green,
				false,
				DebugDrawDuration,
				0,
				1.0f);
			DrawDebugSphere(
				World,
				Candidate.Location,
				18.0f,
				12,
				FColor::Green,
				false,
				DebugDrawDuration,
				0,
				2.0f);
		}
#endif

		const int32 ExistingCandidateIndex = DetectedCandidates.IndexOfByPredicate(
			[InteractableObject = Candidate.InteractableObject.Get()](const FMVInteractionCandidate& ExistingCandidate)
			{
				return ExistingCandidate.InteractableObject.Get() == InteractableObject;
			});
		if (ExistingCandidateIndex == INDEX_NONE)
		{
			DetectedCandidates.Add(Candidate);
		}
		else if (Candidate.Score > DetectedCandidates[ExistingCandidateIndex].Score)
		{
			DetectedCandidates[ExistingCandidateIndex] = Candidate;
		}
	}

	DetectedCandidates.Sort([](const FMVInteractionCandidate& Left, const FMVInteractionCandidate& Right)
	{
		return Left.Score > Right.Score;
	});

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugDetection && !DetectedCandidates.IsEmpty())
	{
		DrawDebugSphere(
			World,
			DetectedCandidates[0].Location,
			28.0f,
			16,
			FColor(0, 255, 128),
			false,
			DebugDrawDuration,
			0,
			3.0f);
	}
#endif

	InteractionCandidates = MoveTemp(DetectedCandidates);
	if (InteractionCandidates.IsEmpty())
	{
		SelectedCandidateIndex = INDEX_NONE;
		if (!SuppressedInteractable.IsValid())
		{
			SetFocusedInteractable(nullptr);
		}
		else
		{
			UpdateInteractionPrompt();
		}
		return;
	}

	int32 NewSelectedCandidateIndex = FindCandidateIndex(FocusedInteractable.Get());
	if (NewSelectedCandidateIndex == INDEX_NONE)
	{
		NewSelectedCandidateIndex = 0;
	}
	if (SuppressedInteractable.IsValid())
	{
		const int32 SuppressedCandidateIndex = FindCandidateIndex(SuppressedInteractable.Get());
		if (SuppressedCandidateIndex == INDEX_NONE)
		{
			SelectedCandidateIndex = INDEX_NONE;
			UpdateInteractionPrompt();
			return;
		}

		NewSelectedCandidateIndex = SuppressedCandidateIndex;
	}

	UObject* NewInteractable = InteractionCandidates[NewSelectedCandidateIndex].InteractableObject.Get();
	if (SuppressedInteractable.IsValid()
		&& SuppressedInteractable.Get() == NewInteractable
		&& !IsDialogueInteractionBlocked()
		&& !IsInteractionSessionActive())
	{
		ReleaseSuppressedInteractable(false);
	}

	const bool bSameInteractable = FocusedInteractable.Get() == NewInteractable;
	SelectedCandidateIndex = NewSelectedCandidateIndex;
	SetFocusedInteractable(NewInteractable);
	if (bSameInteractable && NewInteractable)
	{
		UpdateInteractionPrompt();
	}
}

void UMVPlayerInteractionDetector::ClearFocusedInteractable()
{
	InteractionCandidates.Reset();
	SelectedCandidateIndex = INDEX_NONE;
	ReleaseSuppressedInteractable(true);
	HideInteractionPrompt();
	SetFocusedInteractable(nullptr);
}

bool UMVPlayerInteractionDetector::TryInteract()
{
	UpdateInteractionInputReleaseGate();
	if (bWaitForInteractionInputRelease)
	{
		return false;
	}

	if (IsDialoguePopupActive())
	{
		const bool bSkipped = SkipActiveDialoguePopup();
		if (bSkipped)
		{
			LockInteractionUntilInputReleased();
		}
		return bSkipped;
	}

	if (IsDialogueInteractionBlocked())
	{
		return false;
	}
	if (IsInteractionSessionActive())
	{
		return false;
	}

	RefreshInteractable();

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (!UISubsystem || !UISubsystem->CanUseInteractionPrompt())
	{
		return false;
	}

	UObject* InteractableObject = FocusedInteractable.Get();
	if (!IsInteractableAvailable(InteractableObject) || IsInteractableSuppressed(InteractableObject))
	{
		return false;
	}

	UISubsystem->HideInteractionPrompt();

	SuppressedInteractable = InteractableObject;
	LockInteractionUntilInputReleased();
	IMVInteractableInterface::Execute_Interact(InteractableObject, GetOwnerActor());
	return true;
}

bool UMVPlayerInteractionDetector::SelectNextInteractable()
{
	return SelectInteractableByOffset(1);
}

bool UMVPlayerInteractionDetector::SelectPreviousInteractable()
{
	return SelectInteractableByOffset(-1);
}

bool UMVPlayerInteractionDetector::ShouldRunDetection() const
{
	if (!bDetectionEnabled || !bInitialized)
	{
		return false;
	}

	if (IsOwnerDead())
	{
		return false;
	}

	const APawn* OwnerPawn = GetPlayerCharacter();
	return OwnerPawn && OwnerPawn->IsPlayerControlled() && OwnerPawn->IsLocallyControlled();
}

bool UMVPlayerInteractionDetector::TryBuildCandidate(
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

	AActor* InteractableActor = ResolveInteractableActor(InteractableObject, OverlapComponent);

	const FVector CandidateLocation = OverlapComponent
		? OverlapComponent->Bounds.Origin
		: (InteractableActor ? InteractableActor->GetActorLocation() : FVector::ZeroVector);
	const float Distance = FVector::Distance(Origin, CandidateLocation);
	if (Distance > DetectionRange)
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

	const int32 Priority = IMVInteractableInterface::Execute_GetInteractionPriority(InteractableObject, GetOwnerActor());
	const float DistanceScore = DetectionRange > KINDA_SMALL_NUMBER
		? 1.0f - FMath::Clamp(Distance / DetectionRange, 0.0f, 1.0f)
		: 0.0f;
	OutCandidate.Score = static_cast<float>(Priority) * 100000.0f + Dot * 1000.0f + DistanceScore * 100.0f;
	return true;
}

bool UMVPlayerInteractionDetector::HasLineOfSight(const FMVInteractionCandidate& Candidate) const
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwnerActor();
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

UObject* UMVPlayerInteractionDetector::FindInteractableObject(UPrimitiveComponent* OverlapComponent) const
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

AActor* UMVPlayerInteractionDetector::ResolveInteractableActor(UObject* InteractableObject, UPrimitiveComponent* FallbackComponent) const
{
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
		InteractableActor = FallbackComponent ? FallbackComponent->GetOwner() : nullptr;
	}

	return InteractableActor;
}

bool UMVPlayerInteractionDetector::IsInteractableAvailable(UObject* InteractableObject) const
{
	return IsValid(InteractableObject)
		&& InteractableObject->GetClass()->ImplementsInterface(UMVInteractableInterface::StaticClass())
		&& IMVInteractableInterface::Execute_CanInteract(InteractableObject, GetOwnerActor());
}

bool UMVPlayerInteractionDetector::IsInteractableSuppressed(UObject* InteractableObject) const
{
	return InteractableObject && SuppressedInteractable.Get() == InteractableObject;
}

bool UMVPlayerInteractionDetector::IsInteractableWithinDetectionRange(UObject* InteractableObject) const
{
	const AActor* OwnerActor = GetOwnerActor();
	const AActor* InteractableActor = ResolveInteractableActor(InteractableObject);
	if (!OwnerActor || !InteractableActor || DetectionRange <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared(OwnerActor->GetActorLocation(), InteractableActor->GetActorLocation())
		<= FMath::Square(DetectionRange);
}

bool UMVPlayerInteractionDetector::IsInteractableWithinDialogueEscapeRange(UObject* InteractableObject) const
{
	const AActor* OwnerActor = GetOwnerActor();
	const AActor* InteractableActor = ResolveInteractableActor(InteractableObject);
	const float EscapeRange = FMath::Max(DetectionRange, DialogueEscapeRange);
	if (!OwnerActor || !InteractableActor || EscapeRange <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared(OwnerActor->GetActorLocation(), InteractableActor->GetActorLocation())
		<= FMath::Square(EscapeRange);
}

bool UMVPlayerInteractionDetector::IsDialoguePopupActive() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	return UISubsystem && UISubsystem->IsDialoguePopupActive();
}

bool UMVPlayerInteractionDetector::IsDialogueInteractionBlocked() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	return UISubsystem && UISubsystem->IsDialoguePopupBlockingInteraction();
}

bool UMVPlayerInteractionDetector::IsInteractionSessionActive() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	return UISubsystem && (UISubsystem->IsInteractionSessionActive() || UISubsystem->IsInteractionMenuActive());
}

bool UMVPlayerInteractionDetector::IsOwnerDead() const
{
	const AMVCharacterBase* OwnerCharacter = GetPlayerCharacter();
	return OwnerCharacter && OwnerCharacter->StatComponent && OwnerCharacter->StatComponent->IsDead();
}

bool UMVPlayerInteractionDetector::SkipActiveDialoguePopup() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (!UISubsystem || !UISubsystem->CanSkipDialoguePopup())
	{
		return false;
	}

	UISubsystem->SkipDialoguePopup();
	return true;
}

void UMVPlayerInteractionDetector::HideInteractionPrompt() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->HideInteractionPrompt();
	}
}

void UMVPlayerInteractionDetector::HideActiveDialoguePopup() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->HideDialoguePopup();
	}
}

void UMVPlayerInteractionDetector::HideInteractionMenu() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->HideInteractionMenu();
	}
}

void UMVPlayerInteractionDetector::RestoreDialogueCameraZoom() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->RestoreDialogueCameraZoom();
	}
}

void UMVPlayerInteractionDetector::ReleaseSuppressedInteractable(bool bHideDialogue)
{
	if (!SuppressedInteractable.IsValid())
	{
		return;
	}

	if (bHideDialogue)
	{
		HideActiveDialoguePopup();
		HideInteractionMenu();
	}

	SuppressedInteractable = nullptr;
}

void UMVPlayerInteractionDetector::UpdateDialogueEscapeState()
{
	if (!SuppressedInteractable.IsValid()
		|| (!IsDialogueInteractionBlocked() && !IsInteractionSessionActive()))
	{
		return;
	}

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugDetection)
	{
		UWorld* World = GetWorld();
		const AActor* InteractableActor = ResolveInteractableActor(SuppressedInteractable.Get());
		if (World && InteractableActor)
		{
			const float EscapeRange = FMath::Max(DetectionRange, DialogueEscapeRange);
			const float DebugDrawDuration = FMath::Max(0.05f, World->GetDeltaSeconds() * 2.0f);
			DrawDebugCircle(
				World,
				InteractableActor->GetActorLocation(),
				EscapeRange,
				128,
				FColor::Red,
				false,
				DebugDrawDuration,
				0,
				2.0f,
				FVector::ForwardVector,
				FVector::RightVector,
				false);
		}
	}
#endif

	if (!IsInteractableWithinDetectionRange(SuppressedInteractable.Get()))
	{
		RestoreDialogueCameraZoom();
	}

	if (IsInteractableWithinDialogueEscapeRange(SuppressedInteractable.Get()))
	{
		return;
	}

	ReleaseSuppressedInteractable(true);
	RefreshInteractable();
}

void UMVPlayerInteractionDetector::LockInteractionUntilInputReleased()
{
	if (InteractionInputKey.IsValid() && IsInteractionInputHeld())
	{
		bWaitForInteractionInputRelease = true;
	}
}

void UMVPlayerInteractionDetector::UpdateInteractionInputReleaseGate()
{
	if (!bWaitForInteractionInputRelease)
	{
		return;
	}

	if (!InteractionInputKey.IsValid() || !IsInteractionInputHeld())
	{
		bWaitForInteractionInputRelease = false;
	}
}

bool UMVPlayerInteractionDetector::IsInteractionInputHeld() const
{
	const UWorld* World = GetWorld();
	const APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	return PlayerController && InteractionInputKey.IsValid() && PlayerController->IsInputKeyDown(InteractionInputKey);
}

int32 UMVPlayerInteractionDetector::FindCandidateIndex(UObject* InteractableObject) const
{
	if (!InteractableObject)
	{
		return INDEX_NONE;
	}

	return InteractionCandidates.IndexOfByPredicate([InteractableObject](const FMVInteractionCandidate& Candidate)
	{
		return Candidate.InteractableObject.Get() == InteractableObject;
	});
}

bool UMVPlayerInteractionDetector::SelectInteractableByOffset(int32 Offset)
{
	if (SuppressedInteractable.IsValid() || IsDialogueInteractionBlocked() || IsInteractionSessionActive())
	{
		return false;
	}

	RefreshInteractable();
	if (InteractionCandidates.Num() <= 1 || Offset == 0)
	{
		return false;
	}

	const int32 BaseIndex = SelectedCandidateIndex != INDEX_NONE ? SelectedCandidateIndex : 0;
	const int32 CandidateCount = InteractionCandidates.Num();
	const int32 NewIndex = (BaseIndex + Offset + CandidateCount) % CandidateCount;
	return SetSelectedCandidateIndex(NewIndex);
}

bool UMVPlayerInteractionDetector::SetSelectedCandidateIndex(int32 NewIndex)
{
	if (!InteractionCandidates.IsValidIndex(NewIndex))
	{
		return false;
	}

	SelectedCandidateIndex = NewIndex;
	SetFocusedInteractable(InteractionCandidates[SelectedCandidateIndex].InteractableObject.Get());
	UpdateInteractionPrompt();
	return true;
}

void UMVPlayerInteractionDetector::SetFocusedInteractable(UObject* NewInteractable)
{
	if (SuppressedInteractable.IsValid() && SuppressedInteractable.Get() != NewInteractable)
	{
		if (!IsInteractableWithinDialogueEscapeRange(SuppressedInteractable.Get()))
		{
			ReleaseSuppressedInteractable(true);
		}
		else if (IsDialogueInteractionBlocked() || IsInteractionSessionActive())
		{
			UpdateInteractionPrompt();
			return;
		}
		else
		{
			ReleaseSuppressedInteractable(false);
		}
	}

	if (FocusedInteractable.Get() == NewInteractable)
	{
		return;
	}

	UObject* PreviousInteractable = FocusedInteractable.Get();
	FocusedInteractable = NewInteractable;
	OnFocusedInteractableChanged.Broadcast(PreviousInteractable, NewInteractable);
	UpdateInteractionPrompt();
}

void UMVPlayerInteractionDetector::UpdateInteractionPrompt()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (!UISubsystem)
	{
		return;
	}

	UObject* InteractableObject = FocusedInteractable.Get();
	if (!IsInteractableAvailable(InteractableObject) || IsInteractableSuppressed(InteractableObject))
	{
		UISubsystem->HideInteractionPrompt();
		return;
	}

	FMVInteractionPromptData PromptData;
	PromptData.PromptText = IMVInteractableInterface::Execute_GetInteractionPromptText(InteractableObject, GetOwnerActor());
	if (PromptData.PromptText.IsEmpty())
	{
		PromptData.PromptText = DefaultPromptText;
	}
	PromptData.InputKey = InteractionInputKey;
	PromptData.CandidateIndex = SelectedCandidateIndex != INDEX_NONE ? SelectedCandidateIndex : 0;
	PromptData.CandidateCount = InteractionCandidates.Num();

	UISubsystem->ShowInteractionPrompt(PromptData);
}
