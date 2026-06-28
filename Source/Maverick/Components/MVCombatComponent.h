// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Public/Struct/MVCombatActionTableInput.h"

#include "MVCombatComponent.generated.h"

class UMVAbilityBase;

USTRUCT(BlueprintType)
struct FMVSkillActionStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMVAbilityBase> AbilityInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> DataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;

};


/*
CombatComponent
Fetch Data from DataTable through ChooserTable.

Note:	Getting DataTable through ChooserTable should be implemented in blueprint.
		Getting Row Data comes from Getting DataTable through ChooserTable

		DataTable's Row struct is from "Public/Tables/MVSkillDataTableColumn.h"
		if want to modify Struct, Go to the pulic/Tables/MVSkillDataTableColumn.h


*/


class UChooserTable;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAVERICK_API UMVCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMVCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool TryCombatAction(EMVCombatActionTypes InActionType, FMVSkillDataTableColumn& OutRow, bool FullyStacked = false, int32 SkillIndex = 0);

	bool TryBasicAttack(EMVCombatActionTypes InActionType, FMVSkillDataTableColumn& OutRow);
	bool TrySkill(uint32 SkillIndex, bool FullyStacked);

	// Call when Beginplay or Change Weapon Style
	void RefreshActionMaps();
	
	void ResetBasicAttackMap();
	void ResetSkillMap();
	
	// Set All basic attack indices to 0
	void ResetCurrentIndex();

	

	// Call When Character Change Weapon
	void ChangeWeapon(EMVEquippedStyle NewStyle);
	
protected:
	// Should Return DataTable using ChooserTable In Blueprint
	// Because Using ChooserTable in C++ is So Fucking Shit
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable,Category = "Data")
	UDataTable* GetDataTableFromChooserTable(const FMVCombatActionTableInput& ChooserInput, bool& OutResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Data")
	FMVSkillDataTableColumn GetDataTableRowFromChooserTable(const FMVCombatActionTableInput& ChooserInput, const FName& RowName, bool& OutResult);
	virtual FMVSkillDataTableColumn GetDataTableRowFromChooserTable_Implementation(const FMVCombatActionTableInput& ChooserInput, const FName& RowName, bool& OutResult);


public:
	// Among ComBatActionTypes, selected Enums to use index attack(Basic Attack)
	TArray<EMVCombatActionTypes>WantToMapActionTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<FName, int32>BasicAttackCurrentIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<FName, int32>BasicAttackMaxIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<TSubclassOf<UMVAbilityBase>, FMVSkillActionStruct>SkillMap;

	uint32 SkillMaxIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	EMVEquippedStyle CurrentWeaponStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	float ResetBasicAttackIndexTime = 2.0f;

		
private:
	double LastBasicAttackedTime;

};
