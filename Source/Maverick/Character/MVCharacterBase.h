// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enum/CharacterLocomotionEnums.h"
#include "Struct/CharacterLocomotionStructs.h"

#include "MVCharacterBase.generated.h"

class UMVStatComponent;

UCLASS(Blueprintable, BlueprintType)
class MAVERICK_API AMVCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMVCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION(BlueprintCallable)
	void AttemptCrouch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVStatComponent> StatComponent;


private:
	void UpdateCharacterValue();
	void UpdateRotation();
	void UpdateMovement();

protected:
	UFUNCTION(BlueprintCallable, Category = "LocomotionData")
	void SetStrafeMode(bool StrafeModeOn);
	
private:
	EGait DesiredGait();
	bool CanSprint();
	float CalculateCharacterMovementSpeed(float WalkSpeed, float RunSpeed, float SprintSpeed);
	

public:

	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData")
	float CharacterMoveDirectionAngle;
	
	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData")
	float CharacterMoveDirectionAngleFromAcceleration;
	
	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData")
	uint8 bIsFalling : 1;
	
	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData")
	uint8 bHasMovementInput : 1;
	
	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData")
	ELocomotionDirection LocomotionDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData")
	EGait Gait;

	UPROPERTY(BlueprintReadWrite, Category = "LocomotionData")
	FCharacterInputState CharacterInputState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData")
	TObjectPtr<UCurveFloat> SpeedDirectionCurve;


};
