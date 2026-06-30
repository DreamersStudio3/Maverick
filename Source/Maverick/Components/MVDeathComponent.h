#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "MVDeathComponent.generated.h"

class ACharacter;
class AMVCharacterBase;
class UMVActionComponent;
class UMVDeathDissolveEffect;
class UMVStatComponent;

UENUM(BlueprintType)
enum class EMVDeathPresentationMode : uint8
{
	DeathAction,
	Ragdoll,
	Immediate
};

UENUM(BlueprintType)
enum class EMVDeathPresentationPhase : uint8
{
	Idle,
	Running,
	Finished
};

UENUM(BlueprintType)
enum class EMVDeathActionPose : uint8
{
	Stand,
	Down,
	LandLight,
	LandHeavy
};

UENUM(BlueprintType)
enum class EMVDeathActionFacing : uint8
{
	Front,
	Back
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDeathPresentationEvent, AActor*, DeadActor);

/**
 * 캐릭터 단위의 사망 표현을 소유하는 컴포넌트.
 *
 * 이 컴포넌트는 "죽었는가"를 판정하지 않고, "죽은 actor를 어떻게 표현할 것인가"만 결정한다.
 * StatComponent가 HP 0을 확정하고 OnDeathStarted를 발행하면, DeathComponent는 전달된
 * FMVDeathContext를 바탕으로 death action, ragdoll, immediate 완료 중 하나를 선택한다.
 *
 * 소유 책임:
 *   - 사망 표현 phase(Idle/Running/Finished)와 actor-local death presentation state 관리.
 *   - death action row 선택. Stand, Down, LandLight, LandHeavy pose와 F/B facing을 death context에서 해석한다.
 *   - lethal KD/AB HitReaction이 먼저 재생 중이면 death context를 보류하고, handoff notify 또는 HR 종료 뒤 Down death로 전환한다.
 *   - 낙하 사망 높이를 기록해 LandLight/LandHeavy death action 선택에 반영한다.
 *   - death dissolve notify를 받아 instanced effect UObject를 실행하고 외부 시스템용 cue 이벤트를 발행한다.
 *   - death overlay notify를 받아 사망 UI 표시 cue 이벤트를 발행한다.
 *   - death action 종료, ragdoll 적용, immediate 완료 뒤 OnDeathPresentationFinished를 발행한다.
 *   - death dissolve effect UObject를 초기화하고 dissolve cue/reset 시 effect를 호출한다.
 *   - 부활 시 ResetDeathPresentationForRespawn으로 actor-local death state, ragdoll, 낙하 기록을 초기화하고 reset 이벤트를 발행한다.
 *
 * 소유하지 않는 책임:
 *   - HP 차감, lethal 판정, dead state 확정은 UMVStatComponent가 담당한다.
 *   - non-lethal HitReaction과 lethal KD/AB의 pre-death HitReaction 선택은 UMVHitReactionComponent가 담당한다.
 *   - montage 재생 자체와 Action 이벤트 브로드캐스트는 UMVActionComponent가 담당한다.
 *   - dissolve material, Dynamic Material Instance, mesh hide/restore 같은 세부 시각 효과 구현은 UMVDeathDissolveEffect가 담당한다.
 *   - 플레이어 사망 UI, 로딩, 월드 리셋, 체크포인트 부활은 UMVRespawnSubsystem이 이 컴포넌트의 이벤트를 구독해 처리한다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> 소유 CharacterBase, StatComponent, ActionComponent를 캐시하고 사망/액션 종료 이벤트를 구독한다.
 *   2) OnDeathStarted -> KD/AB HitReaction이 활성화되어 있으면 보류하고, 그 외에는 사망 표현을 즉시 시작한다.
 *   3) HitReaction death handoff notify 또는 HR 종료 -> 보류한 death context로 death action/ragdoll/immediate 표현을 시작한다.
 *   4) death dissolve/overlay notify 또는 표현 종료 -> dissolve effect, overlay cue와 표현 완료 이벤트를 외부 시스템에 알린다.
 *   5) 부활 시 ResetDeathPresentationForRespawn으로 actor-local 표현 상태를 초기화한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVDeathComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Death")
	void NotifyDeathDissolveStarted();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Death")
	void NotifyDeathOverlayRequested();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Death")
	void NotifyHitReactionDeathHandoff();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Death")
	void SetPendingLandingDeathHeight(float FallHeight);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Death")
	void ClearPendingLandingDeathHeight();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Death")
	void ResetDeathPresentationForRespawn();

	UFUNCTION(BlueprintPure, Category = "Maverick|Death")
	EMVDeathPresentationPhase GetDeathPresentationPhase() const { return DeathPresentationPhase; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Death|Event")
	FMVOnDeathPresentationEvent OnDeathPresentationStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Death|Event")
	FMVOnDeathPresentationEvent OnDeathDissolveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Death|Event")
	FMVOnDeathPresentationEvent OnDeathOverlayRequested;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Death|Event")
	FMVOnDeathPresentationEvent OnDeathPresentationFinished;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Death|Event")
	FMVOnDeathPresentationEvent OnDeathPresentationReset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death")
	EMVDeathPresentationMode DeathPresentationMode = EMVDeathPresentationMode::DeathAction;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Maverick|Death|Dissolve")
	TObjectPtr<UMVDeathDissolveEffect> DeathDissolveEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action")
	FDataTableRowHandle DeathActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action")
	FName DeathActionTableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action", meta = (ClampMin = "1"))
	int32 DefaultDeathActionRowIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action")
	bool bCancelActiveActionBeforeDeath = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action", meta = (ClampMin = "0.0"))
	float DeathActionCancelBlendOutTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action")
	bool bUseDownDeathForKnockDownOrAirborne = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action")
	bool bUseTrackedLandingDeathHeightWhenNoHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action", meta = (ClampMin = "0.0", Units = "cm"))
	float HeavyLandingDeathHeightThreshold = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Action", meta = (ClampMin = "0.0", Units = "s"))
	float RecentLandingDeathHeightValidSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Ragdoll")
	bool bEnableRagdollPhysics = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Death|Ragdoll")
	FName RagdollCollisionProfileName = TEXT("Ragdoll");

private:
	void CacheOwnerReferences();
	void BindMovementModeChanged();
	void UnbindMovementModeChanged();
	void BindStatComponentHandlers();
	void BindActionComponentHandlers();
	void BeginDeathPresentation(const FMVDeathContext& DeathContext);
	bool TryDeferDeathPresentationUntilHitReactionEnds(const FMVDeathContext& DeathContext);
	bool TryBeginDeferredDeathPresentation(FName ActionTableName, FName ActionRowName);
	void ClearDeferredDeathPresentation();
	bool TryStartDeathAction(const FMVDeathContext& DeathContext);
	void StartRagdollDeathPresentation();
	void StartDeathDissolveEffect();
	void ResetDeathDissolveEffect();
	void FinishDeathPresentation();
	bool ResolveDeathActionRowHandle(const FMVDeathContext& DeathContext, FDataTableRowHandle& OutActionRowHandle) const;
	FName ResolveDeathActionTableName() const;
	EMVDeathActionPose ResolveDeathActionPose(const FMVDeathContext& DeathContext) const;
	EMVDeathActionFacing ResolveDeathActionFacing(const FMVDeathContext& DeathContext) const;
	EMVDeathActionPose ResolveLandingDeathActionPose(float FallHeight) const;
	bool TryResolveTrackedLandingDeathHeight(float& OutFallHeight) const;
	float ResolveCurrentFallHeight() const;
	FName MakeDeathActionRowName(
		FGameplayTag CharacterIndexCode,
		EMVDeathActionPose Pose,
		EMVDeathActionFacing Facing,
		int32 Index) const;
	FName MakeLegacyDeathActionRowName(FGameplayTag CharacterIndexCode, EMVDeathActionFacing Facing, int32 Index) const;
	FName MakeDeathActionTableName(FGameplayTag CharacterIndexCode) const;
	static FString CharacterIndexCodeToTableToken(FGameplayTag CharacterIndexCode);
	static FString DeathActionPoseToTableToken(EMVDeathActionPose Pose);
	static FString DeathActionFacingToTableToken(EMVDeathActionFacing Facing);
	static bool IsLandingDeathActionPose(EMVDeathActionPose Pose);

	UFUNCTION()
	void HandleDeathStarted(const FMVDeathContext& DeathContext);

	UFUNCTION()
	void HandleActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);

	UFUNCTION()
	void HandleOwnerMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> CachedActionComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMVStatComponent> CachedStatComponent;

	EMVDeathPresentationPhase DeathPresentationPhase = EMVDeathPresentationPhase::Idle;
	FMVDeathContext DeferredDeathContext;
	FName ActiveDeathActionRowName = NAME_None;
	FName DeferredDeathWaitActionTableName = NAME_None;
	FName DeferredDeathWaitActionRowName = NAME_None;
	FName DefaultMeshCollisionProfileName = NAME_None;
	float PendingLandingDeathHeight = 0.0f;
	float FallingStartHeight = 0.0f;
	float RecentLandingDeathHeight = 0.0f;
	float RecentLandingDeathHeightTime = 0.0f;
	bool bDeathDissolveStarted = false;
	bool bDeathOverlayRequested = false;
	bool bRagdollApplied = false;
	bool bHasDeferredDeathContext = false;
	bool bHasPendingLandingDeathHeight = false;
	bool bHasFallingStartHeight = false;
	bool bHasRecentLandingDeathHeight = false;
	bool bMovementModeDelegateBound = false;
};
