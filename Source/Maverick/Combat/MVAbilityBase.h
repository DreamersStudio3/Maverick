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
 *   - StartAbility 시점에 Ability 비용을 스탯 컴포넌트에서 소모하고 활성 상태로 전환한다.
 *   - EndAbility 시점에 활성 상태를 해제해 같은 Ability 인스턴스가 다음 체인/재사용 때 다시 시작될 수 있게 한다.
 *
 * 라이프사이클:
 *   1) CombatComponent가 InitAbility로 row 데이터를 주입한다.
 *   2) Ability NotifyState Begin에서 StartAbility가 호출되어 비용을 소모하고 Ability를 활성화한다.
 *   3) Ability NotifyState End에서 EndAbility가 호출되어 활성 상태를 정리한다.
 */

class UMVCombatComponent;
class AMVCharacterBase;

UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class MAVERICK_API UMVAbilityBase : public UObject, public IMVAbilityInterface
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable)
	void SetOwner(UMVCombatComponent* Owner);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UMVCombatComponent* GetOwner();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMVCharacterBase* GetOwnerCharacter();

	UFUNCTION(BlueprintCallable)
	void InitAbility(const FMVSkillDataTableColumn& Data);
	
	virtual void StartAbility_Implementation() override;
	virtual void EndAbility_Implementation() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UMVCombatComponent> OwnerComponent;
	
	UPROPERTY(BlueprintReadOnly)
	FMVSkillDataTableColumn AbilityData;

	UPROPERTY(Transient)
	bool bAbilityActive = false;

private:
	bool TryConsumeAbilityCost();

	
};
