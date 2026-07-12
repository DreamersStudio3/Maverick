// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Public/Interface/MVAbilityInterface.h"
#include "../Public/Tables/MVSkillDataTableColumn.h"
#include "Struct/MVHitTypes.h"

#include "MVAbilityBase.generated.h"

/**
 * CombatComponent가 액션 테이블 row에서 생성해 몽타주 NotifyState 동안 실행하는 Ability 기본 객체.
 *
 * 책임:
 *   - 스킬 row 데이터와 소유 CombatComponent를 보관한다.
 *   - 공격별 HitReaction Launch 값을 Ability 기본값 또는 Blueprint override로 제공한다.
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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Maverick|Ability|Launch")
	FMVHitLaunchData GetHitLaunchData() const;
	virtual FMVHitLaunchData GetHitLaunchData_Implementation() const;

	// Ability에 설정한 Launch 값을 HitRequest에 복사한다. HitReaction row에서 bUseLaunch를 켜면 이 값으로 밀림이 걸린다.
	UFUNCTION(BlueprintCallable, Category = "Maverick|Ability|Hit")
	void ApplyHitLaunchDataToResolveRequest(UPARAM(ref) FMVHitResolveRequest& Request) const;

	void PrepareAbilityExecution();
	
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	virtual void EndAbility_Implementation() override;

	// Ability Utility Function
	UFUNCTION(BlueprintCallable, Category = "Maverick|Ability Utility")
	void ActiveHitStopToCharacters(AMVCharacterBase* Owner, AMVCharacterBase* Target, float Duration = 0.2f, float DilationAmount = 0.2f);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UActorComponent> OwnerComponent;
	
	UPROPERTY(BlueprintReadOnly)
	FMVSkillDataTableColumn AbilityData;

	// Ability별 기본 Launch 값. Distance는 총 목표거리라 500, Duration 3을 넣으면 초당 약 166.7cm씩 밀리고, VerticalSpeed가 있으면 위로도 뜬다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Launch")
	FMVHitLaunchData HitLaunchData;

	UPROPERTY(Transient)
	bool bAbilityActive = false;

	UPROPERTY(Transient)
	bool bAbilityCostConsumed = false;

private:
	bool TryConsumeAbilityCost();
};
