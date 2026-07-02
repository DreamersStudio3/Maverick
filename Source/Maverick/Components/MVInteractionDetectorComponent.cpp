#include "Components/MVInteractionDetectorComponent.h"

#include "Character/MVCharacterBase.h"
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
	if (!World || !OwnerActor || DetectionRange <= 0.0f)
	{
		ClearFocusedInteractable();
		return;
	}

	if (SuppressedInteractable.IsValid())
	{
		const bool bInteractionHoldActive = IsDialogueInteractionBlocked() || IsPIEActionTestPanelActiveOrPending();
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
		&& !IsDialogueInteractionBlocked())
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

void UMVInteractionDetectorComponent::ClearFocusedInteractable()
{
	InteractionCandidates.Reset();
	SelectedCandidateIndex = INDEX_NONE;
	ReleaseSuppressedInteractable(true);
	HideInteractionPrompt();
	SetFocusedInteractable(nullptr);
}

bool UMVInteractionDetectorComponent::TryInteract()
{
	UpdateInteractionInputReleaseGate();
	if (bWaitForInteractionInputRelease)
	{
		return false;
	}

	if (IsDialogueWindowActive())
	{
		const bool bSkipped = SkipActiveDialogueWindow();
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
	TryShowPIEActionTestPanelForInteractable(InteractableObject);
	IMVInteractableInterface::Execute_Interact(InteractableObject, GetOwner());
	return true;
}

bool UMVInteractionDetectorComponent::SelectNextInteractable()
{
	return SelectInteractableByOffset(1);
}

bool UMVInteractionDetectorComponent::SelectPreviousInteractable()
{
	return SelectInteractableByOffset(-1);
}

bool UMVInteractionDetectorComponent::ShouldRunDetection() const
{
	if (!bDetectionEnabled || !IsActive())
	{
		return false;
	}

	if (IsOwnerDead())
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

	const int32 Priority = IMVInteractableInterface::Execute_GetInteractionPriority(InteractableObject, GetOwner());
	const float DistanceScore = DetectionRange > KINDA_SMALL_NUMBER
		? 1.0f - FMath::Clamp(Distance / DetectionRange, 0.0f, 1.0f)
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

AActor* UMVInteractionDetectorComponent::ResolveInteractableActor(UObject* InteractableObject, UPrimitiveComponent* FallbackComponent) const
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

bool UMVInteractionDetectorComponent::IsInteractableAvailable(UObject* InteractableObject) const
{
	return IsValid(InteractableObject)
		&& InteractableObject->GetClass()->ImplementsInterface(UMVInteractableInterface::StaticClass())
		&& IMVInteractableInterface::Execute_CanInteract(InteractableObject, GetOwner());
}

bool UMVInteractionDetectorComponent::IsInteractableSuppressed(UObject* InteractableObject) const
{
	return InteractableObject && SuppressedInteractable.Get() == InteractableObject;
}

bool UMVInteractionDetectorComponent::IsInteractableWithinDetectionRange(UObject* InteractableObject) const
{
	const AActor* OwnerActor = GetOwner();
	const AActor* InteractableActor = ResolveInteractableActor(InteractableObject);
	if (!OwnerActor || !InteractableActor || DetectionRange <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared(OwnerActor->GetActorLocation(), InteractableActor->GetActorLocation())
		<= FMath::Square(DetectionRange);
}

bool UMVInteractionDetectorComponent::IsInteractableWithinDialogueEscapeRange(UObject* InteractableObject) const
{
	const AActor* OwnerActor = GetOwner();
	const AActor* InteractableActor = ResolveInteractableActor(InteractableObject);
	const float EscapeRange = FMath::Max(DetectionRange, DialogueEscapeRange);
	if (!OwnerActor || !InteractableActor || EscapeRange <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared(OwnerActor->GetActorLocation(), InteractableActor->GetActorLocation())
		<= FMath::Square(EscapeRange);
}

bool UMVInteractionDetectorComponent::IsDialogueWindowActive() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	return UISubsystem && UISubsystem->IsDialogueWindowActive();
}

bool UMVInteractionDetectorComponent::IsDialogueInteractionBlocked() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	return UISubsystem && UISubsystem->IsDialogueWindowBlockingInteraction();
}

bool UMVInteractionDetectorComponent::IsOwnerDead() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	return OwnerCharacter && OwnerCharacter->StatComponent && OwnerCharacter->StatComponent->IsDead();
}

bool UMVInteractionDetectorComponent::IsPIEActionTestPanelActiveOrPending() const
{
#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	return UISubsystem && UISubsystem->IsPIEActionTestPanelActiveOrPending();
#else
	return false;
#endif
}

bool UMVInteractionDetectorComponent::IsPIEActionTestInteractable(UObject* InteractableObject) const
{
#if !UE_BUILD_SHIPPING
	const AActor* InteractableActor = ResolveInteractableActor(InteractableObject);
	const UObject* ObjectForName = InteractableActor ? Cast<const UObject>(InteractableActor) : InteractableObject;
	if (!ObjectForName)
	{
		return false;
	}

	if (Cast<AMVCharacterBase>(InteractableActor) && InteractableActor != GetOwner())
	{
		return true;
	}

	const FString ObjectName = ObjectForName->GetName();
	const UClass* ObjectClass = ObjectForName->GetClass();
	const FString ClassName = ObjectClass ? ObjectClass->GetName() : FString();
	return ObjectName.Contains(TEXT("Carcass"), ESearchCase::IgnoreCase)
		|| ClassName.Contains(TEXT("Carcass"), ESearchCase::IgnoreCase)
		|| ObjectName.Contains(TEXT("BP_Carcass"), ESearchCase::IgnoreCase)
		|| ClassName.Contains(TEXT("BP_Carcass"), ESearchCase::IgnoreCase)
		|| ObjectName.Contains(TEXT("BPCarcass"), ESearchCase::IgnoreCase)
		|| ClassName.Contains(TEXT("BPCarcass"), ESearchCase::IgnoreCase);
#else
	return false;
#endif
}

bool UMVInteractionDetectorComponent::SkipActiveDialogueWindow() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (!UISubsystem || !UISubsystem->CanSkipDialogueWindow())
	{
		return false;
	}

	UISubsystem->SkipDialogueWindow();
	return true;
}

