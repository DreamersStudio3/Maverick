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

/**
 * Chooser 기반 피격 표현과 Groggy·KnockDown·Airborne 시퀀스를 담당한다.
 * Stat이 Groggy의 진입·해제를 결정하고, 이 컴포넌트는 자신이 시작한 액션의 실행 ID를 관리한다.
 * 자연 회복은 소유 액션을 End로 전환하고, Action의 교체 결과에 따라 소유 기록을 정리하거나 Groggy를 복구한다.
 * BeginPlay에서 입력·사망 이벤트를 연결하고, EndPlay에서 입력·이벤트·타이머를 해제한다.
 * Blueprint는 피격·기상 Row와 몽타주 섹션을 구성한다.
 */
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

	bool IsManagingGroggyAction(int32 ActionInstanceId) const;

	// Input Action Handler
	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;

private:
	EMVHitReactionDir CalculateHitDirection(const FVector& ImpactNormal);
	bool PlayHitReaction(const FDataTableRowHandle& HitReactionRowHandle, bool AdditiveCondition);
	void AdjustActionRotation(const FMHitReactionChooserInput& ChooserInput, const AActor* Attacker, bool AdditiveCondition);
	bool ApplyHitReactionLaunch(const FVector& HitDirection, const FMVHitLaunchData& HitLaunchData);
	
	// Airborne, KnockDown의 HitReaction을 위함
	bool BeginHitSequence(const FMVResolvedHitData& HitData);
	// Groggy의 HitReaction을 위함
	bool BeginGroggySequence(const FDataTableRowHandle& RowHandle);
	bool StartGroggyAction(const FDataTableRowHandle& RowHandle, FName StartSection = NAME_None);
	void ClearHitSequence();
	void ClearGroggySequence();

	// Owner의 MovementMode가 바뀌었을 때의 이벤트에 호출되는 호출되는 함수 -> HitSequenceTimerHandle를 Clear하고, HitSequenceLandSectionName을 Play한다.
	UFUNCTION()
	void OwnerCharacterLandEvent(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);
	void EndHitSequence();
	void GetUpCharacter();
	UFUNCTION()
	void UnbindCharacterMovementEvent(bool bNewState);
	// Groggy에서 회복되는 애니메이션 실행
	UFUNCTION()
	void EndGroggySequence();
	UFUNCTION()
	void HandleGroggyActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);
	void HandleGroggyActionReplacementFinished(int32 PreviousActionId, FName PreviousSection, bool bStarted);
	UFUNCTION()
	void CancelGroggyAction();
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
	int32 GroggyActionInstanceId = INDEX_NONE;
	bool bHitSequenceMovementRestricted = false;
	UPROPERTY(Transient)
	FDataTableRowHandle GroggyRowHandle;
	UPROPERTY(Transient)
	FDataTableRowHandle SelectedGetupRowHandle;

	// Initial Value

	// KnockDown, Airborne의 착지 시퀀스 재생을 위한 SectionName
	FName HitSequenceFallSectionName = TEXT("Fall");
	FName HitSequenceLandSectionName = TEXT("Land");
	// Groggy의 Loop, End 시퀀스 재생을 위한 SectionName
	FName GroggySequenceEndSectionName = TEXT("End");

};
