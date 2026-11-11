#pragma once
#include "Combat/Boss/ElectricEelMan/MVElectricEelManAbility.h"
#include "MVElectricEelManRAbility.generated.h"
/** Q/W/E 9회 발동 후 3초 충전, 이후 1초 간격으로 총 3회 광역 피해
 * 발동 시 횟수 소비, 각 방출은 별도 적중 기록, 중단 시 남은 방출 취소
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVElectricEelManRAbility : public UMVElectricEelManAbility
{
	GENERATED_BODY()
public:
	virtual bool CanActivateAbility() override;
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|R", meta=(ClampMin="1"))
	float BurstRadius = 900.0f;
protected:
	virtual void TickSkill(float Elapsed) override;
private:
	int32 BurstCount = 0;
};
