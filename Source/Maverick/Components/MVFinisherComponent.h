// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "MVFinisherComponent.generated.h"

USTRUCT(BlueprintType)
struct FMVFinisherChooserInput
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Character"))
	FGameplayTagContainer AttackerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Character"))
	FGameplayTagContainer VictimTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Item.Weapon"))
	FGameplayTagContainer WeaponTag;

	bool IsValid() const
	{
		return AttackerTag.IsValid() && VictimTag.IsValid() && WeaponTag.IsValid();
	}
	
	void Reset()
	{
		AttackerTag.Reset();
		VictimTag.Reset();
		WeaponTag.Reset();
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



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAVERICK_API UMVFinisherComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMVFinisherComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Public API
public:
	UFUNCTION(BlueprintCallable, Category = "Public API")
	bool TryFinisherMove();

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

};


/*
	Todo:	Should Store Attack Data 
			To Active Ability

*/