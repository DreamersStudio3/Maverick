// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "MVAnimNotifyState_Ability.generated.h"

/**
 * CombatComponent가 현재 실행 중인 Ability를 몽타주 구간에 맞춰 활성화하는 NotifyState.
 *
 * Begin이 넘긴 알림 실행 번호가 일치할 때만 End를 전달한다. Blueprint Notify도 같은 브리지를 사용한다.
 * AbilityClass가 지정되어 있으면 현재 Ability 인스턴스가 해당 클래스일 때만 실행해 잘못 배치된
 * Notify가 다른 체인 단계 Ability를 건드리지 않게 한다.
 */

class UMVAbilityBase;

UCLASS(meta = (DisplayName = "MV Activate Ability"))
class MAVERICK_API UMVAnimNotifyState_Ability : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Ability|Notify")
	static void BeginAbilityNotify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference, int32 InAbilityIndex,
		TSubclassOf<UMVAbilityBase> InAbilityClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Ability|Notify")
	static void EndAbilityNotify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference, TSubclassOf<UMVAbilityBase> InAbilityClass);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Ability")
	TSubclassOf<UMVAbilityBase> AbilityClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Ability")
	int32 AbilityIndex;
	
};
