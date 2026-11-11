#pragma once
#include "Combat/Boss/ElectricEelMan/MVElectricEelManAbility.h"
#include "MVElectricEelManEAbility.generated.h"
class UMVInputManagerComponent;
/** 대상 방향 돌진·포획·지연 전기 피해, 시퀀스 연결 지점은 로그로 제공
 * 포획 동안 입력·이동 잠금 소유, 종료·중단·사망·소멸에서 자신의 잠금만 해제
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVElectricEelManEAbility : public UMVElectricEelManAbility
{
	GENERATED_BODY()
public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|E", meta=(ClampMin="1"))
	float DashDistance = 450.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|E", meta=(ClampMin="0.05"))
	float DashDuration = 0.35f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|E", meta=(ClampMin="1"))
	float GrabDistance = 150.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|E", meta=(ClampMin="0"))
	float DamageDelay = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill|E", meta=(ClampMin="0.05"))
	float HoldDuration = 1.5f;
protected:
	virtual void TickSkill(float Elapsed) override;
	virtual void StopExecution() override;
private:
	TWeakObjectPtr<AMVCharacterBase> Target;
	TWeakObjectPtr<UCharacterMovementComponent> VictimMovement;
	TWeakObjectPtr<UMVInputManagerComponent> LockedInput;
	EMovementMode VictimMode = MOVE_None;
	uint8 VictimCustomMode = 0;
	FVector DashStart = FVector::ZeroVector;
	FVector DashEnd = FVector::ZeroVector;
	float GrabTime = -1.0f;
	bool bDamageDelivered = false;
};
