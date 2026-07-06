#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
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
 * 무기가 없는 상태는 허용하지 않고 기본 무기 row 또는 맨손 무기를 장착한다. 실제 Static/Skeletal 무기 메시에는
 * Trace_Start, Trace_End, Trace_Left, Trace_Right 소켓이 모두 있어야 한다.
 *
 * 라이프사이클:
 *   1) 생성자 -> 기본 맨손 무기 row 값을 준비한다.
 *   2) BeginPlay -> 현재 state가 없으면 DefaultWeaponRow를 우선 장착하고, 실패하면 맨손 무기를 장착한다.
 *   3) EquipWeaponFromRow/EquipBareHand -> state 갱신, 캐릭터 장비 스타일 갱신, 이벤트 브로드캐스트.
 *   4) HitResolver가 타격 확정 시점에 CaptureWeaponHitSnapshot으로 히트 전용 사본을 만든다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVWeaponComponent : public UActorComponent
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
	void EquipBareHand();

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

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Weapon|Event")
	FMVOnEquippedWeaponChanged OnEquippedWeaponChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Default")
	FMVWeaponTableRow BareHandWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Default")
	FDataTableRowHandle DefaultWeaponRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Visual")
	bool bManageWeaponMesh = true;

private:
	void ApplyEquippedWeaponState(const FMVEquippedWeaponState& NewState);
	static FMVEquippedWeaponState MakeStateFromWeaponRow(const FMVWeaponTableRow& WeaponRow);
	static FMVWeaponHitSnapshot MakeHitSnapshotFromState(const FMVEquippedWeaponState& WeaponState);
	bool CanEquipWeaponState(const FMVEquippedWeaponState& WeaponState) const;
	bool TryEquipDefaultWeapon();
	bool ValidateWeaponMesh(const UObject& WeaponMesh, const FGameplayTag& ItemTag) const;
	bool ValidateWeaponTraceSockets(const UObject& WeaponMesh, const FGameplayTag& ItemTag) const;
	void SyncOwnerEquippedStyle() const;
	void RefreshOwnerCombatComponent() const;
	void ApplyWeaponVisual();
	void ClearWeaponVisual();
	UMeshComponent* EnsureWeaponMeshComponent(const UObject& WeaponMesh);
	USceneComponent* ResolveWeaponAttachParent() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon", meta = (AllowPrivateAccess = "true"))
	FMVEquippedWeaponState CurrentWeaponState;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Weapon|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> WeaponMeshComponent;
};
