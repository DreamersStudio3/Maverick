#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "MVDeathRespawnFlow.generated.h"

class AMVCharacterBase;
class UMVDeathComponent;
class UMVDeathOverlayWindow;
class UMVFieldTransitionSubsystem;
class UMVUISubsystem;

/**
 * 플레이어 사망 표현 이벤트를 필드 전환 요청으로 바꾸는 death-only flow 객체.
 *
 * DeathComponent 이벤트를 구독하고 DeathOverlay 표시 완료 이후 death respawn 전환을 요청한다.
 * 사망 몽타주 종료는 overlay가 누락된 경우 overlay를 보장하는 fallback으로만 사용한다.
 */
UCLASS(Transient)
class MAVERICK_API UMVDeathRespawnFlow : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UMVFieldTransitionSubsystem* InTransitionSubsystem);
	void Shutdown();

	virtual UWorld* GetWorld() const override;

private:
	void HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void BindToPlayerDeath(UWorld* World);
	void UnbindFromPlayerDeath();
	bool BeginDeathSequence(AActor* DeadActor);
	void TryStartDeathRespawnTransition();
	void ResetDeathRespawnState();
	void StartDeathRespawnTransitionAfterOverlayDelay();
	AMVCharacterBase* ResolvePlayerCharacter(UWorld* World) const;
	UMVUISubsystem* GetUISubsystem() const;

	UFUNCTION()
	void HandlePlayerDeathPresentationStarted(AActor* DeadActor);

	UFUNCTION()
	void HandlePlayerDeathOverlayRequested(AActor* DeadActor);

	UFUNCTION()
	void HandlePlayerDeathPresentationFinished(AActor* DeadActor);

	UFUNCTION()
	void HandleDeathOverlayMinimumDisplayElapsed();

	UPROPERTY(Transient)
	TObjectPtr<UMVFieldTransitionSubsystem> TransitionSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UMVDeathComponent> BoundDeathComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVDeathOverlayWindow> ActiveDeathOverlay;

	TWeakObjectPtr<AActor> PendingDeadActor;
	FTimerHandle DeathOverlayCompletionDelayTimerHandle;
	bool bDeathSequenceStarted = false;
	bool bDeathOverlayCompleted = false;
};
