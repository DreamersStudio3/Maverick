#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "MVFieldTransitionSubsystem.generated.h"

class AMVCharacterBase;
class UMVActionComponent;
class UMVDeathRespawnFlow;
class UMVLoadingWindow;
class UMVUISubsystem;
class UMVWorldStateSubsystem;

UENUM(BlueprintType)
enum class EMVFieldTransitionReason : uint8
{
	None,
	DeathRespawn,
	CheckpointTravel
};

UENUM(BlueprintType)
enum class EMVFieldTransitionPhase : uint8
{
	Idle,
	PreTransition,
	LoadingReset,
	Relocating,
	Completing
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVFieldTransitionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition")
	EMVFieldTransitionReason Reason = EMVFieldTransitionReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Target")
	bool bUseLastCheckpoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Target")
	bool bHasTargetTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Target")
	FName TargetCheckpointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Target")
	FName TargetFieldId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Target")
	FTransform TargetTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Reset")
	bool bResetFieldActors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Player")
	bool bResetDeathPresentation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Player")
	bool bRestorePlayerStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|UI")
	bool bClearUIBeforeLoading = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|UI")
	bool bResetUIToDefaultOnComplete = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Input")
	bool bRestoreInputOnComplete = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Action")
	FDataTableRowHandle PreTransitionActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Action")
	FName PreTransitionStartSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Action")
	FDataTableRowHandle PostTransitionActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Action")
	FName PostTransitionStartSection = NAME_None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnFieldTransitionPhaseChanged, EMVFieldTransitionPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnFieldTransitionProgressChanged, float, Progress, FText, StepText);

/**
 * 로딩을 동반한 필드 전환의 공통 흐름을 조율하는 GameInstance 서브시스템.
 *
 * 사망 부활과 체크포인트 이동이 공유하는 로딩 전 액션, 로딩창 표시, 필드 actor 리셋, 플레이어 위치 이동,
 * UI/입력 복원, 도착 후 액션을 담당한다. 사망 몽타주, DeathOverlay gate 같은 죽음 전용 정책은
 * `UMVDeathRespawnFlow`가 처리한 뒤 이 서브시스템에 전환 요청을 전달한다.
 */
UCLASS()
class MAVERICK_API UMVFieldTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UMVFieldTransitionSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|FieldTransition")
	bool StartTransition(const FMVFieldTransitionRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Maverick|FieldTransition|Death")
	bool StartDeathRespawnTransition(AActor* DeadActor);

	UFUNCTION(BlueprintCallable, Category = "Maverick|FieldTransition|Checkpoint")
	bool StartCheckpointTravelToTransform(
		FName CheckpointId,
		FName FieldId,
		const FTransform& TargetTransform,
		bool bSaveAsLastCheckpoint = false);

	UFUNCTION(BlueprintCallable, Category = "Maverick|FieldTransition")
	void CompleteTransition();

	UFUNCTION(BlueprintCallable, Category = "Maverick|FieldTransition|Checkpoint")
	bool SetLastCheckpointFromActor(AActor* CheckpointActor, FName CheckpointId, FName FieldId, bool bSaveImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Maverick|FieldTransition")
	EMVFieldTransitionPhase GetTransitionPhase() const { return TransitionPhase; }

	UFUNCTION(BlueprintPure, Category = "Maverick|FieldTransition")
	bool IsTransitionRunning() const { return TransitionPhase != EMVFieldTransitionPhase::Idle; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|FieldTransition|Event")
	FMVOnFieldTransitionPhaseChanged OnFieldTransitionPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|FieldTransition|Event")
	FMVOnFieldTransitionProgressChanged OnFieldTransitionProgressChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Loading", meta = (ClampMin = "0.0"))
	float MinimumLoadingSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|FieldTransition|Loading")
	bool bCompleteLoadingAutomatically = true;

private:
	UFUNCTION()
	void HandlePreTransitionActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);

	void SetTransitionPhase(EMVFieldTransitionPhase NewPhase);
	void BeginLoadingReset();
	void ClearUIBeforeLoadingIfNeeded();
	bool TryStartPreTransitionAction();
	void UnbindPreTransitionAction();
	void UpdateTransitionProgress(float Progress, FText StepText);
	void StartAutomaticLoadingCompletion();
	void ResetTransitionState();
	int32 ResetWorldActorsForTransition(const FMVFieldTransitionRequest& Request);
	FName ResolveTransitionResetFieldId(const FMVFieldTransitionRequest& Request) const;
	bool ApplyTransitionDestination(const FMVFieldTransitionRequest& Request);
	void ResetPlayerStatsForTransition(AMVCharacterBase& Character) const;
	void ResetUIToDefaultAfterTransition();
	void RestorePlayerInputAfterTransition() const;
	void TryPreparePostTransitionAction(AMVCharacterBase& Character, const FMVFieldTransitionRequest& Request);
	void ResumePreparedPostTransitionAction();
	void FinishTransitionAfterScreenChange();
	AMVCharacterBase* ResolvePlayerCharacter(UWorld* World) const;
	UMVWorldStateSubsystem* GetWorldState() const;
	UMVUISubsystem* GetUISubsystem() const;

	EMVFieldTransitionPhase TransitionPhase = EMVFieldTransitionPhase::Idle;
	FMVFieldTransitionRequest ActiveTransitionRequest;
	FTimerHandle AutomaticLoadingTimerHandle;
	FTimerHandle ScreenTransitionCompletionTimerHandle;
	bool bClearedUIBeforeLoading = false;

	UPROPERTY(Transient)
	TObjectPtr<UMVLoadingWindow> ActiveLoadingWindow;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> PreTransitionActionComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> PostTransitionActionComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMVDeathRespawnFlow> DeathRespawnFlow;
};
