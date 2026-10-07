// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "MVWeaponSkillMap.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct FMVWeaponSkillMapData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	FDataTableRowHandle SkillRowHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredPoints = 0;

};

UCLASS()
class MAVERICK_API UMVWeaponSkillMap : public UPrimaryDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FMVWeaponSkillMapData> PassiveSkill;

	UPROPERTY(EditAnywhere)
	TArray<FMVWeaponSkillMapData> QSkill;
	UPROPERTY(EditAnywhere)
	TArray<FMVWeaponSkillMapData> WSkill;
	UPROPERTY(EditAnywhere)
	TArray<FMVWeaponSkillMapData> ESkill;
	UPROPERTY(EditAnywhere)
	TArray<FMVWeaponSkillMapData> RSkill;

};
