#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "GameFramework/Actor.h"
#include "MVBossIntroTrigger.generated.h"

class AMVEnemy;
class AMVAIController;
class APawn;
class APlayerController;
class UBoxComponent;
class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class UMVInputManagerComponent;
class UMVWorldStateSubsystem;

UENUM(BlueprintType)
enum class EMVBossIntroState : uint8 { Waiting, Playing, Completed };
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVBossIntroStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVBossIntroEndedSignature, bool, bCompleted);

/**
 * 플레이어의 박스 진입·최초 보스 등장 연출·완료 위치 저장 소유
 * BeginPlay에서 저장 위치 복원·AI 대기, 완료 후 입력·카메라 복원과 전투 시작
 * 중단·EndPlay는 관람 기록 제외, 생성 시퀀스·이벤트·입력 차단 정리
 * IntroId는 맵 내 고유 값, Level Sequence의 Boss 태그는 실제 보스 액터에 연결
 * 저장은 WorldState, 공격 정책은 기존 AI·전투 도메인 책임; 단일 로컬 플레이어 기준
 */
UCLASS()
class MAVERICK_API AMVBossIntroTrigger : public AActor
{
	GENERATED_BODY()
public:
	AMVBossIntroTrigger();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Intro") TObjectPtr<UBoxComponent> TriggerBox;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Boss Intro") TObjectPtr<AMVEnemy> Boss;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Intro") TObjectPtr<ULevelSequence> IntroSequence;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Intro") FName IntroId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Intro") FName BossBindingTag = TEXT("Boss");
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss Intro") EMVBossIntroState State = EMVBossIntroState::Waiting;
	UPROPERTY(BlueprintAssignable, Category = "Boss Intro") FMVBossIntroStartedSignature OnIntroStarted;
	UPROPERTY(BlueprintAssignable, Category = "Boss Intro") FMVBossIntroEndedSignature OnIntroEnded;
	UFUNCTION(BlueprintCallable, Category = "Boss Intro") bool TryStartIntro(APawn* PlayerPawn);
	UFUNCTION(BlueprintCallable, Category = "Boss Intro") void CancelIntro();
	UFUNCTION(BlueprintPure, Category = "Boss Intro") bool IsIntroPlaying() const { return State == EMVBossIntroState::Playing; }
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaSeconds) override;
private:
	void InitializeEncounter();
	void RestoreCompletedPlacement();
	void AcquirePlayerControl(APawn* PlayerPawn, APlayerController* Controller);
	void RestorePlayerControl();
	void FinishIntro(bool bCompleted);
	void ActivateCombat(APawn* PlayerPawn);
	UFUNCTION() void HandleOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void HandleFinished();
	UFUNCTION() void HandleStopped();
	UFUNCTION() void HandleWorldLoaded(FString SlotName);
	UPROPERTY(Transient) TObjectPtr<ULevelSequencePlayer> SequencePlayer;
	UPROPERTY(Transient) TObjectPtr<ALevelSequenceActor> SequenceActor;
	TWeakObjectPtr<AMVAIController> BossController;
	TWeakObjectPtr<UMVWorldStateSubsystem> WorldState;
	TWeakObjectPtr<APlayerController> LocalPlayer;
	TWeakObjectPtr<APawn> ControlledPawn;
	TWeakObjectPtr<AActor> PreviousViewTarget;
	TWeakObjectPtr<UMVInputManagerComponent> BlockedInput;
	FTransform InitialBossTransform;
	FTransform PlaybackStartTransform;
	FTimerHandle InitializeTimer;
	EMouseCaptureMode SavedCaptureMode = EMouseCaptureMode::CapturePermanently;
	EMouseLockMode SavedLockMode = EMouseLockMode::LockOnCapture;
	uint8 SavedMovementMode = 0;
	uint8 SavedCustomMovementMode = 0;
	bool bSavedCollision = true;
	bool bSavedPawnInput = false;
	bool bSavedCursor = false;
	bool bPlayerControlAcquired = false;
	bool bInitialized = false;
	bool bEncounterActivated = false;
	bool bEndingPlay = false;
};
