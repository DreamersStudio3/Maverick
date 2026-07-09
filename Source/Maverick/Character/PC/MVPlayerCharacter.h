// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "Interface/MVAttackAbilityDataInterface.h"
#include "Tables/MVMovementActionTableTypes.h"
#include "MVPlayerCharacter.generated.h"

class UMVPlayerDodge;
class UMVPlayerInteractionDetector;
class UMVPlayerConsumable;

/**
 * 로컬 플레이어 캐릭터 런타임 본체.
 *
 * CharacterBase의 공통 이동/스탯/액션/전투 연결 위에 플레이어 입력에 묶인 회피, 회복약, 상호작용 감지,
 * 락온 회전 억제 정책을 얹는다. 플레이어 전용 정책은 컴포넌트로 공개하지 않고 이 클래스가 소유한
 * UObject 서브모듈에 위임해 NPC 재사용 가능 컴포넌트와 구분한다.
 *
 * 책임:
 *   - Dodge, 회복약, InteractionDetector 서브모듈을 생성하고 BeginPlay/Tick/EndPlay 수명주기를 전달한다.
 *   - 전력질주 스태미너 비용과 고갈 후 재개 조건을 플레이어 액션 데이터 기준으로 관리한다.
 *   - 플레이어 피격 리액션 핸들러를 공통 피격 이벤트에 연결한다.
 *   - 락온 대상이 있을 때 질주/회피 구간의 pawn rotation extension tick 억제를 관리한다.
 *   - 공격 Ability가 필요한 락온 대상을 제공한다.
 *
 * 라이프사이클:
 *   1) 생성자 -> 플레이어 전용 서브모듈 기본 서브오브젝트를 생성한다.
 *   2) BeginPlay/EndPlay -> 서브모듈 초기화와 해제를 브리지한다.
 *   3) Tick -> 공통 캐릭터 갱신 뒤 상호작용 감지와 락온 회전 억제 상태를 갱신한다.
 */
UCLASS()
class MAVERICK_API AMVPlayerCharacter : public AMVCharacterBase, public IMVAttackAbilityDataInterface
{
	GENERATED_BODY()

public:
	AMVPlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual AActor* GetTargetActor_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	bool TryUseConsumable();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	bool SelectNextInteractable();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	bool SelectPreviousInteractable();

	void BeginLockOnPawnRotationSuppression();
	void EndLockOnPawnRotationSuppression();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void UpdateRecoverableStats(float DeltaTime) override;
	virtual bool CanUseSprint() const override;
	virtual bool ShouldForceWalkGait() const override;

private:
	virtual void BindDamageHandlers() override;
	void CacheSprintActionData();
	float CalculateSprintStaminaDrain(float DeltaTime) const;
	float ResolveSprintStaminaCostPerSecond() const;
	float ResolveSprintMinRequiredStamina() const;
	float ResolveSprintResumeStaminaRatio() const;
	const FMVSprintActionRow* FindSprintActionRow() const;
	FName ResolveSprintActionTableName() const;
	FName ResolveSprintActionRowName() const;
	void RefreshLockOnPawnRotationExtension();
	bool ShouldSuppressLockOnPawnRotation() const;
	bool ShouldPauseSprintStaminaDrain() const;
	bool IsSprintPivoting() const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "PlayerCharacter")
	TObjectPtr<UMVPlayerDodge> Dodge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "PlayerCharacter")
	TObjectPtr<UMVPlayerInteractionDetector> InteractionDetector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "PlayerCharacter")
	TObjectPtr<UMVPlayerConsumable> PlayerConsumable;

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

private:
	bool bHasSprintActionData = false;
	float SprintActionStaminaCost = 20.0f;
	EMVActionResourceCostType SprintActionStaminaCostType = EMVActionResourceCostType::PerSecond;
	float SprintActionMinRequiredStamina = 0.0f;
	float SprintActionRestartStaminaPercent = 70.0f;
	int32 LockOnPawnRotationSuppressionCount = 0;
};