void UMVInteractionDetectorComponent::HideInteractionPrompt() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->HideInteractionPrompt();
	}
}

void UMVInteractionDetectorComponent::HideActiveDialogueWindow() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->HideDialogueWindow();
	}
}

void UMVInteractionDetectorComponent::HidePIEActionTestPanel() const
{
#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->HidePIEActionTestPanel();
	}
#endif
}

void UMVInteractionDetectorComponent::TryShowPIEActionTestPanelForInteractable(UObject* InteractableObject)
{
#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	if (!World || World->WorldType != EWorldType::PIE || !IsPIEActionTestInteractable(InteractableObject))
	{
		return;
	}

	TWeakObjectPtr<AMVCharacterBase> TargetCharacter = Cast<AMVCharacterBase>(ResolveInteractableActor(InteractableObject));
	if (!TargetCharacter.IsValid())
	{
		return;
	}

	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(
		this,
		[this, TargetCharacter]()
		{
			UWorld* CallbackWorld = GetWorld();
			UGameInstance* GameInstance = CallbackWorld ? CallbackWorld->GetGameInstance() : nullptr;
			UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
			if (TargetCharacter.IsValid() && UISubsystem)
			{
				UISubsystem->ShowPIEActionTestPanel(TargetCharacter.Get());
			}
		}));
#endif
}

void UMVInteractionDetectorComponent::RestoreDialogueCameraZoom() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->RestoreDialogueCameraZoom();
	}
}

void UMVInteractionDetectorComponent::ReleaseSuppressedInteractable(bool bHideDialogue)
{
	if (!SuppressedInteractable.IsValid())
	{
		return;
	}

	if (bHideDialogue)
	{
		HideActiveDialogueWindow();
		HidePIEActionTestPanel();
	}

	SuppressedInteractable = nullptr;
}

void UMVInteractionDetectorComponent::UpdateDialogueEscapeState()
{
	if (!SuppressedInteractable.IsValid()
		|| (!IsDialogueInteractionBlocked() && !IsPIEActionTestPanelActiveOrPending()))
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

void UMVInteractionDetectorComponent::LockInteractionUntilInputReleased()
{
	if (InteractionInputKey.IsValid() && IsInteractionInputHeld())
	{
		bWaitForInteractionInputRelease = true;
	}
}

void UMVInteractionDetectorComponent::UpdateInteractionInputReleaseGate()
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

bool UMVInteractionDetectorComponent::IsInteractionInputHeld() const
{
	const UWorld* World = GetWorld();
	const APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	return PlayerController && InteractionInputKey.IsValid() && PlayerController->IsInputKeyDown(InteractionInputKey);
}

int32 UMVInteractionDetectorComponent::FindCandidateIndex(UObject* InteractableObject) const
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

bool UMVInteractionDetectorComponent::SelectInteractableByOffset(int32 Offset)
{
	if (SuppressedInteractable.IsValid() || IsDialogueInteractionBlocked())
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

bool UMVInteractionDetectorComponent::SetSelectedCandidateIndex(int32 NewIndex)
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

void UMVInteractionDetectorComponent::SetFocusedInteractable(UObject* NewInteractable)
{
	if (SuppressedInteractable.IsValid() && SuppressedInteractable.Get() != NewInteractable)
	{
		if (!IsInteractableWithinDialogueEscapeRange(SuppressedInteractable.Get()))
		{
			ReleaseSuppressedInteractable(true);
		}
		else if (IsDialogueInteractionBlocked())
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
	if (!IsInteractableAvailable(InteractableObject) || IsInteractableSuppressed(InteractableObject))
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
	PromptData.CandidateIndex = SelectedCandidateIndex != INDEX_NONE ? SelectedCandidateIndex : 0;
	PromptData.CandidateCount = InteractionCandidates.Num();

	UISubsystem->ShowInteractionPrompt(PromptData);
}
