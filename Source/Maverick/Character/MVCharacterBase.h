// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enum/CharacterLocomotionEnums.h"
#include "Enum/MVEquipmentEnums.h"
#include "Struct/CharacterLocomotionStructs.h"
#include "Tables/MVActionTableTypes.h"

#include "MVCharacterBase.generated.h"

class UMVStatComponent;
class UMVActionComponent;
class UMVDodgeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnStatRecentLossHoldChanged, bool, bHold);
DECLARE_MULTICAST_DELEGATE_OneParam(FMVOnMovementInputReceived, const FVector&);

/**
 * 공통 캐릭터 런타임 본체.
 *
 * 플레이어와 NPC가 공유할 수 있는 이동 상태, 공용 컴포넌트 연결, 이동/무적 잠금 반영,
 * 회복 가능한 스탯 갱신을 관리한다. 회피, 전투, 액션 버퍼 같은 도메인 정책은
 * 전용 컴포넌트가 이 클래스의 공통 상태와 이벤트를 사용해 처리한다.
 *
 * 책임:
 *   - CharacterMovement 기준 locomotion 값과 gait, 장비 스타일, 스태미너 회복 상태를 갱신한다.
 *   - Action/Stat/Dodge 컴포넌트를 소유하고 엔진 라이프사이클과 공용 이벤트를 연결한다.
 *   - ActionComponent의 이동 입력 잠금과 캐릭터 무적 중첩 상태를 CharacterMovement에 반영한다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> 공용 컴포넌트 이벤트와 locomotion 보조 데이터를 초기화한다.
 *   2) AddMovementInput -> 원본 이동 입력을 브로드캐스트한 뒤 CharacterMovement에 전달한다.
 *   3) Tick -> 이동/회전/회복 스탯을 갱신하고 애니메이션 조회용 상태를 정리한다.
 */
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
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

public:
	UFUNCTION(BlueprintCallable)
	void AttemptCrouch();

	UFUNCTION(BlueprintPure, Category = "LocomotionData|Action")
	bool HasDodgeMovementInput() const;

	void ApplyLocomotionDirectionSnapshot(const FVector& MovementDirection);

	UFUNCTION(BlueprintCallable, Category = "LocomotionData|Equipment")
	void SetEquippedStyle(EMVEquippedStyle NewEquippedStyle);

	UFUNCTION(BlueprintPure, Category = "LocomotionData|Equipment")
	EMVEquippedStyle GetEquippedStyle() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Character|State")
	void BeginMovementInputBlock();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Character|State")
	void EndMovementInputBlock();

	UFUNCTION(BlueprintPure, Category = "Maverick|Character|State")
	bool IsMovementInputBlocked() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Character|State")
	void BeginInvincibility();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Character|State")
	void EndInvincibility();

	UFUNCTION(BlueprintPure, Category = "Maverick|Character|State")
	bool IsInvincible() const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnStatRecentLossHoldChanged OnStatRecentLossHoldChanged;

	FMVOnMovementInputReceived OnMovementInputReceived;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVActionComponent> ActionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVDodgeComponent> DodgeComponent;

private:
	void UpdateCharacterValue();
	void UpdateRotation();
	void UpdateMovement(float DeltaTime);
	void UpdateLocomotionDirection();
	void UpdateRecoverableStats(float DeltaTime);
	void CacheSprintActionData();
	void RestartRecoverableStatCooldown();

	UFUNCTION()
	void HandleActionStatRecoveryPauseChanged(bool bPaused);

	UFUNCTION()
	void HandleActionCostConsumed(int32 ActionId);

protected:
	UFUNCTION(BlueprintCallable, Category = "LocomotionData")
	void SetStrafeMode(bool StrafeModeOn);
	
private:
	EGait DesiredGait();
	bool CanSprint();
	bool CanUseSprintStamina() const;
	float CalculateSprintStaminaDrain(float DeltaTime) const;
	float ResolveSprintStaminaCostPerSecond() const;
	float ResolveSprintMinRequiredStamina() const;
	float ResolveSprintResumeStaminaRatio() const;
	float CalculateCharacterMovementSpeed(float WalkSpeed, float RunSpeed, float SprintSpeed);
	float RecoverableStatCooldownRemaining = 0.0f;
	bool bHasSprintActionData = false;
	float SprintActionStaminaCost = 20.0f;
	EMVActionResourceCostType SprintActionStaminaCostType = EMVActionResourceCostType::PerSecond;
	float SprintActionMinRequiredStamina = 0.0f;
	float SprintActionRestartStaminaPercent = 70.0f;
	int32 InvincibilityCount = 0;
	

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Equipment")
	EMVEquippedStyle EquippedStyle = EMVEquippedStyle::BareHand;

	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData|Action")
	uint8 bHasDodgeMovementInput : 1;

	UPROPERTY(BlueprintReadOnly, Category = "LocomotionData|Stamina")
	uint8 bIsSprintBlockedByStamina : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Stamina", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SprintResumeStaminaRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Stamina")
	bool bUseStaminaRecoveryDelay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Action", meta = (DisplayName = "Sprint Action"))
	EMVActionId SprintActionId = EMVActionId::Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Stamina|Fallback", meta = (ClampMin = "0.0"))
	float FallbackSprintStaminaCost = 20.0f;

	UPROPERTY(BlueprintReadWrite, Category = "LocomotionData")
	FCharacterInputState CharacterInputState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData")
	TObjectPtr<UCurveFloat> SpeedDirectionCurve;
};
