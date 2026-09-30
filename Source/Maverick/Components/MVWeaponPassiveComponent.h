#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Struct/MVStatusEffectTypes.h"
#include "Struct/MVWeaponTypes.h"
#include "MVWeaponPassiveComponent.generated.h"

class UMVStatusEffectComponent;
class UMVWeaponComponent;

/**
 * 현재 장착한 무기의 패시브 본체를 활성화하고 해제한다.
 * 시작 시 현재 무기를 확인하고, 이후 장착 변경 이벤트를 따른다.
 * 자신이 적용한 상태 효과 핸들만 보관하고 제거한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVWeaponPassiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVWeaponPassiveComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleEquippedWeaponChanged(const FMVEquippedWeaponState& WeaponState);

	void ClearActivePassives();

	TWeakObjectPtr<UMVWeaponComponent> BoundWeaponComponent;
	TWeakObjectPtr<UMVStatusEffectComponent> BoundStatusEffectComponent;

	UPROPERTY(Transient)
	TArray<FMVStatusEffectHandle> ActivePassiveHandles;
};