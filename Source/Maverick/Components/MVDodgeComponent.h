#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/CharacterLocomotionEnums.h"
#include "MVDodgeComponent.generated.h"

class AMVCharacterBase;
struct FMVActionStatRow;
class UCurveFloat;

/**
 * 회피 액션 전용 런타임 컴포넌트.
 *
 * ActionComponent의 공용 실행/버퍼 흐름 위에서 Dodge가 필요로 하는 입력 방향 스냅샷,
 * Dodge 시작 yaw, chooser 입력 플래그, AnimNotify 기반 launch 이동을 관리한다.
 *
 * 책임:
 *   - 입력 버퍼 구간의 Dodge 이동 의도를 저장하고 buffered action 소비 시점에 재사용한다.
 *   - Roll과 대각 Step은 캐릭터 yaw를 보정하되, Strafe Step 판정은 controller yaw 기준으로 고정한다.
 *   - Step chooser에는 cardinal F/L/R/B 문맥을 제공하고, Step/Backstep은 Roll보다 짧은 launch 거리를 쓴다.
 *   - launch 이동은 AnimNotify 설정을 따르며 걷기 바닥에서는 경사면을 따라 보정한다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> CharacterBase 이동 입력 이벤트와 ActionComponent 준비/입력 스냅샷 delegate를 바인딩한다.
 *   2) OnActionPreparing(Dodge) -> 입력 스냅샷, 캐릭터 회전, chooser용 locomotion 상태를 준비한다.
 *   3) MV Dodge Launch NotifyState -> 거리/시간 설정을 캐시하고 준비된 방향으로 이동을 적용한다.
 *   4) Buffered Dodge 시작 시 NotifyState Begin이 누락되면 캐시한 설정으로 launch를 재초기화한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVDodgeComponent();

protected:
	virtual void BeginPlay() override;

public:
	void PrepareDodgeAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Dodge")
	void UpdateBufferedDodgeMovementInput(const FVector& MovementInputDirection);

	bool BeginDodgeLaunchWindow(
		float NotifyDuration,
		UCurveFloat* DistanceCurve,
		float DistanceScale,
		bool bApplyVerticalLaunch,
		int32 MontageInstanceId);
	void TickDodgeLaunchWindow(float DeltaTime, int32 MontageInstanceId);
	void EndDodgeLaunchWindow(bool bClearHorizontalVelocity, int32 MontageInstanceId);

private:
	void CacheMovementInputDirection(const FVector& MovementInputDirection);
	void HandleOwnerMovementInput(const FVector& MovementInputDirection);
	FVector CaptureMovementInputDirection(const AMVCharacterBase& OwnerCharacter) const;
	FVector ResolveBufferedActionMovementInput(int32 ActionId) const;
	bool CanConsumeBufferedAction(
		int32 ActionId,
		const FVector& MovementInputDirection,
		bool bHasMovementInput) const;
	void ApplyDodgeChooserSnapshot(
		AMVCharacterBase& OwnerCharacter,
		bool bHasMovementInput,
		bool bFreeDodge,
		ELocomotionDirection StrafeInputDirection) const;
	void ClearPreparedDodgeLaunch();
	void StopActiveDodgeLaunch(bool bClearHorizontalVelocity, bool bClearPreparedLaunch = true);
	void CacheDodgeLaunchWindowSettings(
		float NotifyDuration,
		UCurveFloat* DistanceCurve,
		float DistanceScale,
		bool bApplyVerticalLaunch);
	bool TryStartBufferedDodgeLaunchFallback();
	FVector ResolveDodgeLaunchDirection(const AMVCharacterBase& OwnerCharacter) const;
	float ResolveDodgeLaunchDistance(const FMVActionStatRow& ActionStat, const AMVCharacterBase& OwnerCharacter) const;
	float ResolveDodgeLaunchDuration(const FMVActionStatRow& ActionStat, float NotifyDuration) const;
	float EvaluateDodgeLaunchDistanceAlpha(float NormalizedTime) const;
	bool IsCurrentDodgeLaunchMontageInstance(int32 MontageInstanceId) const;

	UFUNCTION()
	void HandleActionPreparing(int32 ActionId);

	UFUNCTION()
	void HandleActionStarted(int32 ActionId);

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> ActiveDodgeLaunchCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCurveFloat> ActiveDodgeLaunchDistanceCurve;

	UPROPERTY(Transient)
	TObjectPtr<UCurveFloat> CachedDodgeLaunchDistanceCurve;

	FVector CachedMovementInputDirection = FVector::ZeroVector;
	FVector PreparedDodgeLaunchDirection = FVector::ZeroVector;
	FVector ActiveDodgeLaunchDirection = FVector::ZeroVector;
	float CachedDodgeLaunchNotifyDuration = 0.0f;
	float CachedDodgeLaunchDistanceScale = 1.0f;
	float ActiveDodgeLaunchTargetDistance = 0.0f;
	float ActiveDodgeLaunchDuration = 0.0f;
	float ActiveDodgeLaunchElapsed = 0.0f;
	float ActiveDodgeLaunchLastAlpha = 0.0f;
	int32 ActiveDodgeLaunchMontageInstanceId = INDEX_NONE;
	bool bCachedDodgeLaunchApplyVertical = true;
	bool bHasCachedDodgeLaunchWindowSettings = false;
	bool bHasPreparedDodgeLaunchDirection = false;
	bool bPreparedDodgeHasMovementInput = false;
	bool bPreparedDodgeUsesRollDistance = false;
	bool bPendingBufferedDodgeLaunchFallback = false;
	bool bDodgeLaunchActive = false;
};
