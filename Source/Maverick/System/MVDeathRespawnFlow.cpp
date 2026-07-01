#include "System/MVDeathRespawnFlow.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVDeathComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVDeathOverlayWindow.h"

namespace
{
constexpr float MVDeathRespawnOverlayCompletionToLoadingDelaySeconds = 1.0f;
}

void UMVDeathRespawnFlow::Initialize(UMVFieldTransitionSubsystem* InTransitionSubsystem)
{
	TransitionSubsystem = InTransitionSubsystem;
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMVDeathRespawnFlow::HandleWorldInit);

	if (UWorld* World = GetWorld())
	{
		BindToPlayerDeath(World);
	}
}

void UMVDeathRespawnFlow::Shutdown()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	UnbindFromPlayerDeath();
	ResetDeathRespawnState();
	TransitionSubsystem = nullptr;
}

UWorld* UMVDeathRespawnFlow::GetWorld() const
{
	return TransitionSubsystem ? TransitionSubsystem->GetWorld() : nullptr;
}

void UMVDeathRespawnFlow::HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	FTimerHandle PostWorldInitTimerHandle;
	const TWeakObjectPtr<UWorld> WeakWorld = World;
	World->GetTimerManager().SetTimer(
		PostWorldInitTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
		{
			UWorld* CurrentWorld = WeakWorld.Get();
			if (CurrentWorld && CurrentWorld->IsGameWorld())
			{
				BindToPlayerDeath(CurrentWorld);
			}
		}),
		0.1f,
		false);
}

void UMVDeathRespawnFlow::BindToPlayerDeath(UWorld* World)
{
	AMVCharacterBase* Character = ResolvePlayerCharacter(World);
	UMVDeathComponent* DeathComponent = Character ? Character->DeathComponent.Get() : nullptr;
	if (!DeathComponent)
	{
		return;
	}

	if (BoundDeathComponent && BoundDeathComponent != DeathComponent)
	{
		UnbindFromPlayerDeath();
	}

	BoundDeathComponent = DeathComponent;
	BoundDeathComponent->OnDeathPresentationStarted.RemoveDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathPresentationStarted);
	BoundDeathComponent->OnDeathPresentationStarted.AddUniqueDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathPresentationStarted);
	BoundDeathComponent->OnDeathOverlayRequested.RemoveDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathOverlayRequested);
	BoundDeathComponent->OnDeathOverlayRequested.AddUniqueDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathOverlayRequested);
	BoundDeathComponent->OnDeathPresentationFinished.RemoveDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathPresentationFinished);
	BoundDeathComponent->OnDeathPresentationFinished.AddUniqueDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathPresentationFinished);
}

void UMVDeathRespawnFlow::UnbindFromPlayerDeath()
{
	if (!BoundDeathComponent)
	{
		return;
	}

	BoundDeathComponent->OnDeathPresentationStarted.RemoveDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathPresentationStarted);
	BoundDeathComponent->OnDeathOverlayRequested.RemoveDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathOverlayRequested);
	BoundDeathComponent->OnDeathPresentationFinished.RemoveDynamic(
		this,
		&UMVDeathRespawnFlow::HandlePlayerDeathPresentationFinished);
	BoundDeathComponent = nullptr;
}

bool UMVDeathRespawnFlow::BeginDeathSequence(AActor* DeadActor)
{
	if (bDeathSequenceStarted)
	{
		return false;
	}

	PendingDeadActor = DeadActor;
	bDeathSequenceStarted = true;
	bDeathOverlayCompleted = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathOverlayCompletionDelayTimerHandle);
	}

	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->ClearAllUI(true);
		ActiveDeathOverlay.Reset();
	}

	return true;
}

void UMVDeathRespawnFlow::TryStartDeathRespawnTransition()
{
	if (!bDeathSequenceStarted || !TransitionSubsystem)
	{
		return;
	}

	if (UMVDeathOverlayWindow* DeathOverlay = ActiveDeathOverlay.Get())
	{
		DeathOverlay->OnMinimumDisplayElapsed.RemoveDynamic(
			this,
			&UMVDeathRespawnFlow::HandleDeathOverlayMinimumDisplayElapsed);
		DeathOverlay->DeactivateWidget();
		ActiveDeathOverlay.Reset();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathOverlayCompletionDelayTimerHandle);
	}

	AActor* DeadActor = PendingDeadActor.Get();
	if (TransitionSubsystem->StartDeathRespawnTransition(DeadActor))
	{
		ResetDeathRespawnState();
	}
}

