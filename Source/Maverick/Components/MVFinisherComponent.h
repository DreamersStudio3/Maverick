// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Public/Enum/MVEquipmentEnums.h"
#include "Interface/MVFinisherTargetDataInterface.h"
#include "Interface/MVActionInputHandlerInterface.h"

#include "MVFinisherComponent.generated.h"

USTRUCT(BlueprintType)
struct FMVFinisherChooserInput
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Character"))
	EMVEquippedStyle WeaponType = EMVEquippedStyle::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Character"))
	FGameplayTagContainer VictimTag;

	bool IsValid() const
	{
		return WeaponType != EMVEquippedStyle::None && VictimTag.IsValid();
	}
	
	void Reset()
	{
		WeaponType = EMVEquippedStyle::None;
		VictimTag.Reset();
	}
};



USTRUCT(BlueprintType)
struct FMVFinisherChooserOutput
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FDataTableRowHandle ActionRow;

	bool IsValid() const
	{
		return ActionRow.DataTable && !ActionRow.RowName.IsNone();
	}

	void Reset()
	{
		ActionRow.DataTable = nullptr;
		ActionRow.RowName = NAME_None;
	}
};

class UMVAbilityBase;

/**
 * 약공격 입력에서 그로기 대상을 찾고 Attacker·Victim 액션의 짝 실행을 조율한다.
 * 홀드 시작에 양측 실행이 성공하면 대상 그로기를 갱신하고 같은 입력을 해제까지 소비한다.
 * Victim으로 넘긴 이동·회전 제한은 해당 액션의 수명을 따른다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAVERICK_API UMVFinisherComponent : public UActorComponent, public IMVFinisherTargetDataInterface, public IMVActionInputHandlerInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMVFinisherComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Handle Input
	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;
	virtual bool TryHandleHoldActionInput(FGameplayTag ActionInputTag, EMVActionInputPhase Phase, float HeldSeconds,
		FVector2D ControllerSpaceInput, bool bHasMovementInput) override;

	// Public API
public:
	UFUNCTION(BlueprintCallable, Category = "Public API")
	bool TryFinisherMove();

	// Interface Implementation
public:
	virtual AActor* GetTargetActor_Implementation() override;

	// Public Properties
public:
	// Attacker
	// Attacker's DataTable Column Struct is FMVSkillDataTableColumn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FSoftObjectPath AttackerChooserTable = TEXT("");

	// Victim
	// Victim's DataTable Column Struct is FMVActionRow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FSoftObjectPath VictimChooserTable = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<UMVAbilityBase> AbilityInstance;

	// Finisher Modifiable value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DetectDistance = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DetectAngle = 45.0f;

private:
	bool CanFinisherMove(AActor*& OutHitActor) const;

	// Check Functions
	bool MakeSphereTrace(AActor*& OutHitActor) const;
	bool JudgeDistanceAndDirection(const AActor* HitActor) const;
	bool CheckThisActorGroggy(const AActor* HitActor) const;
	
	// Find Correct Animations
	bool FindFinisherAnimation(AActor* HitActor, FName FallBackRowName, FDataTableRowHandle& OutAttacker, FDataTableRowHandle& OutVictim);

	// Set MotionWarp Target
	bool SetWarpTarget(const AActor* HitActor);

	// Send Animation Data To Attacker and Victim
	bool SendAnimation(AActor* HitActor, const FDataTableRowHandle& AttackerRowHandle, const FDataTableRowHandle& VictimRowHandle);


	void SetAttackData(const FDataTableRowHandle& AttackDataRowHandle);
	void ClearAttackData();

	void ResetTargetGroggy();

private:
	bool bFinisherConsumedLightPress = false;

	UPROPERTY()
	FDataTableRowHandle AttackData;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;
};

