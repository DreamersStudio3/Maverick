#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVDodgeComponent.generated.h"

class AMVCharacterBase;
struct FMVActionStatRow;
class UCurveFloat;

UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVDodgeComponent();

	void PrepareDodgeAction();
	bool BeginDodgeLaunchWindow(
		float NotifyDuration,
		UCurveFloat* DistanceCurve,
		float DistanceScale,
		bool bApplyVerticalLaunch);
	void TickDodgeLaunchWindow(float DeltaTime);
	void EndDodgeLaunchWindow(bool bClearHorizontalVelocity);

private:
	void StopActiveDodgeLaunch(bool bClearHorizontalVelocity);
	FVector ResolveMovementInputDirection(const AMVCharacterBase& OwnerCharacter) const;
	FVector ResolveDodgeLaunchDirection(const AMVCharacterBase& OwnerCharacter) const;
	float ResolveDodgeLaunchDistance(const FMVActionStatRow& ActionStat, const AMVCharacterBase& OwnerCharacter) const;
	float ResolveDodgeLaunchDuration(const FMVActionStatRow& ActionStat, float NotifyDuration) const;
	float EvaluateDodgeLaunchDistanceAlpha(float NormalizedTime) const;

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> ActiveDodgeLaunchCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCurveFloat> ActiveDodgeLaunchDistanceCurve;

	FVector PreparedDodgeLaunchDirection = FVector::ZeroVector;
	FVector ActiveDodgeLaunchDirection = FVector::ZeroVector;
	float ActiveDodgeLaunchTargetDistance = 0.0f;
	float ActiveDodgeLaunchDuration = 0.0f;
	float ActiveDodgeLaunchElapsed = 0.0f;
	float ActiveDodgeLaunchLastAlpha = 0.0f;
	bool bHasPreparedDodgeLaunchDirection = false;
	bool bDodgeLaunchActive = false;
};
