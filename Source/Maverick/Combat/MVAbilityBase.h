// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Public/Interface/MVAbilityInterface.h"
#include "../Public/Tables/MVSkillDataTableColumn.h"

#include "MVAbilityBase.generated.h"

/**
 * CombatComponent가 액션 테이블 row에서 생성해 몽타주 NotifyState 동안 실행하는 Ability 기본 객체.
 *
 * 책임:
 *   - 스킬 row 데이터와 소유 CombatComponent를 보관한다.
 *   - 액션 실행 중 첫 StartAbility 시점에 Ability 비용을 한 번 소모하고 활성 상태로 전환한다.
 *   - EndAbility 시점에 현재 Notify 구간의 활성 상태를 닫고 체인/쿨다운 타이밍 갱신을 알린다.
 *   - 스탯 회복 pause 구간은 별도 AnimNotifyState가 애니메이션 구간에 맞춰 제어한다.
 *
 * 라이프사이클:
 *   1) CombatComponent가 InitAbility로 row 데이터를 주입한다.
 *   2) CombatComponent가 액션 시작 직전에 PrepareAbilityExecution으로 런타임 상태를 초기화한다.
 *   3) Ability NotifyState Begin에서 StartAbility가 호출되어 비용을 소모하고 Ability를 활성화한다.
 *   4) Ability NotifyState End에서 EndAbility가 호출되어 활성 상태를 정리한다.
 */

class UActorComponent;
class AMVCharacterBase;

UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class MAVERICK_API UMVAbilityBase : public UObject, public IMVAbilityInterface
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable)
	void SetOwner(UActorComponent* Owner);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UActorComponent* GetOwner();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMVCharacterBase* GetOwnerCharacter();

	UFUNCTION(BlueprintCallable)
	void InitAbility(const FMVSkillDataTableColumn& Data);

	void PrepareAbilityExecution();
	
	virtual void StartAbility_Implementation() override;
	virtual void EndAbility_Implementation() override;

	void EnsureAbilityEnded();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UActorComponent> OwnerComponent;
	
	UPROPERTY(BlueprintReadOnly)
	FMVSkillDataTableColumn AbilityData;

	UPROPERTY(Transient)
	bool bAbilityActive = false;

	UPROPERTY(Transient)
	bool bAbilityCostConsumed = false;

private:
	bool TryConsumeAbilityCost();
};
