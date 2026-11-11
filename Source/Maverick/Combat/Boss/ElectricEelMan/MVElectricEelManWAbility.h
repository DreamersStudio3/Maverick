#pragma once
#include "Combat/Boss/ElectricEelMan/MVElectricEelManAbility.h"
#include "MVElectricEelManWAbility.generated.h"
/** 짧은 도약 후 주먹 범위 100% Stagger, 나머지 충격파 범위 50% Flinch 적용
 * 시작 시 목표 지점 고정, 공용 타이머·이동 복구 사용, 착지 시 한 번만 판정
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVElectricEelManWAbility : public UMVElectricEelManAbility
{
	GENERATED_BODY()
public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|W", meta=(ClampMin="0.05"))
	float JumpDuration = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|W", meta=(ClampMin="0"))
	float JumpHeight = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|W", meta=(ClampMin="0"))
	float JumpDistance = 300.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|W")
	FVector ImpactOffset = FVector(120, 0, -40);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|W", meta=(ClampMin="1"))
	float FistRadius = 120.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|W", meta=(ClampMin="1"))
	float ShockwaveRadius = 450.0f;
protected:
	virtual void TickSkill(float Elapsed) override;
private:
	FVector JumpStart = FVector::ZeroVector;
	FVector JumpEnd = FVector::ZeroVector;
};
