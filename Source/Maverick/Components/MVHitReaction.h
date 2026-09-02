// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/Tables/MVActionTableTypes.h"
#include "Public/Struct/MVHitTypes.h"
#include "Engine/DataTable.h"

#include "MVHitReaction.generated.h"



USTRUCT(BlueprintType)
struct FMHitReactionChooserInput
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMVHitReactionDir HitReactionDir = EMVHitReactionDir::Front;

	// PoiseBreak가 아닐 시 Additive만
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPoiseBreak = false;
	
	// 공중에서 맞을 시 Additive만
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAir = false;

	// 그로기 상태에서 맞을 시 Additive만
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGroggy = false;
};


class UChooserTable;
class UMVStatComponent;
class UMVActionComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAVERICK_API UMVHitReaction : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMVHitReaction();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Hit Event Binding
	UFUNCTION(BlueprintCallable)
	void HandleHitEvent(const FMVResolvedHitData& HitData);

private:
	EMVHitReactionDir CalculateHitDirection(const FVector& ImpactNormal);
	bool PlayHitReaction(const FDataTableRowHandle& HitReactionRowHandle, const FMHitReactionChooserInput& ChooserInput);
	void AdjustActionRotation(const FMHitReactionChooserInput& ChooserInput, const AActor* Attacker);
	bool ApplyHitReactionLaunch(const FVector& HitDirection, const FMVHitLaunchData& HitLaunchData);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UChooserTable> HitReactionChooser;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACharacter> OwnerCharacter;

protected:
	UPROPERTY()
	TObjectPtr<UMVStatComponent> StatComponent;
	UPROPERTY()
	TObjectPtr<UMVActionComponent> ActionComponent;

};
