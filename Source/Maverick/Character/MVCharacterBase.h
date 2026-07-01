// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enum/CharacterLocomotionEnums.h"
#include "Enum/MVEquipmentEnums.h"
#include "GameplayTagContainer.h"
#include "Struct/CharacterLocomotionStructs.h"
#include "Struct/MVHitTypes.h"
#include "Tables/MVCharacterTableTypes.h"
#include "Tables/MVMovementActionTableTypes.h"

#include "MVCharacterBase.generated.h"

class UMVStatComponent;
class UMVActionComponent;
class UMVDodgeComponent;
class UMVDeathComponent;
class UMVHitReactionComponent;
class UMVInputManagerComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FMVOnMovementInputReceived, const FVector&);

/**
 * 공통 캐릭터 런타임 본체.
 *
 * 플레이어와 NPC가 공유할 수 있는 캐릭터 데이터 태그, 이동 상태, 공용 컴포넌트 연결, 무적 상태,
 * 피격 이벤트 브리지, 질주 스태미너 소비를 관리한다. 회피, 피격 리액션, 사망 표현, 액션 버퍼 같은 도메인 세부 정책은
 * 전용 컴포넌트가 이 클래스의 공통 상태 변수와 이벤트 함수를 호출해 컴포넌트 안에서 개별적으로 처리한다.
 *
 * 책임:
 *   - CharacterIndexCode를 액션/스탯 컴포넌트에 주입하고 CharacterMovement 기준 locomotion 값과 gait,
 *     장비 스타일, 질주 스태미너 상태를 갱신한다.
 *   - HitResolver가 전달한 결과의 CharacterIndexCode를 확인한 뒤 OnDamaged를 브로드캐스트한다.
 *   - ACharacter의 BeginPlay, Tick, AddMovementInput 진입점에서 초기 데이터 로드,
 *     매 프레임 locomotion/질주 스태미너 갱신, 이동 입력 캐싱과 OnMovementInputReceived 브로드캐스트를 담당한다.
 *   - 질주 중이 아닐 때 StatComponent의 회복 Tick을 호출하되, 회복 쿨다운과 일시정지 정책은 StatComponent가 소유한다.
 *   - 이동 입력은 항상 controller yaw 기준 raw 2D로 계산해 게임을 플레이하는 유저의 의도와 일치시킨다.
 *   - ActionComponent의 이동 입력 잠금은 CharacterMovement의 MaxAcceleration에 반영한다.
 *   - InputManagerComponent가 액션 입력 이벤트에 최근 이동 입력 문맥을 함께 제공할 수 있도록 이동 입력 이벤트를 제공한다.
 *   - 무적 상태는 중첩 카운터로 관리해 여러 무적 구간이 겹쳐도 모든 구간이 끝난 뒤 해제되게 한다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> 공용 컴포넌트 이벤트와 locomotion 보조 데이터를 초기화한다.
 *   2) AddMovementInput -> controller yaw 기준 raw 2D 입력을 누적하고 이동 입력 이벤트를 브로드캐스트한다.
 *   3) Tick -> 이동/회전/질주 스태미너를 갱신하고 애니메이션 조회용 상태를 정리한다.
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
	virtual void PostInitializeComponents() override;
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

	UFUNCTION(BlueprintCallable, Category = "Maverick|Character|Data")
	void SetCharacterIndexCode(FGameplayTag NewCharacterIndexCode);

	UFUNCTION(BlueprintPure, Category = "Maverick|Character|Data")
	FGameplayTag GetCharacterIndexCode() const;

	UFUNCTION(BlueprintPure, Category = "LocomotionData|Action")
	bool HasDodgeMovementInput() const;

	bool TryGetControllerSpaceMovementInput(FVector2D& OutMovementInput, int32 MaxFrameAge = 0) const;
	FRotator ResolveMovementInputReferenceRotation() const;
	FVector ResolveWorldDirectionFromControllerSpaceInput(const FVector2D& ControllerSpaceInput) const;

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

	UFUNCTION(BlueprintCallable, Category = "Maverick|Character|Damage")
	bool OnHitResolved(const FMVResolvedHitData& HitData);

	FMVOnMovementInputReceived OnMovementInputReceived;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Character|Event")
	FMVOnDamagedSignature OnDamaged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVActionComponent> ActionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVDodgeComponent> DodgeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVDeathComponent> DeathComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVHitReactionComponent> HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVInputManagerComponent> InputManagerComponent;

private:
	void ApplyCharacterIndexCodeToComponents();
	void UpdateCharacterValue();
	void UpdateRotation();
	void UpdateMovement(float DeltaTime);
	void UpdateLocomotionDirection();
	void CacheControllerSpaceMovementInput(const FVector& WorldDirection, float ScaleValue);
	FVector2D ResolveControllerSpaceMovementInput(const FVector& WorldDirection, float ScaleValue) const;
	void UpdateRecoverableStats(float DeltaTime);
	void CacheSprintActionData();

protected:
	UFUNCTION(BlueprintCallable, Category = "LocomotionData")
	void SetStrafeMode(bool StrafeModeOn);
	virtual void BindDamageHandlers();
	
private:
	EGait DesiredGait();
	bool CanSprint();
	bool CanUseSprintStamina() const;
	float CalculateSprintStaminaDrain(float DeltaTime) const;
	float ResolveSprintStaminaCostPerSecond() const;
	float ResolveSprintMinRequiredStamina() const;
	float ResolveSprintResumeStaminaRatio() const;
	const FMVSprintActionRow* FindSprintActionRow() const;
	FName ResolveSprintActionTableName() const;
	FName ResolveSprintActionRowName() const;
	float CalculateCharacterMovementSpeed(float WalkSpeed, float RunSpeed, float SprintSpeed);
	bool bHasSprintActionData = false;
	float SprintActionStaminaCost = 20.0f;
	EMVActionResourceCostType SprintActionStaminaCostType = EMVActionResourceCostType::PerSecond;
	float SprintActionMinRequiredStamina = 0.0f;
	float SprintActionRestartStaminaPercent = 70.0f;
	int32 InvincibilityCount = 0;
	FVector2D ControllerSpaceMovementInput = FVector2D::ZeroVector;
	uint64 ControllerSpaceMovementInputFrame = 0;
	

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Character|Data", meta = (Categories = "Character"))
	FGameplayTag CharacterIndexCode;

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
	ELocomotionDirection LocomotionDirectionFromAcceleration;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Sprint|Table")
	FDataTableRowHandle SprintActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Sprint|Table")
	FName SprintActionTableName = TEXT("Sprint");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Sprint|Table")
	FName SprintActionRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData|Sprint|Table", meta = (ClampMin = "1"))
	int32 DefaultSprintRowIndex = 1;

	UPROPERTY(BlueprintReadWrite, Category = "LocomotionData")
	FCharacterInputState CharacterInputState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LocomotionData")
	TObjectPtr<UCurveFloat> SpeedDirectionCurve;
};
