#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVDuelistApproachComponent.generated.h"

class ACharacter;

/** Duelist 전용 전진 이동과 재사용 시각 소유. StateTree가 시작·중단하며 이동 중 목표점은 고정한다. */
UCLASS(ClassGroup=(Maverick), meta=(BlueprintSpawnableComponent))
class MAVERICK_API UMVDuelistApproachComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UMVDuelistApproachComponent();

	UPROPERTY(EditDefaultsOnly, Category="Approach|Visual") TObjectPtr<UMaterialInterface> DashMaterial;
	UPROPERTY(EditAnywhere, Category="Approach|Visual", meta=(HideAlphaChannel)) FLinearColor DashColor = FLinearColor(8.f, 2.f, 0.25f);
	UPROPERTY(EditAnywhere, Category="Approach", meta=(ClampMin="1", Units="cm")) float AttackRange = 300.f;
	UPROPERTY(EditAnywhere, Category="Approach", meta=(ClampMin="1", Units="cm")) float StopRange = 230.f;
	UPROPERTY(EditAnywhere, Category="Approach", meta=(ClampMin="1", Units="cm")) float MaxTravel = 300.f;
	UPROPERTY(EditAnywhere, Category="Approach", meta=(ClampMin="0.01", Units="s")) float Duration = 0.30f;
	UPROPERTY(EditAnywhere, Category="Approach", meta=(ClampMin="0", Units="s")) float Cooldown = 2.f;

	UPROPERTY(EditAnywhere, Category="Approach", meta=(ClampMin="1", Units="deg/s")) float TurnRate = 360.f;

	bool TurnTowards(const AActor* Target, float DeltaSeconds) const;
	bool IsInAttackRange(const AActor* Target) const;
	bool FindDestination(const AActor* Target, FVector& OutDestination) const;
	bool StartApproach(AActor* Target);
	bool IsMoving() const;
	bool CanContinue(const AActor* Target) const;
	void StopApproach();
protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> DashFlash;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInterface>> PreviousMaterials;
	uint16 MotionId = 0;
	double NextReadyTime = 0;
};
