// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "MVAnimNotifyState_Ability.generated.h"

/**
This Notify State doesn't perform Any logic
Currently, Do not Use this Notify State.
In Blueprint, There's an ANS do the same logic.
Use that.
 */

class UMVAbilityBase;

UCLASS(meta = (DisplayName = "MV Activate Ability"))
class MAVERICK_API UMVAnimNotifyState_Ability : public UAnimNotifyState
{
	GENERATED_BODY()

//public:
//	virtual void NotifyBegin(
//		USkeletalMeshComponent* MeshComp,
//		UAnimSequenceBase* Animation,
//		float TotalDuration,
//		const FAnimNotifyEventReference& EventReference) override;
//
//	virtual void NotifyEnd(
//		USkeletalMeshComponent* MeshComp,
//		UAnimSequenceBase* Animation,
//		const FAnimNotifyEventReference& EventReference) override;
//
//	virtual FString GetNotifyName_Implementation() const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UMVAbilityBase> AbilityClass;
	
};
