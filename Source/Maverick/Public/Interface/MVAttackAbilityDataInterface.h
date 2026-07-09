#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MVAttackAbilityDataInterface.generated.h"

class UMeshComponent;
class AActor;

USTRUCT(BlueprintType)
struct MAVERICK_API FMVMeleeWeaponData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Attack Ability Data")
	TObjectPtr<UMeshComponent> WeaponMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Attack Ability Data")
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Attack Ability Data")
	FVector EndLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Attack Ability Data")
	bool bSecondaryWeapon = false;
};

UINTERFACE(BlueprintType)
class MAVERICK_API UMVAttackAbilityDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 공격 Ability가 필요한 무기/타겟 데이터를 구현체에서 얻기 위한 공용 인터페이스.
 *
 * Blueprint `BPI_AttackAbilityData`를 대체하는 C++ 계약이다. 구현체는 공격 Ability가 넘긴
 * 소켓 이름을 기준으로 현재 공격에 사용할 MeleeWeaponData 배열을 반환하고, 필요하면 공격 타겟도 제공한다.
 */
class MAVERICK_API IMVAttackAbilityDataInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Attack Ability Data")
	bool GetMeleeWeaponData(
		FName StartSocketName,
		FName EndSocketName,
		UPARAM(ref) TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Attack Ability Data")
	AActor* GetTargetActor() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Attack Ability Data")
	bool GetMeleeDualWeaponData(
		FName StartSocketName,
		FName EndSocketName,
		UPARAM(ref) TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const;
};
