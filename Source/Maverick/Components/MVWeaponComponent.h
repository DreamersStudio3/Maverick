#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Interface/MVAttackAbilityDataInterface.h"
#include "Struct/MVWeaponTypes.h"
#include "Tables/MVWeaponTableTypes.h"
#include "MVWeaponComponent.generated.h"

class UMeshComponent;
class UObject;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnEquippedWeaponChanged, const FMVEquippedWeaponState&, WeaponState);

/**
 * 캐릭터의 현재 장착 무기 상태를 관리하는 컴포넌트.
 *
 * 무기 DataTable row를 런타임 state로 변환하고, 장착 변경 시 CharacterBase의 EquippedStyle,
 * CombatComponent의 액션 맵 갱신, 선택적 무기 메시 갱신을 연결한다.
 * 기본 무기 row를 장착하고, 설정이 비어 있거나 해석에 실패하면 내장 TestSword row를 fallback으로 사용한다.
 * 공격 trace 소켓은 장착 시각화와 분리해 Ability가 trace 데이터를 요청하는 시점에 확인한다.
 *
 * 라이프사이클:
 *   1) 생성자 -> DefaultWeaponRow를 플레이어 TestSword row로 초기화한다.
 *   2) BeginPlay -> 현재 state가 없으면 DefaultWeaponRow 또는 내장 TestSword fallback을 장착한다.
 *   3) EquipWeaponFromRow/EquipDefaultWeapon -> state 갱신, 캐릭터 장비 스타일 갱신, 이벤트 브로드캐스트.
 *   4) HitResolver가 타격 확정 시점에 CaptureWeaponHitSnapshot으로 히트 전용 사본을 만든다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVWeaponComponent : public UActorComponent, public IMVAttackAbilityDataInterface
{
	GENERATED_BODY()

public:
	UMVWeaponComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon")
	bool EquipWeaponFromRow(const FMVWeaponTableRow& WeaponRow);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Weapon")
	bool EquipDefaultWeapon();

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	FMVEquippedWeaponState GetEquippedWeaponState() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	FMVWeaponHitSnapshot CaptureWeaponHitSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	float GetEquippedWeaponAttackPower() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	EMVEquippedStyle GetEquippedStyle() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Weapon")
	EMVWeaponRangeType GetWeaponRangeType() const;

	virtual bool GetMeleeWeaponData_Implementation(
		FName StartSocketName,
		FName EndSocketName,
		TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const override;
	virtual bool GetMeleeDualWeaponData_Implementation(
		FName StartSocketName,
		FName EndSocketName,
		TArray<FMVMeleeWeaponData>& OutMeleeWeaponData) const override;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Weapon|Event")
	FMVOnEquippedWeaponChanged OnEquippedWeaponChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Default", meta = (DisplayName = "Default Weapon"))
	FDataTableRowHandle DefaultWeaponRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Visual")
	bool bManageWeaponMesh = true;

private:
	void ApplyEquippedWeaponState(const FMVEquippedWeaponState& NewState);
	static FMVEquippedWeaponState MakeStateFromWeaponRow(const FMVWeaponTableRow& WeaponRow);
	static FMVWeaponHitSnapshot MakeHitSnapshotFromState(const FMVEquippedWeaponState& WeaponState);
	bool CanEquipWeaponState(const FMVEquippedWeaponState& WeaponState) const;
	bool ValidateWeaponMesh(const UObject& WeaponMesh, const FGameplayTag& ItemTag) const;
	bool TryBuildMeleeWeaponData(
		UMeshComponent* MeshComponent,
		FName StartSocketName,
		FName EndSocketName,
		bool bSecondaryWeapon,
		FMVMeleeWeaponData& OutData) const;
	void SyncOwnerEquippedStyle() const;
	void RefreshOwnerCombatComponent() const;
	void ApplyWeaponVisual();
	void ClearWeaponVisual();
	void ApplyWeaponMeshVisual(
		UObject* LoadedWeaponMesh,
		TObjectPtr<UMeshComponent>& MeshComponentStorage,
		FName AttachSocketName,
		const FTransform& RelativeTransform,
		const TCHAR* ComponentNameBase);
	void ClearWeaponMeshVisual(TObjectPtr<UMeshComponent>& MeshComponentStorage);
	UMeshComponent* EnsureWeaponMeshComponent(
		TObjectPtr<UMeshComponent>& MeshComponentStorage,
		const UObject& WeaponMesh,
		const TCHAR* ComponentNameBase);
	USceneComponent* ResolveWeaponAttachParent() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon", meta = (AllowPrivateAccess = "true"))
	FMVEquippedWeaponState CurrentWeaponState;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> WeaponMeshComponent;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> SecondaryWeaponMeshComponent;
};
