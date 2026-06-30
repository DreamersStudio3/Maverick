#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "MVRespawnSubsystem.generated.h"

class AMVCharacterBase;
class UMVDeathOverlayWindow;
class UMVLoadingWindow;
class UMVUISubsystem;
class UMVWorldStateSubsystem;

UENUM(BlueprintType)
enum class EMVRespawnPhase : uint8
{
	Idle,
	DeathStarted,
	DeathOverlay,
	LoadingReset,
	Respawning
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnRespawnPhaseChanged, EMVRespawnPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnRespawnProgressChanged, float, Progress, FText, StepText);

/**
 * 플레이어 사망부터 로딩 초기화, 마지막 체크포인트 부활까지 조율하는 GameInstance 서브시스템.
 *
 * 플레이어 DeathComponent 이벤트를 구독하고 DeathOverlay/LoadingWindow 표시를 UI subsystem에 요청한다. 저장 위치와
 * 1회성 스폰/퀘스트 같은 영구 상태는 직접 저장하지 않고 `UMVWorldStateSubsystem`에 조회/요청한다.
 * 필드 액터 리셋 구현이 들어오기 전까지는 progress 이벤트와 단계별 자리만 제공한다.
 *
 * 라이프사이클:
 *   1) GameInstance 생성 시 WorldState 의존성을 초기화하고 월드 초기화 이벤트를 구독한다.
 *   2) 플레이어 DeathComponent의 표현 시작/오버레이 cue/표현 완료 이벤트를 구독해 사망 흐름을 시작한다.
 *   3) DeathComponent 이벤트가 호출하는 공개 API를 통해 UI와 로딩 리셋 단계로 전환한다.
 */
UCLASS()
class MAVERICK_API UMVRespawnSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UMVRespawnSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Respawn")
	bool BeginDeathSequence(AActor* DeadActor);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Respawn")
	void NotifyDeathOverlayRequested();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Respawn", meta = (DeprecatedFunction, DeprecationMessage = "Use NotifyDeathOverlayRequested. Death dissolve no longer displays the overlay."))
	void NotifyDeathDissolveStarted();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Respawn")
	void NotifyDeathMontageEnded();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Respawn")
	void BeginRespawnLoading();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Respawn")
	void CompleteRespawn();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Respawn")
	bool SetLastCheckpointFromActor(AActor* CheckpointActor, FName CheckpointId, FName FieldId, bool bSaveImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Maverick|Respawn")
	EMVRespawnPhase GetRespawnPhase() const { return RespawnPhase; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Respawn")
	bool IsRespawnRunning() const { return RespawnPhase != EMVRespawnPhase::Idle; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Respawn|Event")
	FMVOnRespawnPhaseChanged OnRespawnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Respawn|Event")
	FMVOnRespawnProgressChanged OnRespawnProgressChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Respawn|Loading", meta = (ClampMin = "0.0"))
	float MinimumLoadingSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Respawn|Loading")
	bool bCompleteLoadingAutomatically = true;

private:
	void HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void BindToPlayerDeath(UWorld* World);
	void SetRespawnPhase(EMVRespawnPhase NewPhase);
	void UpdateRespawnProgress(float Progress, FText StepText);
	void StartAutomaticLoadingCompletion();
	void ResetRespawnState();
	int32 ResetWorldActorsForRespawn();
	FName ResolveRespawnResetFieldId() const;
	bool RespawnPlayerAtLastCheckpoint();
	void ResetPlayerStatsForRespawn(AMVCharacterBase& Character) const;
	void ResetUIToDefaultAfterRespawn();
	void RestorePlayerInputAfterRespawn() const;
	AMVCharacterBase* ResolvePlayerCharacter(UWorld* World) const;
	UMVWorldStateSubsystem* GetWorldState() const;
	UMVUISubsystem* GetUISubsystem() const;

	UFUNCTION()
	void HandlePlayerDeathPresentationStarted(AActor* DeadActor);

	UFUNCTION()
	void HandlePlayerDeathOverlayRequested(AActor* DeadActor);

	UFUNCTION()
	void HandlePlayerDeathPresentationFinished(AActor* DeadActor);

	UFUNCTION()
	void HandleDeathOverlayMinimumDisplayElapsed();

	EMVRespawnPhase RespawnPhase = EMVRespawnPhase::Idle;
	TWeakObjectPtr<AActor> PendingDeadActor;
	FTimerHandle AutomaticLoadingTimerHandle;
	bool bDeathMontageEnded = false;
	bool bDeathOverlayMinimumElapsed = false;

	UPROPERTY(Transient)
	TObjectPtr<UMVDeathOverlayWindow> ActiveDeathOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UMVLoadingWindow> ActiveLoadingWindow;
};
