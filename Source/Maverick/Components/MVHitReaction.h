// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/Tables/MVActionTableTypes.h"
#include "Public/Struct/MVHitTypes.h"
#include "Engine/DataTable.h"
#include "Interface/MVActionInputHandlerInterface.h"

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
class UMVInputManagerComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAVERICK_API UMVHitReaction : public UActorComponent, public IMVActionInputHandlerInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMVHitReaction();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Hit Event Binding
	UFUNCTION(BlueprintCallable)
	void HandleHitEvent(const FMVResolvedHitData& HitData);

	// Input Action Handler
	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;

private:
	EMVHitReactionDir CalculateHitDirection(const FVector& ImpactNormal);
	bool PlayHitReaction(const FDataTableRowHandle& HitReactionRowHandle, bool AdditiveCondition);
	void AdjustActionRotation(const FMHitReactionChooserInput& ChooserInput, const AActor* Attacker, bool AdditiveCondition);
	bool ApplyHitReactionLaunch(const FVector& HitDirection, const FMVHitLaunchData& HitLaunchData);
	
	// Airborne, KnockDown의 HitReaction을 위함
	bool BeginHitSequence(const FMVResolvedHitData& HitData);

	// Owner의 MovementMode가 바뀌었을 때의 이벤트에 호출되는 호출되는 함수 -> HitSequenceTimerHandle를 Clear하고, HitSequenceLandSectionName을 Play한다.
	UFUNCTION()
	void OwnerCharacterLandEvent(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);
	void EndHitSequence();
	void GetUpCharacter();
	UFUNCTION()
	void UnbindCharacterMovementEvent(bool bNewState);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UChooserTable> HitReactionChooser;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMVCharacterBase> OwnerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CanGetupDelay = 1.0f;

protected:
	UPROPERTY()
	TObjectPtr<UMVStatComponent> StatComponent;
	UPROPERTY()
	TObjectPtr<UMVActionComponent> ActionComponent;
	UPROPERTY()
	TObjectPtr<UMVInputManagerComponent> InputManagerComponent;

private:
	FTimerHandle HitSequenceTimerHandle;
	FTimerHandle GetUpTimerHandle;
	bool bCanGetUp = false;
	UPROPERTY(Transient)
	FDataTableRowHandle SelectedGetupRowHandle;

	// Initial Value
	FName HitSequenceFallSectionName = TEXT("Fall");
	FName HitSequenceLandSectionName = TEXT("Land");
	

};