void UMVDeathRespawnFlow::ResetDeathRespawnState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathOverlayCompletionDelayTimerHandle);
	}

	if (UMVDeathOverlayWindow* DeathOverlay = ActiveDeathOverlay.Get())
	{
		DeathOverlay->OnMinimumDisplayElapsed.RemoveDynamic(
			this,
			&UMVDeathRespawnFlow::HandleDeathOverlayMinimumDisplayElapsed);
		ActiveDeathOverlay.Reset();
	}

	PendingDeadActor.Reset();
	bDeathSequenceStarted = false;
	bDeathOverlayCompleted = false;
}

void UMVDeathRespawnFlow::StartDeathRespawnTransitionAfterOverlayDelay()
{
	TryStartDeathRespawnTransition();
}

AMVCharacterBase* UMVDeathRespawnFlow::ResolvePlayerCharacter(UWorld* World) const
{
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	return Cast<AMVCharacterBase>(Pawn);
}

UMVUISubsystem* UMVDeathRespawnFlow::GetUISubsystem() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
}

void UMVDeathRespawnFlow::HandlePlayerDeathPresentationStarted(AActor* DeadActor)
{
	BeginDeathSequence(DeadActor);
}

void UMVDeathRespawnFlow::HandlePlayerDeathOverlayRequested(AActor* /*DeadActor*/)
{
	if (TransitionSubsystem && TransitionSubsystem->IsTransitionRunning())
	{
		return;
	}

	if (!bDeathSequenceStarted)
	{
		BeginDeathSequence(ResolvePlayerCharacter(GetWorld()));
	}

	if (UMVDeathOverlayWindow* DeathOverlay = ActiveDeathOverlay.Get())
	{
		if (DeathOverlay->IsActivated())
		{
			return;
		}
	}

	UMVUISubsystem* UISubsystem = GetUISubsystem();
	ActiveDeathOverlay = UISubsystem
		? Cast<UMVDeathOverlayWindow>(UISubsystem->ShowDeathOverlay())
		: nullptr;
	if (UMVDeathOverlayWindow* DeathOverlay = ActiveDeathOverlay.Get())
	{
		DeathOverlay->OnMinimumDisplayElapsed.RemoveDynamic(
			this,
			&UMVDeathRespawnFlow::HandleDeathOverlayMinimumDisplayElapsed);
		DeathOverlay->OnMinimumDisplayElapsed.AddDynamic(
			this,
			&UMVDeathRespawnFlow::HandleDeathOverlayMinimumDisplayElapsed);
	}
}

void UMVDeathRespawnFlow::HandlePlayerDeathPresentationFinished(AActor* /*DeadActor*/)
{
	if (!bDeathSequenceStarted)
	{
		BeginDeathSequence(ResolvePlayerCharacter(GetWorld()));
	}

	if (!ActiveDeathOverlay.IsValid())
	{
		HandlePlayerDeathOverlayRequested(nullptr);
	}
}

void UMVDeathRespawnFlow::HandleDeathOverlayMinimumDisplayElapsed()
{
	if (bDeathOverlayCompleted)
	{
		return;
	}

	bDeathOverlayCompleted = true;
	if (UMVDeathOverlayWindow* DeathOverlay = ActiveDeathOverlay.Get())
	{
		DeathOverlay->OnMinimumDisplayElapsed.RemoveDynamic(
			this,
			&UMVDeathRespawnFlow::HandleDeathOverlayMinimumDisplayElapsed);
		ActiveDeathOverlay.Reset();
	}

	if (MVDeathRespawnOverlayCompletionToLoadingDelaySeconds <= 0.0f)
	{
		TryStartDeathRespawnTransition();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DeathOverlayCompletionDelayTimerHandle,
			this,
			&UMVDeathRespawnFlow::StartDeathRespawnTransitionAfterOverlayDelay,
			MVDeathRespawnOverlayCompletionToLoadingDelaySeconds,
			false);
		return;
	}

	TryStartDeathRespawnTransition();
}
