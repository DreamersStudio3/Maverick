// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Enum/CharacterLocomotionEnums.h"
#include "Struct/CharacterLocomotionStructs.h"

#include "MVAnimInstanceBase.generated.h"

/**
 * 
 */

class AMVCharacterBase;
class UCharacterMovementComponent;

UCLASS()
class MAVERICK_API UMVAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	void ReceiveCharacterData();
	void GetLocationData(float DeltaTime);
	void GetVelocityData();
	void GetRotationData(float DeltaTime);
	void GetAccelerationData(float DeltaTime);
	void GetCharacterStateData();
	void CalculatePivotState();

	// Character Data
public:
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Character")
	TObjectPtr<AMVCharacterBase> Character;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Character")
	TObjectPtr<UCharacterMovementComponent> CharcterMovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Character")
	EGait IncomingGait;
	UPROPERTY(BlueprintReadWrite, Category = "AnimBPData_Character")
	FCharacterInputState CharacterInputState;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Character")
	uint8 bIsFalling : 1;

	// Location Data
public:
	FVector PreviousWorldLocation;
	FVector CurrentWorldLocation;

	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Location")
	float LocationDelta;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Location")
	float DisplacementSpeed;

	// Velocity Data
public:
	FVector PreviousVelocity;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	FVector Velocity;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	float FallSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	uint8 bHasVelocity2D : 1;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	float GroundSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	float MovingDirection;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	float MovingDirectionFromAcceleration;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	ELocomotionDirection LocomotionDirection;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Velocity")
	ELocomotionDirection LocomotionDirectionFromAcceleration;

	// Rotation Data
public:
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Rotation")
	FRotator ActorRotation;
	float PreActorYaw;
	float ActorYaw;
	float ActorYawDelta;
	
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Rotation")
	float LeanAngle;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Rotation")
	float CharacterLookDirectionPitch;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Rotation")
	float CharacterLookDirectionYaw;

	// Acceleration Data
public:
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Acceleration")
	FVector CurrentAcceleration;

	FVector PreviousAcceleration2D;
	FVector CurrentAcceleration2D;

	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Acceleration")
	uint8 IsAccelerating : 1;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Acceleration")
	FVector RelativeAcceleration;

	// Character State Data
public:
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_CharacterState")
	EGait CurrentGait;
	EGait PreviousGait;
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_CharacterState")
	uint8 bGaitChanged : 1;

	// Pivot Data
public:
	UPROPERTY(BlueprintReadOnly, Category = "AnimBPData_Pivot")
	uint8 IsPivot : 1;

};
