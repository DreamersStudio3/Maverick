#pragma once

#include "Combat/MVAbilityBase.h"
#include "MVWeaponSkillCircleAbility.generated.h"

class UMVWeaponSkillAreaWindow;

/** 몽타주 Notify 구간에 시전자 중심 360도 공격을 연다. BP는 두 반경을 지정하며 종료 시 고정 영역을 해제한다. */
UCLASS(Blueprintable)
class MAVERICK_API UMVWeaponSkillCircleAbility : public UMVAbilityBase
{
	GENERATED_BODY()

public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	virtual void EndAbility_Implementation() override;

protected:
	/** 이 거리 미만은 Flinch, 경계부터는 Airborne을 요청한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Circle", meta = (ClampMin = "0", Units = "cm"))
	float InnerRadius = 700.0f;

	/** 공격에 포함하는 최대 수평 거리. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Circle", meta = (ClampMin = "0", Units = "cm"))
	float OuterRadius = 1000.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMVWeaponSkillAreaWindow> Window;
};
